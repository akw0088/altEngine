//=============================================================================
// This file is part of the altEngine distribution
// (https://github.com/akw0088/altEngine/)
// Copyright (c) 2018 Alexander Wright All Rights Reserved.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON - INFRINGEMENT.IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//=============================================================================

///=============================================================================
/// File: engine.cpp
///=============================================================================
/// Engine class, which interfaces with the OS windowing system, this engine is
/// single threaded and will always remain single threaded. We don't want to
/// introduce complicated synchronisation that ends up being more complex and
/// slower than a single thread with cooperative multitasking. Makes debugging
/// way easier.
///
/// Debugging is twice as hard as writing the code in the first place. Therefore,
/// if you write the code as cleverly as possible, you are, by definition, not
/// smart enough to debug it.
///
/// - Brian W.Kernighan and P.J.Plauger in The Elements of Programming Style.
///
/// Accepts keyboard / mouse / network input / timer events
/// outputs rendered image, sound, etc
///=============================================================================



#include "include.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma warning(4:4928) // intellisense bug from my if (1 == sscanf()) line


extern double com_maxfps; // Max FPS limit
extern const char *models[23]; // array of quake 3 model names
unsigned int dissolve_tex;	// texture used for dissolve effect (should probably be moved into Engine class)
const char *teams[3] =  // array of team options (red, blue, spectator/none)
{
	"Red",
	"Blue",
	"None"
};


extern int raster_tcount;
extern int raster_target;


// Prototypes
unsigned int get_url(char *host, char *path, char *response, unsigned int size);


///=============================================================================
/// Function: Engine
///=============================================================================
/// Description: Constructs engine class
///
/// Parameters: None
///=============================================================================
Engine::Engine() :
	netcode(this)
{
	initialized = false;		// initialized flag (prevent rendering before things are set up)
	max_dynamic = 100;			// maximum dynamic entities (bullet casing, wall marks etc) 300 is causing network issues
	max_player = 16;			// Max number of players for network games, arbitrarily set to 16
	max_sources = 32;			// Maximum number of concurrent sound sources
	current_res = 0;			// current resolution
	current_model = 21;			// current model (Ranger)
	current_team = 2;			// Current team (None)
	num_team = 3;				// number of teams
	num_model = 23;				// number of models
	controller = 0;				// xbox controller index number (0-4)
	show_names = false;			// flag whether to draw player names above 3d model
	show_lines = false;			// flag whether to draw lines (usually for func_mover paths)
	show_debug = false;			// flag whether to draw debug information
	show_hud = true;			// flag whether to show the hud
	show_box = false;
	pick_mode = false;
	collision_detect_enable = true; // collision detection enable flag for non-player objects
	num_bot = 0;				// number of active bots
	emitter.enabled = false;	// particle emitter enabled (currently only allows one source emitter of particle effects)
	emitter_count = 0;			// emitter timer count (emitter active when above zero)
	demo = false;				// demo flag (whether using demo quake3 pk3, used to distinguish available maps in menu)
	voted = false;				// flag for callvote votes
	shadowmaps = false;			// flag whether shadow maps are enabled (requires depth maps generated per light source, performance intensive flag)
	node = NULL;				// graph node pointer, used for A* pathfinding
	no_tex = 0;					// no texture texture object (texture used when no texture is loaded, eg: screen before map loads)
	render_quad = 0;			// texture object for frame buffer object, essentially the rendered screen when using frame buffers (render resolution may differ from monitor resolution)
	render_depth = 0;			// texture object for frame buffer object, the depth buffer for the rendered scene
	render_ndepth = 0;			// texture object for frame buffer object, the normal vectors encoded in RGB for the rendered scene
	multisample = 0;			// multisample level used by GPU (Anti-Aliasing)
	lightning_vbo = 0;			// lightning vertex buffer object, used by lightning gun (composed of points rendered as particles)
	lightning_ibo = 0;			// lightning index buffer object, used by lightning gun (composed of points rendered as particles)
	global_vao = 0;				// global vertex array object, used to hold vertex attributes (which are uniform for everything rendered)
	render_fbo = 0;				// render frame buffer object, used for render to texture
	game = NULL;				// pointer to game base class, mostly quake3.cpp, but can theoretically plug in something else (see commando.cpp, which is probably outdated)
	num_pk3 = 0;				// number of loaded pk3 files
	num_shader = 0;				// number of q3 .shader clauses loaded
	param1 = NULL;				// generic parameter from operating system (differs based on OS) Usually something like window handles needed for operations
	param2 = NULL;				// generic parameter from operating system (differs based on OS) Usually something like window handles needed for operations
	particle_tex = 0;			// particle texture used in particle system
	ref = NULL;					// used for A* path finding
	thug22 = NULL;				// thug22 wavefront obj model that was used for the player model, still kept around for player sizing, should delete eventually
	sensitivity = 1.0f;			// mouse sensitivity
	spiral_vbo = 0;				// spiral vertex buffer object used for rail gun particle effect
	spiral_ibo = 0;				// spiral index buffer object used for rail gun particle effect
	all_lights = false;			// all_lights flag, used to enable all lights (performance impact, usually lights are enabled by proximity and whether the are in the BSP PVS
	ingame_menu_timer = 0;		// timer used for the menu system (prevent rapid enable / disable of menu on button presses)
	fullscreen_timer = 0;		// timer used for entering / exiting full screen mode (prevent rapid enable / disable of full screen)
	filter = 0;					// filter used by sound system (EAX effects, like cavern, bathroom, etc)

	bloom_threshold = 0.9f;		// bloom threshold (objects brighter than this are bloomed)
	bloom_strength = 0.5f;		// bloom strength (blend percentage of bloom with image)
	bloom_amount = 20.0f;		// bloom amount (amount of bloom)

	animation = NULL;

	picked_ent = -1;
	edit_ent = -1;

	// far = 0.1  near = 0.3
	dof_near = 0.4f;			// depth of field near distance (outside of near / far are blurred)
	dof_far = 0.1f;				// depth of field far distance (outside of near / far are blurred)

	fb_width = FBO_RESOLUTION;	// frame buffer render resolution (independent of monitor resolution, defaults may be used for shadowmap depth image)
	fb_height = FBO_RESOLUTION; // frame buffer render resolution (independent of monitor resolution, defaults may be used for shadowmap depth image)
	res_scale = 1.0f;			// resolution scale value
	dynamic_resolution = false;	// flag that enables dynamically changing resolution to hit certain fps targets, not really recommended to enable
	xres = 0;					// xres, current resolution
	yres = 0;					// yres, current resolution
	tick_num = 0;				// tick count starting from zero incrementing each tick (roughly 16ms)
	testObj = 0;				// testObj, used for rendering simple object for testing a graphics API (Vulkan, software renderer, raytracer, etc)
	num_light = 0;				// number of active lights
	doom_sound = 0;				// doom sound loaded from WAD file (teleport sound, randomly played when bots are inactive, could load anything though)
	enable_portal = false;		// enable portals (eg: mirrors, portals, a render to texture from any vantage point later used as a wall texture)
	enable_blur = false;		// enable blur post processing effect
	enable_emboss = false;		// enable emboss post processing effect
	enable_bloom = true;		// enable bloom post processing effect
	enable_ssao = false;		// enable screen space ambient occulsion post processing effect
	debug_bloom = false;		// debug bloom
	enable_entities = true;		// enables shadow volume generation for entities (dynamic objects)


	memset(shader_list, 0, sizeof(shader_list));
	memset(hash_result, 0, sizeof(hash_result));
	enable_map_brush_shadow = true;
	
#ifdef OPENGL
	render_mode = MODE_INDIRECT; // Render to Frame Buffer Object (renders to texture)
#else
	render_mode = MODE_FORWARD;	// Render directly to video output buffer
#endif
    
    
#ifdef __APPLE__
    render_mode = MODE_FORWARD;    // Render directly to video output buffer
#endif
    
	raw_mouse = false;			// Enable raw mouse input (USB Human Interface Device events versus WM_MOUSE messages)
	ssao_level = 1.0f;			// level of screen space ambient occulsion (SSAO)
	object_level = 1.0f;		// parameter to SSAO
	//	ssao_radius = 5.0;
	ssao_radius = 0.01f;		// parameter to SSAO
	weight_by_angle = true;		// parameter to SSAO
	point_count = 8;			// parameter to SSAO
	randomize_points = true;	// parameter to SSAO
	enable_map = true;			// flag to enable / disable BSP rendering
	enable_terrain = false;		// flag to enable / disable terrain rendering
	enable_planet = false;		// flag to enable / disable planet rendering (extremely large heightmapped sphere, and I mean extremely large)
	light_list.reserve(128);	// pre-reserve light size to prevent hitching
	enable_stencil = false;		// enable stencil shadow volumes
	enable_map_shadows = false;	// enable shadow map (depth buffer) shadows (requires frame buffer rendering)
	shadow_light = 107;			// index used to select a subset of shadow volumes generated by light entity number indicated (for debugging stencil shadow volumes)

#ifdef VOICECHAT
	sprintf(voice.server, "%s", "127.0.0.1:65530"); // server and port for voice chat
#endif
	srand((unsigned int)time(NULL));


	fov = 50.0f;				// Field of view (double for quake3 style FOV, ie: 100 FOV)
	zNear = 1.0f;				// Near clipping plane
	zFar = 2001.0f;				// far clipping plane (currently flag sets infinite far plane clipping, so not really used)
	inf = true;					// use infinite far plane clipping

#ifdef __linux__
	sensitivity *= 0.1f;		// adjustment to make linux mouse sensitivity similar to windows sensitivity
#endif

#ifdef OPENMP
	omp_set_num_threads(16);	// Maximum number of threads used by OpenMP
#endif
}


///=============================================================================
/// Function: init
///=============================================================================
/// Description: initializes Engine class with information from underlying
/// windowing system (Windows, Linux, Mac OSX, etc)
///
/// Parameters:
///		void *p1 - generic parameter that varies based on OS
///		void *p2 - generic parameter that varies based on OS
///		char *cmdline - command line arguments
///=============================================================================
void Engine::init(void *p1, void *p2, char *cmdline)
{
	float ident[16] = { 1.0f, 0.0f, 0.0f, 0.0f,
						0.0f, 1.0f, 0.0f, 0.0f,
						0.0f, 0.0f, 1.0f, 0.0f,
						0.0f, 0.0f, 0.0f, 1.0f };

	Engine::param1 = p1;
	Engine::param2 = p2;
	initialized = true;
#ifndef D3D11
	enable_cloth = true;
#else
	enable_cloth = false;
#endif

	debugf("altEngine2 built %s\n", __DATE__);

	enum_resolutions();

	// load VM code (interpreted C code) will eventually move game logic into it
	// allows mods without releasing engine code and security as they have limited
	// function call ability
	//vm_main("media/vm/game.qvm", VM_INIT);


	// This is test code that just tests http downloads, it worked, should delete
	//char *http_response = (char *)malloc(1024 * 1024 * 800);
	//http://releases.ubuntu.com/18.04.1/ubuntu-18.04.1-desktop-amd64.iso
	//unsigned int num_read = get_url("127.0.0.1", "/18.04.1/ubuntu-18.04.1-desktop-amd64.iso", http_response, 1024 * 1024 * 800);
	//char *data = NULL;
	//int size = check_content_length((unsigned char *)http_response, num_read, &data);

	//char hash[256] = { 0 };
	//md5sum(data, size, hash);
	//printf("File hash %s\r\n", hash);
	//fd6ac870e4f54d2599a4f140f2687634
	//fd6ac870e4f54d2599a4f140f2687634
	//write_file("download.bin", data, size);

#ifdef G_QUAKE3
	game = new Quake3(); // set base game pointer
#endif

#ifdef OCULUS
	ovr.init(gfx);		// Oculus VR headset init, didn't like how it was specific to just oculus, will probably remove
#endif

	bind_keys();		// set default key binds

	identity = ident;
	projection = ident;

	//visual
	gfx.init(param1, param2);
	gfx.clear();
	gfx.swap();
	gfx.CreateVertexArrayObject(global_vao);
	gfx.SelectVertexArrayObject(global_vao);

	

	// create common vertex / index buffers
	CreateObjects();


	// pk3 file list with md5sum
	newlinelist("media/pk3list.txt", pk3_list, num_pk3, &pk3list);
	check_pk3_md5sum();

	// console command list
	newlinelist("media/cmdlist.txt", cmd_list, num_cmd, &cmdlist);

	// master server ips
	newlinelist("media/sv_master.txt", netcode.master_list, netcode.num_master, &masterlist);


	// texture 0 used when we just need a texture
	no_tex = load_texture(gfx, "media/notexture.tga", false, false, 0);

	// particles texture (rockets/grenade smoke)
	particle_tex = load_texture(gfx, "media/flare.png", false, false, 0);

	// not really used, but for color grading
	palette1 = load_texture(gfx, "media/palette.png", false, false, 0);
	palette2 = load_texture(gfx, "media/palette2.png", false, false, 0);

	dissolve_tex = load_texture(gfx, "media/textures/dissolve.jpg", false, false, 0);


	// skybox vertex / index buffers (cube or sphere)
#ifdef SOFTWARE
	make_skybox(gfx, q3map.skybox_vertex, q3map.skybox_index, q3map.skybox_vbo, q3map.skybox_ibo, false);
#else
	make_skybox(gfx, q3map.skybox_vertex, q3map.skybox_index, q3map.skybox_vbo, q3map.skybox_ibo, true);
#endif

	// player model uses this model for hitbox sizing, the model itself isnt actually shown (and is ugly)
	thug22 = new Entity();
	thug22->rigid = new EntRigidBody(thug22);
	thug22->model = thug22->rigid;
	thug22->model->load(gfx, "media/models/thug22/thug22");

	// Scale down so player closer match to q3 model size
	thug22->model->aabb[0] *= 0.7f;
	thug22->model->aabb[1] *= 0.7f;
	thug22->model->aabb[2] *= 0.7f;
	thug22->model->aabb[3] *= 0.7f;
	thug22->model->aabb[4] *= 0.7f;
	thug22->model->aabb[5] *= 0.7f;
	thug22->model->aabb[6] *= 0.7f;
	thug22->model->aabb[7] *= 0.7f;


	// global shader used for basic shading (menu, fonts, etc)
	global.init(&gfx);
	audio.init();




#ifndef __APPLE__
#ifdef WIN32
#ifndef DEDICATED
	WAVEFORMATEX wf;
	DWORD value = 0;

	wf.wFormatTag = WAVE_FORMAT_PCM;
	wf.nChannels = 2;
	wf.nSamplesPerSec = 44100;
	wf.nAvgBytesPerSec = (44100 * 4);
	wf.nBlockAlign = (2 * 16) / 8;
	wf.wBitsPerSample = 16;
	wf.cbSize = 0;

	// winmm used for voice chat output
	waveOutOpen((LPHWAVEOUT)&hWaveOut, WAVE_MAPPER, &wf, 0, 0, CALLBACK_NULL);
	waveOutGetVolume(hWaveOut, &value);
	menu.data.volume = value / 65535.0f;
#endif
#endif

	// voice chat "always on" style
#ifdef VOICECHAT
	voice.init(audio, netcode.qport);
	audio.capture_start();
#endif
#endif



	// Loads doom 1 sounds from WAD file, played during long idle periods
	// should probably move elsewhere
	wave_t wave;
	waveFormat_t format;
	wave.format = &format;

	int lump_size = 0;
	//DSSHOTGN
	//DSDMPAIN
	char *sound_lump = get_wadfile("media/doom1.wad", "DSTELEPT", &lump_size, &wad);
	if (sound_lump != NULL)
	{
		lump_to_wave(sound_lump, lump_size, &wave);
		audio.load_doom(wave, &doom_sound);
	}

	// main menu class / font rendering
	menu.init(&gfx, &audio, pk3_list, num_pk3);


	// creates audio sources (was originally per entity, but hit max source limit)
	for (int i = 0; i < max_sources; i++)
	{
		audio_source[i] = audio.create_source(false, false);
		global_source[i] = audio.create_source(false, true);
		audio_loop_source[i] = audio.create_source(true, false);
		global_loop_source[i] = audio.create_source(true, true);
	}

	// set audio fall off distances
	set_reference_distance(100.0f);


	// initialize game class
	game->init(this);

	// load menu state machines depending on if demo or full version of pak0.pk3
	// really only difference is map names available 3 in demo vs 19 or so in full version
	if (demo)
	{
		menu.load("media/newmenu.txt", "media/newstate.txt");
	}
	else
	{
		menu.load("media/fullmenu.txt", "media/fullstate.txt");
	}

	// load md5 model (really only loading that zombie soldier guy)
	printf("Loading md5 models...\n");
	load_md5();


	// frame buffer width and height (renders to texture, which is then scaled to screen on full screen quad)
	// dont always init with width and height set yet
	fb_width = (unsigned int)(1024 * res_scale);
	fb_height = (unsigned int)(1024 * res_scale);

	unsigned int normal_depth;

	// create initial frame buffer
	gfx.CreateFramebuffer(fb_width, fb_height, render_fbo, render_quad, render_depth, render_ndepth, multisample, true);

	// create frame buffers used for bloom
	gfx.CreateFramebuffer(fb_width, fb_height, mask_fbo, mask_quad, mask_depth, normal_depth, 0, false);
	gfx.CreateFramebuffer(fb_width, fb_height, blur1_fbo, blur1_quad, blur1_depth, normal_depth, 0, false);
	gfx.CreateFramebuffer(fb_width, fb_height, blur2_fbo, blur2_quad, blur2_depth, normal_depth, 0, false);

	// create frame buffers used for screen space ambient occlusion
	gfx.CreateFramebuffer(fb_width, fb_height, ssao_fbo, ssao_quad, ssao_depth, normal_depth, 0, false);


	// parse quake 3 shader files .shader
	load_q3_shaders();

	printf("Done\n");

#ifndef DIRECTX
	//render menu again for linux
#ifndef VULKAN
	// resize again, fixes issue on xwindows
	gfx.resize(xres,yres);
#endif

#ifdef WIN32
	int x, y, bpp, refresh_rate;

	// set resolution in options menu
	// Note: win10 resolution scaling can downscale you, so tracking monitor res and window res
	get_resolution(x, y, bpp, refresh_rate);
	sprintf(menu.data.resolution, "%dx%d %dHz", x, y, refresh_rate);
	sprintf(menu.data.window, "%dx%d", xres, yres);
#endif
	// draw menu
	gfx.SelectTexture(0, no_tex);

	menu.render(global);
	// swap once (cause rendering a menu screen at 200fps is stupid)
	gfx.swap();
#endif
	
	if (render_mode == MODE_INDIRECT)
	{
		// bind frame buffer texture
		gfx.bindFramebuffer(render_fbo, 2);
		gfx.resize(fb_width, fb_height);
		gfx.bindFramebuffer(0);
	}

#ifdef WIN32
	sprintf(server_comport, "COM1");
	sprintf(client_comport, "COM2");
#else
	sprintf(server_comport, "/dev/ttyS1");
	sprintf(client_comport, "/dev/ttyS2");
#endif
    
    if (cmdline != NULL)
    {
        char *server_com = strstr(cmdline, "server_com");
        if (server_com)
        {
            sprintf(server_comport, "%s", server_com + strlen("server_com "));
        }
        
        char *client_com = strstr(cmdline, "client_com");
        if (client_com)
        {
            sprintf(client_comport, "%s", client_com + strlen("client_com "));
        }
    }



#ifdef DEDICATED
	printf("Dedicated server mode\n");
	printf("Sending cmdline to console\n");
	printf("semicolon delimited example:\n;bind 65535;map media/maps/q3tourney2.bsp;\n");

	if (strlen(cmdline) <= 1)
	{
		printf("No cmdline set, using default: ;bind 65535;map media/maps/q3tourney2.bsp;\n");
		cmdline = ";bind 65535;map media/maps/q3tourney2.bsp;";
	}

	if (p2 != NULL)
	{
		char *name = strtok((char *)cmdline, ";");
		char *cmd = strtok(NULL, ";");

		printf("Binary path: %s\n", name);

		while (cmd != NULL)
		{
			printf("Command string: %s\n", cmd);
			console(cmd);
			cmd = strtok(NULL, ";");
		}
		printf("Finished processing cmdline\n");
	}
#endif
}

///=============================================================================
/// Function: check_pk3_md5sum
///=============================================================================
/// Description: so this checks loaded file checksums against checksums defined
/// in pk3list.txt, a pk3 file is just a zip file of game assets (levels, models,
/// sounds etc)
///
/// Parameters:
///		None
///=============================================================================
void Engine::check_pk3_md5sum()
{
	std::thread pool[64];

	for (unsigned int i = 0; i < num_pk3; i++)
	{
		hash_result[i][0] = '\0';
		pool[i] = std::thread(calc_hash, pk3_list[i] + FILE_OFFSET, hash_result[i]);
	}

	for (unsigned int i = 0; i < num_pk3; i++)
	{
		pool[i].join();
		debugf("Checking hash for %s...", pk3_list[i] + FILE_OFFSET);
		if (strncmp(pk3_list[i], hash_result[i], 32) == 0)
		{
			debugf("Good!\n");
		}
		else
		{
			if (strstr(pk3_list[i], "media/pak0.pk3"))
			{
				if (strstr(hash_result[i], "0613b3d4ef05e613a2b470571498690f"))
				{
					debugf("pak0.pk3 is from Q3A Demo\n");
					demo = true;
				}
				else
				{
					debugf("\n%s failed hash check:\n\t[%s] expected [%.32s]\n", pk3_list[i] + FILE_OFFSET, hash_result[i], pk3_list[i]);
				}
			}
			else
			{
				debugf("\n%s failed hash check:\n\t[%s] expected [%.32s]\n", pk3_list[i] + FILE_OFFSET, hash_result[i], pk3_list[i]);
			}
		}
	}
}

///=============================================================================
/// Function: load_q3_shaders
///=============================================================================
/// Description: this loads q3 shaders listed in shaderlist.txt
/// a shader in quake3 is a path like string that defines surface textures and
/// effects for a surface. Eg: water should be transparent and non-solid,
/// this teleporter should have a electric wavy effect
///
/// Maps will reference shader names, which are like file names, but without
/// an extension
///
/// Parameters:
///		None
///=============================================================================
void Engine::load_q3_shaders()
{
	//parse shaders
	printf("Loading Quake3 shaders...\n");
	get_shaderlist_pk3(shader_list, num_shader);

	if (num_shader == 0)
	{
		printf("Unable to load shaders!\n");
	}

	for (int i = 0; i < num_shader; i++)
	{
		char *shader_file = NULL;
		get_zipfile("media/pak0.pk3", shader_list[i], (unsigned char **)&shader_file, NULL);
		//shader_file = get_file(shader_list[i], NULL);

		printf("Parsing %s...\n", shader_list[i]);
		if (shader_file)
		{
			parse_shader(shader_file, surface_list, shader_list[i]);
			delete[] shader_file;
		}
		delete[] shader_list[i];
		shader_list[i] = NULL;
	}

	char *hack_list[512];
	unsigned int num_hack;
	newlinelist("media/hacklist.txt", hack_list, num_hack, &hacklist);
	for (unsigned int i = 0; i < num_hack; i += 2)
	{
		for (unsigned int j = 0; j < surface_list.size(); j++)
		{
			if (strcmp(hack_list[i], surface_list[j]->name) == 0)
			{
				int value = atoi(hack_list[i + 1]);
				if (value >= 0)
				{
					surface_list[j]->num_stage = value;
				}
				else if (value == -1)
				{
					for (unsigned int k = 0; k < surface_list[j]->num_stage; k++)
					{
						stage_t *stage = &surface_list[j]->stage[k];

						stage->flags.lightmap = false;
						stage->flags.blendfunc_add = false;
						stage->flags.blendfunc_blend = false;
						stage->flags.blendfunc_filter = false;
						stage->flags.blend_dst_color_one = false;
						stage->flags.blend_dst_color_one_minus_dst_alpha = false;
						stage->flags.blend_dst_color_src_alpha = false;
						stage->flags.blend_dst_color_src_color = false;
						stage->flags.blend_dst_color_zero = false;
						stage->flags.blend_one_minus_dst_color_zero = false;
						stage->flags.blend_one_minus_src_alpha_src_alpha = false;
						stage->flags.blend_one_one = false;
						stage->flags.blend_one_src_alpha = false;
						stage->flags.blend_one_src_color = false;
						stage->flags.blend_one_zero = false;
						stage->flags.blend_src_alpha_one_minus_src_alpha = false;
						stage->flags.blend_zero_one = false;
						stage->flags.blend_zero_src_alpha = false;
						stage->flags.blend_zero_src_color = false;
						stage->flags.alpha = true;
					}
					//					surface_list.erase(surface_list.begin() + j);
				}
				else if (value == -2)
				{
					delete surface_list[j];
					surface_list.erase(surface_list.begin() + j);
				}
				break;
			}
		}
	}
}



///=============================================================================
/// Function: load
///=============================================================================
/// Description: This will load a quake 3 bsp level and start the game loop
///
/// Parameters:
///		None
///=============================================================================
void Engine::load(char *level)
{
	matrix4 transformation;
	gametype_t gametype = GAMETYPE_DEATHMATCH;

	if (q3map.loaded)
		return;


	debug_tri_vert = -1;
	debug_tri_index = -1;
	debug_vec_vert = -1;
	debug_vec_index = -1;

	vec3 scale(1.0f, 1.0f, 1.0f);
	vec3 offset(0.0f, 0.0f, 0.0f);

	ball_pos = vec3(7, -5, 0);
	ball_radius = 2.0f;
	ball_time = 0.0f;

	q3map.enable_normalmap = true;
	mlight2.set_normalmap(1.0f);


	if (strstr(level, "ctf"))
	{
		gametype = GAMETYPE_CTF;
	}

	game->load(gametype);

	q3map.anim_list.clear();

	menu.delta("load", *this);
	gfx.clear();
	menu.render(global);
	gfx.swap();
	camera_frame.reset();


	if (post.init(&gfx))
		menu.print("Failed to load post shader");
	if (mlight2.init(&gfx))
		menu.print("Failed to load mlight2 shader");
	if (particle_render.init(&gfx))
		menu.print("Failed to load particle_render shader");
	if (particle_update.init(&gfx))
		menu.print("Failed to load particle_update shader");
	if (ssao.init(&gfx))
		menu.print("Failed to load screen_space shader");

	mlight2.set_dissolve(-1.0f);

	vec3 green(0.0f, 1.0f, 0.0f);
	mlight2.set_fog(0.0f, 100.0f, 500.0f, green);
	mlight2.set_contrast(2.0f);
	mlight2.set_ambient(0.2f);
	mlight2.set_lightmap(1.0f);
	mlight2.set_exposure(1.0f);
	mlight2.set_max(64);


	emitter.position = vec3(0.0f, 100.0f, 0.0f);
	emitter.vel_min = vec3(50.0f, 50.0f, 50.0f);
	emitter.vel_range = vec3(200.0f, 200.0f, 200.0f);
	emitter.color = 0x00FF00FF;
	emitter.size = 2.5f;
	emitter.life_min = 0.5f;
	emitter.life_range = 5.0f;
	emitter.gravity = vec3(0.0f, -GRAVITY, 0.0f);
	emitter.delta_time = 0.008f;
	emitter.num = 1;


	emitter.seed = vec3(rand_float(-100, 200.0f),
		rand_float(-100.0, 200.0f),
		rand_float(-100.0, 200.0f));

	bool hl = false;
	if (hl)
	{
		vec3 color(0.5f, 0.5f, 0.5f);
		matrix4 mvp = projection;

		global.Select();
		global.Params(mvp, 0);
		gfx.clear_color(color);
//		if (hlmap.load(gfx, "media/maps/de_dust2.bsp") != 0)
		if (q1map.load(gfx, "maps/start.bsp") != 0)
		{
			//de_dust2.bsp
			//de_inferno.bsp
			//de_nuke.bsp
			exit(0);
		}
	}
	else
	{
		if (q3map.load(level, pk3_list, num_pk3) == false)
			return;
	}

	if (hl == false)
	{
		q3map.generate_meshes(gfx);
	}


	menu.delta("entities", *this);
	gfx.clear();
	menu.render(global);
	gfx.swap();


	if (hl == false)
	{
		char entfile[128] = { 0 };
		sprintf(entfile, "media/%s.ent", q3map.map_name);
		char *entdata = get_file(entfile, NULL);

		//First n Entities for dynamic items (Dont allocate any at runtime)
		for (unsigned int i = 0; i < max_dynamic; i++)
		{
			Entity *entity = new Entity();
			entity_list.push_back(entity);
		}

		if (entdata != NULL)
		{
			parse_entity(this, entdata, entity_list, gfx, audio);
			delete[] entdata;
		}
		else
		{
			char filename[128];
			const char *data = q3map.get_entities();


			sprintf(filename, "media/%s.ent", q3map.map_name);
			parse_entity(this, data, entity_list, gfx, audio);
			write_file(filename, data, strlen(data));
		}

		debugf("Loaded %d entities\n", entity_list.size());


		int start = entity_list.size();

		char navfile[128] = { 0 };
		sprintf(navfile, "media/%s.nav", q3map.map_name);
		char *navdata = get_file(navfile, NULL);
		if (navdata != NULL)
		{
			parse_entity(this, navdata, entity_list, gfx, audio);
			delete[] navdata;
		}

		num_node = entity_list.size() - start;

		navdata_to_graph(ref, node, entity_list, start);
		print_graph(node, num_node);

		graph.load(node, num_node);
		//delete[] ref;


		game->setup_func(entity_list, &q3map);

		load_entities();
	}
#if 0
	if (strstr(level, "q3tourney2"))
	{
		int index = find_type(ENT_ITEM_ARMOR_COMBAT, 0);

		if (index != -1)
		{
			Entity *ent = entity_list[index];

			carinfo_t info;
			init_default_car(&info);
			ent->vehicle = new Vehicle(ent, &info);
			ent->rigid = ent->vehicle;
			ent->model = ent->rigid;
			ent->trigger = NULL;

			//		q3map.enable_textures = false;
			enable_bloom = false;

			ent->vehicle->load(gfx, "media/models/vehicle/car/car4");
			entity_list[index]->rigid->frame2ent(&camera_frame, input);
		}
	}
#endif

	// This renders map before loading textures

	int player = find_type(ENT_INFO_PLAYER_DEATHMATCH, 0);
	if (player != -1)
	{
		entity_list[player]->rigid->frame2ent(&camera_frame, input);
		camera_frame.set(transformation);
		camera_frame.pos = entity_list[player]->position;
	}
	if (hl == false)
		spatial_testing();
	gfx.clear();
	matrix4 mvp = transformation * projection;

	global.Select();
	render_entities(transformation, projection, true, false);


	if (player != -1)
	{
		entity_list[player]->rigid->frame2ent(&camera_frame, input);
		camera_frame.set(transformation);
		camera_frame.pos = entity_list[player]->position;
	}

	gfx.SelectTexture(0, no_tex);
	gfx.SelectTexture(1, no_tex);
	gfx.SelectTexture(2, no_tex);
	gfx.SelectTexture(3, no_tex);

	mvp = transformation * projection;
	global.Select();
	global.Params(mvp);

	if (hl == false)
	{
		q3map.enable_textures = false;

		q3map.render(camera_frame.pos, gfx, surface_list, mlight2, tick_num, pfrustum);
		q3map.lastIndex = -2; // force generation of new face lists
	}
	camera_frame.set(transformation);

//	render_entities(transformation, projection, true, true);

	menu.delta("textures", *this);
	menu.render(global);
	gfx.swap();


	if (hl == false)
	{
		q3map.load_textures(gfx, surface_list, pk3_list, num_pk3, menu.data.anisotropic);
	}
	menu.delta("loaded", *this);
	menu.stop();
	menu.ingame = false;
	menu.console = false;
//	menu.chat = true;
	menu.chat = false;


	//Setup render to texture

	if (hl == false)
	{
		// Generate depth cubemaps for each light
		if (render_mode == MODE_INDIRECT)
		{
			render_shadowmaps(true);
			gfx.bindFramebuffer(0);
		}

		// render portals at least once
		render_portalcamera();
	}

	if (enable_terrain)
	{
		vec3 offset(0.0f, -1e5, 0.0f);

		terrain.load(gfx, 500.0f, offset, "media/terrain/mt-ruapehu-and-mt-ngauruhoe.png", "media/terrain/terrain_big.png", 0);
	}
	if (enable_planet)
	{
		vec3 offset(0.0f, 0.0f, 0.0f);

		//isocube[0].load(gfx, "media/planet/earth_tex_10k.png", "media/planet/earth_height_10k.png", 9, 1e5);
		isosphere[0].load(gfx, "media/planet/moon_tex.png", "media/planet/moon_height.png", 9, 1e5, offset);
	}
#if 0
	isocube[0].load(gfx, "media/terrain/earth.png", "media/planet/moon_height.png",0, 100.0f);
	isocube[1].load(gfx, "media/terrain/earth.png", "media/planet/moon_height.png",1, 100.0f);
	isocube[2].load(gfx, "media/terrain/earth.png", "media/planet/moon_height.png",2, 100.0f);
	isocube[3].load(gfx, "media/terrain/earth.png", "media/planet/moon_height.png",3, 100.0f);
	isocube[4].load(gfx, "media/terrain/earth.png", "media/planet/moon_height.png",4, 100.0f);
	isocube[5].load(gfx, "media/terrain/earth.png", "media/planet/moon_height.png",5, 100.0f);
	isocube[6].load(gfx, "media/terrain/earth.png", "media/planet/moon_height.png",6, 100.0f);
	isocube[7].load(gfx, "media/terrain/earth.png", "media/planet/moon_height.png",7, 100.0f);
	isocube[8].load(gfx, "media/terrain/earth.png", "media/planet/moon_height.png",8, 100.0f);
	isocube[9].load(gfx, "media/terrain/earth.png", "media/planet/moon_height.png",9, 100.0f);
#endif

	if (netcode.server_flag)
	{
		netcode.report_master();
	}




	// map shadow volumes
	FILE *outfile = fopen("nul", "w");

#ifndef SOFTWARE
	if (strstr(q3map.map_name, "q3tourney2") != 0)
	{
		printf("Loading Brush Volumes...\r\n");
		if (radmap.load("media/maps/q3tourney2_saved_no_sky.map", outfile) == 0)
		{
			radmap.allocate_quads();
			radmap.generate_quads();
			//radmap.clip_quads();
			radmap.intersect_quads();
			//radmap.intersect_bigbox();



			printf("Creating Shadow Volumes...\r\n");
			shadow = new ShadowVolume[radmap.radent[0].num_brush];
			num_shadow_volume = radmap.radent[0].num_brush;

			vec3 vLight(0.0, 10000.0, 0.0);
			//create gfx buffers per brush
			for (unsigned int i = 0; i < radmap.radent[0].num_brush; i++)
			{
				radmap.quadent.quadbrush[i].vbo = gfx.CreateVertexBuffer(radmap.quadent.quadbrush[i].vert_array, radmap.quadent.quadbrush[i].num_vert);
				radmap.quadent.quadbrush[i].ibo = gfx.CreateIndexBuffer(radmap.quadent.quadbrush[i].index_array, radmap.quadent.quadbrush[i].num_index);
				shadow[i].CreateVolume(gfx, radmap.quadent.quadbrush[i].vert_array, radmap.quadent.quadbrush[i].index_array, radmap.quadent.quadbrush[i].num_index, vLight);
			}
		}

	}
#endif


	console("flyby");
}


