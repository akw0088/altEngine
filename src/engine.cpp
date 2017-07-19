#include "include.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define MAX_DEPTH 6

// Height above desired position we allow a step to occur
// (Fairly large as forward velocity can put you fairly deep forward)
#define STAIR_HEIGHT 20.0f
// Stair position and velocity added when we step up a stair
// Too much causes bouncing when standing still
// Too little and we dont walk up steps :)
#define STAIR_POS	1.00f
// Too little and we have lots of friction on flat planes
// Too much causes jumping up stairs
#define STAIR_VEL	0.25f
// When colliding with a wall, velocity towards the wall is clipped
// This is extra clipping past what is necessary, making you stay further away from walls
#define BOUNCE		1.2f

extern double com_maxfps;

static unsigned char huffbuf[HUFFHEAP_SIZE];

Engine::Engine()
{
	initialized = false;
	max_dynamic = 100; // 300 is causing network issues
	max_player = 8;
	max_sources = 32;
	cl_skip = 0;
	sv_maxclients = 8;
	show_names = false;
	show_lines = false;
	show_debug = false;
	show_hud = true;
	entities_enabled = true;
	collision_detect_enable = true;
	num_bot = 0;
	emitter.enabled = false;
	emitter_count = 0;
	demo = false;
	voted = false;
	shadowmaps = false;
	recording_demo = false;
	playing_demo = false;
	node = NULL;
	no_tex = 0;
	depth_tex = 0;
	demofile = NULL;
	multisample = 1;
	lightning_vbo = 0;
	lightning_ibo = 0;
	last_server_sequence = 0;
	client_flag = false;
	global_vao = 0;
	fbo = 0;
	game = NULL;
	num_pk3 = 0;
	num_shader = 0;
	param1 = NULL;
	param2 = NULL;
	particle_tex = 0;
	qport = 0;
	quad_tex = 0;
	ref = NULL;
	thug22 = NULL;
	sensitivity = 1.0f;
	sequence = 0;
	server_flag = false;
	server_spawn = -1;
	spiral_vbo = 0;
	spiral_ibo = 0;
	all_lights = false;
	ingame_menu_timer = 0;
	fullscreen_timer = 0;

	fb_width = 1024;
	fb_height = 1024;
	res_scale = 1.0f;
	dynamic_resolution = false;
	xres = 0;
	yres = 0;
	tick_num = 0;
	testObj = 0;
	num_light = 0;
	doom_sound = 0;
	enable_portal = false;
#ifdef OPENGL
	render_mode = MODE_INDIRECT;
#else
	render_mode = MODE_FORWARD;
#endif
	num_hash = 0;
	net_port = 65535;

	sprintf(sv_hostname, "servername");
	sprintf(sv_motd, "Write engines, not games");
	sprintf(password, "iddqd");
	memset(&netinfo, 0, sizeof(netinfo));

	fov = 50.0f;
	zNear = 1.0f;
	zFar = 2001.0f; // zFar - zNear makes nice values
	inf = true; // above ignored if true
}

/*
void set_view_info(vector *loc, angvec *ang);
void qmap_set_output(char *where, int row);

int qbsp_init()
{
QBsp bsp;

vector loc;
dplane_t pl;

loc.x = 500;
loc.y = 240;
loc.z = 100;

pl.dist = 0.0f;
pl.normal[0] = 0.0f;
pl.normal[1] = 0.0f;
pl.normal[2] = 1.0f;

angvec ang;

ang.tx = 0.0f;
ang.ty = 0.0f;
ang.tz = 0.0f;
set_view_info(&loc, &ang);

bsp.load("start.bsp");


char *scr_buf = (char *)malloc(320 * 200);
int scr_row = 320;
qmap_set_output(scr_buf, scr_row);

bsp.bsp_render_world(&loc, &pl);

FILE *fp = fopen("out.bin", "w");
fwrite(scr_buf, 320 * 240, 1, fp);
fclose(fp);
return 0;
}
*/

void Engine::init(void *p1, void *p2, char *cmdline)
{
	float ident[16] = {	1.0f, 0.0f, 0.0f, 0.0f,
						0.0f, 1.0f, 0.0f, 0.0f,
						0.0f, 0.0f, 1.0f, 0.0f,
						0.0f, 0.0f, 0.0f, 1.0f};

	Engine::param1 = p1;
	Engine::param2 = p2;
	initialized = true;

	srand((unsigned int)time(NULL));
	qport = rand();

#ifdef OPENGL
	render_mode = MODE_INDIRECT;
	//glEnable(GL_STENCIL_TEST);
	glStencilMask(0x00); // disable writes to stencil
	glClearStencil(0x00); // clear stencil to zero
	glStencilFunc(GL_ALWAYS, 0xFF, 0xFF);; // always pass stencil, set to 0xFF
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); // replace if 0xFF if passes
#endif

	multisample = 0;
	sensitivity = 1.0f;

	shadow_light = 107;

#ifdef OPENMP
	omp_set_num_threads(8);
#endif

#ifdef G_QUAKE3
	game = new Quake3();
#endif

#ifdef G_COMMANDO
	game = new Commando();
#endif

	debugf("altEngine2 built %s\n", __DATE__);
	bind_keys();

	identity = ident;
	projection = ident;

	//visual
	gfx.init(param1, param2);

	gfx.CreateVertexArrayObject(global_vao);
	gfx.SelectVertexArrayObject(global_vao);

	gen_spiral(gfx, spiral_ibo, spiral_vbo);
	gen_lightning(gfx, lightning_ibo, lightning_vbo);


	// hash check data files
	newlinelist("media/cmdlist.txt", cmd_list, num_cmd);
	newlinelist("media/pk3list.txt", pk3_list, num_pk3);
	newlinelist("media/pk3hash.txt", hash_list, num_hash);

	if (num_pk3 != num_hash)
	{
		printf("Error: num_pk3 %d num_hash %d, should match\n", num_pk3, num_hash);
		exit(0);
	}


#if NDEBUG
	char hash[32][128];
	std::thread pool[32];
	/*
	if ( check_hash(APP_NAME, APP_HASH, hash) == false)
	{
		printf("Program code failed hash check!\n");
		exit(0);
	}*/


	for (unsigned int i = 0; i < num_pk3 && i < num_hash; i++)
	{
		pool[i] = std::thread(calc_hash, pk3_list[i], &hash[i][0]);
	}

	for(unsigned int i = 0; i < num_pk3 && i < num_hash; i++)
	{
		pool[i].join();
		debugf("Checking hash for %s...", pk3_list[i]);
		if (strcmp(hash_list[i], hash[i]) != 0)
		{
			if (strcmp(pk3_list[i], "media/pak0.pk3") == 0)
			{
				if (strcmp(hash[i], "0613b3d4ef05e613a2b470571498690f") == 0)
				{
					debugf("pak0.pk3 is from Q3A Demo\n");
					demo = true;
				}
				else
				{
					debugf("\n%s failed hash check:\n[%s] expected [%s]\n", pk3_list[i], hash, hash_list[i]);
				}
			}
			else
			{
				debugf("\n%s failed hash check:\n[%s] expected [%s]\n", pk3_list[i], hash, hash_list[i]);
			}
		}
		else
		{
			debugf("Good!\n");
		}
	}
#endif

	no_tex = load_texture(gfx, "media/notexture.tga", false, false);
	particle_tex = load_texture(gfx, "media/flare.png", false, false);
	Model::CreateObjects(gfx);
	Model::make_skybox(gfx);

	thug22 = new Entity();
	thug22->rigid = new RigidBody(thug22);
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

	global.init(&gfx);
	audio.init();

	wave_t wave;
	waveFormat_t format;
	wave.format = &format;

	int lump_size = 0;
	//DSSHOTGN
	//DSDMPAIN
	char *sound_lump = get_wadfile("media/DOOM1.WAD", "DSTELEPT", &lump_size);
	if (sound_lump != NULL)
	{
		lump_to_wave(sound_lump, lump_size, &wave);
		audio.load_doom(wave, &doom_sound);
	}


	menu.init(&gfx, &audio, pk3_list, num_pk3);


	for (int i = 0; i < max_sources; i++)
	{
		audio_source[i] = audio.create_source(false, false);
		global_source[i] = audio.create_source(false, true);
		audio_loop_source[i] = audio.create_source(true, false);
		global_loop_source[i] = audio.create_source(true, true);
	}


	set_reference_distance(100.0f);


	game->init(this);

	if (demo)
	{
		menu.load("media/newmenu.txt", "media/newstate.txt");
	}
	else
	{
		menu.load("media/fullmenu.txt", "media/fullstate.txt");
	}

	//net crap
	sequence = 0;
	server_flag = false;
	client_flag = false;
	memset(&reliable, 0, sizeof(reliablemsg_t));

	for (unsigned int i = 0; i < max_player; i++)
	{
		reliable[i].sequence = -1;
	}
	last_server_sequence = 0;
	testObj = 0;

	printf("Loading md5 models...\n");
	load_md5();



	fb_width = (unsigned int)(1024 * res_scale);
	fb_height = (unsigned int)(1024 * res_scale);
	gfx.setupFramebuffer(fb_width, fb_height, fbo, quad_tex, depth_tex, multisample);

	//parse shaders
	printf("Loading Quake3 shaders...\n");
//	newlinelist("media/shaderlist.txt", shader_list, num_shader);
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
			delete [] shader_file;
		}
	}

	printf("Done\n");

#ifndef DIRECTX
	//render menu again for linux
	gfx.resize(xres,yres);
	menu.render(global);
	gfx.swap();
#endif
	
	if (render_mode == MODE_INDIRECT)
	{
		//Setup render to texture
		gfx.bindFramebuffer(fbo);
		gfx.resize(fb_width, fb_height);
		gfx.bindFramebuffer(0);

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

//	shadowmap.init(&gfx);
}

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

void Engine::load(char *level)
{
	matrix4 transformation;
	gametype_t gametype = GAMETYPE_DEATHMATCH;

	if (q3map.loaded)
		return;

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



	//First n Entities for dynamic items (Dont allocate any at runtime)
	for (unsigned int i = 0; i < max_dynamic; i++)
	{
		Entity *entity = new Entity();
		memcpy(entity->type, "free", 5);
		entity_list.push_back(entity);

		if (i < max_player && (server_flag || playing_demo))
		{
			// Forces server to expect player rigid bodies
			entity->rigid = new RigidBody(entity);

			if (playing_demo)
			{
				entity->player = new Player(entity, gfx, audio, 21, TEAM_NONE, ENT_PLAYER, game->model_table);
				if (i == 0)
					sprintf(entity->type, "player");
			}
		}
	}

	if (post.init(&gfx))
		menu.print("Failed to load post shader");
	if (mlight2.init(&gfx))
		menu.print("Failed to load mlight2 shader");
	if (particle_render.init(&gfx))
		menu.print("Failed to load particle_render shader");
	if (particle_update.init(&gfx))
		menu.print("Failed to load particle_update shader");


	mlight2.set_contrast(2.0);
	mlight2.set_ambient(1.0);
	mlight2.set_lightmap(0.5);
	mlight2.set_max(64);


	emitter.position = vec3(0.0f, 100.0f, 0.0f);
	emitter.vel_min = vec3(50.0f, 50.0f, 50.0);
	emitter.vel_range = vec3(200.0f, 200.0f, 200.0f);
	emitter.color = 0x00FF00FF;
	emitter.size = 2.5f;
	emitter.life_min = 0.5f;
	emitter.life_range = 5.0f;
	// max particle testing
//	emitter.life_min = 50000000000.0f;
//	emitter.life_range = 50000000000.0f;
	emitter.gravity = vec3(0.0f, -GRAVITY, 0.0f);
	emitter.delta_time = 0.008f;
	emitter.num = 1;


	emitter.seed = vec3(rand_float(-100, 200.0f),
		rand_float(-100.0, 200.0f),
		rand_float(-100.0, 200.0f));


	if ( q3map.load(level, pk3_list, num_pk3) == false)
		return;

	q3map.generate_meshes(gfx);


	char entfile[128] = { 0 };
	sprintf(entfile, "media/%s.ent", q3map.map_name);
	char *entdata = get_file(entfile, NULL);
	if (entdata != NULL)
	{
		parse_entity(this, entdata, entity_list, gfx, audio);
		delete [] entdata;
	}
	else
	{
		char filename[128];
		const char *data = q3map.get_entities();

		sprintf(filename, "media/%s.ent", q3map.map_name);
		write_file(filename, data, strlen(data));
		parse_entity(this, q3map.get_entities(), entity_list, gfx, audio);
	}



	int start = entity_list.size();

	char navfile[128] = { 0 };
	sprintf(navfile, "media/%s.nav", q3map.map_name);
	char *navdata = get_file(navfile, NULL);
	if (navdata != NULL)
	{
		parse_entity(this, navdata, entity_list, gfx, audio);
		delete [] navdata;
	}

	int num_node = entity_list.size() - start;

	navdata_to_graph(ref, node, entity_list, start);
	print_graph(node, num_node);

	graph.load(node, num_node);
	delete [] ref;


	game->setup_func(entity_list, q3map);

	menu.delta("entities", *this);
	gfx.clear();
	menu.render(global);
	gfx.swap();
	load_entities();

	// This renders map before loading textures
	camera_frame.set(transformation);
	matrix4 mvp = transformation * projection;

	spatial_testing();
	gfx.clear();
	global.Select();
	global.Params(mvp, 0);
	gfx.SelectTexture(0, no_tex);
	gfx.SelectTexture(1, no_tex);
	gfx.SelectTexture(2, no_tex);
	gfx.SelectTexture(3, no_tex);

	render_entities(transformation, projection, true, false);

	q3map.render(camera_frame.pos, mvp, gfx, surface_list, mlight2, tick_num);
	q3map.lastIndex = -2; // force generation of new face lists
	camera_frame.set(transformation);

	render_entities(transformation, projection, true, true);

	menu.delta("textures", *this);
	menu.render(global);
	gfx.swap();

	q3map.load_textures(gfx, surface_list, pk3_list, num_pk3);
	menu.delta("loaded", *this);
	menu.stop();
	menu.ingame = false;
	menu.console = false;
	menu.chat = true;


	//Setup render to texture

	// Generate depth cubemaps for each light
	if (render_mode == MODE_INDIRECT)
	{
		render_shadowmaps(true);
		gfx.bindFramebuffer(0);
	}


	if (render_mode == MODE_SHADOWVOL)
	{
		for (unsigned int i = 0; i < entity_list.size(); i++)
		{
			if (entity_list[i]->light)
			{
				entity_list[i]->light->generate_volumes(q3map);
				//			entity_list[i]->rigid->angular_velocity f= vec3();
			}
		}
	}

	// render portals at least once
	render_portalcamera();

}

