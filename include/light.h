#include "include.h"

class ShadowMap;
class Global;

#ifndef LIGHT_H
#define LIGHT_H

class Light
{
public:
	Light(Entity *entity, Graphics &gfx, int num);
	void render_shadows();
	void generate_volumes(Bsp &map);
	void extrude(vec3 position);
	void extend(vec3 position);

//	void render_shadowmap(Graphics &gfx, int shadow_res, Bsp &bsp, ShadowMap &shadowmap);
	void render_shadowmap(Graphics &gfx, int shadow_res, Bsp &bsp, Global &shadowmap);

	// Generate matrices for point light shadow map/cubemap
	static void mat_forward(matrix4 &mvp, vec3 &position);
	static void mat_right(matrix4 &mvp, vec3 &position);
	static void mat_backward(matrix4 &mvp, vec3 &position);
	static void mat_left(matrix4 &mvp, vec3 &position);
	static void mat_top(matrix4 &mvp, vec3 &position);
	static void mat_bottom(matrix4 &mvp, vec3 &position);
	static void mat_cube(float *cube, vec3 &position);


	Entity	*entity;
	unsigned int texObjCube[6];
	vec3	color;
	int		intensity;
	bool	active;
	int		light_num;
	vector<shadowvol_t>	shadow_list;
	Edge	edge_list;
};

#endif
