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


///=============================================================================
/// File: raster.cpp
///=============================================================================
/// This class performs software rendering/rasterization
/// essentially if you can draw a single triangle, you can draw all the triangles
///
/// There are a few triangle rendering algorithms here
///
///		barycentric - main triangle algorithm, supports textures, mip mapping
///
///		scanline - top/bottom style with xspans, supports textures
///  but they are not 100% correct (texture coordinates a bit off)
///
///		halfspace - So these variations came from discussions around the
/// swift shader developer on some forum. They do not support textures
/// currently, but will render a filled triangle
///
///		halfspace fast - a variation of the above that breaks filling
/// into smaller blocks, idea being filling a block is faster than
/// drawing spans. Will also render, but no textures etc
///
///		Raytrace - no textures, haven't tested this much past drawing a single
/// triangle, very slow as there is no bounding volume acceleration structures
///
/// Assuming one day I fix the textures on scanline and get it to feature parity
/// with the barycentric version, I hope to do what quake did regarding the 
/// perspective correct texture mapping. Which is to say, dont do perspective
/// correct texture mapping for every pixel. Just do it on the ends and if it
/// spans 16 pixels, do it every 16 pixels with a linear interpolation between.
/// Supposedly this helps pentium one do a FDIV in parallel. But that would mean
/// I would have to do things integer only in the loop to prevent doing any
/// other floating point operations near by
///
/// https://www.jagregory.com/abrash-black-book/#rasterization
///
///
///=============================================================================

#include "raster.h"
#include "common.h"
#include <math.h> // for cos

extern int raster_target;
extern int raster_enabled;
int raster_tcount = 0;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



inline int Raster::imin(int x, int y)
{
	return y ^ ((x ^ y) & -(x < y));
}

inline int Raster::imax(int x, int y)
{
	return y ^ ((x ^ y) & -(x > y));
}

inline void Raster::iclamp(int &a, int mi, int ma)
{
	a = imin(imax(a, mi), ma);
}


int Raster::clip_planes(vertex_t &a, vertex_t &b, vertex_t &c,
	vertex_t &d, vertex_t &e, vertex_t &f)
{
	vertex_t result[6];
	static int once = 0;
	static plane_t xp, xn, yp, yn, zp, zn;
	int ret = 0;
	int hard = 0;
	int easy = 0;

	if (once == 0)
	{
		matrix4 projection;

		float plane_d = 500.0f;

		xp.normal = vec3(1.0f, 0.0f, 0.0f);
		xp.d = plane_d;

		xn.normal = vec3(-1.0f, 0.0f, 0.0f);
		xn.d = plane_d;

		yp.normal = vec3(0.0f, 1.0f, 0.0f);
		yp.d = plane_d;

		yn.normal = vec3(0.0f, -1.0f, 0.0f);
		yn.d = plane_d;

		// near clip
		zp.normal = vec3(0.0f, 0.0f, 1.0f);
		zp.d = 1.0f;

		// far clip
		zn.normal = vec3(0.0f, 0.0f, -1.0f);
		zn.d = 2001.0f;


		projection.perspective(110.0f, 16.0f / 9.0f, 1.0f, 2001.0f, false);

		frustum_t frustum;
		Frame camera;


		if (1)
		{
			get_frustum(projection, &frustum.left);
		}
		else
		{
			camera.up = vec3(0.0f, 1.0f, 0.0f);
			camera.forward = vec3(0.0f, 0.0f, -1.0f);
			gen_frustum(&camera, &frustum);
		}

		xp = frustum.left;
		xn = frustum.right;
		yp = frustum.bottom;
		yn = frustum.top;
		zp = frustum.zNear;
		zn = frustum.zFar;
		once = 1;

		zp.normal.normalize();
		zp.d = 1;
	}
#if 0
	ret = intersect_triangle_plane(xp, a, b, c, result);
	if (ret == ALL_OUT)
	{
		return ALL_OUT;
	}

	if (ret == CLIPPED_EASY)
	{
		a = result[0];
		b = result[1];
		c = result[2];
		easy = 1;
//		return CLIPPED_EASY;
	}

	if (ret == CLIPPED_HARD)
	{
		a = result[0];
		b = result[1];
		c = result[2];
		d = result[3];
		e = result[4];
		f = result[5];
		hard = 1;
//		return CLIPPED_HARD;
	}

	ret = intersect_triangle_plane(xn, a, b, c, result);
	if (ret == ALL_OUT)
	{
		return ALL_OUT;
	}

	if (ret == CLIPPED_EASY)
	{
		a = result[0];
		b = result[1];
		c = result[2];
		easy = 1;
//		return CLIPPED_EASY;
	}

	if (ret == CLIPPED_HARD)
	{
		a = result[0];
		b = result[1];
		c = result[2];
		d = result[3];
		e = result[4];
		f = result[5];
		hard = 1;
//		return CLIPPED_HARD;
	}


	ret = intersect_triangle_plane(yp, a, b, c, result);
	if (ret == ALL_OUT)
	{
		return ALL_OUT;
	}

	if (ret == CLIPPED_EASY)
	{
		a = result[0];
		b = result[1];
		c = result[2];
		easy = 1;
//		return CLIPPED_EASY;
	}

	if (ret == CLIPPED_HARD)
	{
		a = result[0];
		b = result[1];
		c = result[2];
		d = result[3];
		e = result[4];
		f = result[5];
		hard = 1;
//		return CLIPPED_HARD;
	}


	ret = intersect_triangle_plane(yn, a, b, c, result);
	if (ret == ALL_OUT)
	{
		return ALL_OUT;
	}

	if (ret == CLIPPED_EASY)
	{
		a = result[0];
		b = result[1];
		c = result[2];
		easy = 1;
//		return CLIPPED_EASY;
	}

	if (ret == CLIPPED_HARD)
	{
		a = result[0];
		b = result[1];
		c = result[2];
		d = result[3];
		e = result[4];
		f = result[5];
		hard = 1;
//		return CLIPPED_HARD;
	}


	ret = intersect_triangle_plane(zn, a, b, c, result);
	if (ret == ALL_OUT)
	{
		return ALL_OUT;
	}

	if (ret == CLIPPED_EASY)
	{
		a = result[0];
		b = result[1];
		c = result[2];
		easy = 1;
//		return CLIPPED_EASY;
	}

	if (ret == CLIPPED_HARD)
	{
		a = result[0];
		b = result[1];
		c = result[2];
		d = result[3];
		e = result[4];
		f = result[5];
		hard = 1;
//		return CLIPPED_HARD;
	}

#endif

	ret = intersect_triangle_plane(zp, a, b, c, result);
	if (ret == ALL_OUT)
	{
		return ALL_OUT;
	}

	if (ret == CLIPPED_EASY)
	{
		a = result[0];
		b = result[1];
		c = result[2];
		easy = 1;


//		return CLIPPED_EASY;
	}

	if (ret == CLIPPED_HARD)
	{
		a = result[0];
		b = result[1];
		c = result[2];

		d = result[3];
		e = result[4];
		f = result[5];

		hard = 1;
//		return CLIPPED_HARD;
	}

	if (hard)
		return CLIPPED_HARD;
	else if (easy)
		return CLIPPED_EASY;
	else
		return ALL_IN;

	return -1;
}