void Engine::load_md5()
{
	char **animation = NULL;

	animation = new char *[50];
	animation[0] = "media/md5/chaingun_stand_fire.md5anim";
	animation[1] = "media/md5/chaingun_idle.md5anim";
	zcc.load("media/md5/zcc.md5mesh", (char **)animation, 2, gfx);
/*
	animation[0] = "media/md5/sentry/initial.md5anim";
	animation[1] = "media/md5/sentry/fold.md5anim";
	animation[2] = "media/md5/sentry/folded.md5anim";
	animation[3] = "media/md5/sentry/idle_stand1.md5anim";
	animation[4] = "media/md5/sentry/range_attack1.md5anim";
	animation[5] = "media/md5/sentry/range_attackend.md5anim";
	animation[6] = "media/md5/sentry/range_attackstart.md5anim";
	animation[7] = "media/md5/sentry/talk_primary.md5anim";
	animation[8] = "media/md5/sentry/turn_left.md5anim";
	animation[9] = "media/md5/sentry/turn_right.md5anim";
	animation[10] = "media/md5/sentry/unfold.md5anim";
	animation[11] = "media/md5/sentry/walk1.md5anim";
	animation[12] = "media/md5/sentry/walk1_pain.md5anim";
	sentry.load("media/md5/sentry/sentry.md5mesh", (char **)animation, 13, gfx);

	animation[0] = "media/md5/sentry/initial.md5anim";
	animation[1] = "media/md5/sentry/fold.md5anim";
	animation[2] = "media/md5/sentry/folded.md5anim";
	animation[3] = "media/md5/sentry/idle_stand1.md5anim";
	animation[4] = "media/md5/sentry/range_attack1.md5anim";
	animation[5] = "media/md5/sentry/range_attackend.md5anim";
	animation[6] = "media/md5/sentry/range_attackstart.md5anim";
	animation[7] = "media/md5/sentry/talk_primary.md5anim";
	animation[8] = "media/md5/sentry/turn_left.md5anim";
	animation[9] = "media/md5/sentry/turn_right.md5anim";
	animation[10] = "media/md5/sentry/unfold.md5anim";
	animation[11] = "media/md5/sentry/walk1.md5anim";
	animation[12] = "media/md5/sentry/walk1_pain.md5anim";

	int i = 0;
	animation[i++] = "media/md5/zsec_shotgun/af_pose.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/crate_up_A.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/crate_up_B.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/crate_up_C.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/crate_up_D.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/crouch_range_attack.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/crouch_range_attack_aim.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/crouch_range_attack_end.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/crouch_range_attack_loop.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/evade_left.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/evade_right.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/leftarmpain.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/lower.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/raise.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/rightarmpain.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/run.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/shotgun_activate_step_right.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/shotgun_crouch_left_pain.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/shotgun_crouch_right_pain.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/shotgun_step_left.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/shotgun_step_right.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/sight.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/stand.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/stand_aim.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/stand_fire.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/walk.md5anim";

	animation[i++] = "media/md5/zsec_shotgun/wallleanleftshotgun_A.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/wallleanleftshotgun_B.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/wallleanleftshotgun_C.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/wallleanleftshotgun_D.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/wallleanrightshotgun_A.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/wallleanrightshotgun_B.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/wallleanrightshotgun_C.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/wallleanrightshotgun_D.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/wallrotleftshotgun_A.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/wallrotleftshotgun_B.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/wallrotleftshotgun_C.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/wallrotleftshotgun_D.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/wallrotrightshotgun_A.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/wallrotrightshotgun_B.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/wallrotrightshotgun_C.md5anim";
	animation[i++] = "media/md5/zsec_shotgun/wallrotrightshotgun_D.md5anim";

	zsec_shotgun.load("media/md5/zsec_shotgun/zsecshotgun.md5mesh", (char **)animation, i - 1, gfx);
	*/
	delete [] animation;
}

