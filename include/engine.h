#include "include.h"

#ifndef ENGINE_H
#define ENGINE_H

class Engine
{
public:
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
	Graphics		gfx;
	Sound			audio;
	Bsp				map;
	Plane			*collision_plane;
	int				num_planes;
	List<Entity>	entity_list;
	Frame			camera;
	Keyboard		keyboard;
};

#endif