///=============================================================================
/// Function: raster_triangles
///=============================================================================
/// Description: 
///
/// This function will loop through a integer indexed triangle buffer, perform
/// transformations to move the triangle into 2d clip space, and then call the 
/// selected triangle raster function.
///
/// Parameters:
///		widht, height, and block can be used to render a subsection of the viewport
/// which is used with openmp to multithread the rendering. Although single
/// thread works fine too. Each thread has an entire viewport worth of memory
/// to avoid false sharing (ie: if the memory accesses between threads can touch
/// the same cache line, then they will deadlock each other
///
/// Returns:
///		None
///=============================================================================
void Raster::raster_triangles(const raster_t type, const int block, unsigned int *pixels, float *zbuffer, const int width, const int height,
	const matrix4 &mvp, const int *index_array, const vertex_t *vertex_array, const texinfo_t *texture, const texinfo_t *lightmap,
	const int start_index, const int start_vertex, const int num_index, const int num_verts, int clip)
{

	// ensure we are going to draw more than one pixel
	if (width <= 1 || height <= 1)
		return;


	
	// increment triangle count
	raster_tcount++;

	// some code to debug specific triangles
	if (raster_tcount != raster_target && raster_enabled)
	{
		return;
	}

	// loop through the index buffer
	for (int i = start_index; i < start_index + num_index; i += 3)
	{
		int num_point = 3;
		vec4 tri[6];
		vec2 tri_uv[6];
		vec2 tri_luv[6];
		int start = 0;

		// take a triangle, transform it, then clip it against the clip space planes

		// transform model space to clip space
		vec4 v1 = mvp * vec4(vertex_array[start_vertex + index_array[i]].position, 1.0f);
		vec4 v2 = mvp * vec4(vertex_array[start_vertex + index_array[i + 1]].position, 1.0f);
		vec4 v3 = mvp * vec4(vertex_array[start_vertex + index_array[i + 2]].position, 1.0f);

		// initialize triangle output array used for rendering
		// (clipping can generate new triangles, so must be able to handle that)
		tri[0].x = v1.x;
		tri[0].y = v1.y;
		tri[0].z = v1.z;
		tri[0].w = v1.w;

		tri[1].x = v2.x;
		tri[1].y = v2.y;
		tri[1].z = v2.z;
		tri[1].w = v2.w;

		tri[2].x = v3.x;
		tri[2].y = v3.y;
		tri[2].z = v3.z;
		tri[2].w = v3.w;

		// Simple w tests first (throws away triangles outside view frustum)
		int good = 0;
		for (int j = 0; j < 3; j++)
		{
			if (
				((tri[j].x <= tri[j].w && tri[j].x >= -tri[j].w) &&
				(tri[j].y <= tri[j].w && tri[j].y >= -tri[j].w) &&
					(tri[j].z <= tri[j].w && tri[j].z >= -tri[j].w))
				)
			{
				good++;
			}
		}

		if (good == 0)
		{
			// all points of triangle were outside clip range
			continue;
		}

		// backface cull triangles that passed basic frustum check
		vec3 av = vec3(v2) - vec3(v1);
		vec3 bv = vec3(v3) - vec3(v1);
		if (vec3::crossproduct(av, bv) * v1 > 0)
		{
			continue;
		}

		// initialize triangle array texcoords
		tri_uv[0] = vertex_array[start_vertex + index_array[i]].texCoord0;
		tri_uv[1] = vertex_array[start_vertex + index_array[i + 1]].texCoord0;
		tri_uv[2] = vertex_array[start_vertex + index_array[i + 2]].texCoord0;

		tri_luv[0] = vertex_array[start_vertex + index_array[i]].texCoord1;
		tri_luv[1] = vertex_array[start_vertex + index_array[i + 1]].texCoord1;
		tri_luv[2] = vertex_array[start_vertex + index_array[i + 2]].texCoord1;

		if (clip)
		{
			// initialize vertex used for clipping
			vertex_t a = vertex_array[start_vertex + index_array[i]];
			vertex_t b = vertex_array[start_vertex + index_array[i + 1]];
			vertex_t c = vertex_array[start_vertex + index_array[i + 2]];

			vertex_t d = a;
			vertex_t e = b;
			vertex_t f = c;

			// set position to transformed coordinate (clipping uses vec3)
			a.position = v1;
			b.position = v2;
			c.position = v3;

			// at least one point was outside clip box
			if (good < 3)
			{
				// setting w to tangent.x so it gets linearly interpolated when clipped
				a.tangent.x = tri[0].w;
				b.tangent.x = tri[1].w;
				c.tangent.x = tri[2].w;

				// clip against frustum planes
				int ret = clip_planes(a, b, c, d, e, f);
				if (ret == ALL_OUT)
				{
					// triangle is outside frustum, skip it
					// should never happen since we already tested W
					continue;
				}
				else if (ret == CLIPPED_HARD)
				{
					// clipping generated two triangles
					num_point = 6;
					tri[0] = vec4(a.position, a.tangent.x);
					tri[1] = vec4(b.position, b.tangent.x);
					tri[2] = vec4(c.position, c.tangent.x);
					tri[3] = vec4(d.position, d.tangent.x);
					tri[4] = vec4(e.position, e.tangent.x);
					tri[5] = vec4(f.position, f.tangent.x);

					// set new tex coords
					tri_uv[0] = a.texCoord0;
					tri_uv[1] = b.texCoord0;
					tri_uv[2] = c.texCoord0;

					tri_uv[3] = d.texCoord0;
					tri_uv[4] = e.texCoord0;
					tri_uv[5] = f.texCoord0;
				}
				else if (ret == CLIPPED_EASY)
				{
					// clipped easy, just set new values
					tri[0] = vec4(a.position, a.tangent.x);
					tri[1] = vec4(b.position, b.tangent.x);
					tri[2] = vec4(c.position, c.tangent.x);

					// set new tex coords
					tri_uv[0] = a.texCoord0;
					tri_uv[1] = b.texCoord0;
					tri_uv[2] = c.texCoord0;
				}
				else if (ret == ALL_IN)
				{
					// clipped easy, just set new values
					tri[0] = vec4(a.position, a.tangent.x);
					tri[1] = vec4(b.position, b.tangent.x);
					tri[2] = vec4(c.position, c.tangent.x);

					// set new tex coords
					tri_uv[0] = a.texCoord0;
					tri_uv[1] = b.texCoord0;
					tri_uv[2] = c.texCoord0;
				}
			}

		}

		// loop through triangle array
		for (int j = 0; j < num_point; j += 3)
		{

			if (tri[j].w == 0.0f || tri[j+1].w == 0.0f || tri[j+2].w == 0.0f)
			{
				// cant divide by zero, ignore triangle (keep second triangle in case of cliping)
				start = j + 3;
				continue;
			}

			// perspective divide
			float inv = 1 / tri[j].w;
			tri[j].x *= inv;
			tri[j].y *= inv;
			tri[j].z *= inv;

			inv = 1 / tri[j + 1].w;
			tri[j + 1].x *= inv;
			tri[j + 1].y *= inv;
			tri[j + 1].z *= inv;
			
			inv = 1 / tri[j + 2].w;
			tri[j + 2].x *= inv;
			tri[j + 2].y *= inv;
			tri[j + 2].z *= inv;

			// [-1,1] -> [0,1]
			tri[j].x = tri[j].x * 0.5f + 0.5f;
			tri[j].y = tri[j].y * 0.5f + 0.5f;
			tri[j].z = tri[j].z * 0.5f + 0.5f;
			tri[j + 1].x = tri[j + 1].x * 0.5f + 0.5f;
			tri[j + 1].y = tri[j + 1].y * 0.5f + 0.5f;
			tri[j + 1].z = tri[j + 1].z * 0.5f + 0.5f;
			tri[j + 2].x = tri[j + 2].x * 0.5f + 0.5f;
			tri[j + 2].y = tri[j + 2].y * 0.5f + 0.5f;
			tri[j + 2].z = tri[j + 2].z * 0.5f + 0.5f;


			// old clipping style just threw away triangles that needed clipping
			if (clip == false)
			{
				if (tri[j].z < 0 && tri[j + 1].z < 0 && tri[j + 2].z < 0)
				{
					start = j + 3;
					break;
				}
			}

			// keep far plane clipping check (essentially identical to checking W against Z before division
			if (tri[j].z > 1.1000001f || tri[j + 1].z > 1.1000001f || tri[j + 2].z > 1.1000001f)
			{
				start = j + 3;
				continue;
			}

			//[0,1] -> [0,width] viewport transform
			tri[j].x *= width - 1;
			tri[j].y *= height - 1;
			tri[j + 1].x *= width - 1;
			tri[j + 1].y *= height - 1;
			tri[j + 2].x *= width - 1;
			tri[j + 2].y *= height - 1;
		}


		// now we have a good triangle that will fit on the viewport, feed it to the selected raster function
		// big switch statement is for splitting the work into the 16 threads if they are enabled

#ifdef WIN32
		try {
#endif
			// Actually draw the triangle (using 16 tiles and threads or single threaded single tile)
			for (int j = start; j < num_point; j += 3)
			{
				if (type == SPAN)
				{
					switch (block)
					{
					case 0:
						span_triangle(pixels, zbuffer, width, height, texture,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							0, width / 4, 0, height / 4);
						break;
					case 1:
						span_triangle(pixels, zbuffer, width, height, texture,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							width / 4, 2 * width / 4, 0, height / 4);
						break;
					case 2:
						span_triangle(pixels, zbuffer, width, height, texture,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							2 * width / 4, 3 * width / 4, 0, height / 4);
						break;
					case 3:
						span_triangle(pixels, zbuffer, width, height, texture,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							3 * width / 4, width, 0, height / 4);
						break;
					case 4:
						// row 2
						span_triangle(pixels, zbuffer, width, height, texture,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							0, width / 4, height / 4, 2 * height / 4);
						break;
					case 5:
						span_triangle(pixels, zbuffer, width, height, texture,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							width / 4, 2 * width / 4, height / 4, 2 * height / 4);
						break;
					case 6:
						span_triangle(pixels, zbuffer, width, height, texture,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							2 * width / 4, 3 * width / 4, height / 4, 2 * height / 4);
						break;
					case 7:
						span_triangle(pixels, zbuffer, width, height, texture,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							3 * width / 4, width, height / 4, 2 * height / 4);
						break;
					case 8:
						// row 3
						span_triangle(pixels, zbuffer, width, height, texture,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							0, width / 4, 2 * height / 4, 3 * height / 4);
						break;
					case 9:
						span_triangle(pixels, zbuffer, width, height, texture,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							width / 4, 2 * width / 4, 2 * height / 4, 3 * height / 4);
						break;
					case 10:
						span_triangle(pixels, zbuffer, width, height, texture,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							2 * width / 4, 3 * width / 4, 2 * height / 4, 3 * height / 4);
						break;
					case 11:
						span_triangle(pixels, zbuffer, width, height, texture,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							3 * width / 4, width, 2 * height / 4, 3 * height / 4);
						break;
					case 12:
						// row 4
						span_triangle(pixels, zbuffer, width, height, texture,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							0, width / 4, 3 * height / 4, height);
						break;
					case 13:
						span_triangle(pixels, zbuffer, width, height, texture,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							width / 4, 2 * width / 4, 3 * height / 4, height);
						break;
					case 14:
						span_triangle(pixels, zbuffer, width, height, texture,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							2 * width / 4, 3 * width / 4, 3 * height / 4, height);
						break;
					case 15:
						span_triangle(pixels, zbuffer, width, height, texture,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							3 * width / 4, width, 3 * height / 4, height);
						break;
					default:
						span_triangle(pixels, zbuffer, width, height, texture,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							0, width, 0, height);
						break;
					}
				}
				else if (type == BARYCENTRIC)
				{
					switch (block)
					{
					case 0:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							tri_luv[j + 0].x, tri_luv[j + 0].y,
							tri_luv[j + 1].x, tri_luv[j + 1].y,
							tri_luv[j + 2].x, tri_luv[j + 2].y,
							0, width / 4, 0, height / 4);
						break;
					case 1:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							tri_luv[j + 0].x, tri_luv[j + 0].y,
							tri_luv[j + 1].x, tri_luv[j + 1].y,
							tri_luv[j + 2].x, tri_luv[j + 2].y,
							width / 4, 2 * width / 4, 0, height / 4);
						break;
					case 2:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							tri_luv[j + 0].x, tri_luv[j + 0].y,
							tri_luv[j + 1].x, tri_luv[j + 1].y,
							tri_luv[j + 2].x, tri_luv[j + 2].y,
							2 * width / 4, 3 * width / 4, 0, height / 4);
						break;
					case 3:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							tri_luv[j + 0].x, tri_luv[j + 0].y,
							tri_luv[j + 1].x, tri_luv[j + 1].y,
							tri_luv[j + 2].x, tri_luv[j + 2].y,
							3 * width / 4, width, 0, height / 4);
						break;
					case 4:
						// row 2
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							tri_luv[j + 0].x, tri_luv[j + 0].y,
							tri_luv[j + 1].x, tri_luv[j + 1].y,
							tri_luv[j + 2].x, tri_luv[j + 2].y,
							0, width / 4, height / 4, 2 * height / 4);
						break;
					case 5:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							tri_luv[j + 0].x, tri_luv[j + 0].y,
							tri_luv[j + 1].x, tri_luv[j + 1].y,
							tri_luv[j + 2].x, tri_luv[j + 2].y,
							width / 4, 2 * width / 4, height / 4, 2 * height / 4);
						break;
					case 6:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							tri_luv[j + 0].x, tri_luv[j + 0].y,
							tri_luv[j + 1].x, tri_luv[j + 1].y,
							tri_luv[j + 2].x, tri_luv[j + 2].y,
							2 * width / 4, 3 * width / 4, height / 4, 2 * height / 4);
						break;
					case 7:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							tri_luv[j + 0].x, tri_luv[j + 0].y,
							tri_luv[j + 1].x, tri_luv[j + 1].y,
							tri_luv[j + 2].x, tri_luv[j + 2].y,
							3 * width / 4, width, height / 4, 2 * height / 4);
						break;
					case 8:
						// row 3
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							tri_luv[j + 0].x, tri_luv[j + 0].y,
							tri_luv[j + 1].x, tri_luv[j + 1].y,
							tri_luv[j + 2].x, tri_luv[j + 2].y,
							0, width / 4, 2 * height / 4, 3 * height / 4);
						break;
					case 9:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							tri_luv[j + 0].x, tri_luv[j + 0].y,
							tri_luv[j + 1].x, tri_luv[j + 1].y,
							tri_luv[j + 2].x, tri_luv[j + 2].y,
							width / 4, 2 * width / 4, 2 * height / 4, 3 * height / 4);
						break;
					case 10:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							tri_luv[j + 0].x, tri_luv[j + 0].y,
							tri_luv[j + 1].x, tri_luv[j + 1].y,
							tri_luv[j + 2].x, tri_luv[j + 2].y,
							2 * width / 4, 3 * width / 4, 2 * height / 4, 3 * height / 4);
						break;
					case 11:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							tri_luv[j + 0].x, tri_luv[j + 0].y,
							tri_luv[j + 1].x, tri_luv[j + 1].y,
							tri_luv[j + 2].x, tri_luv[j + 2].y,
							3 * width / 4, width, 2 * height / 4, 3 * height / 4);
						break;
					case 12:
						// row 4
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							tri_luv[j + 0].x, tri_luv[j + 0].y,
							tri_luv[j + 1].x, tri_luv[j + 1].y,
							tri_luv[j + 2].x, tri_luv[j + 2].y,
							0, width / 4, 3 * height / 4, height);
						break;
					case 13:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							tri_luv[j + 0].x, tri_luv[j + 0].y,
							tri_luv[j + 1].x, tri_luv[j + 1].y,
							tri_luv[j + 2].x, tri_luv[j + 2].y,
							width / 4, 2 * width / 4, 3 * height / 4, height);
						break;
					case 14:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							tri_luv[j + 0].x, tri_luv[j + 0].y,
							tri_luv[j + 1].x, tri_luv[j + 1].y,
							tri_luv[j + 2].x, tri_luv[j + 2].y,
							2 * width / 4, 3 * width / 4, 3 * height / 4, height);
						break;
					case 15:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							tri_luv[j + 0].x, tri_luv[j + 0].y,
							tri_luv[j + 1].x, tri_luv[j + 1].y,
							tri_luv[j + 2].x, tri_luv[j + 2].y,
							3 * width / 4, width, 3 * height / 4, height);
						break;
					default:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							tri_luv[j + 0].x, tri_luv[j + 0].y,
							tri_luv[j + 1].x, tri_luv[j + 1].y,
							tri_luv[j + 2].x, tri_luv[j + 2].y,
							0, width, 0, height);
						break;
					}
				}
				else if (type == HALFSPACE)
				{
					halfspace_triangle(pixels, zbuffer, width, height, tri[j + 0], tri[j + 1], tri[j + 2]);
				}
			}
