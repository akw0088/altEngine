#include "include.h"

#ifndef RASTER_H
#define RASTER_H


inline void draw_pixel(int *pixels, int width, int height, int x, int y, int z, unsigned int color)
{
//	if (zbuffer[x + y * width] < -z)
	{
		pixels[x + y * width] = color;
//		zbuffer[x + y * width] = -z;
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



inline void draw_xspan(int *pixels, int width, int height, int x1, int y1, int z1, int x2, int color)
{
	if (x1 > x2)
	{
		for (int x = x2; x < x1; x++)
		{
			draw_pixel(pixels, width, height, x, y1, z1, color);
		}
	}
	else
	{
		for (int x = x1; x < x2; x++)
		{
			draw_pixel(pixels, width, height, x, y1, z1, color);
		}
	}
}


inline void fill_bottom_triangle(int *pixels, int width, int height, int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3, int z3, int color)
{
	float invslope1 = (float)(x2 - x1) / (y2 - y1);
	float invslope2 = (float)(x3 - x1) / (y3 - y1);

	float curx1 = x1;
	float curx2 = x1;

	for (int y = y1; y <= y2; y++)
	{
		draw_xspan(pixels, width, height, (int)curx1, y, z1, (int)curx2, color);
		curx1 += invslope1;
		curx2 += invslope2;
	}
}

inline void fill_top_triangle(int *pixels, int width, int height, int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3, int z3, int color)
{
	float invslope1 = (float)(x3 - x1) / (y3 - y1);
	float invslope2 = (float)(x3 - x2) / (y3 - y2);

	float curx1 = x3;
	float curx2 = x3;

	for (int y = y3; y > y1; y--)
	{
		draw_xspan(pixels, width, height, (int)curx1, y, z1, (int)curx2, color);
		curx1 -= invslope1;
		curx2 -= invslope2;
	}
}

void span_triangle(int *pixels, int width, int height, int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3, int z3, int color)
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
		fill_bottom_triangle(pixels, width, height, x1, y1, z1, x2, y2, z2, x3, y3, z3, color);
	}
	else if (y1 == y2)
	{
		// top triangle
		fill_top_triangle(pixels, width, height, x1, y1, z1, x2, y2, z2, x3, y3, z3, color);
	}
	else
	{
		// split triangle
		int x4 = (int)(x1 + ((float)(y2 - y1) / (float)(y3 - y1)) * (x3 - x1));
		int y4 = y2;
		fill_bottom_triangle(pixels, width, height, x1, y1, z1, x2, y2, z3, x4, y4, z3, color);
		fill_top_triangle(pixels, width, height, x2, y2, z2, x4, y4, z2, x3, y3, z3, color);
	}
}

inline int det(int ax, int ay, int bx, int by)
{
	return ax * by - bx *  ay;
}

void barycentric_triangle(int *pixels, int width, int height, int x1, int y1, int z1, int c1, int x2, int y2, int z2, int c2, int x3, int y3, int z3, int c3)
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
				draw_pixel(pixels, width, height, x, y, z, c1);
			}
		}
	}
}


#endif