void Engine::render(double last_frametime)
{
#ifdef WIN32
	int fpOld, fpNew;
	fpOld = _controlfp(0, 0);
	//fpNew = fpOld & ~(EM_OVERFLOW | EM_UNDERFLOW | EM_INEXACT | EM_ZERODIVIDE | EM_DENORMAL | EM_INVALID);
	fpNew = fpOld & ~(EM_ZERODIVIDE | EM_INVALID);
	_controlfp(fpNew, MCW_EM);
#endif

#ifdef VULKAN
	gfx.render();
#else
#if 0
	//test triangle
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
	gfx.DrawArrayTri(0, 0, 3, 3);
	gfx.swap();
#endif

	if (q3map.loaded == false)
		return;

#ifdef DEDICATED
	server_recv();
	return;
#endif
	if (server_flag)
		server_recv();
	if (client_flag)
		client_recv();

	if (render_mode == MODE_INDIRECT)
	{
		bool depth_view = false;
		int spawn = find_type(ENT_PLAYER, 0);
		Player *player = NULL;

		if (spawn != -1)
			player = entity_list[spawn]->player;



		if (shadowmaps || all_lights)
		{
			// Generate depth cubemaps for each light
			render_shadowmaps(all_lights); // done at load time
			gfx.bindFramebuffer(0);
		}
		if (enable_portal)
			render_portalcamera();

		render_to_framebuffer(last_frametime);

		gfx.bindFramebuffer(0);
		gfx.clear();

		if (spawn == -1 || (player && player->current_light == 0))
		{
			render_texture(quad_tex, false);
		}
		else
		{
			for (unsigned int i = max_dynamic; i < entity_list.size(); i++)
			{

				if (spawn == -1)
					break;

				if (player == NULL)
					break;

				if (entity_list[i]->light)
				{
					if (entity_list[i]->light->light_num == player->current_light)
					{
						if (input.scores)
						{
							shadow_light = i;
							printf("selecting light %d for shadows\n", i);
						}

						if (input.duck == false)
						{
							depth_view = true;
							testObj = entity_list[i]->light->depth_tex[player->current_face];
						}
						else
						{
							printf("light %d %d\n", i, entity_list[i]->light->light_num);
							testObj = entity_list[i]->light->quad_tex[player->current_face];
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
	if (render_mode == MODE_FORWARD)
	{
		gfx.clear();
		render_scene(true);

		//render menu
		if (menu.chatmode == false)
			game->render_hud(last_frametime);
		if (menu.ingame)
			menu.render(global);
		if (menu.console)
			menu.render_console(global);
		if (menu.chatmode)
			menu.render_chatmode(global);

		gfx.Depth(true);
		gfx.Blend(false);
	}


	if (render_mode == MODE_SHADOWVOL)
	{
		matrix4 mvp;

		gfx.clear();
		gfx.Blend(true);
		render_scene(false); // render without lights, fill stencil mask, render with lights
		gfx.Blend(false);

		gfx.Color(false);
		gfx.Stencil(true);
		gfx.Depth(false);
		gfx.StencilFunc("always", 0, 0);

		gfx.StencilOp("keep", "keep", "incr"); // increment shadows that pass depth
		render_shadow_volumes(0);

		gfx.StencilOp("keep", "keep", "decr"); // decrement shadows that backface pass depth
		gfx.CullFace(1);
		render_shadow_volumes(0);

		gfx.Depth(true);
		gfx.Color(true);
		gfx.CullFace(0);

		gfx.DepthFunc("<="); // is this necessary?
		gfx.StencilOp("keep", "keep", "keep");

		//all lit surfaces will correspond to a 0 in the stencil buffer
		//all shadowed surfaces will be one
		gfx.StencilFunc("equal", 0, 1);
		// render with lights
		gfx.cleardepth();
		render_scene(true);


		gfx.DepthFunc("<");
		gfx.Stencil(false);

		//render menu
		if (menu.chatmode == false)
			game->render_hud(last_frametime);
		if (menu.ingame)
			menu.render(global);
		if (menu.console)
			menu.render_console(global);
		if (menu.chatmode)
			menu.render_chatmode(global);
	}


#ifdef DIRECTX
	gfx.cleardepth();
	gfx.Blend(false);
#endif

	gfx.swap();
#endif
#ifdef WIN32
	_controlfp(fpOld, MCW_EM);
#endif
}

void Engine::zoom(float level)
{
	projection.perspective(fov / level, (float)xres / yres, zNear, zFar, inf);
}

void Engine::render_portalcamera()
{
	q3map.lastIndex = -1;

	for (unsigned int i = max_dynamic; i < entity_list.size(); i++)
	{
		PortalCamera *portal = entity_list[i]->portal_camera;
		if (portal == NULL)
			continue;

		// Portal surface with target use misc_portal_camera, else it's a mirror
		if (entity_list[i]->target[0] != '\0' && entity_list[i]->ent_type == ENT_MISC_PORTAL_SURFACE)
			continue;

		if (entity_list[i]->ent_type == ENT_MISC_PORTAL_CAMERA)
			continue;

		matrix4 matrix;

		// generate matrices for each light face

		switch (entity_list[i]->angle)
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

		matrix4 mvp = matrix * portal->portal_projection;

		gfx.bindFramebuffer(portal->fbo);
		gfx.resize(fb_width, fb_height);
		gfx.fbAttachTexture(portal->quad_tex);
		gfx.fbAttachDepth(portal->depth_tex);
		gfx.clear();

		vec3 offset(0.0f, 0.0f, 0.0f);
		mlight2.Select();
		mlight2.Params(mvp, light_list, light_list.size(), offset, tick_num);
		q3map.render(entity_list[i]->position, mvp, gfx, surface_list, mlight2, tick_num);
		render_entities(matrix, portal->portal_projection, false, false, false);
		render_players(matrix, portal->portal_projection, false, true);
		gfx.bindFramebuffer(0);
		//gfx.SelectShader(0);
		//gfx.Color(true);
	}

	q3map.lastIndex = -1;
}


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
		Light *light = entity_list[i]->light;
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
			/*
			bool visible = aabb_visible(camera_frame.pos, camera_frame.pos + camera_frame.forward + camera_frame.up, mvp);

			if (visible == false)
				continue;
			*/

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
			gfx.resize(fb_width, fb_height);
			gfx.clear();

			vec3 offset(0.0f, 0.0f, 0.0f);
			mlight2.Params(mvp, light_list, 0, offset, tick_num);

			light->shadow_matrix[j] = (cube[j] * light->shadow_projection) * bias;

			q3map.render(entity_list[i]->position, mvp, gfx, surface_list, mlight2, tick_num);
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

void Engine::set_dynamic_resolution(double last_frametime)
{
	if (q3map.loaded)
	{
		double fps = 1000.0 / last_frametime;
		if (fps < 60.0f && res_scale > 0.1f)
		{
			res_scale *= 0.75f;
			fb_width = (unsigned int)(1024 * res_scale);
			fb_height = (unsigned int)(1024 * res_scale);
			gfx.setupFramebuffer(fb_width, fb_height, fbo, quad_tex, depth_tex, multisample);
		}
		else if (fps > 100.0f && res_scale < 2.0f)
		{
			res_scale *= 1.25f;
			fb_width = (unsigned int)(1024 * res_scale);
			fb_height = (unsigned int)(1024 * res_scale);
			gfx.setupFramebuffer(fb_width, fb_height, fbo, quad_tex, depth_tex, multisample);
		}
	}
	else if (q3map.loaded == false && (abs32(res_scale - 1.0f) > 0.001f))
	{
		res_scale = 1.0f;
		fb_width = (unsigned int)(1024 * res_scale);
		fb_height = (unsigned int)(1024 * res_scale);
		gfx.setupFramebuffer(fb_width, fb_height, fbo, quad_tex, depth_tex, multisample);
	}
}


void Engine::render_to_framebuffer(double last_frametime)
{
	if (dynamic_resolution)
		set_dynamic_resolution(last_frametime);

	gfx.bindFramebuffer(fbo);
	gfx.resize(fb_width, fb_height);
	gfx.fbAttachTexture(quad_tex);
	gfx.fbAttachDepth(depth_tex);

	gfx.clear();
	if (shadowmaps)
		render_scene_using_shadowmap(true);
	else
		render_scene(true);
//	glStencilFunc(GL_GREATER, 128, 0xFF); // only pass if greater than 128
//	render_texture(q3map.portal_tex, false);
//	glStencilFunc(GL_ALWAYS, 0xFF, 0xFF);; // always pass stencil, set to 0xFF



/*
	if (spawn != -1)
	{
		render_shadow_volumes(entity_list[spawn]->player->current_light);
	}
	*/

	//render menu
	if (menu.chatmode == false)
		game->render_hud(last_frametime);
	if (menu.ingame)
		menu.render(global);
	if (menu.console)
		menu.render_console(global);
	if (menu.chatmode)
		menu.render_chatmode(global);


	gfx.Depth(true);
	gfx.Blend(false);

	gfx.bindFramebuffer(0);
	gfx.resize(xres, yres);
}

void Engine::render_texture(int texObj, bool depth_view)
{
	gfx.SelectTexture(0, texObj);
	gfx.SelectIndexBuffer(Model::quad_index);
	gfx.SelectVertexBuffer(Model::quad_vertex);
	global.Select();
	global.Params(identity, 0, depth_view);
	gfx.DrawArrayTri(0, 0, 6, 4);
}

void Engine::render_scene(bool lights)
{
	matrix4 transformation;
	matrix4 mvp;
	vec3 offset(0.0f, 0.0f, 0.0f);

	int player = find_type(ENT_PLAYER, 0);
	if (player != -1)
		entity_list[player]->rigid->frame2ent(&camera_frame, input);


	camera_frame.set(transformation);

	// Rendering entities before map for blends
	render_entities(transformation, projection, lights, false);
	render_players(transformation, projection, lights, game->spectator);

	mlight2.Select();
	mvp = transformation * projection;
	if (lights)
		mlight2.Params(mvp, light_list, light_list.size(), offset, tick_num);
	else
		mlight2.Params(mvp, light_list, 0, offset, tick_num);

	q3map.render(camera_frame.pos, mvp, gfx, surface_list, mlight2, tick_num);

#ifdef PARTICLES
	gfx.Blend(true);
	gfx.BlendFunc(NULL, NULL);

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
		particle_render.Params(mvp, quad1, quad2, 0.0f, 0.0f);
		gfx.SelectTexture(0, particle_tex);
		particle_render.render(gfx, 0, vbo, emitter.num);
	}


	render_trails(transformation);
#endif
#endif

	render_entities(transformation, projection, lights, true);
	render_weapon(transformation, lights, player);
}


// Texture arrays let me pass all the depthmaps
// Next problem is the transformed interpolated fragment per face, per light
// Right now I can only get four shadow omni-lights with this limit
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
		Light *light = entity_list[i]->light;
		if (light == NULL)
			continue;

		if (entity_list[i]->bsp_visible == false)
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
	render_players(transformation, projection, lights, game->spectator);
	mvp = transformation * projection;
	if (lights)
		mlight2.Params(mvp, light_list, light_list.size(), offset, tick_num);
	else
		mlight2.Params(mvp, light_list, 0, offset, tick_num);


	q3map.render(camera_frame.pos, mvp, gfx, surface_list, mlight2, tick_num);

#ifdef PARTICLES
	gfx.Blend(true);
	gfx.BlendFunc(NULL, NULL);

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
		particle_render.Params(mvp, quad1, quad2, 0.0f, 0.0f);
		gfx.SelectTexture(0, particle_tex);
		particle_render.render(gfx, 0, vbo, emitter.num);
	}


	render_trails(transformation);
#endif
#endif

	render_entities(transformation, projection, lights, true);
	render_weapon(transformation, lights, player);

}

void Engine::render_weapon(const matrix4 &trans, bool lights, int i)
{
	matrix4 mvp;

	if (i == -1)
		return;

	vec3 offset = entity_list[i]->position;

	if (entity_list[i]->player == NULL)
		return;


	entity_list[i]->rigid->get_matrix(mvp.m);
	mvp = (mvp * trans) * projection;

	game->draw_flash(*(entity_list[i]->player));
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
}

void Engine::render_trails(matrix4 &trans)
{
	matrix4 mvp;
	bool once = false;
	vec3 quad1 = camera_frame.up;
	vec3 quad2 = vec3::crossproduct(camera_frame.up, camera_frame.forward);


	//render particle trails first
	for (unsigned int i = max_player; i < max_dynamic; i++)
	{
		Model *model = entity_list[i]->model;

		if (model == NULL)
			continue;



		if (once == false && (model->rail_trail || model->lightning_trail))
		{
			//			vec3 offset = entity_list[i]->position;

			// Undo model orientation
			quad1 = model->morientation.transpose() * quad1;
			quad2 = model->morientation.transpose() * quad2;

			particle_render.Select();
			gfx.SelectTexture(0, particle_tex);
			once = true;
		}


		//render rail trail
		if (entity_list[i]->model->rail_trail)
		{
			entity_list[i]->rigid->get_matrix(mvp.m);
			mvp = (mvp * trans) * projection;
			particle_render.Params(mvp, quad1, quad2, 0.0f, 0.0f);

			particle_render.render(gfx, 0, spiral_vbo, 400);
			continue;
		}

		//render lightning trail
		if (entity_list[i]->model->lightning_trail)
		{
			entity_list[i]->rigid->get_matrix(mvp.m);
			mvp = (mvp * trans) * projection;
			particle_render.Params(mvp, quad1, quad2, 0.0f, 0.0f);

			particle_render.render(gfx, 0, lightning_vbo, 400);
			continue;
		}
	}
}

void Engine::render_entities(const matrix4 &trans, matrix4 &proj, bool lights, bool blend, bool vis)
{
	matrix4 mvp;

	if (entities_enabled == false)
		return;

	emitter.enabled = false;

	for (int i = 1; i < MAX_TEXTURES; i++)
	{
		gfx.SelectTexture(i, 0);
	}

	gfx.Blend(false);
	mlight2.Select();

	for (unsigned int i = 0; i < entity_list.size(); i++)
	{
		Entity *entity = entity_list[i];

		if (entity->rigid == NULL)
			continue;

		if (entity->rigid->blend != blend)
			continue;

		if (entity->ent_type == ENT_FUNC_DOOR)
		{
			entity->visible = true;
			entity->bsp_visible = true;
			//printf("break\n");
		}

		if (vis)
		{
			if (entity->visible == false)
				continue;
		}
		else
		{
			if (entity->bsp_visible == false || (entity->trigger && entity->trigger->active))
				continue;
		}

		if (entity->nodraw == true)
			continue;

		if (entity->model != NULL)
		{
			if (entity->model->rail_trail || entity->model->lightning_trail)
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

		entity->rigid->get_matrix(mvp.m);
		mvp = (mvp * trans) * proj;
		if (lights)
		{
			mlight2.Params(mvp, light_list, light_list.size(), offset, tick_num);
		}
		else
		{
			mlight2.Params(mvp, light_list, 0, offset, tick_num);
		}

		//render entity
		entity->rigid->render(gfx);

		if (entity->model_ref != -1 && q3map.enable_textures)
		{
			Frame frame;

			vec3 old = entity->position;
			entity->position = entity->model_offset;
			entity->rigid->get_matrix(mvp.m);
			mvp = (mvp * trans) * proj;
			mlight2.set_matrix(mvp);
			q3map.render_model(entity->model_ref, gfx);

			entity->position = old;
		}

		//  update emitter position if this entity has particles
		if (entity->particle_on)
		{
			emitter.enabled = true;
			emitter.num = entity->num_particle;
			emitter.position = entity->position;
			emitter.gravity = vec3(0.0f, 30.0f, 0.0f);
			emitter_count = 5 * TICK_RATE;
		}
	}
}

void Engine::render_players(matrix4 &trans, matrix4 &proj, bool lights, bool render_self)
{
	matrix4 mvp;
	//render player md5
	for (unsigned int i = 0; i < max_player && i < entity_list.size(); i++)
	{
		Entity *entity = entity_list[i];

		if (entity == NULL)
			continue;

		if (entity->visible == false && entity->player && entity->player->type != SPECTATOR)
			continue;

		if (	(entity->player && entity->player->type == BOT) ||
			(entity->player && entity->player->type == SPECTATOR) ||
			(entity->player && entity->player->type == SERVER) ||
			(entity->player && entity->player->type == PLAYER) ||
			(entity->player && entity->player->type == CLIENT && server_flag))
		{
			if (entity->player->local && render_self == false)
				continue;

			if (entity->player->health > 0)
			{

				//md5 faces right, need to flip right and forward orientation
				// also forcing to yaw only (no up / down rotation)
				entity->rigid->get_matrix(mvp.m);

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

				mvp.m[0] = forward.x;
				mvp.m[1] = forward.y;
				mvp.m[2] = forward.z;

				mvp.m[4] = up.x;
				mvp.m[5] = up.y;
				mvp.m[6] = up.z;

				mvp.m[8]  = right.x;
				mvp.m[9]  = right.y;
				mvp.m[10] = right.z;


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
				zcc.render(gfx, tick_num >> 1);
			}
		}
	}
}


void Engine::render_shadow_volumes(int current_light)
{
	matrix4 mvp;
	matrix4 transformation;

	for (unsigned int i = 0; i < entity_list.size(); i++)
	{
		if (entity_list[i]->light)
		{
			camera_frame.set(transformation);

			mvp = transformation * projection;
			global.Select();
			global.Params(mvp, 0);
			entity_list[i]->light->render_shadow_volumes(current_light);
//			gfx.SelectShader(0);
		}
	}
}

void Engine::screenshot()
{
	static int num = 0;
	char filename[128];
#ifdef WIN32
	GetScreenShot(*((HWND *)param1));
	sprintf(filename, "Screenshot%d.bmp", num++);
	menu.print(filename);
	return;
#else
	int width = fb_width;
	int height = fb_height;
	char *pixel = new char[4 * width * height];
	char *bmp = new char[4 * width * height];

//	gfx.bindFramebuffer(0);
//	gfx.resize(width, height);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
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

void Engine::post_process(int num_passes)
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
		post.Params(0, 1);
		gfx.clear();
		gfx.SelectIndexBuffer(Model::quad_index);
		gfx.SelectVertexBuffer(Model::quad_vertex);
		gfx.DrawArrayTri(0, 0, 6, 4);
//		gfx.SelectShader(0);
//		gfx.DeselectTexture(1);
	}
	temp = post.swap;
	post.swap = post.image;
	post.image = temp;
//	gfx.DeselectTexture(0);
}

void Engine::destroy_buffers()
{
	zcc.destroy_buffers(gfx);
	sentry.destroy_buffers(gfx);
	zsec_shotgun.destroy_buffers(gfx);

	gfx.DeleteFrameBuffer(fbo);

	key_bind.destroy();

	for (unsigned int i = 0; i < snd_wave.size(); i++)
	{
		delete [] snd_wave[i].data;
	}
	snd_wave.clear();
}

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

	#pragma omp parallel for num_threads(8)
	for (unsigned int i = 0; i < entity_list.size(); i++)
	{
#ifdef OPENMP
		int thread_num = omp_get_thread_num();
		int num_thread = omp_get_num_threads();

		if (entity_list[i]->bsp_leaf % num_thread != thread_num)
			continue;

//		printf("bsp leaf %d Handled by thread %d of %d\n", entity_list[i]->bsp_leaf, thread_num, num_thread);
#endif

		// set pursue / evade
		// (really need to move elsewhere, but had an entity loop here)
		RigidBody *rigid = entity_list[i]->rigid;
		if (rigid)
		{
			if (rigid->target)
			{
				if (rigid->pursue_flag == true)
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
			bool bsp_visible = false;
			//bool frustum_visible = false;
			bool visible = false;


			matrix4 trans;

			camera_frame.set(trans);
			matrix4 mvp;
			entity_list[i]->rigid->get_matrix(mvp.m);
			mvp = (mvp * trans) * projection;

			Model *model = entity_list[i]->model;


			if (model)
			{
				vec3 min = model->aabb[0];
				vec3 max = model->aabb[7];

				if (model->rail_trail)
				{
					entity_list[i]->frustum_visible = true; // trail extends past aabb
				}
				else
				{
					entity_list[i]->frustum_visible = aabb_visible(min, max, mvp);
				}
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

			entity_list[i]->bsp_visible = bsp_visible;

			int player = find_type(ENT_PLAYER, 0);

			if (player != -1)
				entity_list[player]->bsp_leaf = leaf_a;
			entity_list[i]->bsp_leaf = leaf_b;

			if ((bsp_visible && entity_list[i]->frustum_visible) || i == (unsigned int)player)
			{
				visible = true;
			}


			// make triggered entities disappear
			Trigger *trigger = entity_list[i]->trigger;
			if (trigger)
			{
				if (trigger->hide == false)
				{
					//Always show with hide flag false
					entity_list[i]->visible = visible;
				}
				else if (trigger->active == false)
				{
					//Only show when not triggered
					entity_list[i]->visible = visible;
				}
			}
			else
			{
				entity_list[i]->visible = visible;
			}
		}
		else
		{
			// Lights? what else?

			int player = find_type(ENT_PLAYER, 0);

			if (player != -1)
				entity_list[player]->bsp_leaf = leaf_a;

			entity_list[i]->visible = q3map.vis_test(camera_frame.pos, entity_list[i]->position, leaf_a, leaf_b);
			entity_list[i]->bsp_leaf = leaf_b;
		}

		if (entity_list[i]->visible == false)
			continue;

		Light *light = entity_list[i]->light;
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
				}
			}
			vec3 dist_vec = entity_list[i]->position - camera_frame.pos;
			float distance = dist_vec.x * dist_vec.x + dist_vec.y * dist_vec.y + dist_vec.z * dist_vec.z;

			activate_light(distance, light);
		}

	}

}

void Engine::activate_light(float distance, Light *light)
{
//	float min_distance = FLT_MAX;

	if (distance < 9 * 800.0f * 800.0f && light->entity->visible)
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

/*
	This function can be threaded by:
	1. Dividing particles by bsp leafs
	   call find leaf for each vertex in aabb
	   if an objects aabb is in multiple leafs
	   union objects in both leafs together
	2. create a thread for each leaf batch.
	3. compute naive O(n^2)
	   each core should be able to handle 1000+ objects in 16ms, so no worrys
	   write results
	4. join threads
*/
void Engine::dynamics()
{

	#pragma omp parallel for num_threads(8)
	for (unsigned int i = 0; i < entity_list.size(); i++)
	{
#ifdef OPENMP
		int thread_num = omp_get_thread_num();
		int num_thread = omp_get_num_threads();

		if (entity_list[i]->bsp_leaf % num_thread != thread_num)
			continue;

		printf("bsp leaf %d Handled by thread %d of %d\n", entity_list[i]->bsp_leaf, thread_num, num_thread);
#endif

		if (entity_list[i]->rigid == NULL)
			continue;

		if (entity_list[i]->nodraw)
			continue;

		if (collision_detect_enable == false && i >= max_player)
			continue;

		RigidBody *body = entity_list[i]->rigid;

		float delta_time = TICK_MS / 1000.0f;
		float target_time = delta_time;
		float current_time = 0.0f;
		int divisions = 0;

		if (body->water)
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

			if (entity_list[i]->model_ref != -1)
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
					body->sleep = true;
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
			body->on_ground = false;
		}

		body->net_force = vec3(0.0f, 0.0f, 0.0f);
	}
}


void ClipVelocity(vec3 &in, vec3 &normal)
{
	float	backoff;
	vec3	change;
	float	overbounce = BOUNCE;

	backoff = in * normal;
	change = (normal * backoff) * overbounce;
	in -= change;
}

/*
	Handles all collision detection
	return true if simulated too far.
	return false if collision handled.
*/
bool Engine::collision_detect(RigidBody &body)
{
//	Plane plane(vec3(0.0f, 1.0f, 0.0f).normalize(), 500.0f);

	if (map_collision(body))
	{
		return true;
	}

	if (body.entity->player)
	{
		if (body_collision(body))
			return true;
	}

	return false;
}

bool Engine::map_collision(RigidBody &body)
{
	Plane plane;
	float depth;
	vec3 staircheck(0.0f, STAIR_HEIGHT, 0.0f);
	vec3 clip(0.0f, 0.0f, 0.0f);
	bool collision = false;
	vec3 mid[4];



	if (body.noclip)
		return false;

	if (body.entity->player)
	{
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

		if (i < 8)
		{
			point = body.aabb[i] - body.center + body.entity->position;
			oldpoint = body.aabb[i] - body.center + body.old_position;
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

		if (q3map.collision_detect(point, oldpoint, (plane_t *)&plane, &depth, body.water, body.water_depth,
			surface_list, body.step_flag && input.use, clip, body.velocity, body.lava, body.slime, body.bsp_trigger_volume))
		{
			if (body.step_flag)
			{
				// Moving, on ground plane, if we get clipped below velocity threshold and will fail to climb stairs
				// So dont mess up :)
				if ( (abs32(body.velocity.x) > 0.25f || abs32(body.velocity.z) > 0.25f))
				{
					vec3 p = point + staircheck;
					//vec3 old = oldpoint + staircheck;

					if (q3map.collision_detect(p, oldpoint, (plane_t *)&plane, &depth, body.water, body.water_depth,
						surface_list, body.step_flag && input.use, clip, body.velocity, body.lava, body.slime, body.bsp_trigger_volume) == false)
					{
						body.entity->position += vec3(0.0f, STAIR_POS, 0.0f);
						body.velocity += vec3(0.0f, STAIR_VEL, 0.0f);
						body.on_ground = true;

						if (body.entity->player)
						{
							float speed = body.velocity.magnitude();
							float speed_scale = 1.2f;

							if (body.entity->player->haste_timer > 0)
								speed_scale = 2.0f;
#define MAX_SPEED 3.0

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

			body.entity->position = body.old_position;
			body.morientation = body.old_orientation;
//			body.impulse(plane, point);
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
		body.on_ground = true;
		if (body.velocity.y <= -RIGID_IMPACT)
		{
			body.hard_impact = true;
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

//O(N^2)
bool Engine::body_collision(RigidBody &body)
{
	for(unsigned int i = 0; i < max_player; i++)
	{
		if (entity_list[i] == body.entity)
			continue;
		if (entity_list[i]->rigid == NULL)
			continue;

//		if (body.entity->bsp_leaf == entity_list[i]->bsp_leaf)
		if (body.collision_distance(*entity_list[i]->rigid))
			return true;

	}
	return false;
}

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
			break;

		int texunit = tex->anim_unit;

		if (tex->freq == 0)
			continue;

		int ani_index = (tick_num % (TICK_RATE / tex->freq)) % tex->num_anim;

		tex->texObj[texunit] = tex->texObjAnim[ani_index];
	}

	spatial_testing(); // mostly sets visible flag
#endif

	//network
	sequence++;
	if (server_flag && sequence)
	{
		server_recv();
		server_send();
	}
	else if (client_flag && sequence)
	{
		client_recv();
		client_send();
	}


	if (playing_demo)
	{
		demo_frameheader_t header;
		static unsigned char data[4096];
		unsigned int num_read;

		memset(data, 0, 4096);
		fread(&header, sizeof(demo_frameheader_t), 1, demofile);
		if (strcmp(header.magic, "frame") != 0)
		{
			playing_demo = false;
			unload();
			menu.print("Invalid frame");
			return;
		}
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
		return;
	}

	game->step(tick);

	dynamics();

	if (recording_demo)
	{
		static servermsg_t servermsg;
		demo_frameheader_t header;

		memcpy(header.magic, "frame", 6);
		serialize_ents(servermsg.data, servermsg.num_ents);
		header.num_ents = servermsg.num_ents;
		header.tick_num = tick_num;

		fwrite(&header, sizeof(demo_frameheader_t), 1, demofile);
		fwrite(&servermsg, servermsg.num_ents * sizeof(entity_t), 1, demofile);
	}

#ifndef DEDICATED
	update_audio();
#endif
}

void Engine::server_send_state(int client)
{
	serverdata_t data;
	int offset = strlen(reliable[client].msg) + 1;
	game->get_state(&data);


	reliable[client].msg[offset - 1] = ' ';
	memcpy(&reliable[client].msg[offset], &data, sizeof(serverdata_t));
	reliable[client].size = 2 * sizeof(int) + sizeof(serverdata_t) + offset;
	reliable[client].sequence = sequence;
}


void Engine::set_spawn_string(char *msg, client_t *client)
{
	sprintf(msg, "<spawn> %d %d ", client->ent_id, find_type(ENT_PLAYER, 0));
	for (unsigned int i = 0; i < client_list.size(); i++)
	{
		char client_index[8];
		if (client_list[i] == client)
			continue;

		sprintf(client_index, "%d ", client_list[i]->ent_id);
		strcat(msg, client_index);
	}
	strcat(msg, "</spawn> ");
}

void Engine::parse_spawn_string(char *msg)
{
	char *start = strstr(msg, "<spawn>");
	char *end = strstr(msg, "</spawn>");
	int count = 0;
	int client;

	if (start && end)
	{
		char temp[512];

		start += 8;
		int length = (int)(end - start);

		memset(temp, 0, sizeof(temp));
		memcpy(temp, start, length);
		char *line = strtok(temp, " ");
		while (line)
		{
			client = atoi(line);
			line = strtok(NULL, " ");

			if (count == 0 && active_clients[count] == false)
			{
				Entity *ent = entity_list[client];

				active_clients[count] = true;
				clean_entity(client);
				sprintf(ent->type, "player");
				ent->rigid = new RigidBody(ent);
				ent->model = ent->rigid;
				ent->rigid->clone(*(thug22->model));
				ent->rigid->step_flag = true;
				ent->position += ent->rigid->center;
				ent->player = new Player(ent, gfx, audio, 21, TEAM_NONE, ENT_PLAYER, game->model_table);
				ent->player->type = PLAYER;
				ent->player->local = true;
				camera_frame.pos = ent->position;
			}
			else
			{
				if (active_clients[count] == false)
				{
					active_clients[count] = true;
					Entity *ent = entity_list[client];
					clean_entity(client);
					sprintf(ent->type, "server");
					ent->rigid = new RigidBody(ent);
					ent->model = ent->rigid;
					ent->rigid->clone(*(thug22->model));
					ent->rigid->step_flag = true;
					ent->position += ent->rigid->center;
					ent->player = new Player(ent, gfx, audio, 21, TEAM_NONE, ENT_SERVER, game->model_table);
					ent->player->local = false;
					ent->player->type = SERVER;
				}
			}
			count++;
		}
	}
}

void Engine::server_recv()
{
	static servermsg_t	servermsg;
	static clientmsg_t clientmsg;
	char socketname[LINE_SIZE];
	bool connected = false;
	int index = -1;

	// get client packet
	int size = net.recvfrom((char *)&clientmsg, 8192, socketname, LINE_SIZE);
	if (size <= 0)
	{
		netinfo.recv_empty = true;
		return;
	}
	netinfo.recv_empty = false;


	if (clientmsg.length != size)
	{
		printf("Packet size mismatch\n");
		return;
	}

	// see if this ip/port combo already connected to server
	for (unsigned int i = 0; i < client_list.size(); i++)
	{
		if (strcmp(client_list[i]->socketname, socketname) == 0)
		{
			if (client_list[i]->qport == clientmsg.qport)
			{
				index = i;
				connected = true;
				break;
			}
		}
	}

	// update client input
	if (connected)
	{
		int keystate;
		memcpy(&keystate, clientmsg.data, 4);
		input_t clientkeys = GetKeyState(keystate);

		if (clientmsg.sequence <= client_list[index]->client_sequence)
		{
			printf("Got old client packet\n");
			client_list[index]->netinfo.dropped++;
			return;
		}

		client_list[index]->server_sequence = clientmsg.server_sequence;
		if (clientmsg.server_sequence > reliable[index].sequence)
		{
			memset(reliable[index].msg, 0, LINE_SIZE);
			reliable[index].size = 0;
			reliable[index].sequence = -1;
		}

		if ((unsigned int)client_list[index]->ent_id > entity_list.size())
		{
			printf("Invalid Entity\n");
			return;
		}

		client_list[index]->last_time = (unsigned int)time(NULL);


		Frame client_frame;
		vec3 client_pos; // dont use, client can cheat, only to see how bad delta is

		client_frame.up.x = clientmsg.up[0];
		client_frame.up.y = clientmsg.up[1];
		client_frame.up.z = clientmsg.up[2];
		client_frame.forward.x = clientmsg.forward[0];
		client_frame.forward.y = clientmsg.forward[1];
		client_frame.forward.z = clientmsg.forward[2];
		vec3 right = vec3::crossproduct(client_frame.up, client_frame.forward);
		right.normalize();

		client_pos.x = clientmsg.pos[0];
		client_pos.y = clientmsg.pos[1];
		client_pos.z = clientmsg.pos[2];

		Entity *client = entity_list[client_list[index]->ent_id];

		client_frame.set(client->rigid->morientation);
		client_frame.pos = client->position + client->rigid->center;
		client_list[index]->input = clientkeys;
		client_list[index]->position_delta = client->position - client_pos;

		if ((unsigned int)clientmsg.length > CLIENT_HEADER + sizeof(int) + sizeof(int) + 1)
		{
			reliablemsg_t *reliablemsg = (reliablemsg_t *)&clientmsg.data[4];

			if (client_list[index]->client_sequence <= reliablemsg->sequence)
			{
				char name[LINE_SIZE] = { 0 };
				char msg[LINE_SIZE] = { 0 };

				debugf("client to server: %s\n", reliablemsg->msg);


				char *start = strstr(reliablemsg->msg, "<chat>");
				if ( start )
				{
					
					sprintf(name, "%s", start + 6);
					char *end = strchr(name, ':');
					if (end != NULL)
					{
						end[0] = '\0';
						sprintf(msg, "say \"%s\"", end + 2);
						//  Echoes chat back to all clients
						chat(name, msg);
					}
				}
			}
		}
		client_list[index]->netinfo.sequence_delta = sequence - clientmsg.sequence;
		client_list[index]->netinfo.ping = netinfo.sequence_delta * TICK_MS;
		client_list[index]->client_sequence = clientmsg.sequence;
	}
	else
	{
		printf("Got input packet from unconnected client %s qport %d\n", socketname, clientmsg.qport);
	}

	// client not in list, check if it is a connect msg
	reliablemsg_t *reliablemsg = (reliablemsg_t *)&clientmsg.data[clientmsg.num_cmds * sizeof(int)];
	if ( strcmp(reliablemsg->msg, "<connect/>") == 0 )
	{
		debugf("client %s qport %d connected\n", socketname, clientmsg.qport);

		if (client_list.size() > sv_maxclients)
		{
			debugf("server full");
			servermsg.sequence = sequence;
			servermsg.client_sequence = clientmsg.sequence;
			servermsg.num_ents = 0;
			sprintf(reliable[index].msg, "full");
			reliable[index].size = (unsigned short)(2 * sizeof(short) + strlen(reliable[index].msg) + 1);
			reliable[index].sequence = sequence;

			memcpy(&servermsg.data[0], &reliable, reliable[index].size);
			servermsg.length = SERVER_HEADER + reliable[index].size;
			net.sendto((char *)&servermsg, servermsg.length, socketname);
			return;
		}

		servermsg.sequence = sequence;
		servermsg.client_sequence = clientmsg.sequence;
		servermsg.num_ents = 0;
		sprintf(reliable[index].msg, "<map>%s</map>", q3map.map_name);
		reliable[index].size = (unsigned short)(2 * sizeof(short) + strlen(reliable[index].msg) + 1);
		reliable[index].sequence = sequence;

		memcpy(&servermsg.data[0], &reliable[index], reliable[index].size);
		servermsg.length = SERVER_HEADER + reliable[index].size;
		net.sendto((char *)&servermsg, servermsg.length, socketname);
		debugf("sent client map data\n");
	}
	else if ( strcmp(reliablemsg->msg, "<spawn/>") == 0 )
	{
		bool found = false;
		unsigned int i = 0;

		for (i = 0; i < client_list.size(); i++)
		{
			if (strcmp(client_list[i]->socketname, socketname) == 0)
			{
				if (client_list[i]->qport == clientmsg.qport)
				{
					found = true;
					client_list[i]->client_sequence = clientmsg.sequence;
					client_list[i]->server_sequence = sequence + 1;
					client_list[i]->last_time = (unsigned int)time(NULL);
					break;
				}
			}
		}

		if (found)
		{
			//printf("Client already spawned, ignoring\n");
			return;
		}
		client_t *client = (client_t *)malloc(sizeof(client_t));
		if (client == NULL)
		{
			debugf("malloc failed allocating client");
			return;
		}

		memset(client, 0, sizeof(client_t));
		client->last_time = (unsigned int)time(NULL);
		strcpy(client->socketname, socketname);
		client->qport = clientmsg.qport;
		client->needs_state = true;


		client_list.push_back(client);
		debugf("client %s qport %d spawned\n", client->socketname, client->qport);
		client->client_sequence = clientmsg.sequence;

		// assign entity to client
		//set to zero if we run out of info_player_deathmatches

		index = (int)client_list.size() - 1;
		char client_name[64];

		sprintf(client_name, "client %d", index);
		game->add_player(entity_list, CLIENT, client->ent_id, client_name);
		printf("client %s qport %d got entity %d\n", socketname, client->qport, client->ent_id);


		servermsg.sequence = sequence;
		servermsg.client_sequence = clientmsg.sequence;
		servermsg.num_ents = 0;





		// let new client know how many players exist
		char spawn_str[256];
		set_spawn_string(spawn_str, client);
		strcat(reliable[index].msg, spawn_str);

		char motd[256];
		sprintf(motd, "<chat>Welcome to %s: %s</chat>", sv_hostname, sv_motd);
		strcat(reliable[index].msg, motd);
		
		reliable[index].size = (unsigned short)(2 * sizeof(unsigned short int) + strlen(reliable[index].msg) + 1);
		reliable[index].sequence = sequence;

		
		memcpy(&servermsg.data[servermsg.num_ents * sizeof(entity_t)], &reliable[index], reliable[index].size);
		servermsg.length = SERVER_HEADER + reliable[index].size;
		net.sendto((char *)&servermsg, servermsg.length, client->socketname);
		debugf("Client is now entity %d\n", client->ent_id);


		// Notify other clients of new player
		for (i = 0; i < client_list.size() - 1; i++)
		{
			set_spawn_string(spawn_str, client_list[i]);
			strcat(reliable[i].msg, spawn_str);

			reliable[i].size = (unsigned short)(2 * sizeof(unsigned short int) + strlen(reliable[i].msg) + 1);
			reliable[i].sequence = sequence;

			memset(&servermsg, 0, sizeof(servermsg));
			servermsg.client_sequence = client_list[i]->client_sequence;
			servermsg.sequence = sequence;
			servermsg.num_ents = 0;
			servermsg.compressed_size = 0;
			servermsg.length = SERVER_HEADER + reliable[i].size;
			memcpy(&servermsg.data[servermsg.num_ents * sizeof(entity_t)], &reliable[i], reliable[i].size);
			net.sendto((char *)&servermsg, servermsg.length, client_list[i]->socketname);
		}

	}
	else if (strcmp(reliablemsg->msg, "getstatus") == 0)
	{
		/*
		"getstatus" responds with all the info that qplug or qspy can see about the server and all connected players.
		Used for getting detailed information after the simple info query.  It is sent along with a challenge string.
		The server will respond with a "getstatusResponse" packet.
		*/
	}
	else if (strcmp(reliablemsg->msg, "getinfo") == 0)
	{
/*
		"getinfo" responds with a short info message that should be enough to determine if a user is interested in a server
		to do a full status.  It is also sent with a challenge string.
		*/

		debugf("getinfo request from %s\n", socketname);
		servermsg.sequence = sequence;
		servermsg.client_sequence = clientmsg.sequence;
		servermsg.num_ents = 0;
		sprintf(reliable[index].msg, "/sv_hostname %s/map %s/players %d/maxplayers %d/gametype %d/fraglimit %d/timelimit %d/capturelimit %d/",
			sv_hostname, q3map.map_name, (int)client_list.size(), max_player, game->gametype, game->fraglimit, game->timelimit, game->capturelimit);
		reliable[index].size = (unsigned short)(2 * sizeof(unsigned short int) + strlen(reliable[index].msg) + 1);
		reliable[index].sequence = sequence;

		memcpy(&servermsg.data[servermsg.num_ents * sizeof(entity_t)], &reliable[index], reliable[index].size);
		servermsg.length = SERVER_HEADER + servermsg.num_ents * sizeof(entity_t) + reliable[index].size;
		net.sendto((char *)&servermsg, servermsg.length, socketname);
	}
	else if (strcmp(reliablemsg->msg, "getchallenge") == 0)
	{
		/*
		"getchallenge" returns a challenge number that can be used in a subsequent connectResponse command.
		We do this to prevent denial of service attacks that flood the server with invalid connection IPs.
		With a challenge, they must give a valid IP address.
		The server will respond with a "challengeResponse" packet.
		*/
	}
	else if (strstr(reliablemsg->msg, "rcon") != 0)
	{
		char cmd[512] = "";
		char pass[512] = "";

		if (sscanf(reliablemsg->msg, "rcon %s %s", &pass[0], &cmd[0]) == 2)
		{
			if (strcmp(pass, password) == 0)
			{
				console(cmd);
			}
		}

		/*
		"rcon" is a remote command to the server.  It's sent as "rcon" followed by the server password,
		followed by the command string to be executed.
		*/
	}


	/*
		"connect" is the first step in a client connecting to a server.  You send the "connect" string followed by the infoString
		containing the protocol version of the client, the qport, the challenge string (obtained via getchallenge), and the userinfo.
		*/

}


int Engine::serialize_ents(unsigned char *data, unsigned short int &num_ents)
{
	for (unsigned int j = 0; j < entity_list.size(); j++)
	{
		entity_t ent;


		ent.id = j;
		ent.type = entity_list[j]->nettype;
		ent.active = 0;

		if (entity_list[j]->trigger)
		{
			if (entity_list[j]->trigger->active)
				ent.active = 1;
			ent.owner = entity_list[j]->trigger->owner;
		}
		RigidBody *rigid = entity_list[j]->rigid;
		if (rigid)
		{
			ent.morientation = rigid->morientation;
			ent.angular_velocity = rigid->angular_velocity;
			ent.velocity = rigid->velocity;
			ent.position = entity_list[j]->position;
		}

		Player *player = entity_list[j]->player;

		if (player != NULL)
		{

			ent.health = player->health;
			ent.armor = player->armor;
			ent.weapon_flags = player->weapon_flags;
			//ent.current_weapon = player->current_weapon;
			ent.ammo_bullets = player->ammo_bullets;
			ent.ammo_shells = player->ammo_shells;
			ent.ammo_rockets = player->ammo_rockets;
			ent.ammo_lightning = player->ammo_lightning;
			ent.ammo_slugs = player->ammo_slugs;
			ent.ammo_plasma = player->ammo_plasma;
		}

		memcpy(&data[j * sizeof(entity_t)], &ent, sizeof(entity_t));
		num_ents++;
	}

//	printf("serialize_ents: size is %d\n", num_ents * sizeof(entity_t));
	return 0;
}

int Engine::deserialize_ents(unsigned char *data, unsigned short int num_ents)
{
	for (int i = 0; i < num_ents; i++)
	{
		entity_t	*ent = (entity_t *)data;

		// dont let bad data cause an exception
		if (ent[i].id >= entity_list.size())
		{
			printf("Invalid entity index, bad packet\n");
			break;
		}


		// Check if an entity is a projectile that needs to be loaded
		if (ent[i].type != entity_list[ent[i].id]->nettype)
		{
			game->make_dynamic_ent(ent[i].type, ent[i].id);
		}

		if (entity_list[ent[i].id]->trigger)
		{
			if (ent[i].active)
				entity_list[ent[i].id]->trigger->active = true;
			else
				entity_list[ent[i].id]->trigger->active = false;

			entity_list[ent[i].id]->trigger->owner = ent[i].owner;
		}

		Player *player = entity_list[ent[i].id]->player;

		if (player != NULL)
		{
			player->health = ent[i].health;
			player->armor = ent[i].armor;
			player->weapon_flags = ent[i].weapon_flags;
			// will force server to sync to our current weapon
			//			entity_list[ent[i].id]->player->current_weapon = ent[i].current_weapon;

			if (ent[i].ammo_bullets - player->ammo_bullets > 1)
				player->ammo_bullets = ent[i].ammo_bullets;
			if (ent[i].ammo_shells - player->ammo_shells > 1)
				player->ammo_shells = ent[i].ammo_shells;
			if (ent[i].ammo_rockets - player->ammo_rockets > 1)
				player->ammo_rockets = ent[i].ammo_rockets;
			if (ent[i].ammo_lightning - player->ammo_lightning > 1)
				player->ammo_lightning = ent[i].ammo_lightning;
			if (ent[i].ammo_slugs - player->ammo_slugs > 1)
				player->ammo_slugs = ent[i].ammo_slugs;
			if (ent[i].ammo_plasma - player->ammo_plasma > 1)
				player->ammo_plasma = ent[i].ammo_plasma;

		}



		if (entity_list[ent[i].id]->rigid)
		{
			if ((unsigned int)find_type(ENT_PLAYER, 0) == ent[i].id)
			{
				// current entity has the clients predicted position
				// the ent[i].position has the server (lagged) position
				// Need to lerp between the two, but then we have time sync issues
				entity_list[ent[i].id]->position = ent[i].position;
				entity_list[ent[i].id]->rigid->velocity = ent[i].velocity;
				camera_frame.pos = ent[i].position;
			}
			else
			{
				RigidBody *rigid = entity_list[ent[i].id]->rigid;
				entity_list[ent[i].id]->position = ent[i].position;
				rigid->velocity = ent[i].velocity;
				rigid->angular_velocity = ent[i].angular_velocity;
				rigid->morientation = ent[i].morientation;
			}
		}
		else
		{
			entity_list[ent[i].id]->position = ent[i].position;
		}
	}
	return 0;
}

void Engine::server_send()
{
	static servermsg_t	servermsg;
	static unsigned char	data[256000];

	if (client_list.size() == 0)
		return;

	servermsg.sequence = sequence;
	servermsg.client_sequence = 0;
	servermsg.num_ents = 0;

	serialize_ents(&data[0], servermsg.num_ents);
	servermsg.compressed_size = (unsigned short)huffman_compress((unsigned char *)&data[0], servermsg.num_ents * sizeof(entity_t),
		servermsg.data, sizeof(servermsg.data), huffbuf);


	if (recording_demo)
	{
		fwrite(&servermsg, servermsg.compressed_size + SERVER_HEADER, 1, demofile);
	}

	for (unsigned int i = 0; i < client_list.size(); i++)
	{
		// idle client timeout
		if (time(NULL) - client_list[i]->last_time > 90)
		{
			debugf("client %s timed out\n", client_list[i]->socketname);
			delete entity_list[client_list[i]->ent_id]->rigid;
			entity_list[client_list[i]->ent_id]->rigid = NULL;
			entity_list[client_list[i]->ent_id]->model = NULL;
			client_list.erase(client_list.begin() + i);
			i--;
			continue;
		}

		reliable[i].size = (unsigned short)(2 * sizeof(short) + strlen(reliable[i].msg) + 1);
		servermsg.client_sequence = client_list[i]->client_sequence;
		servermsg.length = SERVER_HEADER + servermsg.compressed_size + reliable[i].size;
		memcpy(&servermsg.data[servermsg.compressed_size], (void *)&reliable[i], reliable[i].size);

		client_list[i]->netinfo.num_ents = servermsg.num_ents;
		client_list[i]->netinfo.size = servermsg.length;
		netinfo.num_ents = servermsg.num_ents;
		netinfo.size = servermsg.length;

		int num_sent = net.sendto((char *)&servermsg, servermsg.length, client_list[i]->socketname);
		if (num_sent <= 0)
		{
			netinfo.send_full = true;

#ifdef WIN32
			int ret = WSAGetLastError();

			if (ret != WSAEWOULDBLOCK)
			{
				printf("Fatal socket error %d\n", ret);
				printf("size was %d bytes\n", servermsg.length);
			}
#else
			if (errno != EWOULDBLOCK)
			{
				printf("Fatal socket error %d\n", errno);
			}

#endif
		}
		else
		{
			netinfo.send_full = false;
		}

		if (num_sent != servermsg.length)
		{
			netinfo.send_partial = true;
		}
		else
		{
			netinfo.send_partial = false;
		}

		if (client_list[i]->needs_state)
		{
			server_send_state(i);
			client_list[i]->needs_state = false;
		}
	}
}

void Engine::client_recv()
{
	static unsigned char data[256000];
	static servermsg_t	servermsg;
	reliablemsg_t *rmsg = NULL;
	unsigned int socksize = sizeof(sockaddr_in);


	// get entity information
#ifdef WIN32
	int size = ::recvfrom(net.sockfd, (char *)&servermsg, sizeof(servermsg_t), 0, (sockaddr *)&(net.servaddr), (int *)&socksize);
#else
	int size = ::recvfrom(net.sockfd, (char *)&servermsg, sizeof(servermsg_t), 0, (sockaddr *)&(net.servaddr), (unsigned int *)&socksize);
#endif
	if ( size > 0)
	{
		if (size != servermsg.length)
		{
			printf("Packet size mismatch: %d %d\n", size, servermsg.length);
			return;
		}

		if (servermsg.sequence < last_server_sequence)
		{
			printf("Got old server packet\n");
			netinfo.dropped++;
			return;
		}

		double delay = ping_time_end(servermsg.client_sequence);
		if (delay > 0.0)
			netinfo.ping = (int)delay;

//		printf("Recieved %d ents from server\n", servermsg.num_ents);
		if (servermsg.client_sequence > client_reliable.sequence)
		{
			memset(client_reliable.msg, 0, LINE_SIZE);
			client_reliable.size = 0;
			client_reliable.sequence = -1;
		}

		if ((unsigned int)servermsg.length > SERVER_HEADER + servermsg.compressed_size + sizeof(int) + 1)
		{
			rmsg = (reliablemsg_t *)&servermsg.data[servermsg.num_ents * sizeof(entity_t)];
		}

		if (servermsg.num_ents)
		{
			unsigned int dsize = 0;

			dsize = huffman_decompress(servermsg.data, servermsg.compressed_size, data, sizeof(data), huffbuf);
			if (dsize != servermsg.num_ents * sizeof(entity_t))
			{
				printf("Decompressed size mismatch: %d %d\n", dsize, (int)(servermsg.num_ents * sizeof(entity_t)));
				return;
			}
		}

		netinfo.num_ents = servermsg.num_ents;
		netinfo.size = servermsg.length;
		netinfo.sequence_delta = sequence - servermsg.client_sequence;
		netinfo.ping_tick = netinfo.sequence_delta * TICK_MS;

		handle_servermsg(servermsg, data, rmsg);
		last_server_sequence = servermsg.sequence;
	}
}

void Engine::client_send()
{
	static clientmsg_t clientmsg;
	unsigned int socksize = sizeof(sockaddr_in);
	int keystate = GetKeyState(input);

	// Dont want to skip an attack command, so fiddle with send rate
	if (input.attack == false && cl_skip != 0)
	{
		if (tick_num % cl_skip == 0)
			return;
	}

	// send keyboard state
	memset(&clientmsg, 0, sizeof(clientmsg_t));
	clientmsg.sequence = sequence;
	clientmsg.qport = qport;
	clientmsg.server_sequence = last_server_sequence;
	clientmsg.up[0] = camera_frame.up.x;
	clientmsg.up[1] = camera_frame.up.y;
	clientmsg.up[2] = camera_frame.up.z;
	clientmsg.forward[0] = camera_frame.forward.x;
	clientmsg.forward[1] = camera_frame.forward.y;
	clientmsg.forward[2] = camera_frame.forward.z;
	clientmsg.pos[0] = camera_frame.pos.x;
	clientmsg.pos[1] = camera_frame.pos.y;
	clientmsg.pos[2] = camera_frame.pos.z;

	clientmsg.num_cmds = 1;
	memcpy(clientmsg.data, &keystate, sizeof(int));
	memcpy(&clientmsg.data[clientmsg.num_cmds * sizeof(int)], (void *)&client_reliable, client_reliable.size);
	clientmsg.length = CLIENT_HEADER + clientmsg.num_cmds * sizeof(int) + client_reliable.size;

	ping_time_start(sequence);
	int num_sent = ::sendto(net.sockfd, (char *)&clientmsg, clientmsg.length, 0, (sockaddr *)&(net.servaddr), socksize);

	if (server_flag == false)
	{
		if (num_sent <= 0)
			netinfo.send_full = true;
		else
			netinfo.send_full = false;

		if (num_sent != clientmsg.length)
		{
			netinfo.send_partial = true;
		}
		else
		{
			netinfo.send_partial = false;
		}
	}
}

int Engine::handle_servermsg(servermsg_t &servermsg, unsigned char *data, reliablemsg_t *reliablemsg)
{
//	int ent_id = -1;

	if (reliablemsg != NULL)
	{
		if (last_server_sequence <= reliablemsg->sequence)
		{
			int ret;

			debugf("server to client: %s\n", reliablemsg->msg);

			if (strstr(reliablemsg->msg, "<chat>"))
			{
				char msg[256];

				char *start = strstr(reliablemsg->msg, "<chat>");
				start += 6;
				char *end = strstr(reliablemsg->msg, "</chat>");

				memcpy(msg, start, end - start);

				menu.print_chat(msg);
				game->chat_timer = 3 * TICK_RATE;

				#define SND_TALK 268
				play_wave_global(SND_TALK);
			}

			if (strstr(reliablemsg->msg, "data"))
			{
				serverdata_t *data = (serverdata_t *)reliablemsg->msg;

				debugf("client received server state\n");
				game->set_state(data);
			}



			parse_spawn_string(reliablemsg->msg);

			ret = strcmp(reliablemsg->msg, "<disconnect/>");
			if (ret == 0)
			{
				unload();
			}
		}
	}

	deserialize_ents(data, servermsg.num_ents);
	return 0;
}

// packs keyboard input into an integer
int Engine::GetKeyState(input_t &input)
{
	int keystate = 0;

	if (input.attack)
		keystate |= 1;
	if (input.use)
		keystate |= 2;
	if (input.jump)
		keystate |= 4;
	if (input.moveup)
		keystate |= 8;
	if (input.moveleft)
		keystate |= 16;
	if (input.movedown)
		keystate |= 32;
	if (input.moveright)
		keystate |= 64;
	if (input.weapon_down)
		keystate |= 128;
	if (input.weapon_up)
		keystate |= 256;
	
	return keystate;
}

// unpacks keyboard integer into keyboard state
input_t Engine::GetKeyState(int keystate)
{
	input_t input;

	memset((void *)&input, 0, sizeof(input_t));
	if (keystate & 1)
		input.attack = true;
	else
		input.attack = false;

	if (keystate & 2)
		input.use = true;
	else
		input.use = false;

	if (keystate & 4)
		input.jump = true;
	else
		input.jump = false;

	if (keystate & 8)
		input.moveup = true;
	else
		input.moveup = false;

	if (keystate & 16)
		input.moveleft = true;
	else
		input.moveleft = false;

	if (keystate & 32)
		input.movedown = true;
	else
		input.movedown = false;

	if (keystate & 64)
		input.moveright = true;
	else
		input.moveright = false;

	if (keystate & 128)
		input.weapon_down = true;
	else
		input.weapon_down = false;

	if (keystate & 256)
		input.weapon_up = true;
	else
		input.weapon_up = false;

	return input;
}

bool Engine::mousepos(int x, int y, int deltax, int deltay)
{
	static bool once = false;

	if (q3map.loaded == false || menu.ingame == true || menu.console == true || menu.chatmode == true)
	{
		float devicex = (float)x / gfx.width;
		float devicey = (float)y / gfx.height;

		if (menu.console == true)
			return false;

		if (menu.chatmode == true)
			return false;

		bool updated = menu.delta(devicex, devicey);
		if (menu.ingame == false && updated)
		{
			gfx.clear();
			menu.render(global);
			gfx.swap();
		}

		once = true;

		return false;
	}

	if (once)
	{
		//recenter on game load so view doesnt spin too much
		once = false;
		return true;
	}

	camera_frame.update(vec2((float)deltax, (float)deltay), sensitivity);
	return true;
}

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
		key_bind.insert("tab", "scores");
		key_bind.insert("pgup", "pgup");
		key_bind.insert("pgdown", "pgdown");


		key_bind.insert("R", "centerview");
		
		key_bind.insert("F1", "voteyes");
		key_bind.insert("F2", "voteno");
		key_bind.insert("F3", "godmode");
		key_bind.insert("F4", "give all");
		
		key_bind.insert("F5", "respawn");
		//key_bind.insert("F6", "");
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
	seta sv_master5 ""
	seta sv_master4 ""
	seta sv_master3 ""
	seta sv_master2 ""
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

void Engine::keypress(char *key, bool pressed)
{
	bool handled = false;
	char k = 0;
	char *cmd = (char *)key_bind.find(key);
	if (cmd == NULL)
		return;

	if (strcmp("moveup", cmd) == 0)
	{
		handled = true;
		input.moveup = pressed;
		if (*key != 'w' && *key != 'W')
			k = 3;
	}
	else if (strcmp("moveleft", cmd) == 0)
	{
		handled = true;
		input.moveleft = pressed;
		if (*key != 'a' && *key != 'A')
			k = 4;
	}
	else if (strcmp("moveright", cmd) == 0)
	{
		handled = true;
		input.moveright = pressed;
		if (*key != 'd' && *key != 'D')
			k = 6;
	}
	else if (strcmp("movedown", cmd) == 0)
	{
		handled = true;
		input.movedown = pressed;
		if (*key != 's' && *key != 'S')
			k = 5;
	}
	else if (strcmp("jump", cmd) == 0)
	{
		handled = true;
		input.jump = pressed;
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

		input.weapon_up = pressed;
		if (client_flag)
			entity_list[spawn]->player->change_weapon_up();
		k = 2;
	}
	else if (strcmp("weapon_down", cmd) == 0)
	{
		int spawn = find_type(ENT_PLAYER, 0);
		handled = true;

		input.weapon_down = pressed;
		if (client_flag)
			entity_list[spawn]->player->change_weapon_down();
		k = 1;
	}
	else if (strcmp("use", cmd) == 0)
	{
		handled = true;
		input.use = pressed;
	}
	else if (strcmp("zoom", cmd) == 0)
	{
		handled = true;
		input.zoom = pressed;
	}
	else if (strcmp("duck", cmd) == 0)
	{
		handled = true;
		input.duck = pressed;
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

	if (pressed)
		keystroke(k);

	if (handled == false && menu.console == false)
		console(cmd);
}

void Engine::keystroke(char key)
{
	if (q3map.loaded == false)
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
			menu.handle(key, this);

			menu.render(global);
			if (menu.console)
				menu.render_console(global);
			gfx.swap();
		}
	}
	else
	{
		if (menu.console)
			menu.handle_console(key, this);
		else if (menu.ingame)
			menu.handle(key, this);
		else if (menu.chatmode)
			menu.handle_chatmode(key, this);
		else
			handle_game(key);
	}
}

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

void Engine::resize(int width, int height)
{
	xres = width;
	yres = height;


	gfx.resize(width, height);
	post.resize(width, height);

	projection.perspective(fov, (float)width / height, zNear, zFar, inf);

#ifndef __linux__
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
}


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
	audio.load(wave);
	snd_wave.push_back(wave);

	return snd_wave.size() - 1;
}

void Engine::load_models()
{
	if (entity_list.size() == 0)
		return;


	for(unsigned int i = max_dynamic; i < entity_list.size(); i++)
	{
		if (entity_list[i]->model == NULL)
			continue;

		game->map_model(*entity_list[i]);
	}
}

// Loads media that may be shared with multiple entities
void Engine::load_entities()
{
#ifndef DEDICATED
	int spawn = -1;
	if (client_flag == false)
		game->add_player(entity_list, PLAYER, spawn, "UnnamedPlayer");
#endif
//	load_sounds();
	create_sources();
	load_models();

	for(unsigned int i = 0; i < entity_list.size(); i++)
	{
		if (entity_list[i]->light)
			entity_list[i]->rigid->gravity = false;
	}
}

void Engine::fullscreen()
{
#ifdef WIN32
	if (fullscreen_timer == 0)
	{
		HMONITOR hmon;
		MONITORINFO mi = { sizeof(MONITORINFO) };
		HWND hwnd = *((HWND *)param1);

		hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
		GetMonitorInfo(hmon, &mi);

		int xr = abs(mi.rcMonitor.right - mi.rcMonitor.left);
		int yr = abs(mi.rcMonitor.bottom - mi.rcMonitor.top);

		static LONG old_style;
		static int	new_style = WS_CHILD | WS_VISIBLE;

		old_style = SetWindowLong(hwnd, GWL_STYLE, new_style);
		new_style = old_style;
		SetWindowPos(hwnd, HWND_TOP, 0, 0, xr, yr, 0);
		fullscreen_timer = TICK_RATE;
	}
#endif
}

void Engine::clean_entity(int index)
{
	//free audio sources
	if (entity_list[index]->trigger)
		entity_list[index]->trigger->destroy(audio);

	if (entity_list[index]->speaker)
		entity_list[index]->speaker->destroy(audio);
	entity_list[index]->nettype = NT_NONE;

	entity_list[index]->particle_on = false;


	// Light list wont be updated until the next step, so manually delete
	if (entity_list[index]->light)
	{
		for (unsigned int i = 0; i < light_list.size(); i++)
		{
			if (light_list[i]->entity == entity_list[index])
			{
				light_list.erase(light_list.begin() + i);
			}
		}
	}
}

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
				break;
			}
		}

//		if (strcmp(entity_list[index]->type, "free") == 0)
		{
			clean_entity(index);
			entity_list[index]->~Entity();
			return index++;
		}
		index++;
	}

	return max_dynamic - 1;
}

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

		if (strcmp(entity_list[index]->type, "free") == 0)
		{
			clean_entity(index);
			entity_list[index]->~Entity();
			return index++;
		}
		index++;
	}

	return max_player - 1;
}

