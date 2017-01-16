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
	num_dynamic = 100;
	num_player = 8;
	show_names = false;
	show_lines = false;
	show_debug = false;
	show_hud = true;
	num_bot = 0;

	fov = 45.0f;
	zNear = 1.0f;
	zFar = 2001.0f; // zFar - zNear makes nice values
	inf = true; // above ignored if true
}


void Engine::init(void *p1, void *p2)
{
	float ident[16] = {	1.0f, 0.0f, 0.0f, 0.0f,
						0.0f, 1.0f, 0.0f, 0.0f,
						0.0f, 0.0f, 1.0f, 0.0f,
						0.0f, 0.0f, 0.0f, 1.0f};

	Engine::param1 = p1;
	Engine::param2 = p2;
	initialized = true;


	debugf("altEngine2 built %s\n", __DATE__);
	bind_keys();

	identity = ident;
	projection = ident;

	gen_spiral(gfx, spiral_ibo, spiral_vbo);
	gen_lightning(gfx, lightning_ibo, lightning_vbo);

	//visual
	gfx.init(param1, param2);
	gfx.CreateVertexArrayObject(global_vao);
	gfx.SelectVertexArrayObject(global_vao);

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

	rocket = new Entity();
	rocket->rigid = new RigidBody(rocket);
	rocket->model = rocket->rigid;
	rocket->rigid->load(gfx, "media/models/weapons2/rocketl/rocket");

	pineapple = new Entity();
	pineapple->rigid = new RigidBody(pineapple);
	pineapple->model = pineapple->rigid;
	pineapple->rigid->load(gfx, "media/models/weapons2/grenadel/pineapple");

	global.init(&gfx);

	q3.init(this);


	//audio
	audio.init();
	menu.init(&gfx, &audio, pk3_list, num_pk3);
	menu.load("media/newmenu.txt", "media/newstate.txt");

	//net crap
	sequence = 0;
	server_flag = false;
	client_flag = false;
	memset(reliable.msg, 0, LINE_SIZE);
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
	newlinelist("media/shaderlist.txt", shader_list, num_shader);
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

	for (unsigned int i = 0; i < surface_list.size(); i++)
	{
		if (strcmp(surface_list[i]->name, "textures/base_floor/pjgrate1") == 0)
		{
			surface_list[i]->num_stage = 1;
		}
		else if (strcmp(surface_list[i]->name, "textures/base_floor/pjgrate2") == 0)
		{
			surface_list[i]->num_stage = 1;
		}
	}

	printf("Done\n");

#ifndef DIRECTX
	//render menu again for linux
	gfx.resize(xres,yres);
	menu.render(global);
	gfx.swap();
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

	if (q3map.loaded)
		return;
	
	q3.load();

	q3map.anim_list.clear();

	menu.delta("load", *this);
	gfx.clear();
	menu.render(global);
	gfx.swap();
	camera_frame.reset();
	light_frame.reset();



	//First n Entities for dynamic items (Dont allocate any at runtime)
	for (unsigned int i = 0; i < num_dynamic; i++)
	{
		Entity *entity = new Entity();
		memcpy(entity->type, "free", strlen("free") + 1);
		entity_list.push_back(entity);

		if (i < num_player && server_flag)
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
	emitter.gravity = vec3(0.0f, -9.8f, 0.0f);
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
		parse_entity(entdata, entity_list, gfx, audio);
		free((void *)entdata);
	}
	else
	{
		char filename[80];
		const char *data = q3map.get_entities();

		sprintf(filename, "media/%s.ent", q3map.map_name);
		write_file(filename, data, strlen(data));
		parse_entity(q3map.get_entities(), entity_list, gfx, audio);
	}



	int start = entity_list.size();

	char navfile[80] = { 0 };
	sprintf(navfile, "media/%s.nav", q3map.map_name);
	char *navdata = get_file(navfile, NULL);
	if (navdata != NULL)
	{
		parse_entity(navdata, entity_list, gfx, audio);
		free((void *)navdata);
	}

	int num_node = entity_list.size() - start;

	navdata_to_graph(ref, node, entity_list, start);
	print_graph(node, num_node);

	graph.load(node, num_node);
	delete [] ref;


	q3.setup_func(entity_list, q3map);

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
//	gfx.Blend(true);
//	render_shadow_volumes(0); // for debugging
	render_scene(true);
//	gfx.Blend(false);
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
	//gfx.cleardepth();
	q3.render_hud(last_frametime);
	if (menu.ingame)
		menu.render(global);
	if (menu.console)
		menu.render_console(global);
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
		if (entity_list[i]->light && light_list[entity_list[find_player()]->player->current_light] == entity_list[i]->light)
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
				mlight2.Params(mvp, light_list, light_list.size(), offset);
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


	if (find_player() != -1)
		entity_list[find_player()]->rigid->frame2ent(&camera_frame, input);



	camera_frame.set(transformation);

	mlight2.Select();
	mvp = transformation * projection;


	if (lights)
		mlight2.Params(mvp, light_list, light_list.size(), offset);
	else
		mlight2.Params(mvp, light_list, 0, offset);

	q3map.render(camera_frame.pos, mvp, gfx, surface_list, mlight2, tick_num);
	q3map.render_sky(gfx, mlight2, tick_num, surface_list);

	render_entities(transformation, true);


#ifdef PARTICLES
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
#endif


	camera_frame.set(transformation);
	mvp = transformation * projection;

	vec3 quad1 = camera_frame.up;
	vec3 quad2 = vec3::crossproduct(camera_frame.up, camera_frame.forward);

#ifdef PARTICLES
	if (emitter.visible)
	{
		particle_render.Select();
		particle_render.Params(mvp, quad1, quad2, 0.0f, 0.0f);
		gfx.SelectTexture(0, particle_tex);
		particle_render.render(gfx, 0, vbo, emitter.num);
	}
#endif


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
		if (find_player() != -1)
		{
			entity_list[find_player()]->rigid->frame2ent(&camera_frame, input);
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
	mlight2.Params(mvp, light_list, light_list.size(), offset);


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

void Engine::render_client(int i, const matrix4 &trans, bool lights, bool hack)
{
	matrix4 mvp;

	entity_list[i]->rigid->get_matrix(mvp.m);

	//md5 faces right, need to flip right and forward orientation
	vec4 temp;

	temp.x = mvp.m[0];
	temp.y = mvp.m[1];
	temp.z = mvp.m[2];
	temp.w = mvp.m[3];

	mvp.m[0] = mvp.m[8];
	mvp.m[1] = mvp.m[9];
	mvp.m[2] = mvp.m[10];
	mvp.m[3] = mvp.m[11];

	mvp.m[8] = temp.x;
	mvp.m[9] = temp.y;
	mvp.m[10] = temp.z;
	mvp.m[11] = temp.w;

	mvp = (mvp * trans) * projection;
	vec3 offset = entity_list[i]->position;


	if (lights)
	{
		mlight2.Params(mvp, light_list, light_list.size(), offset);
	}
	else
	{
		mlight2.Params(mvp, light_list, 0, offset);
	}
	zcc.render(gfx, tick_num >> 1);
}

void Engine::render_entities(const matrix4 &trans, bool lights)
{
	matrix4 mvp;

	mlight2.Select();
	for (unsigned int i = 0; i < entity_list.size(); i++)
	{
		if (entity_list[i]->visible == false)
			continue;

		if (entity_list[i]->nodraw == true)
			continue;

		if (entity_list[i]->rigid == NULL)
			continue;

		vec3 offset = entity_list[i]->position;

		entity_list[i]->rigid->get_matrix(mvp.m);
		mvp = (mvp * trans) * projection;
		if (lights)
		{
			mlight2.Params(mvp, light_list, light_list.size(), offset);
		}
		else
		{
			mlight2.Params(mvp, light_list, 0, offset);
		}

		//		if (entity_list[i]->light == NULL)
		{
			unsigned int j = 0;

			if ((unsigned int)find_player() == i)
			{
				entity_list[i]->rigid->get_matrix(mvp.m);

				//set weapon coordinates
				mvp.m[12] += mvp.m[0] * -5.0f + mvp.m[4] * 50.0f + mvp.m[8] * 5.0f;
				mvp.m[13] += mvp.m[1] * -5.0f + mvp.m[5] * 50.0f + mvp.m[9] * 5.0f;
				mvp.m[14] += mvp.m[2] * -5.0f + mvp.m[6] * 50.0f + mvp.m[10] * 7.0f;

				mvp = (mvp * trans) * projection;
				if (lights)
				{
					mlight2.Params(mvp, light_list, light_list.size(), offset);
				}
				else
				{
					mlight2.Params(mvp, light_list, 0, offset);
				}

				entity_list[i]->player->render_weapon(gfx);
				continue;
			}


			if (i == (unsigned int)server_spawn)
			{
				render_client(i, trans, lights, true);
				continue;
			}

			for (j = 0; j < client_list.size(); j++)
			{
				if (i == client_list[j]->entity)
				{
					render_client(i, trans, lights, false);
					break;
				}
			}

			if (client_list.size() && j < client_list.size() && i == client_list[j]->entity)
				continue;

			if (strcmp(entity_list[i]->type, "NPC") != 0)
			{
				//bool draw_wander_target = false;


				//render rail trail
				if (entity_list[i]->model->rail_trail)
				{
					vec3 quad1 = camera_frame.up;
					vec3 quad2 = vec3::crossproduct(camera_frame.up, camera_frame.forward);

					// Undo model orientation
					quad1 = entity_list[i]->model->morientation.transpose() * quad1;
					quad2 = entity_list[i]->model->morientation.transpose() * quad2;

					particle_render.Select();
					particle_render.Params(mvp, quad1, quad2, 0.0f, 0.0f);
					gfx.SelectTexture(0, particle_tex);
					particle_render.render(gfx, 0, spiral_vbo, 400);

					// restore normal shader state
					mlight2.Select();
					if (lights)
					{
						mlight2.Params(mvp, light_list, light_list.size(), offset);
					}
					else
					{
						mlight2.Params(mvp, light_list, 0, offset);
					}
					continue;
				}


				//render lightning trail
				if (entity_list[i]->model->lightning_trail)
				{
					vec3 quad1 = camera_frame.up;
					vec3 quad2 = vec3::crossproduct(camera_frame.up, camera_frame.forward);

					// Undo model orientation
					quad1 = entity_list[i]->model->morientation.transpose() * quad1;
					quad2 = entity_list[i]->model->morientation.transpose() * quad2;

					particle_render.Select();
					particle_render.Params(mvp, quad1, quad2, 0.0f, 0.0f);
					gfx.SelectTexture(0, particle_tex);
					particle_render.render(gfx, 0, lightning_vbo, 400);


					// restore normal shader state
					mlight2.Select();
					if (lights)
					{
						mlight2.Params(mvp, light_list, light_list.size(), offset);
					}
					else
					{
						mlight2.Params(mvp, light_list, 0, offset);
					}
					continue;
				}

				entity_list[i]->rigid->render(gfx);



				if (entity_list[i]->num_particle)
				{
					emitter.num = entity_list[i]->num_particle;
					emitter.position = entity_list[i]->position;
					emitter.gravity = vec3(0.0f, 30.0f, 0.0f);
				}


#ifdef NOPE
				entity_list[i]->position += entity_list[i]->rigid->sphere_target;
				if (draw_wander_target)
				{
					entity_list[i]->rigid->get_matrix(mvp.m);
					mvp = (mvp * trans) * projection;
					mlight2.Params(mvp, light_list, light_list.size(), offset);
					q3.ball->rigid->render(gfx);
					entity_list[i]->position -= entity_list[i]->rigid->sphere_target;
					entity_list[i]->rigid->get_matrix(mvp.m);
					mvp = (mvp * trans) * projection;
					mlight2.Params(mvp, light_list, light_list.size(), offset);
				}
#endif
			}


			// render func_ items (doors, moving platforms, etc)
			if (entity_list[i]->model_ref != -1)
			{
//				if (strstr(entity_list[i]->type, "func_") != NULL)
				{
					entity_list[i]->rigid->gravity = false;
					q3map.render_model(entity_list[i]->model_ref, gfx);
				}
			}

		}
		//		entity_list[i]->rigid->render_box(gfx); // bounding box lines
	}


	//render md5 as enemy
	for(unsigned int i = 0; i < entity_list.size(); i++)
	{
		if (strcmp(entity_list[i]->type, "NPC") == 0 && entity_list[i]->player->health > 0)
		{
			entity_list[i]->rigid->get_matrix(mvp.m);


			//md5 faces right, need to flip right and forward orientation
			vec4 temp;

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
			vec3 offset = entity_list[i]->position;

			if (lights)
			{
				mlight2.Params(mvp, light_list, light_list.size(), offset);
			}
			else
			{
				mlight2.Params(mvp, light_list, 0, offset);
			}
			zcc.render(gfx, tick_num >> 1);
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

	if (input.weapon_up)
	{
		if (find_player() != -1)
		{
			entity_list[find_player()]->player->change_weapon_up();
		}
		input.weapon_up = false;
	}

	if (input.weapon_down)
	{
		if (find_player() != -1)
		{
			entity_list[find_player()]->player->change_weapon_down();
		}
		input.weapon_down = false;
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
			entity_list[find_player()]->bsp_leaf = leaf_a;
			entity_list[i]->bsp_leaf = leaf_b;

			if ((bsp_visible && entity_list[i]->frustum_visible) || i == (unsigned int)find_player())
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
			entity_list[i]->visible = q3map.vis_test(camera_frame.pos, entity_list[i]->position, leaf_a, leaf_b);
			entity_list[find_player()]->bsp_leaf = leaf_a;
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
					entity_list[i]->light->timer--;
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

// Need to run all the time because of bots freezing when leaving PVS
//		if ( (server_flag == false && entity_list[i]->visible == false) || entity_list[i]->rigid->sleep == true)
//			continue;


		RigidBody *body = entity_list[i]->rigid;

		float delta_time = 2.0f * TICK_MS / 1000.0f;
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
			float force_bouyant = volume * 0.0015f * 9.8f * submerged_percent;

			body->net_force += vec3(0.0f, force_bouyant, 0.0f);
		}



		while (current_time < delta_time)
		{
			body->save_config(config);
			body->integrate(target_time - current_time);
			if ( collision_detect(*body) )
			{
				// Keeping clipped velocity to prevent "sticking"
				vec3 clip = body->velocity;
				body->load_config(config);
//				printf("%3.3f %3.3f %3.3f clip\n", clip.x, clip.y, clip.z);
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
		body.map_collision = true;
		return true;
	}
	body.map_collision = false;

	if (body_collision(body))
		return true;

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
	
	// Do additional mid point testing (front back left right points, mid level)
	mid[0] = body.aabb[0] + vec3(body.aabb[7].x / 2, 0, body.aabb[7].z / 2);
	mid[1] = body.aabb[0] + vec3(0.0f, body.aabb[7].y / 2, body.aabb[7].z / 2);
	mid[2] = body.aabb[0] + vec3(body.aabb[7].x / 2, body.aabb[7].y, body.aabb[7].z / 2);
	mid[3] = body.aabb[0] + vec3(body.aabb[7].x, body.aabb[7].y / 2, body.aabb[7].z / 2);

	if (body.noclip)
		return false;

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
			point = mid[i - 8] - body.center + body.entity->position;
			oldpoint = mid[i - 8] - body.center + body.old_position;
		}

//		can be used to avoid checking all eight points, but checking all 8 works pretty well
//		vec3 point = body.center + body.entity->position;
//		point -= vec3(0.0f, 100.0f, 0.0f); // subtract player height

		if (q3map.collision_detect(point, oldpoint, (plane_t *)&plane, &depth, body.water, body.water_depth, surface_list, body.step_flag && input.numpad1, clip, body.velocity))
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
			body.impulse(plane, point);
			collision = true;
		}
	}
	if (collision)
	{
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
	for(unsigned int i = 0; i < num_player; i++)
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

	// Animate animated textures
	for (unsigned int i = 0; i < q3map.anim_list.size(); i++)
	{
		texture_t  *tex = q3map.anim_list[i];

		if (tex->num_anim == 0)
			break;

		int texunit = tex->anim_unit;
		int ani_index = (tick_num % (TICK_RATE / tex->freq)) % tex->num_anim;

		tex->texObj[texunit] = tex->texObjAnim[ani_index];
	}

	q3.step(tick);

	// These two funcs loop through all entities, should probably combine
	spatial_testing(); // mostly sets visible flag

   // handles triggers and the projectile as trigger stuff
	for (unsigned int i = 0; i < num_player; i++)
	{
		if (strcmp(entity_list[i]->type, "player") == 0)
			check_triggers(i);
		else if (strcmp(entity_list[i]->type, "NPC") == 0)
			check_triggers(i);
	}


	dynamics();
	update_audio();

	//network
	sequence++;
	if (server_flag && sequence)
		server_step();
	else if (client_flag && sequence)
		client_step();

}

void Engine::check_triggers(int self)
{
	num_light = 0;
	for (unsigned int i = 0; i < entity_list.size(); i++)
	{
		bool inside = false;

		if (entity_list[i]->light)
			num_light++;


		if (strstr(entity_list[i]->type, "func_"))
		{
			float period = 2200.0f; // manually setting for q3tourney6 plat
			float sin_wave = (float)fsin(MY_PI * tick_num / period);
			float square_wave = (float)sign((float)fsin(2 * MY_PI * tick_num / period));
			float amount = sin_wave * square_wave;

			if (strstr(entity_list[i]->type, "func_static"))
				continue;

			if (strstr(entity_list[i]->type, "func_door"))
			{
				amount = 25.0f * amount;
			}
			static int count;
			{
				count++;
				amount *= entity_list[i]->height / 800.0f;
		
				switch (entity_list[i]->angle)
				{
				case 0:
				case 360:
					entity_list[i]->position += vec3(amount, 0.0f, 0.0f);
					break;
				case 90:
					entity_list[i]->position += vec3(0.0f, 0.0f, -amount);
					break;
				case 180:
					entity_list[i]->position += vec3(-amount, 0.0f, 0.0f);
					break;
				case 270:
					entity_list[i]->position += vec3(0.0f, 0.0f, amount);
					break;
				case -1://up
					entity_list[i]->position += vec3(0.0f, amount, 0.0f);
					break;
				case -2://down
					entity_list[i]->position += vec3(0.0f, -amount, 0.0f);
					break;
				}
			}
		}


		// Not a trigger
		if (entity_list[i]->trigger == NULL)
			continue;

		// Delete when not moving
		if (entity_list[i]->trigger->idle == true)
		{
			if (entity_list[i]->rigid)
			{

				if (entity_list[i]->rigid->bounce > entity_list[i]->trigger->num_bounce)
				{
					if (entity_list[i]->trigger->explode == false)
					{
						if (entity_list[i]->trigger->explode_timer <= 0)
						{
							clean_entity(i);
							entity_list[i]->~Entity();
							continue;
						}
						else
						{
							entity_list[i]->trigger->explode_timer--;
						}
					}
					else
					{
						// Explode after being idle for idle_timer time (usually zero)
						if (entity_list[i]->trigger->idle_timer <= 0)
						{
							entity_list[i]->trigger->radius = entity_list[i]->trigger->splash_radius;
							sprintf(entity_list[i]->trigger->action, "damage %d", entity_list[i]->trigger->splash_damage);
							if (entity_list[i]->light == NULL)
							{
								entity_list[i]->light = new Light(entity_list[i], gfx, 999);
							}
							entity_list[i]->light->intensity = entity_list[i]->trigger->explode_intensity;
							entity_list[i]->light->color = entity_list[i]->trigger->explode_color;
							entity_list[i]->trigger->explode = false;

							bool ret = select_wave(entity_list[i]->trigger->source, entity_list[i]->trigger->explode_sound);
							if (ret)
							{
								audio.stop(entity_list[i]->trigger->loop_source);
								audio.play(entity_list[i]->trigger->source);
							}
							else
							{
								debugf("Unable to find PCM data for %s\n", entity_list[i]->trigger->explode_sound);
							}
							continue;
						}
						else
						{
							entity_list[i]->trigger->idle_timer--;
						}
					}
				}
			}
		}


		// Only other players can pick up
		if (entity_list[i]->trigger->owner == self && entity_list[i]->rigid->bounce == 0)
			continue;

		if (entity_list[i]->trigger->owner != -1 &&  entity_list[entity_list[i]->trigger->owner]->player->team == entity_list[self]->player->team && q3.gametype != GAMETYPE_DEATHMATCH)
			continue;

		if (strcmp(entity_list[i]->type, "blue_flag") == 0)
		{
			if (entity_list[self]->player->team == TEAM_BLUE)
				continue;
		}

		if (strcmp(entity_list[i]->type, "red_flag") == 0)
		{
			if (entity_list[self]->player->team == TEAM_RED)
				continue;
		}


		float distance = (entity_list[i]->position - entity_list[self]->position).magnitude();

		if ( distance < entity_list[i]->trigger->radius)
			inside = true;

		if (inside == true && entity_list[i]->trigger->active == false)
		{
			int pickup = true;

			if (entity_list[i]->trigger->armor && entity_list[self]->player->armor >= 200)
				pickup = false;

			if (entity_list[i]->trigger->health && entity_list[self]->player->health >= 100)
				pickup = false;

			if (entity_list[self]->player->state == PLAYER_DEAD)
				pickup = false;

			if (entity_list[self]->player->teleport_timer > 0 && strstr(entity_list[i]->type, "teleport"))
				pickup = false;



			if (pickup)
			{
				entity_list[i]->trigger->active = true;
				q3.console(self, entity_list[i]->trigger->action, menu, entity_list);

				if (entity_list[i]->trigger->projectile)
				{
					if (entity_list[self]->player->health <= 0)
					{
						char word[32] = { 0 };
						char weapon[80];
						int owner = entity_list[i]->trigger->owner;

						entity_list[self]->player->stats.deaths++;
						entity_list[owner]->player->stats.kills++;
						entity_list[owner]->player->stats.hits++;

						if (entity_list[owner]->player->current_weapon == wp_rocket)
						{
							sprintf(weapon, "rocket launcher");
						}
						else if (entity_list[owner]->player->current_weapon == wp_grenade)
						{
							sprintf(weapon, "grenade launcher");
						}
						else if (entity_list[owner]->player->current_weapon == wp_plasma)
						{
							sprintf(weapon, "plasma gun");
						}

						if (entity_list[self]->player->health <= -50)
							sprintf(word, "%s", "gibbed");
						else
							sprintf(word, "%s", "killed");

						char msg[80];
						sprintf(msg, "%s %s %s with a %s\n",
							entity_list[owner]->player->name,
							word,
							entity_list[self]->player->name,
							weapon);
						debugf(msg);
						menu.print_notif(msg);
					}
				}

				entity_list[i]->visible = false;
				entity_list[i]->trigger->timeout = entity_list[i]->trigger->timeout_value;

				if (entity_list[i]->trigger->explode_timer)
				{
					vec3 distance = entity_list[self]->position - entity_list[i]->position;
					float mag = MIN(distance.magnitude(), 50.0f);
					//add knockback to explosions
					entity_list[self]->rigid->velocity += (distance.normalize() * entity_list[i]->trigger->knockback) / mag;
				}

				bool ret = false;
				ret = select_wave(entity_list[i]->trigger->source, entity_list[i]->trigger->pickup_sound);
				if (ret)
				{
					audio.play(entity_list[i]->trigger->source);
				}
				else
				{
					debugf("Unable to find PCM data for %s\n", entity_list[i]->trigger->pickup_sound);
				}
			}
		}


		if (entity_list[i]->trigger->timeout > 0)
		{
			entity_list[i]->trigger->timeout -= 0.016f;
		}
		else
		{
			if (entity_list[i]->trigger->active)
			{
				for (unsigned int j = 0; j < snd_wave.size(); j++)
				{
					if (strcmp(snd_wave[j].file, entity_list[i]->trigger->respawn_sound) == 0)
					{
						audio.select_buffer(entity_list[i]->trigger->source, snd_wave[j].buffer);
						break;
					}
				}
				audio.play(entity_list[i]->trigger->source);
			}

			entity_list[i]->trigger->active = false;
			entity_list[i]->trigger->timeout = 0.0f;
		}
	}
}

void Engine::server_step()
{
	servermsg_t	servermsg;
	clientmsg_t clientmsg;
	char socketname[LINE_SIZE];
	bool connected = false;
	int index = -1;


	// send entities to clients
	send_entities();

	// get client packet
	int size = net.recvfrom((char *)&clientmsg, 8192, socketname, LINE_SIZE);
	if ( size <= 0 )
		return;

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
			index = i;
			connected = true;
			break;
		}
		else
		{
			printf("%s != %s\n", socketname, client_list[i]->socketname);
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
			return;
		}

		client_list[index]->server_sequence = clientmsg.server_sequence;
		if (clientmsg.server_sequence > reliable.sequence)
		{
			memset(reliable.msg, 0, LINE_SIZE);
			reliable.sequence = -1;
		}

		if (client_list[index]->entity > entity_list.size())
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

		Entity *client = entity_list[client_list[index]->entity];


		client_frame.set(entity_list[client_list[index]->entity]->rigid->morientation);
		client_frame.pos = client->position;

		float speed_scale = 1.0f;

		if (entity_list[client_list[index]->entity]->player->haste_timer > 0)
			speed_scale = 2.0f;

		client->rigid->move(clientkeys, speed_scale);


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

			if (client_list[index]->client_sequence < reliablemsg->sequence)
			{
				char msg[LINE_SIZE];

				printf("client msg: %s\n", reliablemsg->msg);
				sprintf(msg, "%s: %s\n", client_list[index]->socketname, reliablemsg->msg);
				chat(entity_list[client_list[index]->entity]->player->name, msg);
			}
		}
		client_list[index]->client_sequence = clientmsg.sequence;
	}

	// client not in list, check if it is a connect msg
	reliablemsg_t *reliablemsg = (reliablemsg_t *)&clientmsg.data[clientmsg.num_cmds * sizeof(int)];
	if ( strcmp(reliablemsg->msg, "connect") == 0 )
	{
		debugf("client %s connected\n", socketname);
		servermsg.sequence = sequence;
		servermsg.client_sequence = clientmsg.sequence;
		servermsg.num_ents = 0;
		sprintf(reliable.msg, "map %s", q3map.map_name);
		reliable.sequence = sequence;
		memcpy(&servermsg.data[servermsg.num_ents * sizeof(entity_t)],
			&reliable,
			sizeof(int) + strlen(reliable.msg) + 1);
		servermsg.length = SERVER_HEADER + servermsg.num_ents * sizeof(entity_t) +
			sizeof(int) + strlen(reliable.msg) + 1;
		net.sendto((char *)&servermsg, servermsg.length, socketname);
		debugf("sent client map data\n");
	}
	else if ( strcmp(reliablemsg->msg, "spawn") == 0 )
	{
		bool found = false;
		client_t *client = (client_t *)malloc(sizeof(client_t));

		client->last_time = (unsigned int)time(NULL);
		strcpy(client->socketname, socketname);

		for (unsigned int i = 0; i < client_list.size(); i++)
		{
			if (strcmp(client_list[i]->socketname, client->socketname) == 0)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			client_list.push_back(client);
		}
		debugf("client %s spawned\n", client->socketname);
		client->client_sequence = clientmsg.sequence;

		// assign entity to client
		//set to zero if we run out of info_player_deathmatches
		client->entity = get_player();
		printf("client %s got entity %d\n", socketname, client->entity);
		entity_list[client->entity]->rigid = new RigidBody(entity_list[client->entity]);
		entity_list[client->entity]->model = entity_list[client->entity]->rigid;
		entity_list[client->entity]->rigid->clone(*(box->model));
		entity_list[client->entity]->player = new Player(entity_list[client->entity], gfx, audio, 21);
		entity_list[client->entity]->position += entity_list[client->entity]->rigid->center;
		entity_list[client->entity]->player->respawn();


		servermsg.sequence = sequence;
		servermsg.client_sequence = clientmsg.sequence;
		servermsg.num_ents = 0;
		
		sprintf(reliable.msg, "spawn %d %d", client->entity, find_player());
		reliable.sequence = sequence;
		memcpy(&servermsg.data[servermsg.num_ents * sizeof(entity_t)],
			&reliable,
			sizeof(int) + strlen(reliable.msg) + 1);
		servermsg.length = SERVER_HEADER + servermsg.num_ents * sizeof(entity_t) +
			sizeof(int) + strlen(reliable.msg) + 1;
		net.sendto((char *)&servermsg, servermsg.length, client->socketname);
		debugf("sent client spawn data\n");
	}
}

