#include "include.h"

#ifndef LIGHT_H
#define LIGHT_H

class Light
{
public:
	Light(Entity *entity);
	void render_shadows();
	void generate_volumes(Bsp &map);
	void extend(vec3 position);


	Entity	*entity;

	vec3	color;
	int		intensity;
	bool	active;
	vector<triangle_t>	backface_list;
};

#endif
