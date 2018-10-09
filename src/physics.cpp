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

#include "physics.h"
#include <list>
#include <algorithm>
#include "sin_table.h"
#include <math.h> // for tanf

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace physics;

// Floating point comparison function (epsilon issues)
#define CMP(x, y) \
	(abs32(x - y) <= FLT_EPSILON * MAX(1.0f, MAX(abs32(x), abs32(y))))


//=============================================================================
//	2D point tests (need to complete)
//=============================================================================

bool PointOnLine(const vec3 &p, const line2_t &line)
{
	float dy = (line.b.y - line.a.y);
	float dx = (line.b.x - line.a.x);
	float m = dy / dx; 
	float B = line.a.y - m * line.a.x;

	return CMP(p.y, (m * p.x + B));
}

bool PointInCircle(const vec2 &point, const circle_t &c)
{
	line2_t line;
	
	line.a = point;
	line.b = c.origin;
	vec2 length = line.a - line.b;

	if ((length * length) < (c.radius * c.radius))
	{
		return true;
	}
	return false;
}

bool PointInRectangle(const vec2 &point, const rect2_t  &rectangle)
{
	vec2 min = rectangle.min;
	vec2 max = rectangle.max;

	return min.x <= point.x && min.y <= point.y && point.x <= max.x && point.y <= max.y;
}

/*
bool PointInOrientedRectangle(const vec2 &point, const box2_t &rectangle)
{
	vec2 rotVector = point - rectangle.origin;
	float theta = -(rectangle.angle);
	float zRotation2x2[] = {
		fcos(theta), fsin(theta),
		-fsin(theta), fcos(theta)
	};

	//Multiply(rotVector.asArray, vec2(rotVector.x, rotVector.y).asArray, 1, 2, zRotation2x2, 2, 2);

	rect2_t local;
	
	local.min = vec2();
	//local.max = rectangle.half_length * 2.0f;

//	vec2 localPoint = rotVector + rectangle.half_length;
	//return PointInRectangle(localPoint, local);
	return false;
}
*/

//=============================================================================
//	3D Point Tests
//=============================================================================
bool PointInSphere( vec3 &point, sphere_t &sphere)
{
	vec3 dist = point - sphere.origin;

	if (dist.magnitudeSq() < sphere.radius * sphere.radius)
	{
		return true;
	}

	return false;
}


vec3 ClosestPoint(const sphere_t &sphere, const vec3 &point)
{
	vec3 dist = point - sphere.origin;
	return dist.normalize() * sphere.radius + sphere.origin;
}



bool PointInAABB(const vec3 &point, const aabb_t &aabb)
{
	if (point.x < aabb.min.x || point.y < aabb.min.y || point.z < aabb.min.z)
	{
		return false;
	}

	if (point.x > aabb.max.x || point.y > aabb.max.y || point.z > aabb.max.z)
	{
		return false;
	}

	return true;
}

vec3 ClosestPoint(const aabb_t &aabb, const vec3 &point)
{
	vec3 result = point;

	result.x = (result.x < aabb.min.x) ? aabb.min.x : result.x;
	result.y = (result.y < aabb.min.x) ? aabb.min.y : result.y; 
	result.z = (result.z < aabb.min.x) ? aabb.min.z : result.z;

	result.x = (result.x > aabb.max.x) ? aabb.max.x : result.x; 
	result.y = (result.y > aabb.max.x) ? aabb.max.y : result.y;   
	result.z = (result.z > aabb.max.x) ? aabb.max.z : result.z;

	return result;
}

bool PointInOBB(const vec3 &point, const obb_t &obb)
{
	vec3 dir = point - obb.origin;

	vec3 right(obb.orientation.m[0], obb.orientation.m[1], obb.orientation.m[2]);
	vec3 up(obb.orientation.m[3], obb.orientation.m[4], obb.orientation.m[5]);
	vec3 forward(obb.orientation.m[6], obb.orientation.m[7], obb.orientation.m[8]);

	float x = dir * right;
	float y = dir * up;
	float z = dir * forward;

	if (x > obb.size.x || x < -obb.size.x)
	{
		return false;
	}

	if (y > obb.size.y || y < -obb.size.y)
	{
		return false;
	}

	if (z > obb.size.z|| z < -obb.size.z)
	{
		return false;
	}

	return true;
}

vec3 ClosestPoint(const obb_t &obb, const vec3 &point)
{
	vec3 result = obb.origin;
	vec3 dir = point - obb.origin;

	vec3 right(obb.orientation.m[0], obb.orientation.m[1], obb.orientation.m[2]);
	vec3 up(obb.orientation.m[3], obb.orientation.m[4], obb.orientation.m[5]);
	vec3 forward(obb.orientation.m[6], obb.orientation.m[7], obb.orientation.m[8]);

	float x = dir * right;
	float y = dir * up;
	float z = dir * forward;

	if (x > obb.size.x)
	{
		x = obb.size.x;
	}

	if (x < -obb.size.x)
	{
		x = -obb.size.x;
	}

	result = result + (right * x);

	if (y > obb.size.y)
	{
		y = obb.size.y;
	}

	if (y < -obb.size.y)
	{
		y = -obb.size.y;
	}

	result = result +  (up * y);

	if (z > obb.size.z)
	{
		z = obb.size.z;
	}

	if (z < -obb.size.z)
	{
		z = -obb.size.z;
	}

	result = result + (forward * z);

	return result;
}


bool PointOnPlane(const vec3 &point, vec3 &normal, float d)
{
	float dot = point * normal;
	CMP(dot - d, 0.0f);
	return dot - d == 0.0f; // TODO: use epsilon
}

vec3 ClosestPoint(const vec3 &point, const plane_t &p)
{
	float dot = point * p.normal;
	float distance = dot - p.d;
	return point - p.normal * distance;
}


vec3 ClosestPoint(const line3_t &line, const vec3 &point)
{
	vec3 lVec = line.b - line.a;
	vec3 dist = point - line.a;

	float t = dist * lVec / (lVec * lVec);
	t = MAX(t, 0.0f);
	t = MIN(t, 1.0f);
	return line.a + lVec * t;
}


bool PointOnLine(const vec3 &point, const line3_t &line)
{
	vec3 closest = ClosestPoint(line, point);
	float distanceSq = (closest - point).magnitudeSq();
	return CMP(distanceSq, 0.0f);
}

bool PointOnRay(const vec3 &point, const ray_t &ray)
{
	if (point == ray.origin)
	{
		return true;
	}

	vec3 norm = point - ray.origin;

	float diff = norm.normalize() * ray.dir;

	// If BOTH vectors point in the same direction,
	// their dot product (diff) should be 1
	return diff == 1.0f; // TODO: epsilon
}

vec3 ClosestPoint(const ray_t &ray, const vec3 &point)
{
	vec3 dist = point - ray.origin;
	float t = dist * ray.dir;

	// direction should already be normalized
//	t /= ray.dir * ray.dir;

	t = MAX(t, 0.0f);
	return vec3(ray.origin + ray.dir * t);
}

//=============================================================================
//	3D shape intersections
//=============================================================================

bool SphereSphere(const sphere_t &s1, const sphere_t &s2)
{
	float radiiSum = s1.radius + s2.radius;

	float sqDistance = (s1.origin - s2.origin).magnitudeSq();

	return sqDistance < radiiSum * radiiSum;
}

bool SphereAABB(const sphere_t& sphere, const aabb_t &aabb)
{
	vec3 closestPoint = ClosestPoint(aabb, sphere.origin);
	float distSq = (sphere.origin - closestPoint).magnitudeSq();
	float radiusSq = sphere.radius * sphere.radius;
	return distSq < radiusSq;
}

bool SphereOBB(const sphere_t &sphere, const obb_t &obb)
{
	vec3 closestPoint = ClosestPoint(obb, sphere.origin);
	vec3 dist = sphere.origin - closestPoint;
	float distSq = dist.magnitudeSq();
	float radiusSq = sphere.radius * sphere.radius;
	return distSq<radiusSq;
}

#define PlaneSphere(plane, sphere) SpherePlane(sphere, plane)


bool SpherePlane(const sphere_t &s, const plane_t &plane)
{
	vec3 closestPoint = ClosestPoint(s.origin, plane);
	float distSq = (s.origin - closestPoint).magnitudeSq();
	float radiusSq = s.radius * s.radius;
	return distSq < radiusSq;
}

bool AABB_AABB(const aabb_t &aabb1, const aabb_t &aabb2)
{
	vec3 aMin = aabb1.min;
	vec3 aMax = aabb2.max;
	vec3 bMin = aabb2.min;
	vec3 bMax = aabb2.max;

	return (aMin.x <= bMax.x && aMax.x >= bMin.x) &&
		(aMin.y <= bMax.y && aMax.y >= bMin.y) &&
		(aMin.z <= bMax.z && aMax.z >= bMin.z);
}

interval_t GetInterval(const aabb_t &aabb, const vec3& axis)
{
	vec3 i = aabb.min;
	vec3 a = aabb.max;

	vec3 vertex[8] = {
		vec3(i.x, a.y, a.z),
		vec3(i.x, a.y, i.z),
		vec3(i.x, i.y, a.z),
		vec3(i.x, i.y, i.z),
		vec3(a.x, a.y, a.z),
		vec3(a.x, a.y, i.z),
		vec3(a.x, i.y, a.z),
		vec3(a.x, i.y, i.z)
	};

	interval_t result;
	result.min = result.max = axis * vertex[0];

	for (unsigned int i = 1; i < 8; ++i)
	{
		float projection = axis * vertex[i];
		result.min = (projection < result.min) ? projection : result.min;
		result.max = (projection > result.max) ? projection : result.max;
	}
	return result;
}

#define AABBTriangle(a, t) TriangleAABB(t, a) 

interval_t GetInterval(const triangle_t & triangle, const vec3& axis)
{
	interval_t result;

	result.min = axis * triangle.a;
	result.max = result.min;

	float value = axis * triangle.b;
	result.min = MIN(result.min, value);
	result.max = MAX(result.max, value);

	value = axis * triangle.c;
	result.min = MIN(result.min, value);
	result.max = MAX(result.max, value);

	return result;
}

interval_t GetInterval(const obb_t &obb, const vec3& axis)
{
	vec3 vertex[8];

	vec3 C = obb.origin;    // OBB Center        
	vec3 E = obb.size;    // OBB Extents    
	const float *o = obb.orientation.m;
	vec3 A[] = {              // OBB Axis        
		vec3(o[0], o[1], o[2]),
		vec3(o[3], o[4], o[5]),
		vec3(o[6], o[7], o[8]),
	};

	vertex[0] = C + A[0] * E.x + A[1] * E.y + A[2] * E.z;
	vertex[1] = C - A[0] * E.x + A[1] * E.y + A[2] * E.z;
	vertex[2] = C + A[0] * E.x - A[1] * E.y + A[2] * E.z;
	vertex[3] = C + A[0] * E.x + A[1] * E.y - A[2] * E.z;
	vertex[4] = C - A[0] * E.x - A[1] * E.y - A[2] * E.z;
	vertex[5] = C + A[0] * E.x - A[1] * E.y - A[2] * E.z;
	vertex[6] = C - A[0] * E.x + A[1] * E.y - A[2] * E.z;
	vertex[7] = C - A[0] * E.x - A[1] * E.y + A[2] * E.z;

	interval_t result;
	result.min = result.max = axis * vertex[0];
	for (unsigned int i = 1; i < 8; ++i)
	{
		float projection = axis * vertex[i];
		result.min = (projection < result.min) ? projection : result.min;
		result.max = (projection > result.max) ? projection : result.max;
	}
	return result;
}


bool OverlapOnAxis(const aabb_t &aabb, const obb_t &obb, const vec3& axis)
{
	interval_t a = GetInterval(aabb, axis);
	interval_t b = GetInterval(obb, axis);
	return ((b.min <= a.max) && (a.min <= b.max));
}

bool AABB_OBB(const aabb_t &aabb, const obb_t &obb)
{
	const float* o = obb.orientation.m;

	vec3 test[15] = {
		vec3(1, 0, 0), // AABB axis 1
		vec3(0, 1, 0), // AABB axis 2
		vec3(0, 0, 1), // AABB axis 3
		vec3(o[0], o[1], o[2]), // OBB axis 1
		vec3(o[3], o[4], o[5]), // OBB axis 2
		vec3(o[6], o[7], o[8]) // OBB axis 3
							   // We will fill out the remaining axis in the next step
	};

	for (unsigned int i = 0; i < 3; ++i) { // Fill out rest of axis
		test[6 + i * 3 + 0] = vec3::crossproduct(test[i], test[0]);
		test[6 + i * 3 + 1] = vec3::crossproduct(test[i], test[1]);
		test[6 + i * 3 + 2] = vec3::crossproduct(test[i], test[2]);
	}

	for (unsigned int i = 0; i < 15; ++i)
	{
		if (!OverlapOnAxis(aabb, obb, test[i]))
		{
			return false; // Seperating axis found
		}
	}
	return true; // Seperating axis not found
}

#define PlaneAABB(plane, aabb) AABBPlane(aabb, plane)

bool AABBPlane(const aabb_t &aabb, const plane_t &plane)
{
	vec3 position = vec3(
		(aabb.max.x - aabb.min.x) / 2.0f,
		(aabb.max.y - aabb.min.y) / 2.0f,
		(aabb.max.z - aabb.min.z) / 2.0f);

	float pLen = 
		(aabb.max.x - aabb.min.x) * abs32(plane.normal.x) +
		(aabb.max.y - aabb.min.y) * abs32(plane.normal.y) +
		(aabb.max.z - aabb.min.z) * abs32(plane.normal.z);

	float dot = plane.normal * position;
	float dist = dot - plane.d;

	return abs32(dist) <= pLen;
}

bool OverlapOnAxis(const obb_t &obb1, const obb_t &obb2, const vec3& axis)
{
	interval_t a = GetInterval(obb1, axis);
	interval_t b = GetInterval(obb1, axis);
	return ((b.min <= a.max) && (a.min <= b.max));
}

bool OBB_OBB(const obb_t &obb1, const obb_t &obb2)
{
	const float* o1 = obb1.orientation.m;
	const float* o2 = obb2.orientation.m;
	vec3 test[15] = {
		vec3(o1[0], o1[1], o1[2]),
		vec3(o1[3], o1[4], o1[5]),
		vec3(o1[6], o1[7], o1[8]),
		vec3(o2[0], o2[1], o2[2]),
		vec3(o2[3], o2[4], o2[5]),
		vec3(o2[6], o2[7], o2[8])
	};

	for (unsigned int i = 0; i < 3; ++i)
	{
		// Fill out rest of axis
		test[6 + i * 3 + 0] = vec3::crossproduct(test[i], test[0]);
		test[6 + i * 3 + 1] = vec3::crossproduct(test[i], test[1]);
		test[6 + i * 3 + 2] = vec3::crossproduct(test[i], test[2]);
	}


	for (unsigned int i = 0; i < 15; ++i)
	{
		if (!OverlapOnAxis(obb1, obb2, test[i]))
		{
			return false; // Seperating axis found
		}
	}
	return true; // Seperating axis not found
}

#define PlaneOBB(plane, obb) OBBPlane(obb, plane)

bool OBBPlane(const obb_t &obb, const plane_t &plane)
{
	// Local variables for readability only
	const float* o = obb.orientation.m;
	vec3 rot[] = {
		// rotation / orientation
		vec3(o[0], o[1], o[2]),
		vec3(o[3], o[4], o[5]),
		vec3(o[6], o[7], o[8]),
	};
	vec3 normal = plane.normal;

	float pLen = 
		obb.size.x * abs32(normal * rot[0]) +
		obb.size.y * abs32(normal * rot[1]) +
		obb.size.z * abs32(normal * rot[2]);

	float dot = plane.normal * obb.origin;
	float dist = dot - plane.d;

	return abs32(dist) <= pLen;
}

