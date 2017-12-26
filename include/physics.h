#ifndef RAY_H
#define RAY_H
#include "include.h"

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
	vec3 a, b, c;
} triangle_t;

typedef struct
{
	float min;
	float max;
} interval_t;

#endif
