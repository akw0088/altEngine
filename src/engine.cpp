#include "include.h"

void Engine::init(void *param1, void *param2)
{
	wave_t	wave;

	try
	{
		audio.init();
		audio.load("media/sound/mwmusic.wav", &wave);
		audio.play(&wave);
	}
	catch (char *error)
	{
		printf("%s\n", error);
	}

	gfx.init(param1, param2);
	try
	{
		map.load("media/maps/q3tourney3.bsp");
	}
	catch (char *error)
	{
		printf("%s\n", error);
		throw error;
	}

	map.loadTextures(gfx);
	Entity *box = new Entity(10.0f, vec3(0.0f, 0.0f, 0.0f));
	entity_list.addEntity(box);
	parse_entity(map.getEnts(), entity_list);
}

void frame2ent(Frame *camera, Entity &entity, Keyboard &keyboard)
{
	vec3		right;

	right = vec3::crossproduct(camera->up, camera->forward);
	right.normalize();

	if (keyboard.control)
	{
	entity.position = camera->pos;

	entity.morientation.m[0] = right.x;
	entity.morientation.m[1] = right.y;
	entity.morientation.m[2] = right.z;

	entity.morientation.m[3] = camera->up.x;
	entity.morientation.m[4] = camera->up.y;
	entity.morientation.m[5] = camera->up.z;

	entity.morientation.m[6] = camera->forward.x;
	entity.morientation.m[7] = camera->forward.y;
	entity.morientation.m[8] = camera->forward.z;
	}
}

void Engine::render()
{
	char msg[80];
	int i, j;

	gfx.clear();

	camera.update(keyboard);
	frame2ent(&camera, entity_list[0], keyboard);

	camera.set();
	map.render(entity_list[0], gfx, keyboard);

	entity_list[0].render(gfx);
	for (i = 1, j = 0; i < entity_list.num_entities; i++)
	{
		/*
		// Needs work =(
		if (entity_list[0].in_frustum(entity_list[i]) == false)
			continue;
		*/
		
		entity_list[i].render(gfx);
		camera.set();
		j++;
	}

	snprintf(msg, 80, "Rendered %d entities", j);
	gfx.drawText(msg, 0.01f, 0.05f);
	gfx.swap();
}

void Engine::step()
{
	for(int i = 0; i < entity_list.num_entities; i++)
	{
		//entity[i].net_force = vec3(0.0f, -9.8f, 0.0f);
		entity_list[i].integrate(0.016f);
	}
/*
	for(int i = 0; i < entity_list.num_entities; i++)
	{
		for(int j = 1; j < entity_list.num_entities; j++)
		{
			if ( entity_list[i].collision_detect(entity_list[j]) )
			{
				// collision detected
			}
		}

		if ( entity_list[i].collision_detect(p) )
		{
			// collision with ground plane
		}
	}
	*/
}

bool Engine::mousepos(int x, int y, int deltax, int deltay)
{
	if (keyboard.escape)
		return false;

	camera.update(vec2((float)deltax, (float)deltay));
	return true;
}

void Engine::keystroke(char *key, bool pressed)
{
	if (strcmp("enter", key) == 0)
		keyboard.enter = pressed;
	else if (strcmp("shift", key) == 0)
		keyboard.shift = pressed;
	else if (strcmp("control", key) == 0)
		keyboard.control = pressed;
	else if (strcmp("escape", key) == 0)
		keyboard.escape = pressed;
	else if (strcmp("up", key) == 0)
		keyboard.up = pressed;
	else if (strcmp("left", key) == 0)
		keyboard.left = pressed;
	else if (strcmp("down", key) == 0)
		keyboard.down = pressed;
	else if (strcmp("right", key) == 0)
		keyboard.right = pressed;
}

void Engine::resize(int width, int height)
{
	gfx.resize(width, height);
}

void Engine::destroy()
{
	map.unload();
	gfx.destroy();
	audio.destroy();
}