#ifdef WIN32
		}
		catch (...)
		{
			printf("caught something\r\n");
		}
#endif
	}

}

///=============================================================================
/// Function: raster_triangles_strip
///=============================================================================
/// Description: 
///
/// Same as raster_triangle, except we have a triangle strip 
///
///
/// Returns:
///		None
///=============================================================================
void Raster::raster_triangles_strip(const raster_t type, const int block, unsigned int *pixels, float *zbuffer, const int width, const int height,
	const matrix4 &mvp, const int *index_array, const vertex_t *vertex_array, const texinfo_t *texture, const texinfo_t *lightmap,
	const int start_index, const int start_vertex, const int num_index, const int num_verts, int clip)
{
	vec4 v1, v2, v3;
	float s1, s2, s3, t1, t2, t3;
	float ls1, ls2, ls3, lt1, lt2, lt3;


	bool even = false;
	for (int i = start_index; i < start_index + num_index;)
	{
		int num_point = 3;
		vec4 tri[6];
		vec2 tri_uv[6];
		vec2 tri_luv[6];
		int start = 0;
		vertex_t a, b, c;


		if (i == start_index)
		{
			a = vertex_array[start_vertex + index_array[i]];
			b = vertex_array[start_vertex + index_array[i + 1]];
			c = vertex_array[start_vertex + index_array[i + 2]];


			v1 = mvp * vec4(vertex_array[start_vertex + index_array[i]].position, 1.0f);
			v2 = mvp * vec4(vertex_array[start_vertex + index_array[i + 1]].position, 1.0f);
			v3 = mvp * vec4(vertex_array[start_vertex + index_array[i + 2]].position, 1.0f);

			s1 = vertex_array[start_vertex + index_array[i]].texCoord0.x;
			t1 = vertex_array[start_vertex + index_array[i]].texCoord0.y;
			s2 = vertex_array[start_vertex + index_array[i + 1]].texCoord0.x;
			t2 = vertex_array[start_vertex + index_array[i + 1]].texCoord0.y;
			s3 = vertex_array[start_vertex + index_array[i + 2]].texCoord0.x;
			t3 = vertex_array[start_vertex + index_array[i + 2]].texCoord0.y;

			ls1 = vertex_array[start_vertex + index_array[i]].texCoord1.x;
			lt1 = vertex_array[start_vertex + index_array[i]].texCoord1.y;
			ls2 = vertex_array[start_vertex + index_array[i + 1]].texCoord1.x;
			lt2 = vertex_array[start_vertex + index_array[i + 1]].texCoord1.y;
			ls3 = vertex_array[start_vertex + index_array[i + 2]].texCoord1.x;
			lt3 = vertex_array[start_vertex + index_array[i + 2]].texCoord1.y;

			i += 3;
		}
		else if (i == start_index + 3 || even)
		{
			v2 = mvp * vec4(vertex_array[start_vertex + index_array[i-2]].position, 1.0f);
			v1 = mvp * vec4(vertex_array[start_vertex + index_array[i-1]].position, 1.0f);
			v3 = mvp * vec4(vertex_array[start_vertex + index_array[i]].position, 1.0f);

			b = vertex_array[start_vertex + index_array[i-2]];
			a = vertex_array[start_vertex + index_array[i-1]];
			c = vertex_array[start_vertex + index_array[i]];


			s2 = vertex_array[start_vertex + index_array[i-2]].texCoord0.x;
			t2 = vertex_array[start_vertex + index_array[i-2]].texCoord0.y;
			s1 = vertex_array[start_vertex + index_array[i-1]].texCoord0.x;
			t1 = vertex_array[start_vertex + index_array[i-1]].texCoord0.y;
			s3 = vertex_array[start_vertex + index_array[i]].texCoord0.x;
			t3 = vertex_array[start_vertex + index_array[i]].texCoord0.y;

			ls2 = vertex_array[start_vertex + index_array[i - 2]].texCoord1.x;
			lt2 = vertex_array[start_vertex + index_array[i - 2]].texCoord1.y;
			ls1 = vertex_array[start_vertex + index_array[i - 1]].texCoord1.x;
			lt1 = vertex_array[start_vertex + index_array[i - 1]].texCoord1.y;
			ls3 = vertex_array[start_vertex + index_array[i]].texCoord1.x;
			lt3 = vertex_array[start_vertex + index_array[i]].texCoord1.y;


			i++;
			even = false;
		}
		else
		{
			a = vertex_array[start_vertex + index_array[i - 2]];
			b = vertex_array[start_vertex + index_array[i - 1]];
			c = vertex_array[start_vertex + index_array[i]];

			v1 = mvp * vec4(vertex_array[start_vertex + index_array[i - 2]].position, 1.0f);
			v2 = mvp * vec4(vertex_array[start_vertex + index_array[i - 1]].position, 1.0f);
			v3 = mvp * vec4(vertex_array[start_vertex + index_array[i]].position, 1.0f);

			s1 = vertex_array[start_vertex + index_array[i - 2]].texCoord0.x;
			t1 = vertex_array[start_vertex + index_array[i - 2]].texCoord0.y;
			s2 = vertex_array[start_vertex + index_array[i - 1]].texCoord0.x;
			t2 = vertex_array[start_vertex + index_array[i - 1]].texCoord0.y;
			s3 = vertex_array[start_vertex + index_array[i]].texCoord0.x;
			t3 = vertex_array[start_vertex + index_array[i]].texCoord0.y;

			ls1 = vertex_array[start_vertex + index_array[i - 2]].texCoord1.x;
			lt1 = vertex_array[start_vertex + index_array[i - 2]].texCoord1.y;
			ls2 = vertex_array[start_vertex + index_array[i - 1]].texCoord1.x;
			lt2 = vertex_array[start_vertex + index_array[i - 1]].texCoord1.y;
			ls3 = vertex_array[start_vertex + index_array[i]].texCoord1.x;
			lt3 = vertex_array[start_vertex + index_array[i]].texCoord1.y;

			i++;
			even = true;
		}

		// initialize triangle output array used for rendering
		// (clipping can generate new triangles, so must be able to handle that)
		tri[0].x = v1.x;
		tri[0].y = v1.y;
		tri[0].z = v1.z;
		tri[0].w = v1.w;
		tri[1].x = v2.x;
		tri[1].y = v2.y;
		tri[1].z = v2.z;
		tri[1].w = v2.w;
		tri[2].x = v3.x;
		tri[2].y = v3.y;
		tri[2].z = v3.z;
		tri[2].w = v3.w;

		// Simple w tests first (throws away triangles outside view frustum)
		int good = 0;
		for (int j = 0; j < 3; j++)
		{
			if (
				((tri[j].x <= tri[j].w && tri[j].x >= -tri[j].w) &&
				(tri[j].y <= tri[j].w && tri[j].y >= -tri[j].w) &&
					(tri[j].z <= tri[j].w && tri[j].z >= -tri[j].w))
				)
			{
				good++;
			}
		}
		if (good == 0)
		{
			// all points of triangle were outside clip range
			continue;
		}

		// backface cull triangles that passed basic frustum check
		vec3 av = vec3(v2) - vec3(v1);
		vec3 bv = vec3(v3) - vec3(v1);
		if (vec3::crossproduct(av, bv) * v1 > 0)
		{
			continue;
		}

		// initialize triangle array texcoords
		tri_uv[0] = vec2(s1, t1);
		tri_uv[1] = vec2(s2, t2);
		tri_uv[2] = vec2(s3, t3);

		tri_luv[0] = vec2(ls1, lt1);
		tri_luv[1] = vec2(ls2, lt2);
		tri_luv[2] = vec2(ls3, lt3);

		if (clip)
		{
			// initialize vertex used for clipping

			vertex_t d;
			vertex_t e;
			vertex_t f;

			// set position to transformed coordinate (clipping uses vec3)
			a.position = v1;
			b.position = v2;
			c.position = v3;

			// at least one point was outside clip box
			if (good < 3)
			{
				// setting w to texCoord1 so it gets linearly interpolated when clipped
				a.texCoord1.x = tri[0].w;
				b.texCoord1.x = tri[1].w;
				c.texCoord1.x = tri[2].w;

				// clip against frustum planes
				int ret = clip_planes(a, b, c, d, e, f);
				if (ret == ALL_OUT)
				{
					// triangle is outside frustum, skip it
					// should never happen since we already tested W
					continue;
				}
				else if (ret == CLIPPED_HARD)
				{
					// clipping generated two triangles
					num_point = 6;
					tri[0] = vec4(a.position, a.texCoord1.x);
					tri[1] = vec4(b.position, b.texCoord1.x);
					tri[2] = vec4(c.position, c.texCoord1.x);
					tri[3] = tri[0];
					tri[4] = tri[1];
					tri[5] = tri[2];
					tri[3] = vec4(d.position, d.texCoord1.x);
					tri[4] = vec4(e.position, e.texCoord1.x);
					tri[5] = vec4(f.position, f.texCoord1.x);


					// set new tex coords
					tri_uv[0] = a.texCoord0;
					tri_uv[1] = b.texCoord0;
					tri_uv[2] = c.texCoord0;

					tri_uv[3] = tri_uv[0];
					tri_uv[4] = tri_uv[1];
					tri_uv[5] = tri_uv[2];

					tri_uv[3] = d.texCoord0;
					tri_uv[4] = e.texCoord0;
					tri_uv[5] = f.texCoord0;
				}
				else if (ret == CLIPPED_EASY)
				{
					// clipped easy, just set new values
					tri[0] = vec4(a.position, a.texCoord1.x);
					tri[1] = vec4(b.position, b.texCoord1.x);
					tri[2] = vec4(c.position, c.texCoord1.x);

					// set new tex coords
					tri_uv[0] = a.texCoord0;
					tri_uv[1] = b.texCoord0;
					tri_uv[2] = c.texCoord0;
				}
			}

		}

		// loop through triangle array
		for (int j = 0; j < num_point; j += 3)
		{
			if (tri[j].w == 0.0f || tri[j + 1].w == 0.0f || tri[j + 2].w == 0.0f)
			{
				// cant divide by zero, ignore triangle
				start = j + 3;
				continue;
			}

			// perspective divide
			float inv = 1 / tri[j].w;
			tri[j].x *= inv;
			tri[j].y *= inv;
			tri[j].z *= inv;

			inv = 1 / tri[j + 1].w;
			tri[j + 1].x *= inv;
			tri[j + 1].y *= inv;
			tri[j + 1].z *= inv;

			inv = 1 / tri[j + 2].w;
			tri[j + 2].x *= inv;
			tri[j + 2].y *= inv;
			tri[j + 2].z *= inv;

			// [-1,1] -> [0,1]
			tri[j].x = tri[j].x * 0.5f + 0.5f;
			tri[j].y = tri[j].y * 0.5f + 0.5f;
			tri[j].z = tri[j].z * 0.5f + 0.5f;
			tri[j + 1].x = tri[j + 1].x * 0.5f + 0.5f;
			tri[j + 1].y = tri[j + 1].y * 0.5f + 0.5f;
			tri[j + 1].z = tri[j + 1].z * 0.5f + 0.5f;
			tri[j + 2].x = tri[j + 2].x * 0.5f + 0.5f;
			tri[j + 2].y = tri[j + 2].y * 0.5f + 0.5f;
			tri[j + 2].z = tri[j + 2].z * 0.5f + 0.5f;


			// old clipping style just threw away triangles that needed clipping
			if (clip == false)
			{
				if (tri[j].z < 0 && tri[j + 1].z < 0 && tri[j + 2].z < 0)
				{
					start = j + 3;
					break;
				}
			}

			// keep far plane clipping check (essentially identical to checking W against Z before division
			if (tri[j].z > 1.0001f || tri[j + 1].z > 1.0001f || tri[j + 2].z > 1.0001f)
			{
				start = j + 3;
				continue;
			}

			//[0,1] -> [0,width] viewport transform
			tri[j].x *= width - 1;
			tri[j].y *= height - 1;
			tri[j + 1].x *= width - 1;
			tri[j + 1].y *= height - 1;
			tri[j + 2].x *= width - 1;
			tri[j + 2].y *= height - 1;
		}


#ifdef WIN32
		try {
#endif
			for (int j = start; j < num_point; j += 3)
			{
				if (type == SPAN)
				{
					switch (block)
					{
					case 0:
						span_triangle(pixels, zbuffer, width, height, texture,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, 0, width / 4, 0, height / 4);
						break;
					case 1:
						span_triangle(pixels, zbuffer, width, height, texture,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, width / 4, 2 * width / 4, 0, height / 4);
						break;
					case 2:
						span_triangle(pixels, zbuffer, width, height, texture,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, 2 * width / 4, 3 * width / 4, 0, height / 4);
						break;
					case 3:
						span_triangle(pixels, zbuffer, width, height, texture,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, 3 * width / 4, width, 0, height / 4);
						break;
					case 4:
						// row 2
						span_triangle(pixels, zbuffer, width, height, texture,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, 0, width / 4, height / 4, 2 * height / 4);
						break;
					case 5:
						span_triangle(pixels, zbuffer, width, height, texture,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, width / 4, 2 * width / 4, height / 4, 2 * height / 4);
						break;
					case 6:
						span_triangle(pixels, zbuffer, width, height, texture,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, 2 * width / 4, 3 * width / 4, height / 4, 2 * height / 4);
						break;
					case 7:
						span_triangle(pixels, zbuffer, width, height, texture,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, 3 * width / 4, width, height / 4, 2 * height / 4);
						break;
					case 8:
						// row 3
						span_triangle(pixels, zbuffer, width, height, texture,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, 0, width / 4, 2 * height / 4, 3 * height / 4);
						break;
					case 9:
						span_triangle(pixels, zbuffer, width, height, texture,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, width / 4, 2 * width / 4, 2 * height / 4, 3 * height / 4);
						break;
					case 10:
						span_triangle(pixels, zbuffer, width, height, texture,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, 2 * width / 4, 3 * width / 4, 2 * height / 4, 3 * height / 4);
						break;
					case 11:
						span_triangle(pixels, zbuffer, width, height, texture,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, 3 * width / 4, width, 2 * height / 4, 3 * height / 4);
						break;
					case 12:
						// row 4
						span_triangle(pixels, zbuffer, width, height, texture,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, 0, width / 4, 3 * height / 4, height);
						break;
					case 13:
						span_triangle(pixels, zbuffer, width, height, texture,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, width / 4, 2 * width / 4, 3 * height / 4, height);
						break;
					case 14:
						span_triangle(pixels, zbuffer, width, height, texture,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, 2 * width / 4, 3 * width / 4, 3 * height / 4, height);
						break;
					case 15:
						span_triangle(pixels, zbuffer, width, height, texture,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, 3 * width / 4, width, 3 * height / 4, height);
						break;
					default:
						span_triangle(pixels, zbuffer, width, height, texture,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, 0, width, 0, height);
						break;
					}
				}
				else if (type == BARYCENTRIC_STRIP)
				{
					switch (block)
					{
					case 0:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3, 0, width / 4, 0, height / 4);
						break;
					case 1:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3, width / 4, 2 * width / 4, 0, height / 4);
						break;
					case 2:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3, 2 * width / 4, 3 * width / 4, 0, height / 4);
						break;
					case 3:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3, 3 * width / 4, width, 0, height / 4);
						break;
					case 4:
						// row 2
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3, 0, width / 4, height / 4, 2 * height / 4);
						break;
					case 5:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3, width / 4, 2 * width / 4, height / 4, 2 * height / 4);
						break;
					case 6:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3, 2 * width / 4, 3 * width / 4, height / 4, 2 * height / 4);
						break;
					case 7:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3, 3 * width / 4, width, height / 4, 2 * height / 4);
						break;
					case 8:
						// row 3
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3, 0, width / 4, 2 * height / 4, 3 * height / 4);
						break;
					case 9:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3, width / 4, 2 * width / 4, 2 * height / 4, 3 * height / 4);
						break;
					case 10:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3, 2 * width / 4, 3 * width / 4, 2 * height / 4, 3 * height / 4);
						break;
					case 11:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3, 3 * width / 4, width, 2 * height / 4, 3 * height / 4);
						break;
					case 12:
						// row 4
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3, 0, width / 4, 3 * height / 4, height);
						break;
					case 13:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3, width / 4, 2 * width / 4, 3 * height / 4, height);
						break;
					case 14:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3, 2 * width / 4, 3 * width / 4, 3 * height / 4, height);
						break;
					case 15:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3, 3 * width / 4, width, 3 * height / 4, height);
						break;
					default:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							(int)tri[j + 0].x, (int)tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							(int)tri[j + 1].x, (int)tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							(int)tri[j + 2].x, (int)tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3, 0, width, 0, height);
						break;
					}
				}
				else if (type == HALFSPACE)
				{
					halfspace_triangle_fast(pixels, zbuffer, width, height, tri[j + 0], tri[j + 1], tri[j + 2]);
				}
			}
