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
#define RGB(r,g,b)   ((unsigned int)(((unsigned char)(r)|((unsigned short)((unsigned char)(g))<<8))|(((unsigned int)(unsigned char)(b))<<16)))
#endif

typedef enum {
	SPAN,
	BARYCENTRIC,
	BARYCENTRIC_STRIP,
	HALFSPACE
} raster_t;


class Raster
{
public:

	// takes a triangle array and feeds it to the selected triangle rendering algorithms (does 3d-to-clipspace transform, backface culling, clipping etc)
	void raster_triangles(const raster_t type, const int block, unsigned int *pixels, float *zbuffer, const int width, const int height,
		const matrix4 &mvp, const int *index_array, const vertex_t *vertex_array, const texinfo_t *texture, const texinfo_t *lightmap,
		const int start_index, const int start_vertex, const int num_index, const int num_verts, int clip);

	// takes a triangle strip and feeds it to the selected triangle rendering algorithm (does 3d-to-clipspace transform, backface culling, clipping etc)
	void raster_triangles_strip(const raster_t type, const int block, unsigned int *pixels, float *zbuffer, const int width, const int height,
		const matrix4 &mvp, const int *index_array, const vertex_t *vertex_array, const texinfo_t *texture, const texinfo_t *lightmap,
		const int start_index, const int start_vertex, const int num_index, const int num_verts, int clip);

private:

	// main triangle render function for 3d, textures, z buffer, mip mapping, etc all works
	// uses barycentric triangle tests, can definitely be optimized for speed
	void barycentric_triangle(unsigned int *pixels, float *zbuffer, const int width, const int height, const texinfo_t *texture, const texinfo_t *lightmap,
		const int x1, const int y1, const float z1, const float w1, const int c1,
		const int x2, const int y2, const float z2, const float w2, const int c2,
		const int x3, const int y3, const float z3, const float w3, const int c3,
		const float u1, const float v1,
		const float u2, const float v2,
		const float u3, const float v3,
		const float lu1, const float lv1,
		const float lu2, const float lv2,
		const float lu3, const float lv3,
		const int minx, const int maxx, const int miny, const int maxy, bool filter = false, bool trilinear = false, bool mipmap = true);


	// draws a triangle using xspans (top, triangle, bottom triangle, splitting) textures half work
	void span_triangle(unsigned int *pixels, float *zbuffer, const int width, const int height, const texinfo_t *texture,
		int x1, int y1, float z1, float w1, int c1,
		int x2, int y2, float z2, float w2, int c2,
		int x3, int y3, float z3, float w3, int c3,
		float u1, float v1,
		float u2, float v2,
		float u3, float v3,
		const int minx, const int maxx, const int miny, const int maxy);

	// 2D determinent can be used to determine winding order, used by barycentric_triangle
	inline int det(int ax, int ay, int bx, int by);


	// span_triangle sub-function draws a special case triangle with flat bottom
	void fill_bottom_triangle(unsigned int *pixels, float *zbuffer, const int width, const int height, const texinfo_t *texture, int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3, int z3, int color,
		float u1, float v1,
		float u2, float v2,
		float u3, float v3, const int minx, const int maxx, const int miny, const int maxy);

	// span_triangle sub-function draws a special case triangle with flat top
	void fill_top_triangle(unsigned int *pixels, float *zbuffer, const int width, const int height, const texinfo_t *texture, int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3, int z3, int color,
		float u1, float v1,
		float u2, float v2,
		float u3, float v3,
		const int minx, const int maxx, const int miny, const int maxy);

	// draws an xspan (horizontal line) using textures, z, buffer, windowing etc
	void draw_xspan(unsigned int *pixels, float *zbuffer, const int width, const int height, const texinfo_t *texture, int x1, int y1, int z1, int x2, int z2, int color, float u1, float v1, float u2, float v2,
		const int minx, const int maxx, const int miny, const int maxy);


	// another triangle rendering algorithm (solid-color, no textures)
	void halfspace_triangle(unsigned int *pixels, float *zbuffer, int width, int height, const vec3 &v1, const vec3 &v2, const vec3 &v3);


	// optimized version of above, breaks scene into small blocks and fills those blocks (solid-color, no textures)
	void halfspace_triangle_fast(unsigned int *pixels, float *zbuffer, int width, int height, const vec3 &v1, const vec3 &v2, const vec3 &v3);

	// draws a pixel updating z buffer with it
	void draw_pixel(unsigned int *pixels, float *zbuffer, int width, int height, int x, int y, float z, unsigned int color);



	// draws a single pixel using texture, mipmaps, filtering, blending etc
	void render_pixel(unsigned int *pixels, float *zbuffer, const int width, const int height, int x, int y, float zi, const texinfo_t *texture, const texinfo_t *lightmap, bool mipmap, int mip_level, float u, float v, float blend, bool filter, bool trilinear);


	// calculates mipmap level
	void calculate_miplevel(const texinfo_t *texture, const float zi, int &mip_level, float &blend);

	// Bi-linear filters for a single component texture (alpha channel only)
	char bilinear_filter_1d(const unsigned char *tex, const int width, const int height, const float u, const float v, bool enable);

	// Bi-linear filters for a three component texture (RGB)
	rgb_t bilinear_filter_3d(const rgb_t *tex, const int width, const int height, const float u, const float v, bool enable);


	// Bi-linear filters for a four component texture (RGBA)
	rgba_t bilinear_filter_4d(const rgba_t *tex, const int width, const int height, const float u, const float v, bool enable);


	// will triangulate a flat polygon using a single point fanning out to other points
	void triangulate(vec4 *point, int &num_point); //triangle fan


	// supposedly fast round function (truncates, subtracts, if delta greater than 0.5, add one) results in an int
	int iround(float x);

	// supposedly fast integer swap function (xor swap)
	void iswap(int &a, int &b);

	// supposedly fast integer min (xor tricks)
	int imin(int x, int y);

	// supposedly fast integer max (xor tricks)
	int imax(int x, int y);


	// supposedly fast integer clamp (uses above imax, imin)
	void iclamp(int &a, int mi, int ma);


	// clips triangle by a frustum planes, some code is commented out though
	int clip_planes(vertex_t &a, vertex_t &b, vertex_t &c,
		vertex_t &d, vertex_t &e, vertex_t &f);


	// clips triangle by viewport window, all integer based so must be raster points
	void clip2d_sutherland_hodgman(int width, int height, vec4 *points, int &num_point);

	// used by above for each edge
	void clip_line(vec4 *points, int &num_point, int x1, int y1, int x2, int y2);

	// used by above for each edge (integer only 2d-line intersection)
	void line_intersect(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, float &xint, float &yint);


};




#endif