///=============================================================================
/// Function: load_md5
///=============================================================================
/// Description: This will load a Doom3 MD5 model file and animations, currently
/// only using zcc, which is the humanoid zombie guy with a chaingun, but can
/// load anything really
///
/// Parameters:
///		None
///=============================================================================
void Engine::load_md5()
{
	animation = new char *[128];

#if 0
	int num_anim = 0;
	animation[ANIM_IDLE] = "media/md5/chaingun_idle.md5anim";
	num_anim++;
	animation[ANIM_ATTACK] = "media/md5/chaingun_stand_fire.md5anim";
	num_anim++;
	animation[ANIM_WALK] = "media/md5/chaingun_walk.md5anim";
	num_anim++;
	animation[ANIM_RUN] = "media/md5/chaingun_run.md5anim";
	num_anim++;
	animation[ANIM_PAIN] = "media/md5/chaingun_pain_chest.md5anim";
	num_anim++;
	animation[ANIM_ALERT] = "media/md5/chaingun_reload.md5anim";
	num_anim++;
	zcc.load("media/md5/zcc.md5mesh", (char **)animation, num_anim, gfx, 0);

	num_anim = 0;
	animation[ANIM_IDLE] = "media/md5/quake4/idle.md5anim";
	num_anim++;
	animation[ANIM_ATTACK] = "media/md5/quake4/machinegun_aim.md5anim";
	num_anim++;
	animation[ANIM_WALK] = "media/md5/quake4/walk.md5anim";
	num_anim++;
	animation[ANIM_RUN] = "media/md5/quake4/run.md5anim";
	num_anim++;
	animation[ANIM_PAIN] = "media/md5/quake4/rocketlauncher_raise.md5anim";
	num_anim++;
	animation[ANIM_ALERT] = "media/md5/quake4/salute.md5anim";

		animation[num_anim++] = "media/md5/quake4/af_pose.md5anim";
		animation[num_anim++] = "media/md5/quake4/cheer.md5anim";
		animation[num_anim++] = "media/md5/quake4/crouch.md5anim";
		animation[num_anim++] = "media/md5/quake4/crouch_down.md5anim";
		animation[num_anim++] = "media/md5/quake4/crouch_up.md5anim";
		animation[num_anim++] = "media/md5/quake4/crouch_walk_backward.md5anim";
		animation[num_anim++] = "media/md5/quake4/crouch_walk_forward.md5anim";
		animation[num_anim++] = "media/md5/quake4/darkmattergun_aim.md5anim";
		animation[num_anim++] = "media/md5/quake4/darkmattergun_aim_down.md5anim";
		animation[num_anim++] = "media/md5/quake4/darkmattergun_aim_up.md5anim";
		animation[num_anim++] = "media/md5/quake4/darkmattergun_fire.md5anim";
		animation[num_anim++] = "media/md5/quake4/darkmattergun_fire_down.md5anim";
		animation[num_anim++] = "media/md5/quake4/darkmattergun_fire_up.md5anim";
		animation[num_anim++] = "media/md5/quake4/darkmattergun_reload.md5anim";
		animation[num_anim++] = "media/md5/quake4/fall.md5anim";
		animation[num_anim++] = "media/md5/quake4/gauntlet_aim.md5anim";
		animation[num_anim++] = "media/md5/quake4/gauntlet_aim_down.md5anim";
		animation[num_anim++] = "media/md5/quake4/gauntlet_aim_up.md5anim";
		animation[num_anim++] = "media/md5/quake4/gauntlet_fire.md5anim";
		animation[num_anim++] = "media/md5/quake4/gauntlet_fire_down.md5anim";
		animation[num_anim++] = "media/md5/quake4/gauntlet_fire_up.md5anim";
		animation[num_anim++] = "media/md5/quake4/grab_a.md5anim";
		animation[num_anim++] = "media/md5/quake4/grab_b.md5anim";
		animation[num_anim++] = "media/md5/quake4/grenadelauncher_aim.md5anim";
		animation[num_anim++] = "media/md5/quake4/grenadelauncher_aim_down.md5anim";
		animation[num_anim++] = "media/md5/quake4/grenadelauncher_aim_up.md5anim";
		animation[num_anim++] = "media/md5/quake4/grenadelauncher_fire.md5anim";
		animation[num_anim++] = "media/md5/quake4/grenadelauncher_fire_down.md5anim";
		animation[num_anim++] = "media/md5/quake4/grenadelauncher_fire_up.md5anim";
		animation[num_anim++] = "media/md5/quake4/grenadelauncher_lower.md5anim";
		animation[num_anim++] = "media/md5/quake4/grenadelauncher_raise.md5anim";
		animation[num_anim++] = "media/md5/quake4/grenadelauncher_reload.md5anim";
		animation[num_anim++] = "media/md5/quake4/hyperblaster_aim.md5anim";
		animation[num_anim++] = "media/md5/quake4/hyperblaster_aim_down.md5anim";
		animation[num_anim++] = "media/md5/quake4/hyperblaster_aim_up.md5anim";
		animation[num_anim++] = "media/md5/quake4/hyperblaster_fire.md5anim";
		animation[num_anim++] = "media/md5/quake4/hyperblaster_fire_down.md5anim";
		animation[num_anim++] = "media/md5/quake4/hyperblaster_fire_up.md5anim";
		animation[num_anim++] = "media/md5/quake4/hyperblaster_reload.md5anim";
		animation[num_anim++] = "media/md5/quake4/idle.md5anim";
		animation[num_anim++] = "media/md5/quake4/idle_loadscreen.md5anim";
		animation[num_anim++] = "media/md5/quake4/jump.md5anim";
		animation[num_anim++] = "media/md5/quake4/lightninggun_aim.md5anim";
		animation[num_anim++] = "media/md5/quake4/lightninggun_aim_down.md5anim";
		animation[num_anim++] = "media/md5/quake4/lightninggun_aim_up.md5anim";
		animation[num_anim++] = "media/md5/quake4/lightninggun_fire.md5anim";
		animation[num_anim++] = "media/md5/quake4/lightninggun_fire_down.md5anim";
		animation[num_anim++] = "media/md5/quake4/lightninggun_fire_up.md5anim";
		animation[num_anim++] = "media/md5/quake4/machinegun_aim.md5anim";
		animation[num_anim++] = "media/md5/quake4/machinegun_aim_down.md5anim";
		animation[num_anim++] = "media/md5/quake4/machinegun_aim_up.md5anim";
		animation[num_anim++] = "media/md5/quake4/machinegun_fire.md5anim";
		animation[num_anim++] = "media/md5/quake4/machinegun_fire_down.md5anim";
		animation[num_anim++] = "media/md5/quake4/machinegun_fire_up.md5anim";
		animation[num_anim++] = "media/md5/quake4/machinegun_lower.md5anim";
		animation[num_anim++] = "media/md5/quake4/machinegun_raise.md5anim";
		animation[num_anim++] = "media/md5/quake4/mp_crouch_walk.md5anim";
		animation[num_anim++] = "media/md5/quake4/mp_crouch_walk_backward.md5anim";
		animation[num_anim++] = "media/md5/quake4/nailgun_aim.md5anim";
		animation[num_anim++] = "media/md5/quake4/nailgun_aim_down.md5anim";
		animation[num_anim++] = "media/md5/quake4/nailgun_aim_up.md5anim";
		animation[num_anim++] = "media/md5/quake4/nailgun_fire.md5anim";
		animation[num_anim++] = "media/md5/quake4/nailgun_fire_down.md5anim";
		animation[num_anim++] = "media/md5/quake4/nailgun_fire_up.md5anim";
		animation[num_anim++] = "media/md5/quake4/nailgun_reload.md5anim";
		animation[num_anim++] = "media/md5/quake4/pain.md5anim";
		animation[num_anim++] = "media/md5/quake4/pistol_aim.md5anim";
		animation[num_anim++] = "media/md5/quake4/pistol_aim_down.md5anim";
		animation[num_anim++] = "media/md5/quake4/pistol_aim_up.md5anim";
		animation[num_anim++] = "media/md5/quake4/pistol_fire.md5anim";
		animation[num_anim++] = "media/md5/quake4/pistol_fire_down.md5anim";
		animation[num_anim++] = "media/md5/quake4/pistol_fire_up.md5anim";
		animation[num_anim++] = "media/md5/quake4/railgun_aim.md5anim";
		animation[num_anim++] = "media/md5/quake4/railgun_aim_down.md5anim";
		animation[num_anim++] = "media/md5/quake4/railgun_aim_up.md5anim";
		animation[num_anim++] = "media/md5/quake4/railgun_fire.md5anim";
		animation[num_anim++] = "media/md5/quake4/railgun_fire_down.md5anim";
		animation[num_anim++] = "media/md5/quake4/railgun_fire_up.md5anim";
		animation[num_anim++] = "media/md5/quake4/rocketlauncher_aim.md5anim";
		animation[num_anim++] = "media/md5/quake4/rocketlauncher_aim_down.md5anim";
		animation[num_anim++] = "media/md5/quake4/rocketlauncher_aim_up.md5anim";
		animation[num_anim++] = "media/md5/quake4/rocketlauncher_fire.md5anim";
		animation[num_anim++] = "media/md5/quake4/rocketlauncher_fire_down.md5anim";
		animation[num_anim++] = "media/md5/quake4/rocketlauncher_fire_up.md5anim";
		animation[num_anim++] = "media/md5/quake4/rocketlauncher_lower.md5anim";
		animation[num_anim++] = "media/md5/quake4/rocketlauncher_raise.md5anim";
		animation[num_anim++] = "media/md5/quake4/rocketlauncher_reload.md5anim";
		animation[num_anim++] = "media/md5/quake4/run.md5anim";
		animation[num_anim++] = "media/md5/quake4/run_backwards.md5anim";
		animation[num_anim++] = "media/md5/quake4/salute.md5anim";
		animation[num_anim++] = "media/md5/quake4/shotgun_aim.md5anim";
		animation[num_anim++] = "media/md5/quake4/shotgun_aim_down.md5anim";
		animation[num_anim++] = "media/md5/quake4/shotgun_aim_up.md5anim";
		animation[num_anim++] = "media/md5/quake4/shotgun_fire.md5anim";
		animation[num_anim++] = "media/md5/quake4/shotgun_fire_down.md5anim";
		animation[num_anim++] = "media/md5/quake4/shotgun_fire_up.md5anim";
		animation[num_anim++] = "media/md5/quake4/shotgun_reload.md5anim";
		animation[num_anim++] = "media/md5/quake4/soft_land.md5anim";
		animation[num_anim++] = "media/md5/quake4/strafe_jump_left.md5anim";
		animation[num_anim++] = "media/md5/quake4/strafe_jump_right.md5anim";
		animation[num_anim++] = "media/md5/quake4/strafe_left.md5anim";
		animation[num_anim++] = "media/md5/quake4/strafe_right.md5anim";
		animation[num_anim++] = "media/md5/quake4/taunt_1.md5anim";
		animation[num_anim++] = "media/md5/quake4/walk.md5anim";
		animation[num_anim++] = "media/md5/quake4/walk_backwards.md5anim";
		animation[num_anim++] = "media/md5/quake4/walk_left.md5anim";
		animation[num_anim++] = "media/md5/quake4/walk_right.md5anim";
		zcc.load("media/md5/quake4/player.md5mesh", (char **)animation, num_anim, gfx, 0);
#endif

	load_quake_md5();
}




void Engine::load_quake_md5()
{

	animation = new char *[128];

	int num_anim = 1;

	animation[ANIM_IDLE] = "media/md5/quake/player.md5anim";
	md5_model[RANGER].load("media/md5/quake/player.md5mesh", (char **)animation, num_anim, gfx, 0);

	animation[ANIM_IDLE] = "media/md5/quake/soldier.md5anim";
	md5_model[GRUNT].load("media/md5/quake/soldier.md5mesh", (char **)animation, num_anim, gfx, 0);

	animation[ANIM_IDLE] = "media/md5/quake/dog.md5anim";
	md5_model[DOG].load("media/md5/quake/dog.md5mesh", (char **)animation, num_anim, gfx, 0);

	animation[ANIM_IDLE] = "media/md5/quake/ogre.md5anim";
	md5_model[OGRE].load("media/md5/quake/ogre.md5mesh", (char **)animation, num_anim, gfx, 0);

	animation[ANIM_IDLE] = "media/md5/quake/zombie.md5anim";
	md5_model[ZOMBIE].load("media/md5/quake/zombie.md5mesh", (char **)animation, num_anim, gfx, 0);

	animation[ANIM_IDLE] = "media/md5/quake/wizard.md5anim";
	md5_model[WIZARD].load("media/md5/quake/wizard.md5mesh", (char **)animation, num_anim, gfx, 0);

	animation[ANIM_IDLE] = "media/md5/quake/knight.md5anim";
	md5_model[KNIGHT].load("media/md5/quake/knight.md5mesh", (char **)animation, num_anim, gfx, 0);

	animation[ANIM_IDLE] = "media/md5/quake/demon.md5anim";
	md5_model[DEMON].load("media/md5/quake/demon.md5mesh", (char **)animation, num_anim, gfx, 0);

	animation[ANIM_IDLE] = "media/md5/quake/shambler.md5anim";
	md5_model[SHAMBLER].load("media/md5/quake/shambler.md5mesh", (char **)animation, num_anim, gfx, 0);

	animation[ANIM_IDLE] = "media/md5/quake/hknight.md5anim";
	md5_model[DEATH_KNIGHT].load("media/md5/quake/hknight.md5mesh", (char **)animation, num_anim, gfx, 0);

	animation[ANIM_IDLE] = "media/md5/quake/enforcer.md5anim";
	md5_model[ENFORCER].load("media/md5/quake/enforcer.md5mesh", (char **)animation, num_anim, gfx, 0);

	animation[ANIM_IDLE] = "media/md5/quake/fish.md5anim";
	md5_model[FISH].load("media/md5/quake/fish.md5mesh", (char **)animation, num_anim, gfx, 0);

	animation[ANIM_IDLE] = "media/md5/quake/tarbaby.md5anim";
	md5_model[SPAWN].load("media/md5/quake/tarbaby.md5mesh", (char **)animation, num_anim, gfx, 0);

	animation[ANIM_IDLE] = "media/md5/quake/shalrath.md5anim";
	md5_model[VORE].load("media/md5/quake/shalrath.md5mesh", (char **)animation, num_anim, gfx, 0);


}
///=============================================================================
/// Function: render
///=============================================================================
/// Description: This is the main rendering entry point, the engine essentially
/// renders in an infinite loop, only getting 16ms timer interrupts to advance
/// the game state (Note: things like mouse events occur 125hz to 1000hz)
///
/// Function does not modify game state and should only read object positions
/// and orientations for drawing
///
/// Parameters:
///		None
///=============================================================================
void Engine::render(double last_frametime)
{
#ifdef WIN32
	int fpOld, fpNew;
	fpOld = _controlfp(0, 0);
	//fpNew = fpOld & ~(EM_OVERFLOW | EM_UNDERFLOW | EM_INEXACT | EM_ZERODIVIDE | EM_DENORMAL | EM_INVALID);
	fpNew = fpOld & ~(EM_ZERODIVIDE | EM_INVALID);
	_controlfp(fpNew, MCW_EM);
#endif

	gen_frustum(&camera_frame, &frustum);
	pfrustum = &frustum;

#ifdef VULKAN
	gfx.DrawArrayTri(0, 0, 6, 6);
#else
#if 0
	test_triangle();
	return;
#endif

	if (q3map.loaded == false && hlmap.loaded == false && q1map.loaded == false)
		return;


	// render a quake1 map (simple rendering compared to q3)
	if (q1map.loaded)
	{
		matrix4 transformation;

		gfx.SelectTexture(0, no_tex);
		gfx.SelectTexture(1, no_tex);
		gfx.SelectTexture(2, no_tex);
		gfx.SelectTexture(3, no_tex);
		camera_frame.set(transformation);
		matrix4 mvp = transformation * projection;
		global.Select();
		global.Params(mvp, 0);
		gfx.clear();
		q1map.render(gfx, camera_frame.pos);
		gfx.swap();
		return;
	}


	// render a source engine map (simple rendering compared to q3)
	if (hlmap.loaded)
	{
		matrix4 transformation;

		gfx.SelectTexture(0, no_tex);
		gfx.SelectTexture(1, no_tex);
		gfx.SelectTexture(2, no_tex);
		gfx.SelectTexture(3, no_tex);
		camera_frame.set(transformation);
		matrix4 mvp = transformation * projection;
		global.Select();
		global.Params(mvp, 0);
		gfx.clear();
		hlmap.render(gfx, camera_frame.pos);
		gfx.swap();
		return;
	}

#ifdef DEDICATED
	netcode.server_recv();
	return;
#endif

	// read packets from network every frame to prevent excessive buffering
	if (netcode.server_flag)
	{
		netcode.server_recv();
	}
	if (netcode.client_flag)
	{
		netcode.client_recv();
	}

	// indirect render ie: render to texture, then render texture to quad
	if (render_mode == MODE_INDIRECT)
	{
		bool depth_view = false;
		int spawn = find_type(ENT_PLAYER, 0);
		EntPlayer *player = NULL;

		if (spawn != -1)
			player = entity_list[spawn]->player;



		if (shadowmaps || all_lights)
		{
			// Generate depth cubemaps for each light
			render_shadowmaps(all_lights); // done at load time
			gfx.bindFramebuffer(0);
		}

		// render see through portal or mirror textures
		if (enable_portal)
		{
			render_portalcamera();
		}

#ifdef OCULUS
		ovr.get_pos(head, hpos, lh, lp, rh, rp, re, rep, le, lep, touch);
		vec3 old_pos = camera_frame.pos;
		camera_frame.pos = old_pos + rep;
		projection.perspective(110.0f, (4.0f / 5.0f), 1.0, 2001.0f, true);
		
		//render's fbo
		render_to_framebuffer(last_frametime);


		gfx.bindFramebuffer(ovr.ovr_fbo[0]);
		gfx.fbAttachTexture(ovr.get_tex(0));
		gfx.resize(ovr.eye_size[0].w, ovr.eye_size[0].h);
		gfx.clear_color(vec3(0.0f, 1.0f, 0.0f));
		gfx.clear();
		render_texture(render_quad, false);
		ovr_CommitTextureSwapChain(ovr.session, ovr.swap_chain[0]);

		camera_frame.pos = old_pos + lep;
		//render's fbo
		render_to_framebuffer(last_frametime);
		gfx.bindFramebuffer(ovr.ovr_fbo[1]);
		gfx.fbAttachTexture(ovr.get_tex(1));
		gfx.resize(ovr.eye_size[1].w, ovr.eye_size[1].h);
		gfx.clear_color(vec3(0.0f, 1.0f, 0.0f));
		gfx.clear();
		render_texture(render_quad, false);
		ovr_CommitTextureSwapChain(ovr.session, ovr.swap_chain[1]);
		camera_frame.pos = old_pos;

		//x.bindFramebuffer(render_fbo);
		//signed int texId;
		//ovr_GetMirrorTextureBufferGL(ovr.session, ovr.mirror_texture, &texId);
		//gfx.fbAttachTexture(texId);
	//	gfx.resize(gfx.width, gfx.height);
//		render_texture(render_quad, false);
#else
		//render to texture
		render_to_framebuffer(last_frametime);
#endif
		gfx.bindFramebuffer(0);

		// clear real backbuffer
		gfx.clear();
		gfx.resize(gfx.width, gfx.height);

		// Post processing (bloom, ssao, etc)
		if ( 1/*spawn == -1 || (player && player->current_light == 0)*/)
		{
			// render fbo to fullscreen quad
			if (enable_ssao)
			{
				render_texture(ssao_quad, false);
			}
			else
			{
				// final quad render
				render_texture(render_quad, false); 
			}

			if (enable_postprocess)
			{
				post_process(5, POST_EDGE);
			}
			else if (enable_blur)
			{
				post_process(5, POST_BLUR);
			}
			else if (enable_emboss)
			{
				post_process(5, POST_EMBOSS);
			}
		}
		else
		{
			// this is for viewing shadowmap color and depth buffers (from light perspective)
			for (unsigned int i = max_dynamic; i < entity_list.size(); i++)
			{

				if (spawn == -1)
					break;

				if (player == NULL)
					break;

				if (entity_list[i]->light)
				{
					EntLight *light = entity_list[i]->light;
					if (light->light_num == player->current_light)
					{
						if (input.scores)
						{
							shadow_light = i;
//							printf("selecting light %d for shadows\n", i);
						}

						if (input.duck == false)
						{
							depth_view = true;
							testObj = light->depth_tex[player->current_face];
						}
						else
						{
							printf("light %d %d\n", i, light->light_num);
							testObj = light->quad_tex[player->current_face];
						}
						break;
					}
				}
			}

			if (input.scores)
				render_texture(q3map.portal_tex, false);
			else
				render_texture(testObj, depth_view);

		}
	}


	// regular forward rendering (eg: directly to video memory, vs render to texture then render to quad)
	if (render_mode == MODE_FORWARD)
	{
		if (enable_stencil == false)
		{
			gfx.clear();
			render_scene(true);
			gfx.Depth(true);
			gfx.Blend(false);
			gfx.cleardepth();

			//render menu
			if (menu.chatmode == false)
				game->render_hud(last_frametime);
			else
				menu.render_chatmode(global);
			if (menu.ingame)
			{
				menu.render(global, true);
				if (menu.stringmode)
					menu.render_stringmode(global);
			}
			if (menu.console)
			{
				menu.render_console(global);
			}
		}
		else
		{
			matrix4 mvp;
			bool zpass_two = true;
			bool zpass = false;
			bool zfail = false;


			// This is regular forward rendering, but with shadow volumes which require stencil tests
			if (zpass && enable_stencil)
			{
				// Depth PASS Stencil Shadows
				gfx.clear();
				render_scene(false); // render without lights, fill stencil mask, render with lights
				gfx.Color(false);
				gfx.Stencil(true);
				gfx.Depth(false); // turn off depth writes
				gfx.StencilFunc(ALWAYS, 0, 0);

				gfx.CullFace(BACKFACE);
				gfx.StencilOp(KEEP, KEEP, INCR); // increment shadows that pass depth
				render_shadow_volumes();

				gfx.CullFace(FRONTFACE);
				gfx.StencilOp(KEEP, KEEP, DECR); // decrement shadows that backface pass depth
				render_shadow_volumes();

				gfx.Depth(true);
				gfx.Color(true);
				gfx.CullFace(BACKFACE);

				gfx.DepthFunc(LEQUAL); // depth already filled, need <=
				gfx.StencilOp(KEEP, KEEP, KEEP);

				//all lit surfaces will correspond to a 0 in the stencil buffer
				//all shadowed surfaces will be one
				gfx.StencilFunc(GEQUAL, 0, ~0);
				// render with lights
				render_scene(true);

				if (input.scores)
					render_shadow_volumes();

				gfx.DepthFunc(LESS);
				gfx.Stencil(false);
			}
			else if (zfail && enable_stencil)
			{
				// Depth FAIL Stencil Shadows (need caps)
				gfx.clear();
				render_scene(false); // render without lights, fill stencil mask, render with lights
				gfx.Color(false);
				gfx.Stencil(true);
				gfx.Depth(false); // turn off depth writes
				gfx.StencilFunc(ALWAYS, 0, 0);

				gfx.CullFace(FRONTFACE);
				gfx.StencilOp(KEEP, INCR, KEEP); // increment shadows that fail depth
				render_shadow_volumes();

				gfx.CullFace(BACKFACE);
				gfx.StencilOp(KEEP, DECR, KEEP); // decrement shadows that backface pass depth
				render_shadow_volumes();

				gfx.Depth(true);
				gfx.Color(true);

				gfx.DepthFunc(LEQUAL); // depth already filled, need <=
				gfx.StencilOp(KEEP, KEEP, KEEP);

				//all lit surfaces will correspond to a 0 in the stencil buffer
				//all shadowed surfaces will be one
				gfx.StencilFunc(GEQUAL, 0, ~0);
				// render with lights
				render_scene(true);

				if (input.scores)
					render_shadow_volumes();

				gfx.DepthFunc(LESS);
				gfx.Stencil(false);
			}
			else if (zpass_two && enable_stencil)
			{
				// Depth PASS Stencil Shadows -- using two sided stencil
				//(allows single call to draw shadow volumes instead of two)
				gfx.clear();
				render_scene(false); // render without lights, fill stencil mask, render with lights


				gfx.Depth(false); // turn off depth writes
				gfx.Color(false);
				gfx.CullFace(NONE);
				gfx.Stencil(true);

				gfx.TwoSidedStencilOp(BACKFACE, KEEP, KEEP, DECR_WRAP);
				gfx.TwoSidedStencilOp(FRONTFACE, KEEP, KEEP, INCR_WRAP);
				gfx.StencilFunc(ALWAYS, 0, ~0);
				render_shadow_volumes();

				gfx.Depth(true);
				gfx.Color(true);

				gfx.DepthFunc(LEQUAL); // depth already filled, need <=
				gfx.StencilOp(KEEP, KEEP, KEEP);

				//all lit surfaces will correspond to a 0 in the stencil buffer
				//all shadowed surfaces will be one
				gfx.StencilFunc(GEQUAL, 0, ~0);
				// render with lights
				//			gfx.cleardepth();
				gfx.CullFace(3);
				render_scene(true);

				if (input.scores)
					render_shadow_volumes(true);

				gfx.DepthFunc(LESS);
				gfx.Stencil(false);
			}
			else
			{
				gfx.clear();
				render_scene(true);
			}


			// render menu
			if (menu.chatmode == false)
				game->render_hud(last_frametime);
			if (menu.ingame)
			{
				menu.render(global, true);
				if (menu.stringmode)
					menu.render_stringmode(global);
			}
			if (menu.console)
				menu.render_console(global);
			if (menu.chatmode)
				menu.render_chatmode(global);

		}
	}

	gfx.Depth(true);
	gfx.Blend(false);

#ifdef DIRECTX
	gfx.cleardepth();
#endif

#ifdef OCULUS
	ovr.start_frame();

	ovr.submit_frame();
	//ovr_WaitToBeginFrame, ovr_BeginFrame, and ovr_EndFrame.
#endif

	gfx.swap();
#endif
#ifdef WIN32
	_controlfp(fpOld, MCW_EM);
#endif
}

///=============================================================================
/// Function: zoom
///=============================================================================
/// Description: This function is used to zoom in, does so by modifying the
/// projection matrix
///
/// Parameters:
///		None
///=============================================================================
void Engine::zoom(float level)
{
	projection.perspective(fov / level, (float)xres / yres, zNear, zFar, inf);
}


///=============================================================================
/// Function: render_portalcamera
///=============================================================================
/// Description: This function renders portals like mirrors or "see through"
/// teleporters, essentially just render to texture and then using that texture
/// on a surface
///
/// Parameters:
///		None
///=============================================================================
void Engine::render_portalcamera()
{
	if (render_mode != MODE_INDIRECT)
	{
		return;
	}

	q3map.lastIndex = -1;

	for (unsigned int i = max_dynamic; i < entity_list.size(); i++)
	{
		EntPortalCamera *portal = entity_list[i]->portal_camera;
		if (portal == NULL)
			continue;

		// Portal surface with target use misc_portal_camera, else it's a mirror
		if (entity_list[i]->entstring->target[0] != '\0' && entity_list[i]->ent_type == ENT_MISC_PORTAL_SURFACE)
			continue;

		if (entity_list[i]->ent_type == ENT_MISC_PORTAL_CAMERA)
			continue;

		matrix4 matrix;

		// generate matrices for each light face

		if (entity_list[i]->brushinfo == NULL)
		{
			matrix4::mat_backward(matrix, entity_list[i]->position);
			break;
		}
		else
		{
			switch (entity_list[i]->brushinfo->angle)
			{
			case 0:
			case 360:
				matrix4::mat_forward(matrix, entity_list[i]->position);
				break;
			case 45:
			case 90:
				matrix4::mat_right(matrix, entity_list[i]->position);
				break;
			case 135:
			case 180:
				matrix4::mat_backward(matrix, entity_list[i]->position);
				break;
			case 245:
			case 270:
				matrix4::mat_left(matrix, entity_list[i]->position);
				break;
			default:
				matrix4::mat_backward(matrix, entity_list[i]->position);
				break;
			}
		}

		matrix4 mvp = matrix * portal->portal_projection;

		gfx.bindFramebuffer(portal->fbo);
		gfx.resize(fb_width, fb_height);
		gfx.fbAttachTexture(portal->quad_tex);
		gfx.fbAttachDepth(portal->depth_tex);
		gfx.clear();

		vec3 offset(0.0f, 0.0f, 0.0f);
		mlight2.Select();
		mlight2.Params(mvp, light_list, light_list.size(), offset, tick_num);

		q3map.render(entity_list[i]->position, gfx, surface_list, mlight2, tick_num, pfrustum);
	
		render_entities(matrix, portal->portal_projection, false, false, false);
		render_players(matrix, portal->portal_projection, false, true);
		gfx.bindFramebuffer(0);


		//gfx.SelectShader(0);
		//gfx.Color(true);
	}

	q3map.lastIndex = -1;
}


///=============================================================================
/// Function: render_shadowmaps
///=============================================================================
/// Description: This function is related to shadow map rendering, shadow map
/// rendering requires rendering from the perspective of every light, and using
/// the depth buffer and some magic math to transform light depth buffers to the
/// same coordinate system to determine whether a point is in shadow or not
///
/// This is the function that renders from each light, currently does cube maps
/// at each light (up, down, forward, left right, back) which in retrospective 
/// is a bit too much, better to only focus on rendering views that *Matter*
/// to the final image, doing things like detecting whether the viewer is in
/// frame.
///
/// Parameters:
///		bool everything - whether to do all enabled lights or just the debug
///	light
///=============================================================================
void Engine::render_shadowmaps(bool everything)
{
	int depth_used = 0;

	float bias[16] = {
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0 };

	mlight2.Select();


	q3map.lastIndex = -1;
	q3map.enable_textures = false;
	gfx.Color(false);
	q3map.enable_shader = false;
	q3map.enable_blend = false;
	q3map.enable_sky = false;

	for (unsigned int i = max_dynamic; i < entity_list.size(); i++)
	{
		EntLight *light = entity_list[i]->light;
		if (light == NULL)
			continue;


		if (everything == false)
		{
			if (i == 100)
				continue;
		}


		matrix4 cube[6];

		// generate matrices for each light face
		matrix4::mat_right(cube[0], entity_list[i]->position);
		matrix4::mat_left(cube[1], entity_list[i]->position);
		matrix4::mat_top(cube[2], entity_list[i]->position);
		matrix4::mat_bottom(cube[3], entity_list[i]->position);
		matrix4::mat_forward(cube[4], entity_list[i]->position);
		matrix4::mat_backward(cube[5], entity_list[i]->position);

		light->shadow_flag = 0;

		if (everything == false)
		{
			if (depth_used == MAX_DEPTH)
				break;
		}

		for (int j = 0; j < 6; j++)
		{
			matrix4 mvp = cube[j] * light->shadow_projection;

			vec3 point = camera_frame.pos + camera_frame.forward + camera_frame.up;
			bool visible = aabb_visible(camera_frame.pos, point, mvp);

			if (visible == false)
				continue;


			if (everything == false)
			{
				int leaf_a = -1;
				int leaf_b = -1;
				if (q3map.vis_test(light->entity->position, camera_frame.pos, leaf_a, leaf_b) == false)
					continue;
			}


			depth_used++;
			if (everything == false)
			{
				if (depth_used == MAX_DEPTH)
					break;
			}

			//printf("Rendering light %d face %d\n", i, j);

			// setting bit 1 through 6
			light->shadow_flag |= (1 << j);


			// No real FPS improvement by masking color buffer
//			gfx.fbAttachTexture(0);
			gfx.bindFramebuffer(light->fbo_shadowmaps[j]);
			gfx.resize(SHADOWMAP_DEFAULT_RES, SHADOWMAP_DEFAULT_RES);
			gfx.clear();

			vec3 offset(0.0f, 0.0f, 0.0f);
			mlight2.Params(mvp, light_list, 0, offset, tick_num);

			light->shadow_matrix[j] = (cube[j] * light->shadow_projection) * bias;


			q3map.render(entity_list[i]->position, gfx, surface_list, mlight2, tick_num, pfrustum);
			render_entities(cube[j], light->shadow_projection, false, false, false);
			render_players(cube[j], light->shadow_projection, false, true);
			gfx.bindFramebuffer(0);
			//gfx.SelectShader(0);
			//gfx.Color(true);
		}
	}
	
	q3map.lastIndex = -1;
	q3map.enable_textures = true;
	gfx.Color(true);
	q3map.enable_shader = true;
	q3map.enable_blend = true;
	q3map.enable_sky = true;
}