#ifdef WIN32
		}
		catch (...)
		{

		}
#endif
	}

}



///=============================================================================
/// Function: draw_pixel
///=============================================================================
/// Description: 
///
/// Eessentially set_pixel() except we also write to the z buffer
/// this function needs to be fast, so no branches here
///
///
/// Returns:
///		None
///=============================================================================
inline void Raster::draw_pixel(unsigned int *pixels, float *zbuffer, int width, int height, int x, int y, float z, unsigned int color)
{
#ifdef THREAD
	pixels[x + y * width] = color;
	zbuffer[x + y * width] = z;
#else
	pixels[x + ((height - 1 - y) * width)] = color;
	zbuffer[x + ((height - 1 - y) * width)] = z;
	//zbuffer[x + y * width] = z;
#endif
}



///=============================================================================
/// Function: draw_xspan
///=============================================================================
/// Description: 
///
/// This is the xspan drawing function for the span triangle raster
/// 
/// Need to fix texture rendering, and perform the 16 pixel divide optimization
///
/// Returns:
///		None
///=============================================================================
inline void Raster::draw_xspan(unsigned int *pixels, float *zbuffer, const int width, const int height, const texinfo_t *texture, int x1, int y1, int z1, int x2, int z2, int color, float u1, float v1, float u2, float v2,
	const int minx, const int maxx, const int miny, const int maxy)
{
	// zero pixels to draw
	if (x1 == x2)
		return;

	int dx; // delta x
	int xs; // x start
	int xe; // x end

	// delta texture coordinate increments
	float du;
	float dv;
	float dz;

	// texture coordinate initial values
	float ui;
	float vi;
	float zi;

	// calculate delta x
	dx = abs32(x2 - x1);


//	if (dx == 1)
//		return;

	// calculate texture coordinate increment delta
	dz = (float)(z2 - z1) / dx;
	du = (float)(u2 - u1) / dx;
	dv = (float)(v2 - v1) / dx;

	if (x1 > x2)
	{
		// start with x2
		xs = x2;
		xe = x1;

		// start textures at uvz2
		ui = u2;
		vi = v2;
		zi = (float)z2;
	}
	else
	{
		// start with x1
		xs = x1;
		xe = x2;


		// start textures at uvz1
		ui = u1;
		vi = v1;
		zi = (float)z1;
	}

	for (int x = xs; x < xe; x++)
	{
		if (x < minx || x >= maxx)
		{
			ui += -du;
			vi += -dv;
			zi += -dz;
			continue;
		}

		if (zbuffer[x + y1 * width] < zi)
		{
			ui += -du;
			vi += -dv;
			zi += -dz;
			continue;
		}

		// ensure we always index into the texture with positive coordinates
		if (ui < 0)
			ui = 1.0f - ui;
		if (vi < 0)
			vi = 1.0f - vi;

		render_pixel(pixels, zbuffer, width, height, x, y1, zi, texture, NULL, false, 0, ui, vi, 0, 0, 0);

//		draw_pixel(pixels, zbuffer, width, height, x, y1, zi, texture->data[index][0]);
		ui += -du;
		vi += -dv;
		zi += -dz;
	}

}


