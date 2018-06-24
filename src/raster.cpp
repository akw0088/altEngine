#include "raster.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void raster_triangles(int *pixels, int *zbuffer, int width, int height, matrix4 &mvp, int *index_array, vertex_t *vertex_array, int start_index, int start_vertex, int num_index, int num_verts)
{
	for (int i = start_index; i < num_index; i += 3)
	{
		vec4 v1 = mvp * vec4(vertex_array[index_array[i]].position, 1.0f);
		vec4 v2 = mvp * vec4(vertex_array[index_array[i+1]].position, 1.0f);
		vec4 v3 = mvp * vec4(vertex_array[index_array[i+2]].position, 1.0f);

		if (width == 1 || height == 1)
			break;

		// backface cull
		vec3 a = vec3(v2) - vec3(v1);
		vec3 b = vec3(v3) - vec3(v1);
		if (vec3::crossproduct(a,b) * vec3(0, 0, -1) > 0)
			continue;

		// perspective divide
		v1 /= v1.w;
		v2 /= v2.w;
		v3 /= v3.w;

		// [-1,1] -> [0,1]
		v1 *= 0.5f;
		v2 *= 0.5f;
		v3 *= 0.5f;

		v1 += 0.5f;
		v2 += 0.5f;
		v3 += 0.5f;

		//[0,1] -> [0,width]
		v1 *= vec4(width, height, 1, 1);
		v2 *= vec4(width, height, 1, 1);
		v3 *= vec4(width, height, 1, 1);

		if (v1.x < 0 || v1.y < 0 || v2.x < 0 || v2.y < 0 || v3.x < 0 || v3.y < 0)
			continue;
		if ((int)v1.x >= width || (int)v1.y >= height || (int)v2.x >= width || (int)v2.y >= height || (int)v3.x >= height || (int)v3.y >= width)
			continue;

		span_triangle(pixels, zbuffer, width, height,
			v1.x, v1.y, v1.z, RGB(255, 0, 0),
			v2.x, v2.y, v2.z, RGB(255, 0, 0),
			v3.x, v3.y, v3.z, RGB(255, 0, 0));


/*		barycentric_triangle(pixels, zbuffer, width, height,
			v1.x, v1.y, v1.z, RGB(255, 0, 0),
			v2.x, v2.y, v2.z, RGB(255, 0, 0),
			v3.x, v3.y, v3.z, RGB(255, 0, 0));*/
	}

}

inline void draw_pixel(int *pixels, int *zbuffer, int width, int height, int x, int y, int z, unsigned int color)
{
	if (width < 0 || height < 0)
		return;
	if (x >= width || y >= height)
		return;

	if (zbuffer[x + y * width] < z)
	{
//		pixels[x + ((height - 1 - y) * width)] = color;
		pixels[x + (y * width)] = color;
		zbuffer[x + y * width] = z;
	}
}