///=============================================================================
/// Function: set_dynamic_resolution
///=============================================================================
/// Description: This sets the frame buffer object resolution dynamically based
/// on the frame time of the last frame, decresing resolution towards minimum
/// if below 60fps, increasing resolution if above 100 fps
///
/// kind of a neat idea, but in practice your FPS may be low due to CPU or something
/// else not tied to resolution, should probably remove this feature
///
/// Parameters:
///		None
///=============================================================================
void Engine::set_dynamic_resolution(double last_frametime)
{
	if (q3map.loaded)
	{
		double fps = 1000.0 / last_frametime;
		if (fps < 60.0f && res_scale > 0.1f)
		{
			res_scale *= 0.75f;
			fb_width = (unsigned int)(FBO_RESOLUTION * res_scale);
			fb_height = (unsigned int)(FBO_RESOLUTION * res_scale);
			gfx.DeleteFrameBuffer(render_fbo, render_quad, render_depth);
			gfx.CreateFramebuffer(fb_width, fb_height, render_fbo, render_quad, render_depth, render_ndepth, multisample, true);
		}
		else if (fps > 100.0f && res_scale < 2.0f)
		{
			res_scale *= 1.25f;
			fb_width = (unsigned int)(FBO_RESOLUTION * res_scale);
			fb_height = (unsigned int)(FBO_RESOLUTION * res_scale);
			gfx.DeleteFrameBuffer(render_fbo, render_quad, render_depth);
			gfx.CreateFramebuffer(fb_width, fb_height, render_fbo, render_quad, render_depth, render_ndepth, multisample, true);
		}
	}
	else if (q3map.loaded == false && (abs32(res_scale - 1.0f) > 0.001f))
	{
		res_scale = 1.0f;
		fb_width = (unsigned int)(FBO_RESOLUTION * res_scale);
		fb_height = (unsigned int)(FBO_RESOLUTION * res_scale);
		gfx.DeleteFrameBuffer(render_fbo, render_quad, render_depth);
		gfx.CreateFramebuffer(fb_width, fb_height, render_fbo, render_quad, render_depth, render_ndepth, multisample, true);
	}
}



void Engine::render_brushes(bool lights)
{
	vec3 offset(0.0f, 0.0f, 0.0f);

	matrix4 transformation;



	int player = find_type(ENT_PLAYER, 0);
	if (player != -1)
	{
		entity_list[player]->rigid->frame2ent(&camera_frame, input);
		camera_frame.set(transformation);
		camera_frame.pos = entity_list[player]->position;
	}

	camera_frame.set(transformation);
	matrix4 mvp = transformation * projection;



	global.Select();
	global.Params(mvp);
	gfx.SelectTexture(0, no_tex);


	for (unsigned int i = 0; i < radmap.radent[0].num_brush; i++)
	{
		gfx.SelectIndexBuffer(radmap.quadent.quadbrush[i].ibo);
		gfx.SelectVertexBuffer(radmap.quadent.quadbrush[i].vbo);
		gfx.DrawArrayTri(0, 0, radmap.quadent.quadbrush[i].num_index, radmap.quadent.quadbrush[i].num_vert);
	}
}


///=============================================================================
/// Function: render_to_framebuffer
///=============================================================================
/// Description: This handles the main rendering to a frame buffer object
/// essentially renders the scene to a texture, then renders a full screen quad
/// with that texture to present the final output. Doing rendering this ways
/// allows resolution to not be tied to physical monitor resolution, and also
/// makes post processing effects easier to perform
///
/// Parameters:
///		None
///=============================================================================
void Engine::render_to_framebuffer(double last_frametime)
{
	if (dynamic_resolution)
	{
		set_dynamic_resolution(last_frametime);
	}

	gfx.bindFramebuffer(render_fbo, 2);
	gfx.resize(fb_width, fb_height);

	if (enable_stencil)
	{
		// Depth PASS Stencil Shadows -- using two sided stencil
		//(allows single call to draw shadow volumes instead of two)
		gfx.clear();
		if (input.scores == false)
		{
			render_scene(false); // render without lights (also fill z-buffer), fill stencil mask, render with lights
		}
		//render_brushes(false);


		gfx.Depth(false); // turn off depth writes
		gfx.Color(false);
		gfx.CullFace(NONE);
		gfx.Stencil(true);



		// using two sided stencil, set different operations for front faces and back faces
		// allows us to render shadow volumes just once instead of twice for z-fail
		gfx.TwoSidedStencilOp(BACKFACE, KEEP, KEEP, DECR_WRAP);
		gfx.TwoSidedStencilOp(FRONTFACE, KEEP, KEEP, INCR_WRAP);
		gfx.StencilFunc(ALWAYS, 0, ~0);
		render_shadow_volumes();

		gfx.Depth(true);
		gfx.Color(true);

		gfx.DepthFunc(LEQUAL); // depth already filled, need <=
		gfx.StencilOp(KEEP, KEEP, KEEP);

		//all lit surfaces will correspond to a 0 in the stencil buffer
		//all shadowed surfaces will be negative
		gfx.StencilFunc(GEQUAL, 0, ~0);
		// render with lights
		gfx.CullFace(3); // enable face culling
		gfx.CullFace(BACKFACE);


		if (input.scores)
		{
			render_shadow_volumes(true);
		}
		else
		{
			if (shadowmaps)
				render_scene_using_shadowmap(true);
			else
				render_scene(true);
		}


		gfx.DepthFunc(LESS);
		gfx.Stencil(false);
	}
	else
	{
		gfx.clear();

		if (shadowmaps)
			render_scene_using_shadowmap(true);
		else
			render_scene(true);
	}


	if (enable_ssao)
	{
		render_ssao(debug_bloom);
		gfx.cleardepth();
//		gfx.bindFramebuffer(render_fbo, 2);
//		gfx.fbAttachTexture(ssao_quad);
	}

	int player = find_type(ENT_PLAYER, 0);


	if (menu.data.skyray)
	{
		render_skyray(debug_bloom);
		gfx.cleardepth();
	}

	if (enable_bloom)
	{
		render_bloom(debug_bloom);
		gfx.cleardepth();
	}

	if (player != -1)
	{
		EntRigidBody *rigid = entity_list[player]->rigid;
		
		if (rigid->flags.water && rigid->water_depth < 2048.0f)
		{
			render_wave(debug_bloom);
			gfx.cleardepth();
		}
		else
		{
			rigid->flags.water = false;
		}
	}

	//render menu
	if (menu.chatmode == false)
		game->render_hud(last_frametime);
	else
		menu.render_chatmode(global);
	if (menu.ingame)
	{
		menu.render(global, true);
		if (menu.stringmode)
			menu.render_stringmode(global);
	}
	if (menu.console)
		menu.render_console(global);

	gfx.Depth(true);
	gfx.Blend(false);

	gfx.bindFramebuffer(0);
	gfx.resize(xres, yres);
}

///=============================================================================
/// Function: CreateObjects
///=============================================================================
/// Description: This creates static objects like the full screen quad, lightning
/// gun particles, rail gun particles, etc (looks like a cube is in there too)
///
/// Parameters:
///		None
///=============================================================================
void Engine::CreateObjects()
{
	int qindex[] = { 0,1,3,0,3,2 };
	vertex_t quad[4];

	memset(&quad, 0, 4 * sizeof(vertex_t));
	quad[0].position = vec3(-1.0f, -1.0f, 0.0f);
	quad[0].texCoord0 = vec2(0.0f, 0.0f);
	quad[0].color = ~0;
	quad[1].position = vec3(-1.0f, 1.0f, 0.0f);
	quad[1].texCoord0 = vec2(0.0f, 1.0f);
	quad[1].color = ~0;
	quad[2].position = vec3(1.0f, -1.0f, 0.0f);
	quad[2].texCoord0 = vec2(1.0f, 0.0f);
	quad[2].color = ~0;
	quad[3].position = vec3(1.0f, 1.0f, 0.0f);
	quad[3].texCoord0 = vec2(1.0f, 1.0f);
	quad[3].color = ~0;

	quad_index = gfx.CreateIndexBuffer(qindex, 6);
	quad_vertex = gfx.CreateVertexBuffer(quad, 4);

	q3map.quad_index = quad_index;
	q3map.quad_vertex = quad_vertex;
	menu.quad_index = quad_index;
	menu.quad_vertex = quad_vertex;


	gen_spiral(gfx, spiral_ibo, spiral_vbo);
	gen_lightning(gfx, lightning_ibo, lightning_vbo);
	gen_unit_box(gfx, EntModel::unit_index, EntModel::unit_vertex);


	/*
	int cube_idx[36];
	vertex_t cube[36];
	vec2 tex[4];

	tex[0] = vec2(0.0f, 0.0f);
	tex[1] = vec2(0.0f, 1.0f);
	tex[2] = vec2(1.0f, 0.0f);
	tex[3] = vec2(1.0f, 1.0f);

	memset(&cube, 0, 36 * sizeof(vertex_t));
	cube[0].position = vec3(-0.5f, -0.5f, -0.5f); //1
	cube[0].texCoord0 = tex[0];
	cube[1].position = vec3(-0.5f, -0.5f, 0.5f);  //2
	cube[1].texCoord0 = tex[1];
	cube[2].position = vec3(-0.5f, 0.5f, -0.5f);  //3
	cube[2].texCoord0 = tex[2];

	cube[3].position = vec3(-0.5f, -0.5f, 0.5f);  //2
	cube[3].texCoord0 = tex[1];
	cube[4].position = vec3(-0.5f, 0.5f, 0.5f);   //4
	cube[4].texCoord0 = tex[3];
	cube[5].position = vec3(-0.5f, 0.5f, -0.5f);  //3
	cube[5].texCoord0 = tex[2];

	cube[6].position = vec3(0.5f, 0.5f, 0.5f);    //8
	cube[6].texCoord0 = tex[3];
	cube[7].position = vec3(-0.5f, -0.5f, 0.5f);  //2
	cube[7].texCoord0 = tex[1];
	cube[8].position = vec3(0.5f, -0.5f, 0.5f);   //6
	cube[8].texCoord0 = tex[1];

	cube[9].position = vec3(0.5f, 0.5f, 0.5f);    //8
	cube[9].texCoord0 = tex[3];
	cube[10].position = vec3(-0.5f, 0.5f, 0.5f);   //4
	cube[10].texCoord0 = tex[3];
	cube[11].position = vec3(-0.5f, -0.5f, 0.5f);  //2
	cube[11].texCoord0 = tex[1];

	cube[12].position = vec3(0.5f, -0.5f, 0.5f);   //6
	cube[12].texCoord0 = tex[1];
	cube[13].position = vec3(0.5f, 0.5f, -0.5f);   //7
	cube[13].texCoord0 = tex[2];
	cube[14].position = vec3(0.5f, 0.5f, 0.5f);    //8
	cube[14].texCoord0 = tex[3];


	cube[15].position = vec3(0.5f, -0.5f, 0.5f);   //6
	cube[15].texCoord0 = tex[1];
	cube[16].position = vec3(0.5f, -0.5f, -0.5f);  //5
	cube[16].texCoord0 = tex[0];
	cube[17].position = vec3(0.5f, 0.5f, -0.5f);   //7
	cube[17].texCoord0 = tex[2];

	cube[18].position = vec3(-0.5f, -0.5f, -0.5f); //1
	cube[18].texCoord0 = tex[0];
	cube[19].position = vec3(-0.5f, 0.5f, -0.5f);  //3
	cube[19].texCoord0 = tex[2];
	cube[20].position = vec3(0.5f, 0.5f, -0.5f);   //7
	cube[20].texCoord0 = tex[2];


	cube[21].position = vec3(-0.5f, -0.5f, -0.5f); //1
	cube[21].texCoord0 = tex[0];
	cube[22].position = vec3(0.5f, 0.5f, -0.5f);   //7
	cube[22].texCoord0 = tex[2];
	cube[23].position = vec3(0.5f, -0.5f, -0.5f);  //5
	cube[23].texCoord0 = tex[0];

	cube[24].position = vec3(-0.5f, -0.5f, -0.5f); //1
	cube[24].texCoord0 = tex[0];
	cube[25].position = vec3(0.5f, -0.5f, -0.5f);  //5
	cube[25].texCoord0 = tex[0];
	cube[26].position = vec3(-0.5f, -0.5f, 0.5f);  //2
	cube[26].texCoord0 = tex[1];

	cube[27].position = vec3(-0.5f, -0.5f, 0.5f);  //2
	cube[27].texCoord0 = tex[1];
	cube[28].position = vec3(0.5f, -0.5f, -0.5f);  //5
	cube[28].texCoord0 = tex[0];
	cube[29].position = vec3(0.5f, -0.5f, 0.5f);   //6
	cube[29].texCoord0 = tex[1];

	cube[30].position = vec3(-0.5f, 0.5f, -0.5f);  //3
	cube[30].texCoord0 = tex[2];
	cube[31].position = vec3(-0.5f, 0.5f, 0.5f);   //4
	cube[31].texCoord0 = tex[3];
	cube[32].position = vec3(0.5f, 0.5f, -0.5f);   //7
	cube[32].texCoord0 = tex[2];

	cube[33].position = vec3(-0.5f, 0.5f, 0.5f);   //4
	cube[33].texCoord0 = tex[3];
	cube[34].position = vec3(0.5f, 0.5f, 0.5f);    //8
	cube[34].texCoord0 = tex[3];
	cube[35].position = vec3(0.5f, 0.5f, -0.5f);   //7
	cube[35].texCoord0 = tex[2];


	for (int i = 0; i < 36; i++)
	{
	cube_idx[i] = i;
	cube[i].color = ~0;
	cube[i].position *= 500.0f;
	}


	Model::cube_index = gfx.CreateIndexBuffer(cube_idx, 36);
	Model::cube_vertex = gfx.CreateVertexBuffer(cube, 36);
	*/
}

///=============================================================================
/// Function: render_texture
///=============================================================================
/// Description: This renders a texture to the full screen quad, can be used for
/// viewing depth textures also
///
/// Parameters:
///		None
///=============================================================================
void Engine::render_texture(int texObj, bool depth_view)
{
	gfx.SelectTexture(0, texObj);
	gfx.SelectIndexBuffer(quad_index);
	gfx.SelectVertexBuffer(quad_vertex);
	global.Select();
	global.Params(identity, depth_view);
	gfx.DrawArrayTri(0, 0, 6, 4);
}


///=============================================================================
/// Function: render_scene
///=============================================================================
/// Description: This is used to render the actual map and entities, since things
/// like shadow volumes need the scene rendered multiple times, (once lit, once with
/// darkness, each time with different stencil pass tests) a function was needed
/// to allow for that
///
/// Parameters:
///		None
///=============================================================================
void Engine::render_scene(bool lights)
{
	matrix4 transformation = identity;
	matrix4 mvp;
	vec3 offset(0.0f, 0.0f, 0.0f);
	float temp = 0.0f;

	int player = find_type(ENT_PLAYER, 0);
	if (player != -1)
	{
		entity_list[player]->rigid->frame2ent(&camera_frame, input);
	}
	camera_frame.set(transformation);

	// Rendering entities before map for blends
	render_entities(transformation, projection, lights, false);
	render_players(transformation, projection, lights, game->get_spectator());


	if (lights == false)
	{
		temp = mlight2.m_brightness;
		mlight2.set_exposure(0.5);
	}
	else
	{
		mlight2.set_exposure(1.0);
	}

	mlight2.Select();
	mvp = transformation * projection;

	if (enable_map)
	{
		if (q3map.enable_textures == false)
		{
			gfx.SelectTexture(0, no_tex);
			gfx.SelectTexture(1, no_tex);
			gfx.SelectTexture(2, no_tex);
			gfx.SelectTexture(3, no_tex);
		}

		if (lights)
			mlight2.Params(mvp, light_list, light_list.size(), offset, tick_num);
		else
			mlight2.Params(mvp, light_list, 0, offset, tick_num);


		if (debug_tri_vert != -1)
		{
			gfx.SelectIndexBuffer(debug_tri_index);
			gfx.SelectVertexBuffer(debug_tri_vert);
			gfx.SelectTexture(0, no_tex);
			gfx.DrawArrayTri(0, 0, debug_tri_list.size(), debug_tri_list_index.size());
		}

		if (debug_vec_vert != -1)
		{
			gfx.SelectIndexBuffer(debug_vec_index);
			gfx.SelectVertexBuffer(debug_vec_vert);
			gfx.SelectTexture(0, no_tex);
			gfx.DrawArrayLine(0, 0, debug_vec_list.size(), debug_vec_list_index.size());
		}


		raster_tcount = 0;

		q3map.render(camera_frame.pos, gfx, surface_list, mlight2, tick_num, pfrustum);

		raster_tcount = 0;

		//draw_plane(gfx, q3map.data.Plane[q3map.data.Node[0].plane], camera_frame.forward, camera_frame.pos);
	}

	if (enable_terrain)
	{
		gfx.SelectTexture(1, 0);
		gfx.SelectTexture(2, 0);
		gfx.SelectTexture(3, 0);
		terrain.render(gfx);
	}

	if (lights == false)
	{
		mlight2.set_brightness(temp);
	}


#ifdef PARTICLES
	gfx.Blend(true);
	gfx.BlendFuncSrcAlphaOneMinusSrcAlpha();

#ifdef OPENGL
	int vbo = 0;

	if (emitter.visible)
	{
		particle_update.Select();
		emitter.seed = vec3(rand_float(0.0, 10.0),
			rand_float(0.0, 10.0),
			rand_float(0.0, 10.0));
		particle_update.Params(emitter);
		vbo = particle_update.step(gfx, emitter);
	}

	if (emitter.enabled == false)
	{
		if (emitter_count > 0)
			emitter_count--;
		else
			emitter.visible = false;

		emitter.position = vec3(0.0f, -10000.0f, 0.0f);
	}
#endif

#ifdef OPENGL
	if (emitter.visible)
	{
		vec3 quad1 = camera_frame.up;
		vec3 quad2 = vec3::crossproduct(camera_frame.up, camera_frame.forward);

		particle_render.Select();
		particle_render.Params(mvp, quad1, quad2, 0.0f, 0.0f, 0.0f);
		gfx.SelectTexture(0, particle_tex);
		particle_render.render(gfx, 0, vbo, emitter.num);
	}


	render_trails(transformation);
#endif
#endif

	render_entities(transformation, projection, lights, true);
	mlight2.set_lightmap(0.5f);
	render_weapon(transformation, lights, player);
	mlight2.set_lightmap(1.0f);
}



///=============================================================================
/// Function: render_scene_using_shadowmap
///=============================================================================
/// Description: This function renders a scene using the depthmaps generated from
/// each light for shadow map rendering. Since I'm doing a cube map for each light,
/// you quickly run into limitations regarding the max number of textures you can
/// pass into the shader ideally you would pass the cube map for each active light
/// and only update those cube maps as needed (ie: something moved in it's view)
///
/// A long time ago you use to be only able to pass to the GPU a very small number
/// of uniforms/constants and per-vertex parameters, (Thinking FX5900 days) so wait
/// long enough and GPU limits are removed in time
///
/// Old comments (probably no longer relevent)
///		Texture arrays let me pass all the depthmaps
///		Next problem is the transformed interpolated fragment per face, per light
///		Right now I can only get four shadow omni-lights with this limit
///
/// Parameters:
///		None
///=============================================================================
void Engine::render_scene_using_shadowmap(bool lights)
{
	matrix4 transformation;
	matrix4 mvp;
	vec3 offset(0.0f, 0.0f, 0.0f);

	int player = find_type(ENT_PLAYER, 0);
	if (player != -1)
		entity_list[player]->rigid->frame2ent(&camera_frame, input);

	camera_frame.set(transformation);


	mlight2.Select();

	unsigned int smatrix = 0;

	for (unsigned int i = max_dynamic; i < entity_list.size(); i++)
	{
		EntLight *light = entity_list[i]->light;
		if (light == NULL)
			continue;

		if (entity_list[i]->flags.bsp_visible == false)
			continue;

		if (i == 100)
			continue;

		if (light->shadow_flag & 1)
		{
			//printf("Using light %d face %d in slot %d\n", i, 0, smatrix);
			mlight2.set_shadow_matrix(smatrix, light->shadow_matrix[0]);
			gfx.SelectTexture(smatrix + 10, light->depth_tex[0]);
			smatrix++;
		}
		if (smatrix == MAX_DEPTH)
			break;

		if (light->shadow_flag & 2)
		{
			//printf("Using light %d face %d in slot %d\n", i, 1, smatrix);
			mlight2.set_shadow_matrix(smatrix, light->shadow_matrix[1]);
			gfx.SelectTexture(smatrix + 10, light->depth_tex[1]);
			smatrix++;
		}
		if (smatrix == MAX_DEPTH)
			break;

		if (light->shadow_flag & 4)
		{
			//printf("Using light %d face %d in slot %d\n", i, 2, smatrix);
			mlight2.set_shadow_matrix(smatrix, light->shadow_matrix[2]);
			gfx.SelectTexture(smatrix + 10, light->depth_tex[2]);
			smatrix++;
		}
		if (smatrix == MAX_DEPTH)
			break;

		if (light->shadow_flag & 8)
		{
			//printf("Using light %d face %d in slot %d\n", i, 3, smatrix);
			mlight2.set_shadow_matrix(smatrix, light->shadow_matrix[3]);
			gfx.SelectTexture(smatrix + 10, light->depth_tex[3]);
			smatrix++;
		}
		if (smatrix == MAX_DEPTH)
			break;

		if (light->shadow_flag & 16)
		{
			//printf("Using light %d face %d in slot %d\n", i, 4, smatrix);
			mlight2.set_shadow_matrix(smatrix, light->shadow_matrix[4]);
			gfx.SelectTexture(smatrix + 10, light->depth_tex[4]);
			smatrix++;
		}
		if (smatrix == MAX_DEPTH)
			break;

		if (light->shadow_flag & 32)
		{
			//printf("Using light %d face %d in slot %d\n", i, 5, smatrix);
			mlight2.set_shadow_matrix(smatrix, light->shadow_matrix[5]);
			gfx.SelectTexture(smatrix + 10, light->depth_tex[5]);
			smatrix++;
		}
		if (smatrix == MAX_DEPTH)
			break;

	}

	// Rendering entities before map for blends
	render_entities(transformation, projection, lights, false);
	render_players(transformation, projection, lights, game->get_spectator());
	mvp = transformation * projection;
	if (lights)
		mlight2.Params(mvp, light_list, light_list.size(), offset, tick_num);
	else
		mlight2.Params(mvp, light_list, 0, offset, tick_num);

	if (enable_map)
		q3map.render(camera_frame.pos, gfx, surface_list, mlight2, tick_num, pfrustum);

#ifdef PARTICLES
	gfx.Blend(true);
	gfx.BlendFuncSrcAlphaOneMinusSrcAlpha();

#ifdef OPENGL
	int vbo = 0;

	if (emitter.visible)
	{
		particle_update.Select();
		emitter.seed = vec3(rand_float(0.0, 10.0),
			rand_float(0.0, 10.0),
			rand_float(0.0, 10.0));
		particle_update.Params(emitter);
		vbo = particle_update.step(gfx, emitter);
	}

	if (emitter.enabled == false)
	{
		if (emitter_count > 0)
			emitter_count--;
		else
			emitter.visible = false;

		emitter.position = vec3(0.0f, -10000.0f, 0.0f);
	}
#endif

//	camera_frame.set(transformation);
//	mvp = transformation * projection;


#ifdef OPENGL
	if (emitter.visible)
	{
		vec3 quad1 = camera_frame.up;
		vec3 quad2 = vec3::crossproduct(camera_frame.up, camera_frame.forward);

		particle_render.Select();
		particle_render.Params(mvp, quad1, quad2, 0.0f, 0.0f, 0.0f);
		gfx.SelectTexture(0, particle_tex);
		particle_render.render(gfx, 0, vbo, emitter.num);
	}


	render_trails(transformation);
#endif
#endif

	render_entities(transformation, projection, lights, true);
	render_weapon(transformation, lights, player);

}


///=============================================================================
/// Function: render_weapon
///=============================================================================
/// Description: This renders the weapon in first person (if one)
/// offsets the weapon matrix a little to give it a off center rotation effect
/// (should add weapon bobbing instead of doing that eventually though)
///
/// Parameters:
///		None
///=============================================================================
void Engine::render_weapon(const matrix4 &trans, bool lights, int i)
{
	matrix4 mvp;

	if (i == -1)
		return;

	vec3 offset = entity_list[i]->position;

	if (entity_list[i]->player == NULL)
		return;

	mlight2.Select();
	entity_list[i]->rigid->get_matrix(mvp.m);

	//set weapon coordinates
	mvp.m[12] += mvp.m[0] * -5.0f + mvp.m[4] * 50.0f + mvp.m[8] * 5.0f;
	mvp.m[13] += mvp.m[1] * -5.0f + mvp.m[5] * 50.0f + mvp.m[9] * 5.0f;
	mvp.m[14] += mvp.m[2] * -5.0f + mvp.m[6] * 50.0f + mvp.m[10] * 7.0f;

	mvp = (mvp * trans) * projection;
	if (lights)
	{
		mlight2.Params(mvp, light_list, light_list.size(), offset, tick_num);
	}
	else
	{
		mlight2.Params(mvp, light_list, 0, offset, tick_num);
	}

	entity_list[i]->player->render_weapon(gfx);

	entity_list[i]->rigid->get_matrix(mvp.m);
	mvp = (mvp * trans) * projection;

	game->draw_flash((entity_list[i]->player));

}

///=============================================================================
/// Function: render_trails
///=============================================================================
/// Description: This is used to render effects like the railgun trail and the
/// lightning gun beam. These are particles rendered similarly to the smoke trails
/// from rockets / grenades, just without any movement
///
/// Parameters:
///		None
///=============================================================================
void Engine::render_trails(matrix4 &trans)
{
	matrix4 mvp;
	bool once = false;
	vec3 quad1 = camera_frame.up;
	vec3 quad2 = vec3::crossproduct(camera_frame.up, camera_frame.forward);


	//render particle trails first
	for (unsigned int i = max_player; i < max_dynamic; i++)
	{
		EntModel *model = entity_list[i]->model;

		if (model == NULL)
			continue;



		if (once == false && (model->flags.rail_trail || model->flags.lightning_trail))
		{
			//			vec3 offset = entity_list[i]->position;

			// Undo model orientation
			quad1 = model->morientation * quad1;
			quad2 = model->morientation * quad2;

			particle_render.Select();
			gfx.SelectTexture(0, particle_tex);
			once = true;
		}


		//render rail trail
		if (entity_list[i]->model->flags.rail_trail)
		{
			entity_list[i]->rigid->get_matrix(mvp.m);
			mvp = (mvp * trans) * projection;
			particle_render.Params(mvp, quad1, quad2, 0.0f, 0.0f, 0.0f);

			particle_render.render(gfx, 0, spiral_vbo, 400);
			continue;
		}

		//render lightning trail
		if (entity_list[i]->model->flags.lightning_trail)
		{
			entity_list[i]->rigid->get_matrix(mvp.m);
			mvp = (mvp * trans) * projection;
			particle_render.Params(mvp, quad1, quad2, 0.0f, 0.0f, 0.0f);

			particle_render.render(gfx, 0, lightning_vbo, 400);
			continue;
		}
	}
}

///=============================================================================
/// Function: render_entities
///=============================================================================
/// Description: This is used to render any entities (essentially any object that
/// isn't the map itself and can move) eg: weapons, armor, players, health, etc)s
///
/// Parameters:
///		None
///=============================================================================
void Engine::render_entities(const matrix4 &trans, matrix4 &proj, bool lights, bool blend, bool vis)
{
	matrix4 mvp;

	if (enable_entities == false)
		return;

	emitter.enabled = false;

	for (int i = 1; i < MAX_TEXTURES; i++)
	{
		gfx.SelectTexture(i, 0);
	}

	gfx.Blend(false);
	mlight2.set_lightmap(0.5f);
	mlight2.Select();

	for (unsigned int i = 0; i < entity_list.size(); i++)
	{
		Entity *entity = entity_list[i];

		if (entity->model == NULL)
			continue;

		if (entity->model->flags.blend != blend)
			continue;

		if (entity->ent_type == ENT_FUNC_DOOR ||
			entity->ent_type == ENT_FUNC_BOBBING ||
			entity->ent_type == ENT_PATH_CORNER ||
			entity->ent_type == ENT_FUNC_TERRAIN ||
			(entity->ent_type == ENT_WEAPON_LIGHTNING && enable_planet))
		{
			entity->flags.visible = true;
			entity->flags.bsp_visible = true;
			//printf("break\n");
		}

		if (vis)
		{
			if (entity->flags.visible == false)
				continue;
		}
		else
		{
			if (entity->flags.bsp_visible == false || (entity->trigger && entity->trigger->active))
				continue;
		}

		if (entity->flags.nodraw == true)
			continue;

		if (entity->model != NULL)
		{
			if (entity->model->flags.rail_trail || entity->model->flags.lightning_trail)
				continue;
		}


		if (entity->player)
		{
			continue;
		}


		if (i < max_player && entity->player && entity->player->type == PLAYER)
		{
			continue;
		}

		if (i < max_player && entity->player && entity->player->type == SERVER)
		{
			continue;
		}

		if (i < max_player && entity->player && entity->player->type == CLIENT)
		{
			continue;
		}

		if (i < max_player  && entity->player && entity->player->type == BOT)
		{
			continue;
		}

		if (i < max_player  && entity->player && entity->player->type == SPECTATOR)
		{
			continue;
		}

		vec3 offset = entity->position;

		entity->model->get_matrix(mvp.m);
		mvp = (mvp * trans) * proj;
		if (lights)
		{
			mlight2.Params(mvp, light_list, light_list.size(), offset, tick_num);
		}
		else
		{
			mlight2.Params(mvp, light_list, 0, offset, tick_num);
		}

		if (entity->model->flags.cull_none)
		{
			gfx.CullFace(2);
		}

#ifndef D3D11
		if (entity->ent_type == ENT_FUNC_CLOTH)
		{
			int index = entity->brushinfo->opening;
			gfx.Blend(true);
			gfx.SelectIndexBuffer(cloth[index]->ibo);
			gfx.SelectVertexBuffer(cloth[index]->vbo);
			gfx.SelectTexture(0, cloth[index]->tex);
			gfx.DrawArrayTri(0, 0, cloth[index]->num_index, cloth[index]->num_vert);
			gfx.Blend(false);
		}
#endif

		//render entity
		if (entity->ent_type == ENT_WEAPON_LIGHTNING && enable_planet)
		{
//			int player = find_type(ENT_PLAYER, 0);
//			int current_light = 0;
//			if (player != -1)
//			{
//				current_light = entity_list[player]->player->current_light;
//			}

			//int lod = clamp(current_light, 0, 10);

			//isocube[0].render(gfx);
			//isosphere[0].render(gfx);
		}
		else
		{
			if ((entity->ent_type == ENT_SENTRY_BASE || entity->ent_type == ENT_SENTRY || entity->ent_type == ENT_CONSTRUCT) && entity->construct)
			{
				if (entity->ent_type == ENT_SENTRY)
				{
					mlight2.set_dissolve(((float)entity->construct->build_timer / (3 * TICK_RATE)));
					gfx.SelectTexture(7, -dissolve_tex);
					entity->rigid->render(gfx);
					entity_list[entity->construct->base_index]->rigid->render(gfx);
					gfx.SelectTexture(7, 0);
					mlight2.set_dissolve(-1.0f);
				}
				else
				{
					mlight2.set_dissolve(((float)entity->construct->build_timer / (3 * TICK_RATE)));
					gfx.SelectTexture(7, -dissolve_tex);
					entity->rigid->render(gfx);
					gfx.SelectTexture(7, 0);
					mlight2.set_dissolve(-1.0f);
				}
			}
			else
			{
				// draw bounding box around entities too
				if (show_box)
				{
					if (picked_ent == -1 || (unsigned int)picked_ent == i)
					{
						entity->rigid->render_box(gfx);
					}
				}

				entity->rigid->render(gfx);
			}
		}



		if (entity->model->flags.cull_none)
		{
			gfx.CullFace(3);
		}



		if (entity->brush_ref > 0 && (unsigned int)entity->brush_ref < q3map.data.num_model && q3map.enable_textures && tick_num > TICK_RATE * 0.5)
		{
			Frame frame;

			vec3 old = entity->position;
			if (entity->brushinfo)
			{
				entity->position = entity->brushinfo->model_offset + (entity->position - entity->brushinfo->origin);
			}
			entity->rigid->get_matrix(mvp.m);
			mvp = (mvp * trans) * proj;
			mlight2.set_matrix(mvp);
			if (enable_map)
			{
				q3map.render_brush_entity(entity->brush_ref, gfx);
			}

			entity->position = old;
		}

		//  update emitter position if this entity has particles
		if (entity->flags.particle_on)
		{
			emitter.enabled = true;
			emitter.num = entity->num_particle;
			emitter.position = entity->position;
			emitter.gravity = vec3(0.0f, 30.0f, 0.0f);
			emitter_count = 5 * TICK_RATE;
		}
	}
	mlight2.set_lightmap(1.0f);
}