bool PlanePlane(const plane_t &plane1, const plane_t &plane2)
{
	vec3 d = vec3::crossproduct(plane1.normal, plane2.normal);

	return abs32(d * d) >= 0.00001f; // Consider using an epsilon!
}


//=============================================================================
//	3D Line Intersections
//=============================================================================
void RaycastClear(raycast_result_t* outResult)
{
	if (outResult != 0)
	{
		outResult->t = -1;
		outResult->hit = false;
		outResult->normal = vec3(0, 0, 1);
		outResult->point = vec3(0, 0, 0);
	}
}

bool Raycast(const sphere_t &sphere, const ray_t &ray, raycast_result_t *result)
{
	vec3 e = sphere.origin - ray.origin;

	RaycastClear(result);
	float rSq = sphere.radius * sphere.radius;
	float eSq = e.magnitudeSq();

	// ray.direction is assumed to be normalized
	float a = e * ray.dir;

	float bSq = eSq - (a * a);
	float f = newtonSqrt(rSq - bSq);

	float t = a - f; // Assume normal intersection!

	// No collision has happened
	if (rSq - (eSq - (a * a)) < 0.0f)
	{
		return false; // -1 is invalid.   
	}
	// Ray starts inside the sphere  
	else if (eSq<rSq) 
	{
		t = a + f; 
		// Just reverse direction   
	}   // else Normal intersection   

	if (result != 0)
	{
		result->t = t;
		result->hit = true;
		result->point = ray.origin + ray.dir * t;
		result->normal = result->point.normalize() - sphere.origin;
	}

	return true;
}

// really just rayboxslab in common.cpp I think
bool Raycast(const aabb_t &aabb, const ray_t &ray, raycast_result_t *result)
{
	float t[6];


	t[0] = (aabb.min.x - ray.origin.x) / ray.dir.x;
	t[1] = (aabb.max.x - ray.origin.x) / ray.dir.x;
	t[2] = (aabb.min.y - ray.origin.y) / ray.dir.y;
	t[3] = (aabb.max.y - ray.origin.y) / ray.dir.y;
	t[4] = (aabb.min.z - ray.origin.z) / ray.dir.z;
	t[5] = (aabb.max.z - ray.origin.z) / ray.dir.z;
	float t_result;

	//Find the largest minimum value
	float tmin = MAX(MAX(MIN(t[0], t[1]), MIN(t[2], t[3])), MIN(t[4], t[5]));

	//Find the smallest maximum value
	float tmax = MIN(MIN(MAX(t[0], t[1]), MAX(t[2], t[3])), MAX(t[4], t[5]));

	RaycastClear(result);

	if (tmax < 0)
	{
		return false;
	}

	if (tmin > tmax)
	{
		return false;
	}

	if (tmin < 0.0f)
	{
		t_result = tmax;
	}

	t_result = tmin;

	if (t_result < 0.0f)
	{
		t_result = tmax;
	}

	if (result != 0)
	{
		result->t = t_result;
		result->hit = true;
		result->point = ray.origin + ray.dir * t_result;

		vec3 normals[] = {
			vec3(-1, 0, 0), vec3(1, 0, 0),
			vec3(0, -1, 0), vec3(0, 1, 0),
			vec3(0, 0, -1), vec3(0, 0, 1)
		};

		for (unsigned int i = 0; i < 6; ++i)
		{
			if (CMP(t_result, t[i]))
			{
				result->normal = normals[i];
			}
		}
	}
	return true;
}

bool Raycast(const obb_t &obb, const ray_t &ray, raycast_result_t *result)
{
	const float *o = obb.orientation.m;
	float t_result;

	// X, Y and Z axis of OBB
	vec3 X(o[0], o[1], o[2]);
	vec3 Y(o[3], o[4], o[5]);
	vec3 Z(o[6], o[7], o[8]);

	vec3 p = obb.origin - ray.origin;


	vec3 f(X * ray.dir, Y * ray.dir, Z * ray.dir);
	vec3 e(X * p, Y * p, Z * p);

	RaycastClear(result);

	float t[6] = { 0, 0, 0, 0, 0, 0 };

	if (CMP(f.x, 0))
	{
		if (-e.x - obb.size.x > 0 || -e.x + obb.size.x < 0)
		{
			return false;
		}
		f.x = 0.00001f;
		// Avoid div by 0!
	}

	t[0] = (e.x + obb.size.x) / f.x; // min
	t[1] = (e.x - obb.size.x) / f.x; // max

	if (CMP(f.y, 0))
	{
		if (-e.y - obb.size.y > 0 || -e.y + obb.size.y < 0)
		{
			return false;
		}
		f.y = 0.00001f;
		// Avoid div by 0!
	}

	t[2] = (e.y + obb.size.y) / f.y; // min
	t[3] = (e.y - obb.size.y) / f.y; // max


	if (CMP(f.z, 0))
	{
		if (-e.z - obb.size.z > 0 || -e.z + obb.size.z < 0)
		{
			return false;
		}
		f.z = 0.00001f;
		// Avoid div by 0!
	}

	t[4] = (e.z + obb.size.z) / f.z; // min
	t[5] = (e.z - obb.size.z) / f.z; // max

	float tmin = MAX(MAX(MIN(t[0], t[1]), MIN(t[2], t[3])), MIN(t[4], t[5]));
	float tmax = MIN(MIN(MAX(t[0], t[1]), MAX(t[2], t[3])), MAX(t[4], t[5]));

	if (tmax < 0)
	{
		return false;
	}

	if (tmin > tmax)
	{
		return false;
	}

	if (tmin < 0.0f)
	{
		t_result = tmax;
	}

	t_result = tmin;

	if (result != 0)
	{
		result->hit = true;
		result->t = t_result;
		result->point = ray.origin + ray.dir * t_result;
		vec3 normals[] = {
			X, X * -1.0f,
			Y, Y * -1.0f,
			Z, Z * -1.0f
		};
		for (unsigned int i = 0; i < 6; ++i)
		{
			if (CMP(t_result, t[i]))
			{
				result->normal = normals[i].normalize();
			}
		}
	}

	return true;
}

bool Raycast(const plane_t &plane, const ray_t &ray, raycast_result_t *result)
{
	float nd = ray.dir * plane.normal;
	float pn = ray.origin * plane.normal;

	RaycastClear(result);
	if (nd >= 0.0f)
	{
		return false;
	}

	float t = (plane.d - pn) / nd;

	if (t >= 0.0f)
	{
		if (result != 0)
		{
			vec3 norm = plane.normal;
			result->t = t;
			result->hit = true;
			result->point = ray.origin + ray.dir * t;
			result->normal = norm.normalize();
		}
		return true;
	}

	return false;
}


// LineTests are really just a boolean hit tests

bool Linetest(const sphere_t &sphere, const line3_t &line)
{
	vec3 closest = ClosestPoint(line, sphere.origin);
	float distSq = (sphere.origin - closest).magnitudeSq();
	return distSq <= (sphere.radius * sphere.radius);
}

bool Linetest(const aabb_t &aabb, const line3_t & line)
{
	ray_t ray;
	vec3 length = line.b - line.a;
	raycast_result_t result;

	ray.origin = line.a;
	ray.dir = length.normalize();
	if (Raycast(aabb, ray, &result) == false)
	{
		return false;
	}

	float t = result.t;
	return t >= 0 && t * t <= (length * length);
}

bool Linetest(const obb_t &obb, const line3_t &line)
{
	ray_t ray;
	vec3 length = line.b - line.a;
	raycast_result_t result;

	ray.origin = line.a;
	ray.dir = length.normalize();
	if (Raycast(obb, ray, &result) == false)
	{
		return false;
	}

	float t = result.t;
	return t >= 0 && t * t <= (length * length);
}

bool Linetest(const plane_t p, const line3_t &line)
{
	vec3 ab = line.b - line.a;
	float nA = p.normal * line.a;
	float nAB = p.normal * ab;

	// If the line and plane are parallel, nAB will be 0
	// This will cause a divide by 0 exception below
	// If you plan on testing parallel lines and planes
	// it is sage to early out when nAB is 0. 
	float t = (p.d - nA) / nAB;
	return t >= 0.0f && t <= 1.0f;
}

//=============================================================================
//	Triangles and Meshes
//=============================================================================

bool PointInTriangle(const vec3 &p, const triangle_t &t)
{
	vec3 a = t.a - p;
	vec3 b = t.b - p;
	vec3 c = t.c - p;

	// The point should be moved too, so they are both
	// relative, but because we don't use p in the
	// equation anymore, we don't need it!
	// p -= p; This would just equal the zero vector!

	vec3 normPBC = vec3::crossproduct(b, c); 	// Normal of PBC (u)    
	vec3 normPCA = vec3::crossproduct(c, a); 	// Normal of PCA (v)    
	vec3 normPAB = vec3::crossproduct(a, b); 	// Normal of PAB (w

	if (normPBC * normPCA < 0.0f)
	{
		return false;
	}
	else if (normPBC * normPAB < 0.0f)
	{
		return false;
	}

	return true;
}

void FromTriangle(const triangle_t &t, plane_t p)
{
	p.normal = vec3::crossproduct (t.b - t.a, t.c - t.a).normalize();
	p.d = p.normal * t.a;
}

vec3 ClosestPoint(const triangle_t &t, const vec3 &p)
{
	plane_t plane;
	FromTriangle(t, plane);

	vec3 closest = ClosestPoint(p, plane);

	if (PointInTriangle(closest, t))
	{
		return closest;
	}

	line3_t l1, l2, l3;

	l1.a = t.a;
	l1.b = t.b;

	l2.a = t.b;
	l2.b = t.c;

	l3.a = t.c;
	l3.b = t.a;

	vec3 c1 = ClosestPoint(l1, p); // Line AB    
	vec3 c2 = ClosestPoint(l2, p); // Line BC    
	vec3 c3 = ClosestPoint(l3, p); // Line CA

	float magSq1 = (p - c1).magnitudeSq();
	float magSq2 = (p - c2).magnitudeSq();
	float magSq3 = (p - c3).magnitudeSq();

	if (magSq1 < magSq2 && magSq1 < magSq3)
	{
		return c1;
	}
	else if (magSq2 < magSq1 && magSq2 < magSq3)
	{
		return c2;
	}
	return c3;
}

bool OverlapOnAxis(const aabb_t &aabb, const triangle_t &triangle, const vec3& axis)
{
	interval_t a = GetInterval(aabb, axis);
	interval_t b = GetInterval(triangle, axis);
	return ((b.min <= a.max) && (a.min <= b.max));
}

bool TriangleSphere(const triangle_t &t, const sphere_t &s)
{
	vec3 closest = ClosestPoint(t, s.origin);
	float magSq = (closest - s.origin).magnitudeSq();
	return magSq <= s.radius * s.radius;
}

bool TriangleAABB(const triangle_t &t, const aabb_t &a)
{
	vec3 f0 = t.b - t.a;
	vec3 f1 = t.c - t.b;
	vec3 f2 = t.a - t.c;

	vec3 u0(1.0f, 0.0f, 0.0f);
	vec3 u1(0.0f, 1.0f, 0.0f);
	vec3 u2(0.0f, 0.0f, 1.0f);

	vec3 test[13] = {
		u0, // AABB Axis 1
		u1, // AABB Axis 2
		u2, // AABB Axis 3
		vec3::crossproduct(f0, f1),
		vec3::crossproduct(u0, f0),
		vec3::crossproduct(u0, f1),
		vec3::crossproduct(u0, f2),
		vec3::crossproduct(u1, f0),
		vec3::crossproduct(u1, f1),
		vec3::crossproduct(u1, f2),
		vec3::crossproduct(u2, f0),
		vec3::crossproduct(u2, f1),
		vec3::crossproduct(u2, f2)
	};
	for (unsigned int i = 0; i < 13; ++i)
	{
		if (!OverlapOnAxis(a, t, test[i]))
		{
			return false; // Separating axis found
		}
	}
	return true; // Separating axis not found
}

#define OBBTriangle(o, t) TriangleOBB(t, o)

bool OverlapOnAxis(const obb_t &obb, const triangle_t &triangle, const vec3& axis)
{
	interval_t a = GetInterval(obb, axis);
	interval_t b = GetInterval(triangle, axis);
	return ((b.min <= a.max) && (a.min <= b.max));
}

bool TriangleOBB(const triangle_t &t, const obb_t &o)
{
	// Compute the edge vectors of the triangle  (ABC)    
	vec3 f0 = t.b - t.a;    
	vec3 f1 = t.c - t.b;    
	vec3 f2 = t.a - t.c; 

	const float *orientation = o.orientation.m;
	vec3 u0(orientation[0], orientation[1], orientation[2]);
	vec3 u1(orientation[3], orientation[4], orientation[5]);
	vec3 u2(orientation[6], orientation[7], orientation[8]);

	vec3 test[13] = {
		u0, // OBB Axis 1
		u1, // OBB Axis 2
		u2, // OBB Axis 
		vec3::crossproduct(f0, f1), // Normal of the Triangle 
		vec3::crossproduct(u0, f0),
		vec3::crossproduct(u0, f1),
		vec3::crossproduct(u0, f2),
		vec3::crossproduct(u1, f0),
		vec3::crossproduct(u1, f1),
		vec3::crossproduct(u1, f2),
		vec3::crossproduct(u2, f0),
		vec3::crossproduct(u2, f1),
		vec3::crossproduct(u2, f2)
	};

	for (unsigned int i = 0; i < 13; ++i)
	{
		if (!OverlapOnAxis(o, t, test[i]))
		{
			return false; // Separating axis found 
		}
	}

	return true;
}

float PlaneEquation(const vec3 &pt, const vec3 &normal, const float d)
{
	return pt * normal - d;
}

#define PlaneTriangle(p, t)    TrianglePlane(t, p

bool TrianglePlane(const triangle_t &t, const plane_t &p)
{
	float side1 = PlaneEquation(t.a, p.normal, p.d);
	float side2 = PlaneEquation(t.b, p.normal, p.d);
	float side3 = PlaneEquation(t.c, p.normal, p.d);

	if (	CMP(side1, 0) &&
		CMP(side2, 0) &&
		CMP(side3, 0))
	{
		return true;
	}

	if (side1 > 0 && side2 > 0 && side3 > 0)
	{
		return false;
	}

	if (side1 < 0 && side2 < 0 && side3 < 0)
	{
		return false;
	}

	return true; // Intersection
}

