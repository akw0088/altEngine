#include "raster.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void raster_triangles(const raster_t type, const int block, int *pixels, float *zbuffer, const int width, const int height,
	const matrix4 &mvp, const int *index_array, const vertex_t *vertex_array, const texinfo_t *texture,
	const int start_index, const int start_vertex, const int num_index, const int num_verts)
{
	for (int i = start_index; i < start_index + num_index; i += 3)
	{
		vec4 v1 = mvp * vec4(vertex_array[start_vertex + index_array[i]].position, 1.0f);
		vec4 v2 = mvp * vec4(vertex_array[start_vertex + index_array[i + 1]].position, 1.0f);
		vec4 v3 = mvp * vec4(vertex_array[start_vertex + index_array[i + 2]].position, 1.0f);

		float s1 = vertex_array[start_vertex + index_array[i]].texCoord0.x;
		float t1 = vertex_array[start_vertex + index_array[i]].texCoord0.y;
		float s2 = vertex_array[start_vertex + index_array[i + 1]].texCoord0.x;
		float t2 = vertex_array[start_vertex + index_array[i + 1]].texCoord0.y;
		float s3 = vertex_array[start_vertex + index_array[i + 2]].texCoord0.x;
		float t3 = vertex_array[start_vertex + index_array[i + 2]].texCoord0.y;

		if (width <= 1 || height <= 1)
			break;

		if (v1.w == 0.0f || v2.w == 0.0f || v3.w == 0.0f)
			continue;

		// perspective divide
		v1.x /= v1.w;
		v1.y /= v1.w;
		v1.z /= v1.w;
		v2.x /= v2.w;
		v2.y /= v2.w;
		v2.z /= v2.w;
		v3.x /= v3.w;
		v3.y /= v3.w;
		v3.z /= v3.w;


		// [-1,1] -> [0,1]
		v1 *= 0.5f;
		v1 += 0.5f;
		v2 *= 0.5f;
		v2 += 0.5f;
		v3 *= 0.5f;
		v3 += 0.5f;

		if (v1.z < 0 && v2.z < 0 && v3.z < 0)
			continue;
		if (v1.z > 1.0001f || v2.z > 1.0001f || v3.z > 1.0001f)
			continue;

		//[0,1] -> [0,width]
		v1 *= vec4(width - 1, height - 1, 1, 1);
		v2 *= vec4(width - 1, height - 1, 1, 1);
		v3 *= vec4(width - 1, height - 1, 1, 1);

		// backface cull
		vec3 a = vec3(v2) - vec3(v1);
		vec3 b = vec3(v3) - vec3(v1);
		if (vec3::crossproduct(a, b) * vec3(0, 0, -1) < 0)
			continue;

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

		for (int j = 0; j < num_point; j += 3)
		{
			if (type == SPAN)
			{
				span_triangle(pixels, zbuffer, width, height,
					tri[j + 0].x, tri[j + 0].y, 0, RGB(255, 0, 0),
					tri[j + 1].x, tri[j + 1].y, 0, RGB(0, 255, 0),
					tri[j + 2].x, tri[j + 2].y, 0, RGB(0, 0, 255));
			}
			else if (type == BARYCENTRIC)
			{
				switch (block)
				{
				case 0:
					barycentric_triangle(pixels, zbuffer, width, height, texture,
						tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
						tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
						tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
						s1, t1, s2, t2, s3, t3, 0, width / 4, 0, height / 4);
					break;
				case 1:
					barycentric_triangle(pixels, zbuffer, width, height, texture,
						tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
						tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
						tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
						s1, t1, s2, t2, s3, t3, width / 4, 2 * width / 4, 0, height / 4);
					break;
				case 2:
					barycentric_triangle(pixels, zbuffer, width, height, texture,
						tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
						tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
						tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
						s1, t1, s2, t2, s3, t3, 2 * width / 4, 3 * width / 4, 0, height / 4);
					break;
				case 3:
					barycentric_triangle(pixels, zbuffer, width, height, texture,
						tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
						tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
						tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
						s1, t1, s2, t2, s3, t3, 3 * width / 4, width, 0, height / 4);
					break;
				case 4:
					// row 2
					barycentric_triangle(pixels, zbuffer, width, height, texture,
						tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
						tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
						tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
						s1, t1, s2, t2, s3, t3, 0, width / 4, height / 4, 2 * height / 4);
					break;
				case 5:
					barycentric_triangle(pixels, zbuffer, width, height, texture,
						tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
						tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
						tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
						s1, t1, s2, t2, s3, t3, width / 4, 2 * width / 4, height / 4, 2 * height / 4);
					break;
				case 6:
					barycentric_triangle(pixels, zbuffer, width, height, texture,
						tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
						tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
						tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
						s1, t1, s2, t2, s3, t3, 2 * width / 4, 3 * width / 4, height / 4, 2 * height / 4);
					break;
				case 7:
					barycentric_triangle(pixels, zbuffer, width, height, texture,
						tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
						tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
						tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
						s1, t1, s2, t2, s3, t3, 3 * width / 4, width, height / 4, 2 * height / 4);
					break;
				case 8:
					// row 3
					barycentric_triangle(pixels, zbuffer, width, height, texture,
						tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
						tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
						tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
						s1, t1, s2, t2, s3, t3, 0, width / 4, 2 * height / 4, 3 * height / 4);
					break;
				case 9:
					barycentric_triangle(pixels, zbuffer, width, height, texture,
						tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
						tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
						tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
						s1, t1, s2, t2, s3, t3, width / 4, 2 * width / 4, 2 * height / 4, 3 * height / 4);
					break;
				case 10:
					barycentric_triangle(pixels, zbuffer, width, height, texture,
						tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
						tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
						tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
						s1, t1, s2, t2, s3, t3, 2 * width / 4, 3 * width / 4, 2 * height / 4, 3 * height / 4);
					break;
				case 11:
					barycentric_triangle(pixels, zbuffer, width, height, texture,
						tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
						tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
						tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
						s1, t1, s2, t2, s3, t3, 3 * width / 4, width, 2 * height / 4, 3 * height / 4);
					break;
				case 12:
					// row 4
					barycentric_triangle(pixels, zbuffer, width, height, texture,
						tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
						tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
						tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
						s1, t1, s2, t2, s3, t3, 0, width / 4, 3 * height / 4, height);
					break;
				case 13:
					barycentric_triangle(pixels, zbuffer, width, height, texture,
						tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
						tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
						tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
						s1, t1, s2, t2, s3, t3, width / 4, 2 * width / 4, 3 * height / 4, height);
					break;
				case 14:
					barycentric_triangle(pixels, zbuffer, width, height, texture,
						tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
						tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
						tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
						s1, t1, s2, t2, s3, t3, 2 * width / 4, 3 * width / 4, 3 * height / 4, height);
					break;
				case 15:
					barycentric_triangle(pixels, zbuffer, width, height, texture,
						tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
						tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
						tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
						s1, t1, s2, t2, s3, t3, 3 * width / 4, width, 3 * height / 4, height);
					break;

				}
			}
			else if (type == HALFSPACE)
			{
				halfspace_triangle_fast(pixels, zbuffer, width, height, tri[j + 0], tri[j + 1], tri[j + 2]);
			}
		}
	}

}