///=============================================================================
/// Function: render_players
///=============================================================================
/// Description: This renders players, which are special in that they have a md5 model
/// that is associated with them (most other models are imported wavefront obj files,
/// which do not move/animate) The file format is essentially just index buffers and
/// vertex buffers for those
///
/// Parameters:
///		None
///=============================================================================
void Engine::render_players(matrix4 &trans, matrix4 &proj, bool lights, bool render_self)
{
	matrix4 mvp;
	//render player md5
	mlight2.set_lightmap(1.0f);
	for (unsigned int i = 0; /*i < max_player &&*/ i < entity_list.size(); i++)
	{
		Entity *entity = entity_list[i];

		if (entity == NULL)
			continue;

		if (entity->player == NULL || entity->rigid == NULL)
			continue; 

		if (entity->flags.visible == false && entity->player && entity->player->type != SPECTATOR)
			continue;

		if (	(entity->player->type == BOT) ||
			(entity->player->type == SPECTATOR) ||
			(entity->player->type == SERVER) ||
			(entity->player->type == PLAYER) ||
			(entity->player->type == CLIENT && netcode.server_flag))
		{
			if (entity->player->local && render_self == false)
				continue;

			if (entity->player->health > -50)
			{
				int oldval = entity->rigid->y_offset;
				if (netcode.client_flag)
				{
					entity->rigid->y_offset = 115;
				}

				entity->rigid->y_offset = (int)(24 * 1.4f);


				//md5 faces right, need to flip right and forward orientation
				// also forcing to yaw only (no up / down rotation)
				entity->rigid->get_matrix(mvp.m);

				if (netcode.client_flag)
				{
					entity->rigid->y_offset = oldval;
				}


				vec3 forward(mvp.m[8], mvp.m[9], mvp.m[10]);
				vec3 up(0.0f, 1.0f, 0.0f);

				forward.y = 0.0f;
				forward.normalize();
				vec3 right = vec3::crossproduct(forward, up);
				right.normalize();
				up = vec3::crossproduct(right, forward);
				up.normalize();

				right = vec3::crossproduct(forward, up);
				right.normalize();

				mvp.m[0] = forward.x * 1.4f;
				mvp.m[1] = forward.y;
				mvp.m[2] = forward.z;

				mvp.m[4] = up.x;
				mvp.m[5] = up.y * 1.4f;
				mvp.m[6] = up.z;

				mvp.m[8]  = right.x;
				mvp.m[9]  = right.y;
				mvp.m[10] = right.z * 1.4f;


				mvp = (mvp * trans) * proj;
				vec3 offset = entity->position;


				if (lights)
				{
					mlight2.Params(mvp, light_list, light_list.size(), offset, tick_num);
				}
				else
				{
					mlight2.Params(mvp, light_list, 0, offset, tick_num);
				}


				md5_model[RANGER].render(gfx, tick_num, &entity->player->ani_state);

				// for debugging md5 animations, walks animation back some so walking forward doesnt move him
				if (entity->player->ani_state.done)
				{
					if (entity->player->done_transform)
					{
						entity->position += entity->player->done_pos;
						entity->player->done_transform = false;
					}
				}
			}
		}
	}
}


///=============================================================================
/// Function: render_shadow_volumes
///=============================================================================
/// Description: This renders shadow volumes for the stencil buffer, currently
/// if enabled it will only make shadow volumes for entities and not the map
/// (some crazy coordinate transform issue or something makes the shadows from
/// the map itself look like garbage, not to mention a rather large performance
/// impact)
///
/// The shadow volumes do some crazy stencil buffer addition and subtraction
/// on front faces and back faces to leave an even or odd result, which will
/// indicate whether a surface is in shadow or not. But these depend on 3d geometry
/// for the shadow volume that is "sealed", which are generated by extruding geometry
/// from the lights perspective on the CPU
///
/// Parameters:
///		None
///=============================================================================
void Engine::render_shadow_volumes(bool debug)
{
#ifdef SHADOWVOL

	matrix4 transformation;
	matrix4 matrix;



	int player = find_type(ENT_PLAYER, 0);
	if (player != -1)
	{
		entity_list[player]->rigid->frame2ent(&camera_frame, input);
		camera_frame.set(transformation);
		camera_frame.pos = entity_list[player]->position;
	}

	matrix4 mvp = transformation * projection;



	global.Select();
	global.Params(mvp);
	gfx.SelectTexture(0, no_tex);



#if 0
	for (unsigned int i = max_dynamic; i < entity_list.size(); i++)
	{
		if (entity_list[i]->light)
			entity_list[i]->light->render_map_shadowvol(gfx);
	}
#endif


	// map shadow volumes from brushes
	bool update_shadow_volumes = false;


	if (enable_map_brush_shadow)
	{
		for (unsigned int i = 0; i < radmap.radent[0].num_brush; i++)
		{
			//		if (i == debug_brush)
			{
				vec3 vLight(0.01f, 10000.0f, 0.01f);

				if (update_shadow_volumes/* && keyboard.control == false*/)
				{
					shadow[i].CreateVolume(gfx,
						radmap.quadent.quadbrush[i].vert_array,
						radmap.quadent.quadbrush[i].index_array,
						radmap.quadent.quadbrush[i].num_index,
						vLight);
				}

				shadow[i].render(gfx);

				//			break;
			}
		}
	}


	
//	q3map.RenderShadowVolumes(gfx, camera_frame.pos, current_light);

	if (enable_entities == false)
		return;

	for (unsigned int i = max_dynamic; i < entity_list.size(); i++)
	{

		if (entity_list[i]->light)
		{
//			if (entity_list[i]->light->light_num == player->current_light)
			{
				if (input.scores)
				{
					shadow_light = i;
//					printf("selecting light %d for shadows\n", i);
				}

				if (entity_list[i]->rigid)
				{

					for (int j = 0; j < entity_list[i]->light->num_shadowvol; j++)
					{
						Entity *ent = entity_list[i];
						vec3 old_pos = ent->position;
						matrix3 old_matrix = ent->model->morientation;

//						ent->position = ent->light->shadow[j].position;
//						ent->model->morientation = ent->light->shadow[j].morientation;
						ent->model->get_matrix(matrix.m);
						ent->position = old_pos;
						ent->model->morientation = old_matrix;

						matrix4 mvp = transformation.premultiply(matrix.m) * projection;
						global.Params(mvp);
//						ent->light->render_shadow_volume(gfx, j);
					}
				}
			}
		}
	}
#endif
}

///=============================================================================
/// Function: screenshot
///=============================================================================
/// Description: This is used to take a screenshot and saves the image to a file
///
/// Parameters:
///		None
///=============================================================================
void Engine::screenshot(unsigned int &luminance, bool luminance_only)
{
	static int num = 0;
	char filename[128];
#ifdef WIN32
	GetScreenShot(*((HWND *)param1), luminance, luminance_only);
	if (luminance_only == false)
	{
		sprintf(filename, "Screenshot%d.bmp", num++);
		menu.print(filename);
	}
	return;
#else
	int width = fb_width;
	int height = fb_height;
	char *pixel = new char[4 * width * height];
	char *bmp = new char[4 * width * height];

//	gfx.bindFramebuffer(0);
//	gfx.resize(width, height);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, render_fbo);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
//	gfx.SelectTexture(0, quad_tex);
	//glGetTexImage(GL_TEXTURE_2D, quad_tex, GL_UNSIGNED_BYTE, GL_RGBA, pixel);
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
	for (int i = 0, j = 0; i < 4 * width * height; i += 4)
	{
		//bmp[j++] = pixel[i + 3];
		bmp[j++] = pixel[i + 2];
		bmp[j++] = pixel[i + 1];
		bmp[j++] = pixel[i];
		bmp[j++] = 0;
	}
	sprintf(filename, "Screenshot%d.bmp", num++);
	write_bitmap(filename, width, height, (int *)bmp);
	menu.print(filename);
	delete[] pixel;
	delete[] bmp;
#endif
}

///=============================================================================
/// Function: post_process
///=============================================================================
/// Description: This is used to perform post processing on the final rendered image
/// before sending it to the monitor, can perform some interesting effects pretty
/// easily
///
/// Parameters:
///		None
///=============================================================================
void Engine::post_process(int num_passes, int type)
{
	int temp;

	gfx.SelectTexture(0, post.image);
	for (int pass = 0; pass < num_passes; pass++)
	{
#ifdef OPENGL
	#ifdef FORWARD
		glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 0, 0, gfx.width, gfx.height, 0);
	#else
		glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 0, 0, fb_width, fb_height, 0);
	#endif
#endif
		gfx.SelectTexture(1, post.swap);
		post.Select();
		post.Params(type, tick_num);
		post.BloomParams(0, 20, bloom_strength, bloom_threshold, fb_width, fb_height);
		gfx.clear();
		gfx.SelectIndexBuffer(quad_index);
		gfx.SelectVertexBuffer(quad_vertex);
		gfx.DrawArrayTri(0, 0, 6, 4);
//		gfx.SelectShader(0);
//		gfx.DeselectTexture(1);
	}
	temp = post.swap;
	post.swap = post.image;
	post.image = temp;
//	gfx.DeselectTexture(0);
}

///=============================================================================
/// Function: render_bloom
///=============================================================================
/// Description: Bloom is a lighting effect where bright objects bleed into
/// surrounding areas, which is done through blurring the "bright" part of an
/// image then blending that blurred image with the main scene
///
/// Parameters:
///		None
///=============================================================================
void Engine::render_bloom(bool debug)
{
	gfx.bindFramebuffer(mask_fbo);
	gfx.resize(fb_width, fb_height);
	gfx.fbAttachTexture(mask_quad);
	gfx.fbAttachDepth(mask_depth);
	gfx.clear();
	gfx.SelectTexture(0, render_quad);
	gfx.SelectTexture(1, render_depth);
	post.Select();
	post.Params(POST_MASK, tick_num);
	post.BloomParams(0, bloom_amount, bloom_strength, bloom_threshold, fb_width, fb_height);
	//	post.Params(POST_DOF, tick_num);
//	post.BloomParams(0, dof_near, dof_far, bloom_threshold);
	gfx.clear();
	gfx.SelectIndexBuffer(quad_index);
	gfx.SelectVertexBuffer(quad_vertex);
	gfx.DrawArrayTri(0, 0, 6, 4); // bright pass filter
	gfx.bindFramebuffer(0);

	gfx.bindFramebuffer(blur1_fbo);
	gfx.resize(fb_width, fb_height);
	gfx.fbAttachTexture(blur1_quad);
	gfx.fbAttachDepth(blur1_depth);
	gfx.clear();
	gfx.SelectTexture(0, mask_quad);
	post.Select();
	post.Params(POST_BLOOM, tick_num);
	post.BloomParams(0, bloom_amount, bloom_strength, bloom_threshold, fb_width, fb_height);
	gfx.clear();
	gfx.SelectIndexBuffer(quad_index);
	gfx.SelectVertexBuffer(quad_vertex);
	gfx.DrawArrayTri(0, 0, 6, 4); // first blur pass
	gfx.bindFramebuffer(0);

	// Reselect original frame buffer texture
	gfx.bindFramebuffer(blur2_fbo);
	gfx.resize(fb_width, fb_height);
	gfx.fbAttachTexture(blur2_quad);
	gfx.fbAttachDepth(blur2_depth);
	gfx.clear();
	gfx.SelectIndexBuffer(quad_index);
	gfx.SelectVertexBuffer(quad_vertex);
	gfx.SelectTexture(0, blur1_quad);
	post.Select();
	post.Params(POST_BLOOM, tick_num);
	post.BloomParams(1, bloom_amount, bloom_strength, bloom_threshold, fb_width, fb_height);
	gfx.DrawArrayTri(0, 0, 6, 4); // second pass

	gfx.bindFramebuffer(render_fbo, 2);
	gfx.resize(fb_width, fb_height);
//	gfx.clear();
	gfx.SelectIndexBuffer(quad_index);
	gfx.SelectVertexBuffer(quad_vertex);
	if (debug)
	{
		gfx.SelectTexture(0, mask_quad);
		gfx.SelectTexture(1, mask_quad);
		gfx.SelectTexture(2, mask_quad);
	}
	else
	{
		gfx.SelectTexture(0, render_quad);
		gfx.SelectTexture(1, blur2_quad);
		gfx.SelectTexture(2, blur2_quad);
	}
	post.Select();
	post.Params(POST_COMBINE, tick_num);
	gfx.DrawArrayTri(0, 0, 6, 4); // add all three together

}

///=============================================================================
/// Function: render_bloom
///=============================================================================
/// Description: This is a cheap effect to emulate crepuscular rays with a radial
/// blur, doesn't look very great, but easy to add
///
/// Parameters:
///		None
///=============================================================================
void Engine::render_skyray(bool debug)
{
	gfx.bindFramebuffer(mask_fbo);
	gfx.resize(fb_width, fb_height);
	gfx.fbAttachTexture(mask_quad);
	gfx.fbAttachDepth(mask_depth);
	gfx.clear();
	gfx.SelectTexture(0, render_quad);
	gfx.SelectTexture(1, render_depth);
	post.Select();
	post.Params(POST_MASK, tick_num);
	post.BloomParams(0, bloom_amount, bloom_strength, bloom_threshold, fb_width, fb_height);
	//	post.Params(POST_DOF, tick_num);
	//	post.BloomParams(0, dof_near, dof_far, bloom_threshold);
	gfx.clear();
	gfx.SelectIndexBuffer(quad_index);
	gfx.SelectVertexBuffer(quad_vertex);
	gfx.DrawArrayTri(0, 0, 6, 4); // bright pass filter
	gfx.bindFramebuffer(0);

	gfx.bindFramebuffer(blur1_fbo);
	gfx.resize(fb_width, fb_height);
	gfx.fbAttachTexture(blur1_quad);
	gfx.fbAttachDepth(blur1_depth);
	gfx.clear();
	gfx.SelectTexture(0, mask_quad);
	post.Select();
	post.Params(POST_RADIAL, tick_num);
	post.BloomParams(0, bloom_amount, bloom_strength, bloom_threshold, fb_width, fb_height);
	gfx.clear();
	gfx.SelectIndexBuffer(quad_index);
	gfx.SelectVertexBuffer(quad_vertex);
	gfx.DrawArrayTri(0, 0, 6, 4); // first blur pass
	gfx.bindFramebuffer(0);

	gfx.bindFramebuffer(render_fbo, 2);
	gfx.resize(fb_width, fb_height);
	//	gfx.clear();
	gfx.SelectIndexBuffer(quad_index);
	gfx.SelectVertexBuffer(quad_vertex);
	if (debug)
	{
		gfx.SelectTexture(0, mask_quad);
		gfx.SelectTexture(1, mask_quad);
		gfx.SelectTexture(2, mask_quad);
	}
	else
	{
		gfx.SelectTexture(0, render_quad);
		gfx.SelectTexture(1, blur1_quad);
		gfx.SelectTexture(2, blur1_quad);
	}
	post.Select();
	post.Params(POST_COMBINE, tick_num);
	gfx.DrawArrayTri(0, 0, 6, 4); // add all three together

}

///=============================================================================
/// Function: render_wave
///=============================================================================
/// Description: This adds an underwater effect (blue tint, some wavyness)
///
/// Parameters:
///		None
///=============================================================================
void Engine::render_wave(bool debug)
{
	gfx.bindFramebuffer(mask_fbo);
	gfx.resize(fb_width, fb_height);
	gfx.fbAttachTexture(mask_quad);
	gfx.fbAttachDepth(mask_depth);

	gfx.SelectTexture(0, render_quad);
	gfx.SelectTexture(1, render_depth);
	post.Select();
	post.Params(POST_WAVE, tick_num);
	post.BloomParams(0, bloom_amount, bloom_strength, bloom_threshold, fb_width, fb_height);

	gfx.clear();
	gfx.SelectIndexBuffer(quad_index);
	gfx.SelectVertexBuffer(quad_vertex);
	gfx.DrawArrayTri(0, 0, 6, 4); // bright pass filter
	gfx.bindFramebuffer(0);
	gfx.bindFramebuffer(render_fbo, 2);

	gfx.resize(fb_width, fb_height);
	//	gfx.clear();
	gfx.SelectIndexBuffer(quad_index);
	gfx.SelectVertexBuffer(quad_vertex);
	gfx.SelectTexture(0, mask_quad);
	gfx.SelectTexture(1, mask_quad);
	gfx.SelectTexture(2, mask_quad);
	post.Select();
	post.Params(POST_COMBINE, tick_num);
	gfx.DrawArrayTri(0, 0, 6, 4); // add all three together

}

///=============================================================================
/// Function: render_ssao
///=============================================================================
/// Description: This adds ambient occulusion (shadowing) using screen space
/// techniques to add shadows to "cracks" using the depth buffer and normals
///
/// Parameters:
///		None
///=============================================================================
void Engine::render_ssao(bool debug)
{
	gfx.bindFramebuffer(ssao_fbo);
	gfx.resize(fb_width, fb_height);
	ssao.Select();

	ssao.Params(ssao_radius, object_level, ssao_level, randomize_points, point_count, fb_width, fb_height);

	gfx.SelectTexture(0, render_quad);
	gfx.SelectTexture(1, render_ndepth);
	gfx.SelectIndexBuffer(quad_index);
	gfx.SelectVertexBuffer(quad_vertex);
//	glDisable(GL_DEPTH_TEST);
	gfx.DrawArrayTri(0, 0, 6, 4);
}


///=============================================================================
/// Function: destroy_buffers
///=============================================================================
/// Description: This is used to delete allocated buffers when unloading a map
///
/// Parameters:
///		None
///=============================================================================
void Engine::destroy_buffers()
{
//	zcc.destroy_buffers(gfx);
//	sentry.destroy_buffers(gfx);
//	zsec_shotgun.destroy_buffers(gfx);

	for (int i = 0; i < MAX_MODEL; i++)
	{
		md5_model[i].destroy_buffers(gfx);
	}

	gfx.DeleteFrameBuffer(render_fbo, render_quad, render_depth);
	gfx.DeleteFrameBuffer(mask_fbo, mask_quad, mask_depth);
	gfx.DeleteFrameBuffer(blur1_fbo, blur1_quad, blur1_depth);
	gfx.DeleteFrameBuffer(blur2_fbo, blur2_quad, blur2_depth);

	key_bind.destroy();

	for (unsigned int i = 0; i < snd_wave.size(); i++)
	{
		delete [] snd_wave[i].data;
	}
	snd_wave.clear();
}


///=============================================================================
/// Function: spatial_testing
///=============================================================================
/// Description: This is used to perform various checks based on distances between
/// objects (eg: enable lights close to the player, which also pass BSP PVS checks)
///
/// Parameters:
///		None
///=============================================================================
void Engine::spatial_testing()
{
	int leaf_a = -1;
	int leaf_b = -1;

	// Run this at ~20 tims a second
//	if (tick_num % 6 != 0)
//		return;

	if (emitter.position.y >= -9999.0f)
	{
		if (q3map.vis_test(emitter.position, camera_frame.pos, leaf_a, leaf_b))
		{
			emitter.visible = true;
		}
		else
		{
			emitter.visible = false;
		}
	}

//	#pragma omp parallel for num_threads(8)
	for (unsigned int i = 0; i < entity_list.size(); i++)
	{
#ifdef OPENMP
//		int thread_num = omp_get_thread_num();
	//	int num_thread = omp_get_num_threads();

		//if (entity_list[i]->bsp_leaf % num_thread != thread_num)
			//continue;

//		printf("bsp leaf %d Handled by thread %d of %d\n", entity_list[i]->bsp_leaf, thread_num, num_thread);
#endif


		// set pursue / evade
		// (really need to move elsewhere, but had an entity loop here)
		EntRigidBody *rigid = entity_list[i]->rigid;
		if (rigid)
		{
			if (rigid->target)
			{
				if (rigid->flags.pursue_flag == true)
				{
//					rigid->wander(20.0f, 1.0f, 5.0f);
					rigid->pursue();
				}
				else
				{
					rigid->evade();
				}
			}
		}

		if (entity_list[i]->model)
		{
			matrix4 trans;
			matrix4 mvp;

			bool bsp_visible = false;
			//bool frustum_visible = false;
			bool visible = false;

			camera_frame.set(trans);
			entity_list[i]->model->get_matrix(mvp.m);
			mvp = (mvp * trans) * projection;

			EntModel *model = entity_list[i]->model;


			vec3 min = model->aabb[0];
			vec3 max = model->aabb[7];

			if (model->flags.rail_trail)
			{
				entity_list[i]->flags.frustum_visible = true; // trail extends past aabb
			}
			else
			{
				entity_list[i]->flags.frustum_visible = aabb_visible(min, max, mvp);
			}


			for(unsigned int j = 0; j < 8; j++)
			{
				vec3 position = entity_list[i]->position + model->aabb[j];
				bool vert_visible = q3map.vis_test(camera_frame.pos, position, leaf_a, leaf_b);

				if (vert_visible)
				{
					bsp_visible = true;
					break;
				}
			}

			entity_list[i]->flags.bsp_visible = bsp_visible;

			int player = find_type(ENT_PLAYER, 0);

			if (player != -1)
				entity_list[player]->bsp_leaf = leaf_a;
			entity_list[i]->bsp_leaf = leaf_b;

			if ((bsp_visible && entity_list[i]->flags.frustum_visible) || i == (unsigned int)player)
			{
				visible = true;
			}


			// make triggered entities disappear
			EntTrigger *trigger = entity_list[i]->trigger;
			if (trigger)
			{
				if (trigger->hide == false)
				{
					//Always show with hide flag false
					entity_list[i]->flags.visible = visible;
				}
				else if (trigger->active == false)
				{
					//Only show when not triggered
					entity_list[i]->flags.visible = visible;
				}
			}
			else
			{
				entity_list[i]->flags.visible = visible;
			}
		}
		else
		{
			// Lights? what else?

			int player = find_type(ENT_PLAYER, 0);

	
			entity_list[i]->flags.visible = q3map.vis_test(camera_frame.pos, entity_list[i]->position, leaf_a, leaf_b);
			entity_list[i]->bsp_leaf = leaf_b;
			if (player != -1)
				entity_list[player]->bsp_leaf = leaf_a;

		}

		if (entity_list[i]->flags.visible == false)
			continue;

		EntLight *light = entity_list[i]->light;
		if (light)
		{
			if (light->timer_flag)
			{
				if (light->timer > 0)
				{
					light->timer -= 25;
				}
				else
				{
					clean_entity(i);
					continue;
				}
			}
			vec3 dist_vec = entity_list[i]->position - camera_frame.pos;
			float distance = dist_vec.x * dist_vec.x + dist_vec.y * dist_vec.y + dist_vec.z * dist_vec.z;

			activate_light(distance, light);
		}

	}

}

///=============================================================================
/// Function: activate_light
///=============================================================================
/// Description: This is used to active lights based on distance
///
/// Parameters:
///		None
///=============================================================================
void Engine::activate_light(float distance, EntLight *light)
{
//	float min_distance = FLT_MAX;

	if (distance < 9 * 800.0f * 800.0f && light->entity->flags.visible)
	{
		if (light->active == false)
		{
			light_list.push_back(light);
			light->active = true;
		}
	}
	else
	{
		// remove active lights that fail distance check
		if (light->active == true )
		{
			for(unsigned int i = 0; i < light_list.size(); i++)
			{
				if (light_list[i] == light)
				{
					light_list.erase(light_list.begin() + i);
					break;
				}
			}
			light->active = false;
//			light->entity->rigid->angular_velocity.x = 0.0;
//			light->entity->rigid->angular_velocity.y = 0.0;
		}
	}
}



///=============================================================================
/// Function: handle_cloth
///=============================================================================
/// Description: Handles physics simulation of cloth objects
/// cloths are essentially treated like a series of interconnected springs
///
/// Parameters:
///		None
///=============================================================================
void Engine::handle_cloth()
{
	float time = 0.125f;

	if (enable_cloth)
	{
		for (unsigned int i = 0; i < cloth.size(); i++)
		{
			if (entity_list[cloth[i]->ent_index]->flags.bsp_visible == false)
				continue;

			cloth[i]->add_force(vec3(0, -9.8f, 0) * time);
			cloth[i]->wind_force(vec3(-0.004f, 0, -0.004f) * time);
			cloth[i]->step(time);


			gfx.DeleteIndexBuffer(cloth[i]->ibo);
			gfx.DeleteVertexBuffer(cloth[i]->vbo);
			cloth[i]->create_buffers(gfx);
		}

		//	ball_time++;
		//	ball_pos.z = cos(ball_time / 50.0f) * 7;
		//cloth1.ballCollision(ball_pos, ball_radius);
	}
}

///=============================================================================
/// Function: dynamics
///=============================================================================
/// Description: Handles physics simulation of entities
///
///
/// Old comments for OpenMP multithreading this: (not really needed, but could be used if scaling up to thousands of objects)
///
///	This function can be threaded by:
///	1. Dividing particles by bsp leafs
///	   call find leaf for each vertex in aabb
///	   if an objects aabb is in multiple leafs
///	   union objects in both leafs together
///	2. create a thread for each leaf batch.
///	3. compute naive O(n^2)
///	   each core should be able to handle 1000+ objects in 16ms, so no worrys
///	   write results
///	4. join threads
///
///
/// Parameters:
///		None
///=============================================================================
void Engine::dynamics()
{

	//handle_springs();

#ifndef SOFTWARE
	handle_cloth();
#endif

//	#pragma omp parallel for num_threads(8)
	for (unsigned int i = 0; i < entity_list.size(); i++)
	{
#ifdef OPENMP
		int thread_num = omp_get_thread_num();
		int num_thread = omp_get_num_threads();

		if (entity_list[i]->bsp_leaf % num_thread != thread_num)
			continue;

		printf("bsp leaf %d Handled by thread %d of %d\n", entity_list[i]->bsp_leaf, thread_num, num_thread);
#endif

		if (entity_list[i]->vehicle)
		{
			entity_list[i]->vehicle->step(0.01f);
			continue;
		}


		if (entity_list[i]->rigid == NULL)
			continue;

		if (entity_list[i]->flags.nodraw)
			continue;

		if (collision_detect_enable == false && i >= max_player)
			continue;

		EntRigidBody *body = entity_list[i]->rigid;

		float delta_time = TICK_MS / 1000.0f;
		float target_time = delta_time;
		float current_time = 0.0f;
		int divisions = 0;

		if (body->flags.water)
		{
			float submerged_percent = 1.0f;
			//float volume = body->get_volume();
			float height = 0.5f * abs32(body->aabb[0].z - body->aabb[7].z);

			if (body->water_depth < height)
			{
				submerged_percent = body->water_depth / height;
			}

			submerged_percent += 0.95f; // prevent floating just below surface

			// clamp
			if (submerged_percent > 1.0f)
				submerged_percent = 1.0f;
			if (submerged_percent < 0.0f)
				submerged_percent = 0.0f;

			//Force_bouyant = volume * density of water * 9.8
			// density of water is 10000 kg/m3, but fudging as a scale factor
			//float force_bouyant = volume * 0.0015f * GRAVITY * submerged_percent;

//			body->net_force += vec3(0.0f, force_bouyant, 0.0f);
		}



		while (current_time < delta_time)
		{
			cfg_t	config;

			body->save_config(config);
			body->integrate(target_time - current_time);

			if (entity_list[i]->brush_ref != -1)
				break;



			if ( collision_detect(*body) )
			{
				vec3 clip = body->velocity;
				body->load_config(config);
				body->velocity = clip;

				target_time = (current_time + target_time) / 2.0f;
				divisions++;

				if (divisions > 10)
				{
//					printf("Entity %d is sleeping\n", i);
					body->flags.sleep = true;
					break;
				}
				continue;
			}
			current_time = target_time;
			target_time = delta_time;
		}
		if (abs32(body->velocity.y) > 1.0f)
		{
			// either jumping or falling
			body->flags.on_ground = false;
		}

		body->net_force = vec3(0.0f, 0.0f, 0.0f);
	}
}


///=============================================================================
/// Function: dynamics
///=============================================================================
/// Description: Handles physics simulation of springs, not really used,
///  but can attach two objects together (think below has armor and lightning gun)
///
/// Parameters:
///		None
///=============================================================================
void Engine::handle_springs()
{
	int num_body_spring = 0;
	int num_spring = 1;
	float spring_k = 0.6f;
	float damping = 0.1f;
	body_spring_t body_spring;
	spring_t spring;

	body_spring.rigid0 = find_type(ENT_ITEM_ARMOR_COMBAT, 0);
	body_spring.rigid1 = find_type(ENT_WEAPON_LIGHTNING, 0);
	body_spring.vertex0 = 0;
	body_spring.vertex1 = 0;

	if (body_spring.rigid0 == -1 || body_spring.rigid1 == -1)
		return;

	int index = find_type(ENT_PLAYER, 0);
	vec3 point; 

	if (index != -1)
		point = entity_list[index]->position;

	spring.anchor = point;
	spring.rigid0 = find_type(ENT_ITEM_ARMOR_COMBAT, 0);
	spring.vertex0 = 0;

	if (spring.rigid0 == -1)
		return;



	for (int i = 0; i < num_body_spring; i++)
	{
		EntRigidBody *rigid0 = entity_list[body_spring.rigid0]->rigid;
		vec3 position0 = rigid0->entity->position + rigid0->model_vertex_array[body_spring.vertex0].position;
		vec3 u0 = position0 + rigid0->center;
		vec3 vu0 = rigid0->velocity + vec3::crossproduct(rigid0->angular_velocity, u0);

		EntRigidBody *rigid1 = entity_list[body_spring.rigid1]->rigid;
		vec3 position1 = rigid1->entity->position + rigid1->model_vertex_array[body_spring.vertex1].position;
		vec3 u1 = position1 + rigid1->center;
		vec3 vu1 = rigid1->velocity + vec3::crossproduct(rigid1->angular_velocity, u1);

		vec3 spring_vector = position1 - position0;
		vec3 spring_force = spring_vector * -spring_k;

		vec3 relative_velocity = vu1 - vu0;

		vec3 damping_force = spring_vector * (relative_velocity * spring_vector) /
			(spring_vector * spring_vector) * -damping;

		spring_force += damping_force;

		rigid0->net_force -= spring_force;
		rigid0->net_torque -= vec3::crossproduct(u0, spring_force);

		// Limit max forces
		if (rigid0->net_force.magnitude() > 100.0f)
			rigid0->net_force = rigid0->net_force.normalize() * 100.0f;
		if (rigid0->net_torque.magnitude() > 100.0f)
			rigid0->net_torque = rigid0->net_torque.normalize() * 100.0f;


		rigid1->net_force += spring_force;
		rigid1->net_torque += vec3::crossproduct(u1, spring_force);

		if (rigid1->net_force.magnitude() > 100.0f)
			rigid1->net_force = rigid1->net_force.normalize() * 100.0f;
		if (rigid1->net_torque.magnitude() > 100.0f)
			rigid1->net_torque = rigid1->net_torque.normalize() * 100.0f;
	}

	for (int i = 0; i < num_spring; i++)
	{
		EntRigidBody *rigid0 = entity_list[spring.rigid0]->rigid;
		vec3 position0 = rigid0->entity->position + rigid0->model_vertex_array[spring.vertex0].position;
		vec3 u0 = position0 + rigid0->center;
		vec3 vu0 = rigid0->velocity + vec3::crossproduct(rigid0->angular_velocity, u0);


		vec3 spring_vector = position0 - spring.anchor;
		vec3 spring_force = spring_vector * -spring_k;

		vec3 damping_force = spring_force * (vu0 * spring_force) / (spring_force * spring_force) * -damping;

		spring_force += damping_force;

		rigid0->net_force += spring_force;
		rigid0->net_torque += vec3::crossproduct(u0, spring_force);

		// Limit max forces
		if (rigid0->net_force.magnitude() > 100.0f)
			rigid0->net_force = rigid0->net_force.normalize() * 100.0f;
		if (rigid0->net_torque.magnitude() > 100.0f)
			rigid0->net_torque = rigid0->net_torque.normalize() * 100.0f;
	}
}

///=============================================================================
/// Function: collision_detect
///=============================================================================
/// Description: This is used to check if the passed rigid body has collided with
/// the map or another solid rigid body. Function handles all collision detection
///
/// Parameters:
///		None
/// Returns:
///		return true if simulated too far. (results in smaller time step)
///		return false if collision handled.
///=============================================================================
bool Engine::collision_detect(EntRigidBody &body)
{
//	Plane plane(vec3(0.0f, 1.0f, 0.0f).normalize(), 500.0f);	

	// ignore func items
	if (body.entity->ent_type == ENT_PATH_CORNER)
	{
		return false;
	}

	if (map_collision(body))
	{
		return true;
	}

	if (terrain.collision_detect(body))
	{
		return true;
	}


	if (body.entity->player || body.entity->construct)
	{
		if (body_collision(body))
			return true;
	}

	return false;
}


///=============================================================================
/// Function: debug_point
///=============================================================================
/// Description: This function is used to draw a point in space
/// with fixed function OpenGL this was easy, little more trouble
/// with modern opengl as even a single point needs a vertex buffer
///
/// Parameters:
///		Point to add to debug point buffer
/// Returns:
/// zero success, non-zero failure
///=============================================================================
int Engine::debug_point(vec3 &pos)
{
	vertex_t vert[6];

	memset(vert, 0, 6 * sizeof(vertex_t));
	vert[0].position = pos + vec3(0.0, 1.0, 0.0) * -5.0f;
	vert[1].position = pos + vec3(0.0, 1.0, 0.0) * 5.0f;

	vert[2].position = pos + vec3(1.0, 0.0, 0.0) * -5.0f;
	vert[3].position = pos + vec3(1.0, 0.0, 0.0) * 5.0f;

	vert[4].position = pos + vec3(0.0, 0.0, 1.0) * -5.0f;
	vert[5].position = pos + vec3(0.0, 0.0, 1.0) * 5.0f;


	debug_vec_list_index.push_back(debug_vec_list.size());
	debug_vec_list.push_back(vert[0]);
	debug_vec_list_index.push_back(debug_vec_list.size());
	debug_vec_list.push_back(vert[1]);
	debug_vec_list_index.push_back(debug_vec_list.size());
	debug_vec_list.push_back(vert[2]);
	debug_vec_list_index.push_back(debug_vec_list.size());
	debug_vec_list.push_back(vert[3]);
	debug_vec_list_index.push_back(debug_vec_list.size());
	debug_vec_list.push_back(vert[4]);
	debug_vec_list_index.push_back(debug_vec_list.size());
	debug_vec_list.push_back(vert[5]);

	debug_vec_vert = gfx.CreateVertexBuffer(debug_vec_list.data(), debug_vec_list.size(), false);
	debug_vec_index = gfx.CreateIndexBuffer(debug_vec_list_index.data(), debug_vec_list_index.size());

	if (debug_vec_vert != -1 && debug_vec_index != -1)
	{
		return 0;
	}

	return -1;
}