///=============================================================================
/// Function: fill_bottom_triangle
///=============================================================================
/// Description: 
///
/// This is the flat bottom handling code for the span triangle algorithm
/// 
///
/// Returns:
///		None
///=============================================================================
inline void Raster::fill_bottom_triangle(unsigned int *pixels, float *zbuffer, const int width, const int height, const texinfo_t *texture, int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3, int z3, int color,
	float u1, float v1,
	float u2, float v2,
	float u3, float v3,
	const int minx, const int maxx, const int miny, const int maxy)
{
	if (y3 - y1 == 0)
		return;

	// flat bottom y2 - y1 == y3 - y1


	float yheight = (y3 - y1);

	float dx_left = (float)(x2 - x1) / yheight; // dx / dy
	float dx_right = (float)(x3 - x1) / yheight; // dx / dy
	float xs = (float)x1;
	float xe = (float)x1;

	float du_left = (float)((u2 - u1)) / yheight; // du / dy
	float du_right = (float)((u3 - u1)) / yheight; // du / dy
	float us = u1;
	float ue = u1;
	float dv_left = (float)(v2 - v1) / yheight; // dv / dy
	float dv_right = (float)(v3 - v1) / yheight; // dv / dy
	float vs = v1;
	float ve = v1;



	for (int y = y1; y < y3; y++)
	{
		if (y < miny || y >= maxy)
		{
			xs += dx_left;
			xe += dx_right;
			us += du_left;
			ue += du_right;
			vs += dv_left;
			ve += dv_right;
			continue;
		}

		draw_xspan(pixels, zbuffer, width, height, texture,
			(int)xs, y, z1, (int)xe, z2, color,
			us, vs, ue, ve,
			minx, maxx, miny, maxy);

		xs += dx_left;
		xe += dx_right;
		us += du_left;
		ue += du_right;
		vs += dv_left;
		ve += dv_right;
	}
}


///=============================================================================
/// Function: fill_top_triangle
///=============================================================================
/// Description: 
///
/// This is the flat top handling code for the span triangle algorithm
/// 
///
/// Returns:
///		None
///=============================================================================
inline void Raster::fill_top_triangle(unsigned int *pixels, float *zbuffer, const int width, const int height, const texinfo_t *texture, int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3, int z3, int color,
	float u1, float v1,
	float u2, float v2,
	float u3, float v3,
	const int minx, const int maxx, const int miny, const int maxy)
{
	if (y3 - y1 == 0)
		return;

	// y3 - y1 == y2 - y1

	float yheight = (y3 - y1);

	float dx_left = (float)(x3 - x1) / yheight;
	float dx_right = (float)(x3 - x2) / yheight;
	float xs = (float)x1;
	float xe = (float)x2;


	float du_left = (float)(u3 - u1) / yheight; // du / dy
	float du_right = (float)(u3 - u2) / yheight; // du / dy
	float us = u1;
	float ue = u2;

	float dv_left = (float)(v3 - v1) / yheight; // dv / dy
	float dv_right = (float)(v3 - v2) / yheight; // dv / dy
	float vs = v1;
	float ve = v2;


	for (int y = y1; y < y3; y++)
	{
		if (y < miny || y >= maxy)
		{
			xs += dx_left;
			xe += dx_right;
			us += du_left;
			ue += du_right;
			vs += dv_left;
			ve += dv_right;
			continue;
		}

		draw_xspan(pixels, zbuffer, width, height, texture, (int)xs, y, z1, (int)xe, z2, color, us, vs, ue, ve, minx, maxx, miny, maxy);
		xs += dx_left;
		xe += dx_right;
		us += du_left;
		ue += du_right;
		vs += dv_left;
		ve += dv_right;

	}
}


///=============================================================================
/// Function: iswap
///=============================================================================
/// Description: 
///
/// This is an integer xor swap, supposedly faster for the frequent swapping
/// or (x1,y1) (x2,y2) points to perserve moving with increasing memory addr
/// 
///
/// Returns:
///		None
///=============================================================================
inline void Raster::iswap(int &a, int &b)
{
	a ^= b;
	b ^= a;
	a ^= b;
}

///=============================================================================
/// Function: span_triangle
///=============================================================================
/// Description: 
///
/// Main span_triangle triangle raster function
/// 
///
/// Returns:
///		None
///=============================================================================
void Raster::span_triangle(unsigned int *pixels, float *zbuffer, const int width, const int height, const texinfo_t *texture,
	int x1, int y1, float z1, float w1, int c1,
	int x2, int y2, float z2, float w2, int c2,
	int x3, int y3, float z3, float w3, int c3,
	float u1, float v1,
	float u2, float v2,
	float u3, float v3,
	const int minx, const int maxx, const int miny, const int maxy)
{

	if (x1 > maxx && x1 < minx && y1 > maxy && y1 < miny &&
		x2 > maxx && x2 < minx && y2 > maxy && y2 < miny &&
		x3 > maxx && x3 < minx && y3 > maxy && y3 < miny)
	{
		return;
	}

	// sort y bottom to top
	// sort y bottom to top
	if (y1 > y2)
	{
		iswap(x1, x2);
		iswap(y1, y2);
		SWAP(z1, z2, float);
		SWAP(w1, w2, float);
		SWAP(u1, u2, float);
		SWAP(v1, v2, float);
	}

	if (y2 > y3)
	{
		iswap(x3, x2);
		iswap(y3, y2);
		SWAP(z3, z2, float);
		SWAP(w3, w2, float);
		SWAP(u3, u2, float);
		SWAP(v3, v2, float);
	}

	if (y1 > y2)
	{
		iswap(x1, x2);
		iswap(y1, y2);
		SWAP(z1, z2, float);
		SWAP(w1, w2, float);
		SWAP(u1, u2, float);
		SWAP(v1, v2, float);
	}


	if (y2 == y3)
	{
		// bottom triangle
		fill_bottom_triangle(pixels, zbuffer, width, height, texture, x1, y1, (int)z1, x2, y2, (int)z2, x3, y3, (int)z3, c1, u1, v1, u2, v2, u3, v3, minx, maxx, miny, maxy);
	}
	else if (y1 == y2)
	{
		// top triangle
		fill_top_triangle(pixels, zbuffer, width, height, texture, x1, y1, (int)z1, x2, y2, (int)z2, x3, y3, (int)z3, c1, u1, v1, u2, v2, u3, v3, minx, maxx, miny, maxy);
	}
	else
	{
		float t = (float)(y2 - y1) / (y3 - y1);
		int new_x = (int)((x3 - x1) * t + x1);
		int new_y = y2;


		float new_u = (u3 - u1) * t + u1;
		float new_v = -((v3 - v1) * t + v1);

		fill_bottom_triangle(pixels, zbuffer, width, height, texture, x1, y1, (int)z1, x2, y2, (int)z3, new_x, new_y, (int)z3, c1, u1, v1, u2, v2, new_u, new_v, minx, maxx, miny, maxy);
		fill_top_triangle(pixels, zbuffer, width, height, texture, x2, y2, (int)z2, new_x, new_y, (int)z2, x3, y3, (int)z3, c1, u2, v2, new_u, new_v, u3, v3, minx, maxx, miny, maxy);
	}
}


