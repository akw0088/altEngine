#include "include.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//#define SHADOWVOL
#define FORWARD
//#define DEFERRED

bool aabb_visible(vec3 &min, vec3 &max, matrix4 &mvp);


Engine::Engine()
{
	initialized = false;
	num_dynamic = 100;
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

	debugf("altEngine2 Version %s\n", "1.1.0");

	identity = ident;
	projection = ident;

	//visual
	gfx.init(param1, param2);
	gfx.CreateVertexArrayObject(global_vao);
	gfx.SelectVertexArrayObject(global_vao);

	no_tex = load_texture(gfx, "media/notexture.tga");
	Model::CreateObjects(gfx);
	global.init(&gfx);


	//audio
	audio.init();
	menu.init(&gfx, &audio);
	menu.load("media/newmenu.txt", "media/newstate.txt");

	//net crap
	sequence = 0;
	server_flag = false;
	client_flag = false;
	memset(reliable.msg, 0, LINE_SIZE);
	reliable.sequence = -1;
	last_server_sequence = 0;
	spawn = -1;
	testObj = 0;

	load_md5();



	fb_width = 1280;
	fb_height = 1280;
	gfx.setupFramebuffer(fb_width, fb_height, fbo, quad_tex, depth_tex);


	shadowmap.init(&gfx);
}

void Engine::load(char *level)
{
	matrix4 transformation;

	if (map.loaded)
		return;

	menu.delta("load", *this);
	gfx.clear();
	menu.render(global);
	gfx.swap();
	camera_frame.reset();
	light_frame.reset();



	//First n Entities for dynamic items (Dont allocate any at runtime)
	for (int i = 0; i < num_dynamic; i++)
	{
		Entity *entity = new Entity();
		memcpy(entity->type, "free", strlen("free") + 1);
		entity_list.push_back(entity);
	}

	if (post.init(&gfx))
		menu.print("Failed to load post shader");
	if (mlight2.init(&gfx))
		menu.print("Failed to load mlight2 shader");
	if (mlight3.init(&gfx))
		menu.print("Failed to load mlight3 shader");


	if ( map.load(level) == false)
		return;

	map.generate_meshes(gfx);

	parse_entity(map.get_entities(), entity_list, gfx);

	menu.delta("entities", *this);
	gfx.clear();
	menu.render(global);
	gfx.swap();
	load_entities();

	// This renders map before loading textures
	camera_frame.set(transformation);
	matrix4 mvp = transformation * projection;
//	entity_list[spawn]->rigid->frame2ent(&camera_frame, keyboard);
	spatial_testing();
	gfx.clear();
	global.Select();
	global.Params(mvp, 0);
	gfx.SelectTexture(0, no_tex);

	map.render(camera_frame.pos, mvp, gfx);
	camera_frame.set(transformation);
	render_entities(transformation, true);

	menu.delta("textures", *this);
	menu.render(global);
	gfx.swap();
	map.load_textures(gfx);
	menu.delta("loaded", *this);
	menu.stop();
	menu.ingame = false;
	menu.console = false;

	/*
	for(unsigned int i = 0; i < entity_list.size(); i++)
	{
		vec3 color(1.0f, 1.0f, 1.0f);

		camera_frame.set(transformation);
		mvp = transformation.premultiply(entity_list[i]->rigid->get_matrix(mvp.m)) * projection;
		vec3 pos = mvp * vec4(entity_list[i]->position.x, entity_list[i]->position.y, entity_list[i]->position.z, 0.0f);

		menu.draw_text(entity_list[i]->type, pos.x, pos.y, pos.z, 1000.0f, color);
	}
	*/

	//Setup render to texture
	gfx.bindFramebuffer(fbo);
#ifdef DEFERRED
	gfx.resize(fb_width, fb_height);
#endif

	// Generate depth cubemaps for each light
	render_shadowmaps();
	gfx.bindFramebuffer(0);


#ifdef SHADOWVOL
	for(int i = 0; i < entity_list.size(); i++)
	{
		if (entity_list[i]->light)
		{
			entity_list[i]->light->generate_volumes(map);
//			entity_list[i]->rigid->angular_velocity = vec3();
		}
	}

#endif

}

void Engine::load_md5()
{
	char **animation = NULL;

	frame_step = 0;
	animation = new char *[50];
	animation[0] = "media/md5/chaingun_walk.md5anim";
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
	if (map.loaded == false)
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
	gfx.Blend(true);
//	render_shadow_volumes(0); // for debugging
	render_scene(true);
	gfx.Blend(false);
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
	gfx.cleardepth();
	debug_messages(last_frametime);
	if (menu.ingame)
		menu.render(global);
	if (menu.console)
		menu.render_console(global);
	gfx.swap();
}

void Engine::render_shadowmaps()
{
	for (unsigned int i = 0; i < entity_list.size(); i++)
	{
		if (entity_list[i]->light && light_list[entity_list[spawn]->player->current_light] == entity_list[i]->light)
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
				mlight2.Params(mvp, light_list, light_list.size());
				map.render(entity_list[i]->position, mvp, gfx);
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
	render_shadow_volumes(entity_list[spawn]->player->current_light);


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
//	gfx.SelectShader(0);
//	gfx.DeselectTexture(0);
}

void Engine::render_scene(bool lights)
{
	matrix4 transformation;
	matrix4 mvp;

	if (spawn != -1)
		entity_list[spawn]->rigid->frame2ent(&camera_frame, input);


	camera_frame.set(transformation);
	render_entities(transformation, true);
	mlight2.Select();
	mvp = transformation * projection;

	if (lights)
		mlight2.Params(mvp, light_list, light_list.size());
	else
		mlight2.Params(mvp, light_list, 0);

	map.render(camera_frame.pos, mvp, gfx);
//	gfx.SelectShader(0);
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
		if (spawn != -1)
		{
			entity_list[spawn]->rigid->frame2ent(&camera_frame, input);
		}

		camera_frame.set(transformation);
	}
	else
	{
		light_frame.set(transformation);
	}


	shadowmap.Select();
		
	render_entities(transformation, lights);
	mlight2.Select();
	mvp = transformation * projection;
	mlight2.Params(mvp, light_list, light_list.size());


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
		map.render(light_frame.pos, mvp, gfx);
//		render_shadow_volumes(entity_list[spawn]->player->current_light);
//		gfx.SelectShader(0);
		return;
	}

	map.render(camera_frame.pos, mvp, gfx);
//	gfx.SelectShader(0);
}

void Engine::render_client(int i, const matrix4 &trans, bool lights, bool hack)
{
	matrix4 mvp;

	entity_list[i]->rigid->get_matrix(mvp.m);

	if (hack)
	{
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
		mvp.m[11] = temp.w;
	}

	 mvp = trans.premultiply(mvp.m) * projection;


	if (lights)
	{
		mlight2.Params(mvp, light_list, light_list.size());
	}
	else
	{
		mlight2.Params(mvp, light_list, 0);
	}
	zcc.render(gfx, frame_step);
}