///=============================================================================
/// Function: debug_vector
///=============================================================================
/// Description: This function is used to draw a vector in space
/// with fixed function OpenGL this was easy, little more trouble
/// with modern opengl as even a single point needs a vertex buffer
///
/// Parameters:
///		Point to add to debug point buffer
/// Returns:
/// zero success, non-zero failure
///=============================================================================
int Engine::debug_vector(vec3 &pos, vec3 &dir)
{
	vertex_t vert[2];

	memset(vert, 0, 2 * sizeof(vertex_t));
	vert[0].position = pos;
	vert[1].position = pos + dir * 10.0f;

	debug_vec_list_index.push_back(debug_vec_list.size());
	debug_vec_list.push_back(vert[0]);
	debug_vec_list_index.push_back(debug_vec_list.size());
	debug_vec_list.push_back(vert[1]);


	debug_vec_vert = gfx.CreateVertexBuffer(debug_vec_list.data(), debug_vec_list.size(), false);
	debug_vec_index = gfx.CreateIndexBuffer(debug_vec_list_index.data(), debug_vec_list_index.size());

	if (debug_vec_vert != -1 && debug_vec_index != -1)
	{
		return 0;
	}

	return -1;
}


///=============================================================================
/// Function: debug_vector
///=============================================================================
/// Description: This function is used to draw a triangle in space
/// with fixed function OpenGL this was easy, little more trouble
/// with modern opengl as even a single point needs a vertex buffer
///
/// Parameters:
///		Point to add to debug point buffer
/// Returns:
/// zero success, non-zero failure
///=============================================================================
int Engine::debug_triangle(vec3 *triangle)
{
	vertex_t vert[6];

	memset(vert, 0, 3 * sizeof(vertex_t));
	vert[0].position = triangle[0];
	vert[1].position = triangle[1];
	vert[2].position = triangle[2];

//	vert[3].position = triangle[0];
//	vert[4].position = triangle[2];
//	vert[5].position = triangle[1];


	debug_tri_list_index.push_back(debug_tri_list.size());
	debug_tri_list.push_back(vert[0]);
	debug_tri_list_index.push_back(debug_tri_list.size());
	debug_tri_list.push_back(vert[1]);
	debug_tri_list_index.push_back(debug_tri_list.size());
	debug_tri_list.push_back(vert[2]);

/*	debug_tri_list_index.push_back(debug_tri_list.size());
	debug_tri_list.push_back(vert[3]);
	debug_tri_list_index.push_back(debug_tri_list.size());
	debug_tri_list.push_back(vert[4]);
	debug_tri_list_index.push_back(debug_tri_list.size());
	debug_tri_list.push_back(vert[5]);
	*/
	debug_tri_vert = gfx.CreateVertexBuffer(debug_tri_list.data(), debug_tri_list.size(), false);
	debug_tri_index = gfx.CreateIndexBuffer(debug_tri_list_index.data(), debug_tri_list_index.size());

	if (debug_tri_vert != -1 && debug_tri_index != -1)
	{
		return 0;
	}

	return -1;
}


///=============================================================================
/// Function: map_collision
///=============================================================================
/// Description: This is used to check if the passed rigid body has collided with
/// the map
///
/// Parameters:
///		None
///
/// Returns:
///		return true if simulated too far. (results in smaller time step)
///		return false if collision handled.
///=============================================================================
bool Engine::map_collision(EntRigidBody &body)
{
	plane_t plane;
	float depth;
	vec3 staircheck(0.0f, STAIR_HEIGHT, 0.0f);
	vec3 clip(0.0f, 0.0f, 0.0f);
	bool collision = false;
	vec3 mid[4];
	float duck_scale = 1.0f;



	if (body.flags.noclip)
		return false;

	if (body.entity->player)
	{
		if (body.entity->player->state == PLAYER_DUCKED)
		{
			duck_scale = 2.0f;
		}

		// Do additional mid point testing (front back left right points, mid level)
		mid[0] = body.aabb[0] + vec3(body.aabb[7].x / 2, 0, body.aabb[7].z / 2);
		mid[1] = body.aabb[0] + vec3(0.0f, body.aabb[7].y / 2, body.aabb[7].z / 2);
		mid[2] = body.aabb[0] + vec3(body.aabb[7].x / 2, body.aabb[7].y, body.aabb[7].z / 2);
		mid[3] = body.aabb[0] + vec3(body.aabb[7].x, body.aabb[7].y / 2, body.aabb[7].z / 2);
	}

	// Check bounding box against map
	for(int i = 0; i < 8 + 4; i++)
	{
		vec3 point;
		vec3 oldpoint;
		content_flag_t flag;

		memset(&flag, 0, sizeof(content_flag_t));
		if (i < 8)
		{
			point = body.aabb[i];
			point.y /= duck_scale;
			oldpoint = point;

			point += -body.center + body.entity->position;
			oldpoint += -body.center + body.old_position;
		}
		else
		{
			if (body.entity->player == NULL)
				break;

			point = mid[i - 8] - body.center + body.entity->position;
			oldpoint = mid[i - 8] - body.center + body.old_position;
		}





//		can be used to avoid checking all eight points, but checking all 8 works pretty well
//		vec3 point = body.center + body.entity->position;
//		point -= vec3(0.0f, 100.0f, 0.0f); // subtract player height


		vec3 tri[3];
		vec3 empty[3];

		memset(tri, 0, sizeof(vec3) * 3);
		memset(empty, 0, sizeof(vec3) * 3);

		if (q3map.collision_detect(point, oldpoint, (plane_t *)&plane, &depth, body.water_depth,
			surface_list, body.flags.step_flag && input.use, clip, body.velocity, body.bsp_trigger_volume, body.bsp_model_platform, flag, tri))
		{

			if (memcpy(tri, empty, sizeof(vec3) * 3))
			{
//				debug_vector(tri[0], plane.normal);
//				debug_triangle(tri);
			}

			body.flags.lava = flag.lava;
			body.flags.slime = flag.slime;
			body.flags.water = flag.water;

			if (body.flags.step_flag)
			{
				if (flag.surf_flags & SURF_METALSTEPS)
				{
					body.step_type = SURF_METALSTEPS;
				}
				else if (flag.surf_flags & SURF_FLESH)
				{
					body.step_type = SURF_FLESH;
				}
				else if (flag.surf_flags & SURF_NOSTEPS)
				{
					body.step_type = SURF_NOSTEPS;
				}

				else
				{
					body.step_type = 0;
				}



				// Moving, on ground plane, if we get clipped below velocity threshold and will fail to climb stairs
				// So dont mess up :)
				if ( (abs32(body.velocity.x) > 0.25f || abs32(body.velocity.z) > 0.25f))
				{
					vec3 p = point + staircheck;
					//vec3 old = oldpoint + staircheck;

					if (q3map.collision_detect(p, oldpoint, (plane_t *)&plane, &depth, body.water_depth,
						surface_list, body.flags.step_flag && input.use, clip, body.velocity, body.bsp_trigger_volume, body.bsp_model_platform, flag, tri) == false)
					{
						body.entity->position += vec3(0.0f, STAIR_POS, 0.0f);
						body.velocity += vec3(0.0f, STAIR_VEL, 0.0f);
						body.flags.on_ground = true;

						if (body.entity->player)
						{
							float speed = body.velocity.magnitude();
							float speed_scale = 1.2f;

							if (body.entity->player->haste_timer > 0)
								speed_scale = body.entity->player->haste_factor;
							#define MAX_SPEED 3.0f

							if (speed > MAX_SPEED * speed_scale)
							{
								body.velocity.x *= (MAX_SPEED * speed_scale / speed);
								body.velocity.z *= (MAX_SPEED * speed_scale / speed);
							}
						}


						continue;
					}
				}
			}

			if (body.bsp_model_platform > 0 && (q3map.model_type[body.bsp_model_platform] > ENT_FUNC_START && q3map.model_type[body.bsp_model_platform] < ENT_FUNC_END))
			{
				vec3 dir = (q3map.model_offset[body.bsp_model_platform] - body.entity->position);
				body.velocity += dir.normalize() * 10.0f;
				body.bsp_model_platform = 0;
			}
			else if (body.bsp_model_platform > 0)
			{
				body.bsp_model_platform = 0;
			}

			body.entity->position = body.old_position;
			body.morientation = body.old_orientation;
			//body.impulse(plane, point);
			collision = true;
		}
	}

	// Do a bsp trace collision to eliminate the case where we are going really fast and pass through objects
	
	if (collision == false && body.entity->player && body.velocity.magnitude() > 6.0f )
	{
		for (int i = 0; i < 8; i++)
		{
			vec3 point;
			vec3 oldpoint;
			vec3 normal;

			point = body.aabb[i] - body.center + body.entity->position;
			oldpoint = body.aabb[i] - body.center + body.old_position;

			q3map.trace(oldpoint, point, normal);

			if (q3map.collision)
			{
				collision = true;
				q3map.collision = false;
				break;
			}
		}
	}
	



	if (collision)
	{
		body.flags.on_ground = true;
		if (body.velocity.y <= -RIGID_IMPACT)
		{
			body.flags.hard_impact = true;
			body.impact_velocity = body.velocity.y;
		}

		vec3 normal = -plane.normal;
		clip = body.velocity;
		ClipVelocity(clip, normal);
		// Clip velocity, and reduce ground friction
		body.velocity = clip + vec3(0.0f, 0.01f, 0.0f);
		body.bounce++;
	}

	return collision;
}


///=============================================================================
/// Function: body_collision
///=============================================================================
/// Description: Handles body / body collision
///
/// O(N^2) -- function is slow / doesnt scale well
///
/// Parameters:
///		None
///
/// Returns:
///		return true if simulated too far. (results in smaller time step)
///		return false if collision handled.
///=============================================================================
bool Engine::body_collision(EntRigidBody &body)
{
	for(unsigned int i = 0; i < max_dynamic; i++)
	{
		// Dont collide with yourself
		if (entity_list[i] == body.entity)
			continue;

		// dont collide if missing rigid class
		if (entity_list[i]->rigid == NULL)
			continue;

		// keep sentry base from colliding from sentry gun
		if (body.entity->ent_type == ENT_CONSTRUCT && entity_list[i]->ent_type == ENT_SENTRY)
			return false;

		// same thing, but order reversed
		if (body.entity->ent_type == ENT_SENTRY && entity_list[i]->ent_type == ENT_CONSTRUCT)
			return false;

		// keep things like machinegun bullets from colliding with players
		if (body.entity->ent_type == ENT_UNKNOWN || entity_list[i]->ent_type == ENT_UNKNOWN)
			return false;

		// keep bots from colliding with eachother
		if (body.entity->ent_type == ENT_NPC && entity_list[i]->ent_type == ENT_NPC)
		{
			return false;
		}



#if 1
		if (entity_list[i]->ent_type == ENT_PLAYER ||
			entity_list[i]->ent_type == ENT_CONSTRUCT || 
			(entity_list[i]->ent_type == ENT_SENTRY && entity_list[i]->construct &&
				entity_list[i]->construct->build_timer == 0) )
		{
			int result = 0;
			vec3 shape1[8];
			vec3 shape2[8];

			for (int j = 0; j < 8; j++)
			{
				shape1[j] = body.aabb[j] + body.entity->position + body.center;
				shape2[j] = entity_list[i]->rigid->aabb[j] + entity_list[i]->position + entity_list[i]->rigid->center;
				//shape2[j] = body.aabb[j] + body.entity->position + body.center + vec3(1000.0f, 1000.0f, 1000.0f);
			}


//			result = separating_axis_theorem(shape1, shape2);
			result = gjk(shape1, shape2, 10, 8, 8);
			if (result)
			{
//				printf("collision between %d and entity type %d\n", entity_list[i]->ent_type, body.entity->ent_type);
				return true;
			}
		}
#endif

		if (body.collision_distance(*entity_list[i]->rigid))
			return true;


//		if (body.entity->bsp_leaf == entity_list[i]->bsp_leaf)



	}
	return false;
}

///=============================================================================
/// Function: step
///=============================================================================
/// Description: Function handles a time step (occurs roughly every 16ms) based
/// on windows timer or linux timer, advances object position through time.
///
/// Where rendering only draws objects (it does not modify game state)
///
/// O(N^2) -- function is slow / doesnt scale well
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Engine::step(int tick)
{
	tick_num = tick;
	if (fullscreen_timer > 0)
		fullscreen_timer--;

	if (q3map.loaded == false)
		return;

	if (ingame_menu_timer > 0)
		ingame_menu_timer--;

#ifndef DEDICATED
	// Animate animated textures
	for (unsigned int i = 0; i < q3map.anim_list.size(); i++)
	{
		texture_t  *tex = q3map.anim_list[i];

		if (tex->num_anim == 0)
			continue;

		int texunit = tex->anim_unit;

		if (tex->freq == 0)
			continue;

		if (tick_num % ((tex->freq * TICK_RATE) / TICK_RATE) == 0)
		{
			int ani_index = tex->anim_count % tex->num_anim;
			tex->texObj[texunit] = tex->texObjAnim[ani_index];
			tex->anim_count++;
		}
	}

	spatial_testing(); // mostly sets visible flag
#endif

	game->step(tick);
	dynamics();


	//network
    netcode.sequence++;
	if (netcode.server_flag && netcode.sequence)
	{
		while (netcode.server_recv());
		netcode.server_send();

#ifdef VOICECHAT
		while (voice.voice_send(audio, netcode.client_list, netcode.client_flag, netcode.server_flag));
		while (voice.voice_recv(audio));
#endif
	}
	else if (netcode.client_flag && netcode.sequence)
	{
		while (netcode.client_recv());
		netcode.client_send(input, camera_frame);
#ifdef VOICECHAT
		while (voice.voice_send(audio, netcode.client_list, netcode.client_flag, netcode.server_flag));
		while (voice.voice_recv(audio));
#endif
	}

	if (netcode.playing_demo)
	{
		demo_frameheader_t header;
		static unsigned char data[4096];
//		unsigned int num_read;

		memset(data, 0, 4096);
		int ret = fread(&header, sizeof(demo_frameheader_t), 1, netcode.demofile);
		if (ret != sizeof(demo_frameheader_t))
		{
			netcode.playing_demo = false;
			unload();
			menu.print("Invalid frame");
			return;
		}

		if (strcmp(header.magic, "frame") != 0)
		{
			netcode.playing_demo = false;
			unload();
			menu.print("Invalid frame");
			return;
		}
/*
		num_read = fread(&data[0], 1, sizeof(entity_t) * header.num_ents, demofile);
		if (num_read != sizeof(entity_t) * header.num_ents)
		{
			playing_demo = false;
			unload();
			menu.print("Not enough entity data in frame");
			return;
		}
		deserialize_ents(data, header.num_ents);
		update_audio();
*/
		return;
	}



	int player = find_type(ENT_PLAYER, 0);

	int current_light = 0;
	if (player != -1)
	{
		current_light = entity_list[player]->player->current_light;
	}

	if (enable_stencil)
	{
		for (unsigned int i = max_dynamic; i < entity_list.size(); i++)
		{
			if (entity_list[i]->light)
			{
				if (enable_map_shadows)
				{
					entity_list[i]->light->generate_map_volumes(gfx, q3map, current_light);
				}

				if (enable_entities)
					entity_list[i]->light->generate_ent_volumes(gfx, entity_list);
			}
		}
		enable_map_shadows = false;
	}


	if (netcode.recording_demo)
	{
		static servermsg_t servermsg;
		demo_frameheader_t header;

		memcpy(header.magic, "frame", 6);
		netcode.serialize_ents(servermsg.data, servermsg.num_ents, servermsg.data_size);
		header.num_ents = servermsg.num_ents;
		header.tick_num = tick_num;

//		fwrite(&header, sizeof(demo_frameheader_t), 1, demofile);
//		fwrite(&servermsg, servermsg.num_ents * sizeof(entity_t), 1, demofile);
	}

#ifndef DEDICATED
	update_audio();
#endif
}


///=============================================================================
/// Function: savegame
///=============================================================================
/// Description: used to save the game state, which is done by serializing all
/// entities and other gamestate info (map, score, etc) to disk
///
/// Note: game state serialization is also used for net code
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Engine::savegame(char *file)
{
	static unsigned char buffer[16584];
	unsigned int size = 0;
	unsigned short num_ents = (unsigned short)entity_list.size();

	netcode.serialize_ents(buffer, num_ents, size);

	char num[80];

	sprintf(num, "-%d", num_ents);
	strcat(file, num);

	write_file(file, (char *)buffer, size);

	debugf("Saved %s", file);
}

///=============================================================================
/// Function: loadgame
///=============================================================================
/// Description: used to load the game state, which is done by deserializing all
/// entities and other gamestate info (map, score, etc) to memory
///
/// Note: game state serialization is also used for net code
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Engine::loadgame(char *file)
{
	unsigned int size = 0;
	unsigned char *data = (unsigned char *)get_file(file, &size);
	int num_ents = atoi(strstr(file, "-") + 1);

	netcode.deserialize_ents(data, num_ents, size);

}


///=============================================================================
/// Function: mousepos_raw
///=============================================================================
/// Description: mouse handler for "raw" USB HID input direct from mouse
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
bool Engine::mousepos_raw(int x, int y, int deltax, int deltay)
{
	static bool once = false;

	if ((q3map.loaded == false && hlmap.loaded == false && q1map.loaded == false) || menu.ingame == true || menu.console == true || menu.chatmode == true)
	{
		return false;
	}

	if (once)
	{
		//recenter on game load so view doesnt spin too much
		once = false;
		return true;
	}

	if (raw_mouse == true)
	{
		if (menu.data.invert)
			camera_frame.update(vec2((float)deltax, (float)-deltay), sensitivity, enable_planet == false);
		else
			camera_frame.update(vec2((float)deltax, (float)deltay), sensitivity, enable_planet == false);
	}

	return true;
}

///=============================================================================
/// Function: mousepos
///=============================================================================
/// Description: mouse handler for normal mouse input (WM_MOUSEMOVE, or mouse events
/// from X11 system)
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
bool Engine::mousepos(int x, int y, int deltax, int deltay)
{
	static bool once = false;

    
	if ((q3map.loaded == false && hlmap.loaded == false && q1map.loaded == false) || menu.ingame == true || menu.console == true || menu.chatmode == true || pick_mode == true)
	{
		float devicex = (float)x / gfx.width;
		float devicey = (float)y / gfx.height;

		if (menu.console == true)
			return false;

		if (menu.chatmode == true)
			return false;

		if (pick_mode)
		{
			show_box = true;
			show_names = true;


			Pick(x, y, gfx.width, gfx.height, camera_frame);


			return false;
		}

		bool updated = menu.delta(devicex, devicey);
		if (menu.ingame == false && updated)
		{
#ifdef OCULUS
			//render's fbo
			gfx.bindFramebuffer(render_fbo);
			gfx.clear();
			menu.render(global);


			ovr.get_pos(head, hpos, lh, lp, rh, rp, re, rep, le, lep, touch);
			vec3 old_pos = camera_frame.pos;
			camera_frame.pos = old_pos + rep;
			projection.perspective(55.0f, (4.0f / 5.0f), 1.0, 2001.0f, true);

			gfx.bindFramebuffer(ovr.ovr_fbo[0]);
			gfx.fbAttachTexture(ovr.get_tex(0));
			gfx.resize(ovr.eye_size[0].w, ovr.eye_size[0].h);
			gfx.clear_color(vec3(0.0f, 1.0f, 0.0f));
			gfx.clear();
			render_texture(render_quad, false);
			ovr_CommitTextureSwapChain(ovr.session, ovr.swap_chain[0]);

			camera_frame.pos = old_pos + lep;


			gfx.bindFramebuffer(ovr.ovr_fbo[1]);
			gfx.fbAttachTexture(ovr.get_tex(1));
			gfx.resize(ovr.eye_size[1].w, ovr.eye_size[1].h);
			gfx.clear_color(vec3(0.0f, 1.0f, 0.0f));
			gfx.clear();
			render_texture(render_quad, false);
			ovr_CommitTextureSwapChain(ovr.session, ovr.swap_chain[1]);
			camera_frame.pos = old_pos;
#else
			gfx.clear();
			menu.render(global);
			gfx.swap();
#endif
		}

#ifdef __APPLE__
        gfx.swap();
#endif
        
		once = true;

		return false;
	}

	if (once)
	{
		//recenter on game load so view doesnt spin too much
		once = false;
		return true;
	}

	if (raw_mouse == false)
	{
		q3map.update = true;
		if (menu.data.invert)
			camera_frame.update(vec2((float)deltax, (float)-deltay), sensitivity, enable_planet == false);
		else
			camera_frame.update(vec2((float)deltax, (float)deltay), sensitivity, enable_planet == false);
	}

	return true;
}

///=============================================================================
/// Function: bind_keys
///=============================================================================
/// Description: used to bind default keys (associate keyboard buttons to input
/// events)
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Engine::bind_keys()
{
	char *file = get_file("media/config.cfg", NULL);
	if (file == NULL)
	{
		key_bind.insert("enter", "jump");
		key_bind.insert("space", "jump");
		key_bind.insert("Z", "pickup");
		key_bind.insert("leftbutton", "attack");
		key_bind.insert("middlebutton", "use");
		key_bind.insert("rightbutton", "zoom");
		key_bind.insert("mousewheelup", "weapon_up");
		key_bind.insert("mousewheeldown", "weapon_down");
		key_bind.insert("]", "weapon_up");
		key_bind.insert("[", "weapon_down");
		key_bind.insert("shift", "duck");
		key_bind.insert("escape", "escape");
		key_bind.insert("control", "control");
		key_bind.insert("up", "moveup");
		key_bind.insert("down", "movedown");
		key_bind.insert("left", "moveleft");
		key_bind.insert("right", "moveright");

		key_bind.insert("~", "console");
		key_bind.insert("`", "console");

		key_bind.insert("T", "talk");
		key_bind.insert("t", "talk");

		key_bind.insert("-", "talkteam");
		key_bind.insert("=", "talkteam");


		key_bind.insert("W", "moveup");
		key_bind.insert("A", "moveleft");
		key_bind.insert("S", "movedown");
		key_bind.insert("D", "moveright");
		key_bind.insert("w", "moveup");
		key_bind.insert("a", "moveleft");
		key_bind.insert("s", "movedown");
		key_bind.insert("d", "moveright");
		key_bind.insert("tab", "scores");
		key_bind.insert("pgup", "pgup");
		key_bind.insert("pgdown", "pgdown");


		key_bind.insert("R", "centerview");
		
		key_bind.insert("F1", "voteyes");
		key_bind.insert("F2", "voteno");
		key_bind.insert("F3", "godmode");
		key_bind.insert("F4", "give all");
		
		key_bind.insert("F5", "respawn");
		key_bind.insert("F6", "r_reload_shaders");
		key_bind.insert("F7", "g_collision 0");
		key_bind.insert("F8", "shadowmaps");

		key_bind.insert("F9", "showdebug");
		key_bind.insert("F10", "fullscreen");
		key_bind.insert("F11", "screenshot");
		//key_bind.insert("F12", "");




		key_bind.insert("numpad0", "walk");
		key_bind.insert("numpad1", "numpad1");
		key_bind.insert("numpad2", "lookforward");
		key_bind.insert("numpad3", "lookdown");
		key_bind.insert("numpad4", "lookleft");
		key_bind.insert("numpad5", "numpad5");
		key_bind.insert("numpad6", "lookright");
		key_bind.insert("numpad7", "numpad7");
		key_bind.insert("numpad8", "lookback");
		key_bind.insert("numpad9", "lookup");

		key_bind.insert("num1", "weapon 1");
		key_bind.insert("num2", "weapon 2");
		key_bind.insert("num3", "weapon 3");
		key_bind.insert("num4", "weapon 4");
		key_bind.insert("num5", "weapon 5");
		key_bind.insert("num6", "weapon 6");
		key_bind.insert("num7", "weapon 7");
		key_bind.insert("num8", "weapon 8");

		key_bind.insert("k", "kill");

		return;
	}

	static char key[128];
	static char value[128];

	char *line = strtok(file, "\r\n");
	int ret = 0;

	while (line)
	{
		ret = sscanf(line, "bind %s \"%s\"\r\n", &key[0], &value[0]);

		//sscanf is grabbing ending quote
		value[strlen(value) - 1] = '\0';
		if (ret == 2)
		{
			key_bind.insert(key, value);
		}
		line = strtok(NULL, "\r\n");
	}
	delete [] file;

	//TODO
	/*
	seta sv_strictAuth "1"
	seta sv_lanForceRate "1"
	seta sv_floodProtect "1"
	seta sv_maxPing "0"
	seta sv_minPing "0"
	seta sv_maxRate "0"
	seta sv_punkbuster "0"
	rate
	cmdrate
	skin
	team red/blue
	seta cg_autoswitch "0"
	banip
	*/



}

///=============================================================================
/// Function: keypress
///=============================================================================
/// Description: This function handles keyboard input, there are two types on input
/// real time up/down key events and higher level typing things for text input, this one
/// is the lower level events, passes in a string representation of the key to
/// avoid using to operating specific key enums. Can probably make enums eventually
/// when we nail down the input set, but performance for key presses isn't really
/// a bottleneck
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Engine::keypress(char *key, bool pressed)
{
	bool handled = false;
	char k = 0;
	char *cmd = "";

	//printf("keypress %s pressed %d\r\n", key, pressed);

	
	cmd = (char *)key_bind.find(key);
	if (cmd == NULL)
		return;

#ifdef __linux__
	k = key[0];
#endif

	q3map.update = true;

	if (strcmp("moveup", cmd) == 0)
	{
		handled = true;
		if (*key != 'w' && *key != 'W')
			k = 3;


		if (edit_ent != -1)
		{
			switch (edit_prop)
			{
			case PROP_POSITION:
				entity_list[edit_ent]->position.z += 10.0f;
				break;
			case PROP_COLOR:
				entity_list[edit_ent]->light->color.z += 0.1f;
				break;
			case PROP_INTENSITY:
				entity_list[edit_ent]->light->intensity += 10.0f;
				break;
			}

		}
		else if (hlmap.loaded || q1map.loaded)
		{
			camera_frame.pos += -camera_frame.forward * 10.0f;
		}
		else
		{
			input.moveup = pressed;
		}

	}
	else if (strcmp("moveleft", cmd) == 0)
	{
		handled = true;
		if (*key != 'a' && *key != 'A')
			k = 4;

		if (edit_ent != -1)
		{
			switch (edit_prop)
			{
			case PROP_POSITION:
				entity_list[edit_ent]->position.x -= 10.0f;
				break;
			case PROP_COLOR:
				entity_list[edit_ent]->light->color.x -= 0.1f;
				break;
			case PROP_INTENSITY:
				entity_list[edit_ent]->light->intensity -= 10.0f;
				break;
			}
		}
		else if (hlmap.loaded || q1map.loaded)
		{
			camera_frame.pos -= vec3::crossproduct(camera_frame.up, camera_frame.forward) * 1.0f;
		}
		else
		{
			input.moveleft = pressed;
		}
	}
	else if (strcmp("moveright", cmd) == 0)
	{
		handled = true;
		if (*key != 'd' && *key != 'D')
			k = 6;

		if (edit_ent != -1)
		{
			switch (edit_prop)
			{
			case PROP_POSITION:
				entity_list[edit_ent]->position.x += 10.0f;
				break;
			case PROP_COLOR:
				entity_list[edit_ent]->light->color.x += 0.1f;
				break;
			case PROP_INTENSITY:
				entity_list[edit_ent]->light->intensity += 10.0f;
				break;
			}

		}
		else if (hlmap.loaded || q1map.loaded)
		{
			camera_frame.pos +=  vec3::crossproduct(camera_frame.up, camera_frame.forward) * 1.0f;
		}
		else
		{
			input.moveright = pressed;
		}
	}
	else if (strcmp("movedown", cmd) == 0)
	{
		handled = true;
		if (*key != 's' && *key != 'S')
			k = 5;

		if (edit_ent != -1)
		{
			switch (edit_prop)
			{
			case PROP_POSITION:
				entity_list[edit_ent]->position.z -= 10.0f;
				break;
			case PROP_COLOR:
				entity_list[edit_ent]->light->color.z -= 0.1f;
				break;
			case PROP_INTENSITY:
				entity_list[edit_ent]->light->intensity -= 10.0f;
				break;
			}
		}
		else if (hlmap.loaded || q1map.loaded)
		{
			camera_frame.pos -= camera_frame.forward * -1.0f;
		}
		else
		{
			input.movedown = pressed;
		}
	}
	else if (strcmp("jump", cmd) == 0)
	{
		handled = true;

		if (edit_ent != -1)
		{
			switch (edit_prop)
			{
			case PROP_POSITION:
				entity_list[edit_ent]->position.y += 10.0f;
				break;
			case PROP_COLOR:
				entity_list[edit_ent]->light->color.y += 0.1f;
				break;
			case PROP_INTENSITY:
				entity_list[edit_ent]->light->intensity += 10.0f;
				break;
			}
			input.jump = false;
		}
		else
		{
			input.jump = pressed;
#ifdef __APPLE__
            // enter should advance the menu
            k = 14;
#endif
		}
	}
	if (strcmp("attack", cmd) == 0)
	{
		handled = true;
		input.attack = pressed;
		k = 14;
	}
	else if (strcmp("weapon_up", cmd) == 0)
	{
		int spawn = find_type(ENT_PLAYER, 0);
		handled = true;

		netcode.sv_maxclients++;
		printf("frame %d\r\n", netcode.sv_maxclients);
		input.weapon_up = pressed;
		if (netcode.client_flag && spawn != -1)
		{
			entity_list[spawn]->player->change_weapon_up();
		}

		if (edit_ent != -1)
		{
			edit_prop++;
			if (edit_prop >= MAX_PROP)
				edit_prop = 0;
		}


		k = 2;
	}
	else if (strcmp("weapon_down", cmd) == 0)
	{
		int spawn = find_type(ENT_PLAYER, 0);
		handled = true;

		netcode.sv_maxclients--;
		printf("frame %d\r\n", netcode.sv_maxclients);
		if (edit_ent != -1)
		{
			edit_prop--;
			if (edit_prop < 0)
				edit_prop = MAX_PROP - 1;
		}


		input.weapon_down = pressed;
		if (netcode.client_flag && spawn != -1)
		{
			entity_list[spawn]->player->change_weapon_down();
		}
		k = 1;
	}
	else if (strcmp("use", cmd) == 0)
	{
		handled = true;
		input.use = pressed;
		raster_target++;
		debugf("set to %d\r\n", raster_target);
	}
	else if (strcmp("zoom", cmd) == 0)
	{
		handled = true;
		input.zoom = pressed;

		k = 15;
	}
	else if (strcmp("duck", cmd) == 0)
	{
		handled = true;

		if (edit_ent != -1)
		{
			switch (edit_prop)
			{
			case PROP_POSITION:
				entity_list[edit_ent]->position.y -= 10.0f;
				break;
			case PROP_COLOR:
				entity_list[edit_ent]->light->color.y -= 0.1f;
				break;
			case PROP_INTENSITY:
				entity_list[edit_ent]->light->intensity -= 10.0f;
				break;
			}
		}
		else
		{
			input.duck = pressed;
		}
	}
	else if (strcmp("pickup", cmd) == 0)
	{
		handled = true;
		input.pickup = pressed;
	}
	else if (strcmp("walk", cmd) == 0)
	{
		handled = true;
		input.walk = pressed;
	}
	else if (strcmp("control", cmd) == 0)
	{
		handled = true;
		input.control = pressed;
	}
	else if (strcmp("escape", cmd) == 0)
	{
		handled = true;
		input.escape = pressed;
		k = 27;
	}
	else if (strcmp("scores", cmd) == 0)
	{
		handled = true;
		input.scores = pressed;
	}
	else if (strcmp("pgup", cmd) == 0)
	{
		handled = true;
		k = 11;
	}
	else if (strcmp("pgdown", cmd) == 0)
	{
		handled = true;
		k = 10;
	}

	if ((pressed && menu.console == false) || (menu.console == true && pressed && k == '`'))
		keystroke(k, key);

	if (handled == false && menu.console == false && menu.ingame == false && menu.stringmode == false)
		console(cmd);
}

///=============================================================================
/// Function: keystroke
///=============================================================================
/// Description: This function handles keyboard input, there are two types on input
/// real time up/down key events and higher level typing things for text input, this one
/// is the higher level events for things like console input, typing for chat, etc
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Engine::keystroke(char key, char *keystr)
{
	//printf("keystroke %c %s\r\n", key, keystr);

	if (q3map.loaded == false && hlmap.loaded == false && q1map.loaded == false)
	{
		menu.ingame = false;

		if (menu.console)
		{
			gfx.clear();
			menu.render(global);
			menu.handle_console(key, this);
			if (menu.console)
				menu.render_console(global);
			gfx.swap();
		}
		else
		{
			gfx.clear();

			if (menu.stringmode)
			{
				menu.handle_stringmode(key, this);
				if (menu.stringmode == false)
				{
					char cmd[512];

					sprintf(cmd, "%s \"%s\"", menu.string_cmd, menu.string_target);
					console(cmd);
				}
			}
			else
			{
				if (menu.bindnextkey)
				{
					char cmd[512];
					menu.bindnextkey = false;

					if (keystr)
					{
						sprintf(cmd, "bind %s %s", keystr, menu.bindcmd);
						console(cmd);
						sprintf(menu.bindstr, "%s", keystr);
					}
				}
				else
				{
					menu.handle(key, this);
				}
			}
			menu.render(global);
			if (menu.console)
			{
				menu.render_console(global);
			}
			if (menu.stringmode)
			{
				menu.render_stringmode(global);
			}

			gfx.swap();
		}
	}
	else
	{
		if (menu.console)
		{
			menu.handle_console(key, this);
		}
		else if (menu.ingame)
		{
			if (menu.bindnextkey)
			{
				char cmd[512];
				menu.bindnextkey = false;

				if (keystr)
				{
					sprintf(cmd, "bind %s %s", keystr, menu.bindcmd);
					console(cmd);
					sprintf(menu.bindstr, "%s", keystr);
				}
			}

			if (menu.stringmode)
			{
				menu.handle_stringmode(key, this);
				if (menu.stringmode == false)
				{
					char cmd[256] = { 0 };

					sprintf(cmd, "%s \"%s\"", menu.string_cmd, menu.string_target);
					console(cmd);
				}
			}
			else
			{
				menu.handle(key, this);
			}
		}
		else if (menu.chatmode)
		{
			menu.handle_chatmode(key, this);
		}
		else if (menu.stringmode)
		{
			menu.handle_stringmode(key, this);
		}
		else
		{
			handle_game(key);
		}
	}
}