void Engine::create_sources()
{
	// create and associate sources
	for(unsigned int i = max_dynamic; i < entity_list.size(); i++)
	{
		Speaker *speaker = entity_list[i]->speaker;
		if (speaker != NULL)
		{
			entity_list[i]->rigid->gravity = false;
			if (speaker->index  != -1)
			{
				audio.select_buffer(speaker->loop_source, snd_wave[entity_list[i]->speaker->index].buffer);
				audio.play(speaker->loop_source);
			}
		}
		else if (entity_list[i]->trigger != NULL)
		{
			entity_list[i]->rigid->gravity = false;
		}
	}

	// position sources
	update_audio();
}

void Engine::update_audio()
{
	audio.listener_position((float *)&(camera_frame.pos));

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

void Engine::kick(unsigned int i)
{
	if (server_flag == false)
		return;

	if (i >= client_list.size())
	{
		debugf("kick() invalid index %d", i);
		return;
	}

	static servermsg_t servermsg = { 0 };

	servermsg.sequence = sequence + 1;
	servermsg.client_sequence = reliable[i].sequence;
	servermsg.num_ents = 0;

	sprintf(reliable[i].msg, "<disconnect/>");
	reliable[i].size = (unsigned short)(2 * sizeof(unsigned short int) + strlen(reliable[i].msg) + 1);
	reliable[i].sequence = sequence;
	servermsg.length = SERVER_HEADER + reliable[i].size;
	memcpy(servermsg.data, &reliable, reliable[i].size);
	net.sendto((char *)&servermsg, servermsg.length, client_list[i]->socketname);
	debugf("sent disconnect to client %d [%s]\n", i, client_list[i]->socketname);
	free((void *)client_list[i]);
	client_list.erase(client_list.begin() + i);
}

void Engine::unload()
{
	if (q3map.loaded == false)
		return;

	game->unload();

	num_bot = 0;

	if (node)
		delete[] node;


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

	if (server_flag)
	{
		while (client_list.size())
		{
			kick(0);
		}
		client_list.clear();
		net.closesock();
	}

	menu.console = false;
	client_flag = 0;
	server_flag = 0;
}

void Engine::destroy()
{
	if (recording_demo)
	{
		console("stop");
	}

	delete [] shader_list[0];
	delete [] hash_list[0];
	delete [] pk3_list[0];
	game->destroy();
	delete game;
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
	audio.destroy();
	printf("quit\n\n");
	quit();
}

void Engine::quit()
{
#ifdef _WINDOWS_
	HWND hwnd = *((HWND *)param1);
	PostMessage(hwnd, WM_CLOSE, 0, 0);
#else
	exit(0);
#endif
}

void Engine::console(char *cmd)
{
	char msg[LINE_SIZE] = { 0 };
	char data[LINE_SIZE] = { 0 };
	int port;
	int ret;

	if (cmd == NULL)
		return;

/*
	static int last_cmd = 0;
	if (last_cmd >= tick_num)
		return;
	last_cmd = tick_num;
	*/


	if (strstr(cmd, "console"))
	{
		menu.console = !menu.console;
		return;
	}

	

	if (strstr(cmd, "centerview"))
	{
		camera_frame.reset();
		return;
	}

	if (strstr(cmd, "talk"))
	{
		menu.chatmode = true;
		return;
	}

	if (strstr(cmd, "talkteam"))
	{
		menu.chatmode = true;
		return;
	}

	if (strstr(cmd, "togglemenu"))
	{
		if (ingame_menu_timer == 0)
			menu.ingame = !menu.ingame;
		ingame_menu_timer = TICK_RATE >> 4;
		return;
	}

	if (strstr(cmd, "voteyes"))
	{
		if (voted == false)
		{
			vote_yes++;
			voted = true;
		}
		return;
	}

	if (strstr(cmd, "voteno"))
	{
		if (voted == false)
		{
			vote_no++;
			voted = true;
		}
		return;
	}

	if (strcmp(cmd, "screenshot") == 0)
	{
		screenshot();
		return;
	}

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
			snprintf(sv_hostname, 127, "%s", data);
			debugf("sv_hostname: %s\n", data);
		}
		else
		{
			debugf("Invalid name, must be alphanumeric + space\n");
		}
		return;
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
			snprintf(sv_motd, 127, "%s", data);
			debugf("sv_motd: %s\n", data);
		}
		else
		{
			debugf("Invalid name, must be alphanumeric + space\n");
		}
		return;
	}


	ret = sscanf(cmd, "echo %s", data);
	if (ret == 1)
	{
		snprintf(msg, LINE_SIZE, "%s\n", data);
		menu.print(msg);
		return;
	}
	menu.print(cmd);

	ret = sscanf(cmd, "map %s", data);
	if (ret == 1)
	{
		unload();
		snprintf(msg, LINE_SIZE, "Loading %s\n", data);
		menu.print(msg);
		load(data);
		return;
	}

	ret = sscanf(cmd, "bind %s %s", data, msg);
	if (ret == 2)
	{
		if (key_bind.update(data, strstr(cmd, msg)))
		{
			return;
		}
		key_bind.insert(data, strstr(cmd, msg));
		return;
	}

	ret = sscanf(cmd, "record %s", data);
	if (ret == 1)
	{

		if (q3map.loaded)
		{
			demo_fileheader_t header;
			menu.print(msg);
			demofile = fopen(data, "wb");

			memcpy(header.magic, "demo", 5);
			memcpy(header.map, q3map.map_name, 64);
			fwrite(&header, sizeof(demo_fileheader_t), 1, demofile);
			recording_demo = true;
		}
		else
		{
			menu.print("Load map first");
		}
		return;
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
			demo_fileheader_t header;

			demofile = fopen(data, "rb");
			if (demofile == NULL)
			{
				menu.print("Unable to open demofile");
				return;
			}

			fread(&header, sizeof(demo_fileheader_t), 1, demofile);
			playing_demo = true;
			load(header.map);
		}
		return;
	}


	if (strstr(cmd, "stop"))
	{
		if (recording_demo)
		{
			menu.print(msg);
			fclose(demofile);
			recording_demo = false;
		}
		return;
	}

	float value = 1.0f;

	ret = sscanf(cmd, "al_reference_dist %f", &value);
	if (ret == 1)
	{
		printf("Setting audio reference distance to %f\n", value);
		set_reference_distance(value);
		return;
	}

	ret = sscanf(cmd, "al_max_dist %f", &value);
	if (ret == 1)
	{
		printf("Setting audio max distance to %f\n", value);
		set_max_distance(value);
		return;
	}

	ret = sscanf(cmd, "al_rolloff %f", &value);
	if (ret == 1)
	{
		printf("Setting audio rolloff factor to %f\n", value);
		set_rolloff_factor(value);
		return;
	}

	ret = sscanf(cmd, "al_model %s", (char *)data);
	if (ret == 1)
	{
		printf("Setting audio model to %d\n", atoi(data));
		audio.set_audio_model(atoi(data));
		return;
	}

	ret = sscanf(cmd, "sv_maxclients %s", (char *)data);
	if (ret == 1)
	{
		int num = atoi(data);

		if (num <= 8 && num >= 0)
		{
			debugf("setting max clients");
			sv_maxclients = num;
		}
		else
		{
			debugf("Invalid input");
		}
		return;
	}

	ret = sscanf(cmd, "net_port %s", (char *)data);
	if (ret == 1)
	{
		int num = atoi(data);

		if (num <= 65535 && num >= 0)
		{
			debugf("setting port");
			net_port = num;
		}
		else
		{
			debugf("Invalid input");
		}
		return;
	}


	if (strstr(cmd, "list") != 0)
	{
#define LIST_SIZE 1024 * 512
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
		return;
	}

	if (strcmp(cmd, "clear") == 0)
	{
		menu.clear_console();
		return;
	}

	if (strcmp(cmd, "fullscreen") == 0)
	{
		fullscreen();
		return;
	}


	ret = sscanf(cmd, "connect %s", data);
	if (ret == 1)
	{
		snprintf(msg, LINE_SIZE, "Connecting to %s\n", data);
		menu.print(msg);
		connect(data);
		return;
	}

	if (strcmp(cmd, "r_res") == 0)
	{
		snprintf(msg, LINE_SIZE, "%dx%d\n", gfx.width, gfx.height);
		menu.print(msg);
		return;
	}
	if (sscanf(cmd, "r_texture %s", data) == 1)
	{
		menu.print(msg);
		if (atoi(data))
		{
			q3map.enable_textures = true;
		}
		else
		{
			q3map.enable_textures = false;
		}
		return;
	}

	if (sscanf(cmd, "r_patch %s", data) == 1)
	{
		menu.print(msg);
		if (atoi(data))
		{
			q3map.enable_patch = true;
		}
		else
		{
			q3map.enable_patch = false;
		}
		return;
	}

	if (sscanf(cmd, "r_maxlight %s", data) == 1)
	{
		menu.print(msg);

		int max = atoi(data);

		mlight2.set_max(max);
		return;
	}

	if (sscanf(cmd, "r_ambient %s", data) == 1)
	{
		menu.print(msg);

		float ambient = (float)atof(data);
		mlight2.set_ambient(ambient);
		return;
	}

	if (sscanf(cmd, "r_brightness %s", data) == 1)
	{
		menu.print(msg);

		float ambient = (float)atof(data);
		mlight2.set_brightness(ambient - 1.0f);
		return;
	}

	if (sscanf(cmd, "r_contrast %s", data) == 1)
	{
		menu.print(msg);

		float contrast = (float)atof(data);
		mlight2.set_contrast(contrast);
		return;
	}

	if (sscanf(cmd, "com_maxfps %s", data) == 1)
	{
		menu.print(msg);

		if (atoi(data) > 0)
			com_maxfps = 1000.0 / atoi(data);
		return;
	}

	if (sscanf(cmd, "r_lightmap %s", data) == 1)
	{
		menu.print(msg);

		float lightmap = (float)atof(data);
		mlight2.set_lightmap(lightmap);
		return;
	}

	ret = strcmp(cmd, "shadowmaps");
	if (ret == 0)
	{
		shadowmaps = !shadowmaps;
		if (shadowmaps)
		{
			mlight2.set_shadowmap(1.0f);

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
			mlight2.set_shadowmap(0.0f);
		}
		snprintf(msg, LINE_SIZE, "%s %d\n", cmd, shadowmaps);
		menu.print(msg);
		return;
	}


	if (strstr(cmd, "postprocess"))
	{
		post_process(5);
		return;
	}

	if (strstr(cmd, "lookforward"))
	{
		vec3 right(-1.0f, 0.0f, 0.0f);
		vec3 up(0.0f, 1.0f, 0.0f);
		vec3 forward(0.0f, 0.0f, 1.0f);

		camera_frame.forward = forward;
		camera_frame.up = up;
		return;
	}

	if (strstr(cmd, "lookdown"))
	{
		vec3 right(1.0f, 0.0f, 0.0f);
		vec3 up(0.0f, 0.0f, 1.0f);
		vec3 forward(0.0f, 1.0f, 0.0f);

		camera_frame.forward = forward;
		camera_frame.up = up;
		return;
	}

	if (strstr(cmd, "lookright"))
	{
		vec3 right(0.0f, 0.0f, 1.0f);
		vec3 up(0.0f, 1.0f, 0.0f);
		vec3 forward(1.0f, 0.0f, 0.0f);

		camera_frame.forward = forward;
		camera_frame.up = up;
		return;
	}


	if (strstr(cmd, "lookleft"))
	{
		vec3 right(0.0f, 0.0f, -1.0f);
		vec3 up(0.0f, 1.0f, 0.0f);
		vec3 forward(-1.0f, 0.0f, 0.0f);

		camera_frame.forward = forward;
		camera_frame.up = up;
		return;
	}

	if (strstr(cmd, "lookback"))
	{
		vec3 right(1.0f, 0.0f, 0.0f);
		vec3 up(0.0f, 1.0f, 0.0f);
		vec3 forward(0.0f, 0.0f, -1.0f);

		camera_frame.forward = forward;
		camera_frame.up = up;
		return;
	}

	if (strstr(cmd, "lookup"))
	{
		vec3 right(1.0f, 0.0f, 0.0f);
		vec3 up(0.0f, 0.0f, -1.0f);
		vec3 forward(0.0f, -1.0f, 0.0f);


		camera_frame.forward = forward;
		camera_frame.up = up;
		return;
	}


	if (strstr(cmd, "movelight"))
	{
		if (shadowmaps)
		{
			Entity *entity = entity_list[shadow_light];
			Entity *player = entity_list[find_type(ENT_PLAYER, 0)];

			printf("Moving shadow light to player position %3.3f %3.3f %3.3f\n",
				entity->position.x,
				entity->position.y,
				entity->position.z);
			entity->position = player->position;
		}
		return;
	}


	if (sscanf(cmd, "r_light %s", data) == 1)
	{
		int mode = 0;
		menu.print(msg);
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
		}
		return;
	}

	ret = sscanf(cmd, "cl_skip %s", data);
	if (ret == 1)
	{
		menu.print(msg);
		debugf("Setting cl_skip to %s\n", data);
		cl_skip = atoi(data);
		return;
	}

	if (sscanf(cmd, "g_collision %s", data) == 1)
	{
		menu.print(msg);
		if (atoi(data))
		{
			collision_detect_enable = true;
		}
		else
		{
			collision_detect_enable = false;
		}
		return;
	}

	if (sscanf(cmd, "r_sky %s", data) == 1)
	{
		menu.print(msg);
		if (atoi(data))
		{
			q3map.enable_sky = true;
		}
		else
		{
			q3map.enable_sky = false;
		}
		return;
	}

	if (strcmp(cmd, "r_max_particles") == 0)
	{
		menu.print(msg);

		sprintf(data, "max particles: %d\n", ParticleUpdate::max_particles);
		return;
	}

	if (sscanf(cmd, "r_max_particles %s", data) == 1)
	{
		menu.print(msg);
		ParticleUpdate::max_particles = atoi(data);
		return;
	}

	if (strcmp(cmd, "r_reload_shaders") == 0)
	{
		menu.print(msg);

		reload_shaders();
		return;
	}

	if (sscanf(cmd, "r_shader %s", data) == 1)
	{
		menu.print(msg);

		if (atoi(data))
		{
			q3map.enable_shader = true;
		}
		else
		{
			q3map.enable_shader = false;
		}
		return;
	}

	if (sscanf(cmd, "r_entities %s", data) == 1)
	{
		menu.print(msg);

		if (atoi(data))
		{
			entities_enabled = true;
		}
		else
		{
			entities_enabled = false;
		}
		return;
	}

	if (sscanf(cmd, "r_blend %s", data) == 1)
	{
		menu.print(msg);

		if (atoi(data))
		{
			q3map.enable_blend = true;
		}
		else
		{
			q3map.enable_blend = false;
		}
		return;
	}

	if (sscanf(cmd, "r_portal %s", data) == 1)
	{
		menu.print(msg);

		if (atoi(data))
		{
			enable_portal = true;
		}
		else
		{
			enable_portal = false;
		}
		return;
	}


	if (sscanf(cmd, "cg_fov %s", data) == 1)
	{
		snprintf(msg, LINE_SIZE, "Setting fov to %d\n", atoi(data));
		menu.print(msg);
		fov = atoi(data) * 0.5f;
		projection.perspective(fov, (float)xres / yres, zNear, zFar, inf);
		return;
	}

	if (sscanf(cmd, "r_rendermode %s", data) == 1)
	{
		menu.print(msg);

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
		else if (atoi(data) == MODE_SHADOWVOL)
		{
			if (q3map.loaded == false)
			{
				snprintf(msg, LINE_SIZE, "Setting rendermode to shadow volumes\n");
				menu.print(msg);
				render_mode = MODE_SHADOWVOL;
			}
			else
			{
				snprintf(msg, LINE_SIZE, "Must enable shadowvolumes before loading map\n");
				menu.print(msg);
			}
		}
		return;
	}

	if (sscanf(cmd, "r_multisample %s", data) == 1)
	{
		menu.print(msg);

		multisample = atoi(data);
#ifdef OPENGL
		if (multisample > 0)
			glEnable(GL_MULTISAMPLE);
		else
			glDisable(GL_MULTISAMPLE);
#endif
		gfx.setupFramebuffer(fb_width, fb_height, fbo, quad_tex, depth_tex, multisample);
		return;
	}


	if (sscanf(cmd, "sensitivity %s", data) == 1)
	{
		snprintf(msg, LINE_SIZE, "Setting mouse sensitivity to %3.3f\n", atof(data));
		menu.print(msg);
		sensitivity = (float)atof(data);
		return;
	}

	if (sscanf(cmd, "res_scale %s", data) == 1)
	{
		snprintf(msg, LINE_SIZE, "Setting resolution scale to %3.3f\n", atof(data));
		menu.print(msg);
		res_scale = (float)atof(data);
		fb_width = (unsigned int)(1024 * res_scale);
		fb_height = (unsigned int)(1024 * res_scale);
		gfx.setupFramebuffer(fb_width, fb_height, fbo, quad_tex, depth_tex, multisample);
		return;
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
		return;
	}

	ret = sscanf(cmd, "bind %d", &port);
	if (ret == 1)
	{
		snprintf(msg, LINE_SIZE, "binding to port %d\n", port);
		menu.print(msg);
		bind(port);
		return;
	}

	ret = strcmp(cmd, "bind default");
	if (ret == 0)
	{
		port = net_port;
		snprintf(msg, LINE_SIZE, "binding to port %d\n", port);
		menu.print(msg);
		bind(port);
		return;
	}

	ret = strcmp(cmd, "quit");
	if (ret == 0)
	{
		if (q3map.loaded)
		{
			unload();
		}
		destroy();
		return;
	}

	ret = strcmp(cmd, "exit");
	if (ret == 0)
	{
		exit(0);
	}

	ret = sscanf(cmd, "r_polygonmode %s", data);
	if (ret == 1)
	{
#ifdef OPENGL
		if (atoi(data) == 1)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else if (atoi(data) == 2)
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
		return;
	}

	ret = sscanf(cmd, "r_frontface %s", data);
	if (ret == 1)
	{
#ifdef OPENGL
		if (atoi(data) == 1)
			glFrontFace(GL_CCW);
		else
			glFrontFace(GL_CW);
#endif
	}

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

