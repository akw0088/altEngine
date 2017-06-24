#include "include.h"

#ifndef LIGHT_H
#define LIGHT_H

class Light
{
public:
	Light(Entity *entity, Graphics &gfx, int num, float scale);
	void render_shadow_volumes(int current_light);
	void generate_volumes(Bsp &map);

	void generate_cubemaps(Graphics &gfx, int width, int height);

	void select_shadowmap(Graphics &gfx, int face);
	void extend(Edge &edge_list, vec3 position, int current_light);

	Entity	*entity;
	unsigned int fbo_shadowmaps[6];
	unsigned int quad_tex[6];
	unsigned int depth_tex[6];
	vec3	color;
	float	intensity;
	float	attenuation;
	bool	active;
	bool	timer_flag;
	int		timer;
	unsigned int		light_num;
	vector<shadowvol_t>	shadow_list;
	Edge	edge_list;
};

#endif
