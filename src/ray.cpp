#include "ray.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define CMP(x, y) (abs32((x)-(y)) <= FLT_EPSILON * MAX(1.0f, MAX(fabsf(x), fabsf(y))) )



bool PointOnLine(const vec3 &p, const line2_t &line)
{
	float dy = (line.b.y - line.a.y);
	float dx = (line.b.x - line.a.x);
	float m = dy / dx; 
	float B = line.a.y - m * line.a.x;

	return CMP(p.y, m * p.x + B);
}

bool PointInCircle(const vec2 &point, const circle_t &c)
{
	line2_t line;
	
	line.a = point;
	line.b = c.origin;
	vec2 length = line.a - line.b;

	if (length * length < c.radius * c.radius)
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


bool PointInOrientedRectangle(const vec2 &point, const box2_t &rectangle)
{
	vec2 rotVector = point - rectangle.origin;
	float theta = -(rectangle.angle);
	float zRotation2x2[] = {
		cosf(theta), sinf(theta),
		-sinf(theta), cosf(theta)
	};

	//Multiply(rotVector.asArray, vec2(rotVector.x, rotVector.y).asArray, 1, 2, zRotation2x2, 2, 2);

	rect2_t local;
	
	local.min = vec2();
	//local.max = rectangle.half_length * 2.0f;

//	vec2 localPoint = rotVector + rectangle.half_length;
	//return PointInRectangle(localPoint, local);
	return false;
}

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

vec3 ClosestPoint(const vec3 &point, const obb_t &obb)
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
	return dot - d == 0.0f;
}

vec3 ClosestPoint(const vec3 &point, vec3 &normal, float d)
{
	float dot = point * normal;
	float distance = dot - d;
	return point - normal * d;
}


vec3 ClosestPoint(const line3_t &line, const vec3 &point)
{
	vec3 lVec = line.b - line.a;
	vec3 dist = point - line.a;

	float t = dist * lVec  / (lVec * lVec);
	t = MAX(t, 0.0f);
	t = MIN(t, 1.0f); 
	return line.a + lVec * t; 
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
	return diff == 1.0f;
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