///=============================================================================
/// Function: handle_game
///=============================================================================
/// Description: So this function is similar to keystroke, but handles the events
/// when a map is loaded and running (different handlers for different states,
/// are used to make keyboard input less of a branching mess
/// eg: if (menu.open) if (map.loaded)
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Engine::handle_game(char key)
{
	int spawn = find_type(ENT_PLAYER, 0);

	switch (key)
	{
	case '0':
		if (spawn != -1)
		{
			entity_list[spawn]->player->current_face = 0;
			console((char *)key_bind.find("num0"));
		}
		break;
	case '1':
		if (spawn != -1)
		{
			entity_list[spawn]->player->current_face = 1;
			console((char *)key_bind.find("num1"));
		}
		break;
	case '2':
		if (spawn != -1)
		{
			entity_list[spawn]->player->current_face = 2;
			console((char *)key_bind.find("num2"));
		}
		break;
	case '3':
		if (spawn != -1)
		{
			entity_list[spawn]->player->current_face = 3;
			console((char *)key_bind.find("num3"));
		}
		break;
	case '4':
		if (spawn != -1)
		{
			entity_list[spawn]->player->current_face = 4;
			console((char *)key_bind.find("num4"));
		}
		break;
	case '5':
		if (spawn != -1)
		{
			entity_list[spawn]->player->current_face = 5;
			console((char *)key_bind.find("num5"));
		}
		break;
	case '6':
		console((char *)key_bind.find("num6"));
		break;
	case '7':
		console((char *)key_bind.find("num7"));
		break;
	case '8':
		console((char *)key_bind.find("num8"));
		break;
	case '9':
		console((char *)key_bind.find("num9"));
		break;

	case '-':
		if (spawn != -1)
		{
			if (entity_list[spawn]->player)
			{
				if (entity_list[spawn]->player->current_light > 0)
					entity_list[spawn]->player->current_light--;
			}
		}
		break;
	case '=':
		if (spawn != -1)
		{
			if (entity_list[spawn]->player)
			{
				if (entity_list[spawn]->player->current_light < num_light)
					entity_list[spawn]->player->current_light++;
			}
		}
		break;

	case 27:
		console("togglemenu");
		break;
	default:
		{
			if (menu.console == false)
			{
				char skey[2];
				sprintf(skey, "%c", key);
				console((char *)key_bind.find(skey));
			}
		}
		break;
	}
}

///=============================================================================
/// Function: resize
///=============================================================================
/// Description: So this function handles resize events, when the window is resized
/// we have to reallocate frame buffers and set up our projection matrix again
/// and other legwork
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Engine::resize(int width, int height)
{
	xres = width;
	yres = height;


	gfx.resize(width, height);
	post.resize(width, height);
	sprintf(menu.data.window, "%dx%d", width, height);

	projection.perspective(fov, (float)width / height, zNear, zFar, inf);
//	projection.ortho(-width/2.0f, width/2.0f, -height/2.0f, height/2.0f, -2001.0f, 2001.0);


	fb_width = (unsigned int)(width * res_scale);
	fb_height = (unsigned int)(height * res_scale);

	unsigned int normal_depth;

	gfx.DeleteFrameBuffer(render_fbo, render_quad, render_depth);
	gfx.DeleteFrameBuffer(mask_fbo, mask_quad, mask_depth);
	gfx.DeleteFrameBuffer(blur1_fbo, blur1_quad, blur1_depth);
	gfx.DeleteFrameBuffer(blur2_fbo, blur2_quad, blur2_depth);
	gfx.DeleteFrameBuffer(ssao_fbo, ssao_quad, ssao_depth);

	gfx.CreateFramebuffer(fb_width, fb_height, render_fbo, render_quad, render_depth, render_ndepth, multisample, true);

	gfx.CreateFramebuffer(fb_width, fb_height, mask_fbo, mask_quad, mask_depth, normal_depth, 0, false);
	gfx.CreateFramebuffer(fb_width, fb_height, blur1_fbo, blur1_quad, blur1_depth, normal_depth, 0, false);
	gfx.CreateFramebuffer(fb_width, fb_height, blur2_fbo, blur2_quad, blur2_depth, normal_depth, 0, false);
	gfx.CreateFramebuffer(fb_width, fb_height, ssao_fbo, ssao_quad, ssao_depth, normal_depth, 0, false);


#ifdef __APPLE__
    gfx.swap();
#endif
#ifndef WIN32
#ifndef VULKAN
	// This should probably be in render
	if (initialized && q3map.loaded == false)
	{
		gfx.clear();
		menu.render(global);
		if (menu.console)
			menu.render_console(global);
		gfx.swap();
	}
#endif
#endif
}


///=============================================================================
/// Function: get_load_wave
///=============================================================================
/// Description: Used to load wave files from disk and be sure it wasn't already
/// loaded before (should probably rename function as it seems oddly named)
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
int Engine::get_load_wave(const char *file)
{
	wave_t wave;

	for (unsigned int j = 0; j < snd_wave.size(); j++)
	{
		if (strcmp(file, snd_wave[j].file) == 0)
		{
			return j;
		}
	}

	strcpy(wave.file, file);

	debugf("Loading wave file %s\n", file);
	audio.load(wave, pk3_list, num_pk3);
	snd_wave.push_back(wave);

	return snd_wave.size() - 1;
}


///=============================================================================
/// Function: load_models
///=============================================================================
/// Description: Used to load models defined by the map, loops through map
/// entities and loads models as needed
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Engine::load_models()
{
	if (entity_list.size() == 0)
		return;


	for(unsigned int i = max_dynamic; i < entity_list.size(); i++)
	{
		if (entity_list[i]->model == NULL)
			continue;

		game->map_model(entity_list[i]);
	}
}


///=============================================================================
/// Function: load_entities
///=============================================================================
/// Description: Loops through map entities and loads whats needed (models,
/// sounds etc)
///
/// Loads media that may be shared with multiple entities
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Engine::load_entities()
{
#ifndef DEDICATED
	int spawn = -1;
	if (netcode.client_flag == false)
		game->add_player(entity_list, PLAYER, spawn, "UnnamedPlayer");
#endif
//	load_sounds();
	create_sources();
	load_models();

	for(unsigned int i = 0; i < entity_list.size(); i++)
	{
		if (entity_list[i]->light)
			entity_list[i]->rigid->flags.gravity = false;
	}
}

///=============================================================================
/// Function: fullscreen
///=============================================================================
/// Description: makes the window fullscreen, which essentially just hides the widgets
/// and resizes the window to monitor resolution
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Engine::fullscreen()
{
	if (fullscreen_timer == 0)
	{
		menu.data.fullscreen = !menu.data.fullscreen;
#ifdef WIN32
		HMONITOR hmon;
		MONITORINFOEX  mi;
		HWND hwnd = *((HWND *)param1);

		mi.cbSize = sizeof(MONITORINFOEX);
		hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
		GetMonitorInfo(hmon, &mi);

		int xr = abs(mi.rcMonitor.right - mi.rcMonitor.left);
		int yr = abs(mi.rcMonitor.bottom - mi.rcMonitor.top);
		sprintf(menu.data.monitor, "%s", mi.szDevice);

		static LONG old_style;
		static LONG	new_style = WS_CHILD | WS_VISIBLE;

		old_style = SetWindowLongPtr(hwnd, GWL_STYLE, new_style);
		new_style = old_style;
		SetWindowPos(hwnd, HWND_TOP, 0, 0, xr, yr, 0);
#endif
#ifdef __linux__
		::fullscreen((Display *)param1, *((Window *)param2));
#endif
		fullscreen_timer = TICK_RATE;
	}
}


///=============================================================================
/// Function: clean_entity
///=============================================================================
/// Description: Free's an entity for reuse, say it expired (eg: rocket exploded)
/// this will free that entry for later use as entity counts are finite
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Engine::clean_entity(int index)
{
	unsigned int i = 0;

	//free audio sources
	if (entity_list[index]->trigger)
		entity_list[index]->trigger->destroy(audio);

	if (entity_list[index]->projectile)
		entity_list[index]->projectile->destroy(audio);

	if (entity_list[index]->speaker)
		entity_list[index]->speaker->destroy(audio);

	entity_list[index]->nettype = NET_NONE;

	entity_list[index]->flags.particle_on = false;

	if (entity_list[index]->construct)
	{
		entity_list[index]->construct->destroy();
		delete entity_list[index]->construct;
		entity_list[index]->construct = NULL;
	}

	if (entity_list[index]->rigid)
	{
		delete entity_list[index]->rigid;
		entity_list[index]->rigid = NULL;
		entity_list[index]->model = NULL;
	}

	if (entity_list[index]->model)
	{
		delete entity_list[index]->model;
		entity_list[index]->model = NULL;
	}



	// Light list wont be updated until the next step, so manually delete
	if (entity_list[index]->light)
	{
		for (i = 0; i < light_list.size(); i++)
		{
			if (light_list[i]->entity == entity_list[index])
			{
				light_list.erase(light_list.begin() + i);
			}
		}

		// light wasnt in light list
		if (i == light_list.size())
		{
			delete entity_list[index]->light;
			entity_list[index]->light = NULL;
		}

	}
}


///=============================================================================
/// Function: get_entity
///=============================================================================
/// Description: Get's a free entity for use for something
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
int Engine::get_entity()
{
	static unsigned int index = max_player;
	int looped = 0;

	while (1)
	{

		if (index == max_dynamic)
		{
			index = max_player;
			looped++;

			if (looped == 2)
			{
				debugf("Unable to find free dynamic entity\n");
				return -1;
			}
		}

		if (index >= entity_list.size())
			break;

		if (entity_list[index]->ent_type == ENT_UNKNOWN)
		{
			clean_entity(index);
			entity_list[index]->~Entity();
			return index++;
		}
		index++;
	}

	return max_dynamic - 1;
}


///=============================================================================
/// Function: find_type
///=============================================================================
/// Description: Find an entity of a specific type
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
int Engine::find_type(int ent_type, int skip)
{
	for (unsigned int i = 0; i < entity_list.size(); i++)
	{
		if (entity_list[i]->ent_type == ent_type)
		{
			if (skip == 0)
				return i;
			skip--;
		}
	}
	return -1;
}


///=============================================================================
/// Function: get_player
///=============================================================================
/// Description: Find the player (should only be one, may be none if spectating)
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
int Engine::get_player()
{
	static unsigned int index = 0;
	int looped = 0;

	while (1)
	{

		if (index == max_player)
		{
			index = 0;
			looped++;

			if (looped == 2)
			{
				debugf("Unable to find free player entity\n");
				break;
			}
		}

		if (entity_list[index]->entstring == NULL)
		{
			clean_entity(index);
			entity_list[index]->~Entity();
			return index++;
		}
		index++;
	}

	return max_player - 1;
}


///=============================================================================
/// Function: create_sources
///=============================================================================
/// Description: creates audio sources and associates them with sound buffers
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Engine::create_sources()
{
	// create and associate sources
	for(unsigned int i = max_dynamic; i < entity_list.size(); i++)
	{
		EntSpeaker *speaker = entity_list[i]->speaker;
		if (speaker != NULL)
		{
			entity_list[i]->rigid->flags.gravity = false;
			if (speaker->index  != -1)
			{
				audio.select_buffer(speaker->loop_source, snd_wave[entity_list[i]->speaker->index].buffer);
				audio.play(speaker->loop_source);
			}
		}
		else if (entity_list[i]->trigger != NULL)
		{
			entity_list[i]->rigid->flags.gravity = false;
		}
	}

	// position sources
	update_audio();
}

///=============================================================================
/// Function: update_audio
///=============================================================================
/// Description: updates the listeners position, can also be used to set things
/// like velocity for doppler effects
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Engine::update_audio()
{
	audio.listener_position((float *)&(camera_frame.pos.x));

	/*
	for(unsigned int i = 0; i < entity_list.size(); i++)
	{
		if (entity_list[i]->speaker)
		{
			audio.source_position(entity_list[i]->speaker->source, (float *)(&entity_list[i]->position));
			audio.source_position(entity_list[i]->speaker->loop_source, (float *)(&entity_list[i]->position));

			if (entity_list[i]->rigid)
			{
				audio.source_velocity(entity_list[i]->speaker->source, (float *)(&entity_list[i]->rigid->velocity));
				audio.source_velocity(entity_list[i]->speaker->loop_source, (float *)(&entity_list[i]->rigid->velocity));
			}
		}

		if (entity_list[i]->trigger)
		{
			audio.source_position(entity_list[i]->trigger->source, (float *)(&entity_list[i]->position));
			audio.source_position(entity_list[i]->trigger->loop_source, (float *)(&entity_list[i]->position));
			if (entity_list[i]->rigid)
			{
				audio.source_velocity(entity_list[i]->trigger->source, (float *)(&entity_list[i]->rigid->velocity));
				audio.source_velocity(entity_list[i]->trigger->loop_source, (float *)(&entity_list[i]->rigid->velocity));
			}
		}
	}
	*/

	int spawn = find_type(ENT_PLAYER, 0);

	if (spawn != -1 && entity_list.size())
	{
		if (entity_list[spawn]->rigid)
		{
			audio.listener_velocity((float *)&(entity_list[spawn]->rigid->velocity));
			audio.listener_orientation((float *)&(entity_list[spawn]->rigid->morientation.m));
		}
	}

}



///=============================================================================
/// Function: unload
///=============================================================================
/// Description: unloads a map and any associated allocated data / objects
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Engine::unload()
{
	print_entity_meminfo(entity_list);

	if (q3map.loaded == false)
		return;

	if (game)
	{
		game->unload();
	}

	num_bot = 0;

	if (node)
	{
		delete[] node;
		node = NULL;
	}

	if (ref)
	{
		delete[] ref;
		ref = NULL;
	}


	for(unsigned int i = 0; i < cloth.size(); i++)
	{
		delete cloth[i];
	}

	cloth.clear();

	menu.ingame = false;
	for(unsigned int i = 0; i < entity_list.size(); i++)
	{
		if (entity_list[i]->speaker)
			entity_list[i]->speaker->destroy(audio);

		if (entity_list[i]->trigger)
			entity_list[i]->trigger->destroy(audio);

		if (entity_list[i]->light)
			entity_list[i]->light->destroy(gfx);

		delete entity_list[i];
	}
	entity_list.clear();
	light_list.clear();

//	light_list.~List();
//	entity_list~List();

	post.destroy();
	mlight2.destroy();
//	mlight3.destroy();
	q3map.unload(gfx);
	menu.play();
	menu.delta("unload", *this);
	menu.render(global);
	gfx.swap();

	if (netcode.server_flag)
	{
		while (netcode.client_list.size())
		{
			netcode.kick(0);
		}
		netcode.client_list.clear();
		netcode.sock.closesock();
		netcode.server_flag = false;
	}

	if (netcode.client_flag)
	{
		netcode.disconnect();
		netcode.client_flag = false;
	}

	menu.console = false;
	netcode.client_flag = 0;
	netcode.server_flag = 0;
}


///=============================================================================
/// Function: destroy
///=============================================================================
/// Description: unloads anything else before closing application
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Engine::destroy()
{
	if (netcode.recording_demo)
	{
		console("stop");
	}

#ifdef OCULUS
	ovr.destroy();
#endif

	delete thug22;
	gfx.DeleteIndexBuffer(q3map.skybox_ibo);
	gfx.DeleteVertexBuffer(q3map.skybox_vbo);

	if (wad)
		delete[] wad;

	if (pk3list)
		delete[] pk3list;
	if (cmdlist)
		delete[] cmdlist;
	if (hashlist)
		delete[] hashlist;
	if (hacklist)
		delete[] hacklist;
	if (masterlist)
		delete[] masterlist;

	delete[] animation;

	game->destroy();
	delete game;
	game = NULL;
	debugf("Shutting down.\n");

	for (unsigned int i = 0; i < surface_list.size(); i++)
	{
		delete surface_list[i];
	}
	surface_list.clear();

	printf("destroying buffers\n");
	destroy_buffers();
	printf("unloading\n");
	unload();
	gfx.GetDebugLog();
	printf("Destroying gfx\n");
	gfx.destroy();
	printf("Destroying audio\n");
	audio.capture_stop();
	audio.destroy();
#ifdef VOICECHAT
	voice.destroy();
#endif

#ifdef DEBUG_FILELOAD
	print_file();
#endif

    printf("quit\n\n");
	quit();
}


///=============================================================================
/// Function: quit
///=============================================================================
/// Description: Posts quit message to cause application to exit any message
/// loops then end execution
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Engine::quit()
{
#ifdef _WINDOWS_
	HWND hwnd = *((HWND *)param1);
	PostMessage(hwnd, WM_CLOSE, 0, 0);
#else
	exit(0);
#endif
}




///=============================================================================
/// Function: console_general
///=============================================================================
/// Description: Part of the command console (~ tilde) some console commands
/// require a player to exist, some don't, these are the commands that will
/// work without any player present
///
/// TODO: Make something like quake's add command, cmd + function pointer to execute with it
/// Also logically seperate Console variables (Cvars) from commands
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
int Engine::console_general(char *cmd)
{
	char msg[512] = { 0 };
	char data[256] = { 0 };
	int ret;


	if (strcmp(cmd, "console") == 0)
	{
		menu.console = !menu.console;
		return 0;
	}



	if (sscanf(cmd, "toggle %s", &data[0]))
	{
		if (strcmp(data, "r_stencil") == 0)
		{
			enable_stencil = !enable_stencil;
			menu.data.shadowvol = enable_stencil;
			return 0;
		}
		else if (strcmp(data, "r_ssao") == 0)
		{
			console("r_ssao");
			menu.data.ssao = enable_ssao;
			return 0;
		}
		else if (strcmp(data, "r_skyray") == 0)
		{
			menu.data.skyray = !menu.data.skyray;
			return 0;
		}
		else if (strcmp(data, "r_bloom") == 0)
		{
			enable_bloom = !enable_bloom;
			menu.data.bloom = enable_bloom;
			return 0;
		}
		else if (strcmp(data, "r_portal") == 0)
		{
			enable_portal = !enable_portal;
			menu.data.portal = enable_portal;
			return 0;
		}
		else if (strcmp(data, "r_shadowmaps") == 0)
		{
			console("r_shadowmaps");
			menu.data.shadowmaps = !menu.data.shadowmaps;
			return 0;
		}
		else if (strcmp(data, "r_fullscreen") == 0)
		{
			fullscreen();
			return 0;
		}
		else if (strcmp(data, "r_antialias") == 0 && strstr(cmd, "up"))
		{
			char cmd[128];

			if (menu.data.antialias == 0)
				menu.data.antialias = 2;
			else
				menu.data.antialias = menu.data.antialias << 1;
			if (menu.data.antialias > 16)
				menu.data.antialias = 0;


			sprintf(cmd, "r_multisample %d", menu.data.antialias);
			console(cmd);
			return 0;
		}
		else if (strcmp(data, "r_antialias") == 0 && strstr(cmd, "down"))
		{
			char cmd[128];

			if (menu.data.antialias == 0)
				menu.data.antialias = 16;
			else
				menu.data.antialias = menu.data.antialias >> 1;

			if (menu.data.antialias == 1)
				menu.data.antialias = 0;

			sprintf(cmd, "r_multisample %d", menu.data.antialias);
			console(cmd);
			return 0;
		}
		else if (strcmp(data, "r_anisotropic") == 0 && strstr(cmd, "up"))
		{
			if (menu.data.anisotropic == 0)
				menu.data.anisotropic = 2;
			else
				menu.data.anisotropic = menu.data.anisotropic << 1;
			if (menu.data.anisotropic > 16)
				menu.data.anisotropic = 0;

			if (menu.data.anisotropic == 1)
				menu.data.anisotropic = 0;

			return 0;
		}
		else if (strcmp(data, "r_anisotropic") == 0 && strstr(cmd, "down"))
		{
			if (menu.data.anisotropic == 0)
				menu.data.anisotropic = 16;
			else
				menu.data.anisotropic = menu.data.anisotropic >> 1;
			return 0;
		}
		else if (strcmp(data, "r_brightness") == 0 && strstr(cmd, "up"))
		{
			menu.data.brightness += 0.1f;
			if (menu.data.brightness > 1.001f)
				menu.data.brightness = 0.0f;
			mlight2.set_exposure(2.0f * menu.data.brightness);
			return 0;
		}
		else if (strcmp(data, "r_brightness") == 0 && strstr(cmd, "down"))
		{
			menu.data.brightness -= 0.1f;
			if (menu.data.brightness < 0.0f)
				menu.data.brightness = 1.0f;
			mlight2.set_exposure(2.0f * menu.data.brightness);
			return 0;
		}
		else if (strcmp(data, "r_contrast") == 0 && strstr(cmd, "up"))
		{
			menu.data.contrast += 0.1f;
			if (menu.data.contrast > 1.001f)
				menu.data.contrast = 0.0f;
			mlight2.set_contrast(2.0f * menu.data.contrast + 1.0f);
			return 0;
		}
		else if (strcmp(data, "r_contrast") == 0 && strstr(cmd, "down"))
		{
			menu.data.contrast -= 0.1f;
			if (menu.data.contrast < 0.0f)
				menu.data.contrast = 1.0f;
			mlight2.set_contrast(2.0f * menu.data.contrast + 1.0f);
			return 0;
		}
		else if (strcmp(data, "r_rscale") == 0 && strstr(cmd, "up"))
		{
			menu.data.rscale += 0.1f;
			if (menu.data.rscale > 1.0f)
				menu.data.rscale = 0.1f;
			sprintf(data, "res_scale %f", 2.0f * menu.data.rscale);
			console(data);
			return 0;
		}
		else if (strcmp(data, "r_rscale") == 0 && strstr(cmd, "down"))
		{
			menu.data.rscale -= 0.1f;
			if (menu.data.rscale < 0.01f)
				menu.data.rscale = 1.0f;
			sprintf(data, "res_scale %f", 2.0f * menu.data.rscale);
			console(data);
			return 0;
		}
		else if (strcmp(data, "r_res") == 0 && strstr(cmd, "up"))
		{
			current_res++;
			if (current_res + 1 > num_res - 1)
				current_res = 0;
			sprintf(menu.data.resolution, "%s", resbuf[current_res + 1]);

			sprintf(menu.data.apply, "Apply");
			return 0;
		}
		else if (strcmp(data, "r_res") == 0 && strstr(cmd, "down"))
		{
			current_res--;
			if (current_res < 0)
				current_res = num_res - 2;
			sprintf(menu.data.resolution, "%s", resbuf[current_res + 1]);

			sprintf(menu.data.apply, "Apply");
			return 0;
		}
		else if (strcmp(data, "cg_crosshair") == 0 && strstr(cmd, "up"))
		{
			char cmd[128];

			menu.data.crosshair++;
			if (menu.data.crosshair > 10)
				menu.data.crosshair = 10;

			sprintf(cmd, "cg_crosshair %d", menu.data.crosshair);
			console(cmd);
			return 0;
		}
		else if (strcmp(data, "cg_crosshair") == 0 && strstr(cmd, "down"))
		{
			char cmd[128];

			menu.data.crosshair--;
			if (menu.data.crosshair < -1)
				menu.data.crosshair = -1;

			sprintf(cmd, "cg_crosshair %d", menu.data.crosshair);
			console(cmd);
			return 0;
		}
		else if (strcmp(data, "in_mouse") == 0 && strstr(cmd, "up"))
		{
			char cmd[128];

			menu.data.mousemode++;
			if (menu.data.mousemode > 1)
				menu.data.mousemode = 1;

			sprintf(cmd, "in_mouse %d", menu.data.mousemode);
			console(cmd);
			return 0;
		}
		else if (strcmp(data, "in_mouse") == 0 && strstr(cmd, "down"))
		{
			char cmd[128];

			menu.data.mousemode--;
			if (menu.data.mousemode < 0)
				menu.data.mousemode = 0;

			sprintf(cmd, "in_mouse %d", menu.data.mousemode);
			console(cmd);
			return 0;
		}
		else if (strcmp(data, "r_vsync") == 0 && strstr(cmd, "up"))
		{
			menu.data.vsync++;
			if (menu.data.vsync >= 3)
				menu.data.vsync = 0;
#ifdef WIN32
#ifndef DEDICATED
#ifdef OPENGL
			if (menu.data.vsync == 0)
				wglSwapIntervalEXT(0);
			else if (menu.data.vsync == 1)
				wglSwapIntervalEXT(1);
			else if (menu.data.vsync == 2)
				wglSwapIntervalEXT(-1); //adaptive vsync (disables stupid divide crap)
#endif
#endif
#endif
#ifdef __linux
										//			if (menu.data.vsync == 0)
										//	glXSwapIntervalExt(0); //adaptive vsync (disables stupid divide crap)
										//			else if (menu.data.vsync == 1)
										//	glXSwapIntervalExt(1);
										//			else if (menu.data.vsync == 2)
										//	glXSwapIntervalExt(-1);
#endif
			return 0;
		}
		else if (strcmp(data, "r_vsync") == 0 && strstr(cmd, "down"))
		{
			menu.data.vsync--;
			if (menu.data.vsync < 0)
				menu.data.vsync = 2;
#ifdef WIN32
#ifndef DEDICATED
#ifdef OPENGL
			if (menu.data.vsync == 0)
				wglSwapIntervalEXT(0);
			else if (menu.data.vsync == 1)
				wglSwapIntervalEXT(1);
			else if (menu.data.vsync == 2)
				wglSwapIntervalEXT(-1); //adaptive vsync (disables stupid divide crap)
#endif
#endif
#endif
#ifdef __linux
										//			if (menu.data.vsync == 0)
										//glXSwapIntervalExt(0); //adaptive vsync (disables stupid divide crap)
										//			else if (menu.data.vsync == 1)
										//glXSwapIntervalExt(1);
										//			else if (menu.data.vsync == 2)
										//glXSwapIntervalExt(-1);
#endif
			return 0;
		}
		else if (strcmp(data, "in_invert") == 0)
		{
			menu.data.invert = !menu.data.invert;
			return 0;
		}
		else if (strcmp(data, "s_volume") == 0 && strstr(cmd, "up"))
		{
			menu.data.volume += 0.1f;
			if (menu.data.volume > 1.01f)
				menu.data.volume = 0.0f;

#ifdef WIN32
#ifndef DEDICATED
			waveOutSetVolume(hWaveOut, (int)(menu.data.volume * 65535));
#endif
#endif
			return 0;
		}
		else if (strcmp(data, "s_volume") == 0 && strstr(cmd, "down"))
		{
			menu.data.volume -= 0.1f;
			if (menu.data.volume < 0.00f)
				menu.data.volume = 1.0f;

#ifdef WIN32
#ifndef DEDICATED
			waveOutSetVolume(hWaveOut, (int)(menu.data.volume * 65535));
#endif
#endif
			return 0;
		}
		else if (strcmp(data, "s_musicvol") == 0 && strstr(cmd, "up"))
		{
			menu.data.musicvol += 0.1f;
			if (menu.data.musicvol > 1.01f)
				menu.data.musicvol = 0.0f;
			return 0;
		}
		else if (strcmp(data, "s_musicvol") == 0 && strstr(cmd, "down"))
		{
			menu.data.musicvol -= 0.1f;
			if (menu.data.musicvol < 0.00f)
				menu.data.musicvol = 1.0f;
			return 0;
		}
		else if (strcmp(data, "s_sfxvol") == 0 && strstr(cmd, "up"))
		{
			menu.data.sfxvol += 0.1f;
			if (menu.data.sfxvol > 1.01f)
				menu.data.sfxvol = 0.0f;
			return 0;
		}
		else if (strcmp(data, "s_sfxvol") == 0 && strstr(cmd, "down"))
		{
			menu.data.sfxvol -= 0.1f;
			if (menu.data.sfxvol < 0.00f)
				menu.data.sfxvol = 1.0f;
			return 0;
		}
		else if (strcmp(data, "cg_model") == 0 && strstr(cmd, "up"))
		{
			current_model++;
			if (current_model >= num_model)
				current_model = 0;
			sprintf(menu.data.model, "%s", EntPlayer::models[current_model]);

			sprintf(menu.data.apply, "Apply");
			return 0;
		}
		else if (strcmp(data, "cg_model") == 0 && strstr(cmd, "down"))
		{
			current_model--;
			if (current_model < 0)
				current_model = num_model - 1;
			sprintf(menu.data.model, "%s", EntPlayer::models[current_model]);
			return 0;
		}
		else if (strcmp(data, "cg_team") == 0 && strstr(cmd, "up"))
		{
			current_team++;
			if (current_team >= num_team)
				current_team = 0;
			sprintf(menu.data.team, "%s", teams[current_team]);

			sprintf(menu.data.apply, "Apply");
			return 0;
		}
		else if (strcmp(data, "cg_team") == 0 && strstr(cmd, "down"))
		{
			current_team--;
			if (current_team < 0)
				current_team = num_team - 1;
			sprintf(menu.data.team, "%s", teams[current_team]);
			return 0;
		}
		else if (strcmp(data, "cg_fov") == 0 && strstr(cmd, "up"))
		{
			menu.data.fov += 0.1f;
			if (menu.data.fov > 1.01f)
				menu.data.fov = 0.0f;

			sprintf(data, "cg_fov %d", (int)(100 * menu.data.fov + 90));
			console(data);
			return 0;
		}
		else if (strcmp(data, "cg_fov") == 0 && strstr(cmd, "down"))
		{
			menu.data.fov -= 0.1f;
			if (menu.data.fov < 0.00f)
				menu.data.fov = 1.0f;

			sprintf(data, "cg_fov %d", (int)(100 * menu.data.fov + 90));
			console(data);
			return 0;
		}
		else if (strcmp(data, "sensitivity") == 0 && strstr(cmd, "up"))
		{
			menu.data.sensitivity += 0.1f;
			if (menu.data.sensitivity > 1.01f)
				menu.data.sensitivity = 0.0f;

			sprintf(data, "sensitivity %lf", 2.5 * menu.data.sensitivity);
			console(data);
			return 0;
		}
		else if (strcmp(data, "sensitivity") == 0 && strstr(cmd, "down"))
		{
			menu.data.sensitivity -= 0.1f;
			if (menu.data.sensitivity < 0.00f)
				menu.data.sensitivity = 1.0f;

			sprintf(data, "sensitivity %lf", 2.5 * menu.data.sensitivity);
			console(data);
			return 0;
		}
		else if (strcmp(data, "name") == 0 && strstr(cmd, "string"))
		{
			menu.stringmode = true;
			menu.string_target = &(menu.data.name[0]);
			strcpy(menu.string_cmd, "name");
			return 0;
		}
		else if (strcmp(data, "bind") == 0 && strstr(cmd, "attack"))
		{
			menu.bindnextkey = true;
			sprintf(menu.bindcmd, "attack");
			sprintf(menu.data.attack, "???");
			menu.bindstr = &menu.data.attack[0];
			return 0;
		}
		else if (strcmp(data, "bind") == 0 && strstr(cmd, "zoom"))
		{
			menu.bindnextkey = true;
			sprintf(menu.bindcmd, "zoom");
			sprintf(menu.data.zoom, "???");
			menu.bindstr = &menu.data.zoom[0];
			return 0;
		}
		else if (strcmp(data, "bind") == 0 && strstr(cmd, "jump"))
		{
			menu.bindnextkey = true;
			sprintf(menu.bindcmd, "jump");
			sprintf(menu.data.jump, "???");
			menu.bindstr = &menu.data.jump[0];
			return 0;
		}
		else if (strcmp(data, "bind") == 0 && strstr(cmd, "duck"))
		{
			menu.bindnextkey = true;
			sprintf(menu.bindcmd, "duck");
			sprintf(menu.data.duck, "???");
			menu.bindstr = &menu.data.duck[0];
			return 0;
		}
		else if (strcmp(data, "bind") == 0 && strstr(cmd, "use"))
		{
			menu.bindnextkey = true;
			sprintf(menu.bindcmd, "use");
			sprintf(menu.data.use, "???");
			menu.bindstr = &menu.data.use[0];
			return 0;
		}
		else if (strcmp(data, "bind") == 0 && strstr(cmd, "moveleft"))
		{
			menu.bindnextkey = true;
			sprintf(menu.bindcmd, "moveleft");
			sprintf(menu.data.moveleft, "???");
			menu.bindstr = &menu.data.moveleft[0];
			return 0;
		}
		else if (strcmp(data, "bind") == 0 && strstr(cmd, "moveright"))
		{
			menu.bindnextkey = true;
			sprintf(menu.bindcmd, "moveright");
			sprintf(menu.data.moveright, "???");
			menu.bindstr = &menu.data.moveright[0];
			return 0;
		}
		else if (strcmp(data, "bind") == 0 && strstr(cmd, "moveforward"))
		{
			menu.bindnextkey = true;
			sprintf(menu.bindcmd, "moveforward");
			sprintf(menu.data.moveforward, "???");
			menu.bindstr = &menu.data.moveforward[0];
			return 0;
		}
		else if (strcmp(data, "bind") == 0 && strstr(cmd, "moveback"))
		{
			menu.bindnextkey = true;
			sprintf(menu.bindcmd, "moveback");
			sprintf(menu.data.moveback, "???");
			menu.bindstr = &menu.data.moveback[0];
			return 0;
		}

		else if (strcmp(data, "bind") == 0 && strstr(cmd, "weapnext"))
		{
			menu.bindnextkey = true;
			sprintf(menu.bindcmd, "weapnext");
			sprintf(menu.data.weapnext, "???");
			menu.bindstr = &menu.data.weapnext[0];
			return 0;
		}
		else if (strcmp(data, "bind") == 0 && strstr(cmd, "weapprev"))
		{
			menu.bindnextkey = true;
			sprintf(menu.bindcmd, "weapprev");
			sprintf(menu.data.weapprev, "???");
			menu.bindstr = &menu.data.weapprev[0];
			return 0;
		}
		else if (strcmp(data, "bind") == 0 && strstr(cmd, "walk"))
		{
			menu.bindnextkey = true;
			sprintf(menu.bindcmd, "walk");
			sprintf(menu.data.walk, "???");
			menu.bindstr = &menu.data.walk[0];
			return 0;
		}
		else if (strcmp(data, "bind") == 0 && strstr(cmd, "spectate"))
		{
			menu.bindnextkey = true;
			sprintf(menu.bindcmd, "spectate");
			sprintf(menu.data.spectate, "???");
			menu.bindstr = &menu.data.spectate[0];
			return 0;
		}
		else if (strcmp(data, "bind") == 0 && strstr(cmd, "console"))
		{
			menu.bindnextkey = true;
			sprintf(menu.bindcmd, "console");
			sprintf(menu.data.console, "???");
			menu.bindstr = &menu.data.console[0];
			return 0;
		}
		else if (strcmp(data, "bind") == 0 && strstr(cmd, "menu"))
		{
			menu.bindnextkey = true;
			sprintf(menu.bindcmd, "togglemenu");
			sprintf(menu.data.menu, "???");
			menu.bindstr = &menu.data.menu[0];
			return 0;
		}
		else if (strcmp(data, "bind") == 0 && strstr(cmd, "scores"))
		{
			menu.bindnextkey = true;
			sprintf(menu.bindcmd, "scores");
			sprintf(menu.data.scores, "???");
			menu.bindstr = &menu.data.scores[0];
			return 0;
		}
	}

	if (strstr(cmd, "oldschool"))
	{
#ifdef WIN32
		set_resolution(640, 480, 32);
#endif
		fullscreen();
		return 0;
	}

	if (strstr(cmd, "centerview"))
	{
		snprintf(msg, LINE_SIZE, "centerview");
		menu.print(msg);
		camera_frame.reset();
		return 0;
	}

	if (strcmp(cmd, "quat") == 0)
	{
		matrix3 mat;
		vec3 right = vec3::crossproduct(camera_frame.up, camera_frame.forward);

		mat.m[0] = right.x;
		mat.m[1] = right.y;
		mat.m[2] = right.z;

		mat.m[3] = camera_frame.up.x;
		mat.m[4] = camera_frame.up.y;
		mat.m[5] = camera_frame.up.z;

		mat.m[6] = camera_frame.forward.x;
		mat.m[7] = camera_frame.forward.y;
		mat.m[8] = camera_frame.forward.z;


		quaternion q;

		q.to_quat(mat);

		snprintf(msg, LINE_SIZE, "pos [%f, %f, %f] quat [%f, i %f, j %f, k %f]\n",
			camera_frame.pos.x,
			camera_frame.pos.y,
			camera_frame.pos.z,
			q.s,
			q.x,
			q.y,
			q.z
		);
		debugf(msg);
		return 0;
	}


	if (strstr(cmd, "talk"))
	{
		menu.chatmode = true;
		return 0;
	}

	if (strstr(cmd, "talkteam"))
	{
		menu.chatmode = true;
		return 0;
	}

	if (strcmp(cmd, "togglemenu") == 0)
	{
		if (ingame_menu_timer == 0)
			menu.ingame = !menu.ingame;
		ingame_menu_timer = TICK_RATE >> 4;
		return 0;
	}

	if (strstr(cmd, "voteyes"))
	{
		if (voted == false)
		{
			snprintf(msg, LINE_SIZE, "voteyes");
			menu.print(msg);
			vote_yes++;
			voted = true;
		}
		return 0;
	}

	if (strstr(cmd, "voteno"))
	{
		if (voted == false)
		{
			snprintf(msg, LINE_SIZE, "voteno");
			menu.print(msg);
			vote_no++;
			voted = true;
		}
		return 0;
	}

	if (strcmp(cmd, "screenshot") == 0)
	{
		unsigned int luminance = 0;
		screenshot(luminance, false);
		snprintf(msg, LINE_SIZE, "screenshot taken with luminance average %d", luminance);
		menu.print(msg);
		return 0;
	}

	ret = sscanf(cmd, "echo %s", data);
	if (ret == 1)
	{
		snprintf(msg, LINE_SIZE, "%s\n", data);
		menu.print(msg);
		return 0;
	}


	ret = sscanf(cmd, "clip %s", data);
	if (ret == 1)
	{
		snprintf(msg, LINE_SIZE, "clip %s\n", data);
		menu.print(msg);
		debugf(msg);
#ifdef SOFTWARE
		gfx.clip(atoi(data));
#endif
		return 0;
	}

	ret = sscanf(cmd, "s_effect %s", data);
	if (ret == 1)
	{
		snprintf(msg, LINE_SIZE, "s_effect %s\n", data);
		menu.print(msg);
		debugf(msg);
		audio.set_effect(atoi(data));

		for (int i = 0; i < max_sources; i++)
		{
			audio.effects(audio_source[i], filter, filter_index);
			audio.effects(global_source[i], filter, filter_index);
			audio.effects(audio_loop_source[i], filter, filter_index);
			audio.effects(global_loop_source[i], filter, filter_index);

		}
		return 0;
	}

	ret = sscanf(cmd, "s_filter_index %s", data);
	if (ret == 1)
	{
		snprintf(msg, LINE_SIZE, "s_filter_index %s\n", data);
		menu.print(msg);
		debugf(msg);

		filter_index = atoi(data);

		for (int i = 0; i < max_sources; i++)
		{
			audio.effects(audio_source[i], filter, filter_index);
			audio.effects(global_source[i], filter, filter_index);
			audio.effects(audio_loop_source[i], filter, filter_index);
			audio.effects(global_loop_source[i], filter, filter_index);
		}
		return 0;
	}

	ret = sscanf(cmd, "s_filter %s", data);
	if (ret == 1)
	{
		snprintf(msg, LINE_SIZE, "s_filter %s\n", data);
		menu.print(msg);
		debugf(msg);

		filter = atoi(data);

		for (int i = 0; i < max_sources; i++)
		{
			audio.effects(audio_source[i], filter, filter_index);
			audio.effects(global_source[i], filter, filter_index);
			audio.effects(audio_loop_source[i], filter, filter_index);
			audio.effects(global_loop_source[i], filter, filter_index);

		}
		return 0;
	}



	ret = sscanf(cmd, "map %s", data);
	if (ret == 1)
	{
		unload();
		snprintf(msg, LINE_SIZE, "Loading %s\n", data);
		menu.print(msg);
		load(data);
		return 0;
	}

	ret = sscanf(cmd, "record %s", data);
	if (ret == 1)
	{

		if (q3map.loaded)
		{
			snprintf(msg, LINE_SIZE, "recording");
			menu.print(msg);
			demo_fileheader_t header;
			menu.print(msg);
			netcode.demofile = fopen(data, "wb");

			memcpy(header.magic, "demo", 5);
			memcpy(header.map, q3map.map_name, 64);
			fwrite(&header, sizeof(demo_fileheader_t), 1, netcode.demofile);
			netcode.recording_demo = true;
		}
		else
		{
			menu.print("Load map first");
		}
		return 0;
	}

	ret = sscanf(cmd, "play %s", data);
	if (ret == 1)
	{

		if (q3map.loaded)
		{
			menu.print("unload map first");
		}
		else
		{
			snprintf(msg, LINE_SIZE, "playing");
			menu.print(msg);
			demo_fileheader_t header;

			netcode.demofile = fopen(data, "rb");
			if (netcode.demofile == NULL)
			{
				menu.print("Unable to open demofile");
				return 0;
			}

			int ret = fread(&header, sizeof(demo_fileheader_t), 1, netcode.demofile);
			if (ret != sizeof(demo_fileheader_t))
			{
				menu.print("Unable to open demofile");
				return 0;
			}
			netcode.playing_demo = true;
			load(header.map);
		}
		return 0;
	}


	ret = sscanf(cmd, "save %s", data);
	if (ret == 1)
	{
		savegame(data);
		return 0;
	}

	ret = sscanf(cmd, "load %s", data);
	if (ret == 1)
	{
		loadgame(data);
		return 0;
	}

	if (strstr(cmd, "stop"))
	{
		if (netcode.recording_demo)
		{
			snprintf(msg, LINE_SIZE, "stopping playeback");
			menu.print(msg);
			fclose(netcode.demofile);
			netcode.recording_demo = false;
		}
		else
		{
			snprintf(msg, LINE_SIZE, "not recording");
			menu.print(msg);
		}
		return 0;
	}

	if (strstr(cmd, "maps") != 0)
	{
#define LIST_SIZE (1024 * 512)
		char *filelist = new char[LIST_SIZE];
		char *line = NULL;

		memset(filelist, 0, LIST_SIZE);
		for (unsigned int i = 0; i < num_pk3; i++)
		{
			list_zipfile(pk3_list[i], &filelist[0]);

			line = strtok(filelist, "\n");
			while (line)
			{
				if (strstr(line, ".bsp") != NULL)
				{
					debugf("map %s", line);
				}
				line = strtok(NULL, "\n");
			}
			memset(filelist, 0, LIST_SIZE);
		}

		delete[] filelist;
		return 0;
	}

	if (strcmp(cmd, "clear") == 0)
	{
		menu.clear_console();
		return 0;
	}

	if (strcmp(cmd, "fullscreen") == 0)
	{
		snprintf(msg, LINE_SIZE, "fullscreen");
		menu.print(msg);
		fullscreen();
		return 0;
	}

	ret = sscanf(cmd, "in_controller %s", (char *)data);
	if (ret == 1)
	{
		int num = atoi(data);

		if (num < 4 && num >= 0)
		{
			debugf("setting controller index to %d", num);
			controller = num;
		}
		else
		{
			debugf("Invalid input");
		}
		return 0;
	}

	if (strstr(cmd, "lookforward"))
	{
		vec3 right(-1.0f, 0.0f, 0.0f);
		vec3 up(0.0f, 1.0f, 0.0f);
		vec3 forward(0.0f, 0.0f, 1.0f);

		camera_frame.forward = forward;
		camera_frame.up = up;
		snprintf(msg, LINE_SIZE, "lookforward");
		menu.print(msg);
		return 0;
	}

	if (strstr(cmd, "lookdown"))
	{
		vec3 right(1.0f, 0.0f, 0.0f);
		vec3 up(0.0f, 0.0f, 1.0f);
		vec3 forward(0.0f, 1.0f, 0.0f);

		camera_frame.forward = forward;
		camera_frame.up = up;
		snprintf(msg, LINE_SIZE, "lookdown");
		menu.print(msg);
		return 0;
	}

	if (strstr(cmd, "lookright"))
	{
		vec3 right(0.0f, 0.0f, 1.0f);
		vec3 up(0.0f, 1.0f, 0.0f);
		vec3 forward(1.0f, 0.0f, 0.0f);

		camera_frame.forward = forward;
		camera_frame.up = up;
		snprintf(msg, LINE_SIZE, "lookright");
		menu.print(msg);
		return 0;
	}


	if (strstr(cmd, "lookleft"))
	{
		vec3 right(0.0f, 0.0f, -1.0f);
		vec3 up(0.0f, 1.0f, 0.0f);
		vec3 forward(-1.0f, 0.0f, 0.0f);

		camera_frame.forward = forward;
		camera_frame.up = up;
		snprintf(msg, LINE_SIZE, "lookleft");
		menu.print(msg);
		return 0;
	}

	if (strstr(cmd, "lookback"))
	{
		vec3 right(1.0f, 0.0f, 0.0f);
		vec3 up(0.0f, 1.0f, 0.0f);
		vec3 forward(0.0f, 0.0f, -1.0f);

		camera_frame.forward = forward;
		camera_frame.up = up;
		snprintf(msg, LINE_SIZE, "lookback");
		menu.print(msg);
		return 0;
	}

	if (strstr(cmd, "lookup"))
	{
		vec3 right(1.0f, 0.0f, 0.0f);
		vec3 up(0.0f, 0.0f, -1.0f);
		vec3 forward(0.0f, -1.0f, 0.0f);


		camera_frame.forward = forward;
		camera_frame.up = up;
		snprintf(msg, LINE_SIZE, "lookup");
		menu.print(msg);
		return 0;
	}


	if (strstr(cmd, "movelight"))
	{
		if (shadowmaps)
		{
			Entity *entity = entity_list[shadow_light];
			Entity *player = entity_list[find_type(ENT_PLAYER, 0)];

			debugf("Moving shadow light to player position %3.3f %3.3f %3.3f\n",
				entity->position.x,
				entity->position.y,
				entity->position.z);
			entity->position = player->position;
		}
		return 0;
	}

	if (sscanf(cmd, "g_collision %s", data) == 1)
	{
		if (atoi(data))
		{
			collision_detect_enable = true;
			snprintf(msg, LINE_SIZE, "Enabling rigid body dynamics");
			menu.print(msg);
		}
		else
		{
			snprintf(msg, LINE_SIZE, "Disabling rigid body dynamics");
			menu.print(msg);
			collision_detect_enable = false;
		}
		return 0;
	}



	if (strcmp(cmd, "sensitivity") == 0)
	{
		debugf("Mouse sensitivity is %f\n", sensitivity);
		return 0;
	}

	if (sscanf(cmd, "sensitivity %s", data) == 1)
	{
		snprintf(msg, LINE_SIZE, "Setting mouse sensitivity to %3.3f\n", atof(data));
		menu.print(msg);
		sensitivity = (float)atof(data);
		return 0;
	}

#ifdef WIN32
	if (strcmp(cmd, "in_mouse") == 0)
	{
		if (raw_mouse == false)
			debugf("Mouse mode is WM_MOUSEMOVE\n");
		else
			debugf("Mouse mode is WM_INPUT (raw mouse input)\n");
		return 0;
	}

	if (sscanf(cmd, "in_mouse %s", data) == 1)
	{
		int mode = atoi(data);

		if (mode == 0)
		{
			snprintf(msg, LINE_SIZE, "Setting mouse input to WM_MOUSEMOVE\n");
			menu.print(msg);
			raw_mouse = false;
			unregister_raw_mouse(NULL);
		}
		else if (mode == 1)
		{
			snprintf(msg, LINE_SIZE, "Setting mouse input to WM_INPUT (raw mouse input)\n");
			menu.print(msg);
			raw_mouse = true;
			register_raw_mouse(*((HWND *)param1));
		}
		return 0;
	}
#endif

	ret = sscanf(cmd, "bind %s %s", data, msg);
	if (ret == 2)
	{
		if (key_bind.update(data, strstr(cmd, msg)))
		{
			snprintf(msg, LINE_SIZE, "binding key");
			menu.print(msg);
			return 0;
		}
		key_bind.insert(data, strstr(cmd, msg));
		snprintf(msg, LINE_SIZE, "binding key");
		menu.print(msg);
		return 0;
	}

	ret = strcmp(cmd, "quit");
	if (ret == 0)
	{
		if (q3map.loaded)
		{
			unload();
		}
		destroy();
		return 0;
	}

	ret = strcmp(cmd, "exit");
	if (ret == 0)
	{
		exit(0);
	}

	return -1;
}

