#include "physics.h"
#include <list>
#include "sin_table.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Floating point comparison function (epsilon issues)
#define CMP(x, y) (abs32((x)-(y)) <= 0.00001f * MAX(1.0f, MAX(abs32(x), abs32(y))) )


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

	for (int i = 1; i < 8; ++i)
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
	for (int i = 1; i < 8; ++i)
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

	for (int i = 0; i < 3; ++i) { // Fill out rest of axis
		test[6 + i * 3 + 0] = vec3::crossproduct(test[i], test[0]);
		test[6 + i * 3 + 1] = vec3::crossproduct(test[i], test[1]);
		test[6 + i * 3 + 2] = vec3::crossproduct(test[i], test[2]);
	}

	for (int i = 0; i < 15; ++i)
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

	for (int i = 0; i < 3; ++i)
	{
		// Fill out rest of axis
		test[6 + i * 3 + 0] = vec3::crossproduct(test[i], test[0]);
		test[6 + i * 3 + 1] = vec3::crossproduct(test[i], test[1]);
		test[6 + i * 3 + 2] = vec3::crossproduct(test[i], test[2]);
	}


	for (int i = 0; i < 15; ++i)
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

		for (int i = 0; i < 6; ++i)
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
		for (int i = 0; i < 6; ++i)
		{
			if (CMP(t_result, t[i]))
			{
				result->normal = normals[i].normalize();
			}
		}
	}

	return true;
}

float Raycast(const plane_t plane, const ray_t &ray)
{
	float nd = ray.dir * plane.normal;
	float pn = ray.origin * plane.normal;


	if (nd >= 0.0f)
	{
		return -1;
	}

	float t = (plane.d - pn) / nd;

	if (t >= 0.0f)
	{
		return t;
	}

	return -1;
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
	for (int i = 0; i < 13; ++i)
	{
		if (!OverlapOnAxis(a, t, test[i]))
		{
			return false; // Separating axis found
		}
	}
	return true; // Separating axis not found
}

#define OBBTriangle(o, t) \    TriangleOBB(t, o)

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

	for (int i = 0; i < 13; ++i)
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

