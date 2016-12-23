#include "include.h"

#ifndef MODEL_H
#define MODEL_H

class Model
{
public:
	Model(Entity *entity);
	virtual ~Model();
	static void CreateObjects(Graphics &gfx);
	void calc_frustum(matrix4 &clip);
	void load(Graphics &gfx, char *file);
	void create_box(Graphics &gfx, vec3 *aabb);
	float *get_matrix(float *matrix);
	void render(Graphics &gfx);
	void render_box(Graphics &gfx);
	void clone(Model &model);
	void getForwardVector(vec3 &forward);
	void make_aabb();

	Entity			*entity;

	int				num_vertex;
	int				num_index;
	vertex_t		*model_array;
	char			*model_file;
	int				model_vertex;
	int				model_index;
	int				model_tex;
	int				normal_tex;

	//quaternion		orientation;
	matrix3			morientation;
	vec3			aabb[8];
	int				box_vao;
	int				box_vertex;
	int				box_index;
	vec3			center;
	vec4			frustum[6];

	static			int quad_vertex;
	static			int quad_index;

	static			int cube_vertex;
	static			int cube_index;
};

#endif
