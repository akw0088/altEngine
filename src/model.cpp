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

#include "model.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include <float.h> // for FLT_MAX

void Model::load(Graphics &gfx, char *file)
{
	char vbo_file[LINE_SIZE];
	char ibo_file[LINE_SIZE];
	char tga_file[LINE_SIZE];
	char normal_file[LINE_SIZE];

	snprintf(vbo_file, LINE_SIZE, "%s.vbo", file);
	snprintf(ibo_file, LINE_SIZE, "%s.ibo", file);
	snprintf(tga_file, LINE_SIZE, "%s.tga", file);
	snprintf(normal_file, LINE_SIZE, "%s_normal.tga", file);

#if 0
	int j = 0;
	for (unsigned int i = 0; i < strlen(file); i++)
	{
		if (file[i] == '/' || file[i] == '\\')
		{
			memset(name, 0, sizeof(name));
			j = 0;
			continue;
		}
		name[j++] = file[i];
	}
#endif

	model_file = get_file(vbo_file, NULL);
	if (model_file == NULL)
	{
		debugf("Unable to load vertex array %s\n", vbo_file);
		index_file = NULL;
		return;
	}

	num_vertex = *((int *)model_file);
	model_vertex_array = (vertex_t *)(model_file + 4);

//	create_box(gfx, aabb);
	make_aabb();

	index_file = get_file(ibo_file, NULL);
	if (index_file == NULL)
	{
		debugf("Unable to load index array %s\n", index_file);
		delete [] model_file;
		model_file = NULL;
		return;
	}

	num_index = *((int *)index_file);
	model_index_array = (unsigned int *)(index_file + 4);

	model_vertex = gfx.CreateVertexBuffer(model_vertex_array, num_vertex);
	model_index = gfx.CreateIndexBuffer(model_index_array, num_index);
//	delete [] index_file;
//	index_file = NULL;
//	delete [] model_file;
//	model_file = NULL;

	model_tex = load_texture(gfx, tga_file, false, false, 0);
	if (model_tex == 0)
	{
		char *pdata = strstr(tga_file, ".tga");

		memcpy(pdata, ".jpg", 4);
		model_tex = load_texture(gfx, tga_file, false, false, 0);
		if (model_tex == 0)
		{
			char *pdata = strstr(tga_file, ".jpg");

			memcpy(pdata, ".png", 4);
			model_tex = load_texture(gfx, tga_file, false, false, 0);
		}
	}

	normal_tex = load_texture(gfx, normal_file, false, false, 0);

	if (entity && entity->rigid)
	{
		entity->rigid->recalc();
	}
}

Model::Model(Entity *entity)
{
	Model::entity = entity;


	num_vertex = 0;
	num_index = 0;
	normal_tex = 0;
	model_vertex = 0;
	model_index = 0;
	model_tex = 0;
	model_file = NULL;
	index_file = NULL;
	model_index_array = NULL;
	model_vertex_array = NULL;
	center = vec3();
	flags.rail_trail = false;
	flags.lightning_trail = false;
	flags.blend = false;
	flags.cull_none = false;

	morientation.m[0] = 1.0f;
	morientation.m[1] = 0.0f;
	morientation.m[2] = 0.0f;

	morientation.m[3] = 0.0f;
	morientation.m[4] = 1.0f;
	morientation.m[5] = 0.0f;

	morientation.m[6] = 0.0f;
	morientation.m[7] = 0.0f;
	morientation.m[8] = 1.0f;
}

// copies visual data from model
void Model::clone(Model &model)
{
	for(int i = 0; i < 8; i++)
	{
		aabb[i] = model.aabb[i];
	}
	center = model.center;
	model_index_array = model.model_index_array;
	model_vertex_array = model.model_vertex_array;
	model_index = model.model_index;
	model_tex = model.model_tex;
	model_vertex = model.model_vertex;
	num_index = model.num_index;
	num_vertex = model.num_vertex;

	//hacks
	if (entity && entity->rigid)
	{
		if (model.entity)
		{
			entity->rigid->flags.gravity = model.entity->rigid->flags.gravity;
			entity->rigid->angular_velocity = model.entity->rigid->angular_velocity;
		}
	}
}

