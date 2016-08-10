#include "include.h"

class ShadowMap;
class Global;
class mLight2;
class Engine;

#ifndef LIGHT_H
#define LIGHT_H

class Light
{
public:
	Light(Entity *entity, Graphics &gfx, int num);
	void render_shadow_volumes();
	void generate_volumes(Bsp &map);
	void extrude(vec3 position);
	void extend(vec3 position);

	void generate_cubemaps(int width, int height);

	void render_shadowmap(Graphics &gfx, matrix4 projection, Bsp &map, mLight2 &mlight2);
	void select_shadowmap(Graphics &gfx, int face);


	// Generate matrices for point light shadow map/cubemap
	static void mat_forward(matrix4 &mvp, vec3 &position);
	static void mat_right(matrix4 &mvp, vec3 &position);
	static void mat_backward(matrix4 &mvp, vec3 &position);
	static void mat_left(matrix4 &mvp, vec3 &position);
	static void mat_top(matrix4 &mvp, vec3 &position);
	static void mat_bottom(matrix4 &mvp, vec3 &position);
	static void mat_cube(float *cube, vec3 &position);


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
