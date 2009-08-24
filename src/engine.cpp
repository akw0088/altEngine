#include "include.h"

void Engine::init(void *param1, void *param2)
{
	wave_t	wave;
	Parse parse;

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
	parse.parse_entity(map.getEnts(), entity_list);
}

void Engine::render()
{
	char msg[80];
	int i, j;

	gfx.clear();

	camera.update(keyboard);
	if (!keyboard.control)
	{
		entity_list[0].position = camera.pos;
	}

	camera.set();
	map.render(entity_list[0], gfx, keyboard);
	for (i = 0, j = 0; i < entity_list.num_entities; i++)
	{
		if (entity_list[i].position == vec3(0.0f, 0.0f, 0.0f))
			continue;
		entity_list[i].render(gfx);
		camera.set();
		j++;
	}

	snprintf(msg, 80, "Rendered %d entities %f %f %f", j, entity_list[12].position.x, entity_list[12].position.y, entity_list[12].position.z);
	gfx.drawText(msg, 0.01f, 0.05f);
	gfx.swap();
}

void Engine::step()
{
	plane	p = {0.0f, 1.0f, 0.0f, -1.0f};

	for(int i = 0; i < entity_list.num_entities; i++)
	{
		//entity[i].net_force = vec3(0.0f, -9.8f, 0.0f);
		entity_list[i].integrate(0.016f);
	}

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
