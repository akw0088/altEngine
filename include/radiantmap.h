#ifndef RADIANTMAP_H
#define RADIANTMAP_H

#include <stdio.h>

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
} brushplane_t;



typedef struct
{
	char key[128];
	char value[128];
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
	int num_point;
} brushpatch_t;

typedef struct
{
	brushplane_t *plane;
	int num_plane;
	int brush_num;


	brushpatch_t *patch;
	int num_patch;

} brush_t;





typedef struct
{
	keyval_t *keyval;
	int num_keyval;

	brush_name_t name;
	brush_t *brushes;
	int num_brush;

	int ent_number;

} radent_t;


enum
{
	P_NONE,
	P_ENTITY,
	P_BRUSH,
	P_PATCH
};



public:
	RadiantMap();
	int load(char *map, FILE *output);
	int save(char *map, FILE *output);

	radent_t *radent;
	int num_ent;

private:
	int trim(char *data, int length);
	int trim_copy(char *data, int length, char *out);
	int trim_edges(char *data, int length);
	int trim_edges_copy(char *data, int length, char *out);



	void indent(int level, FILE *output);
	int parse_patch(char *line);
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
};

#endif