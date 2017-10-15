#include "include.h"

#ifndef LIGHT_H
#define LIGHT_H

#define MAX_SHADOWVOL 1
#define SHADOWVOL_MAX_DIST (800.0f)

#define NUM_CUBE_FACE 6
#define SHADOWMAP_DEFAULT_RES 1024

class Light
{
public:
	Light(Entity *entity, Graphics &gfx, int num, float scale);
	void render_shadow_volume(Graphics &gfx, int index);
	void generate_map_volumes(Bsp &map);
	void generate_ent_volumes(Graphics &gfx, vector<Entity *> &entity_list);

	void destroy(Graphics &gfx);
	void generate_cubemaps(Graphics &gfx);

	Entity	*entity;

	// Shadow maps
	unsigned int fbo_shadowmaps[NUM_CUBE_FACE];
	unsigned int quad_tex[NUM_CUBE_FACE];
	unsigned int depth_tex[NUM_CUBE_FACE];
	unsigned int array_tex;


	matrix4	shadow_projection;
	matrix4	shadow_matrix[NUM_CUBE_FACE];
	int		shadow_flag;


	// Shadow Volumes
	ShadowVolume	shadow[MAX_SHADOWVOL];
	int				num_shadowvol;

	vec3			color;
	float			intensity;
	float			attenuation;
	bool			active;
	bool			timer_flag;
	float			lightmap_scale;
	int				timer;
	unsigned int	light_num;
};

#endif
