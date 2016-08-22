#include "include.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Plane::Plane()
{
	normal.x = 0.0f;
	normal.y = 1.0f;
	normal.z = 0.0f;
	d = 0.0f;
}

Plane::Plane(vec4 vector)
{
	normal = vector;
	d = vector.w;
}

Plane::Plane(vec3 normal, float d)
{
	Plane::normal = normal;
	Plane::d = d;
}