///=============================================================================
/// Function: det
///=============================================================================
/// Description: 
///
/// Calculate the 2d determinent, which is a signed area and can determine
/// vertex winding and edge direction
/// 
///
/// Returns:
///		None
///=============================================================================
inline int Raster::det(int ax, int ay, int bx, int by)
{
	return ax * by - bx *  ay;
}

///=============================================================================
/// Function: calculate_miplevel
///=============================================================================
/// Description: 
///
/// Determines which mip level we are at based on the z position
/// the farther the triangle, you dont need a giant texture to sample from
/// if you do sample full resolution you get super sharp textures that 
/// shimmer a lot
/// 
///
/// Returns:
///		None
///=============================================================================
void Raster::calculate_miplevel(const texinfo_t *texture, const float zi, int &mip_level, float &blend)
{
	float mip_range[7] = { 0.0f, 0.125f, 0.25f, 0.5f, 0.7f, 0.8f, 0.9f };
	int mip_select[7] = { 0, 1, 1, 1, 2, 3, 3 };
//	unsigned int mip_color[32];

	// mip colors go from blue green red, aqua white -- near to far (note RGB is backwards, really BGR)

	if (zi > mip_range[0] && zi <= mip_range[1])
	{
		mip_level = mip_select[0];
		//c = ~0;
		blend = 1.0f - (zi - mip_range[0]) / (mip_range[1] - mip_range[0]);
	}
	else if (zi > mip_range[1] && zi <= mip_range[2])
	{
		mip_level = imin(mip_select[1], texture->num_mip - 1);
		//c = RGB(255, 0, 0);
		blend = 1.0f - (zi - mip_range[1]) / (mip_range[2] - mip_range[1]);
	}
	else if (zi > mip_range[2] && zi <= mip_range[3])
	{
		mip_level = imin(mip_select[2], texture->num_mip - 1);
		//c = RGB(0, 255, 0);
		blend = 1.0f - (zi - mip_range[2]) / (mip_range[3] - mip_range[2]);
	}
	else if (zi > mip_range[3] && zi <= mip_range[4])
	{
		mip_level = imin(mip_select[3], texture->num_mip - 1);
		//c = RGB(0, 0, 255);
		blend = 1.0f - (zi - mip_range[3]) / (mip_range[4] - mip_range[3]);
	}
	else if (zi > mip_range[4] && zi <= mip_range[5])
	{
		mip_level = imin(mip_select[4], texture->num_mip - 1);
		//c = RGB(255, 255, 0);
		blend = 1.0f - (zi - mip_range[4]) / (mip_range[5] - mip_range[4]);
	}
	else if (zi > mip_range[5] && zi <= mip_range[6])
	{
		mip_level = imin(mip_select[5], texture->num_mip - 1);
		//c = RGB(255, 0, 255);
		blend = 1.0f - (zi - mip_range[5]) / (mip_range[6] - mip_range[5]);
	}
	else
	{
		mip_level = imin(mip_select[6], texture->num_mip - 1);
		//c = RGB(255, 255, 255);
	}
}


///=============================================================================
/// Function: render_pixel
///=============================================================================
/// Description: 
///
/// This is essentially a set_pixel type function, but will handle texture filtering
/// 
///
/// Returns:
///		None
///=============================================================================
void Raster::render_pixel(unsigned int *pixels, float *zbuffer, const int width, const int height, int x, int y, float zi, const texinfo_t *texture, const texinfo_t *lightmap, bool mipmap, int mip_level, float u, float v, float blend, bool filter, bool trilinear)
{
	if (texture->components == 1)
	{
		unsigned char color = bilinear_filter_1d((unsigned char *)texture->data[mip_level], texture->width[mip_level], texture->height[mip_level], u, v, filter);


		if (mipmap && trilinear)
		{
			unsigned char color2 = bilinear_filter_1d((unsigned char *)texture->data[mip_level + 1], texture->width[mip_level + 1], texture->height[mip_level + 1], u, v, filter);

			color = imin(imax((int)(color * blend + color2 * (1.0f - blend)), 0), 255);
		}

		color = 0x0;

		draw_pixel(pixels, zbuffer, width, height, x, y, zi, RGB(color, color, color));
	}
	else if (texture->components == 3)
	{

		rgb_t color = bilinear_filter_3d((rgb_t *)texture->data[mip_level], texture->width[mip_level], texture->height[mip_level], u, v, filter);

		if (mipmap && trilinear)
		{
			rgb_t color2 = bilinear_filter_3d((rgb_t *)texture->data[mip_level + 1], texture->width[mip_level + 1], texture->height[mip_level + 1], u, v, filter);

			color.r = imin(imax((int)(color.r * blend + color2.r * (1.0f - blend)), 0), 255);
			color.g = imin(imax((int)(color.g * blend + color2.g * (1.0f - blend)), 0), 255);
			color.b = imin(imax((int)(color.b * blend + color2.b * (1.0f - blend)), 0), 255);
		}

	
		draw_pixel(pixels, zbuffer, width, height, x, y, zi, RGB(color.r, color.g, color.b));
	}
	else
	{
		rgba_t color = bilinear_filter_4d((rgba_t *)texture->data[mip_level], texture->width[mip_level], texture->height[mip_level], u, v, filter);

		if (mipmap && trilinear)
		{
			rgba_t color2 = bilinear_filter_4d((rgba_t *)texture->data[mip_level + 1], texture->width[mip_level + 1], texture->height[mip_level + 1], u, v, filter);

			color.r = imin(imax((int)(color.r * blend + color2.r * (1.0f - blend)), 0), 255);
			color.g = imin(imax((int)(color.g * blend + color2.g * (1.0f - blend)), 0), 255);
			color.b = imin(imax((int)(color.b * blend + color2.b * (1.0f - blend)), 0), 255);
			color.a = imin(imax((int)(color.a * blend + color2.a * (1.0f - blend)), 0), 255);
		}


		unsigned int c = *((unsigned int *)(&color));

		draw_pixel(pixels, zbuffer, width, height, x, y, zi, c);
	}

}


///=============================================================================
/// Function: barycentric_triangle
///=============================================================================
/// Description: 
///
/// This is the current best triangle raster function, loops over bounding box
/// of triangle then does barycentric determinant checks for each edge to 
/// determine if the pixel is inside the triangle or not
///
/// Lot's of floating point operations regarding textures, which is definitely
/// not ideal in terms of speed. But having it work correctly is step 1,
/// speed comes second
/// 
///
/// Returns:
///		None
///=============================================================================
void Raster::barycentric_triangle(unsigned int *pixels, float *zbuffer, const int width, const int height, const texinfo_t *texture, const texinfo_t *lightmap,
	const int x1, const int y1, const float z1, const float w1, const int c1,
	const int x2, const int y2, const float z2, const float w2, const int c2,
	const int x3, const int y3, const float z3, const float w3, const int c3,
	const float u1, const float v1,
	const float u2, const float v2,
	const float u3, const float v3,
	const float lu1, const float lv1,
	const float lu2, const float lv2,
	const float lu3, const float lv3,
	const int minx, const int maxx, const int miny, const int maxy, bool filter, bool trilinear, bool mipmap)
{
	int max_x = imax(x1, imax(x2, x3));
	int min_x = imin(x1, imin(x2, x3));
	int max_y = imax(y1, imax(y2, y3));
	int min_y = imin(y1, imin(y2, y3));


	min_x = imax(min_x, minx);
	min_y = imax(min_y, miny);

	max_x = imax(max_x, minx);
	max_y = imax(max_y, miny);

	min_x = imin(min_x, maxx - 1);
	min_y = imin(min_y, maxy - 1);

	max_x = imin(max_x, maxx - 1);
	max_y = imin(max_y, maxy - 1);


	if (max_x == min_x || max_y == min_y)
		return;

	// triangle spanning vectors
	int vspan1x = (x2 - x1);
	int vspan1y = (y2 - y1);

	int vspan2x = (x3 - x1);
	int vspan2y = (y3 - y1);

	// area of parallelogram formed by spanning vectors
	int area_denom = det(vspan1x, vspan1y, vspan2x, vspan2y);

	// zero area triangle
	if (area_denom == 0)
		return;

	float inv_den = 1.0f / area_denom;

	// find 1/z, u/z, v/z per vertex
	float inverse_w1 = 1.0f / w1;
	float inverse_w2 = 1.0f / w2;
	float inverse_w3 = 1.0f / w3;

	float u_over_w1 = u1 * inverse_w1;
	float v_over_w1 = v1 * inverse_w1;
	float u_over_w2 = u2 * inverse_w2;
	float v_over_w2 = v2 * inverse_w2;
	float u_over_w3 = u3 * inverse_w3;
	float v_over_w3 = v3 * inverse_w3;


	// Everything in these loops must be optimized as they are called per pixel
	for (int y = min_y; y <= max_y; y++)
	{
		int det1;
		int det2right;
		static int det2;
		int qy = (y - y1);

		int det2left = vspan1x * qy;

		det1 = det((min_x - x1), qy, vspan2x, vspan2y);
		det2right = (min_x - x1) *  vspan1y;
		det2 = det2left - det2right;

		// Could unroll loop and use SIMD
		for (int x = min_x; x <= max_x; x++)
		{
			if (x != min_x)
			{
				// should work at bottom of loop, but is giving weird buldge distortion
				det1 += vspan2y;
				det2right += vspan1y;
				det2 = det2left - det2right;
			}

			// barycentric coords (s,t,1-s-t)
			float s = (float)det1 * inv_den; // should probably eliminate floats and use fixed point
			float t = (float)det2 * inv_den;
			float b = 1.0f - s - t;

			// if inside triangle
			if ((s >= 0.0f) && (t >= 0.0f) && (b >= 0.0f))
			{
				float inverse_w = (s * inverse_w2 + t * inverse_w3 + b * inverse_w1);
				float w_interpolated = 1.0f / inverse_w;
				float depth = w_interpolated * 0.0005f; // (1 / 2000.0f) == 0.0005f

				// check zbuffer clears to 1.0 (near) to 0.0 far
#ifdef THREAD
				if (zbuffer[x + y * width] <= depth)
				{
					continue;
				}
#else
				if (zbuffer[x + ((height - 1 - y) * width)] <= depth)
				{
					continue;
				}
#endif

				float u_over_w = s * u_over_w2 + t * u_over_w3 + b * u_over_w1;
				float v_over_w = s * v_over_w2 + t * v_over_w3 + b * v_over_w1;


				// find inverse / multiply to get perspective correct z, u, v
				float u = u_over_w * w_interpolated;
				float v = v_over_w * w_interpolated;

				int mip_level = 0;
				float blend = 1.0f;

				if (mipmap)
				{
					calculate_miplevel(texture, depth, mip_level, blend);
				}

				render_pixel(pixels, zbuffer, width, height, x, y, depth, texture, lightmap, mipmap, mip_level, u, v, blend, filter, trilinear);
			}
		}
	}
}



