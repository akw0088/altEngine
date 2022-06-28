#ifndef RADIANTMAP_H
#define RADIANTMAP_H

#include <stdio.h>
#include <stdint.h>
#include "vector.h"
#include "matrix.h"
#include "types.h"
#include "triangulate.h"


#define MAX_OUTPUT 14880 // 32 choose 3 = 4960, multiply by three
#define MAX_BRUSH_PLANE 32
#define MAX_POINT_PER_PLANE 1024
#define MAX_POINT_PER_BRUSH 8192

class RadiantMap
{
	//(1816 2080 72) (1744 2264 72) (1744 2080 72) q3f_military/tin -16 0 0 0.500000 0.500000 134217728 0 0
	typedef struct
	{
		int v1[3];
		int v2[3];
		int v3[3];

		char name[256];

		int xoffset;
		int yoffset;
		int rotation;

		float xscale;
		float yscale;

		int contents;
		int flags;
		int values;

		//doom 3 style
		vec3 p;
		float d;

		float xxscale;
		float xyscale;
		float xoffsetf;
		float yxscale;
		float yyscale;
		float yoffsetf;
	} brushplane_t;



	typedef struct
	{
		char key[512];
		char value[512];
	} keyval_t;


	typedef struct
	{
		char name[128];
		int number;
	} brush_name_t;


	typedef struct
	{
		int width; 	// width = number of points
		int height; // height = points per line
		int contents;
		int flags;
		int value;
	} patch_control_t;


	typedef struct
	{
		float p1[3];
		float uv1[2];
		float p2[3];
		float uv2[2];
		float p3[3];
		float uv3[2];
		float p4[3];
		float uv4[2];
		float p5[3];
		float uv5[2];
		float p6[3];
		float uv6[2];
		float p7[3];
		float uv7[2];
		float p8[3];
		float uv8[2];
		float p9[3];
		float uv9[2];
	} patch_point_t;

	typedef struct
	{
		char name[128];

		patch_control_t control;
		patch_point_t *points;
		unsigned int num_point;
	} brushpatch_t;

	typedef struct
	{
		brushplane_t *plane;
		unsigned int num_plane;
		unsigned int brush_num;


		brushpatch_t *patch;
		unsigned int num_patch;

	} brush_t;





	typedef struct
	{
		keyval_t *keyval;
		unsigned int num_keyval;

		brush_name_t name;
		brush_t *brush;
		unsigned int num_brush;

		unsigned int ent_number;

	} radent_t;

	enum
	{
		P_NONE,
		P_ENTITY,
		P_BRUSH,
		P_PATCH,
		P_PRIMITIVE
	};





	// clip types
	typedef struct
	{
		vec3	normal;		// Plane normal. 
		float	d;			// The plane distance from origin 
	} plane_t;

	typedef enum
	{
		ALL_IN,
		ALL_OUT,
		CLIPPED_EASY,
		CLIPPED_HARD
	} point_result_t;

	typedef struct
	{
		unsigned int
			a_in : 1,
			b_in : 1,
			c_in : 1,
			reserved : 29;
	} inside_bit_t;

	typedef union
	{
		inside_bit_t bit;
		unsigned int dword;
	} inside_t;


	// quad types
	typedef struct
	{
		vec3 triangle1[3];
		vec3 triangle2[3];
		vec3 a, b, c, d;
		plane_t plane;

		// clipping will feed back to next clip
		vec3 triangle_list[512];
		unsigned int num_triangle;
	} quadplane_t;

	typedef struct
	{
		quadplane_t *quadplane;
		unsigned int num_quadplane;

		vec3 *vert_array;
		unsigned int num_vert;

		unsigned int *index_array;
		unsigned int num_index;
	} quadbrush_t;

	typedef struct
	{
		quadbrush_t *quadbrush;
		int num_brush;
	} quadent_t;


public:
	RadiantMap();


	int load(char *map, FILE *output);

	// Quake1, Quake2, Quake3 .map files
	int load_v1(char *map, FILE *output);

	// Doom3, Quake4, Rage .map files (Quake4 and Rage TBD)
	int load_v2(char *map, FILE *output);



	// used to generate .map file from quadent binary representation (mainly to compare it was correct)
	int save(char *map, FILE *output);



	// raw data from file converted to binary
	radent_t *radent;
	unsigned int num_ent;


	// memory processed data
	quadent_t quadent;



	// allocates data for conversion to triangles
	void allocate_quads();

	// uses parsed data to generate better data for manipulation
	void generate_quads();


	// needs work, although intersect may supersede this
	// idea is you should be able to clip a box down to the correct brush triangles
	void clip_quads();


	// works for all planes, but triangulation is like 95% correct
	void intersect_quads();

	// works for axial planes
	void intersect_bigbox();


private:
	// mainly for clipping
	float Signed2DTriArea(const vec3 &a, const vec3 &b, const vec3 &c);
	float DistPointPlane(const vec3 &q, const vec3 &normal, const float d);
	int intersect_two_points_plane2(const plane_t &plane, const vertex_t &a, const vertex_t &b, vertex_t &result);
	int intersect_two_points_plane(const plane_t &p, const vertex_t &a, const vertex_t &b, vertex_t &result, float &t);
	int intersect_triangle_plane(const plane_t &p, const vertex_t &a, const vertex_t &b, const 	vertex_t &c, vertex_t *result);

	// magic function that makes this all work
	bool intersect_three_planes(plane_t &pl, plane_t &p2, plane_t &p3, vec3 &point);



	// Used to generate the index sets of three planes to intersect for each brush
	void Combination(unsigned int *arr, unsigned int n, unsigned int r, unsigned int *output, unsigned int &num_out);
	void combination_recurse(unsigned int arr[], unsigned int data[], unsigned int start, unsigned int end, unsigned int index, unsigned int r, unsigned int *output, unsigned int &num_out);


	// feeble attempts to fix winding issues
	void sort_point(vec3 *point_array, unsigned int num_point, const vec3 &normal);
	bool is_clockwise(const vec3 &a, const vec3 &b, const vec3 &center, const vec3 &normal);


	// generic triangle conversions
	void triangle_fan_to_array(vec3 *point_array, unsigned int num_point, vec3 *triangle_array, unsigned int &num_triangle, vec3 &normal);
	void triangle_strip_to_array(vec3 *point_array, unsigned int num_point, vec3 *triangle_array, unsigned int &num_triangle, vec3 &normal);

	void fix_winding(vec3 *triangle_array, int num_triangle, vec3 &normal);


	// File parsing code
	void indent(int level, FILE *output);
	int parse_patch2(char *line);
	int parse_patch3(char *line);
	int parse_left_brace(char *line);
	int parse_left_paren(char *line);
	int parse_right_paren(char *line);
	int parse_right_brace(char *line);
	int parse_name(char *line, brush_name_t *name);
	int parse_keyval(char *line, keyval_t *keyval);
	int parse_plane(char *line, brushplane_t *brush);
	int parse_texture(char *line, char *texture);
	int parse_patch_control(char *line, patch_control_t *control);
	int parse_patch_points(char *line, patch_control_t *control, patch_point_t *point);
	int print_patch_points(patch_control_t *control, patch_point_t *point, FILE *output);
	int parse_brushdef(char *line);
	int parse_version(char *line);

	int trim(char *data, int length);
	int trim_copy(char *data, int length, char *out);
	int trim_edges(char *data, int length);
	int trim_edges_copy(char *data, int length, char *out);

	int patch_type;
	unsigned int map_type; // 1 = quake1, 3 = quake 3, 4 = doom3, 5 = quake4, 6 = rage
	Triangulate triangulate;
};

#endif
