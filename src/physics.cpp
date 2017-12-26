#include "physics.h"

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

float Raycast(const sphere_t &sphere, const ray_t &ray)
{
	vec3 e = sphere.origin - ray.origin;

	float rSq = sphere.radius * sphere.radius;
	float eSq = e.magnitudeSq();

	// ray.direction is assumed to be normalized
	float a = e * ray.dir;

	float bSq = eSq - (a * a);
	float f = sqrt(rSq - bSq);

	// No collision has happened
	if (rSq - (eSq - (a * a)) < 0.0f)
	{
		return -1; // -1 is invalid.   
	}
	// Ray starts inside the sphere  
	else if (eSq<rSq) 
	{
		return a + f; 
		// Just reverse direction   
	}   // else Normal intersection   
	return a - f;
}

// really just rayboxslab in common.cpp I think
float Raycast(const aabb_t &aabb, const ray_t &ray)
{
	float t1 = (aabb.min.x - ray.origin.x) / ray.dir.x;
	float t2 = (aabb.max.x - ray.origin.x) / ray.dir.x;
	float t3 = (aabb.min.y - ray.origin.y) / ray.dir.y;
	float t4 = (aabb.max.y - ray.origin.y) / ray.dir.y;
	float t5 = (aabb.min.z - ray.origin.z) / ray.dir.z;
	float t6 = (aabb.max.z - ray.origin.z) / ray.dir.z;

	//Find the largest minimum value
	float tmin = MAX(MAX(MIN(t1, t2), MIN(t3, t4)), MIN(t5, t6));

	//Find the smallest maximum value
	float tmax = MIN(MIN(MAX(t1, t2), MAX(t3, t4)), MAX(t5, t6));

	if (tmax < 0)
	{
		return -1;
	}

	if (tmin > tmax)
	{
		return -1;
	}

	if (tmin < 0.0f)
	{
		return tmax;
	}

	return tmin;
}