int Engine::console_network(char *cmd)
{
	char msg[512] = { 0 };
	char data[256] = { 0 };
	int port;
	int ret;


	ret = sscanf(cmd, "sv_hostname \"%[^\"]s", data);
	if (ret == 1)
	{
		bool valid = true;
		unsigned int data_length = strlen(data);

		for (unsigned int i = 0; i < data_length; i++)
		{
			if (data[i] >= 'A' && data[i] <= 'Z')
				continue;
			if (data[i] >= 'a' && data[i] <= 'z')
				continue;
			if (data[i] >= '0' && data[i] <= '9')
				continue;
			if (data[i] == ' ')
				continue;

			valid = false;
		}
		if (valid)
		{
			snprintf(netcode.sv_hostname, 511, "%s", data);
			debugf("sv_hostname: %s\n", data);
		}
		else
		{
			debugf("Invalid name, must be alphanumeric + space\n");
		}
		return 0;
	}

	if (strstr(cmd, "serverlist"))
	{
		netcode.query_master();
		return 0;
	}

	ret = sscanf(cmd, "sv_motd \"%[^\"]s", data);
	if (ret == 1)
	{
		bool valid = true;
		unsigned int data_length = strlen(data);

		for (unsigned int i = 0; i < data_length; i++)
		{
			if (data[i] >= 'A' && data[i] <= 'Z')
				continue;
			if (data[i] >= 'a' && data[i] <= 'z')
				continue;
			if (data[i] >= '0' && data[i] <= '9')
				continue;
			if (data[i] == ' ')
				continue;

			valid = false;
		}
		if (valid)
		{
			snprintf(netcode.sv_motd, 511, "%s", data);
			debugf("sv_motd: %s\n", data);
		}
		else
		{
			debugf("Invalid name, must be alphanumeric + space\n");
		}
		return 0;
	}



	ret = sscanf(cmd, "sv_maxclients %s", (char *)data);
	if (ret == 1)
	{
		int num = atoi(data);

//		if (num <= 8 && num >= 0)
		{
			debugf("setting max clients");
			netcode.sv_maxclients = num;
		}
		return 0;
	}

	ret = sscanf(cmd, "net_port %s", (char *)data);
	if (ret == 1)
	{
		int num = atoi(data);

		if (num <= 65535 && num >= 0)
		{
			debugf("setting port");
			netcode.net_port = num;
		}
		else
		{
			debugf("Invalid input");
		}
		return 0;
	}

	ret = sscanf(cmd, "bsp_depth %s", (char *)data);
	if (ret == 1)
	{
		int num = atoi(data);

		if (num <= 65535 && num >= 0)
		{
			debugf("setting depth");
			q3map.max_depth = num;
			q3map.lastIndex = -2;
		}
		else
		{
			debugf("Invalid input");
		}
		return 0;
	}

	ret = sscanf(cmd, "bsp_dec %s", (char *)data);
	if (ret == 1)
	{
		int num = atoi(data);
		
		if (num >= 0)
		{
			menu.print_chat(cmd);
			q3map.max_depth -= num;
			q3map.lastIndex = -2;
		}
		else
		{
			debugf("Invalid input");
		}
		return 0;
	}
	
	ret = sscanf(cmd, "bsp_inc %s", (char *)data);
	if (ret == 1)
	{
		int num = atoi(data);
		
		if (num >= 0)
		{
			menu.print_chat(cmd);
			q3map.max_depth += num;
			q3map.lastIndex = -2;
		}
		else
		{
			debugf("Invalid input");
		}
		return 0;
	}

	ret = sscanf(cmd, "connect %s", data);
	if (ret == 1)
	{
		if (q3map.loaded)
		{
			unload();
		}
		snprintf(msg, LINE_SIZE, "Connecting to %s\n", data);
		menu.print(msg);
		netcode.connect(data);
		return 0;
	}

	ret = sscanf(cmd, "cl_skip %s", data);
	if (ret == 1)
	{
		debugf("Setting cl_skip to %s\n", data);
		netcode.cl_skip = atoi(data);
		return 0;
	}

	ret = sscanf(cmd, "bind %d", &port);
	if (ret == 1)
	{
		snprintf(msg, LINE_SIZE, "binding to port %d\n", port);
		menu.print(msg);
		netcode.bind(port);
		return 0;
	}

	ret = strcmp(cmd, "bind default");
	if (ret == 0)
	{
		port = netcode.net_port;
		snprintf(msg, LINE_SIZE, "binding to port %d\n", port);
		menu.print(msg);
		netcode.bind(port);
		return 0;
	}

	return -1;
}

