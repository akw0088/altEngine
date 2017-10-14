#include "include.h"

#ifndef LIGHT_H
#define LIGHT_H

class Light
{
public:
	Light(Entity *entity, Graphics &gfx, int num, float scale);
	void render_shadow_volumes(Graphics &gfx, int current_light);
	void generate_map_volumes(Bsp &map);
	void generate_ent_volumes(Graphics &gfx, vector<Entity *> &entity_list);

	void destroy(Graphics &gfx);
	void generate_cubemaps(Graphics &gfx);

	Entity	*entity;
	unsigned int fbo_shadowmaps[6];
	unsigned int quad_tex[6];
	unsigned int depth_tex[6];
	unsigned int array_tex;


	matrix4 shadow_projection;
	matrix4 shadow_matrix[6];

	ShadowVolume shadow;
	int shadow_flag;

	vec3	color;
	float	intensity;
	float	attenuation;
	bool	active;
	bool	timer_flag;
	float	lightmap_scale;
	int		timer;
	unsigned int		light_num;
};

#endif