int Engine::bind(int port)
{
	if (server_flag)
	{
		debugf("Server already bound to port\n");
		return -1;
	}

	if (net.bind(NULL, port) == 0)
	{
		client_flag = false;
		server_flag = true;
		return 0;
	}
	else
	{
		q3map.unload(gfx);
		return -1;
	}


}

void Engine::connect(char *serverip)
{
	static clientmsg_t	clientmsg;
	static servermsg_t servermsg;

	client_flag = false;

	memset(&clientmsg, 0, sizeof(clientmsg_t));

	clientmsg.sequence = sequence;
	clientmsg.server_sequence = 0;
	clientmsg.num_cmds = 0;
	clientmsg.qport = qport;
	strcpy(client_reliable.msg, "<connect/>");
	client_reliable.size = (unsigned short)(2 * sizeof(unsigned short int) + strlen(client_reliable.msg) + 1);
	client_reliable.sequence = sequence;
	memcpy(&clientmsg.data[clientmsg.num_cmds * sizeof(int)], &client_reliable, client_reliable.size);
	clientmsg.length = CLIENT_HEADER + clientmsg.num_cmds * sizeof(int) + client_reliable.size;

	net.connect(serverip, net_port);
	debugf("Sending map request\n");
	net.send((char *)&clientmsg, clientmsg.length);
	debugf("Waiting for server info\n");

	if ( net.recv((char *)&servermsg, 8192, 5) )
	{
		char level[LINE_SIZE];

		client_flag = true;
		server_flag = false;
		debugf("Connected\n");
		reliablemsg_t *reliablemsg = (reliablemsg_t *)&servermsg.data[0];
		if (sscanf(reliablemsg->msg, "<map>%s</map>", level) == 1)
		{
			char *end = strstr(level, "</map>");
			*end = '\0';
			debugf("Loading %s\n", level);
			load((char *)level);
			strcpy(client_reliable.msg, "<spawn/>");
			client_reliable.size = (unsigned short)(2 * sizeof(unsigned short int) + strlen(client_reliable.msg) + 1);
			client_reliable.sequence = sequence;
			last_server_sequence = servermsg.sequence;
		}
		else
		{
			debugf("Invalid response\n");
		}
	}
	else
	{
		debugf("Connection timed out\n");
	}
}