int Engine::console_render(char *cmd)
{
	char msg[LINE_SIZE] = { 0 };
	char data[LINE_SIZE] = { 0 };
	int ret;

	ret = sscanf(cmd, "r_num_shadowmap %s", data);
	if (ret == 1)
	{
		int value = atoi(data);
		mlight2.set_num_shadowmap(value);
		return 0;
	}

	ret = sscanf(cmd, "r_shadowmap_res_scale %s", data);
	if (ret == 1)
	{
		//		int value = atoi(data);
		//		mlight2.set_num_shadowmap(value);
		return 0;
	}

	if (strcmp(cmd, "r_apply") == 0)
	{
		int x_res;
		int y_res;

		debugf("Setting resolution to %s\n", resbuf[current_res + 1]);
		sscanf(resbuf[current_res + 1], "%dx%d", &x_res, &y_res);
#ifdef WIN32
		set_resolution(x_res, y_res, 32);
#endif
		menu.data.apply[0] = '\0';
		return 0;
	}

	if (strcmp(cmd, "r_res") == 0)
	{
		snprintf(msg, LINE_SIZE, "Resolution: %dx%d\n", gfx.width, gfx.height);
		menu.print(msg);
		return 0;
	}

	if (sscanf(cmd, "r_texture %s", data) == 1)
	{
		if (atoi(data))
		{
			snprintf(msg, LINE_SIZE, "textures enabled");
			menu.print(msg);
			q3map.enable_textures = true;
		}
		else
		{
			snprintf(msg, LINE_SIZE, "textures disabled");
			menu.print(msg);
			q3map.enable_textures = false;
		}
		return 0;
	}

	if (sscanf(cmd, "r_stencil %s", data) == 1)
	{
		if (atoi(data))
		{
			snprintf(msg, LINE_SIZE, "stencil enabled");
			menu.print(msg);
			enable_stencil = true;
			menu.data.shadowvol = true;
		}
		else
		{
			snprintf(msg, LINE_SIZE, "stencil disabled");
			menu.print(msg);
			enable_stencil = false;
			menu.data.shadowvol = false;
		}
		return 0;
	}

	if (sscanf(cmd, "r_fog %s", data) == 1)
	{
		if (atoi(data))
		{
			snprintf(msg, LINE_SIZE, "fog enabled");
			menu.print(msg);
			q3map.enable_fog = true;
		}
		else
		{
			snprintf(msg, LINE_SIZE, "fog disabled");
			menu.print(msg);
			q3map.enable_fog = false;
		}
		return 0;
	}

	if (sscanf(cmd, "r_patch %s", data) == 1)
	{
		if (atoi(data))
		{
			snprintf(msg, LINE_SIZE, "patches enabled");
			menu.print(msg);
			q3map.enable_patch = true;
		}
		else
		{
			snprintf(msg, LINE_SIZE, "patches disabled");
			menu.print(msg);
			q3map.enable_patch = false;
		}
		return 0;
	}


	if (sscanf(cmd, "g_patch_collision %s", data) == 1)
	{
		if (atoi(data))
		{
			snprintf(msg, LINE_SIZE, "patch collision enabled");
			menu.print(msg);
			q3map.enable_patch_collision = true;
		}
		else
		{
			snprintf(msg, LINE_SIZE, "patch collision disabled");
			menu.print(msg);
			q3map.enable_patch_collision = false;
		}
		return 0;
	}

	if (sscanf(cmd, "r_maxlight %s", data) == 1)
	{

		int max = atoi(data);

		mlight2.set_max(max);
		snprintf(msg, LINE_SIZE, "Setting max lights to %d", max);
		menu.print(msg);
		return 0;
	}

	if (sscanf(cmd, "r_ambient %s", data) == 1)
	{
		float ambient = (float)atof(data);
		mlight2.set_ambient(ambient);

		snprintf(msg, LINE_SIZE, "Setting ambient light to %f", ambient);
		menu.print(msg);
		return 0;
	}

	vec3 color;
	if (sscanf(cmd, "r_clearcolor %f %f %f", &color.x, &color.y, &color.z) == 3)
	{

		gfx.clear_color(color);
		snprintf(msg, LINE_SIZE, "Setting clear color to %f %f %f", color.x, color.y, color.z);
		menu.print(msg);
		return 0;
	}

	if (sscanf(cmd, "r_brightness %s", data) == 1)
	{
		float value = (float)atof(data);
		mlight2.set_brightness(value - 1.0f);
		snprintf(msg, LINE_SIZE, "Setting brightness to %f", value);
		menu.print(msg);
		menu.data.brightness = value / 2.0f;
		return 0;
	}

	if (sscanf(cmd, "r_tone %s", data) == 1)
	{
		int value = atoi(data);
		mlight2.set_tone(value);
		snprintf(msg, LINE_SIZE, "Setting tone to %d", value);
		menu.print(msg);
		return 0;
	}
	
	if (strstr(cmd, "r_brightness"))
	{
		snprintf(msg, LINE_SIZE, "brightness %f", mlight2.m_brightness + 1.0f);
		menu.print(msg);
		return 0;
	}

	if (sscanf(cmd, "r_exposure %s", data) == 1)
	{
		float value = (float)atof(data);
		mlight2.set_exposure(value);
		snprintf(msg, LINE_SIZE, "Setting exposure to %f", value);
		menu.print(msg);
		return 0;
	}

	if (strstr(cmd, "r_exposure"))
	{
		snprintf(msg, LINE_SIZE, "exposure %f", mlight2.m_exposure);
		menu.print(msg);
		return 0;
	}

	if (sscanf(cmd, "ssao_radius %s", data) == 1)
	{
		float ambient = (float)atof(data);
		ssao_radius = ambient;
		return 0;
	}

	if (strstr(cmd, "ssao_radius"))
	{
		snprintf(msg, LINE_SIZE, "ssao_radius %f", ssao_radius);
		menu.print(msg);
		return 0;
	}

	if (sscanf(cmd, "r_contrast %s", data) == 1)
	{
		float value = (float)atof(data);
		mlight2.set_contrast(value);
		snprintf(msg, LINE_SIZE, "Setting contrast to %f", value);
		menu.print(msg);
		menu.data.contrast = value;
		return 0;
	}

	if (sscanf(cmd, "r_dissolve %s", data) == 1)
	{
		float value = (float)atof(data);
		mlight2.set_dissolve(value);
		snprintf(msg, LINE_SIZE, "Setting dissolve to %f", value);
		menu.print(msg);
		return 0;
	}

	if (strstr(cmd, "r_contrast"))
	{
		snprintf(msg, LINE_SIZE, "contrast %f", mlight2.m_contrast);
		menu.print(msg);
		return 0;
	}

	if (strstr(cmd, "r_dissolve"))
	{
		snprintf(msg, LINE_SIZE, "dissolve %f", mlight2.m_dissolve);
		menu.print(msg);
		return 0;
	}

	if (sscanf(cmd, "r_lightmap %s", data) == 1)
	{
		float lightmap = (float)atof(data);
		mlight2.set_lightmap(lightmap);

		snprintf(msg, LINE_SIZE, "Setting lightmap to %f", lightmap);
		menu.print(msg);
		return 0;
	}

	ret = strcmp(cmd, "r_shadowmaps");
	if (ret == 0)
	{
		shadowmaps = !shadowmaps;
		if (shadowmaps)
		{
			mlight2.set_shadowmap(1.0f);
			snprintf(msg, LINE_SIZE, "Enabling shadowmaps");
			menu.print(msg);
			menu.data.shadowmaps = true;

			for (unsigned int i = max_dynamic; i < entity_list.size(); i++)
			{
				if (entity_list[i]->light)
				{
					if (entity_list[i]->light->depth_tex[0] == 0)
						entity_list[i]->light->generate_cubemaps(gfx);
				}
			}
		}
		else
		{
			snprintf(msg, LINE_SIZE, "Disabling shadowmaps");
			menu.print(msg);
			menu.data.shadowmaps = false;
			mlight2.set_shadowmap(0.0f);
		}
		return 0;
	}

	if (strstr(cmd, "postprocess"))
	{
		enable_postprocess = !enable_postprocess;
		if (enable_postprocess)
		{
			snprintf(msg, LINE_SIZE, "postprocessing on");
			menu.print(msg);
		}
		else
		{
			snprintf(msg, LINE_SIZE, "postprocessing off");
			menu.print(msg);
		}
		return 0;
	}

	ret = sscanf(cmd, "bloom_threshold %s", data);
	if (ret == 1)
	{
		debugf("Setting bloom_threshold to %s\n", data);
		bloom_threshold = (float)atof(data);
		return 0;
	}

	ret = sscanf(cmd, "bloom_strength %s", data);
	if (ret == 1)
	{
		debugf("Setting bloom_strength to %s\n", data);
		bloom_strength = (float)atof(data);
		return 0;
	}

	ret = sscanf(cmd, "bloom_amount %s", data);
	if (ret == 1)
	{
		debugf("Setting bloom_amount to %s\n", data);
		bloom_amount = (float)atof(data);
		return 0;
	}
	
	ret = sscanf(cmd, "dof_near %s", data);
	if (ret == 1)
	{
		debugf("Setting dof_near to %s\n", data);
		dof_near = (float)atof(data);
		return 0;
	}

	ret = sscanf(cmd, "dof_far %s", data);
	if (ret == 1)
	{
		debugf("Setting dof_far to %s\n", data);
		dof_far = (float)atof(data);
		return 0;
	}


	if (strstr(cmd, "bloom_debug"))
	{
		debug_bloom = !debug_bloom;

		if (debug_bloom)
		{
			snprintf(msg, LINE_SIZE, "bloom debug on");
			menu.print(msg);
		}
		else
		{
			snprintf(msg, LINE_SIZE, "bloom debug off");
			menu.print(msg);
		}

		return 0;
	}

	if (strstr(cmd, "r_bloom"))
	{
		enable_bloom = !enable_bloom;
		if (enable_bloom)
		{
			snprintf(msg, LINE_SIZE, "bloom on");
			menu.print(msg);
			menu.data.bloom = true;
		}
		else
		{
			snprintf(msg, LINE_SIZE, "bloom off");
			menu.print(msg);
			menu.data.bloom = false;
		}
		return 0;
	}

	if (strstr(cmd, "r_ssao"))
	{
		enable_ssao = !enable_ssao;
		if (enable_ssao)
		{
			snprintf(msg, LINE_SIZE, "ssao on");
			//			mlight2.set_brightness(0.5f - 1.0f);
			menu.print(msg);
			menu.data.ssao = true;
		}
		else
		{
			snprintf(msg, LINE_SIZE, "ssao off");
			//			mlight2.set_brightness(1.0f - 1.0f);
			menu.print(msg);
			menu.data.ssao = false;
		}
		return 0;
	}


	if (strstr(cmd, "blur"))
	{
		enable_blur = !enable_blur;
		if (enable_blur)
		{
			snprintf(msg, LINE_SIZE, "blur on");
			menu.print(msg);
		}
		else
		{
			snprintf(msg, LINE_SIZE, "blur off");
			menu.print(msg);
		}
		return 0;
	}

	if (strstr(cmd, "emboss"))
	{
		enable_emboss = !enable_emboss;
		if (enable_emboss)
		{
			snprintf(msg, LINE_SIZE, "emboss on");
			menu.print(msg);
		}
		else
		{
			snprintf(msg, LINE_SIZE, "emboss off");
			menu.print(msg);
		}
		return 0;
	}

	if (strstr(cmd, "r_map"))
	{
		enable_map = !enable_map;
		if (enable_map)
		{
			snprintf(msg, LINE_SIZE, "render map on");
			menu.print(msg);
		}
		else
		{
			snprintf(msg, LINE_SIZE, "render map off");
			menu.print(msg);
		}
		return 0;
	}

	if (strstr(cmd, "r_terrain"))
	{
		enable_terrain = !enable_terrain;
		if (enable_terrain)
		{
			snprintf(msg, LINE_SIZE, "render terrain on");
			menu.print(msg);
		}
		else
		{
			snprintf(msg, LINE_SIZE, "render terrain off");
			menu.print(msg);
		}
		return 0;
	}

	if (strstr(cmd, "r_planet"))
	{
		enable_planet = !enable_planet;
		if (enable_planet)
		{
			snprintf(msg, LINE_SIZE, "render planet on");
			menu.print(msg);
		}
		else
		{
			snprintf(msg, LINE_SIZE, "render planet off");
			menu.print(msg);
		}
		return 0;
	}

	if (sscanf(cmd, "r_light %s", data) == 1)
	{
		int mode = 0;
		mode = atoi(data);

		switch (mode)
		{
		case 0:
			mlight2.set_ambient(1.5);
			mlight2.set_lightmap(0.0);
			mlight2.set_max(0);
			break;
		case 1:
			mlight2.set_ambient(2.0);
			mlight2.set_lightmap(1.0);
			mlight2.set_max(0);
			break;
		case 2:
			mlight2.set_ambient(0.0);
			mlight2.set_lightmap(0.0);
			mlight2.set_max(64);
			break;
		case 3:
			mlight2.set_contrast(2.0);
			mlight2.set_ambient(1.0);
			mlight2.set_lightmap(0.5);
			mlight2.set_max(64);
			break;
		default:
			snprintf(msg, LINE_SIZE, "Invalid light mode [0-3]");
			menu.print(msg);
			return 0;
		}
		snprintf(msg, LINE_SIZE, "setting lightmode to %d", mode);
		menu.print(msg);
		return 0;
	}

	if (sscanf(cmd, "com_maxfps %s", data) == 1)
	{
		if (atoi(data) > 0)
		{
			snprintf(msg, LINE_SIZE, "Setting com_maxfps to %d", atoi(data));
			menu.print(msg);
			com_maxfps = 1000.0 / atoi(data);
		}
		return 0;
	}

	if (strstr(cmd, "com_maxfps"))
	{
		snprintf(msg, LINE_SIZE, "com_maxfps %f", com_maxfps / 1000.0f);
		menu.print(msg);
		return 0;
	}

	if (sscanf(cmd, "r_sky %s", data) == 1)
	{
		if (atoi(data))
		{
			q3map.enable_sky = true;
			snprintf(msg, LINE_SIZE, "Enabling skybox");
			menu.print(msg);
		}
		else
		{
			q3map.enable_sky = false;
			snprintf(msg, LINE_SIZE, "Disabling skybox");
			menu.print(msg);
		}
		return 0;
	}

	if (strcmp(cmd, "r_max_particles") == 0)
	{
		sprintf(data, "max particles is: %d\n", ParticleUpdate::max_particles);
		menu.print(msg);
		return 0;
	}

	if (sscanf(cmd, "r_max_particles %s", data) == 1)
	{
		menu.print(cmd);
		ParticleUpdate::max_particles = atoi(data);
		return 0;
	}

	if (strcmp(cmd, "r_reload_shaders") == 0)
	{
		sprintf(msg, "reloading gpu shaders");
		menu.print(msg);

		reload_shaders();
		return 0;
	}

	if (strcmp(cmd, "r_no_pixel_shader") == 0)
	{
		sprintf(msg, "disabling pixel shader");
		menu.print(msg);

		mlight2.destroy();
		mlight2.init(&gfx, false);
		return 0;
	}

	if (sscanf(cmd, "r_shader %s", data) == 1)
	{
		if (atoi(data))
		{
			q3map.enable_shader = true;
			snprintf(msg, LINE_SIZE, "Enabling q3 shaders");
			menu.print(msg);
		}
		else
		{
			snprintf(msg, LINE_SIZE, "Disabling q3 shaders");
			menu.print(msg);
			q3map.enable_shader = false;
		}
		return 0;
	}

	if (sscanf(cmd, "r_entities %s", data) == 1)
	{
		if (atoi(data))
		{
			snprintf(msg, LINE_SIZE, "Enabling entities");
			menu.print(msg);
			enable_entities = true;
		}
		else
		{
			snprintf(msg, LINE_SIZE, "Disabling entities");
			menu.print(msg);
			enable_entities = false;
		}
		return 0;
	}

	if (sscanf(cmd, "r_blend %s", data) == 1)
	{
		if (atoi(data))
		{
			q3map.enable_blend = true;
			snprintf(msg, LINE_SIZE, "Enabling blends");
			menu.print(msg);
		}
		else
		{
			q3map.enable_blend = false;
			snprintf(msg, LINE_SIZE, "Disabling blends");
			menu.print(msg);
		}
		return 0;
	}

	if (sscanf(cmd, "r_maxstage %s", data) == 1)
	{
		q3map.max_stage = atoi(data);
		snprintf(msg, LINE_SIZE, "Setting q3 shader max stage to %d", q3map.max_stage);
		menu.print(msg);
		return 0;
	}

	if (sscanf(cmd, "r_portal %s", data) == 1)
	{
		if (atoi(data))
		{
			enable_portal = true;
			snprintf(msg, LINE_SIZE, "Enabling portals");
			menu.print(msg);
			menu.data.portal = true;
		}
		else
		{
			enable_portal = false;
			snprintf(msg, LINE_SIZE, "Disabling portals");
			menu.print(msg);
			menu.data.portal = false;
		}
		return 0;
	}

	vec3 scale;
	if (sscanf(cmd, "r_normalmap_scale %f %f %f", &scale.x, &scale.y, &scale.z) == 3)
	{
		mlight2.set_normalmap_scale(scale);
		snprintf(msg, LINE_SIZE, "Setting normalmap scale %f %f %f", scale.x, scale.y, scale.z);
		menu.print(msg);
		return 0;
	}


	if (sscanf(cmd, "r_normalmap %s", data) == 1)
	{
		float value = (float)atof(data);

		q3map.enable_normalmap = true;
		mlight2.set_normalmap(value);
		snprintf(msg, LINE_SIZE, "Setting normalmap to %f", value);
		menu.print(msg);
		return 0;
	}


	if (sscanf(cmd, "r_specular_factor %s", data) == 1)
	{
		float value = (float)atof(data);

		mlight2.set_specular_factor(value);
		snprintf(msg, LINE_SIZE, "Setting specular factor to %f", value);
		menu.print(msg);
		return 0;
	}

	if (sscanf(cmd, "r_diffuse_factor %s", data) == 1)
	{
		float value = (float)atof(data);

		mlight2.set_diffuse_factor(value);
		snprintf(msg, LINE_SIZE, "Setting diffuse factor to %f", value);
		menu.print(msg);
		return 0;
	}


	if (sscanf(cmd, "r_specular_exponent %s", data) == 1)
	{
		float value = (float)atof(data);

		mlight2.set_specular_exponent(value);
		snprintf(msg, LINE_SIZE, "Setting specular exponent to %f", value);
		menu.print(msg);
		return 0;
	}

	if (sscanf(cmd, "r_atten_exponent %s", data) == 1)
	{
		float value = (float)atof(data);

		mlight2.set_atten_exponent(value);
		snprintf(msg, LINE_SIZE, "Setting atten exponent to %f", value);
		menu.print(msg);
		return 0;
	}

	if (sscanf(cmd, "r_atten_scale %s", data) == 1)
	{
		float value = (float)atof(data);

		mlight2.set_atten_scale(value);
		snprintf(msg, LINE_SIZE, "Setting atten scale to %f", value);
		menu.print(msg);
		return 0;
	}

	if (sscanf(cmd, "r_atten_min %s", data) == 1)
	{
		float value = (float)atof(data);

		mlight2.set_atten_min(value);
		snprintf(msg, LINE_SIZE, "Setting atten min to %f", value);
		menu.print(msg);
		return 0;
	}

	if (sscanf(cmd, "r_specular_min %s", data) == 1)
	{
		float value = (float)atof(data);

		mlight2.set_specular_min(value);
		snprintf(msg, LINE_SIZE, "Setting specular min to %f", value);
		menu.print(msg);
		return 0;
	}

	if (sscanf(cmd, "r_diffuse_min %s", data) == 1)
	{
		float value = (float)atof(data);

		mlight2.set_diffuse_min(value);
		snprintf(msg, LINE_SIZE, "Setting diffuse min to %f", value);
		menu.print(msg);
		return 0;
	}


	if (sscanf(cmd, "cg_fov %s", data) == 1)
	{
		snprintf(msg, LINE_SIZE, "Setting fov to %d\n", atoi(data));
		menu.print(msg);
		fov = atoi(data) * 0.5f;
		projection.perspective(fov, (float)xres / yres, zNear, zFar, inf);
		return 0;
	}

	if (sscanf(cmd, "r_rendermode %s", data) == 1)
	{
		if (atoi(data) == MODE_INDIRECT)
		{
			snprintf(msg, LINE_SIZE, "Setting rendermode to indirect\n");
			menu.print(msg);
			render_mode = MODE_INDIRECT;
		}
		else if (atoi(data) == MODE_FORWARD)
		{
			snprintf(msg, LINE_SIZE, "Setting rendermode to forward\n");
			menu.print(msg);
			render_mode = MODE_FORWARD;
		}
		return 0;
	}

	if (sscanf(cmd, "r_multisample %s", data) == 1)
	{
		multisample = atoi(data);
		menu.data.antialias = multisample;
#ifdef OPENGL
		if (multisample > 0)
		{
			snprintf(msg, LINE_SIZE, "Enabling multisampling");
			menu.print(msg);
			glEnable(GL_MULTISAMPLE);
		}
		else
		{
			snprintf(msg, LINE_SIZE, "Disabling multisampling");
			menu.print(msg);
			glDisable(GL_MULTISAMPLE);
		}
#endif
		//		gfx.DeleteFrameBuffer(render_fbo, render_quad, render_depth);
		//		gfx.CreateFramebuffer(fb_width, fb_height, render_fbo, render_quad, render_depth, render_ndepth, multisample, true);
		return 0;
	}

	if (sscanf(cmd, "res_scale %s", data) == 1)
	{
		unsigned int normal_depth;

		snprintf(msg, LINE_SIZE, "Setting resolution scale to %3.3f\n", atof(data));
		menu.print(msg);
		res_scale = (float)atof(data);
		fb_width = (unsigned int)(FBO_RESOLUTION * res_scale);
		fb_height = (unsigned int)(FBO_RESOLUTION * res_scale);

		gfx.DeleteFrameBuffer(render_fbo, render_quad, render_depth);
		gfx.DeleteFrameBuffer(mask_fbo, mask_quad, mask_depth);
		gfx.DeleteFrameBuffer(blur1_fbo, blur1_quad, blur1_depth);
		gfx.DeleteFrameBuffer(blur2_fbo, blur2_quad, blur2_depth);
		gfx.DeleteFrameBuffer(ssao_fbo, ssao_quad, ssao_depth);

		gfx.CreateFramebuffer(fb_width, fb_height, render_fbo, render_quad, render_depth, render_ndepth, multisample, true);

		gfx.CreateFramebuffer(fb_width, fb_height, mask_fbo, mask_quad, mask_depth, normal_depth, 0, false);
		gfx.CreateFramebuffer(fb_width, fb_height, blur1_fbo, blur1_quad, blur1_depth, normal_depth, 0, false);
		gfx.CreateFramebuffer(fb_width, fb_height, blur2_fbo, blur2_quad, blur2_depth, normal_depth, 0, false);
		gfx.CreateFramebuffer(fb_width, fb_height, ssao_fbo, ssao_quad, ssao_depth, normal_depth, 0, false);



		menu.data.rscale = res_scale / 2.0f;
		return 0;
	}

	if (sscanf(cmd, "dynamic_res %s", data) == 1)
	{
		if (atoi(data))
		{
			snprintf(msg, LINE_SIZE, "Dynamic resolution on\n");
			menu.print(msg);
			dynamic_resolution = true;
		}
		else
		{
			snprintf(msg, LINE_SIZE, "Dynamic resolution off\n");
			menu.print(msg);
			dynamic_resolution = false;
		}
		return 0;
	}

	ret = sscanf(cmd, "r_polygonmode %s", data);
	if (ret == 1)
	{
#ifdef OPENGL
		if (atoi(data) == 1)
		{
			snprintf(msg, LINE_SIZE, "Line rendering");
			menu.print(msg);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else if (atoi(data) == 2)
		{
			snprintf(msg, LINE_SIZE, "Point rendering");
			menu.print(msg);
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		}
		else
		{
			snprintf(msg, LINE_SIZE, "Normal rendering");
			menu.print(msg);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
#endif
		return 0;
	}

	ret = sscanf(cmd, "r_frontface %s", data);
	if (ret == 1)
	{
#ifdef OPENGL
		if (atoi(data) == 1)
		{
			snprintf(msg, LINE_SIZE, "Counter clockwise winding");
			menu.print(msg);
			glFrontFace(GL_CCW);
		}
		else
		{
			snprintf(msg, LINE_SIZE, "Clockwise winding");
			menu.print(msg);
			glFrontFace(GL_CW);
		}
#endif
		return 0;
	}

	return -1;
}

///=============================================================================
/// Function: console_sound
///=============================================================================
/// Description: Console command related to audio / sound
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
int Engine::console_sound(char *cmd)
{
	char data[LINE_SIZE] = { 0 };
	int ret;
	float value = 1.0f;

	if (cmd == NULL)
		return 0;

	ret = sscanf(cmd, "al_reference_dist %f", &value);
	if (ret == 1)
	{
		debugf("Setting audio reference distance to %f\n", value);
		set_reference_distance(value);
		return 0;
	}

	ret = sscanf(cmd, "al_max_dist %f", &value);
	if (ret == 1)
	{
		debugf("Setting audio max distance to %f\n", value);
		set_max_distance(value);
		return 0;
	}

	ret = sscanf(cmd, "al_rolloff %f", &value);
	if (ret == 1)
	{
		debugf("Setting audio rolloff factor to %f\n", value);
		set_rolloff_factor(value);
		return 0;
	}

	ret = sscanf(cmd, "al_model %s", (char *)data);
	if (ret == 1)
	{
		debugf("Setting audio model to %d\n", atoi(data));
		audio.set_audio_model(atoi(data));
		return 0;
	}

	return -1;
}


///=============================================================================
/// Function: console
///=============================================================================
/// Description: Main command console entrypoint
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Engine::console(char *cmd)
{

	if (cmd == NULL)
		return;

/*
	static int last_cmd = 0;
	if (last_cmd >= tick_num)
		return;
	last_cmd = tick_num;
	*/

	if (console_general(cmd) == 0)
		return;
	if (console_network(cmd) == 0)
		return;
	if (console_render(cmd) == 0)
		return;
	if (console_sound(cmd) == 0)
		return;

	
	if (q3map.loaded)
	{
		int player = find_type(ENT_PLAYER, 0);

		if (player == -1)
		{
			int player = find_type(ENT_SPECTATOR, 0);
			if (player == -1)
				return;
		}

		game->console(player, cmd, menu, entity_list);
	}
}




///=============================================================================
/// Function: select_wave
///=============================================================================
/// Description: associates a sound file to a sound source (emitter)
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
bool Engine::select_wave(int source, char *file)
{
	if (file == NULL || file[0] == '\0')
		return true;

	for (unsigned int i = 0; i < snd_wave.size(); i++)
	{
		if (strcmp(snd_wave[i].file, file) == 0)
		{
			return audio.select_buffer(source, snd_wave[i].buffer);
		}
	}
	return false;
}

///=============================================================================
/// Function: get_source
///=============================================================================
/// Description: gets a free sound source (of 32 total)
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
int Engine::get_source()
{
	int source;
	static int i = 0;

	source = audio_source[i++];

	if (i == max_sources)
		i = 0;

	return source;
}


///=============================================================================
/// Function: get_loop_source
///=============================================================================
/// Description: gets a free sound source that loops (of 32 total)
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
int Engine::get_loop_source()
{
	int source;
	static int i = 0;

	source = audio_loop_source[i++];

	if (i == max_sources)
		i = 0;

	return source;
}

///=============================================================================
/// Function: get_global_source
///=============================================================================
/// Description: gets a free sound source that is not positional (of 32 total)
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
int Engine::get_global_source()
{
	int source;
	static int i = 0;

	source = global_source[i++];

	if (i == max_sources)
		i = 0;

	return source;
}

///=============================================================================
/// Function: get_global_loop_source
///=============================================================================
/// Description: gets a free sound source that is not positional and loops (of 32 total)
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
int Engine::get_global_loop_source()
{
	int source;
	static int i = 0;

	source = global_loop_source[i++];

	if (i == max_sources)
		i = 0;

	return source;
}


///=============================================================================
/// Function: play_wave_source
///=============================================================================
/// Description: Plays sound with index from passed source
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
bool Engine::play_wave_source(int source, int index)
{
	audio.select_buffer(source, snd_wave[index].buffer);
	audio.play(source);
	return true;
}

///=============================================================================
/// Function: play_wave
///=============================================================================
/// Description: Plays sound at specific position
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
int Engine::play_wave(vec3 &position, int index)
{
	if (index < 0)
		return 0;

	int source = get_source();

	audio.source_position(source, &position.x);

	if (index != 302)
		audio.select_buffer(source, snd_wave[index].buffer);
	else
		audio.select_buffer(source, doom_sound);
	audio.play(source);
	return source;
}


///=============================================================================
/// Function: play_wave_loop
///=============================================================================
/// Description: Plays looping sound at specific position
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
int Engine::play_wave_loop(vec3 &position, int index)
{
	if (index < 0)
		return 0;

	int source = get_loop_source();

	audio.source_position(source, &position.x);

	if (index != 302)
		audio.select_buffer(source, snd_wave[index].buffer);
	else
		audio.select_buffer(source, doom_sound);
	audio.play(source);
	return source;
}

///=============================================================================
/// Function: play_wave_global
///=============================================================================
/// Description: Plays sound without position mixing (regular playback)
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
int Engine::play_wave_global(int index)
{
	if (index < 0)
		return 0;

	int source = get_global_source();
	audio.select_buffer(source, snd_wave[index].buffer);
	audio.play(source);
	return source;
}

///=============================================================================
/// Function: play_wave_global_loop
///=============================================================================
/// Description: Plays looping sound without position mixing (regular playback)
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
int Engine::play_wave_global_loop(int index)
{
	if (index < 0)
		return 0;

	int source = get_global_loop_source();
	audio.select_buffer(source, snd_wave[index].buffer);
	audio.play(source);
	return source;
}


///=============================================================================
/// Function: hitscan
///=============================================================================
/// Description: casts a ray at origin in direction dir and tests for collisions
/// with rigid bodies. The map will block the hitscan
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Engine::hitscan(vec3 &origin, vec3 &dir, int *index_list, int &num_index, int self, float range, bool light)
{
	int j = 0;
	num_index = 0;

	for (unsigned int i = 0; i < entity_list.size(); i++)
	{
		EntRigidBody *rigid = entity_list[i]->rigid;
		if (i == (unsigned int)self)
			continue;

		if (light && rigid)
		{
			float distance = FLT_MAX;
			vec3 min = rigid->aabb[0] + entity_list[i]->position;
			vec3 max = rigid->aabb[7] + entity_list[i]->position;

			if (RayBoxSlab(origin, dir, min, max, distance))
			{
				vec3 endpoint = entity_list[i]->position;
				vec3 normal;

				vec3 dist2 = entity_list[i]->position - origin;
				float enemy_distance = dist2.magnitude();

				if (range > 0.0f && enemy_distance > range)
					continue;

				// Hit someone, do BSP trace to check if a wall is in the way
				q3map.trace(origin, endpoint, normal);
				if (false)
				{
					vec3 dist1 = endpoint - origin;

					q3map.collision = false;
					if (dist1.magnitude() > enemy_distance)
					{
						index_list[j++] = i;
						num_index++;
						break;
					}
				}
				else
				{
					index_list[j++] = i;
					num_index++;
					break;
				}
			}

			continue; // end light hit checks
		}

		if ((entity_list[i]->player || entity_list[i]->construct) && rigid != NULL)
		{
			float distance = FLT_MAX;
			vec3 min = rigid->aabb[0] + entity_list[i]->position - rigid->center;
			vec3 max = rigid->aabb[7] + entity_list[i]->position - rigid->center;

			if (RayBoxSlab(origin, dir, min, max, distance))
			{
				vec3 endpoint = entity_list[i]->position;
				vec3 normal;

				vec3 dist2 = entity_list[i]->position - origin;
				float enemy_distance = dist2.magnitude();

				if (range > 0.0f && enemy_distance > range)
					continue;

				// Hit someone, do BSP trace to check if a wall is in the way
	            q3map.trace(origin, endpoint, normal);
	            if (q3map.collision)
	            {
					vec3 dist1 = endpoint - origin;



	                q3map.collision = false;
					if (dist1.magnitude() > enemy_distance)
					{
						index_list[j++] = i;
						num_index++;
						break;
					}
	            }
				else
				{
					index_list[j++] = i;
					num_index++;
					break;
				}
			}

			continue; // end player hit checks
		}
	}

}


void Engine::rayscan(vec3 &origin, vec3 &dir, int *index_list, int &num_index, int self, float range, bool light)
{
	int j = 0;
	num_index = 0;

	for (unsigned int i = 0; i < entity_list.size(); i++)
	{
		EntRigidBody *rigid = entity_list[i]->rigid;
		if (i == (unsigned int)self)
			continue;

		if (rigid)
		{
			vec3 min = rigid->aabb[0] + entity_list[i]->position;
			vec3 max = rigid->aabb[7] + entity_list[i]->position;

			ray_t ray;
			raycast_result_t res;
			aabb_t aabb;
			ray.origin = origin;
			ray.dir = dir;

			aabb.min = min;
			aabb.max = max;

			if ( Raycast(aabb, ray, &res) )
			{
				vec3 normal;

				vec3 dist2 = entity_list[i]->position - origin;
				float enemy_distance = dist2.magnitude();

				if (range > 0.0f && enemy_distance > range)
					continue;

				index_list[j++] = i;
				num_index++;
				break;
			}

		}

	}
}

///=============================================================================
/// Function: find_path
///=============================================================================
/// Description: A* graph search for bots (only q3tourney2 has nav data currently)
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Engine::find_path(int *&path, int &path_length, int start_path, int end_path)
{
	if (graph.size() == 0)
	{
		path_length = 0;
		return;
	}

	//	printf("Searching for path from node%d to node%d\n", start_path, end_path);
	graph.astar_path(path, ref, start_path, end_path, &path_length);
	//	print_path(path, path_length, node);
}

///=============================================================================
/// Function: reload_shaders
///=============================================================================
/// Description: Reloads GPU shaders in place (useful for editing shader files
/// without restarting the entire program)
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Engine::reload_shaders()
{
	mlight2.destroy();
	mlight2.init(&gfx);
	global.destroy();
	global.init(&gfx);
	particle_update.destroy();
	particle_update.init(&gfx);
	particle_render.destroy();
	particle_render.init(&gfx);
	post.destroy();
	post.init(&gfx);
}


///=============================================================================
/// Function: get_shaderlist_pk3
///=============================================================================
/// Description: Reads shaderlist.txt from pk3 files (filename order takes
/// precedence as in last loaded wins)
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Engine::get_shaderlist_pk3(char **shaderlist, int &num_shader)
{
	char *filelist = new char[LIST_SIZE];
	char *line = NULL;

	memset(filelist, 0, LIST_SIZE);
	for (unsigned int i = 0; i < num_pk3; i++)
	{
		list_zipfile(pk3_list[i] + FILE_OFFSET, &filelist[0]);

		line = strtok(filelist, "\n");
		while (line)
		{
			if (strstr(line, ".shader") != NULL)
			{
				bool found = false;

				for (int j = 0; j < num_shader; j++)
				{
					if (strcmp(shaderlist[j], line) == 0)
					{
						found = true;
						break;
					}
				}
				if (found == false)
				{
					shaderlist[num_shader] = new char[256];
					strcpy(shaderlist[num_shader++], line);
				}
			}
			line = strtok(NULL, "\n");
		}
		memset(filelist, 0, LIST_SIZE);
	}

	delete[] filelist;
}

///=============================================================================
/// Function: set_reference_distance
///=============================================================================
/// Description: Sets positional audio fall off distance
/// Reference distance is where sound will sound "normal" without being louder
/// or quieter
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Engine::set_reference_distance(float value)
{
#ifndef DEDICATED
	for (int i = 0; i < max_sources; i++)
	{
		alSourcef(audio_source[i], AL_REFERENCE_DISTANCE, value);
		alSourcef(global_source[i], AL_REFERENCE_DISTANCE, value);
	}
#endif
}

///=============================================================================
/// Function: set_max_distance
///=============================================================================
/// Description: Sets positional audio fall off distance
/// max distance is where sound will fall below inaudiable
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Engine::set_max_distance(float value)
{
#ifndef DEDICATED
	for (int i = 0; i < max_sources; i++)
	{
		alSourcef(audio_source[i], AL_MAX_DISTANCE, value);
		alSourcef(global_source[i], AL_MAX_DISTANCE, value);
	}
#endif
}

///=============================================================================
/// Function: set_rolloff_factor
///=============================================================================
/// Description: Sets positional audio fall off distance rates
///
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Engine::set_rolloff_factor(float value)
{
#ifndef DEDICATED
	for (int i = 0; i < max_sources; i++)
	{
		alSourcef(audio_source[i], AL_ROLLOFF_FACTOR, value);
		alSourcef(global_source[i], AL_ROLLOFF_FACTOR, value);
	}
#endif
}

///=============================================================================
/// Function: paste
///=============================================================================
/// Description: Used to handle copy / paste for console commands
///
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Engine::paste(char *data, unsigned int size)
{
	if (menu.console || menu.chatmode || menu.stringmode)
	{
		for(unsigned int i = 0; i < size; i++)
			keystroke(data[i], NULL);
	}
}

///=============================================================================
/// Function: paste
///=============================================================================
/// Description: Used to handle copy / paste for console commands
///
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Engine::copy(char *data, unsigned int size)
{
	menu.copy(data, size);
}


///=============================================================================
/// Function: enum_resolutions
///=============================================================================
/// Description: Lists all available resolutions (for menu system)
///
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Engine::enum_resolutions()
{
#ifdef WIN32
	int i = 1;
	DEVMODE dmScreenSettings;


	num_res = 0;
	for (i = 1; i != 0; i++)
	{
		if (EnumDisplaySettings(NULL, i - 1, &dmScreenSettings))
		{
			if (dmScreenSettings.dmPelsWidth < 1024 || dmScreenSettings.dmBitsPerPel < 32 || dmScreenSettings.dmDisplayFrequency < 60)
				continue;

			sprintf(resbuf[num_res], "%dx%d %dHz", dmScreenSettings.dmPelsWidth,
				dmScreenSettings.dmPelsHeight,
				dmScreenSettings.dmDisplayFrequency);

			if (strcmp(resbuf[num_res], resbuf[num_res - 1]) == 0)
			{
				resbuf[num_res][0] = '\0';
				continue;
			}
			num_res++;
			if (num_res >= MAX_RES)
				break;

		}
		else
		{
			i = -1;
		}
	}
#endif
}

#if 0
using namespace physics;

///=============================================================================
/// Function: draw_plane
///=============================================================================
/// Description: draws a 3d plane in space at origin with normal facing fwd 
/// useful for debugging
///
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Engine::draw_plane(Graphics &gfx, plane_t &plane, vec3 &fwd, vec3 &origin)
{
	raycast_result_t result;
	ray_t ray;
	matrix4 mvp;

	ray.origin = origin;
	ray.dir = fwd;
	vec3 pos;

	Raycast(plane, ray, &result);

	if (result.hit == false)
		return;

	pos = vec3(0.0f, 50.0f, 0.0f);

	// make point axial to plane (change forward vector and normalze a bunch)
	vec3 up = camera_frame.up;
	vec3 forward = plane.normal;

	vec3 right = vec3::crossproduct(forward, up);
	right.normalize();
	up = vec3::crossproduct(right, forward);
	up.normalize();
	right = vec3::crossproduct(up, forward);
	right.normalize();

	mvp.m[0] = right.x * 10.0f;
	mvp.m[1] = up.x;
	mvp.m[2] = forward.x;
	mvp.m[3] = 0.0f;

	mvp.m[4] = right.y;
	mvp.m[5] = up.y * 10.0f;
	mvp.m[6] = forward.y;
	mvp.m[7] = 0.0f;

	mvp.m[8] = right.z;
	mvp.m[9] = up.z;
	mvp.m[10] = forward.z * 10.0f;
	mvp.m[11] = 0.0f;


	mvp.m[12] = 0.0f;// -right * pos;
	mvp.m[13] = 0.0f; //-up * pos;
	mvp.m[14] = 0.0f; //-forward * pos;
	mvp.m[15] = 1.0f;

//	mvp = identity;



	mvp = mvp * projection;


	global.Select();
	global.Params(mvp);
	gfx.SelectTexture(0, no_tex);
	gfx.SelectIndexBuffer(quad_index);
	gfx.SelectVertexBuffer(quad_vertex);
	gfx.DrawArrayTri(0, 0, 6, 4);


}
#endif


///=============================================================================
/// Function: test_triangle
///=============================================================================
/// Description: draws a basic triangle just for testing sanity of rendering
/// systems
///
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Engine::test_triangle()
{
	//test triangle for sanity check of renderer backend
	vertex_t tVertices[3];

	memset(&tVertices, 0, sizeof(vertex_t) * 3);
	tVertices[0].position.x = 0.0f;
	tVertices[0].position.y = 0.5f;
	tVertices[0].position.z = 0.0f;


	tVertices[1].position.x = 0.24f;
	tVertices[1].position.y = -0.5f;
	tVertices[1].position.z = 0.0f;

	tVertices[2].position.x = -0.45f;
	tVertices[2].position.y = -0.5f;
	tVertices[2].position.z = 0.0f;

	gfx.clear();

	int index[3] = { 0,1,2 };
	int index_buf = gfx.CreateIndexBuffer(index, 3);
	int vert_buf = gfx.CreateVertexBuffer(tVertices, 3);

	global.Select();
	gfx.SelectIndexBuffer(index_buf);
	gfx.SelectVertexBuffer(vert_buf);
	gfx.SelectTexture(0, no_tex);
	gfx.DrawArrayTri(0, 0, 3, 3);
	gfx.swap();
}


///=============================================================================
/// Function: MousetoRay
///=============================================================================
/// Description:Think this is suppose to be picking code that uses
/// old style glu functions, I dont link with GLU so anything glu like is likely
/// implemented manually
///
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Engine::MousetoRay(int x, int y, matrix4 &model, matrix4 &proj, const Frame &frame, vec3 &start, vec3 &end, vec3 &dir, int width, int height)
{
	// Convert windows upper left coordinates to opengl lower left coorindates
	float winx = (float)x;
	float winy = height - (float)y;

	// unproject the near plane
	gluUnProject(winx, winy, 0.0, model, proj, width, height, start.x, start.y, start.z);

	// unproject the far plane
	gluUnProject(winx, winy, 1.0, model, proj, width, height, end.x, end.y, end.z);

	dir = end - start;
	end = start + dir * 1000.0f;
	dir.normalize();
}


///=============================================================================
/// Function: Pick
///=============================================================================
/// Description: This is the picking code when you enter "pick_mode" in the console
/// It works fine for the near plane, but the ray to the far plane isn't quite right
/// I think my mistake was attempting to find a point on the far plane
/// Seems you can just get a vector at your near plane and follow the ray to the
/// far plane
///
/// console command "pick_mode"
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Engine::Pick(int x, int y, int width, int height, Frame &frame)
{
	vec2 screen;

	screen.x = (float)(x) / width;
	screen.y = (float)(height - y) / height;

	printf("screen %d %d, %f %f\r\n", x, y, screen.x, screen.y);


	vec4 clip_near;
	vec4 clip_far;

	clip_near.x = -2.0f * (screen.x - 0.5f);
	clip_near.y = -2.0f * (screen.y - 0.5f);
	clip_near.z = -0.99f;
	clip_near.w = -1.0f;
	clip_far = clip_near;
	clip_far.z = 0.99f;
	clip_far.w = 1.0f;
	printf("clip %f %f\r\n", clip_near.x, clip_near.y);

	matrix4 transformation;
	camera_frame.set(transformation);

	matrix4 mvp = transformation * projection;
	matrix4 imvp = mvp.inverse();

	vec4 world_near = imvp * clip_near;
	vec4 world_far = imvp * clip_far;

	vec3 origin = vec3(world_near.x / world_near.w, world_near.y / world_near.w, world_near.z / world_near.w);
	vec3 end = vec3(world_far.x / world_far.w, world_far.y / world_far.w, world_far.z / world_far.w);

	vec3 dir = end - origin;
	dir = dir.normalize();


	printf("origin %f %f %f\r\n", origin.x, origin.y, origin.z);
	printf("end %f %f %f\r\n", end.x, end.y, end.z);
	printf("dir %f %f %f\r\n", dir.x, dir.y, dir.z);
	printf("fwd %f %f %f\r\n", frame.forward.x, frame.forward.y, frame.forward.z);

	Frame pick_frame;


//	MousetoRay(x, y, transformation, projection, frame, origin, end, dir, width, height);

	make_frame(dir, frame.up, pick_frame);

	// raycast into scene, intersect with entities
	int index_list[32];
	int num_index = 0;

	int self = find_type(ENT_PLAYER, 0);

	bool debug_pick = true;
	int debug_ent = -1;
	if (debug_pick)
	{
		debug_ent = get_entity();
		Entity *projectile = entity_list[debug_ent];
		projectile->nettype = NET_LIGHTNING;
		projectile->rigid = new EntRigidBody(projectile);
		projectile->model = projectile->rigid;
		projectile->position = origin;
		projectile->rigid->clone(game->get_model_table()[17]);
		projectile->rigid->velocity = vec3();
		projectile->rigid->angular_velocity = vec3();
		projectile->rigid->flags.gravity = false;
		projectile->rigid->bounce = 2;
		projectile->model->flags.rail_trail = true;
		projectile->model = projectile->rigid;
		frame.set(projectile->model->morientation);
		projectile->flags.visible = true; // accomodate for low spatial testing rate
		projectile->rigid->flags.noclip = true;
		projectile->flags.bsp_visible = true;



		projectile->projectile = new EntProjectile(projectile, audio);
		projectile->projectile->action[0] = '\0';

		projectile->projectile->hide = false;
		projectile->projectile->radius = 25.0f;
		projectile->projectile->idle = true;
		projectile->projectile->idle_timer = (int)(0.075 * TICK_RATE);
		projectile->projectile->explode = true;
		projectile->projectile->explode_timer = 5;
		projectile->projectile->owner = self;

	}


	rayscan(origin, frame.forward, index_list, num_index, self, 1000.0f, true);

	picked_ent = -1;
	int min_dist = -1;
	float dist = 1000000.0f;
	for (int i = 0; i < num_index; i++)
	{
		if (i == debug_ent)
			continue;

		if (entity_list[index_list[i]]->flags.frustum_visible)
		{
			printf("%d: Checked entity %d type %s frustum visible %d\r\n", i, index_list[i], entity_list[index_list[i]]->entstring->type,
				entity_list[index_list[i]]->flags.frustum_visible);

			vec3 vec = frame.pos - entity_list[index_list[i]]->position;
			if (abs32(vec.magnitude()) < dist)
			{
				dist = vec.magnitude();
				min_dist = i;
			}
		}
	}

	if (min_dist != -1)
	{
		int i = min_dist;
		printf("%d: Picked entity %d type %s frustum visible %d\r\n", i, index_list[i], entity_list[index_list[i]]->entstring->type,
			entity_list[index_list[i]]->flags.frustum_visible);
		picked_ent = index_list[i];
	}
	else
	{
		printf("No pick %d checked\r\n", num_index);
	}


}



///=============================================================================
/// Function: edit_entity
///=============================================================================
/// Description: So this code selects an entity to edit in entity edit mode
/// allows you to change it's position, and some properties
/// mainly for lighting you can change intensity, color, and position for
/// prototyping lights
///
/// In console type "edit <entity number>" you can view entity names with 
/// console command "shownames"
/// 
/// You can select the property with the mouse wheel and change with arrow keys
/// as well as enter and shift
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Engine::edit_entity(unsigned int index)
{
	if (index >= entity_list.size())
	{
		edit_ent = -1;
		picked_ent = -1;
		show_names = false;
		show_box = false;
		return;
	}

	edit_ent = index;
	picked_ent = index;
	show_names = true;
	show_box = true;
}