void Engine::render_entities(const matrix4 &trans, bool lights)
{
	matrix4 mvp;

	mlight2.Select();
	for(unsigned int i = 0; i < entity_list.size(); i++)
	{
		if (entity_list[i]->visible == false)
			continue;

		if (entity_list[i]->rigid == NULL)
		{
			entity_list[i]->decal->render(gfx, global);
			continue;
		}

		mvp = trans.premultiply(entity_list[i]->rigid->get_matrix(mvp.m)) * projection;
		if (lights)
		{
			mlight2.Params(mvp, light_list, light_list.size());
		}
		else
		{
			mlight2.Params(mvp, light_list, 0);
		}

//		if (entity_list[i]->light == NULL)
		{
			unsigned int j = 0;

			if (spawn == i)
			{
				entity_list[i]->rigid->get_matrix(mvp.m);

				//set weapon coordinates
				mvp.m[12] += mvp.m[0] * -5.0f + mvp.m[4] * 50.0f + mvp.m[8] * 5.0f;
				mvp.m[13] += mvp.m[1] * -5.0f + mvp.m[5] * 50.0f + mvp.m[9] * 5.0f;
				mvp.m[14] += mvp.m[2] * -5.0f + mvp.m[6] * 50.0f + mvp.m[10] * 7.0f;
				mvp = trans.premultiply(mvp.m) * projection;
				if (lights)
				{
					mlight2.Params(mvp, light_list, light_list.size());
				}
				else
				{
					mlight2.Params(mvp, light_list, 0);
				}

				entity_list[i]->player->render_weapon(gfx);
				continue;
			}


			if (i == server_spawn)
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

			entity_list[i]->rigid->render(gfx);
		}
//		entity_list[i]->rigid->render_box(gfx); // bounding box lines
		
	}


	//render md5 as second to last entity
	
	if (entity_list.size())
	{
		if (entity_list[entity_list.size() - 1]->rigid == NULL)
			return;

		mvp = trans.premultiply(entity_list[entity_list.size() - 1]->rigid->get_matrix(mvp.m)) * projection;

		if (lights)
		{
			mlight2.Params(mvp, light_list, light_list.size());
		}
		else
		{
			mlight2.Params(mvp, light_list, 0);
		}
		zcc.render(gfx, frame_step);
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

			glMatrixMode(GL_PROJECTION_MATRIX);
			glLoadMatrixf(projection.m);
			glMatrixMode(GL_MODELVIEW_MATRIX);
			glLoadMatrixf(transformation.m);


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

void Engine::debug_messages(double last_frametime)
{
	char msg[LINE_SIZE];
	projection = identity;
	vec3 color(1.0f, 1.0f, 1.0f);

	snprintf(msg, LINE_SIZE, "Debug Messages: lastframe %.2f ms %.2f fps", last_frametime, 1000.0 / last_frametime);
	menu.draw_text(msg, 0.01f, 0.025f, 0.025f, color);
	snprintf(msg, LINE_SIZE, "%d active lights.", (int)light_list.size());
	menu.draw_text(msg, 0.01f, 0.05f, 0.025f, color);
	if (spawn != -1)
	{
		snprintf(msg, LINE_SIZE, "Bullets: %d", entity_list[spawn]->player->ammo_bullets);
		menu.draw_text(msg, 0.01f, 0.075f, 0.025f, color);
		snprintf(msg, LINE_SIZE, "Shells: %d", entity_list[spawn]->player->ammo_shells);
		menu.draw_text(msg, 0.01f, 0.1f, 0.025f, color);
		snprintf(msg, LINE_SIZE, "Rockets: %d", entity_list[spawn]->player->ammo_rockets);
		menu.draw_text(msg, 0.01f, 0.125f, 0.025f, color);
		snprintf(msg, LINE_SIZE, "Bolts: %d", entity_list[spawn]->player->ammo_lightning);
		menu.draw_text(msg, 0.01f, 0.15f, 0.025f, color);
		snprintf(msg, LINE_SIZE, "position: %3.3f %3.3f %3.3f", entity_list[spawn]->position.x, entity_list[spawn]->position.y, entity_list[spawn]->position.z);
		menu.draw_text(msg, 0.01f, 0.175f, 0.025f, color);
		snprintf(msg, LINE_SIZE, "velocity: %3.3f %3.3f %3.3f", entity_list[spawn]->rigid->velocity.x, entity_list[spawn]->rigid->velocity.y, entity_list[spawn]->rigid->velocity.z);
		menu.draw_text(msg, 0.01f, 0.2f, 0.025f, color);

		snprintf(msg, LINE_SIZE, "%d/%d", entity_list[spawn]->player->health, entity_list[spawn]->player->armor);
		menu.draw_text(msg, 0.15f, 0.95f, 0.050f, color);
	}
	projection.perspective(45.0, (float)gfx.width / gfx.height, 1.0f, 2001.0f, false);
}

void Engine::destroy_buffers()
{
	zcc.destroy_buffers(gfx);
	sentry.destroy_buffers(gfx);
	zsec_shotgun.destroy_buffers(gfx);

	gfx.DeleteFrameBuffer(fbo);

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

	if (input.mousewheelup)
	{
		if (spawn != -1)
		{
			entity_list[spawn]->player->change_weapon_up();
		}
		input.mousewheelup = false;
	}

	if (input.mousewheeldown)
	{
		if (spawn != -1)
		{
			entity_list[spawn]->player->change_weapon_down();
		}
		input.mousewheeldown = false;
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
			bool frustum_visible = false;
			bool visible = false;


			matrix4 transformation;

			camera_frame.set(transformation);
			matrix4 mvp = transformation * projection;
			vec3 min = entity_list[i]->model->morientation * entity_list[i]->model->aabb[0];
			vec3 max = entity_list[i]->model->morientation * entity_list[i]->model->aabb[7];

			frustum_visible = aabb_visible(min, max, mvp);

			for(int j = 0; j < 8; j++)
			{
				vec3 position = entity_list[i]->position + entity_list[i]->model->aabb[j];
				bool vert_visible = map.vis_test(camera_frame.pos, position);

				if (vert_visible)
				{
					bsp_visible = true;
					break;
				}
			}

			if ((bsp_visible && frustum_visible) || i == spawn)
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
			entity_list[i]->visible = map.vis_test(camera_frame.pos, entity_list[i]->position);
		}

		if (entity_list[i]->visible == false)
			continue;

		vec3 dist_vec = entity_list[i]->position - camera_frame.pos;
		float distance = dist_vec.x * dist_vec.x + dist_vec.y * dist_vec.y + dist_vec.z * dist_vec.z;

		if (entity_list[i]->light)
			activate_light(distance, entity_list[i]->light);

	}
}

void Engine::activate_light(float distance, Light *light)
{
	if (distance < 800.0f * 800.0f && light->entity->visible)
	{
		if (light->active == false)
		{
			light_list.push_back(light);
			light->active = true;
//			light->entity->rigid->angular_velocity.x = 10.0;
//			light->entity->rigid->angular_velocity.y = 10.0;
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
	for(int i = 0; i < entity_list.size(); i++)
	{
		if (entity_list[i]->rigid == NULL)
			continue;

		if ( (server_flag == false && entity_list[i]->visible == false) || entity_list[i]->rigid->sleep == true)
			continue;

		RigidBody *body = entity_list[i]->rigid;
		if (entity_list[i]->vehicle != NULL)
			body = entity_list[i]->vehicle;
		float delta_time = 0.016f;
		float target_time = delta_time;
		float current_time = 0.0f;
		int divisions = 0;

		while (current_time < delta_time)
		{
			body->save_config(config);
			body->integrate(target_time - current_time);
			if ( collision_detect(*body) )
			{
				body->load_config(config);

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

	/*
	if (body_collision(body))
		return true;
		*/
	return false;
}

bool Engine::map_collision(RigidBody &body)
{
	Plane plane;
	float depth;
	vec3 staircheck(0.0f, 20.0f, 0.0f);

	// Check bounding box against map
	for(int i = 0; i < 8; i++)
	{
		vec3 point = body.aabb[i] - body.center + body.entity->position;

//		can be used to avoid checking all eight points, but checking all 8 works pretty well
//		vec3 point = body.center + body.entity->position;
//		point -= vec3(0.0f, 100.0f, 0.0f); // subtract player height

		//bsps cant really give us depth of penetration, only hit/no hit
		if (map.collision_detect(point, (plane_t *)&plane, &depth))
		{
			if (depth > -0.25f && depth < 0.0f)
			{
				// Note this will never occur from BSP because we go from no collision to deep penetration instantly
				point = point * (1.0f / UNITS_TO_METERS);
				body.entity->position = body.old_position;
				body.morientation = body.old_orientation;
				body.impulse(plane, point);
			}
			else
			{
				if (body.step_flag)
				{
					float dist = body.velocity.y;

					if (dist < 0)
						dist *= -1;

					if (dist < 0.1f)
					{
						vec3 p = point + staircheck;

						if (map.collision_detect(p, (plane_t *)&plane, &depth) == false)
						{
							body.entity->position += vec3(0.0f, 2.5f, 0.0f);
							return false;
						}
					}
				}
				return true;
			}
		}
	}
	return false;
}

//O(N^2)
bool Engine::body_collision(RigidBody &body)
{
	for(unsigned int i = 0; i < entity_list.size(); i++)
	{
		if (entity_list[i] == body.entity)
			continue;

		if (map.leaf_test(body.entity->position, entity_list[i]->position))
			body.collision_detect(*entity_list[i]->rigid);
	}
	return false;
}

void Engine::step()
{
	frame_step++;

	if (map.loaded == false)
		return;

	if (spawn != -1)
	{
		if (entity_list[spawn]->player->health <= 0 && entity_list[spawn]->player->dead == false)
		{
			debugf("%s died\n", entity_list[spawn]->player->name);
			entity_list[spawn]->player->kill();
		}
		handle_weapons(*(entity_list[spawn]->player));
	}


	// These two funcs loop through all entities, should probably combine
	spatial_testing(); // mostly sets visible flag
	check_triggers();  // handles triggers and the projectile as trigger stuff

	//entity test movement
	if (menu.ingame == false && menu.console == false)
	{
		if (input.control == true)
			light_frame.update(input);
		else if (spawn != -1)
		{
			if (entity_list[spawn]->player->health > 0)
			{
				entity_list[spawn]->rigid->move(camera_frame, input);
			}
		}
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

void Engine::check_triggers()
{
	num_light = 0;
	for (unsigned int i = 0; i < entity_list.size(); i++)
	{
		bool inside = false;

		if (entity_list[i]->light)
			num_light++;

		// Not a trigger
		if (entity_list[i]->trigger == NULL)
			continue;

		// Delete when not moving
		if (entity_list[i]->trigger->idle == true)
		{
			if (entity_list[i]->rigid)
			{
				// Seems to work, but the collision detect flag should work
				if (entity_list[i]->rigid->old_position == entity_list[i]->position)
				{

					if (entity_list[i]->trigger->explode == false)
					{

						if (entity_list[i]->trigger->explode_timer == 0)
						{
							// Light list wont be updated until the next step, so manually delete
							remove_light(i);
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
						entity_list[i]->trigger->radius = 250.0f;
						memcpy(entity_list[i]->trigger->action, "damage 50", strlen("damage 50") + 1);
						if (entity_list[i]->light == NULL)
						{
							entity_list[i]->light = new Light(entity_list[i], gfx, 999);
							entity_list[i]->light->intensity = 1000.0f;
						}
						entity_list[i]->light->color = vec3(1.0f, 0.0f, 0.0f);
						entity_list[i]->trigger->explode = false;
						entity_list[i]->trigger->self = true;
						continue;
					}
				}
			}
		}


		// Only other players can pick up
		if (entity_list[i]->trigger->self == false)
			continue;

		float distance = (entity_list[i]->position - entity_list[spawn]->position).magnitude();

		if ( distance < entity_list[i]->trigger->radius)
			inside = true;

		if (inside == true && entity_list[i]->trigger->active == false)
		{
			entity_list[i]->trigger->active = true;
			console(entity_list[i]->trigger->action);

			entity_list[i]->visible = false;
			entity_list[i]->trigger->timeout = 30.0f;

			if (entity_list[i]->trigger->explode_timer)
			{
				vec3 distance = entity_list[spawn]->position - entity_list[i]->position;
				float mag = MIN(distance.magnitude(), 50.0f);
				//add knockback to explosions
				entity_list[spawn]->rigid->velocity +=  (distance.normalize() * 750.0f) / mag;
			}

			for (unsigned int j = 0; j < snd_wave.size(); j++)
			{
				if (strcmp(snd_wave[j].file, entity_list[i]->trigger->pickup_snd) == 0)
				{
					audio.select_buffer(entity_list[i]->trigger->source, snd_wave[j].buffer);
					break;
				}
			}
			audio.play(entity_list[i]->trigger->source);
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
					if (strcmp(snd_wave[j].file, entity_list[i]->trigger->respawn_snd) == 0)
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
		button_t clientkeys = GetKeyState(keystate);

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

		//hack, models are 90 degrees to the right, so make right forward
		/*
		client_frame.forward.x = right.x;
		client_frame.forward.y = right.y;
		client_frame.forward.z = right.z;
		*/


		Entity *client = entity_list[client_list[index]->entity];


		client_frame.set(entity_list[client_list[index]->entity]->rigid->morientation);
		client_frame.pos = client->position;
		client->rigid->move(client_frame, clientkeys);


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

		if (clientmsg.length > CLIENT_HEADER + sizeof(int) + sizeof(int) + 1)
		{
			reliablemsg_t *reliablemsg = (reliablemsg_t *)&clientmsg.data[4];

			if (client_list[index]->client_sequence < reliablemsg->sequence)
			{
				char msg[LINE_SIZE];

				printf("client msg: %s\n", reliablemsg->msg);
				sprintf(msg, "%s: %s\n", client_list[index]->socketname, reliablemsg->msg);
				menu.print(msg);
				chat(msg);
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
		sprintf(reliable.msg, "map %s", map.map_name);
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
		client->entity = 0;
		int count = 0;
		for (unsigned int i = 0; i < entity_list.size(); i++)
		{
			if (strcmp(entity_list[i]->type, "info_player_deathmatch") == 0)
			{
				if (count == client_list.size())
				{
					printf("client %s got entity %d\n", socketname, i);
					client->entity = i;
					entity_list[client->entity]->rigid = new RigidBody(entity_list[client->entity]);
					entity_list[client->entity]->rigid->load(gfx, "media/models/thug22/thug22");
					entity_list[client->entity]->player = new Player(entity_list[client->entity], gfx, audio);
					entity_list[client->entity]->position += entity_list[client->entity]->rigid->center;
					break;
				}
				count++;
			}
		}

		servermsg.sequence = sequence;
		servermsg.client_sequence = clientmsg.sequence;
		servermsg.num_ents = 0;
		
		sprintf(reliable.msg, "spawn %d %d", client->entity, spawn);
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

			bool visible = map.vis_test(entity_list[j]->position,
				entity_list[client_list[i]->entity]->position);
			if ( visible == false )
				continue;

			ent.id = j;
			ent.morientation = entity_list[j]->rigid->morientation;
			ent.angular_velocity = entity_list[j]->rigid->angular_velocity;
			ent.velocity = entity_list[j]->rigid->velocity;
			ent.position = entity_list[j]->position;

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
	int cl_skip = 4;

	rate_skip++;

	if (rate_skip % cl_skip == 0)
		return;

	//printf("client keystate %d\n", keystate);

	// get entity information
#ifdef WIN32
	int size = ::recvfrom(net.sockfd, (char *)&servermsg, 8192, 0, (sockaddr *)&(net.servaddr), ( int *)&socksize);
#else
#ifdef  MACOS
	int size = ::recvfrom(net.sockfd, (char *)&servermsg, 8192, 0, (sockaddr *)&(net.servaddr), (unsigned int *)&socksize);
#else
	int size = ::recvfrom(net.sockfd, (char *)&servermsg, 8192, 0, (sockaddr *)&(net.servaddr), (unsigned int *)&socksize);
#endif
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

		if ( servermsg.length > SERVER_HEADER + servermsg.num_ents * sizeof(entity_t) + sizeof(int) + 1)
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
					spawn = get_entity();

					memcpy(entity_list[spawn]->type, "client", strlen("client") + 1);
					entity_list[spawn]->position = entity_list[entity]->position;
					entity_list[spawn]->rigid = new RigidBody(entity_list[spawn]);
					entity_list[spawn]->rigid->load(gfx, "media/models/thug22/thug22");
					entity_list[spawn]->rigid->step_flag = true;
					entity_list[spawn]->position += entity_list[spawn]->rigid->center;
					entity_list[spawn]->player = new Player(entity_list[spawn], gfx, audio);
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
int Engine::GetKeyState(button_t &kb)
{
	int keystate = 0;

	if (kb.leftbutton)
		keystate |= 1;
	if (kb.middlebutton)
		keystate |= 2;
	if (kb.rightbutton)
		keystate |= 4;
	if (kb.enter)
		keystate |= 8;
	if (kb.up)
		keystate |= 16;
	if (kb.left)
		keystate |= 32;
	if (kb.down)
		keystate |= 64;
	if (kb.right)
		keystate |= 128;

	return keystate;
}

// unpacks keyboard integer into keyboard state
button_t Engine::GetKeyState(int keystate)
{
	button_t kb;

	memset((void *)&kb, 0, sizeof(button_t));
	if (keystate & 1)
		kb.leftbutton = true;
	else
		kb.leftbutton = false;

	if (keystate & 2)
		kb.middlebutton = true;
	else
		kb.middlebutton = false;

	if (keystate & 4)
		kb.rightbutton = true;
	else
		kb.rightbutton = false;

	if (keystate & 8)
		kb.enter = true;
	else
		kb.enter = false;

	if (keystate & 16)
		kb.up = true;
	else
		kb.up = false;

	if (keystate & 32)
		kb.left = true;
	else
		kb.left = false;

	if (keystate & 64)
		kb.down = true;
	else
		kb.down = false;

	if (keystate & 128)
		kb.right = true;
	else
		kb.right = false;

	return kb;
}

bool Engine::mousepos(int x, int y, int deltax, int deltay)
{
	static bool once = false;

	if (map.loaded == false || menu.ingame == true || menu.console == true)
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
		light_frame.update(vec2((float)deltax, (float)deltay));
	return true;
}

void Engine::keypress(char *key, bool pressed)
{
	char k = 0;

	if (strcmp("enter", key) == 0)
	{
		input.enter = pressed;
	}
	else if (strcmp("leftbutton", key) == 0)
	{
		input.leftbutton = pressed;
		k = 14;
	}
	else if (strcmp("middlebutton", key) == 0)
	{
		input.middlebutton = pressed;
	}
	else if (strcmp("rightbutton", key) == 0)
	{
		input.rightbutton = pressed;
	}
	else if (strcmp("mousewheelup", key) == 0)
	{
		input.mousewheelup = pressed;
	}
	else if (strcmp("mousewheeldown", key) == 0)
	{
		input.mousewheeldown = pressed;
	}
	else if (strcmp("shift", key) == 0)
	{
		input.shift = pressed;
	}
	else if (strcmp("control", key) == 0)
	{
		input.control = pressed;
	}
	else if (strcmp("escape", key) == 0)
	{
		input.escape = pressed;
	}
	else if (strcmp("up", key) == 0)
	{
		input.up = pressed;
	}
	else if (strcmp("left", key) == 0)
	{
		input.left = pressed;
	}
	else if (strcmp("down", key) == 0)
	{
		input.down = pressed;
	}
	else if (strcmp("right", key) == 0)
	{
		input.right = pressed;
	}
	else if (strcmp("numpad0", key) == 0)
	{
		input.numpad0 = pressed;
	}
	else if (strcmp("numpad1", key) == 0)
	{
		input.numpad1 = pressed;
	}
	else if (strcmp("numpad2", key) == 0)
	{
		input.numpad2 = pressed;
	}
	else if (strcmp("numpad3", key) == 0)
	{
		input.numpad3 = pressed;
	}
	else if (strcmp("numpad4", key) == 0)
	{
		input.numpad4 = pressed;
	}
	else if (strcmp("numpad5", key) == 0)
	{
		input.numpad5 = pressed;
	}
	else if (strcmp("numpad6", key) == 0)
	{
		input.numpad6 = pressed;
	}
	else if (strcmp("numpad7", key) == 0)
	{
		input.numpad7 = pressed;
	}
	else if (strcmp("numpad8", key) == 0)
	{
		input.numpad8 = pressed;
	}
	else if (strcmp("numpad9", key) == 0)
	{
		input.numpad9 = pressed;
	}

	if (pressed)
		keystroke(k);
}

void Engine::keystroke(char key)
{
	if (map.loaded == false)
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


	projection.perspective(45.0, (float)width / height, 1.0f, 2001.0f, false);

#ifndef __linux__
	// This should probably be in render
	if (initialized && map.loaded == false)
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


	//load player sounds
	strcpy(wave[0].file, "media/sound/weapons/railgun/rg_hum.wav");
	audio.load(wave[0]);
	if (wave[0].data != NULL)
		snd_wave.push_back(wave[0]);

	strcpy(wave[0].file, "media/sound/weapons/lightning/lg_hum.wav");
	audio.load(wave[0]);
	if (wave[0].data != NULL)
		snd_wave.push_back(wave[0]);

	strcpy(wave[0].file, "media/sound/weapons/shotgun/sshotf1b.wav");
	audio.load(wave[0]);
	if (wave[0].data != NULL)
		snd_wave.push_back(wave[0]);

	strcpy(wave[0].file, "media/sound/weapons/railgun/railgf1a.wav");
	audio.load(wave[0]);
	if (wave[0].data != NULL)
		snd_wave.push_back(wave[0]);

	strcpy(wave[0].file, "media/sound/weapons/lightning/lg_fire.wav");
	audio.load(wave[0]);
	if (wave[0].data != NULL)
		snd_wave.push_back(wave[0]);

	strcpy(wave[0].file, "media/sound/weapons/rocket/rocklf1a.wav");
	audio.load(wave[0]);
	if (wave[0].data != NULL)
		snd_wave.push_back(wave[0]);


	for(unsigned int i = 0; i < entity_list.size(); i++)
	{
		unsigned int	num_wave = 0;
		bool			add = true;

		if (entity_list[i]->speaker)
		{
			if (!strcmp(entity_list[i]->speaker->file, "info_player_deathmatch"))
			{
				strcpy(wave[0].file, entity_list[i]->speaker->file);
				num_wave++;
			}
		}
		else if (entity_list[i]->trigger)
		{
			if (entity_list[i]->trigger->pickup_snd[0] != '\0')
			{
				strcpy(wave[num_wave].file, entity_list[i]->trigger->pickup_snd);
				num_wave++;
			}

			if (entity_list[i]->trigger->respawn_snd[0] != '\0')
			{
				strcpy(wave[num_wave].file, entity_list[i]->trigger->respawn_snd);
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

	entity_list[num_dynamic]->model->load(gfx, "media/models/box");
	for(unsigned int i = num_dynamic; i < entity_list.size(); i++)
	{
		bool loaded = false;

		if (entity_list[i]->model == NULL)
			continue;

		for(unsigned int j = num_dynamic; j < i; j++)
		{
			if (entity_list[j]->model == NULL)
				continue;

			entity_list[i]->model->clone(*entity_list[num_dynamic]->model);
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
	}

}

void Engine::init_camera()
{
	for(unsigned int i = 0; i < entity_list.size(); i++)
	{
		char *type = entity_list[i]->type;

		if (type == NULL)
			continue;

		if ( strcmp(type, "info_player_deathmatch") == 0 )
		{
			camera_frame.pos = entity_list[i]->position;

			spawn = get_entity();

			memcpy(entity_list[spawn]->type, "player", strlen("player") + 1);
			entity_list[spawn]->position = entity_list[i]->position;
			entity_list[spawn]->rigid = new RigidBody(entity_list[spawn]);
			entity_list[spawn]->rigid->load(gfx, "media/models/thug22/thug22");
			entity_list[spawn]->rigid->step_flag = true;
			entity_list[spawn]->model = entity_list[spawn]->rigid;
			entity_list[spawn]->player = new Player(entity_list[i], gfx, audio);
			entity_list[spawn]->position += vec3(0.0f, 10.0f, 0.0f); //adding some height
			break;
		}
	}
	audio.listener_position((float *)&(camera_frame.pos));
}

// Loads media that may be shared with multiple entities
void Engine::load_entities()
{
	init_camera();
	load_sounds();
	create_sources();
	load_models();

	for(unsigned int i = 0; i < entity_list.size(); i++)
	{
		if (entity_list[i]->light)
			entity_list[i]->rigid->gravity = false;
	}


	if (spawn != -1)
	{
		entity_list[spawn]->rigid->load(gfx, "media/models/thug22/thug22");
		//entity_list[spawn]->rigid->load(gfx, "media/models/box");
		entity_list[spawn]->position += entity_list[spawn]->rigid->center;
	}
}

void Engine::remove_light(int index)
{
	// Light list wont be updated until the next step, so manually delete
	if (entity_list[index]->light)
	{
		for (int i = 0; i < light_list.size(); i++)
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
			remove_light(index);

			entity_list[index]->~Entity();
			return index++;
		}
		index++;
	}

	return num_dynamic - 1;
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
					entity_list[i]->speaker->source = audio.create_source(entity_list[i]->speaker->loop, false);
#ifndef __OBJC__
					alSourcef(entity_list[i]->speaker->source, AL_GAIN, 4.0f);
#endif
					audio.select_buffer(entity_list[i]->speaker->source, snd_wave[j].buffer);
					audio.effects(entity_list[i]->speaker->source);

					if (entity_list[i]->speaker->loop)
						audio.play(entity_list[i]->speaker->source);
					break;
				}
			}
		}
		else if (entity_list[i]->trigger != NULL)
		{
			entity_list[i]->rigid->gravity = false;
			entity_list[i]->trigger->source = audio.create_source(false, false);
#ifndef __OBJC__
            alSourcef(entity_list[i]->trigger->source, AL_GAIN, 30.0f);
#endif
			audio.effects(entity_list[i]->trigger->source);
		}
	}

	// position sources
	update_audio();
}

void Engine::update_audio()
{
	audio.listener_position((float *)&(camera_frame.pos));

	if (spawn != -1)
	{
		audio.listener_velocity((float *)&(entity_list[spawn]->rigid->velocity));
		audio.listener_orientation((float *)&(entity_list[spawn]->rigid->morientation.m));
	}
	for(unsigned int i = 0; i < entity_list.size(); i++)
	{
		if (entity_list[i]->speaker)
		{
			audio.source_position(entity_list[i]->speaker->source, (float *)(&entity_list[i]->position));
			audio.source_velocity(entity_list[i]->speaker->source, (float *)(&entity_list[i]->rigid->velocity));
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
	if (map.loaded == false)
		return;

	menu.ingame = false;
	for(unsigned int i = 0; i < entity_list.size(); i++)
	{
		if (entity_list[i]->speaker)
			entity_list[i]->speaker->destroy(audio);
		delete entity_list[i];
	}
	entity_list.clear();
	light_list.clear();

//	light_list.~List();
//	entity_list~List();

	post.destroy();
	mlight2.destroy();
//	mlight3.destroy();
	map.unload(gfx);
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
	debugf("Shutting down.\n");
	destroy_buffers();
	unload();
	gfx.GetDebugLog();
	gfx.destroy();
	audio.destroy();
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


// Need asset manager class so things arent doubly loaded
int load_texture(Graphics &gfx, char *file_name)
{
	int width, height, components, format;
	int tex_object;

	byte *bytes = gltLoadTGA(file_name, &width, &height, &components, &format);
	if (bytes == NULL)
	{
		debugf("Unable to load texture %s\n", file_name);
		return 0;
	}
	tex_object = gfx.LoadTexture(width, height, components, format, bytes);
	delete [] bytes;

#ifndef DIRECTX
	if (format == GL_BGRA_EXT)
	{
		// negative means it has an alpha channel
		return -tex_object;
	}
#endif
	return tex_object;
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

	ret = sscanf(cmd, "damage %s", data);
	if (ret == 1)
	{
		snprintf(msg, LINE_SIZE, "damage %s\n", data);
		menu.print(msg);

		int damage = abs32(atoi(data));
		int health_damage = damage / 3;
		int armor_damage = 2 * health_damage;

		if (armor_damage > entity_list[spawn]->player->armor)
		{
			armor_damage -= entity_list[spawn]->player->armor;
			entity_list[spawn]->player->armor = 0;
			health_damage += armor_damage;
		}
		else
		{
			entity_list[spawn]->player->armor -= armor_damage;
		}

		entity_list[spawn]->player->health -= health_damage;

		return;
	}

	ret = sscanf(cmd, "health %s", data);
	if (ret == 1)
	{
		snprintf(msg, LINE_SIZE, "health %s\n", data);
		menu.print(msg);
		entity_list[spawn]->player->health += atoi(data);
		if (entity_list[spawn]->player->health > 100)
			entity_list[spawn]->player->health = 100;
		return;
	}

	ret = sscanf(cmd, "armor %s", data);
	if (ret == 1)
	{
		snprintf(msg, LINE_SIZE, "armor %s\n", data);
		menu.print(msg);
		entity_list[spawn]->player->armor += atoi(data);
		if (entity_list[spawn]->player->armor > 200)
			entity_list[spawn]->player->armor = 200;
		return;
	}

	if (strcmp(cmd, "weapon_grenadelauncher") == 0)
	{
		snprintf(msg, LINE_SIZE, "weapon_grenadelauncher\n");
		menu.print(msg);

		if (entity_list[spawn]->player->current_weapon == wp_none)
			entity_list[spawn]->player->current_weapon = wp_grenade;

		entity_list[spawn]->player->weapon_flags |= wp_grenade;
		entity_list[spawn]->player->ammo_rockets = MAX(10, entity_list[spawn]->player->ammo_grenades);
		return;
	}

	if (strcmp(cmd, "weapon_rocketlauncher") == 0)
	{
		snprintf(msg, LINE_SIZE, "weapon_rocketlauncher\n");
		menu.print(msg);

		if (entity_list[spawn]->player->current_weapon == wp_none)
			entity_list[spawn]->player->current_weapon = wp_rocket;

		entity_list[spawn]->player->weapon_flags |= wp_rocket;
		entity_list[spawn]->player->ammo_rockets = MAX(10, entity_list[spawn]->player->ammo_rockets);
		return;
	}

	if (strcmp(cmd, "weapon_shotgun")  == 0)
	{
		snprintf(msg, LINE_SIZE, "weapon_shotgun\n");
		menu.print(msg);

		if (entity_list[spawn]->player->current_weapon == wp_none)
			entity_list[spawn]->player->current_weapon = wp_shotgun;

		entity_list[spawn]->player->weapon_flags |= wp_shotgun;
		entity_list[spawn]->player->ammo_shells = MAX(10, entity_list[spawn]->player->ammo_shells);
		return;
	}

	if (strcmp(cmd, "weapon_lightning")  == 0)
	{
		snprintf(msg, LINE_SIZE, "weapon_lightning\n");
		menu.print(msg);

		if (entity_list[spawn]->player->current_weapon == wp_none)
			entity_list[spawn]->player->current_weapon = wp_lightning;

		entity_list[spawn]->player->weapon_flags |= wp_lightning;
		entity_list[spawn]->player->ammo_lightning = MAX(100, entity_list[spawn]->player->ammo_lightning);
		return;
	}

	if (strcmp(cmd, "weapon_railgun") == 0)
	{
		snprintf(msg, LINE_SIZE, "weapon_railgun\n");
		menu.print(msg);

		if (entity_list[spawn]->player->current_weapon == wp_none)
			entity_list[spawn]->player->current_weapon = wp_railgun;

		entity_list[spawn]->player->weapon_flags |= wp_railgun;
		entity_list[spawn]->player->ammo_slugs = MAX(10, entity_list[spawn]->player->ammo_slugs);
		return;
	}

	if (strcmp(cmd, "weapon_plasma") == 0)
	{
		snprintf(msg, LINE_SIZE, "weapon_plasma\n");
		menu.print(msg);

		if (entity_list[spawn]->player->current_weapon == wp_none)
			entity_list[spawn]->player->current_weapon = wp_plasma;

		entity_list[spawn]->player->weapon_flags |= wp_plasma;
		entity_list[spawn]->player->ammo_slugs = MAX(10, entity_list[spawn]->player->ammo_plasma);
		return;
	}

	ret = sscanf(cmd, "ammo_rockets %s", data);
	if (ret == 1)
	{
		snprintf(msg, LINE_SIZE, "ammo_rockets %s\n", data);
		menu.print(msg);
		entity_list[spawn]->player->ammo_rockets += atoi(data);
		return;
	}

	ret = sscanf(cmd, "ammo_slugs %s", data);
	if (ret == 1)
	{
		snprintf(msg, LINE_SIZE, "ammo_slugs %s\n", data);
		menu.print(msg);
		entity_list[spawn]->player->ammo_slugs += atoi(data);
		return;
	}

	ret = sscanf(cmd, "ammo_shells %s", data);
	if (ret == 1)
	{
		snprintf(msg, LINE_SIZE, "ammo_shells %s\n", data);
		menu.print(msg);
		entity_list[spawn]->player->ammo_shells += atoi(data);
		return;
	}

	ret = sscanf(cmd, "ammo_bullets %s", data);
	if (ret == 1)
	{
		snprintf(msg, LINE_SIZE, "ammo_bullets %s\n", data);
		menu.print(msg);
		entity_list[spawn]->player->ammo_bullets += atoi(data);
		return;
	}

	ret = sscanf(cmd, "ammo_lightning %s", data);
	if (ret == 1)
	{
		snprintf(msg, LINE_SIZE, "ammo_lightning %s\n", data);
		menu.print(msg);
		entity_list[spawn]->player->ammo_lightning += atoi(data);
		return;
	}

	ret = sscanf(cmd, "ammo_plasma %s", data);
	if (ret == 1)
	{
		snprintf(msg, LINE_SIZE, "ammo_plasma %s\n", data);
		menu.print(msg);
		entity_list[spawn]->player->ammo_plasma += atoi(data);
		return;
	}

	ret = sscanf(cmd, "ammo_bfg %s", data);
	if (ret == 1)
	{
		snprintf(msg, LINE_SIZE, "ammo_bfg %s\n", data);
		menu.print(msg);
		entity_list[spawn]->player->ammo_bfg += atoi(data);
		return;
	}

	ret = sscanf(cmd, "teleport %s", data);
	if (ret == 1)
	{
		snprintf(msg, LINE_SIZE, "target %s\n", data);
		menu.print(msg);

		for (unsigned int i = 0; i < entity_list.size(); i++)
		{
			if (entity_list[i]->type && strcmp(entity_list[i]->type, "misc_teleporter_dest"))
				continue;
				
			if (!strcmp(entity_list[i]->target_name, data))
			{
				matrix4 matrix;
				entity_list[spawn]->position = entity_list[i]->position + vec3(0.0f, 50.0f, 0.0f);

				switch (entity_list[i]->angle)
				{
				case 0:
					matrix4::mat_left(matrix, entity_list[spawn]->position);
					break;
				case 90:
					matrix4::mat_forward(matrix, entity_list[spawn]->position);
					break;
				case 180:
					matrix4::mat_right(matrix, entity_list[spawn]->position);
					break;
				case 270:
					matrix4::mat_backward(matrix, entity_list[spawn]->position);
					break;
				}
				camera_frame.forward.x = matrix.m[8];
				camera_frame.forward.y = matrix.m[9];
				camera_frame.forward.z = matrix.m[10];
				camera_frame.up = vec3(0.0f, 1.0f, 0.0f);
				break;

			}
		}
		return;
	}

	ret = (int)strstr(cmd, "respawn");
	if (ret)
	{
		static int last_spawn = 0;
		unsigned int i = last_spawn;
		bool spawned = false;

		ret = sscanf(cmd, "respawn %s", data);
		if (ret == 1)
		{
			int index = atoi(data);

			if (index < 0 || index > entity_list.size())
			{
				debugf("respawn given invalid entity index\n");
				return;
			}

			matrix4 matrix;

			camera_frame.set(matrix);
			entity_list[spawn]->position = entity_list[index]->position + vec3(0.0f, 50.0f, 0.0f);

			switch (entity_list[i]->angle)
			{
			case 0:
				matrix4::mat_left(matrix, entity_list[spawn]->position);
				break;
			case 90:
				matrix4::mat_forward(matrix, entity_list[spawn]->position);
				break;
			case 180:
				matrix4::mat_right(matrix, entity_list[spawn]->position);
				break;
			case 270:
				matrix4::mat_backward(matrix, entity_list[spawn]->position);
				break;
			}
			camera_frame.forward.x = matrix.m[8];
			camera_frame.forward.y = matrix.m[9];
			camera_frame.forward.z = matrix.m[10];
			camera_frame.up = vec3(0.0f, 1.0f, 0.0f);
			debugf("Spawning on entity %d\n", index);
			return;
		}


		while (spawned == false)
		{
			for (i = last_spawn; i < entity_list.size(); i++)
			{
				if (entity_list[i]->type && strcmp(entity_list[i]->type, "info_player_deathmatch"))
					continue;

				{
					matrix4 matrix;

					camera_frame.set(matrix);
					entity_list[spawn]->position = entity_list[i]->position + vec3(0.0f, 50.0f, 0.0f);

					switch (entity_list[i]->angle)
					{
					case 0:
						matrix4::mat_left(matrix, entity_list[spawn]->position);
						break;
					case 90:
						matrix4::mat_forward(matrix, entity_list[spawn]->position);
						break;
					case 180:
						matrix4::mat_right(matrix, entity_list[spawn]->position);
						break;
					case 270:
						matrix4::mat_backward(matrix, entity_list[spawn]->position);
						break;
					}
					camera_frame.forward.x = matrix.m[8];
					camera_frame.forward.y = matrix.m[9];
					camera_frame.forward.z = matrix.m[10];
					camera_frame.up = vec3(0.0f, 1.0f, 0.0f);
					last_spawn = i + 1;
					debugf("Spawning on entity %d\n", i);
					entity_list[spawn]->player->respawn();
					spawned = true;
					break;

				}
			}

			if (i == entity_list.size())
			{
				if (last_spawn != 0)
				{
					last_spawn = 0;
				}
				else
				{
					debugf("Failed to find a spawn point");
					break;
				}
			}
		}
		return;
	}

	ret = sscanf(cmd, "push %s", data);
	if (ret == 1)
	{
		snprintf(msg, LINE_SIZE, "push %s\n", data);
		menu.print(msg);

		for (unsigned int i = 0; i < entity_list.size(); i++)
		{
			if (!strcmp(entity_list[i]->target_name, data))
			{
				//target - origin
				vec3 dir = entity_list[i]->position - entity_list[spawn]->position;

				//add velocity towards target
				entity_list[spawn]->rigid->velocity += dir;
				break;
			}
		}
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

	ret = sscanf(cmd, "say \"%[^\"]s", data);
	if (ret == 1)
	{
		chat(cmd);
		return;
	}

	ret = strcmp(cmd, "sv_list");
	if (ret == 0)
	{
		unsigned int current = (unsigned int)time(NULL);

		snprintf(msg, LINE_SIZE, "Client list\n");
		menu.print(msg);

		snprintf(msg, LINE_SIZE, "s: %s %d kills %d deaths %s %d idle\n", entity_list[spawn]->player->name,
			entity_list[spawn]->player->stats.kills,
			entity_list[spawn]->player->stats.deaths,
			"127.0.0.1:65535",
			0);
		menu.print(msg);


		for (unsigned int i = 0; i < client_list.size(); i++)
		{
			snprintf(msg, LINE_SIZE, "%d: %s %d kills %d deaths %s %d idle\n", i, entity_list[client_list[i]->entity]->player->name,
				entity_list[client_list[i]->entity]->player->stats.kills, 
				entity_list[client_list[i]->entity]->player->stats.deaths,
				client_list[i]->socketname,
				current - client_list[i]->last_time);
			menu.print(msg);
		}
		return;
	}

	ret = sscanf(cmd, "kick %s", data);
	if (ret == 1)
	{
		kick(atoi(data));
		return;
	}

	if (strcmp(cmd, "r_res") == 0)
	{
		snprintf(msg, LINE_SIZE, "%dx%d\n", gfx.width, gfx.height);
		menu.print(msg);
		return;
	}

	ret = sscanf(cmd, "bind %d", &port);
	if (ret == 1)
	{
		snprintf(msg, LINE_SIZE, "binding to port %d\n", port);
		menu.print(msg);
		load("media/maps/q3tourney2.bsp");
		bind(port);
		return;
	}

	ret = strcmp(cmd, "unit_test_maps");
	if (ret == 0)
	{
		if (map.loaded)
		{
			map.unload(gfx);
		}
		map.load("media/maps/q3dm1.bsp");
		map.unload(gfx);
		map.load("media/maps/q3dm2.bsp");
		map.unload(gfx);
		map.load("media/maps/q3dm3.bsp");
		map.unload(gfx);
		map.load("media/maps/q3dm4.bsp");
		map.unload(gfx);
		map.load("media/maps/q3dm5.bsp");
		map.unload(gfx);
		map.load("media/maps/q3dm6.bsp");
		map.unload(gfx);
		map.load("media/maps/q3dm7.bsp");
		map.unload(gfx);
		map.load("media/maps/q3dm8.bsp");
		map.unload(gfx);
		map.load("media/maps/q3dm9.bsp");
		map.unload(gfx);
		map.load("media/maps/q3dm10.bsp");
		map.unload(gfx);
		map.load("media/maps/q3dm11.bsp");
		map.unload(gfx);
		map.load("media/maps/q3dm12.bsp");
		map.unload(gfx);
		map.load("media/maps/q3dm13.bsp");
		map.unload(gfx);
		map.load("media/maps/q3dm14.bsp");
		map.unload(gfx);
		map.load("media/maps/q3dm15.bsp");
		map.unload(gfx);
		map.load("media/maps/q3dm16.bsp");
		map.unload(gfx);
		map.load("media/maps/q3dm17.bsp");
		map.unload(gfx);
		map.load("media/maps/q3dm18.bsp");
		map.unload(gfx);
		map.load("media/maps/q3dm19.bsp");
		map.unload(gfx);
		map.load("media/maps/q3tourney1.bsp");
		map.unload(gfx);
		map.load("media/maps/q3tourney2.bsp");
		map.unload(gfx);
		map.load("media/maps/q3tourney3.bsp");
		map.unload(gfx);
		map.load("media/maps/q3tourney4.bsp");
		map.unload(gfx);
		map.load("media/maps/q3tourney5.bsp");
		map.unload(gfx);
		map.load("media/maps/q3tourney6.bsp");
		map.unload(gfx);
		map.load("media/maps/q3ctf1.bsp");
		map.unload(gfx);
		map.load("media/maps/q3ctf2.bsp");
		map.unload(gfx);
		map.load("media/maps/q3ctf3.bsp");
		map.unload(gfx);
		map.load("media/maps/q3ctf4.bsp");
		map.unload(gfx);
		return;
	}

	ret = strcmp(cmd, "quit");
	if (ret == 0)
	{
		if (map.loaded)
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

	ret = strcmp(cmd, "noclip");
	if (ret == 0)
	{
		if (spawn != -1)
		{
			entity_list[spawn]->rigid->sleep = true;
		}
		return;
	}

	ret = sscanf(cmd, "animation %s", data);
	if (ret == 1)
	{
		snprintf(msg, LINE_SIZE, "%s\n", cmd);
		menu.print(msg);
		zcc.select_animation(atoi(data));
		return;
	}

		ret = strcmp(cmd, "give all");
	if (ret == 0)
	{
		snprintf(msg, LINE_SIZE, "give all\n");
		menu.print(msg);
		entity_list[spawn]->player->ammo_bfg = 999;
		entity_list[spawn]->player->ammo_bullets = 999;
		entity_list[spawn]->player->ammo_lightning= 999;
		entity_list[spawn]->player->ammo_plasma = 999;
		entity_list[spawn]->player->ammo_grenades = 999;
		entity_list[spawn]->player->ammo_rockets = 999;
		entity_list[spawn]->player->ammo_shells = 999;
		entity_list[spawn]->player->ammo_slugs = 999;
		entity_list[spawn]->player->armor = 200;
		entity_list[spawn]->player->health = 100;
		entity_list[spawn]->player->weapon_flags = ~0;
		return;
	}


	if (strcmp(cmd, "disconnect") == 0)
	{
		snprintf(msg, LINE_SIZE, "disconnecting\n");
		menu.print(msg);
		unload();
		return;
	}

	snprintf(msg, LINE_SIZE, "Unknown command: %s\n", cmd);
	menu.print(msg);
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
		map.unload(gfx);
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

void Engine::chat(char *msg)
{
	strcat(reliable.msg, msg);
	reliable.sequence = sequence;
}

void Engine::handle_weapons(Player &player)
{
	bool fired = false;
	if (player.reload_timer > 0)
		player.reload_timer--;

	if (player.dead)
	{
		if (input.leftbutton && player.reload_timer == 0)
		{
			console("respawn");
		}

		return;
	}

	switch (player.current_weapon)
	{
	case wp_railgun:
		player.weapon_idle_sound = "media/sound/weapons/railgun/rg_hum.wav";
		//		audio.select_buffer(entity->speaker->source, snd_wave[WP_RAILGUN_IDLE].buffer);
		break;
	case wp_lightning:
		player.weapon_idle_sound = "media/sound/weapons/lightning/lg_hum.wav";
		break;
	default:
		player.weapon_idle_sound = "";
		break;
	}

	if (input.leftbutton && player.reload_timer == 0)
	{

		if (player.current_weapon == wp_rocket && player.ammo_rockets > 0)
		{
			player.reload_timer = 30;

			fired = true;
			Entity *entity = entity_list[get_entity()];
			entity->position = camera_frame.pos;

			entity->trigger = new Trigger(entity);
			entity->trigger->hide = false;
			entity->trigger->self = false;
			entity->trigger->idle = true;
			entity->trigger->explode = true;
			entity->trigger->explode_timer = 10;
			memcpy(entity->trigger->action, "damage 100", strlen("damage 100") + 1);

			entity->light = new Light(entity, gfx, 999);
			entity->light->color = vec3(1.0f, 1.0f, 1.0f);
			entity->light->intensity = 1000.0f;

			entity->rigid = new RigidBody(entity);
			entity->model = entity->rigid;
			camera_frame.set(entity->rigid->morientation);

			entity->rigid->load(gfx, "media/models/rocket/rocket");
			entity->rigid->velocity = camera_frame.forward * -6.25f;
			entity->rigid->net_force = camera_frame.forward * -10.0f;
			entity->rigid->angular_velocity = vec3();
			entity->rigid->gravity = false;


			player.attack_sound = "media/sound/weapons/rocket/rocklf1a.wav";
		}
		else if (player.current_weapon == wp_plasma && player.ammo_plasma > 0)
		{
			player.reload_timer = 5;

			fired = true;
			Entity *entity = entity_list[get_entity()];
			entity->rigid = new RigidBody(entity);
			entity->model = entity->rigid;
			entity->position = camera_frame.pos;
			entity->rigid->load(gfx, "media/models/ball");
			entity->rigid->velocity = camera_frame.forward * -125.0f;
			entity->rigid->angular_velocity = vec3();
			entity->rigid->gravity = false;
			entity->trigger = new Trigger(entity);
			entity->trigger->hide = false;
			entity->trigger->self = false;
			entity->trigger->idle = true;
			memcpy(entity->trigger->action, "damage 15", 11);

			camera_frame.set(entity->model->morientation);

			entity->light = new Light(entity, gfx, 999);
			entity->light->color = vec3(0.0f, 0.0f, 1.0f);
			entity->light->intensity = 1000.0f;

//			player.attack_sound = "media/sound/weapons/rocket/rocklf1a.wav";
		}
		else if (player.current_weapon == wp_grenade && player.ammo_grenades > 0)
		{
			player.reload_timer = 30;

			fired = true;
			Entity *entity = entity_list[get_entity()];
			entity->rigid = new RigidBody(entity);
			entity->model = entity->rigid;
			entity->position = camera_frame.pos;
			entity->rigid->load(gfx, "media/models/pineapple/pineapple");
			entity->rigid->velocity = camera_frame.forward * -25.0f;
			entity->rigid->angular_velocity = vec3();
			entity->rigid->gravity = true;
			entity->trigger = new Trigger(entity);
			entity->trigger->hide = false;
			entity->trigger->self = false;
			entity->trigger->idle = true;
			entity->trigger->explode = true;
			entity->trigger->explode_timer = 10;


			memcpy(entity->trigger->action, "damage 100", 12);

			camera_frame.set(entity->model->morientation);

			//			player.attack_sound = "media/sound/weapons/rocket/rocklf1a.wav";
		}
		else if (player.current_weapon == wp_lightning && player.ammo_lightning > 0)
		{
			player.reload_timer = 10;

			fired = true;
			Entity *entity = entity_list[get_entity()];
			entity->rigid = new RigidBody(entity);
			entity->position = camera_frame.pos;
			entity->rigid->clone(*(entity_list[num_dynamic + 0]->model));
			entity->rigid->velocity = camera_frame.forward * -1.0f;
			entity->rigid->angular_velocity = vec3();
			entity->rigid->gravity = false;
			entity->model = entity->rigid;
			entity->rigid->set_target(*(entity_list[spawn]));
			camera_frame.set(entity->model->morientation);
			player.ammo_rockets--;


			entity->light = new Light(entity, gfx, 999);
			entity->light->color = vec3(1.0f, 1.0f, 1.0f);
			entity->light->intensity = 1000.0f;
			player.ammo_lightning--;

			player.attack_sound = "media/sound/weapons/lightning/lg_fire.wav";
		}
		else if (player.current_weapon == wp_railgun && player.ammo_slugs > 0)
		{
			player.reload_timer = 120; // two seconds

			fired = true;
			Entity *entity = entity_list[get_entity()];
			entity->rigid = new RigidBody(entity);
			entity->position = camera_frame.pos;
			entity->rigid->clone(*(entity_list[num_dynamic + 1]->model));
			entity->rigid->velocity = camera_frame.forward * -100.0f;
			entity->rigid->angular_velocity = vec3();
			entity->rigid->gravity = false;
			entity->model = entity->rigid;
			camera_frame.set(entity->model->morientation);
			player.ammo_slugs--;

			player.attack_sound = "media/sound/weapons/railgun/railgf1a.wav";
		}
		else if (player.current_weapon == wp_shotgun && player.ammo_shells > 0)
		{
			player.reload_timer = 60; // one seconds

			player.ammo_shells--;

			fired = true;
			vec3 forward;
			float distance;
			player.entity->model->getForwardVector(forward);

			map.hitscan(player.entity->position, forward, distance);
			//vec3 end = player.entity->position + forward * distance;


//			Entity *entity = new Entity();
//			entity->decal = new Decal(entity);
//			entity->position = end;
//			entity->decal->normal = normal;

			player.attack_sound = "media/sound/weapons/shotgun/sshotf1b.wav";
		}


		if (fired)
		{
			for (unsigned int i = 0; i < snd_wave.size(); i++)
			{
				if (strcmp(snd_wave[i].file, player.attack_sound) == 0)
				{
					audio.select_buffer(player.entity->speaker->source, snd_wave[i].buffer);
					break;
				}
			}
			audio.play(player.entity->speaker->source);
		}

	}
}