void Engine::chat(char *name, char *msg)
{
	char data[1024];

	if (name == NULL)
	{
		int index = find_type(ENT_PLAYER, 0);

		//chatmode chat
		sprintf(data, "%s: %s", entity_list[index]->player->name, msg);
	}
	else
	{
		//console chat command
		// skip past 'say "'
		char *pmsg = msg + 5;
		// remove ending "
		pmsg[strlen(pmsg) - 1] = '\0';
		sprintf(data, "%s: %s", name, pmsg);
	}


	for (unsigned int i = 0; i < max_player; i++)
	{
		sprintf(msg, "<chat>%s</chat>", data);
		strcat(reliable[i].msg, msg);
		reliable[i].size = (unsigned short)(2 * sizeof(unsigned short int) + strlen(reliable[i].msg) + 1);
		reliable[i].sequence = sequence;
	}

	// Client will get message back from server
	if (client_flag == false)
	{
		menu.print_chat(data);
		game->chat_timer = 3 * TICK_RATE;

		int self = find_type(ENT_PLAYER, 0);
		if (self != -1)
		{
			play_wave_global(SND_TALK);
		}
	}
}




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

int Engine::get_source()
{
	int source;
	static int i = 0;

	source = audio_source[i++];

	if (i == max_sources)
		i = 0;

	return source;
}