void Engine::send_entities()
{
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
			delete entity_list[client_list[i]->entity]->rigid;
			entity_list[client_list[i]->entity]->rigid = NULL;
			entity_list[client_list[i]->entity]->model = NULL;
			client_list.erase(client_list.begin() + i);
			i--;
			continue;
		}

		for (unsigned int j = 0; j < entity_list.size(); j++)
		{
			entity_t ent;
			int leaf_a;
			int leaf_b;

			bool visible = q3map.vis_test(entity_list[j]->position,
				entity_list[client_list[i]->entity]->position, leaf_a, leaf_b);
			if ( visible == false )
				continue;

			ent.id = j;
			if (entity_list[j]->rigid)
			{
				ent.morientation = entity_list[j]->rigid->morientation;
				ent.angular_velocity = entity_list[j]->rigid->angular_velocity;
				ent.velocity = entity_list[j]->rigid->velocity;
				ent.position = entity_list[j]->position;
			}

			memcpy(&servermsg.data[servermsg.num_ents * sizeof(entity_t)],
				&ent, sizeof(entity_t));
			servermsg.num_ents++;
		}
		memcpy(&servermsg.data[servermsg.num_ents * sizeof(entity_t)],
			(void *)&reliable,
			sizeof(int) + strlen(reliable.msg) + 1);
		servermsg.length = SERVER_HEADER + servermsg.num_ents * sizeof(entity_t) +
			sizeof(int) + strlen(reliable.msg) + 1;
		servermsg.client_sequence = client_list[i]->client_sequence;
		net.sendto((char *)&servermsg, servermsg.length, client_list[i]->socketname);
	}
}

