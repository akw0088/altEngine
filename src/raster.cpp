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

#include "raster.h"
#include "common.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

inline int imin(int x, int y)
{
	return y ^ ((x ^ y) & -(x < y));
}

inline int imax(int x, int y)
{
	return y ^ ((x ^ y) & -(x > y));
}


int clip_planes(vertex_t &a, vertex_t &b, vertex_t &c,
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


		//projection.perspective(110.0f, 16.0f / 9.0f, 1.0f, 2001.0f, false);

		//get_frustum(projection, frustum);
		frustum_t frustum;

		Frame camera;
		camera.up = vec3(0.0f, 1.0f, 0.0f);
		camera.forward = vec3(0.0f, 0.0f, -1.0f);
		gen_frustum(&camera, &frustum);

		xp = frustum.left;
		xn = frustum.right;
		yp = frustum.bottom;
		yn = frustum.top;
		zp = frustum.zNear;
		zn = frustum.zFar;
		once = 1;
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

void raster_triangles(const raster_t type, const int block, int *pixels, float *zbuffer, const int width, const int height,
	const matrix4 &mvp, const int *index_array, const vertex_t *vertex_array, const texinfo_t *texture, const texinfo_t *lightmap,
	const int start_index, const int start_vertex, const int num_index, const int num_verts, int clip)
{
	if (width <= 1 || height <= 1)
		return;

	for (int i = start_index; i < start_index + num_index; i += 3)
	{
		// transform model space to clip space
		vec4 v1 = mvp * vec4(vertex_array[start_vertex + index_array[i]].position, 1.0f);
		vec4 v2 = mvp * vec4(vertex_array[start_vertex + index_array[i + 1]].position, 1.0f);
		vec4 v3 = mvp * vec4(vertex_array[start_vertex + index_array[i + 2]].position, 1.0f);

		// backface cull
		vec3 av = vec3(v2) - vec3(v1);
		vec3 bv = vec3(v3) - vec3(v1);
		if (vec3::crossproduct(av, bv) * vec3(0, 0, -1) < 0)
		{
			continue;
		}

		// initialize vertex used for clipping
		vertex_t a = vertex_array[start_vertex + index_array[i]];
		vertex_t b = vertex_array[start_vertex + index_array[i + 1]];
		vertex_t c = vertex_array[start_vertex + index_array[i + 2]];

		vertex_t d;
		vertex_t e;
		vertex_t f;

		// set position to transformed coordinate (clipping uses vec3)
		a.position = v1;
		b.position = v2;
		c.position = v3;

		int num_point = 3;
		vec4 tri[6];
		vec2 tri_uv[6];
		vec2 tri_luv[6];
		bool skip = false;

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

		// initialize triangle array texcoords
		tri_uv[0] = vertex_array[start_vertex + index_array[i]].texCoord0;
		tri_uv[1] = vertex_array[start_vertex + index_array[i + 1]].texCoord0;
		tri_uv[2] = vertex_array[start_vertex + index_array[i + 2]].texCoord0;

		tri_luv[0] = vertex_array[start_vertex + index_array[i]].texCoord1;
		tri_luv[1] = vertex_array[start_vertex + index_array[i + 1]].texCoord1;
		tri_luv[2] = vertex_array[start_vertex + index_array[i + 2]].texCoord1;
		
		if (clip)
		{
			// setting w to texCoord1 so it gets linearly interpolated when clipped
			a.texCoord1.x = tri[0].w;
			b.texCoord1.x = tri[1].w;
			c.texCoord1.x = tri[2].w;

			//simple w tests first
			int good = 0;
			for (int j = 0; j < 3; j++)
			{
				if (
						((tri[j].x < tri[j].w && tri[j].x > -tri[j].w) &&
						 (tri[j].y < tri[j].w && tri[j].y > -tri[j].w) &&
						 (tri[j].z < tri[j].w && tri[j].z > -tri[j].w))
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

			// at least one point was outside clip box
			if (good < 3)
			{
				// clip against frustum planes
				int ret = clip_planes(a, b, c, d, e, f);
				if (ret == ALL_OUT)
				{
					// triangle is outside frustum, skip it
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
				else if (CLIPPED_EASY)
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

			if (tri[j].w == 0.0f || tri[j+1].w == 0.0f || tri[j+2].w == 0.0f)
			{
				// cant divide by zero, ignore triangle
				skip = true;
				break;
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
					skip = true;
					break;
				}
			}

			// keep far plane clipping check
			if (tri[j].z > 1.0001f || tri[j + 1].z > 1.0001f || tri[j + 2].z > 1.0001f)
			{
				skip = true;
				break;
			}

			//[0,1] -> [0,width] viewport transform
			tri[j].x *= width - 1;
			tri[j].y *= height - 1;
			tri[j + 1].x *= width - 1;
			tri[j + 1].y *= height - 1;
			tri[j + 2].x *= width - 1;
			tri[j + 2].y *= height - 1;
		}

		if (skip)
			continue;

#ifdef WIN32
		try {
#endif
			for (int j = 0; j < num_point; j += 3)
			{
				if (type == SPAN)
				{
					switch (block)
					{
					case 0:
						span_triangle(pixels, zbuffer, width, height, texture,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							0, width / 4, 0, height / 4);
						break;
					case 1:
						span_triangle(pixels, zbuffer, width, height, texture,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							width / 4, 2 * width / 4, 0, height / 4);
						break;
					case 2:
						span_triangle(pixels, zbuffer, width, height, texture,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							2 * width / 4, 3 * width / 4, 0, height / 4);
						break;
					case 3:
						span_triangle(pixels, zbuffer, width, height, texture,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							3 * width / 4, width, 0, height / 4);
						break;
					case 4:
						// row 2
						span_triangle(pixels, zbuffer, width, height, texture,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							0, width / 4, height / 4, 2 * height / 4);
						break;
					case 5:
						span_triangle(pixels, zbuffer, width, height, texture,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							width / 4, 2 * width / 4, height / 4, 2 * height / 4);
						break;
					case 6:
						span_triangle(pixels, zbuffer, width, height, texture,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							2 * width / 4, 3 * width / 4, height / 4, 2 * height / 4);
						break;
					case 7:
						span_triangle(pixels, zbuffer, width, height, texture,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							3 * width / 4, width, height / 4, 2 * height / 4);
						break;
					case 8:
						// row 3
						span_triangle(pixels, zbuffer, width, height, texture,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							0, width / 4, 2 * height / 4, 3 * height / 4);
						break;
					case 9:
						span_triangle(pixels, zbuffer, width, height, texture,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							width / 4, 2 * width / 4, 2 * height / 4, 3 * height / 4);
						break;
					case 10:
						span_triangle(pixels, zbuffer, width, height, texture,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							2 * width / 4, 3 * width / 4, 2 * height / 4, 3 * height / 4);
						break;
					case 11:
						span_triangle(pixels, zbuffer, width, height, texture,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							3 * width / 4, width, 2 * height / 4, 3 * height / 4);
						break;
					case 12:
						// row 4
						span_triangle(pixels, zbuffer, width, height, texture,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							0, width / 4, 3 * height / 4, height);
						break;
					case 13:
						span_triangle(pixels, zbuffer, width, height, texture,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							width / 4, 2 * width / 4, 3 * height / 4, height);
						break;
					case 14:
						span_triangle(pixels, zbuffer, width, height, texture,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							2 * width / 4, 3 * width / 4, 3 * height / 4, height);
						break;
					case 15:
						span_triangle(pixels, zbuffer, width, height, texture,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							tri_uv[j + 0].x, tri_uv[j + 0].y,
							tri_uv[j + 1].x, tri_uv[j + 1].y,
							tri_uv[j + 2].x, tri_uv[j + 2].y,
							3 * width / 4, width, 3 * height / 4, height);
						break;
					default:
						span_triangle(pixels, zbuffer, width, height, texture,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
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
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
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
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
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
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
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
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
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
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
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
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
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
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
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
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
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
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
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
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
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
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
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
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
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
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
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
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
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
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
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
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
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
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
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

void raster_triangles_strip(const raster_t type, const int block, int *pixels, float *zbuffer, const int width, const int height,
	const matrix4 &mvp, const int *index_array, const vertex_t *vertex_array, const texinfo_t *texture, const texinfo_t *lightmap,
	const int start_index, const int start_vertex, const int num_index, const int num_verts, int clip)
{
	vec4 v1, v2, v3;
	float s1, s2, s3, t1, t2, t3;
	float ls1, ls2, ls3, lt1, lt2, lt3;


	bool even = false;
	for (int i = start_index; i < start_index + num_index;)
	{
		if (i == start_index)
		{
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

		if (width <= 1 || height <= 1)
			break;

		if (v1.w == 0.0f || v2.w == 0.0f || v3.w == 0.0f)
			continue;

		// perspective divide
		float inv = 1 / v1.w;
		v1.x *= inv;
		v1.y *= inv;
		v1.z *= inv;
		inv = 1 / v2.w;
		v2.x *= inv;
		v2.y *= inv;
		v2.z *= inv;
		inv = 1 / v3.w;
		v3.x *= inv;
		v3.y *= inv;
		v3.z *= inv;

		// [-1,1] -> [0,1]
		v1.x = v1.x * 0.5f + 0.5f;
		v1.y = v1.y * 0.5f + 0.5f;
		v1.z = v1.z * 0.5f + 0.5f;
		v2.x = v2.x * 0.5f + 0.5f;
		v2.y = v2.y * 0.5f + 0.5f;
		v2.z = v2.z * 0.5f + 0.5f;
		v3.x = v3.x * 0.5f + 0.5f;
		v3.y = v3.y * 0.5f + 0.5f;
		v3.z = v3.z * 0.5f + 0.5f;

		if (v1.z < 0 && v2.z < 0 && v3.z < 0)
			continue;
		if (v1.z > 1.0001f || v2.z > 1.0001f || v3.z > 1.0001f)
			continue;
		
		// backface cull
		vec3 a = vec3(v2) - vec3(v1);
		vec3 b = vec3(v3) - vec3(v1);
		if (vec3::crossproduct(a, b) * vec3(0, 0, -1) < 0)
			continue;

		//[0,1] -> [0,width]
		v1.x *= width - 1;
		v1.y *= height - 1;
		v2.x *= width - 1;
		v2.y *= height - 1;
		v3.x *= width - 1;
		v3.y *= height - 1;

		int num_point = 3;
		vec4 tri[3];

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

#ifdef WIN32
		try {
#endif
			for (int j = 0; j < num_point; j += 3)
			{
				if (type == SPAN)
				{
					switch (block)
					{
					case 0:
						span_triangle(pixels, zbuffer, width, height, texture,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, 0, width / 4, 0, height / 4);
						break;
					case 1:
						span_triangle(pixels, zbuffer, width, height, texture,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, width / 4, 2 * width / 4, 0, height / 4);
						break;
					case 2:
						span_triangle(pixels, zbuffer, width, height, texture,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, 2 * width / 4, 3 * width / 4, 0, height / 4);
						break;
					case 3:
						span_triangle(pixels, zbuffer, width, height, texture,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, 3 * width / 4, width, 0, height / 4);
						break;
					case 4:
						// row 2
						span_triangle(pixels, zbuffer, width, height, texture,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, 0, width / 4, height / 4, 2 * height / 4);
						break;
					case 5:
						span_triangle(pixels, zbuffer, width, height, texture,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, width / 4, 2 * width / 4, height / 4, 2 * height / 4);
						break;
					case 6:
						span_triangle(pixels, zbuffer, width, height, texture,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, 2 * width / 4, 3 * width / 4, height / 4, 2 * height / 4);
						break;
					case 7:
						span_triangle(pixels, zbuffer, width, height, texture,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, 3 * width / 4, width, height / 4, 2 * height / 4);
						break;
					case 8:
						// row 3
						span_triangle(pixels, zbuffer, width, height, texture,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, 0, width / 4, 2 * height / 4, 3 * height / 4);
						break;
					case 9:
						span_triangle(pixels, zbuffer, width, height, texture,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, width / 4, 2 * width / 4, 2 * height / 4, 3 * height / 4);
						break;
					case 10:
						span_triangle(pixels, zbuffer, width, height, texture,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, 2 * width / 4, 3 * width / 4, 2 * height / 4, 3 * height / 4);
						break;
					case 11:
						span_triangle(pixels, zbuffer, width, height, texture,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, 3 * width / 4, width, 2 * height / 4, 3 * height / 4);
						break;
					case 12:
						// row 4
						span_triangle(pixels, zbuffer, width, height, texture,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, 0, width / 4, 3 * height / 4, height);
						break;
					case 13:
						span_triangle(pixels, zbuffer, width, height, texture,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, width / 4, 2 * width / 4, 3 * height / 4, height);
						break;
					case 14:
						span_triangle(pixels, zbuffer, width, height, texture,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, 2 * width / 4, 3 * width / 4, 3 * height / 4, height);
						break;
					case 15:
						span_triangle(pixels, zbuffer, width, height, texture,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, 3 * width / 4, width, 3 * height / 4, height);
						break;
					default:
						span_triangle(pixels, zbuffer, width, height, texture,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
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
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3, 0, width / 4, 0, height / 4);
						break;
					case 1:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3, width / 4, 2 * width / 4, 0, height / 4);
						break;
					case 2:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3, 2 * width / 4, 3 * width / 4, 0, height / 4);
						break;
					case 3:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3, 3 * width / 4, width, 0, height / 4);
						break;
					case 4:
						// row 2
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3, 0, width / 4, height / 4, 2 * height / 4);
						break;
					case 5:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3, width / 4, 2 * width / 4, height / 4, 2 * height / 4);
						break;
					case 6:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3, 2 * width / 4, 3 * width / 4, height / 4, 2 * height / 4);
						break;
					case 7:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3, 3 * width / 4, width, height / 4, 2 * height / 4);
						break;
					case 8:
						// row 3
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3, 0, width / 4, 2 * height / 4, 3 * height / 4);
						break;
					case 9:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3, width / 4, 2 * width / 4, 2 * height / 4, 3 * height / 4);
						break;
					case 10:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3, 2 * width / 4, 3 * width / 4, 2 * height / 4, 3 * height / 4);
						break;
					case 11:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3, 3 * width / 4, width, 2 * height / 4, 3 * height / 4);
						break;
					case 12:
						// row 4
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3, 0, width / 4, 3 * height / 4, height);
						break;
					case 13:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3, width / 4, 2 * width / 4, 3 * height / 4, height);
						break;
					case 14:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3, 2 * width / 4, 3 * width / 4, 3 * height / 4, height);
						break;
					case 15:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
							s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3, 3 * width / 4, width, 3 * height / 4, height);
						break;
					default:
						barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
							tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
							tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
							tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
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

inline void draw_pixel(int *pixels, float *zbuffer, int width, int height, int x, int y, int z, unsigned int color)
{
#ifdef THREAD
	pixels[x + y * width] = color;
#else
	pixels[x + ((height - 1 - y) * width)] = color;
#endif
	zbuffer[x + y * width] = z;
}

void draw_line(int *pixels, int width, int height, int x1, int y1, int x2, int y2, int color)
{
	int i;
	int	x, y;
	float slope;
	int deltax;
	int deltay;

	if (x1 > width)
	{
		x1 = width;
	}
	if (x1 < 0)
	{
		x1 = 0;
	}
	if (y1 > height)
	{
		y1 = height;
	}
	if (y1 < 0)
	{
		y1 = 0;
	}

	if (x2 > width)
	{
		x2 = width;
	}
	if (x2 < 0)
	{
		x2 = 0;
	}
	if (y2 > height)
	{
		y2 = height;
	}
	if (y2 < 0)
	{
		y2 = 0;
	}

	deltax = abs32(x2 - x1);
	deltay = abs32(y2 - y1);

	if (deltax == 0 && deltay == 0)
		return;

	//We want x to always move right
	if (x2 - x1 < 0)
	{
		draw_line(pixels, width, height, x2, y2, x1, y1, color);
		return;
	}

	if (x2 - x1 != 0)
	{
		slope = (float)(y2 - y1) / (x2 - x1);
	}
	else
	{
		slope = 1000.0f;
	}

	if (slope > 1.0f)
	{
		//slope is greater than one, flip axis, redo everything
		if (y2 - y1 < 0)
		{
			draw_line(pixels, width, height, x2, y2, x1, y1, color);
			return;
		}

		slope = 1.0f / slope;

		for (i = 0; i <= deltay; i++)
		{
			y = y1 + i;
			x = x1 + slope * i;

			pixels[x + y * width] = color;
		}
	}
	else if (slope < -1.0f)
	{
		if (y2 - y1 < 0)
		{
			int temp;

			temp = y2;
			y2 = y1;
			y1 = temp;
			temp = x2;
			x2 = x1;
			x1 = temp;
		}

		slope = 1.0f / slope;

		for (i = 0; i <= deltay; i++)
		{
			y = y1 + i;
			x = x1 + slope * i;

			pixels[x + y * width] = color;
		}
	}
	else
	{
		for (i = 0; i <= deltax; i++)
		{
			x = x1 + i;
			y = y1 + slope * i;

			pixels[x + y * width] = color;
		}
	}
}

void flood_fill(int *pixels, int width, int height, int x, int y, int old_color, int new_color)
{
	if (x < width && x >= 0 && y < height && y >= 0 && pixels[x + y * width] != new_color)
	{
		pixels[x + y * width] = new_color;
		flood_fill(pixels, width, height, x + 1, y, old_color, new_color);
		flood_fill(pixels, width, height, x - 1, y, old_color, new_color);
		flood_fill(pixels, width, height, x, y + 1, old_color, new_color);
		flood_fill(pixels, width, height, x, y - 1, old_color, new_color);
	}
}



inline void draw_xspan(int *pixels, float *zbuffer, const int width, const int height, const texinfo_t *texture, int x1, int y1, int z1, int x2, int z2, int color, float u1, float v1, float u2, float v2,
	const int minx, const int maxx, const int miny, const int maxy)
{
	if (x1 == x2)
		return;

	int dx;
	int xs;
	int xe;

	float du;
	float dv;
	float dz;

	float ui;
	float vi;
	float zi;

	dx = abs32(x2 - x1);
	if (dx == 1)
		return;

	dz = (float)(z2 - z1) / dx;
	du = (float)(u2 - u1) / dx;
	dv = (float)(v2 - v1) / dx;

	if (x1 > x2)
	{
		xs = x2;
		xe = x1;

		ui = u2 - (int)u2;
		vi = v2 - (int)v2;
		zi = z2;
	}
	else
	{
		xs = x1;
		xe = x2;

		ui = u1 - (int)u1;
		vi = v1 - (int)v1;
		zi = z1;
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

		if (ui < 0)
			ui = 1.0f - ui;
		if (vi < 0)
			vi = 1.0f - vi;

		ui = ui - (int)ui;
		vi = vi - (int)vi;

		int ux = ui * (texture->width - 1);
		int vy = vi * (texture->height - 1);

		int index = vy * (texture->width) + ux;
		if (index < 0 || index >= texture->width * texture->height)
			index = 0;


		draw_pixel(pixels, zbuffer, width, height, x, y1, zi, texture->data[index]);
		ui += -du;
		vi += -dv;
		zi += -dz;
	}

}


inline void fill_bottom_triangle(int *pixels, float *zbuffer, const int width, const int height, const texinfo_t *texture, int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3, int z3, int color,
	float u1, float v1,
	float u2, float v2,
	float u3, float v3,
	const int minx, const int maxx, const int miny, const int maxy)
{
	if (y3 - y1 == 0)
		return;

	// flat bottom y2 - y1 == y3 - y1

	float invslope1 = (float)(x2 - x1) / (y3 - y1); // dx / dy
	float invslope2 = (float)(x3 - x1) / (y3 - y1); // dx / dy
	float curx1 = x1;
	float curx2 = x1;

	float uinvslope1 = (float)((u2 - u1)) / (y3 - y1); // du / dy
	float uinvslope2 = (float)((u3 - u1)) / (y3 - y1); // du / dy
	float curu1 = u1;
	float curu2 = u1;
	float vinvslope1 = (float)(v2 - v1) / (y3 - y1); // dv / dy
	float vinvslope2 = (float)(v3 - v1) / (y3 - y1); // dv / dy
	float curv1 = v1;
	float curv2 = v1;



	for (int y = y1; y < y3; y++)
	{
		if (y < miny || y >= maxy)
		{
			curx1 += invslope1;
			curx2 += invslope2;
			curu1 += uinvslope1;
			curu2 += uinvslope2;
			curv1 += vinvslope1;
			curv2 += vinvslope2;
			continue;
		}

		draw_xspan(pixels, zbuffer, width, height, texture, (int)curx1, y, z1, (int)curx2, z2, color, curu1, curv1, curu2, curv2, minx, maxx, miny, maxy);
		curx1 += invslope1;
		curx2 += invslope2;
		curu1 += uinvslope1;
		curu2 += uinvslope2;
		curv1 += vinvslope1;
		curv2 += vinvslope2;
	}
}


inline void fill_top_triangle(int *pixels, float *zbuffer, const int width, const int height, const texinfo_t *texture, int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3, int z3, int color,
	float u1, float v1,
	float u2, float v2,
	float u3, float v3,
	const int minx, const int maxx, const int miny, const int maxy)
{
	if (y3 - y1 == 0)
		return;

	// y3 - y1 == y2 - y1
	float invslope1 = (float)(x3 - x1) / (y3 - y1);
	float invslope2 = (float)(x3 - x2) / (y3 - y1);
	float curx1 = x1;
	float curx2 = x2;


	float uinvslope1 = (float)(u3 - u1) / (y3 - y1); // du / dy
	float uinvslope2 = (float)(u3 - u2) / (y3 - y1); // du / dy
	float curu1 = u1;
	float curu2 = u2;

	float vinvslope1 = (float)(v3 - v1) / (y3 - y1); // dv / dy
	float vinvslope2 = (float)(v3 - v2) / (y3 - y1); // dv / dy
	float curv1 = v1;
	float curv2 = v2;


	// 3
	//12

	for (int y = y1; y < y3; y++)
	{
		if (y < miny || y >= maxy)
		{
			curx1 += invslope1;
			curx2 += invslope2;
			curu1 += uinvslope1;
			curu2 += uinvslope2;
			curv1 += vinvslope1;
			curv2 += vinvslope2;
			continue;
		}

		draw_xspan(pixels, zbuffer, width, height, texture, (int)curx1, y, z1, (int)curx2, z2, color, curu1, curv1, curu2, curv2, minx, maxx, miny, maxy);
		curx1 += invslope1;
		curx2 += invslope2;
		curu1 += uinvslope1;
		curu2 += uinvslope2;
		curv1 += vinvslope1;
		curv2 += vinvslope2;

	}
}

inline void iswap(int &a, int &b)
{
	a ^= b;
	b ^= a;
	a ^= b;
}

void span_triangle(int *pixels, float *zbuffer, const int width, const int height, const texinfo_t *texture,
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
		fill_bottom_triangle(pixels, zbuffer, width, height, texture, x1, y1, z1, x2, y2, z2, x3, y3, z3, c1, u1, v1, u2, v2, u3, v3, minx, maxx, miny, maxy);
	}
	else if (y1 == y2)
	{
		// top triangle
		fill_top_triangle(pixels, zbuffer, width, height, texture, x1, y1, z1, x2, y2, z2, x3, y3, z3, c1, u1, v1, u2, v2, u3, v3, minx, maxx, miny, maxy);
	}
	else
	{
		float t = (float)(y2 - y1) / (y3 - y1);
		int new_x = (int)((x3 - x1) * t + x1);
		int new_y = y2;


		float new_u = (u3 - u1) * t + u1;
		float new_v = -((v3 - v1) * t + v1);

		fill_bottom_triangle(pixels, zbuffer, width, height, texture, x1, y1, z1, x2, y2, z3, new_x, new_y, z3, c1, u1, v1, u2, v2, new_u, new_v, minx, maxx, miny, maxy);
		fill_top_triangle(pixels, zbuffer, width, height, texture, x2, y2, z2, new_x, new_y, z2, x3, y3, z3, c1, u2, v2, new_u, new_v, u3, v3, minx, maxx, miny, maxy);
	}
}

inline int det(int ax, int ay, int bx, int by)
{
	return ax * by - bx *  ay;
}

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
	const int minx, const int maxx, const int miny, const int maxy)
{
//	if (!(w1 && w2 && w3))
//	{
//		return;
//	}

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


	if (max_x == min_x || max_y == min_y)		return;
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

	// find 1/z, u/z, v/z per vertex
	float iz1 = 1.0f / w1;
	float iz2 = 1.0f / w2;
	float iz3 = 1.0f / w3;

	float uiz1 = u1 * iz1;
	float viz1 = v1 * iz1;
	float uiz2 = u2 * iz2;
	float viz2 = v2 * iz2;
	float uiz3 = u3 * iz3;
	float viz3 = v3 * iz3;

	for (int y = min_y; y <= max_y; y++)
	{
		for (int x = min_x; x <= max_x; x++)
		{
			// center xy over origin point
			int qx = (x - x1);
			int qy = (y - y1);


			// barycentric coords (s,t,1-s-t)
			float inv_den = 1.0f / area_denom;
			float s = (float)det(qx, qy, vspan2x, vspan2y) * inv_den;
			float t = (float)det(vspan1x, vspan1y, qx, qy) * inv_den;
			float b = 1.0f - s - t;

			// if inside triangle
			if ((s >= 0.0f) && (t >= 0.0f) && (b >= 0.0f))
			{
				float u, v;

				float iz = s * iz2 + t *  iz3 + b * iz1;
				if (!iz)
				{
					continue;
				}

				// interpolate 1/z, u/z, v/z which are linear equations
				float z = s * z2 + t *  z3 + b * z1;

				if (zbuffer[x + y * width] < z)
				{
					continue;
				}

				float iu = s * uiz2 + t * uiz3 + b * uiz1;
				float iv = s * viz2 + t * viz3 + b * viz1;


				// find inverse / multiply to get perspective correct z, u, v
				float zi = 1.0f / iz;
				u = iu * zi;
				v = iv * zi;

				u = u - (int)u;
				v = v - (int)v;

				int ux = (int)((texture->width - 1) * u);
				int vy = (int)((texture->height - 1) * v);

				if (u < 0)
					ux = (texture->width - 1) + ux;

				if (v < 0)
					vy = (texture->height - 1) + vy;

				if (texture->components == 1)
				{
					char *tex = (char *)texture->data;

					char color = tex[vy * texture->width + ux];
					draw_pixel(pixels, zbuffer, width, height, x, y, z, RGB(color, color, color));
				}
				else if (texture->components == 3)
				{
					rgb_t *rgb = (rgb_t *)texture->data;

					rgb_t color = rgb[vy * texture->width + ux];
					draw_pixel(pixels, zbuffer, width, height, x, y, z, RGB(color.r, color.g, color.b));
				}
				else
				{
					draw_pixel(pixels, zbuffer, width, height, x, y, z, texture->data[vy * texture->width + ux]);
				}
			}
		}
	}
}


void line_intersect(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, float &xint, float &yint)
{
	float num = (x1*y2 - y1*x2) * (x3 - x4) - (x1 - x2) * (x3*y4 - y3*x4);
	float den = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
	xint = num / den;
	num = (x1*y2 - y1*x2) * (y3 - y4) - (y1 - y2) * (x3*y4 - y3*x4);
	yint = num / den;
}

void clip_line(vec4 *points, int &num_point, int x1, int y1, int x2, int y2)
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
			line_intersect(x1, y1, x2, y2, a.x, a.y, b.x, b.y, out[out_num].x, out[out_num].y);
			out_num++;

			out[out_num] = b;
			out_num++;
		}

		// b is outside
		else if (a_pos < 0 && b_pos >= 0)
		{
			// add intersection with edge
			line_intersect(x1, y1, x2, y2, a.x, a.y, b.x, b.y, out[out_num].x, out[out_num].y);
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

void clip2d_sutherland_hodgman(int width, int height, vec4 *points, int &num_point)
{
	clip_line(points, num_point, 0, 0, 0, height);
	clip_line(points, num_point, 0, height, width, height);
	clip_line(points, num_point, width, height, width, 0);
	clip_line(points, num_point, width, 0, 0, 0);
}

void halfspace_triangle(int *pixels, float *zbuffer, int width, int height, const vec2 &v1, const vec2 &v2, const vec2 &v3)
{
	float y1 = v1.y;
	float y2 = v2.y;
	float y3 = v3.y;
	float x1 = v1.x;
	float x2 = v2.x;
	float x3 = v3.x;

	// Bounding rectangle
	int maxx = MAX(x1, MAX(x2, x3));
	int minx = MIN(x1, MIN(x2, x3));
	int maxy = MAX(y1, MAX(y2, y3));
	int miny = MIN(y1, MIN(y2, y3));


	// Scan through bounding rectangle
	for (int y = miny; y < maxy; y++)
	{
		for (int x = minx; x < maxx; x++)
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

int iround(float x)
{
	int xi = x;
	if (x - xi > 0.5f)
		return xi + 1;
	return xi;
}

void halfspace_triangle_fast(int *pixels, float *zbuffer, int width, int height, const vec3 &v1, const vec3 &v2, const vec3 &v3)
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
	int maxx = MAX(X1, MAX(X2, X3));
	int minx = MIN(X1, MIN(X2, X3));
	int maxy = MAX(Y1, MAX(Y2, Y3));
	int miny = MIN(Y1, MIN(Y2, Y3));


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
	if (DY12 < 0 || (DY12 == 0 && DX12 > 0)) C1++;
	if (DY23 < 0 || (DY23 == 0 && DX23 > 0)) C2++;
	if (DY31 < 0 || (DY31 == 0 && DX31 > 0)) C3++;

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



void triangulate(vec4 *point, int &num_point)
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