int Engine::get_loop_source()
{
	int source;
	static int i = 0;

	source = audio_loop_source[i++];

	if (i == max_sources)
		i = 0;

	return source;
}

int Engine::get_global_source()
{
	int source;
	static int i = 0;

	source = global_source[i++];

	if (i == max_sources)
		i = 0;

	return source;
}

int Engine::get_global_loop_source()
{
	int source;
	static int i = 0;

	source = global_loop_source[i++];

	if (i == max_sources)
		i = 0;

	return source;
}


bool Engine::play_wave_source(int source, int index)
{
	audio.select_buffer(source, snd_wave[index].buffer);
	audio.play(source);
	return true;
}

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

int Engine::play_wave_global(int index)
{
	if (index < 0)
		return 0;

	int source = get_global_source();
	audio.select_buffer(source, snd_wave[index].buffer);
	audio.play(source);
	return source;
}

int Engine::play_wave_global_loop(int index)
{
	if (index < 0)
		return 0;

	int source = get_global_loop_source();
	audio.select_buffer(source, snd_wave[index].buffer);
	audio.play(source);
	return source;
}


void Engine::hitscan(vec3 &origin, vec3 &dir, int *index_list, int &num_index, int self, float range)
{
	int j = 0;
	num_index = 0;

	for (unsigned int i = 0; i < entity_list.size(); i++)
	{
		RigidBody *rigid = entity_list[i]->rigid;
		if (i == (unsigned int)self)
			continue;

		if (entity_list[i]->player && rigid != NULL)
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
		}
	}

}

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
}