void Engine::client_step()
{
	servermsg_t	servermsg;
	clientmsg_t clientmsg;
	unsigned int socksize = sizeof(sockaddr_in);
	int keystate = GetKeyState(input);
	static int rate_skip = 0;
	int cl_skip = 8;

	rate_skip++;

	if (rate_skip % cl_skip == 0)
		return;

	//printf("client keystate %d\n", keystate);

	// get entity information
#ifdef WIN32
	int size = ::recvfrom(net.sockfd, (char *)&servermsg, 8192, 0, (sockaddr *)&(net.servaddr), ( int *)&socksize);
#else
	int size = ::recvfrom(net.sockfd, (char *)&servermsg, 8192, 0, (sockaddr *)&(net.servaddr), (unsigned int *)&socksize);
#endif
	if ( size > 0)
	{
		if (size != servermsg.length)
		{
			printf("Packet size mismatch: %d %d\n", size, servermsg.length);
			return;
		}

		if (servermsg.sequence <= last_server_sequence)
		{
			printf("Got old server packet\n");
			return;
		}

//		printf("Recieved %d ents from server\n", servermsg.num_ents);
		if (servermsg.client_sequence > reliable.sequence)
		{
			memset(reliable.msg, 0, LINE_SIZE);
			reliable.sequence = -1;
		}

		for(int i = 0; i < servermsg.num_ents; i++)
		{
			entity_t	*ent = (entity_t *)servermsg.data;

			// dont let bad data cause an exception
			if (ent[i].id >= entity_list.size())
			{
				printf("Invalid entity index, bad packet\n");
				break;
			}

			// need better way to identify entities
			entity_list[ent[i].id]->position = ent[i].position;
			entity_list[ent[i].id]->rigid->velocity = ent[i].velocity;
			entity_list[ent[i].id]->rigid->angular_velocity = ent[i].angular_velocity;
			entity_list[ent[i].id]->rigid->morientation = ent[i].morientation;
		}

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

		if ( (unsigned int)servermsg.length > SERVER_HEADER + servermsg.num_ents * sizeof(entity_t) + sizeof(int) + 1)
		{
			reliablemsg_t *reliablemsg = (reliablemsg_t *)&servermsg.data[servermsg.num_ents * sizeof(entity_t)];

			if (last_server_sequence < reliablemsg->sequence)
			{
				int entity;

				debugf("server msg: %s\n", reliablemsg->msg);
				menu.print(reliablemsg->msg);

				int ret = sscanf(reliablemsg->msg, "spawn %d %d", &entity, &server_spawn);
				if ( ret )
				{
					int client = get_player();

					sprintf(entity_list[client]->type, "client");
					entity_list[client]->position = entity_list[entity]->position;
					entity_list[client]->rigid = new RigidBody(entity_list[client]);
					entity_list[client]->model = entity_list[client]->rigid;
					entity_list[client]->rigid->clone(*(thug22->model));
					entity_list[client]->rigid->step_flag = true;
					entity_list[client]->position += entity_list[client]->rigid->center;
					entity_list[client]->player = new Player(entity_list[client], gfx, audio, 21);
//					entity_list[spawn]->player->respawn();
				}

				ret = strcmp(reliablemsg->msg, "disconnect");
				if (ret == 0)
				{
					unload();
				}
			}
		}
		last_server_sequence = servermsg.sequence;
	}

	// send keyboard state
	memset(&clientmsg, 0, sizeof(clientmsg_t));
	clientmsg.sequence = sequence;
	clientmsg.server_sequence = last_server_sequence;
	clientmsg.up[0] = camera_frame.up.x;
	clientmsg.up[1] = camera_frame.up.y;
	clientmsg.up[2] = camera_frame.up.z;
	clientmsg.forward[0] = camera_frame.forward.x;
	clientmsg.forward[1] = camera_frame.forward.y;
	clientmsg.forward[2] = camera_frame.forward.z;
	clientmsg.num_cmds = 1;
	memcpy(clientmsg.data, &keystate, sizeof(int));
	memcpy(&clientmsg.data[clientmsg.num_cmds * sizeof(int)],
		(void *)&reliable,
		sizeof(int) + strlen(reliable.msg) + 1);
	clientmsg.length = CLIENT_HEADER + clientmsg.num_cmds * sizeof(int)
		+ sizeof(int) + strlen(reliable.msg) + 1;
	::sendto(net.sockfd, (char *)&clientmsg, clientmsg.length, 0, (sockaddr *)&(net.servaddr), socksize);
}