bool TrianglePlane(const triangle_t &t, const vec3 &normal, float d)
{
	float side1 = PlaneEquation(t.a, normal, d);
	float side2 = PlaneEquation(t.b, normal, d);
	float side3 = PlaneEquation(t.c, normal, d);

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

	for (int i = 0; i < 11; ++i)
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

	for (int i = 0; i < 11; ++i) 
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

float Raycast(const triangle_t &triangle, const ray_t &ray)
{
	plane_t plane;
	FromTriangle(triangle, plane);
	float t = Raycast(plane, ray);
	if (t < 0.0f)
	{
		return t;
	}

	vec3 result = ray.origin + ray.dir * t;

	vec3 barycentric = Barycentric(result, triangle);
	if (barycentric.x >= 0.0f && barycentric.x <= 1.0f &&
		barycentric.y >= 0.0f && barycentric.y <= 1.0f &&
		barycentric.z >= 0.0f && barycentric.z <= 1.0f)
	{
		return t;
	}

	return -1;
}

bool Linetest(const triangle_t &triangle, const line3_t &line)
{
	ray_t ray;
	ray.origin = line.a;
	vec3 length = line.b - line.a;
	ray.dir = length.normalize();

	float t = Raycast(triangle, ray);
	return t >= 0 && t * t <= (length * length);
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


vec3 aabb_center_word(aabb_t &aabb)
{
	return vec3(
		(aabb.max.x - aabb.min.x) * 0.5f + aabb.min.x,
		(aabb.max.y - aabb.min.y) * 0.5f + aabb.min.y,
		(aabb.max.z - aabb.min.z) * 0.5f + aabb.min.z
	);
}

vec3 aabb_center_model(aabb_t &aabb)
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
		for (int i = 0; i < 8; ++i)  // For each child
		{
			node->children[i].numTriangles = 0;
			for (int j = 0; j < node->numTriangles; ++j)
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

			for (int j = 0; j < node->numTriangles; ++j)
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

		for (int i = 0; i < 8; ++i)
		{
			SplitBVHNode(&node->children[i], model, depth);
		}
	}
}

void FreeBVHNode(bvh_node_t* node)
{
	if (node->children != 0)
	{
		for (int i = 0; i < 8; ++i)
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
	for (int i = 1; i < mesh.numTriangles * 3; ++i)
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

	for (int i = 0; i < mesh.numTriangles; ++i)
	{
		mesh.accelerator->triangles[i] = i;
	}

	// Recursively split BVH tree
	SplitBVHNode(mesh.accelerator, mesh, 3);
}

float MeshRay(const mesh_t &mesh, const ray_t &ray)
{
	if (mesh.accelerator == 0)
	{
		for (int i = 0; i < mesh.numTriangles; ++i)
		{
			float result = Raycast(mesh.triangles[i], ray);
			if (result >= 0)
			{
				return result;
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
			bvh_node_s* iterator = *(toProcess.begin());
			toProcess.erase(toProcess.begin());

			if (iterator->numTriangles >= 0)
			{
				for (int i = 0; i< iterator->numTriangles; ++i)
				{
					// Do a raycast against the triangle
					float r = Raycast(mesh.triangles[iterator->triangles[i]], ray);
					if (r >= 0)
					{
						return r;
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
						toProcess.push_front(
							&iterator->children[i]);
					}
				}
			}
		}
	}
	return -1;
}

bool MeshAABB(const mesh_t &mesh, const aabb_t &aabb)
{
	if (mesh.accelerator == 0)
	{
		for (int i = 0; i < mesh.numTriangles; ++i)
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
				for (int i = 0; i<iterator->numTriangles; ++i)
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

//=============================================================================
//	Models and Scenes
//=============================================================================

//=============================================================================
//	Camera and Frustum
//=============================================================================

//=============================================================================
//	Models and Scenes
//=============================================================================


//=============================================================================
//	Constraint Solving
//=============================================================================

class Particle
{
public:
	void AddImpulse(const vec3& impulse)
	{
		velocity = velocity + impulse;
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
		/* OLD: vec3 acceleration = forces * (1.0f / mass); */
		/* NEW: */
		vec3 acceleration = forces * InvMass();
		velocity = velocity * friction + acceleration * dt;
		position = position + velocity * dt;
	}

	void ApplyForces()
	{
		forces = vec3(0.0f, -9.8f * mass, 0.0f);
	}

public:
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
	void Initialize(int gridSize, float distance, const vec3& position);
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

void Cloth::Initialize(int gridSize, float distance, const vec3& position)
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

	for (int x = 0; x < gridSize; ++x)
	{
		for (int z = 0; z < gridSize; ++z)
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

	for (int x = 0; x < gridSize; ++x)
	{
		for (int z = 0; z < gridSize - 1; ++z)
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

	for (int x = 0; x < gridSize - 1; ++x)
	{
		for (int z = 0; z < gridSize; ++z)
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

	for (int x = 0; x < gridSize - 1; ++x)
	{
		for (int z = 0; z < gridSize - 1; ++z)
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

	for (int x = 1; x < gridSize; ++x)
	{
		for (int z = 0; z < gridSize - 1; ++z)
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

	for (int x = 0; x < gridSize; ++x)
	{
		for (int z = 0; z < gridSize - 2; ++z)
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

	for (int x = 0; x < gridSize - 2; ++x)
	{
		for (int z = 0; z < gridSize; ++z)
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
	for (int i = 0, size = shear.size(); i < size; ++i)
	{
		shear[i].SetConstants(k, b);
	}
}

void Cloth::SetBendSprings(float k, float b)
{
	for (int i = 0, size = bend.size(); i < size; ++i)
	{
		bend[i].SetConstants(k, b);
	}
}

void Cloth::SetParticleMass(float mass)
{
	for (int i = 0, size = verts.size(); i< size; ++i)
	{
		verts[i].mass = mass;
	}
}

void Cloth::ApplyForces()
{
	for (int i = 0, size = verts.size(); i< size; ++i)
	{
		verts[i].ApplyForces();
	}
}

void Cloth::Update(float dt)
{
	for (int i = 0, size = verts.size(); i< size; ++i)
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
	for (int i = 0, size = verts.size(); i< size; ++i)
	{
		verts[i].SolveConstraints(constraints);
	}
}

void Cloth::Render()
{
	for (int x = 0; x < clothSize - 1; ++x)
	{
		for (int z = 0; z < clothSize - 1; ++z)
		{
			int tl = z * clothSize + x;
			int bl = (z + 1) * clothSize + x;
			int tr = z * clothSize + (x + 1);
			int br = (z + 1) * clothSize + (x + 1);

			glBegin(GL_TRIANGLES);
			glVertex3f(verts[tl].position.x, verts[tl].position.y, verts[tl].position.z);
			glVertex3f(verts[br].position.x, verts[br].position.y, verts[br].position.z);
			glVertex3f(verts[bl].position.x, verts[bl].position.y, verts[bl].position.z);

			glVertex3f(verts[tl].position.x, verts[tl].position.y, verts[tl].position.z);
			glVertex3f(verts[tr].position.x, verts[tr].position.y, verts[tr].position.z);
			glVertex3f(verts[br].position.x, verts[br].position.y, verts[br].position.z);
			glEnd();
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
	glBegin(GL_LINES);
	glVertex3f(l.a.x, l.a.y, l.a.z);
	glVertex3f(l.b.x, l.b.y, l.b.z);
	glEnd();
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
