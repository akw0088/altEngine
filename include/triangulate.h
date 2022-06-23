#ifndef TRIANGULATE_H
#define TRIANGULATE_H

#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <math.h>
#include <vector>
#include "vector.h"
#include "matrix.h"

#define MAX_POLY (8192)
#define MAX_BAD (8192)
#define MAX_TRIANGLE (8192)

#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define MIN(x,y) ((x) < (y) ? (x) : (y))

typedef struct
{
	unsigned int ab_ab : 1;
	unsigned int ac_ac : 1;
	unsigned int bc_bc : 1;
	unsigned int ab_ac : 1;
	unsigned int ac_bc : 1;
	unsigned int bc_ab : 1;
	unsigned int ab_bc : 1;
	unsigned int ac_ab : 1;
	unsigned int bc_ac : 1;
	unsigned int reserved : 23;
} edge_list_t;


class Triangulate
{
public:
	// Warning, this is not fully functional yet, any shared edges cause issues
	void BowyerWatson(vec3 *point, unsigned int num_point, vec3 *tri, unsigned int num_tri);


#ifdef WIN32
	void debug_BowyerWatson(HDC hdc, vec3 *point, unsigned int num_point, vec3 *tri, unsigned int num_tri, float scale, POINT offset);
	static void draw_triangle(HDC hdc, vec3 &a, vec3 &b, vec3 &c, float scale, POINT offset);
	static void draw_circle(HDC hdc, vec3 &c, float radius, float scale, POINT offset);
	static void draw_line(HDC hdc, vec3 &a, vec3 &b, float scale, POINT offset);
	static void draw_point(HDC hdc, vec3 &point, float scale, POINT offset);
	static unsigned int draw_mode;
	static unsigned int debug_point;
#endif

private:
	void compare_edges(vec3 &T1_a, vec3 &T1_b, vec3 &T1_c, vec3 &T2_a, vec3 &T2_b, vec3 &T2_c, vec3 *polygon, unsigned int &num_poly, vec3 *shared, unsigned int &num_shared);
	int add_point_in_polygon(vec3 &point, vec3 *poly, unsigned int &num_poly, vec3 *tri, unsigned int &num_triangle);
	void get_circum_circle(vec3 &a, vec3 &b, vec3 &c, float &radius, vec3 &center);
	bool point_in_sphere(vec3 &point, vec3 &origin, float radius);
	bool add_poly(vec3 na, vec3 nb, vec3 *polygon, unsigned int &num_poly);

};
#endif
