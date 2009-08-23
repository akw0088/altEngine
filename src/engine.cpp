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
	map.load("media/maps/q3tourney3.bsp");
	map.loadTextures(gfx);
	initialized = true;

	Entity *box = new Entity(10.0f, vec3(1.0f, 1.0f, 1.0f));
	entities = box;
	num_entities = 1;
}

void Engine::render()
{
	if (!initialized)
		return;

	gfx.clear();
	camera.update(keyboard);
	camera.set();

//	gfx.drawText("media/maps/q3tourney3.bsp", 0.01f, 0.01f);
	map.render(entities[0].position, gfx, keyboard);
	if (!keyboard.control)
		entities[0].position = camera.pos;
	entities[0].render(gfx);
	gfx.swap();
}

void Engine::step()
{
	plane	p = {0.0f, 1.0f, 0.0f, -1.0f};

	for(int i = 0; i < num_entities; i++)
	{
		entities[i].net_force = vec3(0.0f, -9.8f, 0.0f);
		entities[i].integrate(0.016f);
	}

	for(int i = 0; i < num_entities; i++)
	{
		for(int j = 1; j < num_entities; j++)
		{
			if ( entities[i].collision_detect(entities[j]) )
			{
				// collision detected
			}
		}

		if ( entities[i].collision_detect(p) )
		{
			// collision with ground plane
		}
	}
}

bool Engine::mousepos(int x, int y, int deltax, int deltay)
{
	if (keyboard.escape)
		return false;

	camera.update(vec2(deltax, deltay));
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
	initialized = false;
	map.unload();
	gfx.destroy();
	audio.destroy();
}