///=============================================================================
/// Function: line_intersect
///=============================================================================
/// Description: 
///
/// This is 2D Line intersection code, used for 2d clipping of viewport
///
///
/// Returns:
///		None
///=============================================================================
void Raster::line_intersect(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, float &xint, float &yint)
{
	float num = (float)((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4));
	float den = (float)((x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4));
	xint = num / den;
	num = (float)((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4));
	yint = num / den;
}

///=============================================================================
/// Function: clip_line
///=============================================================================
/// Description: 
///
/// This is 2D Line clipping code, used for 2d clipping of viewport
///
///
/// Returns:
///		None
///=============================================================================
void Raster::clip_line(vec4 *points, int &num_point, int x1, int y1, int x2, int y2)
{
	vec4 out[32];
	int out_num = 0;

	for (int i = 0; i < num_point; i++)
	{
		// get a line from two points (a,b)
		int k = (i + 1);
		if (k == num_point)
			k = 0;

		vec4 a = points[i];
		vec4 b = points[k];

		// test points against clip line
		int a_pos = ((int)x2 - (int)x1) * ((int)a.y - (int)y1) - ((int)y2 - (int)y1) * ((int)a.x - (int)x1);
		int b_pos = ((int)x2 - (int)x1) * ((int)b.y - (int)y1) - ((int)y2 - (int)y1) * ((int)b.x - (int)x1);

		// both points are inside
		if (a_pos < 0 && b_pos < 0)
		{
			// add b
			out[out_num] = b;
			out_num++;
		}

		// a is outside
		else if (a_pos >= 0 && b_pos < 0)
		{
			// add intersection with edge and b
			line_intersect(x1, y1, x2, y2, (int)a.x, (int)a.y, (int)b.x, (int)b.y, out[out_num].x, out[out_num].y);
			out_num++;

			out[out_num] = b;
			out_num++;
		}

		// b is outside
		else if (a_pos < 0 && b_pos >= 0)
		{
			// add intersection with edge
			line_intersect(x1, y1, x2, y2, (int)a.x, (int)a.y, (int)b.x, (int)b.y, out[out_num].x, out[out_num].y);
			out_num++;
		}

		// both points outside, clipped
	}

	// Copy new points into array
	num_point = out_num;
	for (int i = 0; i < num_point; i++)
	{
		points[i] = out[i];
	}
}


///=============================================================================
/// Function: clip2d_sutherland_hodgman
///=============================================================================
/// Description: 
///
/// This is 2D Line clipping code, used for 2d clipping of viewport
///
///
/// Returns:
///		None
///=============================================================================
void Raster::clip2d_sutherland_hodgman(int width, int height, vec4 *points, int &num_point)
{
	clip_line(points, num_point, 0, 0, 0, height);
	clip_line(points, num_point, 0, height, width, height);
	clip_line(points, num_point, width, height, width, 0);
	clip_line(points, num_point, width, 0, 0, 0);
}


///=============================================================================
/// Function: halfspace_triangle
///=============================================================================
/// Description: 
///
/// This is a triangle raster function, works, but no textures, just solid color
///
///
/// Returns:
///		None
///=============================================================================
void Raster::halfspace_triangle(unsigned int *pixels, float *zbuffer, int width, int height, const vec3 &v1, const vec3 &v2, const vec3 &v3)
{
	float y1 = v1.y;
	float y2 = v2.y;
	float y3 = v3.y;
	float x1 = v1.x;
	float x2 = v2.x;
	float x3 = v3.x;

	// Bounding rectangle
	int max_x = imax((int)x1, imax((int)x2, (int)x3));
	int min_x = imin((int)x1, imin((int)x2, (int)x3));
	int max_y = imax((int)y1, imax((int)y2, (int)y3));
	int min_y = imin((int)y1, imin((int)y2, (int)y3));



	min_x = imax(min_x, 0);
	max_x = imin(max_x, width);

	min_y = imax(min_y, 0);
	max_y = imin(max_y, height);


	if (x1 >= max_x && x1 <= min_x && y1 >= max_y && y1 <= min_y &&
		x2 >= max_x && x2 <= min_x && y2 >= max_y && y2 <= min_y &&
		x3 >= max_x && x3 <= min_x && y3 >= max_y && y3 <= min_y)
	{
		return;
	}


	// Scan through bounding rectangle
	for (int y = min_y; y < max_y; y++)
	{
		for (int x = min_x; x < max_x; x++)
		{
			// When all half-space functions positive, pixel is in triangle
			if ((x1 - x2) * (y - y1) - (y1 - y2) * (x - x1) > 0 &&
				(x2 - x3) * (y - y2) - (y2 - y3) * (x - x2) > 0 &&
				(x3 - x1) * (y - y3) - (y3 - y1) * (x - x3) > 0)
			{
				draw_pixel(pixels, zbuffer, width, height, x, y, 4, RGB(255, 0, 0));
			}
		}
	}
}


///=============================================================================
/// Function: iround
///=============================================================================
/// Description: 
///
/// This is a float to integer rounding function, just truncates, subtracts, and
/// compares with 0.5
///
///
/// Returns:
///		None
///=============================================================================
int Raster::iround(float x)
{
	int xi = (int)x;
	if (x - xi > 0.5f)
		return xi + 1;
	return xi;
}



