#include "include.h"

#ifndef RAYTRACE_H

#define invsqrtf(x) (1.0f / sqrtf(x))
typedef vec3 color_t;

typedef struct
{
	vec3 start;
	vec3 dir;
} ray_t;


typedef struct
{
	color_t diffuse;
	float reflection;
	color_t specular;
	float power;
} material_t;

typedef struct
{
	vec3 pos;
	float size;
	int materialId;
} sphere_t;

typedef struct
{
	vec3 n;
	vec3 p;
	int materialId;
} rplane_t;


typedef struct
{
	vec3 a, b, c;
} triangle_t;

typedef struct
{
	vec3 pos;
	color_t intensity;
} light_t;
#endif
