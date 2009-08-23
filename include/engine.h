#include "include.h"

#ifndef ENGINE_H
#define ENGINE_H

class Engine
{
public:
	Engine()
	{
		initialized = false;
	}

	void init(void *param1, void *param2);
	void render();
	void step();
	bool mousepos(int x, int y, int deltax, int deltay);
	void keystroke(char *key, bool pressed);
	void resize(int width, int height);
	void destroy();
	~Engine()
	{
	}
private:
	bool		initialized;
	Bsp			map;
//	voxel		map;
	Graphics	gfx;
	Sound		audio;
	Entity		*entities;
	int			num_entities;
	Frame		camera;
	Keyboard	keyboard;
};

#endif