void Model::make_aabb()
{
	aabb[0] = vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	aabb[7] = vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	vec3 sum = vec3();
	for (int i = 0; i < num_vertex; i++)
	{
		sum += model_vertex_array[i].position;

		if (model_vertex_array[i].position.x < aabb[0].x)
			aabb[0].x = model_vertex_array[i].position.x;
		else if (model_vertex_array[i].position.y < aabb[0].y)
			aabb[0].y = model_vertex_array[i].position.y;
		else if (model_vertex_array[i].position.z < aabb[0].z)
			aabb[0].z = model_vertex_array[i].position.z;

		if (model_vertex_array[i].position.x > aabb[7].x)
			aabb[7].x = model_vertex_array[i].position.x;
		else if (model_vertex_array[i].position.y > aabb[7].y)
			aabb[7].y = model_vertex_array[i].position.y;
		else if (model_vertex_array[i].position.z > aabb[7].z)
			aabb[7].z = model_vertex_array[i].position.z;
	}
	center = sum / (float)num_vertex;
	// binary order
	aabb[1] = vec3(aabb[0].x, aabb[0].y, aabb[7].z);
	aabb[2] = vec3(aabb[0].x, aabb[7].y, aabb[0].z);
	aabb[3] = vec3(aabb[0].x, aabb[7].y, aabb[7].z);
	aabb[4] = vec3(aabb[7].x, aabb[0].y, aabb[0].z);
	aabb[5] = vec3(aabb[7].x, aabb[0].y, aabb[7].z);
	aabb[6] = vec3(aabb[7].x, aabb[7].y, aabb[0].z);
}


void Model::render(Graphics &gfx)
{
	gfx.SelectIndexBuffer(model_index);
	gfx.SelectVertexBuffer(model_vertex);
	if (flags.blend)
	{
		gfx.Blend(true);
		gfx.BlendFuncOneOne();
	}

	gfx.SelectTexture(0, model_tex);
//	gfx.SelectTexture(2, normal_tex);
	gfx.DrawArrayTri(0, 0, num_index, num_vertex);
	if (flags.blend)
	{
		gfx.Blend(false);
	}

}

#if 0
void Model::render_box(Graphics &gfx)
{
	gfx.SelectIndexBuffer(box_index);
	gfx.SelectVertexBuffer(box_vertex);
	gfx.SelectTexture(0, model_tex);
	gfx.DrawArrayTri(0, 0, 24, 24);
}
#endif

float *Model::get_matrix(float *matrix)
{
	matrix[0] = morientation.m[0];
	matrix[1] = morientation.m[1];
	matrix[2] = morientation.m[2];
	matrix[3] = 0.0f;

	matrix[4] = morientation.m[3];
	matrix[5] = morientation.m[4];
	matrix[6] = morientation.m[5];
	matrix[7] = 0.0f;

	matrix[8]  = morientation.m[6];
	matrix[9]  = morientation.m[7];
	matrix[10] = morientation.m[8];
	matrix[11] = 0.0f;

	/* matrix rotates around center, but position is arbitrary point
	from which verts are defined */
	vec3 offset = morientation * center;

	matrix[12] = entity->position.x - offset.x;
	matrix[13] = entity->position.y - offset.y;
	matrix[14] = entity->position.z - offset.z;
	matrix[15] = 1.0f;
	return matrix;
}


Model::~Model()
{
	if (index_file != NULL)
		delete[] index_file;
	index_file = NULL;


	if (model_file != NULL)
		delete [] model_file;
	model_file = NULL;
}


