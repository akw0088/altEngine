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


#include "vector.h"
#include "quaternion.h"

#pragma once

typedef struct
{
	int fstart;
	int flength;
	int fstart_end;
	int flength_end;
	int animation_frame;
	bool done;
} animation_state_t;

typedef struct
{
	vec3	position;		// (x, y, z) position. 
	vec2	texCoord0;	// (u, v) texture coordinate
	vec2	texCoord1;	// (u, v) lightmap coordinate
	vec3	normal;		// (x, y, z) normal vector
	int		color;		// RGBA color for the vertex 
	vec4	tangent;
} vertex_t;

struct md5_joint_t
{
	char		name[256];
	int		parent;

	vec3		pos;
	quaternion	orient;
};

struct md5_vertex_t
{
	float u;
	float v;

	int start;
	int count;
};

struct md5_triangle_t
{
	int a;
	int b;
	int c;
};

struct md5_weight_t
{
	int	joint;
	float	bias;

	vec3	pos;
};

typedef struct
{
	vec3 min;
	vec3 max;
} md5_aabb_t;


struct md5_mesh_t
{
	md5_vertex_t	*vertex;
	md5_triangle_t	*triangle;
	md5_weight_t	*weight;

	int num_vertex;
	int num_tri;
	int num_weight;

	char shader[256];
};

struct md5_model_t
{
	char name[128];
	struct md5_joint_t *joint;
	struct md5_mesh_t *mesh;

	int num_joint;
	int num_mesh;
};

typedef struct
{
	vec3 pos;
	quaternion orient;
} md5_base_t;


typedef struct
{
	char name[64];
	int parent;
	int flag;
	int start;
} md5_hierarchy_t;

struct md5_anim_t
{
	int num_frame;
	int num_joint;
	int frame_rate;
	int num_ani;

	md5_base_t *base;
	md5_aabb_t *aabb;
	md5_hierarchy_t *hierarchy;
	float *frame;
};

typedef struct anim_list_s
{
	char name[256];
	struct md5_anim_t *anim;
	struct anim_list_s *next;
} anim_list_t;

typedef struct
{
	md5_joint_t	**frame;
	int			**frame_index;
	int			**count_index;
	int			**frame_vertex;
	int			**count_vertex;
} md5_buffer_t;