bool OverlapOnAxis(const triangle_t &t1, const triangle_t &t2, const vec3 &axis)
{
	interval_t a = GetInterval(t1, axis);
	interval_t b = GetInterval(t2, axis);
	return ((b.min <= a.max) && (a.min <= b.max));
}
/*
// Using "robust"version
bool TriangleTriangle(const triangle_t &t1, const triangle_t &t2)
{
	vec3 t1_f0 = t1.b - t1.a; // Triangle 1, Edge 0   
	vec3 t1_f1 = t1.c - t1.b; // Triangle 1, Edge 1   
	vec3 t1_f2 = t1.a - t1.c; // Triangle 1, Edge 2 

	vec3 t2_f0 = t2.b - t2.a; // Triangle 2, Edge 0   
	vec3 t2_f1 = t2.c - t2.b; // Triangle 2, Edge 1   
	vec3 t2_f2 = t2.a - t2.c; // Triangle 2, Edge 2

	vec3 axisToTest[] = {
		vec3::crossproduct(t1_f0, t1_f1),
		vec3::crossproduct(t2_f0, t2_f1),
		vec3::crossproduct(t2_f0, t1_f0),
		vec3::crossproduct(t2_f0, t1_f1),
		vec3::crossproduct(t2_f0, t1_f2),
		vec3::crossproduct(t2_f1, t1_f0),
		vec3::crossproduct(t2_f1, t1_f1),
		vec3::crossproduct(t2_f1, t1_f2),
		vec3::crossproduct(t2_f2, t1_f0),
		vec3::crossproduct(t2_f2, t1_f1),
		vec3::crossproduct(t2_f2, t1_f2),
	};

	for (unsigned int i = 0; i < 11; ++i)
	{
		if (!OverlapOnAxis(t1, t2, axisToTest[i]))
		{
			return false; // Seperating axis found      
		}
	}
}
*/

vec3 SatCrossEdge(const vec3& a, const vec3& b, const vec3& c, const vec3& d)
{
	vec3 ab = a - b;
	vec3 cd = c - d;
	vec3 result = vec3::crossproduct(ab, cd);

	if (!CMP(result.magnitudeSq(), 0))
	{
		return result; // Not parallel!
	}
	else
	{ // ab and cd are parallel 
		vec3 axis = vec3::crossproduct(ab, c - a);
		result = vec3::crossproduct(ab, axis);

		if (!CMP(result.magnitudeSq(), 0))
		{
			return result; // Not parallel        
		}
	}
	return vec3();
}

bool TriangleTriangle(const triangle_t &t1, const triangle_t &t2)
{
	vec3 axisToTest[] = {
		// Triangle 1, Normal       
		SatCrossEdge(t1.a, t1.b, t1.b, t1.c),
		// Triangle 2, Normal       
		SatCrossEdge(t2.a, t2.b, t2.b, t2.c),

		SatCrossEdge(t2.a, t2.b, t1.a, t1.b),
		SatCrossEdge(t2.a, t2.b, t1.b, t1.c),
		SatCrossEdge(t2.a, t2.b, t1.c, t1.a),
		SatCrossEdge(t2.b, t2.c, t1.a, t1.b),
		SatCrossEdge(t2.b, t2.c, t1.b, t1.c),
		SatCrossEdge(t2.b, t2.c, t1.c, t1.a),
		SatCrossEdge(t2.c, t2.a, t1.a, t1.b),
		SatCrossEdge(t2.c, t2.a, t1.b, t1.c),
		SatCrossEdge(t2.c, t2.a, t1.c, t1.a)
	};

	for (unsigned int i = 0; i < 11; ++i) 
	{
		if (!OverlapOnAxis(t1, t2, axisToTest[i])) 
		{
			if (!CMP(axisToTest[i].magnitudeSq(), 0)) 
			{
				return false; // Seperating axis found            
			}
		}
	}
	return true; // Seperating axis not found 
}

vec3 Barycentric(const vec3 &p, const triangle_t &t)
{
	vec3 ap = p - t.a;
	vec3 bp = p - t.b;
	vec3 cp = p - t.c;

	vec3 ab = t.b - t.a;
	vec3 ac = t.c - t.a;
	vec3 bc = t.c - t.b;
	vec3 cb = t.b - t.c;
	vec3 ca = t.a - t.c;

	vec3 v = ab - ab.project(cb);
	float a = 1.0f - ((v * ap) / (v * ab));

	v = bc - bc.project(ac);
	float b = 1.0f - ((v * bp) / (v * bc));

	v = ca - ca.project(ab);
	float c = 1.0f - ((v * cp) / (v * ca));

	return vec3(a, b, c);
}

float Raycast(const triangle_t &triangle, const ray_t &ray, raycast_result_t *result)
{
	plane_t plane;

	RaycastClear(result);
	FromTriangle(triangle, plane);
	Raycast(plane, ray, result);
	if (result->t < 0.0f)
	{
		//missed plane
		return false;
	}

	float t = result->t;

	vec3 r = ray.origin + ray.dir * result->t;

	RaycastClear(result);
	vec3 barycentric = Barycentric(r, triangle);
	if (barycentric.x >= 0.0f && barycentric.x <= 1.0f &&
		barycentric.y >= 0.0f && barycentric.y <= 1.0f &&
		barycentric.z >= 0.0f && barycentric.z <= 1.0f)
	{
		result->t = t;
		result->hit = true;
		result->point = ray.origin + ray.dir * t;
		result->normal = plane.normal;
	}

	return false;
}

bool Linetest(const triangle_t &triangle, const line3_t &line)
{
	ray_t ray;
	ray.origin = line.a;
	vec3 length = line.b - line.a;
	ray.dir = length.normalize();
	
	raycast_result_t result;
	Raycast(triangle, ray, &result);

	return result.t >= 0 && result.t * result.t <= (length * length);
}

// Missing from book?
bool Linetest(const mesh_t &mesh, const line3_t &line)
{
	if (mesh.accelerator == 0)
	{
		for (unsigned int i = 0; i < mesh.numTriangles; ++i)
		{
			if (Linetest(mesh.triangles[i], line))
			{
				return true;
			}
		}
	}
	else
	{
		std::list<bvh_node_t *> toProcess;
		toProcess.push_front(mesh.accelerator);

		// Recursivley walk the BVH tree
		while (!toProcess.empty())
		{
			bvh_node_t *iterator = *(toProcess.begin());
			toProcess.erase(toProcess.begin());

			if (iterator->numTriangles >= 0)
			{
				// Iterate trough all triangles of the node
				for (unsigned int i = 0; i < iterator->numTriangles; ++i)
				{
					// Triangle indices in BVHNode index the mesh
					if (Linetest(mesh.triangles[iterator->triangles[i]], line))
					{
						return true;
					}
				}
			}

			if (iterator->children != 0)
			{
				for (int i = 8 - 1; i >= 0; --i)
				{
					// Only push children whos bounds intersect the test geometry
					if (Linetest(iterator->children[i].bounds, line))
					{
						toProcess.push_front(&iterator->children[i]);
					}
				}
			}
		}
	}
	return false;
}

bool MeshSphere(const mesh_t &mesh, const sphere_t &sphere)
{
	if (mesh.accelerator == 0)
	{
		for (unsigned int i = 0; i < mesh.numTriangles; ++i)
		{
			if (TriangleSphere(mesh.triangles[i], sphere))
			{
				return true;
			}
		}
	}
	else
	{
		std::list<bvh_node_t*> toProcess;
		toProcess.push_front(mesh.accelerator);

		// Recursivley walk the BVH tree
		while (!toProcess.empty()) {
			bvh_node_t* iterator = *(toProcess.begin());
			toProcess.erase(toProcess.begin());

			if (iterator->numTriangles >= 0)
			{
				// Iterate trough all triangles of the node
				for (unsigned int i = 0; i < iterator->numTriangles; ++i)
				{
					// Triangle indices in BVHNode index the mesh
					if (TriangleSphere(mesh.triangles[iterator->triangles[i]], sphere))
					{
						return true;
					}
				}
			}

			if (iterator->children != 0)
			{
				for (int i = 8 - 1; i >= 0; --i)
				{
					// Only push children whos bounds intersect the test geometry
					if (SphereAABB(sphere, iterator->children[i].bounds))
					{
						toProcess.push_front(&iterator->children[i]);
					}
				}
			}
		}
	}
	return false;
}

bool MeshOBB(const mesh_t &mesh, const obb_t &obb)
{
	if (mesh.accelerator == 0)
	{
		for (unsigned int i = 0; i < mesh.numTriangles; ++i)
		{
			if (TriangleOBB(mesh.triangles[i], obb))
			{
				return true;
			}
		}
	}
	else
	{
		std::list<bvh_node_t *> toProcess;
		toProcess.push_front(mesh.accelerator);

		// Recursivley walk the BVH tree
		while (!toProcess.empty())
		{
			bvh_node_t* iterator = *(toProcess.begin());
			toProcess.erase(toProcess.begin());

			if (iterator->numTriangles >= 0)
			{
				// Iterate trough all triangles of the node
				for (unsigned int i = 0; i < iterator->numTriangles; ++i)
				{
					// Triangle indices in BVHNode index the mesh
					if (TriangleOBB(mesh.triangles[iterator->triangles[i]], obb))
					{
						return true;
					}
				}
			}

			if (iterator->children != 0)
			{
				for (int i = 8 - 1; i >= 0; --i)
				{
					// Only push children whos bounds intersect the test geometry
					if (AABB_OBB(iterator->children[i].bounds, obb))
					{
						toProcess.push_front(&iterator->children[i]);
					}
				}
			}
		}
	}
	return false;
}

bool MeshPlane(const mesh_t &mesh, const plane_t &plane)
{
	if (mesh.accelerator == 0)
	{
		for (unsigned int i = 0; i < mesh.numTriangles; ++i)
		{
			if (TrianglePlane(mesh.triangles[i], plane))
			{
				return true;
			}
		}
	}
	else
	{
		std::list<bvh_node_t*> toProcess;
		toProcess.push_front(mesh.accelerator);

		// Recursivley walk the BVH tree
		while (!toProcess.empty())
		{
			bvh_node_t* iterator = *(toProcess.begin());
			toProcess.erase(toProcess.begin());

			if (iterator->numTriangles >= 0)
			{
				// Iterate trough all triangles of the node
				for (unsigned int i = 0; i < iterator->numTriangles; ++i)
				{
					// Triangle indices in BVHNode index the mesh
					if (TrianglePlane(mesh.triangles[iterator->triangles[i]], plane))
					{
						return true;
					}
				}
			}

			if (iterator->children != 0)
			{
				for (int i = 8 - 1; i >= 0; --i)
				{
					// Only push children whos bounds intersect the test geometry
					if (AABBPlane(iterator->children[i].bounds, plane))
					{
						toProcess.push_front(&iterator->children[i]);
					}
				}
			}
		}
	}
	return false;
}

bool MeshTriangle(const mesh_t &mesh, const triangle_t &triangle)
{
	if (mesh.accelerator == 0)
	{
		for (unsigned int i = 0; i < mesh.numTriangles; ++i)
		{
			if (TriangleTriangle(mesh.triangles[i], triangle))
			{
				return true;
			}
		}
	}
	else
	{
		std::list<bvh_node_t *> toProcess;
		toProcess.push_front(mesh.accelerator);

		// Recursivley walk the BVH tree
		while (!toProcess.empty())
		{
			bvh_node_t* iterator = *(toProcess.begin());
			toProcess.erase(toProcess.begin());

			if (iterator->numTriangles >= 0)
			{
				// Iterate trough all triangles of the node
				for (unsigned int i = 0; i < iterator->numTriangles; ++i)
				{
					// Triangle indices in BVHNode index the mesh
					if (TriangleTriangle(mesh.triangles[iterator->triangles[i]], triangle))
					{
						return true;
					}
				}
			}

			if (iterator->children != 0)
			{
				for (int i = 8 - 1; i >= 0; --i)
				{
					// Only push children whos bounds intersect the test geometry
					if (TriangleAABB(triangle, iterator->children[i].bounds))
					{
						toProcess.push_front(&iterator->children[i]);
					}
				}
			}
		}
	}
	return false;
}

float MeshRay(const mesh_t &mesh, const ray_t &ray)
{
	if (mesh.accelerator == 0)
	{
		for (unsigned int i = 0; i < mesh.numTriangles; ++i)
		{
			raycast_result_t result;
			Raycast(mesh.triangles[i], ray, &result);
			if (result.t >= 0)
			{
				return result.t;
			}
		}
	}
	else
	{
		std::list<bvh_node_t *> toProcess;
		toProcess.push_front(mesh.accelerator);
		// Recursivley walk the BVH tree
		while (!toProcess.empty())
		{
			bvh_node_t* iterator = *(toProcess.begin());
			toProcess.erase(toProcess.begin());

			if (iterator->numTriangles >= 0)
			{
				for (unsigned int i = 0; i< iterator->numTriangles; ++i)
				{
					raycast_result_t result;

					// Do a raycast against the triangle
					Raycast(mesh.triangles[iterator->triangles[i]], ray, &result);
					if (result.t >= 0)
					{
						return result.t;
					}
				}
			}

			if (iterator->children != 0)
			{
				for (int i = 8 - 1; i >= 0; --i)
				{
					raycast_result_t result;
					Raycast(iterator->children[i].bounds, ray, &result);

					if (result.t >= 0)
					{
						toProcess.push_front(&iterator->children[i]);
					}
				}
			}
		}
	}
	return -1;
}

float Raycast(const mesh_t &mesh, const ray_t &ray)
{
	return MeshRay(mesh, ray);
}

//=============================================================================
//	3D Mesh tests with spatial optimizations (bvh nodes)
//=============================================================================

aabb_t AABB(vec3 max, vec3 min)
{
	aabb_t result;

	result.max = max;
	result.min = min;

	return result;
}


vec3 aabb_center_word(const aabb_t &aabb)
{
	return vec3(
		(aabb.max.x - aabb.min.x) * 0.5f + aabb.min.x,
		(aabb.max.y - aabb.min.y) * 0.5f + aabb.min.y,
		(aabb.max.z - aabb.min.z) * 0.5f + aabb.min.z
	);
}

vec3 aabb_center_model(const aabb_t &aabb)
{
	return vec3(
		(aabb.max.x - aabb.min.x) * 0.5f,
		(aabb.max.y - aabb.min.y) * 0.5f,
		(aabb.max.z - aabb.min.z) * 0.5f
	);
}

void SplitBVHNode(bvh_node_t* node, const mesh_t &model, int depth)
{
	if (depth-- == 0) { // Decrements depth
		return;
	}

	if (node->children == 0) // Only split if it's a leaf
	{
		if (node->numTriangles > 0) // Only split if this node contains triangles
		{
			node->children = new bvh_node_t[8];

			vec3 c = aabb_center_word(node->bounds);
			vec3 e = (node->bounds.max - node->bounds.min) * 0.5f;

			node->children[0].bounds =
				AABB(c + vec3(-e.x, +e.y, -e.z), e);
			node->children[1].bounds =
				AABB(c + vec3(+e.x, +e.y, -e.z), e);
			node->children[2].bounds =
				AABB(c + vec3(-e.x, +e.y, +e.z), e);
			node->children[3].bounds =
				AABB(c + vec3(+e.x, +e.y, +e.z), e);
			node->children[4].bounds =
				AABB(c + vec3(-e.x, -e.y, -e.z), e);
			node->children[5].bounds =
				AABB(c + vec3(+e.x, -e.y, -e.z), e);
			node->children[6].bounds =
				AABB(c + vec3(-e.x, -e.y, +e.z), e);
			node->children[7].bounds =
				AABB(c + vec3(+e.x, -e.y, +e.z), e);
		}
	}

	// If this node was just split
	if (node->children != 0 && node->numTriangles > 0)
	{
		for (unsigned int i = 0; i < 8; ++i)  // For each child
		{
			node->children[i].numTriangles = 0;
			for (unsigned int j = 0; j < node->numTriangles; ++j)
			{
				triangle_t t = model.triangles[node->triangles[j]];

				if (TriangleAABB(t, node->children[i].bounds))
				{
					node->children[i].numTriangles += 1;
				}
			}

			if (node->children[i].numTriangles == 0)
			{
				continue;
			}

			node->children[i].triangles = new int[node->children[i].numTriangles];
			int index = 0;

			for (unsigned int j = 0; j < node->numTriangles; ++j)
			{
				triangle_t t = model.triangles[node->triangles[j]];
				if (TriangleAABB(t, node->children[i].bounds))
				{
					node->children[i].triangles[index++] = node->triangles[j];
				}
			}
		}
		node->numTriangles = 0;
		delete[] node->triangles;
		node->triangles = 0;

		for (unsigned int i = 0; i < 8; ++i)
		{
			SplitBVHNode(&node->children[i], model, depth);
		}
	}
}