// packs keyboard input into an integer
int Engine::GetKeyState(input_t &input)
{
	int keystate = 0;

	if (input.attack)
		keystate |= 1;
	if (input.use)
		keystate |= 2;
	if (input.zoom)
		keystate |= 4;
	if (input.jump)
		keystate |= 8;
	if (input.moveup)
		keystate |= 16;
	if (input.moveleft)
		keystate |= 32;
	if (input.movedown)
		keystate |= 64;
	if (input.moveright)
		keystate |= 128;

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
		input.zoom = true;
	else
		input.zoom = false;

	if (keystate & 8)
		input.jump = true;
	else
		input.jump = false;

	if (keystate & 16)
		input.moveup = true;
	else
		input.moveup = false;

	if (keystate & 32)
		input.moveleft = true;
	else
		input.moveleft = false;

	if (keystate & 64)
		input.movedown = true;
	else
		input.movedown = false;

	if (keystate & 128)
		input.moveright = true;
	else
		input.moveright = false;

	return input;
}

bool Engine::mousepos(int x, int y, int deltax, int deltay)
{
	static bool once = false;

	if (q3map.loaded == false || menu.ingame == true || menu.console == true)
	{
		float devicex = (float)x / gfx.width;
		float devicey = (float)y / gfx.height;

		if (menu.console == true)
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

	if (input.control == false)
		camera_frame.update(vec2((float)deltax, (float)deltay));
	else
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
	}
	else if (strcmp("moveleft", cmd) == 0)
	{
		input.moveleft = pressed;
	}
	else if (strcmp("movedown", cmd) == 0)
	{
		input.movedown = pressed;
	}
	else if (strcmp("moveright", cmd) == 0)
	{
		input.moveright = pressed;
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
		input.weapon_up = pressed;
	}
	else if (strcmp("weapon_down", cmd) == 0)
	{
		input.weapon_down = pressed;
	}
	else if (strcmp("use", cmd) == 0)
	{
		input.use = pressed;
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
		else
			handle_game(key);
	}
}

