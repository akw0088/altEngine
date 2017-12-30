#ifndef RAY_H
#define RAY_H
#include "include.h"

#define RIGIDBODY_TYPE_BASE  0
#define RIGIDBODY_TYPE_PARTICLE 1
#define RIGIDBODY_TYPE_SPHERE 2
#define RIGIDBODY_TYPE_BOX  3


typedef struct
{
	vec2 a;
	vec2 b;
} line2_t;

typedef struct
{
	vec3 a;
	vec3 b;
} line3_t;


typedef struct
{
	vec2 origin;
	float radius;
} circle_t;

typedef struct
{
	vec3 origin;
	float radius;
} sphere_t;


typedef struct
{
	vec2 max;
	vec2 min;
} rect2_t;

typedef struct
{
	vec2 origin;
	float half_length;
	float angle;
} box2_t;

typedef struct
{
	vec3 origin;
	float half_length;
	float angle;
} box3_t;

typedef struct
{
	vec3 min;
	vec3 max;
} aabb_t;

typedef struct
{
	vec3 origin;
	vec3 size;
	matrix3 orientation;
} obb_t;

typedef struct
{
	vec3 origin;
	vec3 dir;
} ray_t;

typedef struct
{
	vec3 point;
	vec3 normal;
	float t;
	bool hit;
} raycast_result_t;

typedef struct
{
	vec3 a, b, c;
} triangle_t;

typedef struct
{
	float min;
	float max;
} interval_t;




typedef struct bvh_node_s
{
	aabb_t bounds;
	struct bvh_node_s *children;
	int numTriangles;
	int *triangles;
} bvh_node_t;

typedef struct
{
	int numTriangles;
	union
	{
		triangle_t *triangles;//size = numTriangles
		vec3 *vertices; //size = numTriangles * 3
		float *values; //size = numTriangles * 3 * 3
	};
	bvh_node_t *accelerator;
} mesh_t;

class CModel;

typedef struct OctreeNode
{
	aabb_t bounds;
	OctreeNode* children;
	std::vector<CModel*> models;

	inline OctreeNode()
	{
		children = NULL;
	}

	inline ~OctreeNode()
	{
		if (children != 0)
		{
			delete[] children;
		}
	}
} OctreeNode;

typedef struct Frustum
{
	union {
		struct {
			plane_t top;
			plane_t bottom;
			plane_t left;
			plane_t right;
			plane_t znear;
			plane_t zfar;
		} p;
		plane_t planes[6];
	};
	inline Frustum()
	{
	}
} Frustum;


#endif