void FreeBVHNode(bvh_node_t* node)
{
	if (node->children != 0)
	{
		for (unsigned int i = 0; i < 8; ++i)
		{
			FreeBVHNode(&node->children[i]);
		}
		delete[] node->children;
		node->children = 0;
	}

	if (node->numTriangles != 0 || node->triangles != 0)
	{
		delete[] node->triangles;
		node->triangles = 0;
		node->numTriangles = 0;
	}
}

void AccelerateMesh(mesh_t &mesh)
{
	if (mesh.accelerator != 0)
	{
		return;
	}

	vec3 min = mesh.vertices[0];
	vec3 max = mesh.vertices[0];
	for (unsigned int i = 1; i < mesh.numTriangles * 3; ++i)
	{
		min.x = MIN(mesh.vertices[i].x, min.x);
		min.y = MIN(mesh.vertices[i].y, min.y);
		min.z = MIN(mesh.vertices[i].z, min.z);
		max.x = MAX(mesh.vertices[i].x, max.x);
		max.y = MAX(mesh.vertices[i].y, max.y);
		max.z = MAX(mesh.vertices[i].z, max.z);
	}

	mesh.accelerator = new bvh_node_t;
	mesh.accelerator->bounds.min = min;
	mesh.accelerator->bounds.max = max;
	mesh.accelerator->numTriangles = mesh.numTriangles;

	mesh.accelerator->triangles =
		new int[mesh.numTriangles];

	for (unsigned int i = 0; i < mesh.numTriangles; ++i)
	{
		mesh.accelerator->triangles[i] = i;
	}

	// Recursively split BVH tree
	SplitBVHNode(mesh.accelerator, mesh, 3);
}

bool MeshAABB(const mesh_t &mesh, const aabb_t &aabb)
{
	if (mesh.accelerator == 0)
	{
		for (unsigned int i = 0; i < mesh.numTriangles; ++i)
		{
			// The TirangleAABB test here would change
			// if we where testing a shape other than AABB
			if (TriangleAABB(mesh.triangles[i], aabb))
			{
				return true;
			}
		}
	}
	else
	{
		std::list<bvh_node_s*> toProcess;
		toProcess.push_front(mesh.accelerator);
		while (!toProcess.empty())
		{
			bvh_node_s* iterator = *(toProcess.begin());
			toProcess.erase(toProcess.begin());
			if (iterator->numTriangles >= 0)
			{
				for (unsigned int i = 0; i<iterator->numTriangles; ++i)
				{
					// The TirangleAABB test here would change
					// if we where testing a shape other than AABB
					if (TriangleAABB(mesh.triangles[iterator->triangles[i]], aabb))
					{
						return true;
					}
				}
			}
			if (iterator->children != 0)
			{
				for (int i = 8 - 1; i >= 0; --i)
				{
					// The AABBAABB test here would change
					// if we where testing a shape other than AABB
					if (AABB_AABB(iterator->children[i].bounds,aabb))
					{
						toProcess.push_front(&iterator->children[i]);
					}
				}
			}
		}
	}
	return false;
}

matrix4 Translation(vec3 pos)
{
	matrix4 result;
	float fm[] = {
			1.0, 0.0, 0.0, 0.0,
			0.0, 1.0, 0.0, 0.0,
			0.0, 0.0, 1.0, 0.0,
			pos.x, pos.y, pos.z, 1.0,
	};
	
	for (int i = 0; i < 16; i++)
	{
		result.m[i] = fm[i];
	}

	return result;
}

#define RAD2DEG(x) ((x) * 57.295754f)
#define DEG2RAD(x) ((x) * 0.0174533f)