float Raycast(const obb_t &obb, const ray_t &ray)
{
	const float *o = obb.orientation.m;
	// X, Y and Z axis of OBB
	vec3 X(o[0], o[1], o[2]);
	vec3 Y(o[3], o[4], o[5]);
	vec3 Z(o[6], o[7], o[8]);

	vec3 p = obb.origin - ray.origin;


	vec3 f(X * ray.dir, Y * ray.dir, Z * ray.dir);
	vec3 e(X * p, Y * p, Z * p);


	float t[6] = { 0, 0, 0, 0, 0, 0 };

	if (CMP(f.x, 0))
	{
		if (-e.x - obb.size.x > 0 || -e.x + obb.size.x < 0)
		{
			return -1;
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
			return -1;
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
			return -1;
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
		return -1.0f;
	}

	if (tmin > tmax)
	{
		return -1.0f;
	}

	if (tmin < 0.0f)
	{
		return tmax;
	}

	return tmin;
}

float Raycast(const vec3 &normal, float d, const ray_t &ray)
{
	float nd = ray.dir * normal;
	float pn = ray.origin * normal;


	if (nd >= 0.0f)
	{
		return -1;
	}

	float t = (d - pn) / nd;

	if (t >= 0.0f)
	{
		return t;
	}

	return -1;
}


bool Linetest(const sphere_t &sphere, const line3_t &line)
{
	vec3 closest = ClosestPoint(line, sphere.origin);
	float distSq = (sphere.origin - closest).magnitudeSq();
	return distSq <= (sphere.radius * sphere.radius);
}

bool Linetest(const aabb_t &aabb, const line3_t & line)
{
	ray_t ray;   ray.origin = line.a;
	vec3 length = line.b - line.a;
	ray.dir = length.normalize();
	float t = Raycast(aabb, ray);
	return t >= 0 && t * t <= (length * length);
}

bool Linetest(const obb_t &obb, const line3_t &line)
{
	ray_t ray;
	vec3 length = line.b - line.a;
	ray.origin = line.a;
	ray.dir = length.normalize();
	float t = Raycast(obb, ray);
	return t >= 0 && t * t <= (length * length);
}

bool Linetest(const vec3 &normal, float d, const line3_t &line)
{
	vec3 ab = line.b - line.a;
	float nA = normal * line.a;
	float nAB = normal * ab;

	// If the line and plane are parallel, nAB will be 0
	// This will cause a divide by 0 exception below
	// If you plan on testing parallel lines and planes
	// it is sage to early out when nAB is 0. 
	float t = (d - nA) / nAB;
	return t >= 0.0f && t <= 1.0f;
}


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

void FromTriangle(const triangle_t &t, vec3 &normal, float &d)
{
	normal = vec3::crossproduct (t.b - t.a, t.c - t.a).normalize();
	d = normal * t.a;
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

vec3 ClosestPoint(const triangle_t &t, const vec3 &p)
{
	vec3 normal;
	float d;
	FromTriangle(t, normal, d);

	vec3 closest = ClosestPoint(p, normal, d);

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

#define AABBTriangle(a, t) TriangleAABB(t, a) 

interval_t GetInterval(const triangle_t &triangle, vec3 &axis)
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

	if (CMP(side1, 0) && CMP(side2, 0) && CMP(side3, 0))
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

bool TriangleTriangleRobust(const triangle_t &t1, const triangle_t &t2)
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
	vec3 normal;
	float d;
	FromTriangle(triangle, normal, d);
	float t = Raycast(normal, d, ray);
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


class Particle
{
public:
	void Particle::AddImpulse(const vec3& impulse)
	{
		velocity = velocity + impulse;
	}

	float Particle::InvMass()
	{
		if (mass == 0.0f)
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

//			if (Linetest(constraints[i], traveled))
			{
				vec3 direction = velocity.normalize();
				ray_t ray;
				
				ray.origin = oldPosition;
				ray.dir = direction;

//				RaycastResult result;
	//			if (Raycast(constraints[i], ray, &result))
				{
//					position = result.point + result.normal * 0.002f;
//					vec3 vn = result.normal * (result.normal * velocity);
//					vec3 vt = velocity - vn;

					oldPosition = position;
//					velocity = vt - vn * bounce;
					break;
				}
			}
		}
	}

	void Particle::SetMass(float m)
	{
		if (m < 0)
		{
			m = 0;
		}
		mass = m;
	}

	void Particle::SetFriction(float f)
	{
		if (f < 0)
		{
			f = 0;
		}
		friction = f;
	}

	vec3 Particle::GetVelocity()
	{
		return velocity;
	}

	vec3 Particle::GetPosition()
	{
		return position;
	}

	void Particle::SetPosition(vec3 &p)
	{
		position = p;
	}

	void Particle::Update(float dt)
	{
		oldPosition = position;
		/* OLD: vec3 acceleration = forces * (1.0f / mass); */
		/* NEW: */
		vec3 acceleration = forces * InvMass();
		velocity = velocity * friction + acceleration * dt;
		position = position + velocity * dt;
	}

	void Particle::ApplyForces()
	{
		forces = vec3(0.0f, -9.8f * mass, 0.0f);
	}

	void Particle::SetBounce(float bounce)
	{
		bounce = bounce;
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

	void Spring::SetParticles(Particle* _p1, Particle* _p2)
	{
		p1 = _p1;
		p2 = _p2; 
	}

	Particle* Spring::GetP1()
	{
		return p1;
	}

	Particle* Spring::GetP2()
	{
		return p2;
	}

	void Spring::SetConstants(float _k, float _b)
	{
		k = _k;
		b = _b;
	}

	void Spring::ApplyForce(float dt)
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
			verts[i].SetMass(1.0f);
			verts[i].SetBounce(0.0f);
			verts[i].SetFriction(0.9f);
		}
	}

	for (int x = 0; x < gridSize; ++x)
	{
		for (int z = 0; z < gridSize - 1; ++z)
		{
			int i = z * gridSize + x;
			int j = (z + 1) * gridSize + x;

			vec3 iPos = verts[i].GetPosition();
			vec3 jPos = verts[j].GetPosition();
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

			vec3 iPos = verts[i].GetPosition();
			vec3 jPos = verts[j].GetPosition();
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

			vec3 iPos = verts[i].GetPosition();
			vec3 jPos = verts[j].GetPosition();
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

			vec3 iPos = verts[i].GetPosition();
			vec3 jPos = verts[j].GetPosition();
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

			vec3 iPos = verts[i].GetPosition();
			vec3 jPos = verts[j].GetPosition();
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

			vec3 iPos = verts[i].GetPosition();
			vec3 jPos = verts[j].GetPosition(); 
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
	for (int i = 0; i < structural.size(); ++i)
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
		verts[i].SetMass(mass);
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
	for (int i = 0; i < structural.size(); ++i)
	{
		structural[i].ApplyForce(dt);
	}
	
	for (int i = 0, size = shear.size(); i < size; ++i)
	{
		shear[i].ApplyForce(dt);
	}
	
	for (int i = 0, size = bend.size(); i < size; ++i)
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
			glVertex3f(verts[tl].GetPosition().x, verts[tl].GetPosition().y, verts[tl].GetPosition().z);
			glVertex3f(verts[br].GetPosition().x, verts[br].GetPosition().y, verts[br].GetPosition().z);
			glVertex3f(verts[bl].GetPosition().x, verts[bl].GetPosition().y, verts[bl].GetPosition().z);


			glVertex3f(verts[tl].GetPosition().x, verts[tl].GetPosition().y, verts[tl].GetPosition().z);
			glVertex3f(verts[tr].GetPosition().x, verts[tr].GetPosition().y, verts[tr].GetPosition().z);
			glVertex3f(verts[br].GetPosition().x, verts[br].GetPosition().y, verts[br].GetPosition().z);
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
	vec3 pos1 = p1->GetPosition();
	vec3 pos2 = p2->GetPosition();
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
	vec3 delta = p2->GetPosition() - p1->GetPosition();
	float distance = delta.magnitude();

	float correction = (distance - length) / distance;

	p1->SetPosition(p1->GetPosition() + delta
		* 0.5f * correction);
	p2->SetPosition(p2->GetPosition() - delta
		* 0.5f * correction);

	p1->SolveConstraints(constraints);
	p2->SolveConstraints(constraints);
}
