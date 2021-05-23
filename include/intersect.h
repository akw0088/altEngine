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

// So this code is all from Game Physics Cookbook, not too sure if it is copyrighted as it
// is a book intended for learning, mostly intersection code

#ifndef PHYSICS_H
#define PHYSICS_H
#include "include.h"

#define RIGIDBODY_TYPE_BASE  0
#define RIGIDBODY_TYPE_PARTICLE 1
#define RIGIDBODY_TYPE_SPHERE 2
#define RIGIDBODY_TYPE_BOX  3


namespace intersect {

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
		std::vector<intersect::CModel*> models;

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
	} COctreeNode;

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

};

bool Raycast(const plane_t &plane, const intersect::ray_t &ray, intersect::raycast_result_t *result);
bool Raycast(const intersect::aabb_t &aabb, const intersect::ray_t &ray, intersect::raycast_result_t *result);
bool PointInAABB(const vec3 &point, const intersect::aabb_t &aabb);
bool PointOnPlane(const vec3 &point, vec3 &normal, float d);
float DistPointPlane(const vec3 &q, const vec3 &normal, const float d);
float Signed2DTriArea(const vec3 &a, const vec3 &b, const vec3 &c);
#endif