void Engine::get_shaderlist_pk3(char **shaderlist, int &num_shader)
{
#define LIST_SIZE 1024 * 512
	char *filelist = new char[LIST_SIZE];
	char *line = NULL;

	memset(filelist, 0, LIST_SIZE);
	for (unsigned int i = 0; i < num_pk3; i++)
	{
		list_zipfile(pk3_list[i], &filelist[0]);

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

void Engine::set_reference_distance(float value)
{
	for (int i = 0; i < max_sources; i++)
	{
		alSourcef(audio_source[i], AL_REFERENCE_DISTANCE, value);
		alSourcef(global_source[i], AL_REFERENCE_DISTANCE, value);
	}
}

void Engine::set_max_distance(float value)
{
	for (int i = 0; i < max_sources; i++)
	{
		alSourcef(audio_source[i], AL_MAX_DISTANCE, value);
		alSourcef(global_source[i], AL_MAX_DISTANCE, value);
	}
}

void Engine::set_rolloff_factor(float value)
{
	for (int i = 0; i < max_sources; i++)
	{
		alSourcef(audio_source[i], AL_ROLLOFF_FACTOR, value);
		alSourcef(global_source[i], AL_ROLLOFF_FACTOR, value);
	}
}

void Engine::paste(char *data, unsigned int size)
{
	if (menu.console || menu.chatmode)
	{
		for(unsigned int i = 0; i < size; i++)
			keystroke(data[i]);
	}
}

void Engine::copy(char *data, unsigned int size)
{
	menu.copy(data, size);
}