void raster_triangles_strip(const raster_t type, const int block, int *pixels, float *zbuffer, const int width, const int height,
	const matrix4 &mvp, const int *index_array, const vertex_t *vertex_array, const texinfo_t *texture,
	const int start_index, const int start_vertex, const int num_index, const int num_verts)
{
	vec4 v1, v2, v3;
	float s1, s2, s3, t1, t2, t3;
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
			i++;
			even = true;
		}

		if (width <= 1 || height <= 1)
			break;

		if (v1.w == 0.0f || v2.w == 0.0f || v3.w == 0.0f)
			continue;

		// perspective divide
		v1.x /= v1.w;
		v1.y /= v1.w;
		v1.z /= v1.w;
		v2.x /= v2.w;
		v2.y /= v2.w;
		v2.z /= v2.w;
		v3.x /= v3.w;
		v3.y /= v3.w;
		v3.z /= v3.w;

		// [-1,1] -> [0,1]
		v1 *= 0.5f;
		v1 += 0.5f;
		v2 *= 0.5f;
		v2 += 0.5f;
		v3 *= 0.5f;
		v3 += 0.5f;

		if (v1.z < 0 && v2.z < 0 && v3.z < 0)
			continue;
		if (v1.z > 1.0001f || v2.z > 1.0001f || v3.z > 1.0001f)
			continue;

		//[0,1] -> [0,width]
		v1 *= vec4(width - 1, height - 1, 1, 1);
		v2 *= vec4(width - 1, height - 1, 1, 1);
		v3 *= vec4(width - 1, height - 1, 1, 1);

		// backface cull
		vec3 a = vec3(v2) - vec3(v1);
		vec3 b = vec3(v3) - vec3(v1);
		if (vec3::crossproduct(a, b) * vec3(0, 0, -1) < 0)
			continue;

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

		for (int j = 0; j < num_point; j += 3)
		{
			if (type == SPAN)
			{
				span_triangle(pixels, zbuffer, width, height,
					tri[j + 0].x, tri[j + 0].y, 0, RGB(255, 0, 0),
					tri[j + 1].x, tri[j + 1].y, 0, RGB(0, 255, 0),
					tri[j + 2].x, tri[j + 2].y, 0, RGB(0, 0, 255));
			}
			else if (type == BARYCENTRIC)
			{
				switch (block)
				{
				case 0:
					barycentric_triangle(pixels, zbuffer, width, height, texture,
						tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
						tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
						tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
						s1, t1, s2, t2, s3, t3, 0, width / 4, 0, height / 4);
					break;
				case 1:
					barycentric_triangle(pixels, zbuffer, width, height, texture,
						tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
						tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
						tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
						s1, t1, s2, t2, s3, t3, width / 4, 2 * width / 4, 0, height / 4);
					break;
				case 2:
					barycentric_triangle(pixels, zbuffer, width, height, texture,
						tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
						tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
						tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
						s1, t1, s2, t2, s3, t3, 2 * width / 4, 3 * width / 4, 0, height / 4);
					break;
				case 3:
					barycentric_triangle(pixels, zbuffer, width, height, texture,
						tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
						tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
						tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
						s1, t1, s2, t2, s3, t3, 3 * width / 4, width, 0, height / 4);
					break;
				case 4:
					// row 2
					barycentric_triangle(pixels, zbuffer, width, height, texture,
						tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
						tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
						tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
						s1, t1, s2, t2, s3, t3, 0, width / 4, height / 4, 2 * height / 4);
					break;
				case 5:
					barycentric_triangle(pixels, zbuffer, width, height, texture,
						tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
						tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
						tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
						s1, t1, s2, t2, s3, t3, width / 4, 2 * width / 4, height / 4, 2 * height / 4);
					break;
				case 6:
					barycentric_triangle(pixels, zbuffer, width, height, texture,
						tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
						tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
						tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
						s1, t1, s2, t2, s3, t3, 2 * width / 4, 3 * width / 4, height / 4, 2 * height / 4);
					break;
				case 7:
					barycentric_triangle(pixels, zbuffer, width, height, texture,
						tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
						tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
						tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
						s1, t1, s2, t2, s3, t3, 3 * width / 4, width, height / 4, 2 * height / 4);
					break;
				case 8:
					// row 3
					barycentric_triangle(pixels, zbuffer, width, height, texture,
						tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
						tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
						tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
						s1, t1, s2, t2, s3, t3, 0, width / 4, 2 * height / 4, 3 * height / 4);
					break;
				case 9:
					barycentric_triangle(pixels, zbuffer, width, height, texture,
						tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
						tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
						tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
						s1, t1, s2, t2, s3, t3, width / 4, 2 * width / 4, 2 * height / 4, 3 * height / 4);
					break;
				case 10:
					barycentric_triangle(pixels, zbuffer, width, height, texture,
						tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
						tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
						tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
						s1, t1, s2, t2, s3, t3, 2 * width / 4, 3 * width / 4, 2 * height / 4, 3 * height / 4);
					break;
				case 11:
					barycentric_triangle(pixels, zbuffer, width, height, texture,
						tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
						tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
						tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
						s1, t1, s2, t2, s3, t3, 3 * width / 4, width, 2 * height / 4, 3 * height / 4);
					break;
				case 12:
					// row 4
					barycentric_triangle(pixels, zbuffer, width, height, texture,
						tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
						tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
						tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
						s1, t1, s2, t2, s3, t3, 0, width / 4, 3 * height / 4, height);
					break;
				case 13:
					barycentric_triangle(pixels, zbuffer, width, height, texture,
						tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
						tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
						tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
						s1, t1, s2, t2, s3, t3, width / 4, 2 * width / 4, 3 * height / 4, height);
					break;
				case 14:
					barycentric_triangle(pixels, zbuffer, width, height, texture,
						tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
						tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
						tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
						s1, t1, s2, t2, s3, t3, 2 * width / 4, 3 * width / 4, 3 * height / 4, height);
					break;
				case 15:
					barycentric_triangle(pixels, zbuffer, width, height, texture,
						tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
						tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
						tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
						s1, t1, s2, t2, s3, t3, 3 * width / 4, width, 3 * height / 4, height);
					break;

				}
			}
			else if (type == HALFSPACE)
			{
				halfspace_triangle_fast(pixels, zbuffer, width, height, tri[j + 0], tri[j + 1], tri[j + 2]);
			}
		}
	}

}

