#include "include.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//#define SHADOWVOL
#define FORWARD
//#define DEFERRED

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
#define BOUNCE		1.1f

Engine::Engine()
{
	initialized = false;
	max_dynamic = 100; // 300 is causing network issues
	max_player = 8;
	max_sources = 32;
	cl_skip = 0;
	show_names = false;
	show_lines = false;
	show_debug = false;
	show_hud = true;
	entities_enabled = true;
	collision_detect_enable = true;
	num_bot = 0;
	emitter.enabled = false;
	demo = false;

	sprintf(servername, "altEngine Server %s", __DATE__);
	sprintf(password, "iddqd");
	memset(&netinfo, 0, sizeof(netinfo));

	fov = 45.0f;
	zNear = 1.0f;
	zFar = 2001.0f; // zFar - zNear makes nice values
	inf = true; // above ignored if true
}


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
	newlinelist("media/pk3list.txt", pk3_list, num_pk3);
	newlinelist("media/pk3hash.txt", hash_list, num_hash);

	if (num_pk3 != num_hash)
	{
		printf("Error: num_pk3 %d num_hash %d, should match\n", num_pk3, num_hash);
		exit(0);
	}


#ifdef NDEBUG 
	char hash[128];
	/*
	if ( check_hash(APP_NAME, APP_HASH, hash) == false)
	{
		printf("Program code failed hash check!\n");
		exit(0);
	}*/

	for (int i = 0; i < num_pk3 && i < num_hash; i++)
	{
		printf("Checking hash for %s...", pk3_list[i]);
		if (check_hash(pk3_list[i], hash_list[i], hash) == false)
		{
			if (strcmp(pk3_list[i], "media/pak0.pk3") == 0)
			{
				if (strcmp(hash, "0613b3d4ef05e613a2b470571498690f") == 0)
				{
					printf("pak0.pk3 is from Q3A Demo\n");
					demo = true;
				}
				else
				{
					printf("\n%s failed hash check:\n[%s] expected [%s]\n", pk3_list[i], hash, hash_list[i]);
					exit(0);
				}
			}
			else
			{
				printf("\n%s failed hash check:\n[%s] expected [%s]\n", pk3_list[i], hash, hash_list[i]);
			}
		}
		else
		{
			printf("Good!\n");
		}
	}
#endif

	no_tex = load_texture(gfx, "media/notexture.tga", false);
	particle_tex = load_texture(gfx, "media/flare.png", false);
	Model::CreateObjects(gfx);
	Model::make_skybox(gfx);

	box = new Entity();
	box->rigid = new RigidBody(box);
	box->model = box->rigid;
	box->model->load(gfx, "media/models/box");

	ball = new Entity();
	ball->rigid = new RigidBody(ball);
	ball->model = ball->rigid;
	ball->model->load(gfx, "media/models/ball");

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



	rocket = new Entity();
	rocket->rigid = new RigidBody(rocket);
	rocket->model = rocket->rigid;
	rocket->rigid->load(gfx, "media/models/weapons2/rocketl/rocket");

	pineapple = new Entity();
	pineapple->rigid = new RigidBody(pineapple);
	pineapple->model = pineapple->rigid;
	pineapple->rigid->load(gfx, "media/models/weapons2/grenadel/pineapple");

	shell = new Entity();
	shell->rigid = new RigidBody(shell);
	shell->model = shell->rigid;
	shell->model->load(gfx, "media/models/weapons2/shells/s_shell");

	bullet = new Entity();
	bullet->rigid = new RigidBody(bullet);
	bullet->model = bullet->rigid;
	bullet->model->load(gfx, "media/models/weapons2/shells/M_shell");


	gib0 = new Entity();
	gib0->rigid = new RigidBody(gib0);
	gib0->model = gib0->rigid;
	gib0->model->load(gfx, "media/models/gibs/abdomen");

	gib1 = new Entity();
	gib1->rigid = new RigidBody(gib1);
	gib1->model = gib1->rigid;
	gib1->model->load(gfx, "media/models/gibs/arm");

	gib2 = new Entity();
	gib2->rigid = new RigidBody(gib2);
	gib2->model = gib2->rigid;
	gib2->model->load(gfx, "media/models/gibs/brain");

	gib3 = new Entity();
	gib3->rigid = new RigidBody(gib3);
	gib3->model = gib3->rigid;
	gib3->model->load(gfx, "media/models/gibs/chest");

	gib4 = new Entity();
	gib4->rigid = new RigidBody(gib4);
	gib4->model = gib4->rigid;
	gib4->model->load(gfx, "media/models/gibs/fist");

	gib5 = new Entity();
	gib5->rigid = new RigidBody(gib5);
	gib5->model = gib5->rigid;
	gib5->model->load(gfx, "media/models/gibs/foot");

	gib6 = new Entity();
	gib6->rigid = new RigidBody(gib6);
	gib6->model = gib6->rigid;
	gib6->model->load(gfx, "media/models/gibs/forearm");

	gib7 = new Entity();
	gib7->rigid = new RigidBody(gib7);
	gib7->model = gib7->rigid;
	gib7->model->load(gfx, "media/models/gibs/intestine");

	gib8 = new Entity();
	gib8->rigid = new RigidBody(gib8);
	gib8->model = gib8->rigid;
	gib8->model->load(gfx, "media/models/gibs/leg");

	gib9 = new Entity();
	gib9->rigid = new RigidBody(gib9);
	gib9->model = gib9->rigid;
	gib9->model->load(gfx, "media/models/gibs/skull");



	global.init(&gfx);
	audio.init();
	menu.init(&gfx, &audio, pk3_list, num_pk3);


	for (int i = 0; i < max_sources; i++)
	{
		audio_source[i] = audio.create_source(false, false);
		global_source[i] = audio.create_source(false, true);
	}


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
	reliable.sequence = -1;
	last_server_sequence = 0;
	testObj = 0;

	printf("Loading md5 models...\n");
	load_md5();



	fb_width = 1280;
	fb_height = 1280;
	gfx.setupFramebuffer(fb_width, fb_height, fbo, quad_tex, depth_tex);

	//parse shaders
	printf("Loading Quake3 shaders...\n");
//	newlinelist("media/shaderlist.txt", shader_list, num_shader);
	get_shaderlist_pk3(shader_list, num_shader);


	if (num_shader == 0)
	{
		printf("****************Unable to load shaderlist.txt\n");
		exit(0);
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
			free((void *)shader_file);
		}
	}

	printf("Done\n");

#ifndef DIRECTX
	//render menu again for linux
	gfx.resize(xres,yres);
	menu.render(global);
	gfx.swap();