///=============================================================================
/// Function: halfspace_triangle_fast
///=============================================================================
/// Description: 
///
/// This is a triangle raster function, works, but no textures, just solid color
/// this one differs from the previous in that it breaks the triangle into blocks
/// and can better fill those blocks if they happen to have no edges in them
/// smaller triangles would make this higher overhead than larger triangles
/// though, as the idea is you fill more blocks than the work to find them
///
///
/// Returns:
///		None
///=============================================================================
void Raster::halfspace_triangle_fast(unsigned int *pixels, float *zbuffer, int width, int height, const vec3 &v1, const vec3 &v2, const vec3 &v3)
{
	// 28.4 fixed-point coordinates
	const int Y1 = iround(16.0f * v1.y);
	const int Y2 = iround(16.0f * v2.y);
	const int Y3 = iround(16.0f * v3.y);
	const int X1 = iround(16.0f * v1.x);
	const int X2 = iround(16.0f * v2.x);
	const int X3 = iround(16.0f * v3.x);

	// Deltas
	const int DX12 = X1 - X2;
	const int DX23 = X2 - X3;
	const int DX31 = X3 - X1;
	const int DY12 = Y1 - Y2;
	const int DY23 = Y2 - Y3;
	const int DY31 = Y3 - Y1;

	// Fixed-point deltas
	const int FDX12 = DX12 << 4;
	const int FDX23 = DX23 << 4;
	const int FDX31 = DX31 << 4;
	const int FDY12 = DY12 << 4;
	const int FDY23 = DY23 << 4;
	const int FDY31 = DY31 << 4;


	// Bounding rectangle
	int maxx = imax((int)X1, imax((int)X2, (int)X3));
	int minx = imin((int)X1, imin((int)X2, (int)X3));
	int maxy = imax((int)Y1, imax((int)Y2, (int)Y3));
	int miny = imin((int)Y1, imin((int)Y2, (int)Y3));



	minx = imax(minx, 0);
	maxx = imin(maxx, width);

	miny = imax(miny, 0);
	maxy = imin(maxy, height);


	if (X1 >= maxx && X1 <= minx && Y1 >= maxy && Y1 <= miny &&
		X2 >= maxx && X2 <= minx && Y2 >= maxy && Y2 <= miny &&
		X3 >= maxx && X3 <= minx && Y3 >= maxy && Y3 <= miny)
	{
		return;
	}



	// this function breaks scene up into small blocks and saves times by filling entire sub-blocks
	minx = (minx + 0xF) >> 4;
	maxx = (maxx + 0xF) >> 4;
	miny = (miny + 0xF) >> 4;
	maxy = (maxy + 0xF) >> 4;


	// Block size, standard 8x8 (must be power of two)
	const int q = 4;


	// Start in corner of 8x8 block
	minx &= ~(q - 1);
	miny &= ~(q - 1);


	// Half-edge constants
	int C1 = DY12 * X1 - DX12 * Y1;
	int C2 = DY23 * X2 - DX23 * Y2;
	int C3 = DY31 * X3 - DX31 * Y3;

	// Correct for fill convention
	if (DY12 < 0 || (DY12 == 0 && DX12 > 0))
	{
		C1++;
	}
	if (DY23 < 0 || (DY23 == 0 && DX23 > 0))
	{
		C2++;
	}
	if (DY31 < 0 || (DY31 == 0 && DX31 > 0))
	{
		C3++;
	}

	// Loop through blocks
	for (int y = miny; y < maxy; y += q)
	{
		for (int x = minx; x < maxx; x += q)
		{
			// Corners of block
			int x0 = x << 4;
			int x1 = (x + q - 1) << 4;
			int y0 = y << 4;
			int y1 = (y + q - 1) << 4;

			// Evaluate half-space functions
			bool a00 = C1 + DX12 * y0 - DY12 * x0 > 0;
			bool a10 = C1 + DX12 * y0 - DY12 * x1 > 0;
			bool a01 = C1 + DX12 * y1 - DY12 * x0 > 0;
			bool a11 = C1 + DX12 * y1 - DY12 * x1 > 0;
			int a = (a00 << 0) | (a10 << 1) | (a01 << 2) | (a11 << 3);


			bool b00 = C2 + DX23 * y0 - DY23 * x0 > 0;
			bool b10 = C2 + DX23 * y0 - DY23 * x1 > 0;
			bool b01 = C2 + DX23 * y1 - DY23 * x0 > 0;
			bool b11 = C2 + DX23 * y1 - DY23 * x1 > 0;
			int b = (b00 << 0) | (b10 << 1) | (b01 << 2) | (b11 << 3);

			bool c00 = C3 + DX31 * y0 - DY31 * x0 > 0;
			bool c10 = C3 + DX31 * y0 - DY31 * x1 > 0;
			bool c01 = C3 + DX31 * y1 - DY31 * x0 > 0;
			bool c11 = C3 + DX31 * y1 - DY31 * x1 > 0;
			int c = (c00 << 0) | (c10 << 1) | (c01 << 2) | (c11 << 3);


			// Skip block when outside an edge
			if (a == 0x0 || b == 0x0 || c == 0x0)
				continue;

			// Accept whole block when totally covered
			if (a == 0xF && b == 0xF && c == 0xF)
			{
				for (int iy = 0; iy < q; iy++)
				{
					for (int ix = x; ix < x + q; ix++)
					{
						draw_pixel(pixels, zbuffer, width, height, ix, y + iy, v1.z, RGB(0, 255, 0));
					}
				}
			}
			else // Partially covered block
			{
				int CY1 = C1 + DX12 * y0 - DY12 * x0;
				int CY2 = C2 + DX23 * y0 - DY23 * x0;
				int CY3 = C3 + DX31 * y0 - DY31 * x0;


				for (int iy = y; iy < y + q; iy++)
				{
					int CX1 = CY1;
					int CX2 = CY2;
					int CX3 = CY3;

					for (int ix = x; ix < x + q; ix++)
					{
						if (CX1 > 0 && CX2 > 0 && CX3 > 0)
						{
							draw_pixel(pixels, zbuffer, width, height, ix, iy, v1.z, RGB(255, 0, 0));
						}
						CX1 -= FDY12;
						CX2 -= FDY23;
						CX3 -= FDY31;
					}
					CY1 += FDX12;
					CY2 += FDX23;
					CY3 += FDX31;
				}
			}
		}
	}
}



///=============================================================================
/// Function: triangulate
///=============================================================================
/// Description: 
///
/// This is a basic triangulation function for ordered points on a plane
/// essentially makes a triangle fan, but requires they be clockwise/counterclock wise
/// in order
///
///
/// Returns:
///		None
///=============================================================================
void Raster::triangulate(vec4 *point, int &num_point)
{
	vec4 out[256];
	vec4 *p0;
	vec4 *phelper;
	int j = 0;


	p0 = &point[0];
	phelper = &point[1];

	for (int i = 2; i < num_point; i++)
	{
		out[j + 0] = *p0;
		out[j + 1] = *phelper;
		out[j + 2] = point[i];
		phelper = &point[i];
		j += 3;
	}

	// Copy new points into array
	num_point = j;
	for (int i = 0; i < num_point; i++)
	{
		point[i] = out[i];
	}
}


///=============================================================================
/// Function: bilinear_filter_1d
///=============================================================================
/// Description: 
///
/// This perform bilinear interpolation on a 8 bit texture (single channel)
///
///
/// Returns:
///		None
///=============================================================================
char Raster::bilinear_filter_1d(const unsigned char *tex, const int width, const int height, const float u, const float v, bool enable)
{
	float mu = u - (int)u;
	float mv = v - (int)v;

	mu = mu * width;
	mv = mv * height;
	int x = (int)mu;
	int y = (int)mv;
	float u_fraction = mu - x;
	float v_fraction = mv - y;
	float u_opp = 1.0f - u_fraction;
	float v_opp = 1.0f - v_fraction;

	if (u < 0)
		x = width + x;

	if (v < 0)
		y = height + y;

	if (u < 0)
		x = width + x;

	if (v < 0)
		y = height + y;

	if (x < 0)
		x = 0;
	if (x >= width)
		x = width - 1;

	if (y < 0)
		y = 0;
	if (y >= height)
		y = height - 1;

	if (enable)
	{
		return imin(imax((int)((tex[x + y * width] * u_opp + tex[(x + 1) + (y * width)] * u_fraction) * v_opp +
			(tex[x + (y + 1) * width] * u_opp + tex[(x + 1) + ((y + 1) * width)] * u_fraction) * v_fraction), 0), 255);

	}
	else
	{
		return tex[x + y * width];
	}
}


///=============================================================================
/// Function: bilinear_filter_3d
///=============================================================================
/// Description: 
///
/// This perform bilinear interpolation on a RGB texture (three channel)
///
///
/// Returns:
///		None
///=============================================================================
rgb_t Raster::bilinear_filter_3d(const rgb_t *tex, const int width, const int height, const float u, const float v, bool enable)
{
	float mu = u - (int)u;
	float mv = v - (int)v;

	mu = mu * width;
	mv = mv * height;
	int x = (int)mu;
	int y = (int)mv;
	float u_fraction = mu - x;
	float v_fraction = mv - y;
	float u_opp = 1.0f - u_fraction;
	float v_opp = 1.0f - v_fraction;
	rgb_t result;

	if (u < 0)
		x = width + x;

	if (v < 0)
		y = height + y;

	if (u < 0)
		x = width + x;

	if (v < 0)
		y = height + y;

	if (x < 0)
		x = 0;
	if (x >= width)
		x = width - 1;

	if (y < 0)
		y = 0;
	if (y >= height)
		y = height - 1;

	if (enable)
	{
		result.r = imin(imax((int)((tex[x + y * width].r * u_opp + tex[(x + 1) + (y * width)].r * u_fraction) * v_opp +
			(tex[x + (y + 1) * width].r * u_opp + tex[(x + 1) + ((y + 1) * width)].r * u_fraction) * v_fraction), 0), 255);

		result.g = imin(imax((int)((tex[x + y * width].g * u_opp + tex[(x + 1) + (y * width)].g * u_fraction) * v_opp +
			(tex[x + (y + 1) * width].g * u_opp + tex[(x + 1) + ((y + 1) * width)].g * u_fraction) * v_fraction), 0), 255);

		result.b = imin(imax((int)((tex[x + y * width].b * u_opp + tex[(x + 1) + (y * width)].b * u_fraction) * v_opp +
			(tex[x + (y + 1) * width].b * u_opp + tex[(x + 1) + ((y + 1) * width)].b * u_fraction) * v_fraction), 0), 255);
	}
	else
	{
		return tex[x + y * width];
	}

	return result;
}


///=============================================================================
/// Function: bilinear_filter_4d
///=============================================================================
/// Description: 
///
/// This perform bilinear interpolation on a RGBA texture (four channel)
///
///
/// Returns:
///		None
///=============================================================================
inline rgba_t Raster::bilinear_filter_4d(const rgba_t *tex, const int width, const int height, const float u, const float v, bool enable)
{
	float mu = u - (int)u;
	float mv = v - (int)v;

	mu = mu * width;
	mv = mv * height;
	int x = (int)mu;
	int y = (int)mv;
	float u_fraction = mu - x;
	float v_fraction = mv - y;
	float u_opp = 1.0f - u_fraction;
	float v_opp = 1.0f - v_fraction;
	rgba_t result;

	if (u < 0)
		x = width + x;

	if (v < 0)
		y = height + y;

	if (x < 0)
		x = 0;
	if (x >= width)
		x = width - 1;

	if (y < 0)
		y = 0;
	if (y >= height)
		y = height - 1;

	if (width < 4 || height < 4)
	{
		return tex[x + y * width];
	}

	if (enable)
	{
		result.r = imin(imax((int)((tex[x + y * width].r * u_opp + tex[(x + 1) + (y * width)].r * u_fraction) * v_opp +
			 (tex[x + (y + 1) * width].r * u_opp + tex[(x + 1) + ((y + 1) * width)].r * u_fraction) * v_fraction), 0),255);

		result.g = imin(imax((int)((tex[x + y * width].g * u_opp + tex[(x + 1) + (y * width)].g * u_fraction) * v_opp +
			 (tex[x + (y + 1) * width].g * u_opp + tex[(x + 1) + ((y + 1) * width)].g * u_fraction) * v_fraction), 0), 255);

		result.b = imin(imax((int)((tex[x + y * width].b * u_opp + tex[(x + 1) + (y * width)].b * u_fraction) * v_opp +
			 (tex[x + (y + 1) * width].b * u_opp + tex[(x + 1) + ((y + 1) * width)].b * u_fraction) * v_fraction), 0), 255);

		result.a = imin(imax((int)((tex[x + y * width].a * u_opp + tex[(x + 1) + (y * width)].a * u_fraction) * v_opp +
			 (tex[x + (y + 1) * width].a * u_opp + tex[(x + 1) + ((y + 1) * width)].a * u_fraction) * v_fraction), 0),255);
	}
	else
	{
		return tex[x + y * width];
	}

	return result;
}