void Engine::handle_game(char key)
{
	int spawn = find_player();

	switch (key)
	{
	case '~':
	case '`':
		menu.console = !menu.console;
		break;
	case 'r':
		camera_frame.reset();
		break;

	case '0':
		if (spawn != -1)
		{
			entity_list[find_player()]->player->current_face = 0;
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
		menu.ingame = !menu.ingame;
		break;
	case '[':
		if (spawn != -1)
		{
			entity_list[spawn]->player->change_weapon_down();
		}
		break;
	case ']':
		if (spawn != -1)
		{
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

void Engine::load_sounds()
{
	wave_t wave[8] = { {0} };

	Player::load_sounds(audio, snd_wave);
	
	for(unsigned int i = 0; i < entity_list.size(); i++)
	{
		unsigned int	num_wave = 0;
		bool			add = true;

		if (entity_list[i]->speaker)
		{
				strcpy(wave[0].file, entity_list[i]->speaker->file);
				num_wave++;
		}
		else if (entity_list[i]->trigger)
		{
			if (entity_list[i]->trigger->pickup_sound[0] != '\0')
			{
				strcpy(wave[num_wave].file, entity_list[i]->trigger->pickup_sound);
				num_wave++;
			}

			if (entity_list[i]->trigger->respawn_sound[0] != '\0')
			{
				strcpy(wave[num_wave].file, entity_list[i]->trigger->respawn_sound);
				num_wave++;
			}
		}

		for(unsigned int k = 0; k < num_wave; k++)
		{
			for(unsigned int j = 0; j < snd_wave.size(); j++)
			{
				char *file = snd_wave[j].file;
				if (strcmp(wave[k].file, file) == 0)
				{
					add = false;
					break;
				}
			}

			if (add == false)
				continue;

			debugf("Loading wave file %s\n", wave[k].file);
			audio.load(wave[k]);
			if (wave[k].data == NULL)
				continue;

			snd_wave.push_back(wave[k]);
		}


	}
}

// To prevent making a class that looks exactly like model...
// I will search previous entities for models that are already loaded
void Engine::load_models()
{
	if (entity_list.size() == 0)
		return;


	for(unsigned int i = num_dynamic; i < entity_list.size(); i++)
	{
		bool loaded = false;

		if (entity_list[i]->model == NULL)
			continue;

		for(unsigned int j = num_dynamic; j < i; j++)
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
		ent.model->load(gfx, "media/models/powerups/ammo/ammo");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (strcmp(ent.type, "ammo_rockets") == 0)
	{
		debugf("Loading ammo_rockets\n");
		ent.model->load(gfx, "media/models/powerups/ammo/ammo");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (strcmp(ent.type, "ammo_lightning") == 0)
	{
		debugf("Loading ammo_rockets\n");
		ent.model->load(gfx, "media/models/powerups/ammo/ammo");
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
	else if (strcmp(ent.type, "item_armor_body") == 0)
	{
		debugf("Loading item_armor_body\n");
		ent.model->load(gfx, "media/models/powerups/armor/item_armor_combat");
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
		ent.model->load(gfx, "media/models/powerups/health/health");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (strcmp(ent.type, "item_health_large") == 0)
	{
		debugf("Loading item_health_large\n");
		ent.model->load(gfx, "media/models/powerups/health/health");
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

}

// Loads media that may be shared with multiple entities
void Engine::load_entities()
{
	q3.init_camera(entity_list);
	load_sounds();
	create_sources();
	load_models();

	for(unsigned int i = 0; i < entity_list.size(); i++)
	{
		if (entity_list[i]->light)
			entity_list[i]->rigid->gravity = false;
	}

	int spawn = find_player();
	entity_list[spawn]->rigid->clone(*(thug22->model));
	entity_list[spawn]->position += entity_list[spawn]->rigid->center;
}

void Engine::clean_entity(int index)
{
	//free audio sources
	if (entity_list[index]->trigger)
		entity_list[index]->trigger->destroy(audio);

	if (entity_list[index]->speaker)
		entity_list[index]->speaker->destroy(audio);


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
	static unsigned int index = 0;
	int looped = 0;

	while (1)
	{

		if (index == num_dynamic)
		{
			index = num_player;
			looped++;

			if (looped == 2)
			{
				debugf("Unable to find free dynamic entity\n");
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

	return num_dynamic - 1;
}

int Engine::find_player()
{
	for (unsigned int i = 0; i < num_player; i++)
	{
		if (strcmp(entity_list[i]->type, "player") == 0)
			return i;
	}
	return -1;
}

int Engine::get_player()
{
	static unsigned int index = 0;
	int looped = 0;

	while (1)
	{

		if (index == num_player)
		{
			index = 0;
			looped++;

			if (looped == 2)
			{
				debugf("Unable to find free dynamic entity\n");
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

	return num_player - 1;
}

void Engine::create_sources()
{
	// create and associate sources
	for(unsigned int i = 0; i < entity_list.size(); i++)
	{
		if (entity_list[i]->speaker != NULL)
		{
			entity_list[i]->rigid->gravity = false;
			for(unsigned int j = 0; j < snd_wave.size(); j++)
			{
				if (strcmp(snd_wave[j].file, entity_list[i]->speaker->file) == 0)
				{
					audio.select_buffer(entity_list[i]->speaker->loop_source, snd_wave[j].buffer);
					audio.play(entity_list[i]->speaker->loop_source);
					break;
				}
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

	for(unsigned int i = 0; i < entity_list.size(); i++)
	{
		if (entity_list[i]->speaker)
		{
			audio.source_position(entity_list[i]->speaker->source, (float *)(&entity_list[i]->position));
			audio.source_velocity(entity_list[i]->speaker->source, (float *)(&entity_list[i]->rigid->velocity));
			audio.source_position(entity_list[i]->speaker->loop_source, (float *)(&entity_list[i]->position));
			audio.source_velocity(entity_list[i]->speaker->loop_source, (float *)(&entity_list[i]->rigid->velocity));
		}

		if (entity_list[i]->trigger)
		{
			audio.source_position(entity_list[i]->trigger->source, (float *)(&entity_list[i]->position));
			audio.source_velocity(entity_list[i]->trigger->source, (float *)(&entity_list[i]->rigid->velocity));
			audio.source_position(entity_list[i]->trigger->loop_source, (float *)(&entity_list[i]->position));
			audio.source_velocity(entity_list[i]->trigger->loop_source, (float *)(&entity_list[i]->rigid->velocity));
		}
	}

	int spawn = find_player();

	if (spawn != -1 && entity_list.size())
	{
		audio.listener_velocity((float *)&(entity_list[spawn]->rigid->velocity));
		audio.listener_orientation((float *)&(entity_list[spawn]->rigid->morientation.m));
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
	reliable.sequence = sequence + 1;
	servermsg.length = SERVER_HEADER +
		sizeof(int) + strlen(reliable.msg) + 1;
	memcpy(servermsg.data, &reliable, sizeof(int) + strlen(reliable.msg) + 1);
	net.sendto((char *)&servermsg, servermsg.length, client_list[i]->socketname);
	debugf("sent disconnect to client %d [%s]\n", i, client_list[i]->socketname);
	free((void *)client_list[i]);
	client_list.erase(client_list.begin() + i);
}

void Engine::unload()
{
	if (q3map.loaded == false)
		return;

	q3.unload();

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
	q3.destroy();
	debugf("Shutting down.\n");
	delete box;
	delete ball;
	delete thug22;
	delete rocket;
	delete pineapple;

	
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
		load("maps/q3tourney2.bsp");
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
		if (atoi(data) == 1)
			glFrontFace(GL_CCW);
		else
			glFrontFace(GL_CW);
	}

	if (q3map.loaded)
	{
		q3.console(find_player(), cmd, menu, entity_list);
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
	strcpy(reliable.msg, "connect");
	reliable.sequence = sequence;
	memcpy(&clientmsg.data[clientmsg.num_cmds * sizeof(int)],
		&reliable,
		sizeof(int) + strlen(reliable.msg) + 1);
	clientmsg.length = CLIENT_HEADER + clientmsg.num_cmds * sizeof(int)
		+ sizeof(int) + strlen(reliable.msg) + 1;

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
	char data[512];

	strcat(reliable.msg, msg);
	reliable.sequence = sequence;

	// skip past 'say "'
	char *pmsg = msg + 5;
	// remove ending "
	pmsg[strlen(pmsg) - 1] = '\0';
	sprintf(data, "%s: %s", name, pmsg);
	menu.print_chat(data);
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


// We are only checking for hitting of entities
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
				index_list[j++] = i;
				num_index++;
			}
		}
	}

}