#if 0
void Model::calc_frustum(matrix4 &clip)
{
//	matrix4	clip;

//	clip = transformation * projection;

	// This will extract the RIGHT side of the frustum
	frustum[0].x = clip.m[ 3] - clip.m[ 0];
	frustum[0].y = clip.m[ 7] - clip.m[ 4];
	frustum[0].z = clip.m[11] - clip.m[ 8];
	frustum[0].w = clip.m[15] - clip.m[12];
	frustum[0].normalize();

	// This will extract the LEFT side of the frustum
	frustum[1].x = clip.m[ 3] + clip.m[ 0];
	frustum[1].y = clip.m[ 7] + clip.m[ 4];
	frustum[1].z = clip.m[11] + clip.m[ 8];
	frustum[1].w = clip.m[15] + clip.m[12];
	frustum[1].normalize();

	// This will extract the BOTTOM side of the frustum
	frustum[2].x = clip.m[ 3] + clip.m[ 1];
	frustum[2].y = clip.m[ 7] + clip.m[ 5];
	frustum[2].z = clip.m[11] + clip.m[ 9];
	frustum[2].w = clip.m[15] + clip.m[13];
	frustum[2].normalize();

	// This will extract the TOP side of the frustum
	frustum[3].x = clip.m[ 3] - clip.m[ 1];
	frustum[3].y = clip.m[ 7] - clip.m[ 5];
	frustum[3].z = clip.m[11] - clip.m[ 9];
	frustum[3].w = clip.m[15] - clip.m[13];
	frustum[3].normalize();

	// This will extract the BACK side of the frustum
	frustum[4].x = clip.m[ 3] - clip.m[ 2];
	frustum[4].y = clip.m[ 7] - clip.m[ 6];
	frustum[4].z = clip.m[11] - clip.m[10];
	frustum[4].w = clip.m[15] - clip.m[14];
	frustum[4].normalize();

	// This will extract the FRONT side of the frustum
	frustum[5].x = clip.m[ 3] + clip.m[ 2];
	frustum[5].y = clip.m[ 7] + clip.m[ 6];
	frustum[5].z = clip.m[11] + clip.m[10];
	frustum[5].w = clip.m[15] + clip.m[14];
	frustum[5].normalize();
}
#endif
/*
bool Model::in_frustum(Global &global, vec3 &position, matrix4 &transformation, matrix4 &projection)
{
	((Plane)left_plane).draw_plane(global, transformation, projection);
	((Plane)right_plane).draw_plane(global,  transformation, projection);

	((Plane)top_plane).draw_plane(global,  transformation, projection);
	((Plane)bottom_plane).draw_plane(global,  transformation, projection);

	((Plane)near_plane).draw_plane(global,  transformation, projection);
	((Plane)far_plane).draw_plane(global,  transformation, projection);

	float result = (vec3)left_plane * position - left_plane.w;

	if (result > 0)
	{
		result = (vec3)right_plane * position - right_plane.w;
		if (result > 0)
		{
			result = (vec3)top_plane * position - top_plane.w;
			if (result > 0)
			{
				result = (vec3)bottom_plane * position - bottom_plane.w;
				if (result > 0)
				{
					result = (vec3)near_plane * position - near_plane.w;
					if (result > 0)
					{
						result = (vec3)far_plane * position - far_plane.w;
						if (result > 0)
						{
							return true;
						}
					}
				}
			}
		}
	}

	// skipping farplane due to infinite projection
//	if ((vec3)left_plane * position - left_plane.w > 0 && 
//	(vec3)right_plane * position - right_plane.w > 0 &&
//	(vec3)top_plane * position - top_plane.w > 0 &&
//	(vec3)bottom_plane * position - bottom_plane.w > 0 &&
//	(vec3)near_plane * position - near_plane.w > 0)
//	{
//		return true;
//	}
//	else
//	{
//		return false;
//	}
	return false;
}
*/

#if 0
void Model::create_box(Graphics &gfx, vec3 *box)
{
	vertex_t	vert[8];

	memset(&vert, 0, sizeof(vert));
	for(int i = 0; i < 8; i++)
		vert[i].position = box[i];
	int	index[24] = {	0, 1,
						0, 2,
						1, 3,
						2, 3,
						4, 5,
						4, 6,
						5, 7,
						6, 7,
						0, 4,
						1, 5,
						2, 6,
						3, 7};

	box_vertex = gfx.CreateVertexBuffer(vert, 8);
	box_index = gfx.CreateIndexBuffer(index, 24);
}
#endif


void Model::getForwardVector(vec3 &forward)
{
	forward.x = entity->model->morientation.m[6];
	forward.y = entity->model->morientation.m[7];
	forward.z = entity->model->morientation.m[8];
}

void Model::get_frame(Frame &frame)
{
	frame.up.x = morientation.m[3];
	frame.up.y = morientation.m[4];
	frame.up.z = morientation.m[5];
	frame.forward.x = -morientation.m[6];
	frame.forward.y = -morientation.m[7];
	frame.forward.z = -morientation.m[8];
	frame.pos = entity->position;
}

Model::Model()
{
	entity = NULL;
	num_vertex = 0;
	num_index = 0;
	normal_tex = 0;
	model_vertex = 0;
	model_index = 0;
	model_tex = 0;
	model_file = NULL;
	index_file = NULL;
	model_vertex_array = NULL;
	model_index_array = NULL;
	center = vec3();
	flags.rail_trail = false;
	flags.lightning_trail = false;
	flags.blend = false;


	morientation.m[0] = 1.0f;
	morientation.m[1] = 0.0f;
	morientation.m[2] = 0.0f;

	morientation.m[3] = 0.0f;
	morientation.m[4] = 1.0f;
	morientation.m[5] = 0.0f;

	morientation.m[6] = 0.0f;
	morientation.m[7] = 0.0f;
	morientation.m[8] = 1.0f;
}

