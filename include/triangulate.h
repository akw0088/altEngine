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
#define MAX_MODE (7)


#define EPSILON 0.001f

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


typedef enum
{
	TRIANGULATE_TRI_POINT,
	TRIANGULATE_CIR_POINT,
	TRIANGULATE_TRI_BAD,
	TRIANGULATE_POLY_SHARED,
	TRIANGULATE_POLY_ALL,
	TRIANGULATE_POST_DELETE,
	TRIANGULATE_OUTPUT,
	TRIANGULATE_VORONOI,
	TRIANGULATE_VORONOI2,
	TRIANGULATE_MAX_ITEM
} TriangulateDrawMode_t;


class Triangulate
{
public:
	// Warning, this is not fully functional yet, any shared edges cause issues
	void BowyerWatson(const vec3 *point, unsigned int num_point, vec3 *tri, unsigned int &num_tri);


#ifdef WIN32
	void debug_BowyerWatson(HDC hdc, const vec3 *point, unsigned int num_point, vec3 *tri, unsigned int &num_tri, float scale, POINT offset);
	static void draw_triangle(HDC hdc, const vec3 &a, const vec3 &b, const vec3 &c, float scale, POINT offset);
	static void draw_circle(HDC hdc, const vec3 &c, float radius, float scale, POINT offset);
	static void draw_line(HDC hdc, const vec3 &a, const vec3 &b, float scale, POINT offset);
	static void draw_point(HDC hdc, const vec3 &point, float size, float scale, POINT offset);
	static void draw_fill(HDC hdc, const vec3 &point, COLORREF color, float scale, POINT offset);
	static int draw_mode;
	static char draw_names[10][80];
#endif
	static int debug_point;

private:
	void compare_edges(const vec3 &T1_a, const vec3 &T1_b, const vec3 &T1_c, const vec3 &T2_a, const vec3 &T2_b, const vec3 &T2_c, vec3 *polygon, unsigned int &num_poly, vec3 *shared, unsigned int &num_shared);
	int add_point_in_polygon(const vec3 &point, vec3 *poly, unsigned int &num_poly, vec3 *tri, unsigned int &num_triangle);
	void get_circum_circle(const vec3 &a, const vec3 &b, const vec3 &c, float &radius, vec3 &center);
	bool point_in_sphere(const vec3 &point, vec3 &origin, float radius);
	bool add_poly(const vec3 &na, const vec3 &nb, vec3 *polygon, unsigned int &num_poly);
	bool point_in_triangle(const vec3 &p, const vec3 &tri_a, const vec3 &tri_b, const vec3 &tri_c);
	bool point_is_same(const vec3 &a, const vec3 &b);

	bool is_triangle_neighbor(vec3 &a1, vec3 &b1, vec3 &c1, vec3 &a2, vec3 &b2, vec3 &c2, vec3 *shared_out, unsigned int &num_shared);

	// These will modify num_triangles
	void delete_triangle(const vec3 &a, const vec3 &b, const vec3 &c, vec3 *triangles, unsigned int &num_triangles);
	void delete_triangle_with_edge(const vec3 &a, const vec3 &b, vec3 *triangles, unsigned int &num_triangles);
	void delete_triangle_with_vertex(const vec3 &a, vec3 *triangle, unsigned int &num_triangle);
};
#endif
