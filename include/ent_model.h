//=============================================================================
// This file is part of the altEngine distribution
// (https://github.com/akw0088/altEngine/)
// Copyright (c) 2018 Alexander Wright All Rights Reserved.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON - INFRINGEMENT.IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//=============================================================================

#include "include.h"

#ifndef MODEL_H
#define MODEL_H

class EntModel
{
public:
	EntModel(Entity *entity);
	EntModel();
	virtual ~EntModel();
	void load(Graphics &gfx, char *file);
	virtual float *get_matrix(float *matrix);
	void render(Graphics &gfx);
	void clone(EntModel &model);
	void getForwardVector(vec3 &forward);
	virtual void get_frame(Frame &frame);
	void make_aabb();

	//	void calc_frustum(matrix4 &clip);
	//	void create_box(Graphics &gfx, vec3 *aabb);
	//	void render_box(Graphics &gfx);

	Entity			*entity;

	int				num_vertex;
	int				num_index;
	vertex_t		*model_vertex_array;
	unsigned int	*model_index_array;
	char			*model_file;
	char			*index_file;
	int				model_vertex;
	int				model_index;
	int				model_tex;
	int				normal_tex;

	//quaternion		orientation;
	matrix3			morientation;
	vec3			aabb[8];
	vec3			center;

//	char name[128];

	struct flag
	{
		unsigned char blend : 1,
			cull_none : 1,
			rail_trail : 1,
			lightning_trail : 1,
			pad : 4;
	} flags;

};

#endif
