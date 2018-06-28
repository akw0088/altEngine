#include "include.h"

#ifndef RASTER_H
#define RASTER_H
#ifdef __linux__

typedef struct
{
	long x;
	long y;
	long z;
} ivec3;
#define RGB(r,g,b)   ((unsigned int)(((char)(r)|((short)((char)(g))<<8))|(((int)(char)(b))<<16)))
#endif

typedef enum {
	SPAN,
	BARYCENTRIC,
	HALFSPACE
} raster_t;

void raster_triangles(raster_t type, int *pixels, float *zbuffer, int width, int height, matrix4 &mvp, int *index_array, vertex_t *vertex_array, texinfo_t *texture_array, int start_index, int start_vertex, int num_index, int num_verts);
void raster_triangles_strip(raster_t type, int *pixels, float *zbuffer, int width, int height, matrix4 &mvp, int *index_array, vertex_t *vertex_array, texinfo_t *texture_array, int start_index, int start_vertex, int num_index, int num_verts);
void barycentric_triangle(int *pixels, float *zbuffer, int width, int height, texinfo_t *texture, int x1, int y1, float z1, int c1, int x2, int y2, float z2, int c2, int x3, int y3, float z3, int c3,
	float u1, float v1, float u2, float v2, float u3, float v3);
void span_triangle(int *pixels, float *zbuffer, int width, int height, int x1, int y1, int z1, int c1, int x2, int y2, int z2, int c2, int x3, int y3, int z3, int c3);
void clip2d_sutherland_hodgman(int width, int height, vec3 *points, int &num_point);
void triangulate(vec3 *point, int &num_point);
void halfspace_triangle(int *pixels, float *zbuffer, int width, int height, const vec2 &v1, const vec2 &v2, const vec2 &v3);
void halfspace_triangle_fast(int *pixels, float *zbuffer, int width, int height, const vec3 &v1, const vec3 &v2, const vec3 &v3);
#endif
