#include "include.h"

#ifndef DECAL_H
#define DECAL_H

class Decal
{
public:
	Decal(Entity *entity);
	void init(Graphics &gfx);
	void render(Graphics &gfx, Global &global);
	Entity			*entity;

private:
	unsigned int decal_tex;
	matrix4 mvp;
};

#endif
