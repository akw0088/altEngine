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

#include "include.h"

#include "raytrace.h"

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
	BARYCENTRIC_STRIP,
	HALFSPACE
} raster_t;

void raster_triangles(const raster_t type, const int block, int *pixels, float *zbuffer, const int width, const int height,
	const matrix4 &mvp, const int *index_array, const vertex_t *vertex_array, const texinfo_t *texture, const texinfo_t *lightmap,
	const int start_index, const int start_vertex, const int num_index, const int num_verts, int clip);

void raster_triangles_strip(const raster_t type, const int block, int *pixels, float *zbuffer, const int width, const int height,
	const matrix4 &mvp, const int *index_array, const vertex_t *vertex_array, const texinfo_t *texture, const texinfo_t *lightmap,
	const int start_index, const int start_vertex, const int num_index, const int num_verts, int clip);
void barycentric_triangle(int *pixels, float *zbuffer, const int width, const int height, const texinfo_t *texture, const texinfo_t *lightmap,
	const int x1, const int y1, const float z1, const float w1, const int c1,
	const int x2, const int y2, const float z2, const float w2, const int c2,
	const int x3, const int y3, const float z3, const float w3, const int c3,
	const float u1, const float v1,
	const float u2, const float v2,
	const float u3, const float v3,
	const float lu1, const float lv1,
	const float lu2, const float lv2,
	const float lu3, const float lv3,
	const int minx, const int maxx, const int miny, const int maxy, bool filter = false, bool trilinear = false, bool mipmap = false);

void span_triangle(int *pixels, float *zbuffer, const int width, const int height, const texinfo_t *texture,
	int x1, int y1, float z1, float w1, int c1,
	int x2, int y2, float z2, float w2, int c2,
	int x3, int y3, float z3, float w3, int c3,
	float u1, float v1,
	float u2, float v2,
	float u3, float v3,
	const int minx, const int maxx, const int miny, const int maxy);
void clip2d_sutherland_hodgman(int width, int height, vec4 *points, int &num_point);
void halfspace_triangle(int *pixels, float *zbuffer, int width, int height, const vec2 &v1, const vec2 &v2, const vec2 &v3);
void halfspace_triangle_fast(int *pixels, float *zbuffer, int width, int height, const vec3 &v1, const vec3 &v2, const vec3 &v3);



bool render_raytrace(vertex_t *vertex_array, int *index_array, int num_vert, int num_index, int width, int height, int *pixel, raytrace::light_t *light, int num_light, matrix4 &mvp);
#endif