matrix4 ZRotation(float angle)
{
	angle = DEG2RAD(angle);
	return matrix4(
		fcos(angle), fsin(angle), 0.0f, 0.0f,
		-fsin(angle), fcos(angle), 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

matrix4 YRotation(float angle)
{
	angle = DEG2RAD(angle);
	return matrix4(
		fcos(angle), 0.0f, -fsin(angle), 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		fsin(angle), 0.0f, fcos(angle), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

matrix4 XRotation(float angle)
{
	angle = DEG2RAD(angle);
	return matrix4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, fcos(angle), fsin(angle), 0.0f,
		0.0f, -fsin(angle), fcos(angle), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

matrix3 ZRotation3(float angle)
{
	angle = DEG2RAD(angle);
	return matrix3(
		fcos(angle), fsin(angle), 0.0f,
		-fsin(angle), fcos(angle), 0.0f,
		0.0f, 0.0f, 1.0f
	);
}

matrix3 YRotation3(float angle)
{
	angle = DEG2RAD(angle);
	return matrix3(
		fcos(angle), 0.0f, -fsin(angle),
		0.0f, 1.0f, 0.0f,
		fsin(angle), 0.0f, fcos(angle)
	);
}

matrix3 XRotation3(float angle)
{
	angle = DEG2RAD(angle);
	return matrix3(
		1.0f, 0.0f, 0.0f,
		0.0f, fcos(angle), fsin(angle),
		0.0f, -fsin(angle), fcos(angle)
	);
}


// this is pretty inefficent
matrix4 Rotation(float pitch, float yaw, float roll)
{
	return ZRotation(roll) * XRotation(pitch) * YRotation(yaw);
}

//=============================================================================
//	Models and Scenes
//=============================================================================
matrix3 Rotation3x3(float pitch, float yaw, float roll)
{
	return ZRotation3(roll) * XRotation3(pitch) * YRotation3(yaw);
}

class physics::CModel
{
protected:
	mesh_t *content;
	aabb_t bounds;
public:
	vec3 position;
	vec3 rotation;
	CModel *parent;

	inline CModel()
	{
		parent = NULL;
		content = NULL;
	}

	inline mesh_t *GetMesh() const
	{
		return content;
	}

	inline aabb_t GetBounds() const
	{
		return bounds;
	}

	void SetContent(mesh_t* mesh)
	{
		content = mesh;
		if (content != 0)
		{
			vec3 min = mesh->vertices[0];
			vec3 max = mesh->vertices[0];
			for (unsigned int i = 1; i< mesh->numTriangles * 3; ++i)
			{
				min.x = MIN(mesh->vertices[i].x, min.x);
				min.y = MIN(mesh->vertices[i].y, min.y);
				min.z = MIN(mesh->vertices[i].z, min.z);
				max.x = MAX(mesh->vertices[i].x, max.x);
				max.y = MAX(mesh->vertices[i].y, max.y);
				max.z = MAX(mesh->vertices[i].z, max.z);
			}
			bounds.max = max;
			bounds.min = min;			
		}
	}

	matrix4 GetWorldMatrix() const
	{
		matrix4 translation = Translation(position);
		matrix4 rot = Rotation(
			rotation.x,
			rotation.y,
			rotation.z
		);
		matrix4 localMat = rot * translation;

		matrix4 parentMat;
		if (parent != 0)
		{
			parentMat = parent->GetWorldMatrix();
		}

		return localMat * parentMat;
	}

	obb_t GetOBB(const CModel& model)
	{
		matrix4 world = model.GetWorldMatrix();
		matrix3 world3;

		world3.matrix4to3(world);
		aabb_t aabb = model.GetBounds();
		obb_t obb;

		obb.size = aabb.max - aabb.min;
		obb.origin = world3 * aabb_center_model(aabb);
		obb.orientation = world3;
		return obb;
	}

	float ModelRay(const CModel& model, const ray_t &ray)
	{
		matrix4 world = model.GetWorldMatrix();
		matrix4 inv = world.inverse();

		ray_t local;
		local.origin = inv * vec4(ray.origin.x, ray.origin.y, ray.origin.z, 1.0f);
		local.dir = inv * vec4(ray.origin.x, ray.origin.y, ray.origin.z, 1.0f);
		local.dir.normalize();

		if (model.GetMesh() != 0)
		{
			return MeshRay(*(model.GetMesh()), local);
		}
		return -1;
	}


	bool Linetest(const CModel& model, const line3_t &line)
	{
		matrix4 world = model.GetWorldMatrix();
		matrix4 inv = world.inverse();

		line3_t local;
		local.a = inv * vec4(line.a.x, line.a.y, line.a.z, 1.0f);
		local.b = inv * vec4(line.b.x, line.b.y, line.b.z, 1.0f);

		if (model.GetMesh() != 0)
		{
			return ::Linetest(*(model.GetMesh()), local);
		}
		return false;
	}

	bool ModelSphere(const CModel& model, const sphere_t &sphere)
	{
		matrix4 world = model.GetWorldMatrix();
		matrix4 inv = world.inverse();

		sphere_t local;
		local.origin = inv * vec4(sphere.origin.x, sphere.origin.y, sphere.origin.z, 1.0f);

		if (model.GetMesh() != 0)
		{
			return MeshSphere(*(model.GetMesh()), local);
		}
		return false;
	}

	bool ModelAABB(const CModel &model, const aabb_t &aabb)
	{
		matrix4 world = model.GetWorldMatrix();
		matrix4 inv = world.inverse();

		obb_t local;
		local.size = aabb.max - aabb.min;
		vec3 p = aabb_center_model(aabb);
		local.origin = inv * vec4(p.x, p.y, p.z, 1.0f);
		local.orientation.matrix4to3(inv);

		if (model.GetMesh() != 0)
		{
			return MeshOBB(*(model.GetMesh()), local);
		}
		return false;
	}

	bool ModelOBB(const CModel& model, const obb_t &obb)
	{
		matrix4 world = model.GetWorldMatrix();
		matrix4 inv = world.inverse();

		obb_t local;
		local.size = obb.size;
		local.origin = inv * vec4(obb.origin.x, obb.origin.y, obb.origin.z, 1.0f);
		local.orientation.matrix4to3(inv);

		if (model.GetMesh() != 0)
		{
			return MeshOBB(*(model.GetMesh()), local);
		}
		return false;
	}

	bool ModelPlane(const CModel& model, const plane_t& plane)
	{
		matrix4 world = model.GetWorldMatrix();
		matrix4 inv = world.inverse();

		plane_t local;
		local.normal = inv * vec4(plane.normal.x, plane.normal.y, plane.normal.z, 1.0f);
		local.d = plane.d;
		if (model.GetMesh() != 0)
		{
			return MeshPlane(*(model.GetMesh()), local);
		}
		return false;
	}

	bool ModelTriangle(const CModel& model, const triangle_t &triangle)
	{
		matrix4 world = model.GetWorldMatrix();
		matrix4 inv = world.inverse();

		triangle_t local;
		local.a = inv * vec4(triangle.a.x, triangle.a.y, triangle.a.z, 1.0f);
		local.b = inv * vec4(triangle.b.x, triangle.b.y, triangle.b.z, 1.0f);
		local.c = inv * vec4(triangle.c.x, triangle.c.y, triangle.c.z, 1.0f);

		if (model.GetMesh() != 0)
		{
			return MeshTriangle(*(model.GetMesh()), local);
		}
		return false;
	}

};

//=============================================================================
//	Oct tree
//=============================================================================

void SplitTree(COctreeNode* node, int depth)
{
	if (depth-- <= 0)  // Decrements depth
	{
		return;
	}

	if (node->children == 0)
	{
		node->children = new COctreeNode[8];
		vec3 c = aabb_center_model(node->bounds);
		vec3 e = (node->bounds.max - node->bounds.min) * 0.5f;

		node->children[0].bounds =
			AABB(c + vec3(-e.x, +e.y, -e.z), e);
		node->children[1].bounds =
			AABB(c + vec3(+e.x, +e.y, -e.z), e);
		node->children[2].bounds =
			AABB(c + vec3(-e.x, +e.y, +e.z), e);
		node->children[3].bounds =
			AABB(c + vec3(+e.x, +e.y, +e.z), e);
		node->children[4].bounds =
			AABB(c + vec3(-e.x, -e.y, -e.z), e);
		node->children[5].bounds =
			AABB(c + vec3(+e.x, -e.y, -e.z), e);
		node->children[6].bounds =
			AABB(c + vec3(-e.x, -e.y, +e.z), e);
		node->children[7].bounds =
			AABB(c + vec3(+e.x, -e.y, +e.z), e);
	}

	if (node->children != 0 && node->models.size() > 0)
	{
		for (int i = 0; i < 8; ++i) // For each child
		{
			for (int j = 0, size = node->models.size(); j < size; ++j)
			{
				obb_t bounds;// = GetOBB(*node->models[j]);
				if (AABB_OBB(node->children[i].bounds, bounds))
				{
					node->children[i].models.push_back(
						node->models[j]
					);
				}
			}
		}
		node->models.clear();
		for (int i = 0; i < 8; ++i)  // Recurse
		{
			SplitTree(&(node->children[i]), depth);
		}
	}
}

//=============================================================================
//	Scene
//=============================================================================

struct OctreeNode;

class Scene
{
protected:
	std::vector<physics::CModel*> objects;
	COctreeNode *octree;
public:
	void AddModel(physics::CModel* model);
	void RemoveModel(physics::CModel* model);
	void UpdateModel(physics::CModel* model);
	std::vector<physics::CModel*> FindChildren(const physics::CModel* model);

	physics::CModel* Raycast(const ray_t &ray);
	std::vector<physics::CModel*> Query(const sphere_t &sphere);
	std::vector<physics::CModel*> Query(const aabb_t &aabb);
	bool Accelerate(const vec3& position, float size);

	void Insert(COctreeNode* node, physics::CModel* model);
	void Remove(COctreeNode* node, physics::CModel* model);
	void Update(COctreeNode* node, physics::CModel* model);

	physics::CModel* FindClosest(const std::vector<physics::CModel*>& set, const ray_t &ray);
	physics::CModel* Raycast(COctreeNode* node, const ray_t &ray);
	std::vector<physics::CModel*> Query(COctreeNode* node, const sphere_t &sphere);
	std::vector<physics::CModel*> Query(COctreeNode* node, const aabb_t &aabb);

	inline Scene()
	{
		octree = NULL;
	}
	inline ~Scene()
	{
		if (octree != NULL)
		{
			delete octree;
		}
	}
private:
	Scene(const Scene&);
	Scene& operator=(const Scene&);
};

bool Scene::Accelerate(const vec3& position, float size)
{
	if (octree != 0)
	{
		return false;
	}

	vec3 min(position.x - size,
		position.y - size,
		position.z - size);
	vec3 max(position.x + size,
		position.y + size,
		position.z + size);

	// Construct tree root
	octree = new COctreeNode();
	//octree->bounds;// = FromMinMax(min, max);
	octree->children = 0;
	for (int i = 0, size = objects.size(); i< size; ++i)
	{
		octree->models.push_back(objects[i]);
	}

	SplitTree(octree, 5);
	return true;
}

float ModelRay(const CModel& model, const ray_t& ray)
{
	matrix4 world = model.GetWorldMatrix();
	matrix4 inv = world.inverse();
	ray_t local;
	local.origin = inv * vec4(ray.origin.x, ray.origin.y, ray.origin.z, 1.0f);
	local.dir = inv * vec4(ray.dir.x, ray.dir.y, ray.dir.z, 1.0f);
	local.dir.normalize();
	if (model.GetMesh() != 0)
	{
		return MeshRay(*(model.GetMesh()), local);
	}
	return -1;
}
/*
CModel* Scene::Raycast(const ray_t& ray)
{
	if (octree != 0)
	{
//		return ::Raycast(octree, ray);
	}

	CModel* result = 0;
	float result_t = -1;
	for (int i = 0, size = objects.size(); i< size; ++i) {
		float t = ModelRay(*objects[i], ray);
		if (result == 0 && t >= 0) {
			result = objects[i];
			result_t = t;
		}
		else if (result != 0 && t <result_t) {
			result = objects[i];
			result_t = t;
		}
	}
	return result;
}
*/
std::vector<CModel*> Scene::Query(const sphere_t &sphere)
{
	if (octree != 0)
	{
//		return ::Query(octree, sphere);
	}

	std::vector<CModel*> result;
	for (int i = 0, size = objects.size(); i< size; ++i)
	{
		obb_t bounds;// = GetOBB(*objects[i]);
		if (SphereOBB(sphere, bounds))
		{
			result.push_back(objects[i]);
		}
	}
	return result;
}

std::vector<physics::CModel*> Scene::Query(const aabb_t &aabb)
{
	if (octree != 0)
	{
//		return ::Query(octree, aabb);
	}

	std::vector<physics::CModel*> result;
	for (int i = 0, size = objects.size(); i< size; ++i)
	{
		obb_t bounds;// = GetOBB(*objects[i]);
		if (AABB_OBB(aabb, bounds))
		{
			result.push_back(objects[i]);
		}
	}
	return result;
}

void Scene::AddModel(physics::CModel* model)
{
	if (std::find(objects.begin(), objects.end(), model) != objects.end())
	{
		// Duplicate object, don't add
		return;
	}

	objects.push_back(model);
}

void Scene::RemoveModel(physics::CModel* model)
{
//	objects.erase(std::remove(objects.begin(), objects.end(), model), objects.end());
}

void Scene::UpdateModel(physics::CModel* model)
{
	// Placeholder
}

std::vector<physics::CModel*> Scene::FindChildren(const physics::CModel* model)
{
	std::vector<physics::CModel*> result;
	for (int i = 0, size = objects.size(); i < size; ++i)
	{
		if (objects[i] == 0 || objects[i] == model)
		{
			continue;
		}

		physics::CModel* iterator = objects[i]->parent;
		if (iterator != 0)
		{
			if (iterator == model)
			{
				result.push_back(objects[i]);
				continue;
			}
			iterator = iterator->parent;
		}
	}

	return result;
}

physics::CModel* Scene::Raycast(const ray_t &ray)
{
	physics::CModel* result = 0;
	float result_t = -1;

	for (int i = 0, size = objects.size(); i < size; ++i)
	{
		float t = ModelRay(*objects[i], ray);
		if (result == 0 && t >= 0)
		{
			result = objects[i];
			result_t = t;
		}
		else if (result != 0 && t < result_t)
		{
			result = objects[i];
			result_t = t;
		}
	}
	return result;
}

/*
std::vector<CModel*> Scene::Query(const sphere_t &sphere)
{
	std::vector<CModel*> result;

	for (int i = 0, size = objects.size(); i < size; ++i)
	{
		obb_t bounds;// = GetOBB(*objects[i]);

		if (SphereOBB(sphere, bounds))
		{
			result.push_back(objects[i]);
		}
	}

	return result;
}*/
/*
std::vector<CModel*> Scene::Query(const aabb_t &aabb)
{
	std::vector<CModel*> result;

	for (int i = 0, size = objects.size(); i < size; ++i)
	{
		obb_t bounds;// = GetOBB(*objects[i]);

		if (AABB_OBB(aabb, bounds))
		{
			result.push_back(objects[i]);
		}
	}

	return result;
}
*/

void Scene::Insert(COctreeNode* node, physics::CModel* model)
{
	obb_t bounds;// = GetOBB(*model);
	if (AABB_OBB(node->bounds, bounds))
	{
		if (node->children == 0)
		{
			node->models.push_back(model);
		}
		else
		{
			for (int i = 0; i < 8; ++i)
			{
				Insert(&(node->children[i]), model);
			}
		}
	}
}

void Scene::Remove(COctreeNode* node, physics::CModel* model)
{
	if (node->children == 0)
	{
		std::vector<physics::CModel*>::iterator it =
			std::find(node->models.begin(), node->models.end(), model);

		if (it != node->models.end())
		{
			node->models.erase(it);
		}
	}
	else
	{
		for (int i = 0; i < 8; ++i)
		{
			Remove(&(node->children[i]), model);
		}
	}
}

void Scene::Update(COctreeNode* node, CModel* model)
{
	Remove(node, model);
	Insert(node, model);
}

CModel *Scene::FindClosest(const std::vector<CModel*>& set, const ray_t &ray)
{
	if (set.size() == 0)
	{
		return 0;
	}

	CModel* closest = 0;
	float closest_t = -1;

	for (int i = 0, size = set.size(); i < size; ++i)
	{
		float this_t = ModelRay(*set[i], ray);

		if (this_t < 0)
		{
			continue;
		}

		if (closest_t < 0 || this_t < closest_t) {
			closest_t = this_t;
			closest = set[i];
		}
	}
	return closest;
}

physics::CModel* Scene::Raycast(COctreeNode* node, const ray_t &ray)
{
	raycast_result_t result;
//	Raycast(node->bounds, ray, &result);
	float t = result.t;

	if (t >= 0)
	{
		if (node->children == 0)
		{
			return FindClosest(node->models, ray);
		}
		else
		{
			std::vector<physics::CModel*> results;
			for (int i = 0; i < 8; ++i)
			{
				physics::CModel* result =
					Raycast(&(node->children[i]), ray);
				if (result != 0)
				{
					results.push_back(result);
				}
			}
			return FindClosest(results, ray);
		}
	}
	return 0;
}

std::vector<physics::CModel*> Scene::Query(COctreeNode* node, const sphere_t &sphere)
{
	std::vector<physics::CModel*> result;

	if (SphereAABB(sphere, node->bounds))
	{
		if (node->children == 0)
		{
			for (int i = 0, size = node->models.size();	i < size; ++i)
			{
				obb_t bounds;// = GetOBB(*(node->models[i]));
				if (SphereOBB(sphere, bounds))
				{
					result.push_back(node->models[i]);
				}
			}
		}
		else
		{
			for (int i = 0; i < 8; ++i)
			{
				std::vector<physics::CModel*> child = Query(&(node->children[i]), sphere);
				if (child.size() > 0)
				{
					result.insert(result.end(), child.begin(), child.end());
				}
			}
		}
	}
	return result;
}

std::vector<physics::CModel*> Scene::Query(COctreeNode* node, const aabb_t &aabb)
{
	std::vector<physics::CModel*> result;

	if (AABB_AABB(aabb, node->bounds))
	{
		if (node->children == 0)
		{
			for (int i = 0, size = node->models.size(); i < size; ++i)
			{
				obb_t bounds;// = GetOBB(*(node->models[i]));
				if (AABB_OBB(aabb, bounds))
				{
					result.push_back(node->models[i]);
				}
			}
		}
		else
		{
			for (int i = 0; i < 8; ++i)
			{
				std::vector<physics::CModel*> child = Query(&(node->children[i]), aabb);
				if (child.size() > 0)
				{
					result.insert(result.end(), child.begin(), child.end());
				}
			}
		}
	}
	return result;
}





//=============================================================================
//	Camera and Frustum
//=============================================================================
class Camera
{
protected:
	float m_nFov;
	float m_nAspect;
	float m_nNear;
	float m_nFar;
	float m_nWidth;
	float m_nHeight;

	matrix4 m_matWorld; // World Transform
					 // View Transform = Inverse(World Transform)
	matrix4 m_matProj;
	int m_nProjectionMode;
	// ^ 0 - Perspective, 1 - Ortho, 2 - User
public:
	Camera();
	inline virtual ~Camera()
	{
	}
	matrix4 GetWorldMatrix();
	matrix4 GetViewMatrix(); // Inverse of world!
	matrix4 GetProjectionMatrix();
	void SetProjection(const matrix4& projection);
	void SetWorld(const matrix4& view);

	float GetAspect();
	bool IsOrthographic();
	bool IsPerspective();
	bool IsOrthoNormal();
	void OrthoNormalize();

	void Resize(int width, int height);
	matrix4 Perspective(float fov, float aspect, float zNear, float zFar);
	matrix4 Ortho(float left, float right, float bottom, float top, float zNear, float zFar);
	Frustum GetFrustum();

};

matrix4 Camera::Perspective(float fov, float aspect, float zNear, float zFar)
{
	float tanHalfFov = tanf(DEG2RAD((fov * 0.5f)));
	float fovY = 1.0f / tanHalfFov; // cot(fov/2)
	float fovX = fovY / aspect; // cot(fov/2) / aspect

	matrix4 result;

	result.m[0] = fovX;
	result.m[5] = fovY;
	result.m[10] = zFar / (zFar - zNear); // far / range
	result.m[11] = 1.0f;
	result.m[14] = -zNear * result.m[10]; // - near * (far / range)
	result.m[15] = 0.0f;

	return result;
}

matrix4 Camera::Ortho(float left, float right, float bottom, float top, float zNear, float zFar)
{
	float _11 = 2.0f / (right - left);
	float _22 = 2.0f / (top - bottom);
	float _33 = 1.0f / (zFar - zNear);
	float _41 = (left + right) / (left - right);
	float _42 = (top + bottom) / (bottom - top);
	float _43 = (zNear) / (zNear - zFar);

	return matrix4(
		_11, 0.0f, 0.0f, 0.0f,
		0.0f, _22, 0.0f, 0.0f,
		0.0f, 0.0f, _33, 0.0f,
		_41, _42, _43, 1.0f
	);
}

Camera::Camera()
{
	m_nFov = 60.0f;
	m_nAspect = 1.3f;
	m_nNear = 0.01f;
	m_nFar = 1000.0f;
	m_nWidth = 1.0;
	m_nHeight = 1.0f;
	m_matWorld = matrix4();
	m_matProj = Perspective(m_nFov, m_nAspect, m_nNear, m_nFar);
	m_nProjectionMode = 0;
}

matrix4 Camera::GetWorldMatrix()
{
	return m_matWorld;
}

matrix4 Camera::GetViewMatrix()
{
	if (!IsOrthoNormal())
	{
		OrthoNormalize();
	}

	matrix4 inverse = m_matWorld.transpose();
	inverse.m[12] = inverse.m[3] = 0.0f;
	inverse.m[13] = inverse.m[7] = 0.0f;
	inverse.m[14] = inverse.m[11] = 0.0f;

	vec3 right		= vec3(m_matWorld.m[0], m_matWorld.m[1], m_matWorld.m[2]);
	vec3 up			= vec3(m_matWorld.m[4], m_matWorld.m[5], m_matWorld.m[6]);
	vec3 forward	= vec3(m_matWorld.m[8], m_matWorld.m[9], m_matWorld.m[10]);
	vec3 position	= vec3(m_matWorld.m[12], m_matWorld.m[13], m_matWorld.m[14]);

	inverse.m[12] = -right * position;
	inverse.m[13] = -up * position;
	inverse.m[14] = -forward * position;
	return inverse;
}

matrix4 Camera::GetProjectionMatrix()
{
	return m_matProj;
}

float Camera::GetAspect()
{
	return m_nAspect;
}

bool Camera::IsOrthographic()
{
	return m_nProjectionMode == 1;
}

bool Camera::IsPerspective()
{
	return m_nProjectionMode == 0;
}

bool Camera::IsOrthoNormal()
{
	vec3 right = vec3(m_matWorld.m[0], m_matWorld.m[1], m_matWorld.m[2]); 
	vec3 up = vec3(m_matWorld.m[5], m_matWorld.m[6], m_matWorld.m[7]);
	vec3 forward = vec3(m_matWorld.m[9], m_matWorld.m[10], m_matWorld.m[11]);

	if (!CMP(right * right, 1.0f) ||
		!CMP(up * up, 1.0f) ||
		!CMP(forward * forward, 1.0f))
	{
		return false; // Axis are not normal length
	}

	if (!CMP(forward * up, 0.0f) ||
		!CMP(forward * right, 0.0f) ||
		!CMP(right * up, 0.0f))
	{
		return false; // Axis are not perpendicular
	}
	return true;
}

void Camera::OrthoNormalize()
{
	//vec3 right = vec3(m_matWorld.m[0], m_matWorld.m[1], m_matWorld.m[2]);
	vec3 up = vec3(m_matWorld.m[5], m_matWorld.m[6], m_matWorld.m[7]);
	vec3 forward = vec3(m_matWorld.m[9], m_matWorld.m[10], m_matWorld.m[11]);

	vec3 f = forward.normalize();
	vec3 r = vec3::crossproduct(up, f).normalize();
	vec3 u = vec3::crossproduct(f, r);

	m_matWorld = matrix4(
		r.x, r.y, r.z, 0.0f,
		u.x, u.y, u.z, 0.0f,
		f.x, f.y, f.z, 0.0f,
		m_matWorld.m[12], m_matWorld.m[13], m_matWorld.m[14], 1.0f 
	);
}

void Camera::Resize(int width, int height)
{
	m_nAspect = (float)width / (float)height;

	if (m_nProjectionMode == 0)  // Perspective
	{
		m_matProj = Perspective(m_nFov, m_nAspect, m_nNear, m_nFar);
	}
	else if (m_nProjectionMode == 1) // Ortho
	{
		m_nWidth = (float)width;
		m_nHeight = (float)height;
		float halfW = m_nWidth * 0.5f;
		float halfH = m_nHeight * 0.5f;
		m_matProj = Ortho(-halfW, halfW,
			halfH, -halfH, m_nNear, m_nFar);
	}
	// m_nProjectionMode == 2
	// User defined
}

void Camera::SetProjection(const matrix4& projection)
{
	m_matProj = projection;
	m_nProjectionMode = 2;
}

void Camera::SetWorld(const matrix4& view)
{
	m_matWorld = view;
}


Frustum Camera::GetFrustum()
{
	Frustum result;

	matrix4 vp = GetViewMatrix() * GetProjectionMatrix();

	vec3 col1(vp.m[0], vp.m[4], vp.m[8]);//, vp._41
	vec3 col2(vp.m[1], vp.m[5], vp.m[9]);//, vp._42
	vec3 col3(vp.m[2], vp.m[6], vp.m[10]);//, vp._43
	vec3 col4(vp.m[3], vp.m[7], vp.m[11]);//, vp._44

	result.p.left.normal		= col4 + col1;
	result.p.right.normal		= col4 - col1;
	result.p.bottom.normal		= col4 + col2;
	result.p.top.normal		= col4 - col2;
//	result.znear.normal		= col3; 	//directx
	result.p.znear.normal		= col4 + col3; //opengl
	result.p.zfar.normal		= col4 - col3;

	result.p.left.d		= vp.m[15] + vp.m[12];
	result.p.right.d	= vp.m[15] - vp.m[12];
	result.p.bottom.d	= vp.m[15] + vp.m[13];
	result.p.top.d		= vp.m[15] - vp.m[13];
	//result.znear.d	= vp._43;			//directx style [0,1]
	result.p.znear.d	= vp.m[15] + vp.m[14];		//opengl style [-1,1]
	result.p.zfar.d	= vp.m[15] - vp.m[14];

	for (int i = 0; i < 6; ++i)
	{
		float mag = 1.0f / result.planes[i].normal.magnitude();
		result.planes[i].normal = result.planes[i].normal*mag;
		result.planes[i].d *= mag;
	}
	return result;
}

class OrbitCamera : public Camera
{
protected:
	vec3 target;
	vec2 panSpeed;

	float zoomDistance;
	vec2 zoomDistanceLimit; // x = min, y = max;
	float zoomSpeed;

	vec2 rotationSpeed;
	vec2 yRotationLimit; // x = min, y = max
	vec2 currentRotation;

	float ClampAngle(float angle, float min, float max);
public:
	OrbitCamera();
	inline virtual ~OrbitCamera() { }
	void Rotate(const vec2& deltaRot, float deltaTime);
	void Zoom(float deltaZoom, float deltaTime);
	void Pan(const vec2& delataPan, float deltaTime);
	void Update(float dt);
};

OrbitCamera::OrbitCamera()
{
	target = vec3(0, 0, 0);
	zoomDistance = 10.0f;
	zoomSpeed = 200.0f;
	rotationSpeed = vec2(250.0f, 120.0f);
	yRotationLimit = vec2(-20.0f, 80.0f);
	zoomDistanceLimit = vec2(3.0f, 15.0f);
	currentRotation = vec2(0, 0);
	panSpeed = vec2(180.0f, 180.0f);
}

void OrbitCamera::Rotate(const vec2& deltaRot, float deltaTime)
{
	currentRotation.x += deltaRot.x * rotationSpeed.x * zoomDistance* deltaTime;
	currentRotation.y += deltaRot.y * rotationSpeed.y * zoomDistance * deltaTime;

	currentRotation.x = ClampAngle(currentRotation.x, -360, 360);
	currentRotation.y = ClampAngle(currentRotation.y, yRotationLimit.x, yRotationLimit.y);
}

void OrbitCamera::Zoom(float deltaZoom, float deltaTime)
{
	zoomDistance = zoomDistance + deltaZoom * zoomSpeed * deltaTime;

	if (zoomDistance<zoomDistanceLimit.x)
	{
		zoomDistance = zoomDistanceLimit.x;
	}
	if (zoomDistance>zoomDistanceLimit.y)
	{
		zoomDistance = zoomDistanceLimit.y;
	}
}

void OrbitCamera::Pan(const vec2& delataPan, float deltaTime)
{
	vec3 right = vec3(m_matWorld.m[0], m_matWorld.m[1], m_matWorld.m[2]);

	float xPanMag = delataPan.x * panSpeed.x * deltaTime;
	target = target - (right * xPanMag);

	float yPanMag = delataPan.y * panSpeed.y * deltaTime;
	target = target + (vec3(0, 1, 0) * yPanMag);
}

void OrbitCamera::Update(float dt)
{
	vec3 rotation = vec3(currentRotation.y, currentRotation.x, 0);
	matrix3 orient = Rotation3x3(rotation.x, rotation.y, rotation.z);
	vec3 direction = orient * vec3(0.0, 0.0, -zoomDistance);
	vec3 position = direction + target;

	matrix4 view;

	vec3 up(0.0f, 1.0f, 0.0f);
	view.lookat(position, target, up);
	m_matWorld = view.inverse();
}

float OrbitCamera::ClampAngle(float angle, float min, float max)
{
	while (angle < -360)
	{
		angle += 360;
	}
	while (angle > 360)
	{
		angle -= 360;
	}
	if (angle < min)
	{
		angle = min;
	}
	if (angle > max)
	{
		angle = max;
	}
	return angle;
}



// Cramer's Rule
//http://www.purplemath.com/modules/cramers.htm
vec3 Intersection(plane_t p1, plane_t p2, plane_t p3)
{
	matrix3 D(
		p1.normal.x, p2.normal.x, p3.normal.x,
		p1.normal.y, p2.normal.y, p3.normal.y,
		p1.normal.z, p2.normal.z, p3.normal.z
	);

	vec3 A(-p1.d, -p2.d, -p3.d);

	matrix3 Dx = D;
	matrix3 Dy = D;
	matrix3 Dz = D;
	Dx.m[0] = A.x; Dx.m[1] = A.y; Dx.m[2] = A.z;
	Dy.m[3] = A.x; Dy.m[4] = A.y; Dy.m[5] = A.z;
	Dz.m[6] = A.x; Dz.m[7] = A.y; Dz.m[8] = A.z;

	float detD = D.det();
	if (CMP(detD, 0))
	{
		return vec3();
	}

	float detDx = Dx.det();
	float detDy = Dy.det();
	float detDz = Dz.det();

	return vec3(detDx / detD, detDy / detD, detDz / detD);
}

void GetCorners(const Frustum& f, vec3* outCorners)
{
	outCorners[0] = Intersection(f.p.znear, f.p.top,    f.p.left);
	outCorners[1] = Intersection(f.p.znear, f.p.top,    f.p.right);
	outCorners[2] = Intersection(f.p.znear, f.p.bottom, f.p.left);
	outCorners[3] = Intersection(f.p.znear, f.p.bottom, f.p.right);
	outCorners[4] = Intersection(f.p.zfar,  f.p.top,    f.p.left);
	outCorners[5] = Intersection(f.p.zfar,  f.p.top,    f.p.right);
	outCorners[6] = Intersection(f.p.zfar,  f.p.bottom, f.p.left);
	outCorners[7] = Intersection(f.p.zfar,  f.p.bottom, f.p.right);
}


bool Intersects(const Frustum& f, const vec3 &p)
{
	for (int i = 0; i < 6; ++i)
	{
		vec3 normal = f.planes[i].normal;
		float dist = f.planes[i].d;
		float side = p * normal + dist;

		if (side < 0.0f)
		{
			return false;
		}
	}
	return true;
}

bool Intersects(const Frustum& f, const sphere_t& s)
{
	for (int i = 0; i < 6; ++i)
	{
		vec3 normal = f.planes[i].normal;
		float dist = f.planes[i].d;
		float side = s.origin * normal + dist;
		if (side < -s.radius)
		{
			return false;
		}
	}
	return true;
}

float Classify(const aabb_t &aabb, const plane_t &plane)
{
	vec3 size;

	size = aabb.max - aabb.min;

	float r = abs32(size.x * plane.normal.x)
			+ abs32(size.y * plane.normal.y)
			+ abs32(size.z * plane.normal.z);

	float d = plane.normal * aabb_center_word(aabb) + plane.d;

	if (abs32(d) < r)
	{
		return 0.0f;
	}
	else if (d < 0.0f)
	{
		return d + r;
	}
	return d - r;
}

float Classify(const obb_t &obb, const plane_t &plane)
{
	vec3 normal = obb.orientation * plane.normal;

	// maximum extent in direction of plane normal
	float r = abs32(obb.size.x * normal.x)
		+ abs32(obb.size.y * normal.y)
		+ abs32(obb.size.z * normal.z);
	// signed distance between box center and plane
	float d = plane.normal * obb.origin + plane.d;
	// return signed distance
	if (abs32(d) < r)
	{
		return 0.0f;
	}
	else if (d < 0.0f)
	{
		return d + r;
	}
	return d - r;
}

bool Intersects(const Frustum& f, const aabb_t &aabb)
{
	for (int i = 0; i < 6; ++i)
	{
		if (Classify(aabb, f.planes[i]) < 0)
		{
			return false;
		}
	}

	return true;
}

bool Intersects(const Frustum& f, const obb_t &obb)
{
	for (int i = 0; i < 6; ++i)
	{
		if (Classify(obb, f.planes[i]) < 0)
		{
			return false;
		}
	}
	return true;
}


class CRigidbody
{
public:
	CRigidbody()
	{
	}

	void Update(float deltaTime)
	{
	}
	void Render()
	{
	}
	void ApplyForces()
	{
	}
	void SolveConstraints(const std::vector<obb_t>& constraints)
	{
	}
};


//=============================================================================
//	Constraint Solving
//=============================================================================


//physics system moved to bottom of file



class Particle
{
public:
	Particle()
	{
		type = RIGIDBODY_TYPE_PARTICLE;
		friction = 0.95f;
		mass = 1.0f;
		bounce = 0.7f;
	}

	void Render()
	{
		sphere_t visual;
		visual.origin = position;
		visual.radius = 0.1f;
//		::Render(visual);
	}

	void AddImpulse(const vec3& impulse)
	{
		velocity = velocity + impulse;
	}

	inline bool HasVolume()
	{
		return type == RIGIDBODY_TYPE_SPHERE || type == RIGIDBODY_TYPE_BOX;
	}

	float InvMass()
	{
		if (mass < 0.0f)
		{
			return 0.0f;
		}
		return 1.0f / mass;
	}

	void SolveConstraints(const std::vector<obb_t>& constraints)
	{
		int size = constraints.size();
		for (int i = 0; i < size; ++i)
		{
			line3_t traveled;
			traveled.a = oldPosition;
			traveled.b = position;

			if (Linetest(constraints[i], traveled))
			{
				vec3 direction = velocity.normalize();
				ray_t ray;

				ray.origin = oldPosition;
				ray.dir = direction;

				raycast_result_t result;
				if (Raycast(constraints[i], ray, &result))
				{
					position = result.point + result.normal * 0.002f;
					vec3 vn = result.normal * (result.normal * velocity);
					vec3 vt = velocity - vn;

					oldPosition = position;
					velocity = vt - vn * bounce;
					break;
				}
			}
		}
	}

	void Update(float dt)
	{
		oldPosition = position;
		vec3 acceleration = forces * InvMass();
		vec3 oldVelocity = velocity;
		velocity = velocity * friction + acceleration * dt;
		position = position + (oldVelocity + velocity) * 0.5f * dt;

		/*
		//verlet, eliminates velocity state and derives it from position - oldposition
		vec3 velocity = position - oldPosition;
		oldPosition = position;
		float deltaSquare = deltaTime * deltaTime;
		position = position + (velocity * friction + forces * deltaSquare);
		*/

	}

	void ApplyForces()
	{
		forces = vec3(0.0f, -9.8f * mass, 0.0f);
	}

public:
	int type;
	float mass;
	vec3 forces;
	vec3 velocity;
	vec3 position;
	vec3 oldPosition;
	float friction;
	float bounce;
};


//=============================================================================
//	Manifolds and Impulse
//=============================================================================
typedef struct 
{
	bool colliding;
	vec3 normal;
	float depth;
	std::vector<vec3> contacts;
} CollisionManifold;

void ResetCollisionManifold(CollisionManifold* result)
{
	if (result != 0)
	{
		result->colliding = false;
		result->normal = vec3(0, 0, 1);
		result->depth = FLT_MAX;
		result->contacts.clear();
	}
}

CollisionManifold FindCollisionFeatures(const sphere_t& A, const sphere_t& B)
{
	CollisionManifold result;
	ResetCollisionManifold(&result);

	float r = A.radius + B.radius;
	vec3 d = B.origin - A.origin;

	if (d.magnitudeSq() - r * r > 0 || d.magnitudeSq() == 0.0f)
	{
		return result;
	}

	d.normalize();
	result.colliding = true;
	result.normal = d;
	result.depth = abs32(d.magnitudeSq() - r) * 0.5f;

	// dtp - Distance to intersection point

	float dtp = A.radius - result.depth;
	vec3 contact = A.origin + d * dtp;
	result.contacts.push_back(contact);

	return result;
}

CollisionManifold FindCollisionFeatures(const obb_t &A, const sphere_t &B)
{
	CollisionManifold result;
	ResetCollisionManifold(&result);

	vec3 closestPoint = ClosestPoint(A, B.origin);

	float distanceSq = (closestPoint - B.origin).magnitudeSq();
	if (distanceSq > B.radius * B.radius)
	{
		return result;
	}

	vec3 normal;
	if (CMP(distanceSq, 0.0f))
	{
		float mSq = (closestPoint - A.origin).magnitudeSq();
		if (CMP(mSq, 0.0f)) { return result;
		}
		// Closest point is at the center of the sphere
		normal = (closestPoint - A.origin).normalize();
	}
	else
	{
		normal = (B.origin - closestPoint).normalize();
	}

	vec3 outsidePoint = B.origin - normal * B.radius;
	float distance = (closestPoint - outsidePoint).magnitude();
	result.colliding = true;
	result.contacts.push_back(closestPoint + (outsidePoint - closestPoint) * 0.5f);
	result.normal = normal;
	result.depth = distance * 0.5f;
	return result;
}


std::vector<vec3> GetVertices(const obb_t &obb)
{
	std::vector<vec3> v;
	v.resize(8);

	vec3 C = obb.origin;
	// OBB Center    
	vec3 E = obb.size;
	// OBB Extents
	const float* o = obb.orientation.m;
	vec3 A[] = {
		// OBB Axis
		vec3(o[0], o[1], o[2]),
		vec3(o[3], o[4], o[5]),
		vec3(o[6], o[7], o[8])
	};

	v[0] = C + A[0] * E.x + A[1] * E.y + A[2] * E.z;
	v[1] = C - A[0] * E.x + A[1] * E.y + A[2] * E.z;
	v[2] = C + A[0] * E.x - A[1] * E.y + A[2] * E.z;
	v[3] = C + A[0] * E.x + A[1] * E.y - A[2] * E.z;
	v[4] = C - A[0] * E.x - A[1] * E.y - A[2] * E.z;
	v[5] = C + A[0] * E.x - A[1] * E.y - A[2] * E.z;
	v[6] = C - A[0] * E.x + A[1] * E.y - A[2] * E.z;
	v[7] = C - A[0] * E.x - A[1] * E.y + A[2] * E.z;

	return v;
}

std::vector<line3_t> GetEdges(const obb_t &obb)
{
	std::vector<line3_t> result;
	result.reserve(12);

	std::vector<vec3> v = GetVertices(obb);

	int index[][2] = {
		// Indices of edge-vertices
		{6,1},{6,3},{6,4},{2,7},{2,5},{2,0},
		{0,1},{0,3},{7,1},{7,4},{4,5},{5,3}
	};


	for (int j = 0; j < 12; ++j)
	{
		line3_t line;
		line.a = v[index[j][0]];
		line.b = v[index[j][1]];
		result.push_back(line);
	}

	return result;
}


std::vector<plane_t> GetPlanes(const obb_t &obb)
{
	vec3 c = obb.origin;
	// OBB Center    
	vec3 e = obb.size;
	// OBB Extents
	const float *o = obb.orientation.m;

	vec3 a[] = {
		// OBB Axis
		vec3(o[0], o[1], o[2]),
		vec3(o[3], o[4], o[5]),
		vec3(o[6], o[7], o[8])
	};


	std::vector<plane_t> result;
	result.resize(6);

	result[0].normal = a[0];
	result[0].d = a[0] * (c + a[0] * e.x);
	result[1].normal = -a[0];
	result[1].d = -(a[0] * (c - a[0] * e.x));

	result[2].normal = a[1];
	result[2].d = a[1] * (c + a[1] * e.y);
	result[3].normal = -a[1];
	result[3].d = -(a[1] * (c - a[1] * e.y));

	result[4].normal = a[2];
	result[4].d = a[2] * (c + a[2] * e.z);
	result[5].normal = -a[2];
	result[5].d = -(a[2] * (c - a[2] * e.z));

	return result;
}

bool ClipToPlane(const plane_t& plane, const line3_t& line, vec3* outPoint)
{
	vec3 ab = line.b - line.a;
	float nAB = plane.normal * ab;
	if (CMP(nAB, 0))
	{
		return false;
	}

	float nA = plane.normal * line.a;
	float t = (plane.d - nA) / nAB;

	if (t >= 0.0f && t <= 1.0f)
	{
		if (outPoint != 0)
		{
			*outPoint = line.a + ab * t;
		}
		return true;
	}
	return false;
}

std::vector<vec3> ClipEdgesToOBB(const std::vector<line3_t>& edges, const obb_t &obb)
{
	std::vector<vec3> result;
	result.reserve(edges.size());
	vec3 intersection;

	std::vector<plane_t> planes = GetPlanes(obb);

	for (unsigned int i = 0; i < planes.size(); ++i)
	{
		for (unsigned int j = 0; j < edges.size(); ++j)
		{
			if (ClipToPlane(planes[i], edges[j], &intersection))
			{
				if (PointInOBB(intersection, obb))
				{
					result.push_back(intersection);
				}
			}
		}
	}
	return result;
}

float PenetrationDepth(const obb_t &o1, const obb_t &o2, const vec3& axis, bool* outShouldFlip)
{
	vec3 temp = axis;
	temp.normalize();

	interval_t i1 = GetInterval(o1, temp);
	interval_t i2 = GetInterval(o2, temp);

	if (!((i2.min <= i1.max) && (i1.min <= i2.max)))
	{
		return 0.0f; // No penerattion
	} 

	float len1 = i1.max - i1.min;
	float len2 = i2.max - i2.min;

	float min = MIN(i1.min, i2.min);
	float max = MAX(i1.max, i2.max);

	float length = max - min;

	if (outShouldFlip != 0)
	{
		*outShouldFlip = (i2.min < i1.min);
	}

	return (len1 + len2) - length;
}

CollisionManifold FindCollisionFeatures(const obb_t &A, const obb_t &B)
{
	CollisionManifold result;
	ResetCollisionManifold(&result);

	const float* o1 = A.orientation.m;
	const float* o2 = B.orientation.m;

	vec3 test[15] = {
		// Face axis        
		vec3(o1[0], o1[1], o1[2]),
		vec3(o1[3], o1[4], o1[5]),
		vec3(o1[6], o1[7], o1[8]),
		vec3(o2[0], o2[1], o2[2]),
		vec3(o2[3], o2[4], o2[5]),
		vec3(o2[6], o2[7], o2[8])
	};
	
	for (unsigned int i = 0; i < 3; ++i)
	{
		// Fill out rest of axis
		test[6 + i * 3 + 0] = vec3::crossproduct(test[i], test[0]);
		test[6 + i * 3 + 1] = vec3::crossproduct(test[i], test[1]);
		test[6 + i * 3 + 2] = vec3::crossproduct(test[i], test[2]);
	}

	vec3* hitNormal = 0;
	bool shouldFlip;

	for (unsigned int i = 0; i < 15; ++i)
	{
		if (test[i].magnitudeSq() < 0.001f)
		{
			continue;
		}

		float depth = PenetrationDepth(A, B, test[i], &shouldFlip);
		if (depth <= 0.0f)
		{
			return result;
		}
		else if (depth <result.depth)
		{
			if (shouldFlip)
			{
				test[i] = test[i] * -1.0f;
			}

			result.depth = depth;
			hitNormal = &test[i];
		}
	}
	if (hitNormal == 0)
	{
		return result;
	}
	vec3 axis = (*hitNormal).normalize();

	std::vector<vec3> c1 = ClipEdgesToOBB(GetEdges(B), A);
	std::vector<vec3> c2 = ClipEdgesToOBB(GetEdges(A), B);
	result.contacts.reserve(c1.size() + c2.size());
	result.contacts.insert(result.contacts.end(), c1.begin(), c1.end());
	result.contacts.insert(result.contacts.end(), c2.begin(), c2.end());

	interval_t i = GetInterval(A, axis);
	float distance = (i.max - i.min)* 0.5f - result.depth * 0.5f;
	vec3 pointOnPlane = A.origin + axis * distance;
	for (int i = result.contacts.size() - 1; i >= 0; --i)
	{
		vec3 contact = result.contacts[i];
		vec3 temp = (pointOnPlane - contact);
		result.contacts[i] = contact + (axis * (axis * temp));

		for (int j = result.contacts.size() - 1; j >i; --j)
		{
			if ((result.contacts[j] - result.contacts[i]).magnitudeSq() < 0.0001f)
			{
				result.contacts.erase(result.contacts.begin() + j);
				break;
			}
		}


	}
	result.colliding = true;
	result.normal = axis;
	return result;
}

class RigidbodyVolume
{
public:
	inline RigidbodyVolume()
	{
		type = RIGIDBODY_TYPE_BASE;
		e = 0.5f;
		mass = 1.0f;
		friction = 0.6f;
	}

	inline RigidbodyVolume(int bodyType)
	{
		e = 0.5f;
		mass = 1.0f;
		friction = 0.6f;
		type = bodyType;
	}

	void Render();
	void Update(float dt);
	void ApplyForces();

	void SynchCollisionVolumes();
	float InvMass();
	void AddLinearImpulse(const vec3& impulse);

	matrix4 InvTensor();
	virtual void AddRotationalImpulse(const vec3& point, const vec3& impulse);


public:
	int type;
	vec3 position;
	vec3 velocity;
	vec3 forces; // Sum of all forces
	float mass; // Coefficient of restitution
	float e;
	float friction;
	obb_t box;
	sphere_t sphere;

	vec3 orientation;
	vec3 angVel;
	vec3 torques; // Sum torques
};

void RigidbodyVolume::ApplyForces()
{
	forces = vec3(0.0f, -9.8f * mass, 0.0f);
}

void RigidbodyVolume::AddLinearImpulse(const vec3& impulse)
{
	velocity = velocity + impulse;
}

float RigidbodyVolume::InvMass()
{ 
	if (mass == 0.0f)
	{
		return 0.0f;
	}
	return 1.0f / mass;
}

matrix4 RigidbodyVolume::InvTensor()
{
	float ix = 0.0f;
	float iy = 0.0f;
	float iz = 0.0f;
	float iw = 0.0f;

	if (mass != 0 && type == RIGIDBODY_TYPE_SPHERE)
	{
		float r2 = sphere.radius * sphere.radius;
		float fraction = (2.0f / 5.0f);
		ix = r2 * mass * fraction;
		iy = r2 * mass * fraction;
		iz = r2 * mass * fraction;
		iw = 1.0f;
	}
	else if (mass != 0 && type == RIGIDBODY_TYPE_BOX)
	{
		vec3 size = box.size * 2.0f;
		float fraction = (1.0f / 12.0f);
		float x2 = size.x * size.x;
		float y2 = size.y * size.y;
		float z2 = size.z * size.z;
		ix = (y2 + z2) * mass * fraction;
		iy = (x2 + z2) * mass * fraction;
		iz = (x2 + y2) * mass * fraction;
		iw = 1.0f;
	}

	return matrix4(
		ix, 0, 0, 0,
		0, iy, 0, 0,
		0, 0, iz, 0,
		0, 0, 0, iw).inverse();
}

void RigidbodyVolume::AddRotationalImpulse(const vec3& point, const vec3& impulse)
{
	vec3 centerOfMass = position;
	vec3 torque = vec3::crossproduct(point - centerOfMass, impulse);

	vec3 angAccel = vec3(InvTensor() * vec4(torque.x, torque.y, torque.z, 1.0f));
	angVel = angVel + angAccel;
}

void RigidbodyVolume::SynchCollisionVolumes()
{
	sphere.origin = position;
	box.origin = position;

	box.orientation = Rotation3x3(
		RAD2DEG(orientation.x),
		RAD2DEG(orientation.y),
		RAD2DEG(orientation.z)
	);
}

void RigidbodyVolume::Render()
{
	SynchCollisionVolumes();
	if (type == RIGIDBODY_TYPE_SPHERE)
	{
		//::Render(sphere);
	}
	else if (type == RIGIDBODY_TYPE_BOX)
	{
		//::Render(box);
	}
}

void RigidbodyVolume::Update(float dt)
{
	const float damping = 0.98f;
	vec3 acceleration = forces * InvMass();
	velocity += acceleration * dt;
	velocity *= damping;

	position += velocity * dt;
	SynchCollisionVolumes();

	if (type == RIGIDBODY_TYPE_BOX)
	{
		vec3 angAccel = vec3(InvTensor() * vec4(torques.x, torques.y, torques.z, 1.0f));
		angVel = angVel + angAccel * dt;
		angVel = angVel * damping;
		orientation = orientation + angVel * dt;
	}
}

void ApplyImpulse(RigidbodyVolume& A, RigidbodyVolume& B, const CollisionManifold& M, int c)
{
	float invMass1 = A.InvMass();
	float invMass2 = B.InvMass();
	float invMassSum = invMass1 + invMass2;
	if (invMassSum == 0.0f)
	{
		return; // Both objects have infinate mass!
	}

	vec3 r1 = M.contacts[c] - A.position;
	vec3 r2 = M.contacts[c] - B.position;

	matrix4 i1 = A.InvTensor();
	matrix4 i2 = B.InvTensor();

	// Relative velocity
	vec3 relativeVel = (B.velocity + vec3::crossproduct(B.angVel, r2)) - (A.velocity + vec3::crossproduct(A.angVel, r1));
	// Relative collision normal
	vec3 relativeNorm = M.normal;
	relativeNorm.normalize();

	if (relativeVel * relativeNorm > 0.0f)
	{
		return;
	}

	float e = MIN(A.e, B.e);
	float numerator = (relativeVel * relativeNorm) * -(1.0f + e);
	float d1 = invMassSum;
	vec3 temp1 = vec3::crossproduct(r1, relativeNorm);
	vec3 d2 = vec3::crossproduct(vec3(i1 * vec4(temp1.x, temp1.y, temp1.z, 1.0f)), r1);
	vec3 temp2 = vec3::crossproduct(r2, relativeNorm);
	vec3 d3 = vec3::crossproduct(vec3(i2 * vec4(temp2.x, temp2.y, temp2.z, 1.0f)), r2);

	float denominator = d1 + relativeNorm * (d2 + d3);
	float j = (denominator == 0.0f) ? 0.0f : numerator / denominator;
	if (M.contacts.size() > 0.0f && j != 0.0f)
	{
		j /= (float)M.contacts.size();
	}

	vec3 impulse = relativeNorm * j;
	A.velocity = A.velocity - impulse * invMass1;
	B.velocity = B.velocity + impulse * invMass2;

	temp1 = vec3::crossproduct(r1, impulse);
	temp2 = vec3::crossproduct(r2, impulse);
	A.angVel = A.angVel - i1 * vec4(temp1.x, temp1.y, temp1.z, 1.0f);
	B.angVel = B.angVel + i2 * vec4(temp2.x, temp2.y, temp2.z, 1.0f);

	vec3 t = relativeVel - (relativeNorm * (relativeVel * relativeNorm));

	if (CMP(t.magnitudeSq(), 0.0f))
	{
		return;
	}
	t.normalize();

	numerator = -(relativeVel * t);
	d1 = invMassSum;

	temp1 = vec3::crossproduct(r1, t);
	temp2 = vec3::crossproduct(r2, t);

	d2 = vec3::crossproduct(vec3(i1 * vec4(temp1.x, temp1.y, temp1.z, 1.0f)), r1);
	d3 = vec3::crossproduct(vec3(i2 * vec4(temp2.x, temp2.y, temp2.z, 1.0f)), r2);
	denominator = d1 + t * (d2 + d3);

	if (denominator == 0.0f)
	{
		return;
	}

	float jt = numerator / denominator;
	if (M.contacts.size() > 0.0f &&jt != 0.0f)
	{
		jt /= (float)M.contacts.size();
	}

	if (CMP(jt, 0.0f))
	{
		return;
	}

	float friction = newtonSqrt(A.friction * B.friction);
	if (jt > j * friction)
	{
		jt = j * friction;
	}
	else if (jt < -j * friction)
	{
		jt = -j * friction;
	}

	vec3 tangentImpuse = t * jt;
	A.velocity = A.velocity - tangentImpuse * invMass1;
	B.velocity = B.velocity + tangentImpuse * invMass2;

	temp1 = vec3::crossproduct(r1, tangentImpuse);
	temp2 = vec3::crossproduct(r2, tangentImpuse);


	A.angVel = A.angVel - i1 * vec4(temp1.x, temp1.y, temp1.z, 1.0f);
	B.angVel = B.angVel + i2 * vec4(temp2.x, temp2.y, temp2.z, 1.0f);


}

CollisionManifold FindCollisionFeatures(RigidbodyVolume& ra, RigidbodyVolume& rb)
{
	CollisionManifold result;
	ResetCollisionManifold(&result);
	if (ra.type == RIGIDBODY_TYPE_SPHERE)
	{
		if (rb.type == RIGIDBODY_TYPE_SPHERE)
		{
			result = FindCollisionFeatures(ra.sphere, rb.sphere);
		}
		else if (rb.type == RIGIDBODY_TYPE_BOX)
		{
			result = FindCollisionFeatures(rb.box, ra.sphere);
			result.normal = result.normal * -1.0f;
		}
	}
	else if (ra.type == RIGIDBODY_TYPE_BOX)
	{
		if (rb.type == RIGIDBODY_TYPE_BOX)
		{
			result = FindCollisionFeatures(ra.box, rb.box);
		}
		else if (rb.type == RIGIDBODY_TYPE_SPHERE)
		{
			result = FindCollisionFeatures(ra.box, rb.sphere);
		}
	}
	return result;
}


//=============================================================================
//	Springs and Joints
//=============================================================================

class Spring
{
protected:
	Particle* p1;
	Particle* p2;
	// higher k = stiff sprint, lower k = loose spring
	float k; // [-n to 0]
	float b; // [0 to 1], default to 0
	float restingLength;
	
public:
	inline Spring(float _k, float _b, float len) : k(_k), b(_b), restingLength(len)
	{
	}

	void SetParticles(Particle* _p1, Particle* _p2)
	{
		p1 = _p1;
		p2 = _p2; 
	}

	Particle* GetP1()
	{
		return p1;
	}

	Particle* GetP2()
	{
		return p2;
	}

	void SetConstants(float _k, float _b)
	{
		k = _k;
		b = _b;
	}

	void ApplyForce(float dt)
	{
		vec3 relPos = p2->position - p1->position;
		vec3 relVel = p2->velocity - p1->velocity;

		float x = relPos.magnitude() - restingLength;
		float v = relVel.magnitude();

		float F = (-k * x) + (-b * v);

		vec3 impulse = relPos.normalize() * F;

		p1->AddImpulse(impulse * p1->InvMass());
		p2->AddImpulse(impulse*  -1.0f * p2->InvMass());
	}

};


class Cloth
{
public:
	void Initialize(unsigned int gridSize, float distance, const vec3& position);
	void SetStructuralSprings(float k, float b);
	void SetShearSprings(float k, float b);
	void SetBendSprings(float k, float b);
	void SetParticleMass(float mass);

	void ApplyForces();
	void Update(float dt);
	void ApplySpringForces(float dt);
	void SolveConstraints(const vector<obb_t> &constraints);

	void Render();

protected:
	vector<Particle> verts;
	vector<Spring> structural;
	vector<Spring> shear;
	vector<Spring> bend;
	float clothSize;
};

void Cloth::Initialize(unsigned int gridSize, float distance, const vec3& position)
{
	float k = -1.0f;
	float b = 0.0f;
	clothSize = gridSize;

	verts.clear();
	structural.clear();
	shear.clear();
	bend.clear();

	verts.resize(gridSize * gridSize);

	float hs = (float)(gridSize - 1) * 0.5f;

	if (gridSize < 3)
	{
		gridSize = 3;
	}

	for (unsigned int x = 0; x < gridSize; ++x)
	{
		for (unsigned int z = 0; z < gridSize; ++z)
		{
			int i = z * gridSize + x;
			float x_pos = ((float)x + position.x - hs) * distance;
			float z_pos = ((float)z + position.z - hs) * distance;

			verts[i].position =vec3(x_pos, position.y, z_pos);
			verts[i].mass = 1.0f;
			verts[i].bounce = 0.0f;
			verts[i].friction = 0.9f;
		}
	}

	for (unsigned int x = 0; x < gridSize; ++x)
	{
		for (unsigned int z = 0; z < gridSize - 1; ++z)
		{
			int i = z * gridSize + x;
			int j = (z + 1) * gridSize + x;

			vec3 iPos = verts[i].position;
			vec3 jPos = verts[j].position;
			vec3 dist = (iPos - jPos);
			float rest = dist.magnitude();

			Spring spring(k, b, rest);
			spring.SetParticles(&verts[i], &verts[j]);
			structural.push_back(spring);
		}
	}

	for (unsigned int x = 0; x < gridSize - 1; ++x)
	{
		for (unsigned int z = 0; z < gridSize; ++z)
		{
			int i = z * gridSize + x;
			int j = z * gridSize + (x + 1);

			vec3 iPos = verts[i].position;
			vec3 jPos = verts[j].position;
			vec3 dist = (iPos - jPos);
			float rest = dist.magnitude();

			Spring spring(k, b, rest);
			spring.SetParticles(&verts[i], &verts[j]);
			structural.push_back(spring);
		}
	}

	for (unsigned int x = 0; x < gridSize - 1; ++x)
	{
		for (unsigned int z = 0; z < gridSize - 1; ++z)
		{
			int i = z * gridSize + x;
			int j = (z + 1) * gridSize + (x + 1);

			vec3 iPos = verts[i].position;
			vec3 jPos = verts[j].position;
			vec3 dist = (iPos - jPos);
			float rest = dist.magnitude();

			Spring spring(k, b, rest);
			spring.SetParticles(&verts[i], &verts[j]);
			shear.push_back(spring);
		}
	}

	for (unsigned int x = 1; x < gridSize; ++x)
	{
		for (unsigned int z = 0; z < gridSize - 1; ++z)
		{
			int i = z * gridSize + x;
			int j = (z + 1) * gridSize + (x - 1);

			vec3 iPos = verts[i].position;
			vec3 jPos = verts[j].position;
			vec3 dist = (iPos - jPos);
			float rest = dist.magnitude();

			Spring spring(k, b, rest);
			spring.SetParticles(&verts[i], &verts[j]);
			shear.push_back(spring);
		}
	}

	for (unsigned int x = 0; x < gridSize; ++x)
	{
		for (unsigned int z = 0; z < gridSize - 2; ++z)
		{
			int i = z * gridSize + x;
			int j = (z + 2) * gridSize + x;

			vec3 iPos = verts[i].position;
			vec3 jPos = verts[j].position;
			vec3 dist = (iPos - jPos);
			float rest = dist.magnitude();

			Spring spring(k, b, rest);
			spring.SetParticles(&verts[i], &verts[j]);
			bend.push_back(spring);
		}
	}

	for (unsigned int x = 0; x < gridSize - 2; ++x)
	{
		for (unsigned int z = 0; z < gridSize; ++z)
		{
			int i = z * gridSize + x; 
			int j = z * gridSize + (x + 2);

			vec3 iPos = verts[i].position;
			vec3 jPos = verts[j].position;
			vec3 dist = (iPos - jPos);
			float rest = dist.magnitude();

			Spring spring(k, b, rest);
			spring.SetParticles(&verts[i], &verts[j]);
			bend.push_back(spring);
		}
	}
}

void Cloth::SetStructuralSprings(float k, float b)
{
	for (unsigned int i = 0; i < structural.size(); ++i)
	{
		structural[i].SetConstants(k, b);
	}
}

void Cloth::SetShearSprings(float k, float b)
{
	for (unsigned int i = 0, size = shear.size(); i < size; ++i)
	{
		shear[i].SetConstants(k, b);
	}
}

void Cloth::SetBendSprings(float k, float b)
{
	for (unsigned int i = 0, size = bend.size(); i < size; ++i)
	{
		bend[i].SetConstants(k, b);
	}
}

void Cloth::SetParticleMass(float mass)
{
	for (unsigned int i = 0, size = verts.size(); i< size; ++i)
	{
		verts[i].mass = mass;
	}
}

void Cloth::ApplyForces()
{
	for (unsigned int i = 0, size = verts.size(); i< size; ++i)
	{
		verts[i].ApplyForces();
	}
}

void Cloth::Update(float dt)
{
	for (unsigned int i = 0, size = verts.size(); i< size; ++i)
	{
		verts[i].Update(dt);
	}
}

void Cloth::ApplySpringForces(float dt)
{
	for (unsigned int i = 0; i < structural.size(); ++i)
	{
		structural[i].ApplyForce(dt);
	}
	
	for (unsigned int i = 0, size = shear.size(); i < size; ++i)
	{
		shear[i].ApplyForce(dt);
	}
	
	for (unsigned int i = 0, size = bend.size(); i < size; ++i)
	{
		bend[i].ApplyForce(dt);
	}
}

void Cloth::SolveConstraints(const std::vector<obb_t> &constraints)
{
	for (unsigned int i = 0, size = verts.size(); i< size; ++i)
	{
		verts[i].SolveConstraints(constraints);
	}
}

void Cloth::Render()
{
	for (unsigned int x = 0; x < clothSize - 1; ++x)
	{
		for (unsigned int z = 0; z < clothSize - 1; ++z)
		{
			int tl = z * clothSize + x;
			int bl = (z + 1) * clothSize + x;
			int tr = z * clothSize + (x + 1);
			int br = (z + 1) * clothSize + (x + 1);

#ifdef OLDOPENGL
			glBegin(GL_TRIANGLES);
			glVertex3f(verts[tl].position.x, verts[tl].position.y, verts[tl].position.z);
			glVertex3f(verts[br].position.x, verts[br].position.y, verts[br].position.z);
			glVertex3f(verts[bl].position.x, verts[bl].position.y, verts[bl].position.z);

			glVertex3f(verts[tl].position.x, verts[tl].position.y, verts[tl].position.z);
			glVertex3f(verts[tr].position.x, verts[tr].position.y, verts[tr].position.z);
			glVertex3f(verts[br].position.x, verts[br].position.y, verts[br].position.z);
			glEnd();
#endif
		}
	}
}


class DistanceJoint/* : public Rigidbody*/
{
protected:
	Particle* p1;
	Particle* p2;
	float length;
public:    
	void Initialize(Particle* _p1, Particle* _p2, float len);
	void SolveConstraints(const std::vector<obb_t> &constraints);
	void Render();
};

void DistanceJoint::Initialize(Particle* _p1, Particle* _p2, float len)
{
	p1 = _p1;
	p2 = _p2;
	length = len;
}

void DistanceJoint::Render()
{
	vec3 pos1 = p1->position;
	vec3 pos2 = p2->position;
	line3_t l;
	l.a = pos1;
	l.b = pos2;

	//render line
#ifdef OLDOPENGL
	glBegin(GL_LINES);
	glVertex3f(l.a.x, l.a.y, l.a.z);
	glVertex3f(l.b.x, l.b.y, l.b.z);
	glEnd();
#endif
}

void DistanceJoint::SolveConstraints(const std::vector<obb_t>& constraints)
{
	vec3 delta = p2->position - p1->position;
	float distance = delta.magnitude();

	float correction = (distance - length) / distance;

	p1->position = p1->position + delta * 0.5f * correction;
	p2->position = p2->position + delta * 0.5f * correction;

	p1->SolveConstraints(constraints);
	p2->SolveConstraints(constraints);
}




class PhysicsSystem
{
protected:
	std::vector<CRigidbody*> bodies;
	std::vector<obb_t> constraints;
	std::vector<CRigidbody*> colliders1;
	std::vector<CRigidbody*> colliders2;
	std::vector<CollisionManifold> results;

	float LinearProjectionPercent;
	float PenetrationSlack;
	// [1 to 20], Larger = more accurate
	int ImpulseIteration;
public:
	PhysicsSystem();
	void Update(float deltaTime);
	void Render();
	void AddRigidbody(CRigidbody* body);
	void AddConstraint(const obb_t &constraint);
	void ClearRigidbodys();
	void ClearConstraints();
};



PhysicsSystem::PhysicsSystem()
{
	LinearProjectionPercent = 0.45f;
	PenetrationSlack = 0.01f;
	ImpulseIteration = 5;
	colliders1.reserve(100);
	colliders2.reserve(100);
	results.reserve(100);
}

void PhysicsSystem::AddRigidbody(CRigidbody* body)
{
	bodies.push_back(body);
}

void PhysicsSystem::AddConstraint(const obb_t& obb)
{
	constraints.push_back(obb);
}

void PhysicsSystem::ClearRigidbodys()
{
	bodies.clear();
}

void PhysicsSystem::ClearConstraints()
{
	constraints.clear();
}

void PhysicsSystem::Render()
{
	static const float rigidbodyDiffuse[] = {
		200.0f / 255.0f, 0.0f, 0.0f, 0.0f
	};
	static const float rigidbodyAmbient[] = {
		200.0f / 255.0f, 50.0f / 255.0f, 50.0f / 255.0f, 0.0f
	};
	static const float constraintDiffuse[] = {
		0.0f, 200.0f / 255.0f, 0.0f, 0.0f
	};
	static const float constraintAmbient[] = {
		50.0f / 255.0f, 200.0f / 255.0f, 50.0f / 255.0f, 0.0f
	};
	static const float zero[] = {
		0.0f, 0.0f, 0.0f, 0.0f
	};

#ifdef OLDOPENGL
	glColor3f(rigidbodyDiffuse[0], rigidbodyDiffuse[1], rigidbodyDiffuse[2]);
	glLightfv(GL_LIGHT0, GL_AMBIENT, rigidbodyAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, rigidbodyDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, zero);
#endif

	for (int i = 0, size = bodies.size(); i < size; ++i)
	{
		bodies[i]->Render();
	}

#ifdef OLDOPENGL
	glColor3f(constraintDiffuse[0], constraintDiffuse[1], constraintDiffuse[2]);
	glLightfv(GL_LIGHT0, GL_AMBIENT, constraintAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, constraintDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, zero);
#endif


	for (unsigned int i = 0; i < constraints.size(); ++i)
	{
		//		::Render(constraints[i]);
	}
}

void PhysicsSystem::Update(float deltaTime)
{
	colliders1.clear();
	colliders2.clear();
	results.clear();

	for (int i = 0, size = bodies.size(); i < size; ++i)
	{
		for (int j = i; j < size; ++j)
		{
			if (i == j)
			{
				continue;
			}
			CollisionManifold result;
			ResetCollisionManifold(&result);

			if (1 /*bodies[i]->HasVolume() && bodies[j]->HasVolume()*/)
			{
				RigidbodyVolume* m1 = (RigidbodyVolume*)bodies[i];
				RigidbodyVolume* m2 = (RigidbodyVolume*)bodies[j];
				result = FindCollisionFeatures(*m1, *m2);
			}

			if (result.colliding)
			{
				colliders1.push_back(bodies[i]);
				colliders2.push_back(bodies[j]);
				results.push_back(result);
			}
		}

		for (int i = 0, size = bodies.size(); i < size; ++i)
		{
			bodies[i]->ApplyForces();
		}

		for (int k = 0; k < ImpulseIteration; ++k)
		{
			for (unsigned int i = 0; i < results.size(); ++i)
			{
				int jSize = results[i].contacts.size();
				for (int j = 0; j < jSize; ++j)
				{
					RigidbodyVolume* m1 = (RigidbodyVolume*)colliders1[i];
					RigidbodyVolume* m2 = (RigidbodyVolume*)colliders2[i];
					ApplyImpulse(*m1, *m2, results[i], j);
				}
			}
		}

		for (int i = 0, size = bodies.size(); i < size; ++i)
		{
			bodies[i]->Update(deltaTime);
		}

		for (int i = 0, size = results.size(); i < size; ++i)
		{
			RigidbodyVolume* m1 = (RigidbodyVolume*)colliders1[i];
			RigidbodyVolume* m2 = (RigidbodyVolume*)colliders2[i];
			float totalMass = m1->InvMass() + m2->InvMass();

			if (totalMass == 0.0f)
			{
				continue;
			}

			float depth = MAX(results[i].depth - PenetrationSlack, 0.0f);
			float scalar = depth / totalMass;
			vec3 correction = results[i].normal * scalar * LinearProjectionPercent;

			m1->position = m1->position - correction * m1->InvMass();
			m2->position = m2->position + correction * m2->InvMass();

			m1->SynchCollisionVolumes();
			m2->SynchCollisionVolumes();
		}

		for (int i = 0, size = bodies.size(); i < size; ++i)
		{
			bodies[i]->SolveConstraints(constraints);
		}
	}
}

// get the minimum distance between 2 line segments
float distance_segment_segment(line3_t &s1, line3_t &s2)
{
	vec3	u = s1.b - s1.a;
	vec3	v = s2.b - s2.a;
	vec3	w = s1.a - s2.a;
	float	a = u * u;
	float	b = u * v;
	float	c = v * v;
	float	d = u * w;
	float	e = v * w;
	float	D = a *  c - b*  b;
	float	sc, sN;
	float	sD = D;		// sc = sN / sD, default sD = D >= 0
	float	tc, tN;
	float	tD = D;		// tc = tN / tD, default tD = D >= 0

	// compute the line parameters of the two closest points
	if (D < 0.001f)
	{
		// the lines are almost parallel
		sN = 0.0;         // force using point P0 on segment S1
		sD = 1.0;         // to prevent possible division by 0.0 later
		tN = e;
		tD = c;
	}
	else
	{
		// get the closest points on the infinite lines
		sN = b * e - c * d;
		tN = a * e - b * d;
		if (sN < 0.0)
		{
			// sc < 0 => the s=0 edge is visible
			sN = 0.0;
			tN = e;
			tD = c;
		}
		else if (sN > sD)
		{
			// sc > 1  => the s=1 edge is visible
			sN = sD;
			tN = e + b;
			tD = c;
		}
	}

	if (tN < 0.0)
	{            // tc < 0 => the t=0 edge is visible
		tN = 0.0;
		// recompute sc for this edge
		if (-d < 0.0)
		{
			sN = 0.0;
		}
		else if (-d > a)
		{
			sN = sD;
		}
		else
		{
			sN = -d;
			sD = a;
		}
	}
	else if (tN > tD)
	{
		// tc > 1  => the t=1 edge is visible
		tN = tD;
		// recompute sc for this edge
		if ((-d + b) < 0.0)
		{
			sN = 0;
		}
		else if ((-d + b) > a)
		{
			sN = sD;
		}
		else
		{
			sN = (-d + b);
			sD = a;
		}
	}
	// finally do the division to get sc and tc
	sc = (abs32(sN) < 0.001f ? 0.0 : sN / sD);
	tc = (abs32(tN) < 0.001f ? 0.0 : tN / tD);

	// get the difference of the two closest points
	vec3 dP = w + (u * sc) - (v * tc);  // =  S1(sc) - S2(tc)

	return dP.magnitude();   // return the closest distance
}

bool IntersectCapsule(vec3 &eye, vec3 &dir, vec3 &capsule_a, vec3 &capsule_b, float radius)
{
	vec3 end = eye + (dir * 8192);
	line3_t segment1;
	line3_t segment2;

	segment1.a = eye;
	segment1.b = end;
	segment2.a = capsule_a;
	segment2.b = capsule_b;
	float dist = distance_segment_segment(segment1, segment2);

	if (dist < radius)
	{
		return true;
	}

	return false;
}
