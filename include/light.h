#include "include.h"

#ifndef LIGHT_H
#define LIGHT_H

class Light
{
public:
	Light(Entity *entity, Graphics &gfx, int num);
	void render_shadow_volumes();
	void generate_volumes(Bsp &map);

	void generate_cubemaps(int width, int height);

	void select_shadowmap(Graphics &gfx, int face);

	Entity	*entity;
	unsigned int quad_tex[6];
	unsigned int depth_tex[6];
	vec3	color;
	int		intensity;
	bool	active;
	int		light_num;
	vector<shadowvol_t>	shadow_list;
	Edge	edge_list;
};

#endif
