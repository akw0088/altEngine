#include "include.h"

#ifndef LIGHT_H
#define LIGHT_H

class Light
{
public:
	Light(Entity *entity);
	void render_shadows();
	void generate_volumes(Bsp &map);
	void extend(Edge &edge_list, vec3 pos);


	Entity	*entity;

	vec3	color;
	int		intensity;
	bool	active;
	Edge	edge_list;
};

#endif