inline void draw_pixel(int *pixels, float *zbuffer, int width, int height, int x, int y, int z, unsigned int color)
{
	if (zbuffer[x + y * width] > z)
	{
		pixels[x + ((height - 1 - y) * width)] = color;
		zbuffer[x + y * width] = z;
	}
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



inline void draw_xspan(int *pixels, float *zbuffer, int width, int height, int x1, int y1, int z1, int x2, int color)
{
	if (x1 > x2)
	{
		for (int x = x2; x < x1; x++)
		{
			draw_pixel(pixels, zbuffer, width, height, x, y1, z1, color);
		}
	}
	else
	{
		for (int x = x1; x < x2; x++)
		{
			draw_pixel(pixels, zbuffer, width, height, x, y1, z1, color);
		}
	}
}


inline void fill_bottom_triangle(int *pixels, float *zbuffer, int width, int height, int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3, int z3, int color)
{
	if (y2 - y1 == 0 || y3 - y1 == 0)
		return;

	float invslope1 = (float)(x2 - x1) / (y2 - y1);
	float invslope2 = (float)(x3 - x1) / (y3 - y1);

	float curx1 = x1;
	float curx2 = x1;

	for (int y = y1; y <= y2; y++)
	{
		draw_xspan(pixels, zbuffer, width, height, (int)curx1, y, z1, (int)curx2, color);
		curx1 += invslope1;
		curx2 += invslope2;
	}
}

inline void fill_top_triangle(int *pixels, float *zbuffer, int width, int height, int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3, int z3, int color)
{
	if (y3 - y1 == 0 || y3 - y2 == 0)
		return;

	float invslope1 = (float)(x3 - x1) / (y3 - y1);
	float invslope2 = (float)(x3 - x2) / (y3 - y2);

	float curx1 = x3;
	float curx2 = x3;

	for (int y = y3; y > y1; y--)
	{
		draw_xspan(pixels, zbuffer, width, height, (int)curx1, y, z1, (int)curx2, color);
		curx1 -= invslope1;
		curx2 -= invslope2;
	}
}

void span_triangle(int *pixels, float *zbuffer, int width, int height, int x1, int y1, int z1, int c1, int x2, int y2, int z2, int c2, int x3, int y3, int z3, int c3)
{
	// sort y
	if (y1 > y2)
	{
		int temp = x2;
		x2 = x1;
		x1 = temp;

		temp = y2;
		y2 = y1;
		y1 = temp;
	}

	if (y2 > y3)
	{
		int temp = x3;
		x3 = x2;
		x2 = temp;

		temp = y3;
		y3 = y2;
		y2 = temp;
	}

	if (y1 > y2)
	{
		int temp = x2;
		x2 = x1;
		x1 = temp;

		temp = y2;
		y2 = y1;
		y1 = temp;
	}

	if (y2 == y3)
	{
		// bottom triangle
		fill_bottom_triangle(pixels, zbuffer, width, height, x1, y1, z1, x2, y2, z2, x3, y3, z3, c1);
	}
	else if (y1 == y2)
	{
		// top triangle
		fill_top_triangle(pixels, zbuffer, width, height, x1, y1, z1, x2, y2, z2, x3, y3, z3, c1);
	}
	else
	{
		// split triangle
		int x4 = (int)(x1 + ((float)(y2 - y1) / (float)(y3 - y1)) * (x3 - x1));
		int y4 = y2;
		fill_bottom_triangle(pixels, zbuffer, width, height, x1, y1, z1, x2, y2, z3, x4, y4, z3, c1);
		fill_top_triangle(pixels, zbuffer, width, height, x2, y2, z2, x4, y4, z2, x3, y3, z3, c1);
	}
}

inline int det(int ax, int ay, int bx, int by)
{
	return ax * by - bx *  ay;
}

void barycentric_triangle(int *pixels, float *zbuffer, const int width, const int height, const texinfo_t *texture,
	const int x1, const int y1, const float z1, const float w1, const int c1,
	const int x2, const int y2, const float z2, const float w2, const int c2,
	const int x3, const int y3, const float z3, const float w3, const int c3,
	const float u1, const float v1,
	const float u2, const float v2,
	const float u3, const float v3,
	const int minx, const int maxx, const int miny, const int maxy)
{
	if (!(w1 && w2 && w3))
	{
		return;
	}

	int max_x = MAX(x1, MAX(x2, x3));
	int min_x = MIN(x1, MIN(x2, x3));
	int max_y = MAX(y1, MAX(y2, y3));
	int min_y = MIN(y1, MIN(y2, y3));

	if (min_x < minx)
		min_x = minx;
	if (min_y < miny)
		min_y = miny;

	if (max_x < minx)
		max_x = minx;
	if (max_y < miny)
		max_y = miny;

	if (min_x >= maxx)
		min_x = maxx - 1;
	if (min_y >= maxy)
		min_y = maxy - 1;

	if (max_x >= maxx)
		max_x = maxx - 1;
	if (max_y >= maxy)
		max_y = maxy - 1;


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

	float iz1 = 0.0f;
	float iz2 = 0.0f;
	float iz3 = 0.0f;
	float uiz1 = 0.0f;
	float viz1 = 0.0f;
	float uiz2 = 0.0f;
	float viz2 = 0.0f;
	float uiz3 = 0.0f;
	float viz3 = 0.0f;



	// find 1/z, u/z, v/z per vertex
	iz1 = 1.0f / w1;
	iz2 = 1.0f / w2;
	iz3 = 1.0f / w3;

	uiz1 = u1 * iz1;
	viz1 = v1 * iz1;
	uiz2 = u2 * iz2;
	viz2 = v2 * iz2;
	uiz3 = u3 * iz3;
	viz3 = v3 * iz3;


	for (int y = min_y; y <= max_y; y++)
	{
		for (int x = min_x; x <= max_x; x++)
		{
			// center xy over origin point
			int qx = (x - x1);
			int qy = (y - y1);


			// barycentric coords (s,t,1-s-t)
			float s = (float)det(qx, qy, vspan2x, vspan2y) / area_denom;
			float t = (float)det(vspan1x, vspan1y, qx, qy) / area_denom;

			// if inside triangle
			if ((s >= 0) && (t >= 0) && (s + t <= 1))
			{
				float u;
				float v;

				float iz = s * iz2 + t *  iz3 + (1 - s - t) * iz1;
				if (!iz)
				{
					continue;
				}

				// interpolate 1/z, u/z, v/z which are linear equations
				float iu = s * uiz2 + t * uiz3 + (1 - s - t) * uiz1;
				float iv = s * viz2 + t * viz3 + (1 - s - t) * viz1;
				float z = s * z2 + t *  z3 + (1 - s - t) * z1;

				// find inverse / multiply to get perspective correct z, u, v
				float zi = 1.0f / iz;
				u = iu * zi;
				v = iv * zi;


				int ux;
				int vy;
				if (u >= 0)
				{
					ux = (int)((texture->width - 1) * u);
					ux = ux % (texture->width - 1);
				}
				else
				{
					ux = (int)((texture->width - 1) * -u);
					ux = ux % (texture->width - 1);
					ux = texture->width - 1 - ux;
				}

				if (v > 0)
				{
					vy = (int)((texture->height - 1) * v);
					vy = vy % (texture->height - 1);
				}
				else
				{
					vy = (int)((texture->height - 1) * -v);
					vy = vy % (texture->height - 1);
					vy = texture->height - 1 - vy;
				}

				int index = ux + vy * (texture->width);

				if (index <  0 || index >= texture->width * texture->height)
					index = 0;

				draw_pixel(pixels, zbuffer, width, height, x, y, z, texture->data[index]);
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