#endif

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
	light_frame.reset();



	//First n Entities for dynamic items (Dont allocate any at runtime)
	for (unsigned int i = 0; i < max_dynamic; i++)
	{
		Entity *entity = new Entity();
		memcpy(entity->type, "free", strlen("free") + 1);
		entity_list.push_back(entity);

		if (i < max_player && server_flag)
		{
			// Forces server to expect player rigid bodies
			entity->rigid = new RigidBody(entity);
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


	char entfile[80] = { 0 };
	sprintf(entfile, "media/%s.ent", q3map.map_name);
	char *entdata = get_file(entfile, NULL);
	if (entdata != NULL)
	{
		parse_entity(this, entdata, entity_list, gfx, audio);
		free((void *)entdata);
	}
	else
	{
		char filename[80];
		const char *data = q3map.get_entities();

		sprintf(filename, "media/%s.ent", q3map.map_name);
		write_file(filename, data, strlen(data));
		parse_entity(this, q3map.get_entities(), entity_list, gfx, audio);
	}



	int start = entity_list.size();

	char navfile[80] = { 0 };
	sprintf(navfile, "media/%s.nav", q3map.map_name);
	char *navdata = get_file(navfile, NULL);
	if (navdata != NULL)
	{
		parse_entity(this, navdata, entity_list, gfx, audio);
		free((void *)navdata);
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

	q3map.render(camera_frame.pos, mvp, gfx, surface_list, mlight2, tick_num);
	q3map.lastIndex = -2; // force generation of new face lists
	camera_frame.set(transformation);
	render_entities(transformation, true);

	menu.delta("textures", *this);
	menu.render(global);
	gfx.swap();

	q3map.load_textures(gfx, surface_list, pk3_list, num_pk3);
	menu.delta("loaded", *this);
	menu.stop();
	menu.ingame = false;
	menu.console = false;
	menu.chat = true;


#ifdef DEFERRED
	//Setup render to texture
	gfx.bindFramebuffer(fbo);
	gfx.resize(fb_width, fb_height);

	// Generate depth cubemaps for each light
	render_shadowmaps();
	gfx.bindFramebuffer(0);
#endif


#ifdef SHADOWVOL
	for(int i = 0; i < entity_list.size(); i++)
	{
		if (entity_list[i]->light)
		{
			entity_list[i]->light->generate_volumes(q3map);
//			entity_list[i]->rigid->angular_velocity = vec3();
		}
	}

#endif

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

#ifdef DEFERRED

	//Setup render to texture
	gfx.bindFramebuffer(fbo);
	gfx.resize(fb_width, fb_height);

	// Generate depth cubemaps for each light
	render_shadowmaps(); // done at load time

	gfx.bindFramebuffer(0);

	render_to_framebuffer();

	gfx.clear();

	if (spawn != -1 && entity_list[spawn]->player->current_light == 0)
		render_texture(quad_tex);
	else
	{
		for (unsigned int i = 0; i < entity_list.size(); i++)
		{
			if (entity_list[i]->light)
			{
				if (entity_list[i]->light->light_num == entity_list[spawn]->player->current_light)
				{
					if (input.control)
					{
						testObj = entity_list[i]->light->depth_tex[entity_list[spawn]->player->current_face];
					}
					else
					{
						testObj = entity_list[i]->light->quad_tex[entity_list[spawn]->player->current_face];
					}
					break;
				}
			}
		}

		render_texture(testObj);
	}
#endif
#ifdef FORWARD
	gfx.clear();
	render_scene(true);
#endif
#ifdef SHADOWVOL
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
	gfx.CullFace("front");
	render_shadow_volumes(0);

	gfx.Depth(true);
	gfx.Color(true);
	gfx.CullFace("back");

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
#endif


	//Handle realtime keys (typing can be slower)
	handle_input();

	//render menu
	if (menu.chatmode == false)
		game->render_hud(last_frametime);
	if (menu.ingame)
		menu.render(global);
	if (menu.console)
		menu.render_console(global);
	if (menu.chatmode)
		menu.render_chatmode(global);
	gfx.swap();
}

void Engine::zoom(float level)
{
	projection.perspective(fov / level, (float)xres / yres, zNear, zFar, inf);
}

void Engine::render_shadowmaps()
{
	for (unsigned int i = 0; i < entity_list.size(); i++)
	{
		if (entity_list[i]->light && light_list[entity_list[find_type("player", 0)]->player->current_light] == entity_list[i]->light)
		{
			Light *light = entity_list[i]->light;

			matrix4 cube[6];

			// Generate matrices
			matrix4::mat_right(cube[0], entity_list[i]->position);
			matrix4::mat_left(cube[1], entity_list[i]->position);
			matrix4::mat_top(cube[2], entity_list[i]->position);
			matrix4::mat_bottom(cube[3], entity_list[i]->position);
			matrix4::mat_forward(cube[4], entity_list[i]->position);
			matrix4::mat_backward(cube[5], entity_list[i]->position);
			for (int j = 0; j < 6; j++)
			{
				matrix4 mvp = cube[j] * projection;

				gfx.fbAttachTexture(light->quad_tex[j]);
//				gfx.fbAttachTexture(0);
				gfx.fbAttachDepth(light->depth_tex[j]);
				gfx.clear();
//				gfx.Color(false);
				//		glDrawBuffer(GL_NONE);
				//		glReadBuffer(GL_NONE);
				//gfx.CullFace("front");

				render_entities(cube[j], true);
				//gfx.CullFace("back");

				mlight2.Select();
				vec3 offset(0.0f, 0.0f, 0.0f);
				mlight2.Params(mvp, light_list, light_list.size(), offset, tick_num);
				q3map.render(entity_list[i]->position, mvp, gfx, surface_list, mlight2, tick_num);
//				gfx.SelectShader(0);
//				gfx.Color(true);
			}
		}
	}
}


void Engine::render_to_framebuffer()
{
	gfx.bindFramebuffer(fbo);
	gfx.resize(fb_width, fb_height);
	gfx.fbAttachTexture(quad_tex);
	gfx.fbAttachDepth(depth_tex);

	gfx.clear();
	render_scene_using_shadowmap(true);
/*
	if (spawn != -1)
	{
		render_shadow_volumes(entity_list[spawn]->player->current_light);
	}
	*/

	gfx.bindFramebuffer(0);
	gfx.resize(xres, yres);
}

void Engine::render_texture(int texObj)
{
	gfx.SelectTexture(0, texObj);
	gfx.SelectIndexBuffer(Model::quad_index);
	gfx.SelectVertexBuffer(Model::quad_vertex);
	global.Select();
	global.Params(identity, 0);
	gfx.DrawArrayTri(0, 0, 6, 4);
}

void Engine::render_scene(bool lights)
{
	matrix4 transformation;
	matrix4 mvp;
	vec3 offset(0.0f, 0.0f, 0.0f);

	int player = find_type("player", 0);
	if (player != -1)
		entity_list[player]->rigid->frame2ent(&camera_frame, input);

	camera_frame.set(transformation);

	// Rendering entities before map for blends
	render_entities(transformation, lights);
	render_players(transformation, lights);

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

#ifndef DIRECTX
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
		emitter.visible = false;
		emitter.position = vec3(0.0f, -10000.0, 0.0f);
	}
#endif

	camera_frame.set(transformation);
	mvp = transformation * projection;

	vec3 quad1 = camera_frame.up;
	vec3 quad2 = vec3::crossproduct(camera_frame.up, camera_frame.forward);

#ifndef DIRECTX
	if (emitter.visible)
	{
		particle_render.Select();
		particle_render.Params(mvp, quad1, quad2, 0.0f, 0.0f);
		gfx.SelectTexture(0, particle_tex);
		particle_render.render(gfx, 0, vbo, emitter.num);
	}


	render_trails(transformation);
#endif
#endif

	render_weapon(transformation, lights, find_type("player", 0));
}


// Texture arrays let me pass all the depthmaps
// Next problem is the transformed interpolated fragment per face, per light
// Right now I can only get four shadow omni-lights with this limit
void Engine::render_scene_using_shadowmap(bool lights)
{
	matrix4 mvp;
	matrix4 transformation;

	if (input.control == false)
	{
		int player = find_type("player", 0);

		if (player != -1)
		{
			entity_list[player]->rigid->frame2ent(&camera_frame, input);
		}

		camera_frame.set(transformation);
	}
	else
	{
		light_frame.set(transformation);
	}


	//shadowmap.Select();
		
	render_entities(transformation, lights);
	vec3 offset(0.0f, 0.0f, 0.0f);
	mlight2.Select();
	mvp = transformation * projection;
	mlight2.Params(mvp, light_list, light_list.size(), offset, tick_num);


//	shadowmap.Params(mvp, shadowmvp);
	if (light_list.size())
	{
		int num_shadow_cube = 0;
		for (unsigned int i = 0; i < entity_list.size(); i++)
		{
			if (entity_list[i]->light)
			{
				gfx.SelectTexture(num_shadow_cube, entity_list[i]->light->depth_tex[0]);
				gfx.SelectTexture(num_shadow_cube, entity_list[i]->light->depth_tex[1]);
				gfx.SelectTexture(num_shadow_cube, entity_list[i]->light->depth_tex[2]);
				gfx.SelectTexture(num_shadow_cube, entity_list[i]->light->depth_tex[3]);
				gfx.SelectTexture(num_shadow_cube, entity_list[i]->light->depth_tex[4]);
				gfx.SelectTexture(num_shadow_cube, entity_list[i]->light->depth_tex[5]);
				num_shadow_cube++;
			}
		}
	}

	if (input.control)
	{
		q3map.render(light_frame.pos, mvp, gfx, surface_list, mlight2, tick_num);
//		render_shadow_volumes(entity_list[spawn]->player->current_light);
//		gfx.SelectShader(0);
		return;
	}

	q3map.render(camera_frame.pos, mvp, gfx, surface_list, mlight2, tick_num);
	q3map.render_model(0, gfx);

//	for (int i = 0; i < q3map.data.num_model; i++)
//	{
//	}
//	gfx.SelectShader(0);
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
		if (entity_list[i]->model == NULL)
			continue;


		if (once == false && (entity_list[i]->model->rail_trail || entity_list[i]->model->lightning_trail))
		{
			//			vec3 offset = entity_list[i]->position;

			// Undo model orientation
			quad1 = entity_list[i]->model->morientation.transpose() * quad1;
			quad2 = entity_list[i]->model->morientation.transpose() * quad2;

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

void Engine::render_entities(const matrix4 &trans, bool lights)
{
	matrix4 mvp;

	if (entities_enabled == false)
		return;

	emitter.enabled = false;


	gfx.Blend(false);
	mlight2.Select();
	for (unsigned int i = 0; i < entity_list.size(); i++)
	{
		Entity *entity = entity_list[i];

		if (entity->visible == false)
			continue;

		if (entity->nodraw == true)
			continue;

		if (entity->rigid == NULL)
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
		mvp = (mvp * trans) * projection;
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

		//  update emitter position if this entity has particles
		if (entity->particle_on)
		{
			emitter.enabled = true;
			emitter.num = entity->num_particle;
			emitter.position = entity->position;
			emitter.gravity = vec3(0.0f, 30.0f, 0.0f);
		}
	}
}

void Engine::render_players(matrix4 &trans, bool lights)
{
	matrix4 mvp;
	//render player md5
	for (unsigned int i = 0; i < max_player; i++)
	{
		Entity *entity = entity_list[i];

		if (entity->visible == false)
			continue;

		if (	(entity->player && entity->player->type == BOT) ||
			(entity->player && entity->player->type == SPECTATOR) ||
			(entity->player && entity->player->type == SERVER) ||
			(entity->player && entity->player->type == CLIENT && server_flag))
		{
			if (entity->player->local)
				continue;

			if (entity->player->health > 0)
			{

				//md5 faces right, need to flip right and forward orientation
				vec4 temp;

				entity->rigid->get_matrix(mvp.m);

				temp.x = mvp.m[0];
				temp.y = mvp.m[1];
				temp.z = mvp.m[2];
				temp.w = mvp.m[3];

				mvp.m[0] = mvp.m[8];
				mvp.m[1] = mvp.m[9];
				mvp.m[2] = mvp.m[10];
				mvp.m[3] = mvp.m[11];

				mvp.m[8] = -temp.x;
				mvp.m[9] = -temp.y;
				mvp.m[10] = -temp.z;
				mvp.m[11] = -temp.w;


				mvp = (mvp * trans) * projection;
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
			if (input.control)
			{
				light_frame.set(transformation);
			}
			else
			{
				camera_frame.set(transformation);
			}

			mvp = transformation * projection;
			global.Select();
			global.Params(mvp, 0);
			entity_list[i]->light->render_shadow_volumes(current_light);
//			gfx.SelectShader(0);
		}
	}
}

void Engine::post_process(int num_passes)
{
	int temp;

	gfx.SelectTexture(0, post.image);
	for (int pass = 0; pass < num_passes; pass++)
	{
#ifndef DIRECTX
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


void Engine::handle_input()
{
	if (input.numpad0)
	{
		post_process(5);
	}



	if (input.numpad2)
	{
		vec3 right(-1.0f, 0.0f, 0.0f);
		vec3 up(0.0f, 1.0f, 0.0f);
		vec3 forward(0.0f, 0.0f, 1.0f);

		camera_frame.forward = forward;
		camera_frame.up = up;
	}

	if (input.numpad3)
	{
		vec3 right(1.0f, 0.0f, 0.0f);
		vec3 up(0.0f, 0.0f, 1.0f);
		vec3 forward(0.0f, 1.0f, 0.0f);

		camera_frame.forward = forward;
		camera_frame.up = up;
	}

	if (input.numpad6)
	{
		vec3 right(0.0f, 0.0f, 1.0f);
		vec3 up(0.0f, 1.0f, 0.0f);
		vec3 forward(1.0f, 0.0f, 0.0f);

		camera_frame.forward = forward;
		camera_frame.up = up;
	}


	if (input.numpad4)
	{
		vec3 right(0.0f, 0.0f, -1.0f);
		vec3 up(0.0f, 1.0f, 0.0f);
		vec3 forward(-1.0f, 0.0f, 0.0f);

		camera_frame.forward = forward;
		camera_frame.up = up;
	}

	if (input.numpad8)
	{
		vec3 right(1.0f, 0.0f, 0.0f);
		vec3 up(0.0f, 1.0f, 0.0f);
		vec3 forward(0.0f, 0.0f, -1.0f);

		camera_frame.forward = forward;
		camera_frame.up = up;
	}

	if (input.numpad9)
	{
		vec3 right(1.0f, 0.0f, 0.0f);
		vec3 up(0.0f, 0.0f, -1.0f);
		vec3 forward(0.0f, -1.0f, 0.0f);


		camera_frame.forward = forward;
		camera_frame.up = up;
	}
}

void Engine::spatial_testing()
{
	int leaf_a = -1;
	int leaf_b = -1;

	// Run this at ~20 tims a second
	if (tick_num % 6 != 0)
		return;

	if (q3map.vis_test(emitter.position, camera_frame.pos,
		leaf_a, leaf_b))
	{
		emitter.visible = true;
	}
	else
	{
		emitter.visible = false;
	}

	for (unsigned int i = 0; i < entity_list.size(); i++)
	{
		// set pursue / evade
		// (really need to move elsewhere, but had an entity loop here)
		if (entity_list[i]->rigid)
		{
			if (entity_list[i]->rigid->target)
			{
				if (entity_list[i]->rigid->pursue_flag == true)
				{
//					entity_list[i]->rigid->wander(20.0f, 1.0f, 5.0f);
					entity_list[i]->rigid->pursue();
				}
				else
				{
					entity_list[i]->rigid->evade();
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
			vec3 min = entity_list[i]->model->aabb[0];
			vec3 max = entity_list[i]->model->aabb[7];



			if (entity_list[i]->model && entity_list[i]->model->rail_trail)
			{
				entity_list[i]->frustum_visible = true; // trail extends past aabb
			}
			else
			{
				entity_list[i]->frustum_visible = aabb_visible(min, max, mvp);
			}


			for(int j = 0; j < 8; j++)
			{
				vec3 position = entity_list[i]->position + entity_list[i]->model->aabb[j];
				bool vert_visible = q3map.vis_test(camera_frame.pos, position, leaf_a, leaf_b);

				if (vert_visible)
				{
					bsp_visible = true;
					break;
				}
			}

			entity_list[i]->bsp_visible = bsp_visible;

			int player = find_type("player", 0);

			if (player != -1)
				entity_list[player]->bsp_leaf = leaf_a;
			entity_list[i]->bsp_leaf = leaf_b;

			if ((bsp_visible && entity_list[i]->frustum_visible) || i == (unsigned int)player)
			{
				visible = true;
			}


			// make triggered entities disappear
			if (entity_list[i]->trigger)
			{
				if (entity_list[i]->trigger->hide == false)
				{
					//Always show with hide flag false
					entity_list[i]->visible = visible;
				}
				else if (entity_list[i]->trigger->active == false)
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

			int player = find_type("player", 0);

			if (player != -1)
				entity_list[player]->bsp_leaf = leaf_a;

			entity_list[i]->visible = q3map.vis_test(camera_frame.pos, entity_list[i]->position, leaf_a, leaf_b);
			entity_list[i]->bsp_leaf = leaf_b;
		}

		if (entity_list[i]->visible == false)
			continue;

		vec3 dist_vec = entity_list[i]->position - camera_frame.pos;
		float distance = dist_vec.x * dist_vec.x + dist_vec.y * dist_vec.y + dist_vec.z * dist_vec.z;

		if (entity_list[i]->light)
		{
			if (entity_list[i]->light->timer_flag)
			{
				if (entity_list[i]->light->timer > 0)
				{
					entity_list[i]->light->timer -= 25;
				}
				else
				{
					clean_entity(i);
				}
			}

			activate_light(distance, entity_list[i]->light);
		}

	}

}

void Engine::activate_light(float distance, Light *light)
{
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
	cfg_t	config;

	#pragma omp parallel for num_threads(8)
	for(unsigned int i = 0; i < entity_list.size(); i++)
	{
		if (entity_list[i]->rigid == NULL)
			continue;

		if (entity_list[i]->nodraw || entity_list[i]->rigid->noclip)
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
			float volume = body->get_volume();
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
			float force_bouyant = volume * 0.0015f * GRAVITY * submerged_percent;

			body->net_force += vec3(0.0f, force_bouyant, 0.0f);
		}



		while (current_time < delta_time)
		{
			body->save_config(config);
			body->integrate(target_time - current_time);
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

//	if (body_collision(body))
//		return true;

	return false;
}

bool Engine::map_collision(RigidBody &body)
{
	Plane plane;
	float depth;
	vec3 staircheck(0.0f, STAIR_HEIGHT, 0.0f);
	vec3 clip(0.0f, 0.0f, 0.0f);
	bool collision = false;


	if (body.noclip)
		return false;

	vec3 mid[4];
	
	// Do additional mid point testing (front back left right points, mid level)
	mid[0] = body.aabb[0] + vec3(body.aabb[7].x / 2, 0, body.aabb[7].z / 2);
	mid[1] = body.aabb[0] + vec3(0.0f, body.aabb[7].y / 2, body.aabb[7].z / 2);
	mid[2] = body.aabb[0] + vec3(body.aabb[7].x / 2, body.aabb[7].y, body.aabb[7].z / 2);
	mid[3] = body.aabb[0] + vec3(body.aabb[7].x, body.aabb[7].y / 2, body.aabb[7].z / 2);

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
			point = body.aabb[i] - body.center + body.entity->position;
			oldpoint = body.aabb[i] - body.center + body.old_position;
		}


//		can be used to avoid checking all eight points, but checking all 8 works pretty well
//		vec3 point = body.center + body.entity->position;
//		point -= vec3(0.0f, 100.0f, 0.0f); // subtract player height

		if (q3map.collision_detect(point, oldpoint, (plane_t *)&plane, &depth, body.water, body.water_depth,
			surface_list, body.step_flag && input.numpad1, clip, body.velocity))
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
						surface_list, body.step_flag && input.numpad1, clip, body.velocity) == false)
					{
						body.entity->position += vec3(0.0f, STAIR_POS, 0.0f);
						body.velocity += vec3(0.0f, STAIR_VEL, 0.0f);
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
	if (collision == false && body.velocity.magnitude() > 6.0f )
	{
		for (int i = 0; i < 8; i++)
		{
			vec3 point;
			vec3 oldpoint;

			point = body.aabb[i] - body.center + body.entity->position;
			oldpoint = body.aabb[i] - body.center + body.old_position;

			q3map.trace(oldpoint, point);

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
		body.velocity = clip + vec3(0.0f, 0.1f, 0.0f);
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

		if (body.entity->bsp_leaf == entity_list[i]->bsp_leaf)
			body.collision_detect(*entity_list[i]->rigid);
	}
	return false;
}

void Engine::step(int tick)
{
	tick_num = tick;
	if (q3map.loaded == false)
		return;

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

	game->step(tick);

	dynamics();
#ifndef DEDICATED
	update_audio();
#endif
}

void Engine::server_send_state(int client)
{
	serverdata_t data;
	game->get_state(&data);
	memcpy(reliable.msg, &data, sizeof(serverdata_t));
	reliable.size = 2 * sizeof(int) + sizeof(serverdata_t);
	reliable.sequence = sequence;
}


void Engine::server_recv()
{
	servermsg_t	servermsg;
	clientmsg_t clientmsg;
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
		if (clientmsg.server_sequence > reliable.sequence)
		{
			memset(reliable.msg, 0, LINE_SIZE);
			reliable.size = 0;
			reliable.sequence = -1;
		}

		if ((unsigned int)client_list[index]->ent_id > entity_list.size())
		{
			printf("Invalid Entity\n");
			return;
		}

		client_list[index]->last_time = (unsigned int)time(NULL);


		Frame client_frame;

		client_frame.up.x = clientmsg.up[0];
		client_frame.up.y = clientmsg.up[1];
		client_frame.up.z = clientmsg.up[2];
		client_frame.forward.x = clientmsg.forward[0];
		client_frame.forward.y = clientmsg.forward[1];
		client_frame.forward.z = clientmsg.forward[2];
		vec3 right = vec3::crossproduct(client_frame.up, client_frame.forward);
		right.normalize();

		Entity *client = entity_list[client_list[index]->ent_id];

		client_frame.set(client->rigid->morientation);
		client_frame.pos = client->position + client->rigid->center;
		client_list[index]->input = clientkeys;




		/*
		printf("-> server_sequence %d\n"
			"<- client_sequence %d\n"
			"<- clients_server_sequence %d\n"
			"client delay %d steps\n"
			"reliable msg: %s\n",
			sequence, clientmsg.sequence, clientmsg.server_sequence,
			sequence - clientmsg.server_sequence,
			reliable.msg);
		*/

		if ((unsigned int)clientmsg.length > CLIENT_HEADER + sizeof(int) + sizeof(int) + 1)
		{
			reliablemsg_t *reliablemsg = (reliablemsg_t *)&clientmsg.data[4];

			if (client_list[index]->client_sequence <= reliablemsg->sequence)
			{
				char name[LINE_SIZE] = { 0 };
				char msg[LINE_SIZE] = { 0 };

				debugf("client to server: %s\n", reliablemsg->msg);


				if (strstr(reliablemsg->msg, "chat"))
				{
					sprintf(name, "%s", reliablemsg->msg + 5);
					char *end = strstr(name, ":");

					end[0] = '\0';
					sprintf(msg, "say \"%s\"", strstr(reliablemsg->msg, ":") + 2);
					//  Echoes chat back to all clients
					chat(name, msg);
				}
			}
		}
		netinfo.sequence_delta = sequence - clientmsg.sequence;
		netinfo.ping = netinfo.sequence_delta * TICK_MS;
		client_list[index]->client_sequence = clientmsg.sequence;
	}
	else
	{
		printf("Got input packet from unconnected client %s qport %d\n", socketname, clientmsg.qport);
	}

	// client not in list, check if it is a connect msg
	reliablemsg_t *reliablemsg = (reliablemsg_t *)&clientmsg.data[clientmsg.num_cmds * sizeof(int)];
	if ( strcmp(reliablemsg->msg, "connect") == 0 )
	{
		debugf("client %s qport %d connected\n", socketname, clientmsg.qport);
		servermsg.sequence = sequence;
		servermsg.client_sequence = clientmsg.sequence;
		servermsg.num_ents = 0;
		sprintf(reliable.msg, "map %s", q3map.map_name);
		reliable.size = 2 * sizeof(int) + strlen(reliable.msg) + 1;
		reliable.sequence = sequence;

		memcpy(&servermsg.data[servermsg.num_ents * sizeof(entity_t)], &reliable, reliable.size);
		servermsg.length = SERVER_HEADER + servermsg.num_ents * sizeof(entity_t) + reliable.size;
		net.sendto((char *)&servermsg, servermsg.length, socketname);
		debugf("sent client map data\n");
	}
	else if ( strcmp(reliablemsg->msg, "spawn") == 0 )
	{
		bool found = false;
		client_t *client = (client_t *)malloc(sizeof(client_t));
		if (client == NULL)
		{
			debugf("malloc failed allocating client");
			return;
		}

		client->last_time = (unsigned int)time(NULL);
		strcpy(client->socketname, socketname);
		client->qport = clientmsg.qport;
		client->needs_state = true;

		for (unsigned int i = 0; i < client_list.size(); i++)
		{
			if (strcmp(client_list[i]->socketname, client->socketname) == 0)
			{
				if (client_list[i]->qport == client->qport)
				{
					found = true;
					break;
				}
			}
		}

		if (found)
		{
			printf("Client already spawned, ignoring\n");
			free((void *)client);
			return;
		}

		client_list.push_back(client);
		debugf("client %s qport %d spawned\n", client->socketname, client->qport);
		client->client_sequence = clientmsg.sequence;

		// assign entity to client
		//set to zero if we run out of info_player_deathmatches

		char client_name[80];

		sprintf(client_name, "client %d", (int)client_list.size() - 1);
		game->add_player(entity_list, CLIENT, client->ent_id, client_name);
		printf("client %s qport %d got entity %d\n", socketname, client->qport, client->ent_id);


		servermsg.sequence = sequence;
		servermsg.client_sequence = clientmsg.sequence;
		servermsg.num_ents = 0;
		
		sprintf(reliable.msg, "spawn %d %d", client->ent_id, find_type("player", 0));
		reliable.size = 2 * sizeof(int) + strlen(reliable.msg) + 1;
		reliable.sequence = sequence;
		memcpy(&servermsg.data[servermsg.num_ents * sizeof(entity_t)], &reliable, reliable.size);
		servermsg.length = SERVER_HEADER + servermsg.num_ents * sizeof(entity_t) + reliable.size;
		net.sendto((char *)&servermsg, servermsg.length, client->socketname);
		debugf("Client is now entity %d\n", client->ent_id);
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
		sprintf(reliable.msg, "/servername %s/map %s/players %d/maxplayers %d/gametype %d/fraglimit %d/timelimit %d/capturelimit %d/",
			servername, q3map.map_name, client_list.size(), max_player, game->gametype, game->fraglimit, game->timelimit, game->capturelimit);
		reliable.size = 2 * sizeof(int) + strlen(reliable.msg) + 1;
		reliable.sequence = sequence;

		memcpy(&servermsg.data[servermsg.num_ents * sizeof(entity_t)], &reliable, reliable.size);
		servermsg.length = SERVER_HEADER + servermsg.num_ents * sizeof(entity_t) + reliable.size;
		net.sendto((char *)&servermsg, servermsg.length, socketname);
		debugf("sent client map data\n");

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

		if (sscanf(reliablemsg->msg, "rcon %s %s", &pass, &cmd) == 2)
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

void Engine::server_send()
{
//	static entity_t old_ent[1024];
//	unsigned char	*compressed = NULL;
//	unsigned char	*data = NULL;
//	unsigned int	compressed_length = 0;
	servermsg_t	servermsg;

	servermsg.sequence = sequence;
	servermsg.client_sequence = 0;
	servermsg.num_ents = 0;


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


		for (unsigned int j = 0; j < entity_list.size(); j++)
		{
			entity_t ent;
			/*
			// Vis test causing issues for some reason
			int leaf_a;
			int leaf_b;

			bool visible = q3map.vis_test(entity_list[j]->position,
				entity_list[client_list[i]->ent_id]->position, leaf_a, leaf_b);
			if ( visible == false )
				continue;
			*/

			ent.id = j;
			ent.type = entity_list[j]->nettype;
			ent.active = false;

			if (entity_list[j]->trigger)
			{
				if (entity_list[j]->trigger->active)
					ent.active = true;
				ent.owner = entity_list[j]->trigger->owner;
			}

			if (entity_list[j]->rigid)
			{
				ent.morientation = entity_list[j]->rigid->morientation;
				ent.angular_velocity = entity_list[j]->rigid->angular_velocity;
				ent.velocity = entity_list[j]->rigid->velocity;
				ent.position = entity_list[j]->position;
			}


			if (entity_list[j]->player)
			{
				ent.health = entity_list[j]->player->health;
				ent.armor = entity_list[j]->player->armor;
				ent.weapon_flags = entity_list[j]->player->weapon_flags;
				//ent.current_weapon = entity_list[j]->player->current_weapon;
				ent.ammo_bullets = entity_list[j]->player->ammo_bullets;
				ent.ammo_shells = entity_list[j]->player->ammo_shells;
				ent.ammo_rockets = entity_list[j]->player->ammo_rockets;
				ent.ammo_lightning = entity_list[j]->player->ammo_lightning;
				ent.ammo_slugs = entity_list[j]->player->ammo_slugs;
				ent.ammo_plasma = entity_list[j]->player->ammo_plasma;
			}

//			if (memcmp((void *)&old_ent[j], (void *)&ent, sizeof(entity_t)) != 0)
			if (1)
			{
			//	old_ent[j] = ent;
				memcpy(&servermsg.data[j * sizeof(entity_t)],
					&ent, sizeof(entity_t));
				servermsg.num_ents++;
			}
		}

		if (client_list[i]->needs_state)
		{
			server_send_state(i);
			client_list[i]->needs_state = false;
		}

		memcpy(&servermsg.data[servermsg.num_ents * sizeof(entity_t)], (void *)&reliable, reliable.size);
		servermsg.length = SERVER_HEADER + servermsg.num_ents * sizeof(entity_t) + reliable.size;
		servermsg.client_sequence = client_list[i]->client_sequence;

		client_list[i]->netinfo.num_ents = servermsg.num_ents;
		client_list[i]->netinfo.size = servermsg.length;
		
		if (servermsg.length > 8192)
		{
			//printf("Warning: Server packet too big!\nsize %d\n", servermsg.length);
		}

//		unsigned int dsize = 0;
//		huffman_encode_memory((unsigned char *)&servermsg, servermsg.length, &compressed, &compressed_length);
//		int num_sent = net.sendto((char *)&compressed, compressed_length, client_list[i]->socketname);
//		free((void *)compressed);
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
	}
}

void Engine::client_recv()
{
//	static unsigned char	compressed[256000];
//	unsigned char *data = NULL;
	servermsg_t	servermsg;
	reliablemsg_t *reliablemsg = NULL;
	unsigned int socksize = sizeof(sockaddr_in);


	// get entity information
#ifdef WIN32
	int size = ::recvfrom(net.sockfd, (char *)&servermsg, sizeof(servermsg_t), 0, (sockaddr *)&(net.servaddr), (int *)&socksize);
#else
	int size = ::recvfrom(net.sockfd, (char *)&servermsg, sizeof(servermsg_t), 0, (sockaddr *)&(net.servaddr), (unsigned int *)&socksize);
#endif
	if ( size > 0)
	{
// huffman_decode doesnt do too well with partial data, might need to pack into a struct
//		unsigned int dsize = 0;
//		memset(&compressed, 0, 256000);
//		huffman_decode_memory((unsigned char *)compressed, (unsigned int)size, (unsigned char **)&data, &dsize);
//		servermsg = (servermsg_t *)&data;
//		size = dsize;

		if (size != servermsg.length)
		{
			printf("Packet size mismatch: %d %d\n", size, servermsg.length);
			return;
		}

		if (servermsg.sequence <= last_server_sequence)
		{
			printf("Got old server packet\n");
			netinfo.dropped++;
			return;
		}

//		printf("Recieved %d ents from server\n", servermsg.num_ents);
		if (servermsg.client_sequence > reliable.sequence)
		{
			memset(reliable.msg, 0, LINE_SIZE);
			reliable.size = 0;
			reliable.sequence = -1;
		}

		if ((unsigned int)servermsg.length > SERVER_HEADER + servermsg.num_ents * sizeof(entity_t) + sizeof(int) + 1)
		{
			reliablemsg = (reliablemsg_t *)&servermsg.data[servermsg.num_ents * sizeof(entity_t)];
		}


		netinfo.num_ents = servermsg.num_ents;
		netinfo.size = servermsg.length;
		netinfo.sequence_delta = sequence - servermsg.client_sequence;
		netinfo.ping = netinfo.sequence_delta * TICK_MS;

		handle_servermsg(servermsg, reliablemsg);

		/*
		printf("-> client_sequence %d\n"
			"<- server_sequence %d\n"
			"<- servers_client_sequence %d\n"
			"server delay %d steps\n"
			"reliable msg: %s\n",
			sequence, servermsg.sequence, servermsg.client_sequence,
			sequence - servermsg.client_sequence,
			reliable.msg);
		*/


		last_server_sequence = servermsg.sequence;
//		free((void *)data);
	}
}

void Engine::client_send()
{
	clientmsg_t clientmsg;
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
	clientmsg.num_cmds = 1;
	memcpy(clientmsg.data, &keystate, sizeof(int));
	memcpy(&clientmsg.data[clientmsg.num_cmds * sizeof(int)], (void *)&reliable, reliable.size);
	clientmsg.length = CLIENT_HEADER + clientmsg.num_cmds * sizeof(int) + reliable.size;
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


int Engine::handle_servermsg(servermsg_t &servermsg, reliablemsg_t *reliablemsg)
{
	if (reliablemsg != NULL)
	{
		if (last_server_sequence <= reliablemsg->sequence)
		{
			int client;

			debugf("server to client: %s\n", reliablemsg->msg);

			if (strstr(reliablemsg->msg, "chat"))
			{
				menu.print_chat(reliablemsg->msg + 5);
				game->chat_timer = 3 * TICK_RATE;

				#define SND_TALK 267
				play_wave_global(SND_TALK);
			}

			if (strstr(reliablemsg->msg, "data"))
			{
				serverdata_t *data = (serverdata_t *)reliablemsg->msg;

				debugf("client received server state\n");
				game->set_state(data);
			}

			int ret = sscanf(reliablemsg->msg, "spawn %d %d", &client, &server_spawn);
			if (ret == 2)
			{
				clean_entity(client);
				sprintf(entity_list[client]->type, "player");
				entity_list[client]->rigid = new RigidBody(entity_list[client]);
				entity_list[client]->model = entity_list[client]->rigid;
				entity_list[client]->rigid->clone(*(thug22->model));
				entity_list[client]->rigid->step_flag = true;
				entity_list[client]->position += entity_list[client]->rigid->center;
				entity_list[client]->player = new Player(entity_list[client], gfx, audio, 21, TEAM_NONE);
				entity_list[client]->player->type = PLAYER;
				entity_list[client]->player->local = true;
				camera_frame.pos = entity_list[client]->position;

				if (server_spawn != -1)
				{
					clean_entity(server_spawn);
					sprintf(entity_list[server_spawn]->type, "server");
					entity_list[server_spawn]->rigid = new RigidBody(entity_list[server_spawn]);
					entity_list[server_spawn]->model = entity_list[server_spawn]->rigid;
					entity_list[server_spawn]->rigid->clone(*(thug22->model));
					entity_list[server_spawn]->rigid->step_flag = true;
					entity_list[server_spawn]->position += entity_list[server_spawn]->rigid->center;
					entity_list[server_spawn]->player = new Player(entity_list[server_spawn], gfx, audio, 21, TEAM_NONE);
					entity_list[server_spawn]->player->local = false;
					entity_list[server_spawn]->player->type = SERVER;
				}
			}

			ret = strcmp(reliablemsg->msg, "disconnect");
			if (ret == 0)
			{
				unload();
			}
		}
	}



	for (int i = 0; i < servermsg.num_ents; i++)
	{
		entity_t	*ent = (entity_t *)servermsg.data;

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

		if (entity_list[ent[i].id]->player)
		{
			entity_list[ent[i].id]->player->health = ent[i].health;
			entity_list[ent[i].id]->player->armor = ent[i].armor;
			entity_list[ent[i].id]->player->weapon_flags = ent[i].weapon_flags;
			// will force server to sync to our current weapon
//			entity_list[ent[i].id]->player->current_weapon = ent[i].current_weapon;

			if (ent[i].ammo_bullets - entity_list[ent[i].id]->player->ammo_bullets > 1)
				entity_list[ent[i].id]->player->ammo_bullets = ent[i].ammo_bullets;
			if (ent[i].ammo_shells - entity_list[ent[i].id]->player->ammo_shells > 1)
				entity_list[ent[i].id]->player->ammo_shells = ent[i].ammo_shells;
			if (ent[i].ammo_rockets - entity_list[ent[i].id]->player->ammo_rockets > 1)
				entity_list[ent[i].id]->player->ammo_rockets = ent[i].ammo_rockets;
			if (ent[i].ammo_lightning - entity_list[ent[i].id]->player->ammo_lightning > 1)
				entity_list[ent[i].id]->player->ammo_lightning = ent[i].ammo_lightning;
			if (ent[i].ammo_slugs - entity_list[ent[i].id]->player->ammo_slugs > 1)
				entity_list[ent[i].id]->player->ammo_slugs = ent[i].ammo_slugs;
			if (ent[i].ammo_plasma - entity_list[ent[i].id]->player->ammo_plasma > 1)
				entity_list[ent[i].id]->player->ammo_plasma = ent[i].ammo_plasma;

		}



		if (entity_list[ent[i].id]->rigid)
		{
			if ((unsigned int)find_type("player", 0) == ent[i].id)
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
				entity_list[ent[i].id]->position = ent[i].position;
				entity_list[ent[i].id]->rigid->velocity = ent[i].velocity;
				entity_list[ent[i].id]->rigid->angular_velocity = ent[i].angular_velocity;
				entity_list[ent[i].id]->rigid->morientation = ent[i].morientation;
			}
		}
		else
		{
			entity_list[ent[i].id]->position = ent[i].position;
		}
	}


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

	camera_frame.update(vec2((float)deltax, (float)deltay));
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
		key_bind.insert("shift", "duck");
		key_bind.insert("escape", "escape");
		key_bind.insert("control", "control");
		key_bind.insert("up", "moveup");
		key_bind.insert("down", "movedown");
		key_bind.insert("left", "moveleft");
		key_bind.insert("right", "moveright");

		key_bind.insert("W", "moveup");
		key_bind.insert("A", "moveleft");
		key_bind.insert("S", "movedown");
		key_bind.insert("D", "moveright");
		key_bind.insert("tab", "scores");


		key_bind.insert("numpad0", "numpad0");
		key_bind.insert("numpad1", "numpad1");
		key_bind.insert("numpad2", "numpad2");
		key_bind.insert("numpad3", "numpad3");
		key_bind.insert("numpad4", "numpad4");
		key_bind.insert("numpad5", "numpad5");
		key_bind.insert("numpad6", "numpad6");
		key_bind.insert("numpad7", "numpad7");
		key_bind.insert("numpad8", "numpad8");
		key_bind.insert("numpad9", "numpad9");

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
	free((void *)file);

	//TBD
	/*
	key_bind.insert(make_pair((char *)"~", (char *)"console"));
	key_bind.insert(make_pair((char *)"k", (char *)"kill"));
	key_bind.insert(make_pair((char *)"t", (char *)"talk"));
	key_bind.insert(make_pair((char *)"q", (char *)"walk"));
	key_bind.insert(make_pair((char *)"F1", (char *)"voteyes"));
	key_bind.insert(make_pair((char *)"F2", (char *)"voteno"));
	key_bind.insert(make_pair((char *)"F11", (char *)"screenshot"));
	*/


	//seta com_maxfps "85"
	//console(-1, "com_maxfps")
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
	seta sv_maxclients "8"
	seta sv_hostname "noname"
	rate
	cmdrate
	sensitivity
	name
	model
	skin
	team red/blue
	seta cg_autoswitch "0"
	resolution
	seta r_customheight "1080"
	seta r_customwidth "1080"
	net_port 65535
	banip
	*/



}

void Engine::keypress(char *key, bool pressed)
{
	char k = 0;
	char *cmd = (char *)key_bind.find(key);
	if (cmd == NULL)
		return;

	if (strcmp("attack", cmd) == 0)
	{
		input.attack = pressed;
		k = 14;
	}
	else if (strcmp("moveup", cmd) == 0)
	{
		input.moveup = pressed;
		if (*key != 'w' && *key != 'W')
			k = 3;
	}
	else if (strcmp("moveleft", cmd) == 0)
	{
		input.moveleft = pressed;
		if (*key != 'a' && *key != 'A')
			k = 4;
	}
	else if (strcmp("movedown", cmd) == 0)
	{
		input.movedown = pressed;
		if (*key != 's' && *key != 'S')
			k = 5;
	}
	else if (strcmp("moveright", cmd) == 0)
	{
		input.moveright = pressed;
		if (*key != 'd' && *key != 'D')
			k = 6;
	}
	else if (strcmp("jump", cmd) == 0)
	{
		input.jump = pressed;
	}
	else if (strcmp("duck", cmd) == 0)
	{
		input.duck = pressed;
	}
	else if (strcmp("weapon_up", cmd) == 0)
	{
		int spawn = find_type("player", 0);

		input.weapon_up = pressed;
		if (client_flag)
			entity_list[spawn]->player->change_weapon_up();
	}
	else if (strcmp("weapon_down", cmd) == 0)
	{
		int spawn = find_type("player", 0);

		input.weapon_down = pressed;
		if (client_flag)
			entity_list[spawn]->player->change_weapon_down();
	}
	else if (strcmp("use", cmd) == 0)
	{
		input.use = pressed;
	}
	else if (strcmp("pickup", cmd) == 0)
	{
		input.pickup = pressed;
	}
	else if (strcmp("zoom", cmd) == 0)
	{
		input.zoom = pressed;
	}
	else if (strcmp("control", cmd) == 0)
	{
		input.control = pressed;
	}
	else if (strcmp("escape", cmd) == 0)
	{
		input.escape = pressed;
		k = 27;
	}
	else if (strcmp("numpad0", cmd) == 0)
	{
		input.numpad0 = pressed;
	}
	else if (strcmp("numpad1", cmd) == 0)
	{
		input.numpad1 = pressed;
	}
	else if (strcmp("numpad2", cmd) == 0)
	{
		input.numpad2 = pressed;
	}
	else if (strcmp("numpad3", cmd) == 0)
	{
		input.numpad3 = pressed;
	}
	else if (strcmp("numpad4", cmd) == 0)
	{
		input.numpad4 = pressed;
	}
	else if (strcmp("numpad5", cmd) == 0)
	{
		input.numpad5 = pressed;
	}
	else if (strcmp("numpad6", cmd) == 0)
	{
		input.numpad6 = pressed;
	}
	else if (strcmp("numpad7", cmd) == 0)
	{
		input.numpad7 = pressed;
	}
	else if (strcmp("numpad8", cmd) == 0)
	{
		input.numpad8 = pressed;
	}
	else if (strcmp("numpad9", cmd) == 0)
	{
		input.numpad9 = pressed;
	}
	else if (strcmp("scores", cmd) == 0)
	{
		input.scores = pressed;
	}

	if (pressed)
		keystroke(k);
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
	int spawn = find_type("player", 0);

	switch (key)
	{
	case '~':
	case '`':
		menu.console = !menu.console;
		break;
	case 'T':
	case 't':
		menu.chatmode = true;
		break;
	case 'r':
		camera_frame.reset();
		break;

	case '0':
		if (spawn != -1)
		{
			entity_list[spawn]->player->current_face = 0;
		}
		break;
	case '1':
		if (spawn != -1)
		{
			entity_list[spawn]->player->current_face = 1;
		}
		break;
	case '2':
		if (spawn != -1)
		{
			entity_list[spawn]->player->current_face = 2;
		}
		break;
	case '3':
		if (spawn != -1)
		{
			entity_list[spawn]->player->current_face = 3;
		}
		break;
	case '4':
		if (spawn != -1)
		{
			entity_list[spawn]->player->current_face = 4;
		}
		break;
	case '5':
		if (spawn != -1)
		{
			entity_list[spawn]->player->current_face = 5;
		}
		break;
	case '6':
	case '7':
	case '8':
	case '9':
		break;

	case '-':
		if (spawn != -1)
		{
			if (entity_list[spawn]->player->current_light > 0)
				entity_list[spawn]->player->current_light--;
		}
		break;
	case '=':
		if (spawn != -1)
		{
			if (entity_list[spawn]->player->current_light < num_light)
				entity_list[spawn]->player->current_light++;
		}
		break;

	case 27:
		menu.ingame = true;
		break;
	case '[':
		if (spawn != -1)
		{
			input.weapon_down = true;
			entity_list[spawn]->player->change_weapon_down();
		}
		break;
	case ']':
		if (spawn != -1)
		{
			input.weapon_up = true;
			entity_list[spawn]->player->change_weapon_up();
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

// To prevent making a class that looks exactly like model...
// I will search previous entities for models that are already loaded
void Engine::load_models()
{
	if (entity_list.size() == 0)
		return;


	for(unsigned int i = max_dynamic; i < entity_list.size(); i++)
	{
		bool loaded = false;

		if (entity_list[i]->model == NULL)
			continue;

		for(unsigned int j = max_dynamic; j < i; j++)
		{
			if (entity_list[j]->model == NULL)
				continue;

			entity_list[i]->model->clone(*(box->model));
			if (strcmp(entity_list[i]->type, entity_list[j]->type) == 0)
			{
				entity_list[i]->model->clone(*entity_list[j]->model);
				loaded = true;
				break;
			}
		}

		if (loaded)
			continue;

		load_model(*entity_list[i]);
	}
}

void Engine::load_model(Entity &ent)
{
	if (strcmp(ent.type, "item_armor_shard") == 0)
	{
		debugf("Loading item_armor_shard\n");
		ent.model->load(gfx, "media/models/powerups/armor/shard");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (strcmp(ent.type, "weapon_rocketlauncher") == 0)
	{
		debugf("Loading weapon_rocketlauncher\n");
		ent.model->load(gfx, "media/models/weapons2/rocketl/rocketl");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (strcmp(ent.type, "ammo_shells") == 0)
	{
		debugf("Loading ammo_shells\n");
		ent.model->load(gfx, "media/models/powerups/ammo/ammo_shells");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (strcmp(ent.type, "ammo_rockets") == 0)
	{
		debugf("Loading ammo_rockets\n");
		ent.model->load(gfx, "media/models/powerups/ammo/ammo_rockets");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (strcmp(ent.type, "ammo_cells") == 0)
	{
		debugf("Loading ammo_cells\n");
		ent.model->load(gfx, "media/models/powerups/ammo/ammo_lightning");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (strcmp(ent.type, "ammo_bfg") == 0)
	{
		debugf("Loading ammo_bfg\n");
		ent.model->load(gfx, "media/models/powerups/ammo/ammo_lightning");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (strcmp(ent.type, "ammo_grenades") == 0)
	{
		debugf("Loading ammo_bfg\n");
		ent.model->load(gfx, "media/models/powerups/ammo/ammo_grenades");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (strcmp(ent.type, "ammo_lightning") == 0)
	{
		debugf("Loading ammo_lightning\n");
		ent.model->load(gfx, "media/models/powerups/ammo/ammo_lightning");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (strcmp(ent.type, "ammo_slugs") == 0)
	{
		debugf("Loading ammo_slugs\n");
		ent.model->load(gfx, "media/models/powerups/ammo/ammo_slugs");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (strcmp(ent.type, "ammo_bullets") == 0)
	{
		debugf("Loading ammo_bullets\n");
		ent.model->load(gfx, "media/models/powerups/ammo/ammo_bullets");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (strcmp(ent.type, "item_armor_combat") == 0)
	{
		debugf("Loading item_armor_combat\n");
		ent.model->load(gfx, "media/models/powerups/armor/item_armor_combat");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (strcmp(ent.type, "team_CTF_blueflag") == 0)
	{
		debugf("Loading team_CTF_blueflag\n");
		ent.model->load(gfx, "media/models/flags/b_flag");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.position += vec3(0.0f, 50.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (strcmp(ent.type, "team_CTF_redflag") == 0)
	{
		debugf("Loading team_CTF_redflag\n");
		ent.model->load(gfx, "media/models/flags/r_flag");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.position += vec3(0.0f, 50.0f, 0.0);
		ent.rigid->gravity = false;
	}
	
	else if (strcmp(ent.type, "item_armor_body") == 0)
	{
		debugf("Loading item_armor_body\n");
		ent.model->load(gfx, "media/models/powerups/armor/item_armor_body");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (strcmp(ent.type, "item_quad") == 0)
	{
		debugf("Loading item_quad\n");
		ent.model->load(gfx, "media/models/powerups/instant/quad");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (strcmp(ent.type, "item_regen") == 0)
	{
		debugf("Loading item_regen\n");
		ent.model->load(gfx, "media/models/powerups/instant/regen");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (strcmp(ent.type, "item_invis") == 0)
	{
		debugf("Loading item_invis\n");
		ent.model->load(gfx, "media/models/powerups/instant/invis");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (strcmp(ent.type, "item_flight") == 0)
	{
		debugf("Loading item_flight\n");
		ent.model->load(gfx, "media/models/powerups/instant/flight");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (strcmp(ent.type, "item_haste") == 0)
	{
		debugf("Loading item_haste\n");
		ent.model->load(gfx, "media/models/powerups/instant/haste");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (strcmp(ent.type, "holdable_medkit") == 0)
	{
		debugf("Loading holdable_medkit\n");
		ent.model->load(gfx, "media/models/powerups/holdable/medkit");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (strcmp(ent.type, "holdable_teleporter") == 0)
	{
		debugf("Loading holdable_teleporter\n");
		ent.model->load(gfx, "media/models/powerups/holdable/teleporter");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (strcmp(ent.type, "weapon_lightning") == 0)
	{
		debugf("Loading weapon_lightning\n");
		ent.model->load(gfx, "media/models/weapons2/lightning/lightning");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (strcmp(ent.type, "weapon_bfg") == 0)
	{
		debugf("Loading weapon_bfg\n");
		ent.model->load(gfx, "media/models/weapons2/lightning/lightning");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (strcmp(ent.type, "weapon_shotgun") == 0)
	{
		debugf("Loading weapon_shotgun\n");
		ent.model->load(gfx, "media/models/weapons2/shotgun/shotgun");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (strcmp(ent.type, "weapon_railgun") == 0)
	{
		debugf("Loading weapon_railgun\n");
		ent.model->load(gfx, "media/models/weapons2/railgun/railgun");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (strcmp(ent.type, "weapon_grenadelauncher") == 0)
	{
		debugf("Loading weapon_grenadelauncher\n");
		ent.model->load(gfx, "media/models/weapons2/grenadel/grenade");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (strcmp(ent.type, "weapon_plasmagun") == 0)
	{
		debugf("Loading weapon_plasmagun\n");
		ent.model->load(gfx, "media/models/weapons2/plasma/plasma");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (strcmp(ent.type, "item_health") == 0)
	{
		debugf("Loading item_health\n");
		ent.model->load(gfx, "media/models/powerups/health/item_health");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (strcmp(ent.type, "item_health_large") == 0)
	{
		debugf("Loading item_health_large\n");
		ent.model->load(gfx, "media/models/powerups/health/item_health_large");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (strcmp(ent.type, "item_health_small") == 0)
	{
		debugf("Loading item_health_large\n");
		ent.model->load(gfx, "media/models/powerups/health/item_health_small");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (strcmp(ent.type, "item_health_mega") == 0)
	{
		debugf("Loading item_health_mega\n");
		ent.model->load(gfx, "media/models/powerups/health/item_health_mega");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (strcmp(ent.type, "info_player_deathmatch") == 0)
	{
		debugf("Loading info_player_deathmatch\n");
		ent.rigid->load(gfx, "media/models/ball");
		ent.rigid->gravity = false;
		ent.nodraw = true;
	}
	else if (strcmp(ent.type, "info_player_deathmatch") == 0)
	{
		debugf("Loading info_player_deathmatch\n");
		ent.rigid->load(gfx, "media/models/ball");
		ent.rigid->gravity = false;
		ent.nodraw = true;
	}
	else if (strcmp(ent.type, "team_CTF_bluespawn") == 0)
	{
		debugf("Loading team_CTF_bluespawn\n");
		ent.rigid->load(gfx, "media/models/ball");
		ent.rigid->gravity = false;
		ent.nodraw = true;
	}
	else if (strcmp(ent.type, "team_CTF_redspawn") == 0)
	{
		debugf("Loading team_CTF_redspawn\n");
		ent.rigid->load(gfx, "media/models/ball");
		ent.rigid->gravity = false;
		ent.nodraw = true;
	}
	else if (strcmp(ent.type, "team_CTF_blueplayer") == 0)
	{
		debugf("Loading team_CTF_blueplayer\n");
		ent.rigid->load(gfx, "media/models/ball");
		ent.rigid->gravity = false;
		ent.nodraw = true;
	}
	else if (strcmp(ent.type, "team_CTF_redplayer") == 0)
	{
		debugf("Loading team_CTF_redplayer\n");
		ent.rigid->load(gfx, "media/models/ball");
		ent.rigid->gravity = false;
		ent.nodraw = true;
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

int Engine::find_type(char *type, int skip)
{
	for (unsigned int i = 0; i < entity_list.size(); i++)
	{
		if (strcmp(entity_list[i]->type, type) == 0)
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
		if (entity_list[i]->speaker != NULL)
		{
			entity_list[i]->rigid->gravity = false;
			if (entity_list[i]->speaker->index  != -1)
			{
				audio.select_buffer(entity_list[i]->speaker->loop_source, snd_wave[entity_list[i]->speaker->index].buffer);
				audio.play(entity_list[i]->speaker->loop_source);
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

	int spawn = find_type("player", 0);

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

	servermsg_t servermsg = { 0 };

	servermsg.sequence = sequence + 1;
	servermsg.client_sequence = reliable.sequence;
	servermsg.num_ents = 0;

	sprintf(reliable.msg, "disconnect");
	reliable.size = 2 * sizeof(int) + strlen(reliable.msg) + 1;
	reliable.sequence = sequence;
	servermsg.length = SERVER_HEADER + reliable.size;
	memcpy(servermsg.data, &reliable, reliable.size);
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
	free((void *)shader_list[0]);
	free((void *)hash_list[0]);
	free((void *)pk3_list[0]);
	game->destroy();
	delete game;
	debugf("Shutting down.\n");
	delete box;
	delete ball;
	delete thug22;
	delete rocket;
	delete pineapple;

	delete bullet;
	delete shell;

	delete gib0;
	delete gib1;
	delete gib2;
	delete gib3;
	delete gib4;
	delete gib5;
	delete gib6;
	delete gib7;
	delete gib8;
	delete gib9;

	
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

	debugf("Console: %s\n", cmd);

	ret = sscanf(cmd, "log %s", data);
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

	if (strstr(cmd, "list") != 0)
	{
#define LIST_SIZE 1024 * 512
		char *filelist = new char[LIST_SIZE];
		char *line = NULL;

		memset(filelist, 0, LIST_SIZE);
		for (int i = 0; i < num_pk3; i++)
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

	if (sscanf(cmd, "r_patch %s", data) == 1)
	{
		menu.print(msg);
		if (atoi(data))
		{
			q3map.patch_enabled = true;
		}
		else
		{
			q3map.patch_enabled = false;
		}
		return;
	}

	if (sscanf(cmd, "r_maxlight %s", data) == 1)
	{
		int max = 64;

		menu.print(msg);
		max = atoi(data);

		mlight2.set_max(max);
		return;
	}

	if (sscanf(cmd, "r_ambient %s", data) == 1)
	{
		float ambient = 1.0f;

		menu.print(msg);
		ambient = (float)atof(data);

		mlight2.set_ambient(ambient);
		return;
	}

	if (sscanf(cmd, "r_lightmap %s", data) == 1)
	{
		float lightmap = 1.0f;

		menu.print(msg);
		lightmap = (float)atof(data);

		mlight2.set_lightmap(lightmap);
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
			q3map.sky_enabled = true;
		}
		else
		{
			q3map.sky_enabled = false;
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
			q3map.shader_enabled = true;
		}
		else
		{
			q3map.shader_enabled = false;
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
			q3map.blend_enabled = true;
		}
		else
		{
			q3map.blend_enabled = false;
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

	ret = sscanf(cmd, "bind %d", &port);
	if (ret == 1)
	{
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
#ifndef DIRECTX
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
		int player = find_type("player", 0);

		if (player == -1)
			return;

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
	clientmsg_t	clientmsg;
	servermsg_t servermsg;

	client_flag = false;

	memset(&clientmsg, 0, sizeof(clientmsg_t));

	clientmsg.sequence = sequence;
	clientmsg.server_sequence = 0;
	clientmsg.num_cmds = 0;
	clientmsg.qport = qport;
	strcpy(reliable.msg, "connect");
	reliable.size = 2 * sizeof(int) + strlen(reliable.msg) + 1;
	reliable.sequence = sequence;
	memcpy(&clientmsg.data[clientmsg.num_cmds * sizeof(int)], &reliable, reliable.size);
	clientmsg.length = CLIENT_HEADER + clientmsg.num_cmds * sizeof(int) + reliable.size;

	net.connect(serverip, 65535);
	debugf("Sending map request\n");
	net.send((char *)&clientmsg, clientmsg.length);
	debugf("Waiting for server info\n");

	if ( net.recv((char *)&servermsg, 8192, 5) )
	{
		char level[LINE_SIZE];

		client_flag = true;
		debugf("Connected\n");
		reliablemsg_t *reliablemsg = (reliablemsg_t *)&servermsg.data[servermsg.num_ents * sizeof(entity_t)];
		if (sscanf(reliablemsg->msg, "map %s", level) == 1)
		{
			debugf("Loading %s\n", level);
			load((char *)level);
			strcpy(reliable.msg, "spawn");
			reliable.size = 2 * sizeof(int) + strlen(reliable.msg) + 1;
			reliable.sequence = sequence;
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
		int index = find_type("player", 0);

		//chatmode chat
		sprintf(data, "chat %s: %s", entity_list[index]->player->name, msg);
	}
	else
	{
		//console chat command
		// skip past 'say "'
		char *pmsg = msg + 5;
		// remove ending "
		pmsg[strlen(pmsg) - 1] = '\0';
		sprintf(data, "chat %s: %s", name, pmsg);
	}


	strcat(reliable.msg, data);
	reliable.size = 2 * sizeof(int) + strlen(reliable.msg) + 1;
	reliable.sequence = sequence;

	// Client will get message back from server
	if (client_flag == false)
	{
		menu.print_chat(data + 5);
		game->chat_timer = 3 * TICK_RATE;

		int self = find_type("player", 0);
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

int Engine::get_global_source()
{
	int source;
	static int i = 0;

	source = global_source[i++];

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

bool Engine::play_wave(vec3 &position, int index)
{
	if (index < 0)
		return false;

	int source = get_source();

	audio.source_position(source, &position.x);

	audio.select_buffer(source, snd_wave[index].buffer);
	audio.play(source);
	return true;
}

bool Engine::play_wave_global(int index)
{
	if (index < 0)
		return false;

	int source = get_global_source();
	audio.select_buffer(source, snd_wave[index].buffer);
	audio.play(source);
	return true;
}


void Engine::hitscan(vec3 &origin, vec3 &dir, int *index_list, int &num_index, int self)
{
	int j = 0;
	num_index = 0;

	for (unsigned int i = 0; i < entity_list.size(); i++)
	{
		if (i == (unsigned int)self)
			continue;

		if (entity_list[i]->player && entity_list[i]->rigid)
		{
			float distance = FLT_MAX;
			vec3 min = entity_list[i]->rigid->aabb[0] + entity_list[i]->position - entity_list[i]->rigid->center;
			vec3 max = entity_list[i]->rigid->aabb[7] + entity_list[i]->position - entity_list[i]->rigid->center;

			if (RayBoxSlab(origin, dir, min, max, distance))
			{
				vec3 endpoint = entity_list[i]->position;

				// Hit someone, do BSP trace to check if a wall is in the way
	                        q3map.trace(origin, endpoint);
	                        if (q3map.collision)
	                        {
					vec3 dist1 = endpoint - origin;
					vec3 dist2 = entity_list[i]->position - origin;

	                                q3map.collision = false;
					if (dist1.magnitude() > dist2.magnitude() )
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
	for (int i = 0; i < num_pk3; i++)
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