void draw_line(int *pixels, int width, int height, int x1, int y1, int x2, int y2, int color)
{
	int i;
	int	x, y;
	int flip_axis = 0;
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



inline void draw_xspan(int *pixels, int *zbuffer, int width, int height, int x1, int y1, int z1, int x2, int color)
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


inline void fill_bottom_triangle(int *pixels, int *zbuffer, int width, int height, int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3, int z3, int color)
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

inline void fill_top_triangle(int *pixels, int *zbuffer, int width, int height, int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3, int z3, int color)
{
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

void span_triangle(int *pixels, int *zbuffer, int width, int height, int x1, int y1, int z1, int c1, int x2, int y2, int z2, int c2, int x3, int y3, int z3, int c3)
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

void barycentric_triangle(int *pixels, int *zbuffer, int width, int height, int x1, int y1, int z1, int c1, int x2, int y2, int z2, int c2, int x3, int y3, int z3, int c3)
{
	int max_x = MAX(x1, MAX(x2, x3));
	int min_x = MIN(x1, MIN(x2, x3));
	int max_y = MAX(y1, MAX(y2, y3));
	int min_y = MIN(y1, MIN(y2, y3));

	// triangle spanning vectors
	int vspan1x = (x2 - x1);
	int vspan1y = (y2 - y1);

	int vspan2x = (x3 - x1);
	int vspan2y = (y3 - y1);

	for (int y = min_y; y <= max_y; y++)
	{
		for (int x = min_x; x <= max_x; x++)
		{
			// center xy over origin point
			int qx = (x - x1);
			int qy = (y - y1);

			// area of parallelogram formed by spanning vectors
			int area_denom = det(vspan1x, vspan1y, vspan2x, vspan2y);

			// zero area triangle
			if (area_denom == 0)
				return;

			// barycentric coords (s,t,1-s-t)
			float s = (float)det(qx, qy, vspan2x, vspan2y) / area_denom;
			float t = (float)det(vspan1x, vspan1y, qx, qy) / area_denom;

			int z = s*z1 + t*z2 + (1 - s - t)*z3;

			// if inside triangle
			if ((s >= 0) && (t >= 0) && (s + t <= 1))
			{
				int r = (s*c1);
				int g = (t*c2);
				int b = ((1 - s - t)*c3);
				draw_pixel(pixels, zbuffer, width, height, x, y, z, c1);
			}
		}
	}
}

void line_intersect(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, long &xint, long &yint)
{
	int num = (x1*y2 - y1*x2) * (x3 - x4) - (x1 - x2) * (x3*y4 - y3*x4);
	int den = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
	xint = num / den;
	num = (x1*y2 - y1*x2) * (y3 - y4) - (y1 - y2) * (x3*y4 - y3*x4);
	yint = num / den;
}

void clip_line(POINT *points, int &num_point, int x1, int y1, int x2, int y2)
{
	POINT new_points[512];
	int new_num_point = 0;

	for (int i = 0; i < num_point; i++)
	{
		// get a line from two points (a,b)
		int k = (i + 1);
		if (k == num_point)
			k = 0;

		int ax = points[i].x;
		int ay = points[i].y;
		int bx = points[k].x;
		int by = points[k].y;

		// test points against clip line
		int a_pos = (x2 - x1) * (ay - y1) - (y2 - y1) * (ax - x1);
		int b_pos = (x2 - x1) * (by - y1) - (y2 - y1) * (bx - x1);

		// both points are inside
		if (a_pos < 0 && b_pos < 0)
		{
			// add b
			new_points[new_num_point].x = bx;
			new_points[new_num_point].y = by;
			new_num_point++;
		}

		// a is outside
		else if (a_pos >= 0 && b_pos < 0)
		{
			// add intersection with edge and b
			line_intersect(x1, y1, x2, y2, ax, ay, bx, by, new_points[new_num_point].x, new_points[new_num_point].y);
			new_num_point++;

			new_points[new_num_point].x = bx;
			new_points[new_num_point].y = by;
			new_num_point++;
		}

		// b is outside
		else if (a_pos < 0 && b_pos >= 0)
		{
			// add intersection with edge
			line_intersect(x1, y1, x2, y2, ax, ay, bx, by, new_points[new_num_point].x, new_points[new_num_point].y);
			new_num_point++;
		}

		// both points outside, clipped
	}

	// Copy new points into array
	num_point = new_num_point;
	for (int i = 0; i < num_point; i++)
	{
		points[i].x = new_points[i].x;
		points[i].y = new_points[i].y;
	}
}

int clip2d_sutherland_hodgman(int width, int height, POINT *points, int &num_point)
{
	clip_line(points, num_point, 0, 0, 0, height);
	clip_line(points, num_point, 0, height, width, height);
	clip_line(points, num_point, width, height, width, 0);
	clip_line(points, num_point, width, 0, 0, 0);
	return 0;
}
