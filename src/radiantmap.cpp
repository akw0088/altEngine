#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "radiantmap.h"
#include "vector.h"
#include <vector>


using namespace std;

#define MAX_LINE 2048
#define MAX(x,y) (x) > (y) ? (x) : (y)
#define MIN(x,y) (x) < (y) ? (x) : (y)


RadiantMap::RadiantMap()
{
	radent = NULL;
	num_ent = 0;
}


int RadiantMap::trim(char *data, int length)
{
	int pos = 0;
	for (int i = 0; i < length; i++)
	{
		char c = data[i];

		if (c == '\r' || c == '\n' || c == ' ')
		{
			continue;
		}
		data[pos++] = c;
	}
	return pos;
}


int RadiantMap::trim_copy(char *data, int length, char *out)
{
	int pos = 0;
	for (int i = 0; i < length; i++)
	{
		char c = data[i];

		if (c == '\r' || c == '\n' || c == ' ')
		{
			continue;
		}
		out[pos++] = c;
	}
	return pos;
}

int RadiantMap::trim_edges_copy(char *data, int length, char *out)
{
	int pos = 0;
	int i = 0;

	for (i = 0; i < length; i++)
	{
		char c = data[i];

		if (c == '\r' || c == '\n' || c == ' ')
		{
			continue;
		}
		else
		{
			break;
		}


		out[pos++] = c;
	}


	for (i = length - 1; i >= 0; i--)
	{
		char c = data[i];

		if (c == '\r' || c == '\n' || c == ' ')
		{
			continue;
		}
		else
		{
			break;
		}
	}

	out[i] = '\0';

	return pos;
}


int RadiantMap::trim_edges(char *data, int length)
{
	int pos = 0;
	int i = 0;
	int leading = 1;

	for (i = 0; i < length; i++)
	{
		char c = data[i];

		if (leading && (c == '\r' || c == '\n' || c == ' '))
		{
			continue;
		}
		else
		{
			leading = 0;
		}


		data[pos++] = c;
	}

	data[pos] = '\0';

	length = strlen(data);
	for (i = length - 1; i >= 0; i--)
	{
		char c = data[i];

		if (c == ' ')
		{
			data[i] = '\0';
			continue;
		}
		else
		{
			break;
		}
	}

	return pos;
}




int RadiantMap::parse_patch(char *line)
{
	if (strcmp(line, "patchDef2\n") == 0)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

int RadiantMap::parse_left_brace(char *line)
{

	if (line[0] == '{')
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

int RadiantMap::parse_left_paren(char *line)
{

	if (line[0] == '(' && line[1] == '\n')
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

int RadiantMap::parse_right_paren(char *line)
{

	if (line[0] == ')'  && line[1] == '\n')
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

int RadiantMap::parse_right_brace(char *line)
{

	if (line[0] == '}')
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

int RadiantMap::parse_name(char *line, brush_name_t *name)
{
	int ret = sscanf(line, "// %s %d",
		name->name,
		&name->number
	);

	if (ret == 2)
	{
		return 0;
	}

	return -1;
}

int RadiantMap::parse_keyval(char *line, keyval_t *keyval)
{
	//"classname" "worldspawn"
	int ret = sscanf(line, "\"%[^\"]\" \"%[^\"]\"",
		keyval->key,
		keyval->value
	);

	if (ret == 2)
	{
		return 0;
	}

	return -1;
}

int RadiantMap::parse_plane(char *line, brushplane_t *brush)
{
	// planex planey planez texture xoff yoff rotatation scalex scaley 
	//(1816 2080 72) (1744 2264 72) (1744 2080 72) q3f_military/tin -16 0 0 0.500000 0.500000 134217728 0 0

	int ret = sscanf(line, "( %d %d %d ) ( %d %d %d ) ( %d %d %d ) %s %d %d %d %f %f %d %d %d",
		&brush->v1[0], &brush->v1[1], &brush->v1[2],
		&brush->v2[0], &brush->v2[1], &brush->v2[2],
		&brush->v3[0], &brush->v3[1], &brush->v3[2],
		brush->name,
		&brush->xoffset,
		&brush->yoffset,
		&brush->rotation,
		&brush->xscale,
		&brush->yscale,
		&brush->contents,
		&brush->flags,
		&brush->values
	);

	if (ret == 18)
	{
		return 0;
	}

	return -1;
}


int RadiantMap::parse_texture(char *line, char *texture)
{
	if (sscanf(line, "%s", texture) == 1)
		return 0;
	else
		return -1;
}


int RadiantMap::parse_patch_control(char *line, patch_control_t *control)
{
	int ret = sscanf(line, "( %d %d %d %d %d )",
			&control->width,
			&control->height,
			&control->contents,
			&control->flags,
			&control->value
	);


	if (ret == 5)
	{
		return 0;
	}

	return -1;
}



int RadiantMap::parse_patch_points(char *line, patch_control_t *control, patch_point_t *point)
{
	//( ( 2136 2176 360 0 0 ) ( 2160 2176 336 0 0.258900 ) ( 2184 2176 312 0 0.517799 ) )
	//( ( 2888 1784 -256 0 0 ) ( 2888 1397.850464 -256 0 6.033587 ) ( 2888 1000 -256 0 12.250000 ) )



	char varpoint[MAX_LINE] = { 0 };
	char match[2 * MAX_LINE] = { 0 };



	for (int i = 0; i < control->height; i++)
	{
		strcat(varpoint, " ( %f %f %f %f %f ) ");
	}

	sprintf(match, "( %s )", varpoint);


	switch (control->height)
	{
	case 3:
	{
		int ret = sscanf(line, match,
			&point->p1[0], &point->p1[1], &point->p1[2], &point->uv1[0], &point->uv1[1],
			&point->p2[0], &point->p2[1], &point->p2[2], &point->uv2[0], &point->uv2[1],
			&point->p3[0], &point->p3[1], &point->p3[2], &point->uv3[0], &point->uv3[1]
		);
		if (ret == 3 * 5)
		{
			return 0;
		}
		else
		{
			return -1;
		}
		break;
	}
	case 4:
	{
		int ret = sscanf(line, match,
			&point->p1[0], &point->p1[1], &point->p1[2], &point->uv1[0], &point->uv1[1],
			&point->p2[0], &point->p2[1], &point->p2[2], &point->uv2[0], &point->uv2[1],
			&point->p3[0], &point->p3[1], &point->p3[2], &point->uv3[0], &point->uv3[1],
			&point->p4[0], &point->p4[1], &point->p4[2], &point->uv4[0], &point->uv4[1]
		);
		if (ret == 4 * 5)
		{
			return 0;
		}
		else
		{
			return -1;
		}
		break;
	}
	case 5:
	{
		int ret = sscanf(line, match,
			&point->p1[0], &point->p1[1], &point->p1[2], &point->uv1[0], &point->uv1[1],
			&point->p2[0], &point->p2[1], &point->p2[2], &point->uv2[0], &point->uv2[1],
			&point->p3[0], &point->p3[1], &point->p3[2], &point->uv3[0], &point->uv3[1],
			&point->p4[0], &point->p4[1], &point->p4[2], &point->uv4[0], &point->uv4[1],
			&point->p5[0], &point->p5[1], &point->p5[2], &point->uv5[0], &point->uv5[1]
		);
		if (ret == 5 * 5)
		{
			return 0;
		}
		else
		{
			return -1;
		}
		break;
	}
	case 6:
	{
		int ret = sscanf(line, match,
			&point->p1[0], &point->p1[1], &point->p1[2], &point->uv1[0], &point->uv1[1],
			&point->p2[0], &point->p2[1], &point->p2[2], &point->uv2[0], &point->uv2[1],
			&point->p3[0], &point->p3[1], &point->p3[2], &point->uv3[0], &point->uv3[1],
			&point->p4[0], &point->p4[1], &point->p4[2], &point->uv4[0], &point->uv4[1],
			&point->p5[0], &point->p5[1], &point->p5[2], &point->uv5[0], &point->uv5[1],
			&point->p6[0], &point->p6[1], &point->p6[2], &point->uv6[0], &point->uv6[1]
		);
		if (ret == 6 * 5)
		{
			return 0;
		}
		else
		{
			return -1;
		}
		break;
	}
	case 7:
	{
		int ret = sscanf(line, match,
			&point->p1[0], &point->p1[1], &point->p1[2], &point->uv1[0], &point->uv1[1],
			&point->p2[0], &point->p2[1], &point->p2[2], &point->uv2[0], &point->uv2[1],
			&point->p3[0], &point->p3[1], &point->p3[2], &point->uv3[0], &point->uv3[1],
			&point->p4[0], &point->p4[1], &point->p4[2], &point->uv4[0], &point->uv4[1],
			&point->p5[0], &point->p5[1], &point->p5[2], &point->uv5[0], &point->uv5[1],
			&point->p6[0], &point->p6[1], &point->p6[2], &point->uv6[0], &point->uv6[1],
			&point->p7[0], &point->p7[1], &point->p7[2], &point->uv7[0], &point->uv7[1]
		);
		if (ret == 7 * 5)
		{
			return 0;
		}
		else
		{
			return -1;
		}
		break;
	}
	case 8:
	{
		int ret = sscanf(line, match,
			&point->p1[0], &point->p1[1], &point->p1[2], &point->uv1[0], &point->uv1[1],
			&point->p2[0], &point->p2[1], &point->p2[2], &point->uv2[0], &point->uv2[1],
			&point->p3[0], &point->p3[1], &point->p3[2], &point->uv3[0], &point->uv3[1],
			&point->p4[0], &point->p4[1], &point->p4[2], &point->uv4[0], &point->uv4[1],
			&point->p5[0], &point->p5[1], &point->p5[2], &point->uv5[0], &point->uv5[1],
			&point->p6[0], &point->p6[1], &point->p6[2], &point->uv6[0], &point->uv6[1],
			&point->p7[0], &point->p7[1], &point->p7[2], &point->uv7[0], &point->uv7[1],
			&point->p8[0], &point->p8[1], &point->p8[2], &point->uv8[0], &point->uv8[1]
		);
		if (ret == 8 * 5)
		{
			return 0;
		}
		else
		{
			return -1;
		}
		break;
	}
	case 9:
	{
		int ret = sscanf(line, match,
			&point->p1[0], &point->p1[1], &point->p1[2], &point->uv1[0], &point->uv1[1],
			&point->p2[0], &point->p2[1], &point->p2[2], &point->uv2[0], &point->uv2[1],
			&point->p3[0], &point->p3[1], &point->p3[2], &point->uv3[0], &point->uv3[1],
			&point->p4[0], &point->p4[1], &point->p4[2], &point->uv4[0], &point->uv4[1],
			&point->p5[0], &point->p5[1], &point->p5[2], &point->uv5[0], &point->uv5[1],
			&point->p6[0], &point->p6[1], &point->p6[2], &point->uv6[0], &point->uv6[1],
			&point->p7[0], &point->p7[1], &point->p7[2], &point->uv7[0], &point->uv7[1],
			&point->p8[0], &point->p8[1], &point->p8[2], &point->uv8[0], &point->uv8[1],
			&point->p9[0], &point->p9[1], &point->p9[2], &point->uv9[0], &point->uv9[1]
		);
		if (ret == 9 * 5)
		{
			return 0;
		}
		else
		{
			return -1;
		}
		break;
	}
	}



	return -1;
}


int RadiantMap::print_patch_points(patch_control_t *control, patch_point_t *point, FILE *output)
{

	char varpoint[MAX_LINE] = { 0 };
	char match[2 * MAX_LINE] = { 0 };

	for (int i = 0; i < control->height; i++)
	{
		strcat(varpoint, " ( %f %f %f %f %f ) ");
	}

	sprintf(match, "( %s )\r\n", varpoint);

	switch (control->height)
	{
	case 3:
	{
		fprintf(output, match,
			point->p1[0], point->p1[1], point->p1[2], point->uv1[0], point->uv1[1],
			point->p2[0], point->p2[1], point->p2[2], point->uv2[0], point->uv2[1],
			point->p3[0], point->p3[1], point->p3[2], point->uv3[0], point->uv3[1]
		);
		break;
	}
	case 4:
	{
		fprintf(output, match,
			point->p1[0], point->p1[1], point->p1[2], point->uv1[0], point->uv1[1],
			point->p2[0], point->p2[1], point->p2[2], point->uv2[0], point->uv2[1],
			point->p3[0], point->p3[1], point->p3[2], point->uv3[0], point->uv3[1],
			point->p4[0], point->p4[1], point->p4[2], point->uv4[0], point->uv4[1]
		);
		break;
	}
	case 5:
	{
		fprintf(output, match,
			point->p1[0], point->p1[1], point->p1[2], point->uv1[0], point->uv1[1],
			point->p2[0], point->p2[1], point->p2[2], point->uv2[0], point->uv2[1],
			point->p3[0], point->p3[1], point->p3[2], point->uv3[0], point->uv3[1],
			point->p4[0], point->p4[1], point->p4[2], point->uv4[0], point->uv4[1],
			point->p5[0], point->p5[1], point->p5[2], point->uv5[0], point->uv5[1]
		);
		break;
	}
	case 6:
	{
		fprintf(output, match,
			point->p1[0], point->p1[1], point->p1[2], point->uv1[0], point->uv1[1],
			point->p2[0], point->p2[1], point->p2[2], point->uv2[0], point->uv2[1],
			point->p3[0], point->p3[1], point->p3[2], point->uv3[0], point->uv3[1],
			point->p4[0], point->p4[1], point->p4[2], point->uv4[0], point->uv4[1],
			point->p5[0], point->p5[1], point->p5[2], point->uv5[0], point->uv5[1],
			point->p6[0], point->p6[1], point->p6[2], point->uv6[0], point->uv6[1]
		);
		break;
	}
	case 7:
	{
		fprintf(output, match,
			point->p1[0], point->p1[1], point->p1[2], point->uv1[0], point->uv1[1],
			point->p2[0], point->p2[1], point->p2[2], point->uv2[0], point->uv2[1],
			point->p3[0], point->p3[1], point->p3[2], point->uv3[0], point->uv3[1],
			point->p4[0], point->p4[1], point->p4[2], point->uv4[0], point->uv4[1],
			point->p5[0], point->p5[1], point->p5[2], point->uv5[0], point->uv5[1],
			point->p6[0], point->p6[1], point->p6[2], point->uv6[0], point->uv6[1],
			point->p7[0], point->p7[1], point->p7[2], point->uv7[0], point->uv7[1]

		);
		break;
	}
	case 8:
	{
		fprintf(output, match,
			point->p1[0], point->p1[1], point->p1[2], point->uv1[0], point->uv1[1],
			point->p2[0], point->p2[1], point->p2[2], point->uv2[0], point->uv2[1],
			point->p3[0], point->p3[1], point->p3[2], point->uv3[0], point->uv3[1],
			point->p4[0], point->p4[1], point->p4[2], point->uv4[0], point->uv4[1],
			point->p5[0], point->p5[1], point->p5[2], point->uv5[0], point->uv5[1],
			point->p6[0], point->p6[1], point->p6[2], point->uv6[0], point->uv6[1],
			point->p7[0], point->p7[1], point->p7[2], point->uv7[0], point->uv7[1],
			point->p8[0], point->p8[1], point->p8[2], point->uv8[0], point->uv8[1]
		);
		break;
	}
	case 9:
	{
		fprintf(output, match,
			point->p1[0], point->p1[1], point->p1[2], point->uv1[0], point->uv1[1],
			point->p2[0], point->p2[1], point->p2[2], point->uv2[0], point->uv2[1],
			point->p3[0], point->p3[1], point->p3[2], point->uv3[0], point->uv3[1],
			point->p4[0], point->p4[1], point->p4[2], point->uv4[0], point->uv4[1],
			point->p5[0], point->p5[1], point->p5[2], point->uv5[0], point->uv5[1],
			point->p6[0], point->p6[1], point->p6[2], point->uv6[0], point->uv6[1],
			point->p7[0], point->p7[1], point->p7[2], point->uv7[0], point->uv7[1],
			point->p8[0], point->p8[1], point->p8[2], point->uv8[0], point->uv8[1],
			point->p9[0], point->p9[1], point->p9[2], point->uv9[0], point->uv9[1]
		);
		break;
	}
	}

	return 0;
}


void RadiantMap::indent(int level, FILE *output)
{
	for (int i = 0; i < level; i++)
	{
		fprintf(output, "\t");
	}
}

int RadiantMap::load(char *map, FILE *output)
{
	FILE *fp = NULL;


	fp = fopen(map, "r");
	if (fp == NULL)
	{
		printf("Unable to open %s\r\n", map);
		return -1;
	}

	int level = 0;
	int line_num = 0;

	int state = P_NONE;
	int num_plane = 0;


	radent = NULL;
	num_ent = 0;

	char line[MAX_LINE] = { 0 };
	while (fgets(line, MAX_LINE + 1, fp) != NULL)
	{
		brushplane_t brushplane;
		brushpatch_t brushpatch;
		keyval_t keyval;
		brush_name_t name;
		patch_control_t control;
		patch_point_t point;

		line_num++;


		trim_edges(line, strlen(line));



		if (state == P_PATCH)
		{
			char texture[1024];


			if (parse_left_brace(line) == 0)
			{
				indent(level++, output);
				fprintf(output, "{\r\n");
			}
			else if (parse_right_brace(line) == 0)
			{
				indent(--level, output);
				fprintf(output, "}\r\n");
				state = P_BRUSH;
			}
			else if (parse_patch_control(line, &control) == 0)
			{
				indent(level, output);

				fprintf(output, "( %d %d %d %d %d )\r\n",
					control.width,
					control.height,
					control.contents,
					control.flags,
					control.value
				);

				radent[num_ent - 1].brushes[radent[num_ent - 1].num_brush - 1].patch[radent[num_ent - 1].brushes[radent[num_ent - 1].num_brush - 1].num_patch - 1].control = control;
			}
			else if (parse_left_paren(line) == 0)
			{
				indent(level++, output);
				fprintf(output, "(\r\n");
			}
			else if (parse_right_paren(line) == 0)
			{
				indent(--level, output);
				fprintf(output, ")\r\n");
			}
			else if (parse_patch_points(line, &control, &point) == 0)
			{
				//( ( 2136 2176 360 0 0 ) ( 2160 2176 336 0 0.258900 ) ( 2184 2176 312 0 0.517799 ) )

				indent(level, output);


				print_patch_points(&control, &point, output);

				radent[num_ent - 1].brushes[radent[num_ent - 1].num_brush - 1].patch[radent[num_ent - 1].brushes[radent[num_ent - 1].num_brush - 1].num_patch - 1].num_point++;


				radent[num_ent - 1].brushes[radent[num_ent - 1].num_brush - 1].patch[radent[num_ent - 1].brushes[radent[num_ent - 1].num_brush - 1].num_patch - 1].points = (patch_point_t *)
					realloc(radent[num_ent - 1].brushes[radent[num_ent - 1].num_brush - 1].patch[radent[num_ent - 1].brushes[radent[num_ent - 1].num_brush - 1].num_patch - 1].points,
						radent[num_ent - 1].brushes[radent[num_ent - 1].num_brush - 1].patch[radent[num_ent - 1].brushes[radent[num_ent - 1].num_brush - 1].num_patch - 1].num_point * sizeof(patch_point_t));

				radent[num_ent - 1].brushes[radent[num_ent - 1].num_brush - 1].patch[radent[num_ent - 1].brushes[radent[num_ent - 1].num_brush - 1].num_patch - 1].points[radent[num_ent - 1].brushes[radent[num_ent - 1].num_brush - 1].patch[radent[num_ent - 1].brushes[radent[num_ent - 1].num_brush - 1].num_patch - 1].num_point - 1] = point;
			}
			else if (parse_texture(line, texture) == 0)
			{
				indent(level, output);
				fprintf(output, "%s\r\n", texture);



				sprintf(radent[num_ent - 1].brushes[radent[num_ent - 1].num_brush - 1].patch[radent[num_ent - 1].brushes[radent[num_ent - 1].num_brush - 1].num_patch - 1].name,
					"%s", texture);
			}



		}
		else if (state == P_BRUSH)
		{
			if (parse_patch(line) == 0)
			{
				indent(level, output);
				fprintf(output, "patchDef2\r\n");
				state = P_PATCH;



				radent[num_ent - 1].brushes[radent[num_ent - 1].num_brush - 1].num_patch++;
				radent[num_ent - 1].brushes[radent[num_ent - 1].num_brush - 1].patch = (brushpatch_t *)
					realloc(radent[num_ent - 1].brushes[radent[num_ent - 1].num_brush - 1].patch,
						radent[num_ent - 1].brushes[radent[num_ent - 1].num_brush - 1].num_patch * sizeof(brushpatch_t));


				brushpatch.name[0] = '\0';
				brushpatch.num_point = 0;
				brushpatch.points = NULL;
				brushpatch.control.width = 0;
				brushpatch.control.height = 0;
				brushpatch.control.contents = 0;
				brushpatch.control.flags = 0;
				brushpatch.control.value = 0;

				radent[num_ent - 1].brushes[radent[num_ent - 1].num_brush - 1].patch[radent[num_ent - 1].brushes[radent[num_ent - 1].num_brush - 1].num_patch - 1] = brushpatch;

			}
			else if (parse_plane(line, &brushplane) == 0)
			{
				indent(level, output);
				fprintf(output, "( %d %d %d ) ( %d %d %d ) ( %d %d %d ) %s %d %d %d %f %f %d %d %d\r\n",
					brushplane.v1[0], brushplane.v1[1], brushplane.v1[2],
					brushplane.v2[0], brushplane.v2[1], brushplane.v2[2],
					brushplane.v3[0], brushplane.v3[1], brushplane.v3[2],
					brushplane.name,
					brushplane.xoffset, brushplane.yoffset,
					brushplane.rotation,
					brushplane.xscale, brushplane.yscale,
					brushplane.contents,
					brushplane.flags,
					brushplane.values
				);

				radent[num_ent - 1].brushes[radent[num_ent - 1].num_brush - 1].num_plane++;
				radent[num_ent - 1].brushes[radent[num_ent - 1].num_brush - 1].plane = (brushplane_t *)
					realloc(radent[num_ent - 1].brushes[radent[num_ent - 1].num_brush - 1].plane,
						radent[num_ent - 1].brushes[radent[num_ent - 1].num_brush - 1].num_plane * sizeof(brushplane_t));

				radent[num_ent - 1].brushes[radent[num_ent - 1].num_brush - 1].plane[radent[num_ent - 1].brushes[radent[num_ent - 1].num_brush - 1].num_plane - 1] = brushplane;


			}
			else if (parse_left_brace(line) == 0)
			{
				indent(level++, output);
				fprintf(output, "{\r\n");
			}
			else if (parse_right_brace(line) == 0)
			{
				indent(--level, output);
				fprintf(output, "}\r\n");
				num_plane = 0;
				state = P_ENTITY;
			}
		}
		else if (state == P_ENTITY)
		{
			if (parse_keyval(line, &keyval) == 0)
			{
				indent(level, output);
				fprintf(output, "\"%s\" \"%s\"\r\n", keyval.key, keyval.value);


				radent[num_ent - 1].num_keyval++;

				radent[num_ent - 1].keyval = (keyval_t *)realloc(radent[num_ent - 1].keyval, radent[num_ent - 1].num_keyval * sizeof(keyval_t));
				sprintf(radent[num_ent - 1].keyval[radent[num_ent - 1].num_keyval - 1].key, "%s", keyval.key);
				sprintf(radent[num_ent - 1].keyval[radent[num_ent - 1].num_keyval - 1].value, "%s", keyval.value);
			}
			else if (parse_left_brace(line) == 0)
			{
				indent(level++, output);
				fprintf(output, "{\r\n");
			}
			else if (parse_right_brace(line) == 0)
			{
				indent(--level, output);
				fprintf(output, "}\r\n");
				state = P_NONE;
			}
			else if (parse_name(line, &name) == 0)
			{
				indent(level, output);
				fprintf(output, "// %s %d\r\n", name.name, name.number);

				if (strcmp(name.name, "brush") == 0)
				{
					state = P_BRUSH;
				}


				radent[num_ent - 1].num_brush++;

				radent[num_ent - 1].brushes = (brush_t *)realloc(radent[num_ent - 1].brushes, radent[num_ent - 1].num_brush * sizeof(brush_t));


				radent[num_ent - 1].brushes[radent[num_ent - 1].num_brush - 1].num_plane = 0;
				radent[num_ent - 1].brushes[radent[num_ent - 1].num_brush - 1].plane = NULL;
				radent[num_ent - 1].brushes[radent[num_ent - 1].num_brush - 1].num_patch = 0;
				radent[num_ent - 1].brushes[radent[num_ent - 1].num_brush - 1].patch = NULL;
				radent[num_ent - 1].brushes[radent[num_ent - 1].num_brush - 1].brush_num = name.number;
			}
		}
		else
		{
			if (parse_name(line, &name) == 0)
			{
				indent(level, output);
				fprintf(output, "// %s %d\r\n", name.name, name.number);


				//technically this line is optional, some maps might no have commented lines
				if (strcmp(name.name, "entity") == 0)
				{
					state = P_ENTITY;

					num_ent++;
					radent = (radent_t *)realloc(radent, num_ent * sizeof(radent_t));

					radent[num_ent - 1].num_brush = 0;
					radent[num_ent - 1].num_keyval = 0;
					radent[num_ent - 1].ent_number = name.number;
					radent[num_ent - 1].name = name;
					radent[num_ent - 1].brushes = NULL;
					radent[num_ent - 1].keyval = NULL;
				}

			}
			else if (parse_left_brace(line) == 0)
			{
				indent(level++, output);
				fprintf(output, "{\r\n");

				if (state == P_NONE)
				{
					state = P_ENTITY;

					num_ent++;
					radent = (radent_t *)realloc(radent, num_ent * sizeof(radent_t));



					brush_name_t name;

					sprintf(name.name, "entity");
					name.number = num_ent - 1;

					radent[num_ent - 1].num_brush = 0;
					radent[num_ent - 1].num_keyval = 0;
					radent[num_ent - 1].ent_number = name.number;
					radent[num_ent - 1].name = name;
					radent[num_ent - 1].brushes = NULL;
					radent[num_ent - 1].keyval = NULL;
				}

			}
			else if (parse_right_brace(line) == 0)
			{
				indent(--level, output);
				fprintf(output, "}\r\n");
			}
			else
			{
				indent(level, output);
				fprintf(output, "Warning: Unknown line [%s]\r\n", line);
			}
		}

	}

	fclose(fp);

	return 0;
}


int RadiantMap::save(char *map, FILE *output)
{
	// save from binary representation


	for (int i = 0; i < num_ent; i++)
	{
		fprintf(output, "// %s %d\r\n", "entity", radent[i].ent_number);
		fprintf(output, "{\r\n");
		for (int j = 0; j < radent[i].num_keyval; j++)
		{
			fprintf(output, "\"%s\" \"%s\"\r\n", radent[i].keyval[j].key, radent[i].keyval[j].value);
		}


		// for each brush
		for (int j = 0; j < radent[i].num_brush; j++)
		{
			fprintf(output, "// brush %d\r\n", radent[i].brushes[j].brush_num);
			fprintf(output, "{\r\n");


			// for each plane 
			for (int k = 0; k < radent[i].brushes[j].num_plane; k++)
			{

				brushplane_t *brushplane = &radent[i].brushes[j].plane[k];

				fprintf(output, "( %d %d %d ) ( %d %d %d ) ( %d %d %d ) %s %d %d %d %f %f %d %d %d\r\n",
					brushplane->v1[0], brushplane->v1[1], brushplane->v1[2],
					brushplane->v2[0], brushplane->v2[1], brushplane->v2[2],
					brushplane->v3[0], brushplane->v3[1], brushplane->v3[2],
					brushplane->name,
					brushplane->xoffset, brushplane->yoffset,
					brushplane->rotation,
					brushplane->xscale, brushplane->yscale,
					brushplane->contents,
					brushplane->flags,
					brushplane->values
				);
			}

			// for each patch (should only ever be one)
			for (int k = 0; k < radent[i].brushes[j].num_patch; k++)
			{
				brushpatch_t *brushpatch = &radent[i].brushes[j].patch[k];

				fprintf(output, "patchDef2\r\n");
				fprintf(output, "{\r\n");
				fprintf(output, "%s\r\n", brushpatch->name);

				fprintf(output, "( %d %d %d %d %d )\r\n",
					brushpatch->control.width,
					brushpatch->control.height,
					brushpatch->control.contents,
					brushpatch->control.flags,
					brushpatch->control.value
				);


				fprintf(output, "(\r\n");
				for (int l = 0; l < radent[i].brushes[j].patch[k].num_point; l++)
				{
					print_patch_points(&radent[i].brushes[j].patch[k].control, &radent[i].brushes[j].patch[k].points[l], output);
				}
				fprintf(output, ")\r\n");
				fprintf(output, "}\r\n");

			}
			fprintf(output, "}\r\n");

		}

		fprintf(output, "}\r\n");
	}

	return 0;
}




bool RadiantMap::get_intersection(plane_t &p1, plane_t &p2, plane_t &p3, vec3 &point)
{
	vec3 cross_p2p3 = vec3::crossproduct(p2.normal, p3.normal);
	float denom = cross_p2p3 * p1.normal;

	if (denom == 0)
	{
//		printf("line formed by p2 and p3 is parallel to plane p1\r\n");
		return false;
	}

	vec3 cross_p3p1 = vec3::crossproduct(p3.normal, p1.normal);
	vec3 cross_p1p2 = vec3::crossproduct(p1.normal, p2.normal);

	point = 
		(cross_p2p3 * p1.d) +
		(cross_p3p1 * p2.d) +
		(cross_p1p2 * p3.d);

	point = point / denom;

	return true;
}



float RadiantMap::DistPointPlane(const vec3 &q, const vec3 &normal, const float d)
{
	return q * normal - d;
}

int RadiantMap::intersect_two_points_plane2(const plane_t &plane, const vertex_t &a, const vertex_t &b, vertex_t &result)
{
	float da = ((a.position * plane.normal) - plane.d) / plane.normal.magnitude();   // distance plane -> point a
	float db = ((b.position * plane.normal) - plane.d) / plane.normal.magnitude();   // distance plane -> point b

	float s = da / (da - db);   // intersection factor (between 0 and 1)

	if (s < 0 || s > 1)
	{
		return -1;
	}

	result.position = a.position + (b.position - a.position) * s;

	// update tex coords
	result.texCoord0 = a.texCoord0 + (b.texCoord0 - a.texCoord0) * s;
	result.texCoord1 = a.texCoord1 + (b.texCoord1 - a.texCoord1) * s;
	return 0;
}

int RadiantMap::intersect_two_points_plane(const plane_t &p, const vertex_t &a, const vertex_t &b, vertex_t &result, float &t)
{
	// plane equation
	// ax + by + cz + d = 0

	// plug in parametric line
	// a*(x0 + vx * t) + b*(y0 + vy * t) + c*(z0 + vz * t) + d = 0

	// solve for t

	// a*x0 + a*vx*t + b*y0 + b*vy*t + c*z0 + c*vz*t + d = 0
	// a*x0 + b*y0 + c*z0 + d + a*vx*t + b*vy*t + c*vz*t = 0
	// a*x0 + b*y0 + c*z0 + d + t*(a*vx + b*vy + c*vz) = 0
	// t*(a*vx + b*vy + c*vz) = -(a*x0 + b*y0 + c*z0 + d)
	// t = -(a*x0 + b*y0 + c*z0 + d) / (a*vx + b*vy + c*vz)


	vec3 origin = a.position;
	vec3 dir = b.position - a.position;

	float denom = (p.normal * dir);
	if (denom == 0.0f)
		return -1;

	t = -(origin * p.normal - p.d) / denom;
	if (t <= 0.0 || t >= 1.0)
		return -1;

	result.position = origin + dir * t;
	result.texCoord0 = a.texCoord0 * (1.0f - t) + b.texCoord0 * t;
	result.texCoord1 = a.texCoord1 * (1.0f - t) + b.texCoord1 * t;
	return 0;
}



// returns positive if CCW negative if CW winding
float RadiantMap::Signed2DTriArea(const vec3 &a, const vec3 &b, const vec3 &c)
{
	return (a.x - c.x) * (b.y - c.y) - (a.y - c.y) * (b.x - c.x);
}

int RadiantMap::intersect_triangle_plane(const plane_t &p, const vertex_t &a, const vertex_t &b, const 	vertex_t &c, vertex_t *result)
{
	// classify points that are out of plane
	inside_t inside;
	float epsilon = 0.0001f;

	// initialize result to given triangle
	result[0] = a;
	result[1] = b;
	result[2] = c;


	float input_area = Signed2DTriArea(a.position, b.position, c.position);


	inside.dword = 0;
	inside.bit.a_in = DistPointPlane(a.position, p.normal, p.d) >= 0;
	inside.bit.b_in = DistPointPlane(b.position, p.normal, p.d) >= 0;
	inside.bit.c_in = DistPointPlane(c.position, p.normal, p.d) >= 0;

	// all points outside plane, early exit
	if (inside.dword == 0)
		return ALL_OUT;

	// all points inside plane, early exit
	if (inside.dword == 7)
		return ALL_IN;

	float t = -1;

	// easy case, one triangle, two points move in
	if (inside.bit.a_in == 1 && inside.bit.b_in == 0 && inside.bit.c_in == 0)
	{
		vertex_t ab, ac;

		int ret = intersect_two_points_plane(p, a, b, ab, t);
		if (ret != 0)
		{
			printf("Error: didnt intersect despite being classified as exiting Line %d\r\n", __LINE__);
			// just render normal
			return ALL_OUT;
		}

		ret = intersect_two_points_plane(p, a, c, ac, t);
		if (ret != 0)
		{
			//printf("Error: didnt intersect despite being classified as exiting\r\n");
			// just render normal
			return ALL_OUT;
		}

		result[0] = a;
		result[1] = ab;
		result[2] = ac;

		float output_area = Signed2DTriArea(a.position, ab.position, ac.position);

		if (!((input_area > 0 && output_area > 0) || (input_area <= 0 && output_area <= 0)))
		{
			result[0] = a;
			result[2] = ab;
			result[1] = ac;
			return CLIPPED_EASY;
		}


		output_area = Signed2DTriArea(result[0].position, result[1].position, result[2].position);
		if (fabs(output_area) > fabs(input_area) + epsilon)
		{
			// clipped triangles should be smaller?
			printf("Clipping error, area increased, discarding triangle Line %d\r\n %f > %f\r\n", __LINE__, output_area, input_area);
			return ALL_OUT;
		}

		return CLIPPED_EASY;
	}
	else if (inside.bit.a_in == 0 && inside.bit.b_in == 1 && inside.bit.c_in == 0)
	{
		vertex_t ba, bc;

		int ret = intersect_two_points_plane(p, b, a, ba, t);
		if (ret != 0)
		{
			printf("Error: didnt intersect despite being classified as exiting Line %d\r\n", __LINE__);
			// just render normal
			return ALL_OUT;
		}

		ret = intersect_two_points_plane(p, b, c, bc, t);
		if (ret != 0)
		{
			printf("Error: didnt intersect despite being classified as exiting Line %d\r\n", __LINE__);
			// just render normal
			return ALL_OUT;
		}

		result[0] = ba;
		result[1] = b;
		result[2] = bc;


		float output_area = Signed2DTriArea(ba.position, b.position, bc.position);


		if (!((input_area > 0 && output_area > 0) || (input_area <= 0 && output_area <= 0)))
		{
			result[0] = ba;
			result[2] = b;
			result[1] = bc;
			return CLIPPED_EASY;
		}

		output_area = Signed2DTriArea(result[0].position, result[1].position, result[2].position);
		if (fabs(output_area) > fabs(input_area) + epsilon)
		{
			// clipped triangles should be smaller?
			printf("Clipping error, area increased, discarding triangle Line %d\r\n %f > %f\r\n", __LINE__, output_area, input_area);
			return ALL_OUT;
		}



		return CLIPPED_EASY;

	}
	else if (inside.bit.a_in == 0 && inside.bit.b_in == 0 && inside.bit.c_in == 1)
	{
		vertex_t ca, cb;

		int ret = intersect_two_points_plane(p, c, a, ca, t);
		if (ret != 0)
		{
			printf("Error: didnt intersect despite being classified as exiting Line %d\r\n", __LINE__);
			// just render normal
			return ALL_OUT;
		}

		ret = intersect_two_points_plane(p, c, b, cb, t);
		if (ret != 0)
		{
			printf("Error: didnt intersect despite being classified as exiting Line %d\r\n", __LINE__);
			// just render normal
			return ALL_OUT;
		}

		result[0] = ca;
		result[1] = cb;
		result[2] = c;


		float output_area = Signed2DTriArea(ca.position, cb.position, c.position);

		if (!((input_area > 0 && output_area > 0) || (input_area <= 0 && output_area <= 0)))
		{
			result[0] = ca;
			result[2] = cb;
			result[1] = c;
			return CLIPPED_EASY;
		}


		output_area = Signed2DTriArea(result[0].position, result[1].position, result[2].position);
		if (fabs(output_area) > fabs(input_area) + epsilon)
		{
			// clipped triangles should be smaller?
			printf("Clipping error, area increased, discarding triangle Line %d\r\n %f > %f\r\n", __LINE__, output_area, input_area);
			return ALL_OUT;
		}


		return CLIPPED_EASY;
	}

	// hard case, tip chopped off, two triangles
	else if (inside.bit.a_in == 0 && inside.bit.b_in == 1 && inside.bit.c_in == 1)
	{
		vertex_t ab;
		vertex_t ca;

		int ret = intersect_two_points_plane(p, a, b, ab, t);
		if (ret != 0)
		{
			printf("Error: didnt intersect despite being classified as exiting Line %d\r\n", __LINE__);
			// just render normal
			return ALL_OUT;
		}

		ret = intersect_two_points_plane(p, a, c, ca, t);
		if (ret != 0)
		{
			printf("Error: didnt intersect despite being classified as exiting Line %d\r\n", __LINE__);
			// just render normal
			return ALL_OUT;
		}

		result[0] = ab;
		result[1] = b;
		result[2] = c;

		result[3] = ca;
		result[4] = ab;
		result[5] = c;


		float output_area1 = Signed2DTriArea(ab.position, b.position, c.position);
		float output_area2 = Signed2DTriArea(ca.position, ab.position, c.position);



		if (!((input_area > 0 && output_area1 > 0) || (input_area <= 0 && output_area1 <= 0)))
		{
			result[0] = ab;
			result[2] = b;
			result[1] = c;
		}

		if (!((input_area > 0 && output_area2 > 0) || (input_area <= 0 && output_area2 <= 0)))
		{
			result[3] = ca;
			result[5] = ab;
			result[4] = c;
		}


		output_area1 = Signed2DTriArea(result[0].position, result[1].position, result[2].position);
		output_area2 = Signed2DTriArea(result[3].position, result[4].position, result[5].position);

		if (fabs(output_area1 + output_area2) > fabs(input_area) + epsilon)
		{
			// clipped triangles should be smaller?
			printf("Clipping error, area increased, discarding triangle Line %d\r\n %f > %f\r\n", __LINE__, output_area1 + output_area2, input_area);
			return ALL_OUT;
		}


		return CLIPPED_HARD;
	}
	else if (inside.bit.a_in == 1 && inside.bit.b_in == 0 && inside.bit.c_in == 1)
	{
		vertex_t ab;
		vertex_t cb;


		int ret = intersect_two_points_plane(p, a, b, ab, t);
		if (ret != 0)
		{
			printf("Error: didnt intersect despite being classified as exiting Line %d\r\n", __LINE__);
			// just render normal
			return ALL_OUT;
		}

		ret = intersect_two_points_plane(p, b, c, cb, t);
		if (ret != 0)
		{
			printf("Error: didnt intersect despite being classified as exiting Line %d\r\n", __LINE__);
			// just render normal
			return ALL_OUT;
		}
		result[0] = a;
		result[1] = ab;
		result[2] = c;

		result[3] = cb;
		result[4] = ab;
		result[5] = c;


		float output_area1 = Signed2DTriArea(a.position, ab.position, c.position);
		float output_area2 = Signed2DTriArea(cb.position, ab.position, c.position);


		if (!((input_area > 0 && output_area1 > 0) || (input_area <= 0 && output_area1 <= 0)))
		{
			result[0] = a;
			result[2] = ab;
			result[1] = c;
		}

		if (!((input_area > 0 && output_area2 > 0) || (input_area <= 0 && output_area2 <= 0)))
		{
			result[3] = cb;
			result[5] = ab;
			result[4] = c;
		}

		output_area1 = Signed2DTriArea(result[0].position, result[1].position, result[2].position);
		output_area2 = Signed2DTriArea(result[3].position, result[4].position, result[5].position);


		if (fabs(output_area1 + output_area2) > fabs(input_area) + epsilon)
		{
			// clipped triangles should be smaller?
			printf("Clipping error, area increased, discarding triangle Line %d\r\n %f > %f\r\n", __LINE__, output_area1 + output_area2, input_area);
			return ALL_OUT;
		}


		return CLIPPED_HARD;
	}
	else if (inside.bit.a_in == 1 && inside.bit.b_in == 1 && inside.bit.c_in == 0)
	{
		vertex_t ac;
		vertex_t bc;

		int ret = intersect_two_points_plane(p, a, c, ac, t);
		if (ret != 0)
		{
			printf("Error: didnt intersect despite being classified as exiting Line %d\r\n", __LINE__);
			// just render normal
			return ALL_OUT;
		}

		ret = intersect_two_points_plane(p, b, c, bc, t);
		if (ret != 0)
		{
			printf("Error: didnt intersect despite being classified as exiting Line %d\r\n", __LINE__);
			// just render normal
			return ALL_OUT;
		}
		result[0] = a;
		result[1] = b;
		result[2] = ac;

		result[3] = bc;
		result[4] = ac;
		result[5] = b;

		float output_area1 = Signed2DTriArea(a.position, b.position, ac.position);
		float output_area2 = Signed2DTriArea(bc.position, ac.position, b.position);



		if (!((input_area > 0 && output_area1 > 0) || (input_area <= 0 && output_area1 <= 0)))
		{
			result[0] = a;
			result[2] = b;
			result[1] = ac;
		}

		if (!((input_area > 0 && output_area2 > 0) || (input_area <= 0 && output_area2 <= 0)))
		{
			result[3] = bc;
			result[5] = ac;
			result[4] = b;
		}

		output_area1 = Signed2DTriArea(result[0].position, result[1].position, result[2].position);
		output_area2 = Signed2DTriArea(result[3].position, result[4].position, result[5].position);

		if (fabs(output_area1 + output_area2) > fabs(input_area) + epsilon)
		{
			// clipped triangles should be smaller?
			printf("Clipping error, area increased, discarding triangle Line %d\r\n %f > %f\r\n", __LINE__, output_area1 + output_area2, input_area);
			return ALL_OUT;
		}

		return CLIPPED_HARD;
	}

	printf("Error: intersect_triangle_plane() shouldn't get here\r\n");
	return ALL_OUT;
}


void RadiantMap::allocate_quads()
{
	quadent.num_brush = radent[0].num_brush;
	quadent.quadbrush = NULL;
	quadent.quadbrush = (quadbrush_t *)realloc(quadent.quadbrush, quadent.num_brush * sizeof(quadbrush_t));

	for (int i = 0; i < quadent.num_brush; i++)
	{
		quadent.quadbrush[i].num_quadplane = radent[0].brushes[i].num_plane;
		quadent.quadbrush[i].quadplane = NULL;
		quadent.quadbrush[i].num_vert = 0;
		quadent.quadbrush[i].vert_array = NULL;
		quadent.quadbrush[i].num_index = 0;
		quadent.quadbrush[i].index_array = NULL;
		quadent.quadbrush[i].quadplane = (quadplane_t *)realloc(quadent.quadbrush[i].quadplane, quadent.quadbrush[i].num_quadplane * sizeof(quadplane_t));
		memset(quadent.quadbrush[i].quadplane, 0, sizeof(quadplane_t));
	}
}


void RadiantMap::generate_quads()
{
	// Only generate quads for entity 0 (worldspawn)
	for (int i = 0; i < radent[0].num_brush; i++)
	{
		for (int j = 0; j < radent[0].brushes[i].num_plane; j++)
		{
			const vec3 a(radent[0].brushes[i].plane[j].v1[0], radent[0].brushes[i].plane[j].v1[1], radent[0].brushes[i].plane[j].v1[2]);
			const vec3 b(radent[0].brushes[i].plane[j].v2[0], radent[0].brushes[i].plane[j].v2[1], radent[0].brushes[i].plane[j].v2[2]);
			const vec3 c(radent[0].brushes[i].plane[j].v3[0], radent[0].brushes[i].plane[j].v3[1], radent[0].brushes[i].plane[j].v3[2]);


			// get parallelogram vertex (draw on paper in 3d and use vector addition)
			vec3 d = a + (c - b);

			// find mid point for quad
			vec3 mid = (a + b + c + d) / 4.0f;

			float size = 100000;
			vec3 big_triangle1[3];

			// extend parallelogram to infinite plane by moving away from midpoint
			big_triangle1[0] = a + ((a - mid) * size);
			big_triangle1[1] = b + ((b - mid) * size);
			big_triangle1[2] = c + ((c - mid) * size);

			vec3 big_triangle2[3];

			// two triangles
			big_triangle2[0] = a + ((a - mid) * size);
			big_triangle2[1] = c + ((c - mid) * size);
			big_triangle2[2] = d + ((d - mid) * size);

			// save original triangles
			quadent.quadbrush[i].quadplane[j].triangle1[0] = a;
			quadent.quadbrush[i].quadplane[j].triangle1[1] = b;
			quadent.quadbrush[i].quadplane[j].triangle1[2] = c;

			quadent.quadbrush[i].quadplane[j].triangle2[0] = a;
			quadent.quadbrush[i].quadplane[j].triangle2[1] = c;
			quadent.quadbrush[i].quadplane[j].triangle2[2] = d;

			// save triangles used for clipping
			int k = 0;
			quadent.quadbrush[i].quadplane[j].triangle_list[k++] = big_triangle1[0];
			quadent.quadbrush[i].quadplane[j].triangle_list[k++] = big_triangle1[1];
			quadent.quadbrush[i].quadplane[j].triangle_list[k++] = big_triangle1[2];
			quadent.quadbrush[i].quadplane[j].triangle_list[k++] = big_triangle2[0];
			quadent.quadbrush[i].quadplane[j].triangle_list[k++] = big_triangle2[1];
			quadent.quadbrush[i].quadplane[j].triangle_list[k++] = big_triangle2[2];
			quadent.quadbrush[i].quadplane[j].num_triangle = 6;

			// save original points of parallelogram
			quadent.quadbrush[i].quadplane[j].a = a;
			quadent.quadbrush[i].quadplane[j].b = b;
			quadent.quadbrush[i].quadplane[j].c = c;
			quadent.quadbrush[i].quadplane[j].d = d;


			// save the plane (assuming CCW ordering)
			vec3 ba = b - a;
			vec3 ca = c - a;

			vec3 normal;


			normal = vec3::crossproduct(ba, ca).normalize();


			
			quadent.quadbrush[i].quadplane[j].plane.normal = normal.normalize();
			quadent.quadbrush[i].quadplane[j].plane.d = a * normal;

		}
	}
}

// TODO: Test / Fix this func
void RadiantMap::clip_quads()
{
	for (int i = 0; i < radent[0].num_brush; i++)
	{
		for (int j = 0; j < radent[0].brushes[i].num_plane; j++)
		{
			for (int l = 0; l < quadent.quadbrush[i].quadplane[j].num_triangle; l += 3)
			{
				vertex_t result[6];
				vertex_t a;
				vertex_t b;
				vertex_t c;

				a.position = quadent.quadbrush[i].quadplane[j].triangle_list[l];
				a.texCoord0.x = radent[0].brushes[i].plane[j].xoffset / radent[0].brushes[i].plane[j].xscale;
				a.texCoord0.y = radent[0].brushes[i].plane[j].yoffset / radent[0].brushes[i].plane[j].yscale;

				b.position = quadent.quadbrush[i].quadplane[j].triangle_list[l+1];
				b.texCoord0.x = radent[0].brushes[i].plane[j].xoffset / radent[0].brushes[i].plane[j].xscale;
				b.texCoord0.y = radent[0].brushes[i].plane[j].yoffset / radent[0].brushes[i].plane[j].yscale;


				c.position = quadent.quadbrush[i].quadplane[j].triangle_list[l+2];
				c.texCoord0.x = radent[0].brushes[i].plane[j].xoffset / radent[0].brushes[i].plane[j].xscale;
				c.texCoord0.y = radent[0].brushes[i].plane[j].yoffset / radent[0].brushes[i].plane[j].yscale;

				for (int k = 0; k < quadent.quadbrush[i].num_quadplane; k++)
				{
					// dont clip against yourself
					if (k == j)
					{
						continue;
					}

					// dont clip against parallel planes
					float dotprod = quadent.quadbrush[i].quadplane[k].plane.normal * quadent.quadbrush[i].quadplane[j].plane.normal;
					if (fabs(dotprod) <= 0.00001f)
					{
						continue;
					}

					int ret = intersect_triangle_plane(quadent.quadbrush[i].quadplane[k].plane, a, b, c, result);

					switch (ret)
					{
					case ALL_IN:
						quadent.quadbrush[i].quadplane[j].triangle_list[l + 0] = a.position;
						quadent.quadbrush[i].quadplane[j].triangle_list[l + 1] = b.position;
						quadent.quadbrush[i].quadplane[j].triangle_list[l + 2] = c.position;
						break;
					case ALL_OUT:
						break;
					case CLIPPED_EASY:
						quadent.quadbrush[i].quadplane[j].triangle_list[l + 0] = result[0].position;
						quadent.quadbrush[i].quadplane[j].triangle_list[l + 1] = result[1].position;
						quadent.quadbrush[i].quadplane[j].triangle_list[l + 2] = result[2].position;
						break;
					case CLIPPED_HARD:
						quadent.quadbrush[i].quadplane[j].triangle_list[l + 0] = result[0].position;
						quadent.quadbrush[i].quadplane[j].triangle_list[l + 1] = result[1].position;
						quadent.quadbrush[i].quadplane[j].triangle_list[l + 2] = result[2].position;


						if (quadent.quadbrush[i].quadplane[j].num_triangle + 3 > 512)
						{
							printf("clipping: exceeded max triangles, triangle dropped\r\n");
							break;
						}
						quadent.quadbrush[i].quadplane[j].triangle_list[quadent.quadbrush[i].quadplane[j].num_triangle + 0] = result[3].position;
						quadent.quadbrush[i].quadplane[j].triangle_list[quadent.quadbrush[i].quadplane[j].num_triangle + 1] = result[4].position;
						quadent.quadbrush[i].quadplane[j].triangle_list[quadent.quadbrush[i].quadplane[j].num_triangle + 2] = result[5].position;
						quadent.quadbrush[i].quadplane[j].num_triangle += 3; // add new triangle to the end
						break;
					}
				}
			}
		}

	}


}


void RadiantMap::intersect_quads()
{
	for (int i = 0; i < radent[0].num_brush; i++)
	{
//		for (int j = 0; j < radent[0].brushes[i].num_plane; j++)
		{
			vec3 point(0.0f, 0.0f, 0.0f);
			plane_t A, B, C, D, E, F;

			memset(&A, 0, sizeof(plane_t));
			memset(&B, 0, sizeof(plane_t));
			memset(&C, 0, sizeof(plane_t));
			memset(&D, 0, sizeof(plane_t));
			memset(&E, 0, sizeof(plane_t));
			memset(&F, 0, sizeof(plane_t));

			// Assuming we have 6 planes (which is most normal square brushes)
			A = quadent.quadbrush[i].quadplane[0].plane; // X+
			B = quadent.quadbrush[i].quadplane[1].plane; // Y-
			C = quadent.quadbrush[i].quadplane[2].plane; // X-
			D = quadent.quadbrush[i].quadplane[3].plane; // Y+
			E = quadent.quadbrush[i].quadplane[4].plane; // Z+
			F = quadent.quadbrush[i].quadplane[5].plane; // Z-

			// Powerset of ABCDEF essentially
			// ABC ABD ABE ABF ACD ACE ACF ADE ADF AEF BCD BCE BCF BDE BDF BEF CDE CDF CEF DEF 
			if (get_intersection(A, B, C, point))
			{
				// X+ Y- X- -- should fail for boxes
				printf("\tintersection ABC %f %f %f\r\n", point.x, point.y, point.z);
				quadent.quadbrush[i].num_vert++;
				quadent.quadbrush[i].vert_array = (vec3 *)realloc(quadent.quadbrush[i].vert_array, (quadent.quadbrush[i].num_vert) * sizeof(vec3));
				quadent.quadbrush[i].vert_array[quadent.quadbrush[i].num_vert - 1] = point;
			}
			if (get_intersection(A, B, D, point))
			{
				// X+ Y- Y+ -- should fail for boxes
				printf("\tintersection ABD %f %f %f\r\n", point.x, point.y, point.z);
				quadent.quadbrush[i].num_vert++;
				quadent.quadbrush[i].vert_array = (vec3 *)realloc(quadent.quadbrush[i].vert_array, (quadent.quadbrush[i].num_vert) * sizeof(vec3));
				quadent.quadbrush[i].vert_array[quadent.quadbrush[i].num_vert - 1] = point;
			}
			if (get_intersection(A, B, E, point))
			{
				// X+ Y- Z+ -- 101
				printf("\tintersection ABE %f %f %f\r\n", point.x, point.y, point.z);
				quadent.quadbrush[i].num_vert++;
				quadent.quadbrush[i].vert_array = (vec3 *)realloc(quadent.quadbrush[i].vert_array, (quadent.quadbrush[i].num_vert) * sizeof(vec3));
				quadent.quadbrush[i].vert_array[quadent.quadbrush[i].num_vert - 1] = point;
			}
			if (get_intersection(A, B, F, point))
			{
				// X+ Y- Z- -- 100
				printf("\tintersection ABF %f %f %f\r\n", point.x, point.y, point.z);
				quadent.quadbrush[i].num_vert++;
				quadent.quadbrush[i].vert_array = (vec3 *)realloc(quadent.quadbrush[i].vert_array, (quadent.quadbrush[i].num_vert) * sizeof(vec3));
				quadent.quadbrush[i].vert_array[quadent.quadbrush[i].num_vert - 1] = point;
			}
			if (get_intersection(A, C, D, point))
			{
				// X+ X- Y+ -- should fail for boxes
				printf("\tintersection ACD %f %f %f\r\n", point.x, point.y, point.z);
				quadent.quadbrush[i].num_vert++;
				quadent.quadbrush[i].vert_array = (vec3 *)realloc(quadent.quadbrush[i].vert_array, (quadent.quadbrush[i].num_vert) * sizeof(vec3));
				quadent.quadbrush[i].vert_array[quadent.quadbrush[i].num_vert - 1] = point;
			}
			if (get_intersection(A, C, E, point))
			{
				// X+ X-  -- should fail for boxes
				printf("\tintersection ACE %f %f %f\r\n", point.x, point.y, point.z);
				quadent.quadbrush[i].num_vert++;
				quadent.quadbrush[i].vert_array = (vec3 *)realloc(quadent.quadbrush[i].vert_array, (quadent.quadbrush[i].num_vert) * sizeof(vec3));
				quadent.quadbrush[i].vert_array[quadent.quadbrush[i].num_vert - 1] = point;
			}
			if (get_intersection(A, C, F, point))
			{
				// X+ X-  -- should fail for boxes
				printf("\tintersection ACF %f %f %f\r\n", point.x, point.y, point.z);
				quadent.quadbrush[i].num_vert++;
				quadent.quadbrush[i].vert_array = (vec3 *)realloc(quadent.quadbrush[i].vert_array, (quadent.quadbrush[i].num_vert) * sizeof(vec3));
				quadent.quadbrush[i].vert_array[quadent.quadbrush[i].num_vert - 1] = point;
			}
			if (get_intersection(A, D, E, point))
			{
				// X+ Y+ Z+ -- 111
				printf("\tintersection ADE %f %f %f\r\n", point.x, point.y, point.z);
				quadent.quadbrush[i].num_vert++;
				quadent.quadbrush[i].vert_array = (vec3 *)realloc(quadent.quadbrush[i].vert_array, (quadent.quadbrush[i].num_vert) * sizeof(vec3));
				quadent.quadbrush[i].vert_array[quadent.quadbrush[i].num_vert - 1] = point;
			}
			if (get_intersection(A, D, F, point))
			{
				// X+ Y+ Z- -- 110
				printf("\tintersection ADF %f %f %f\r\n", point.x, point.y, point.z);
				quadent.quadbrush[i].num_vert++;
				quadent.quadbrush[i].vert_array = (vec3 *)realloc(quadent.quadbrush[i].vert_array, (quadent.quadbrush[i].num_vert) * sizeof(vec3));
				quadent.quadbrush[i].vert_array[quadent.quadbrush[i].num_vert - 1] = point;
			}
			if (get_intersection(A, E, F, point))
			{
				// X+ Z+ Z- -- should fail for boxes
				printf("\tintersection AEF %f %f %f\r\n", point.x, point.y, point.z);
				quadent.quadbrush[i].num_vert++;
				quadent.quadbrush[i].vert_array = (vec3 *)realloc(quadent.quadbrush[i].vert_array, (quadent.quadbrush[i].num_vert) * sizeof(vec3));
				quadent.quadbrush[i].vert_array[quadent.quadbrush[i].num_vert - 1] = point;
			}
			if (get_intersection(B, C, D, point))
			{
				// Y- X- Y+ -- should fail for boxes
				printf("\tintersection BCD %f %f %f\r\n", point.x, point.y, point.z);
				quadent.quadbrush[i].num_vert++;
				quadent.quadbrush[i].vert_array = (vec3 *)realloc(quadent.quadbrush[i].vert_array, (quadent.quadbrush[i].num_vert) * sizeof(vec3));
				quadent.quadbrush[i].vert_array[quadent.quadbrush[i].num_vert - 1] = point;
			}
			if (get_intersection(B, C, E, point))
			{
				// Y- X- Z+ -- should fail for boxes
				printf("\tintersection BCE %f %f %f\r\n", point.x, point.y, point.z);
				quadent.quadbrush[i].num_vert++;
				quadent.quadbrush[i].vert_array = (vec3 *)realloc(quadent.quadbrush[i].vert_array, (quadent.quadbrush[i].num_vert) * sizeof(vec3));
				quadent.quadbrush[i].vert_array[quadent.quadbrush[i].num_vert - 1] = point;
			}
			if (get_intersection(B, C, F, point))
			{
				printf("\tintersection BCF %f %f %f\r\n", point.x, point.y, point.z);
				quadent.quadbrush[i].num_vert++;
				quadent.quadbrush[i].vert_array = (vec3 *)realloc(quadent.quadbrush[i].vert_array, (quadent.quadbrush[i].num_vert) * sizeof(vec3));
				quadent.quadbrush[i].vert_array[quadent.quadbrush[i].num_vert - 1] = point;
			}
			if (get_intersection(B, D, E, point))
			{
				printf("\tintersection BDE %f %f %f\r\n", point.x, point.y, point.z);
				quadent.quadbrush[i].num_vert++;
				quadent.quadbrush[i].vert_array = (vec3 *)realloc(quadent.quadbrush[i].vert_array, (quadent.quadbrush[i].num_vert) * sizeof(vec3));
				quadent.quadbrush[i].vert_array[quadent.quadbrush[i].num_vert - 1] = point;
			}
			if (get_intersection(B, D, F, point))
			{
				printf("\tintersection BDF %f %f %f\r\n", point.x, point.y, point.z);
				quadent.quadbrush[i].num_vert++;
				quadent.quadbrush[i].vert_array = (vec3 *)realloc(quadent.quadbrush[i].vert_array, (quadent.quadbrush[i].num_vert) * sizeof(vec3));
				quadent.quadbrush[i].vert_array[quadent.quadbrush[i].num_vert - 1] = point;
			}
			if (get_intersection(B, E, F, point))
			{
				printf("\tintersection BEF %f %f %f\r\n", point.x, point.y, point.z);
				quadent.quadbrush[i].num_vert++;
				quadent.quadbrush[i].vert_array = (vec3 *)realloc(quadent.quadbrush[i].vert_array, (quadent.quadbrush[i].num_vert) * sizeof(vec3));
				quadent.quadbrush[i].vert_array[quadent.quadbrush[i].num_vert - 1] = point;
			}
			if (get_intersection(C, D, E, point))
			{
				printf("\tintersection CDE %f %f %f\r\n", point.x, point.y, point.z);
				quadent.quadbrush[i].num_vert++;
				quadent.quadbrush[i].vert_array = (vec3 *)realloc(quadent.quadbrush[i].vert_array, (quadent.quadbrush[i].num_vert) * sizeof(vec3));
				quadent.quadbrush[i].vert_array[quadent.quadbrush[i].num_vert - 1] = point;
			}
			if (get_intersection(C, D, F, point))
			{
				printf("\tintersection CDF %f %f %f\r\n", point.x, point.y, point.z);
				quadent.quadbrush[i].num_vert++;
				quadent.quadbrush[i].vert_array = (vec3 *)realloc(quadent.quadbrush[i].vert_array, (quadent.quadbrush[i].num_vert) * sizeof(vec3));
				quadent.quadbrush[i].vert_array[quadent.quadbrush[i].num_vert - 1] = point;
			}
			if (get_intersection(C, E, F, point))
			{
				printf("\tintersection CEF %f %f %f\r\n", point.x, point.y, point.z);
				quadent.quadbrush[i].num_vert++;
				quadent.quadbrush[i].vert_array = (vec3 *)realloc(quadent.quadbrush[i].vert_array, (quadent.quadbrush[i].num_vert) * sizeof(vec3));
				quadent.quadbrush[i].vert_array[quadent.quadbrush[i].num_vert - 1] = point;
			}
			if (get_intersection(D, E, F, point))
			{
				printf("\tintersection DEF %f %f %f\r\n", point.x, point.y, point.z);
				quadent.quadbrush[i].num_vert++;
				quadent.quadbrush[i].vert_array = (vec3 *)realloc(quadent.quadbrush[i].vert_array, (quadent.quadbrush[i].num_vert) * sizeof(vec3));
				quadent.quadbrush[i].vert_array[quadent.quadbrush[i].num_vert - 1] = point;
			}
		}
	}
}




// So this is good for axial 6 plane brushes, but not really for much else
void RadiantMap::intersect_bigbox()
{
	for (int i = 0; i < radent[0].num_brush; i++)
	{
		// idea is you have a large box, then clip that down to the brush
		vec3 aabb[8];
		vec3 output[8];
		vec3 original[8];
		int backfront = -1;


		vec3 intermediate[6][8];


		int intersection_count = 0;
		int intersection_test_count = 0;


		// Original copy so we can ignore diagonals
		original[0] = vec3(-8192, -8192, -8192);
		original[7] = vec3(8192, 8192, 8192);
		original[1] = vec3(original[0].x, original[0].y, original[7].z);
		original[2] = vec3(original[0].x, original[7].y, original[0].z);
		original[3] = vec3(original[0].x, original[7].y, original[7].z);
		original[4] = vec3(original[7].x, original[0].y, original[0].z);
		original[5] = vec3(original[7].x, original[0].y, original[7].z);
		original[6] = vec3(original[7].x, original[7].y, original[0].z);

		// GL_TRAINGLES outward facing faces
		int	index_array[36] = { 
			2,1,0,
			2,3,1,
			5,1,7,
			1,3,7,
			7,6,5,
			6,4,5,
			6,2,0,
			4,6,0,
			1,4,0,
			5,4,1,
			6,3,2,
			6,7,3
		};
			
		vec2 texcoords[36] =
		{
			// Front face
			vec2(0.0f, 0.0f),			//2
			vec2(0.0f, -1.0f),			//1
			vec2(-1.0f, -1.0f),			//3

			vec2(0.0f, 0.0f),			//2
			vec2(-1.0f, -1.0f),			//3
			vec2(-1.0f, 0.0f),			//4


			// Back face
			vec2(0.0f, 0.0f),			//2
			vec2(0.0f, 1.0f),			//1
			vec2(1.0f, 1.0f),			//3

			vec2(0.0f, 0.0f),			//2
			vec2(1.0f, 1.0f),			//3
			vec2(1.0f, 0.0f),			//4

			// Left face
			vec2(0.0f, -1.0f),			//1
			vec2(0.0f, 0.0f),			//2
			vec2(-1.0f, -1.0f),			//3

			vec2(0.0f, 0.0f),			//2
			vec2(-1.0f, 0.0f),			//4
			vec2(-1.0f, -1.0f),			//3

			// Right face
			vec2(0.0f, 1.0f),			//1
			vec2(0.0f, 0.0f),			//2
			vec2(1.0f, 1.0f),			//3

			vec2(0.0f, 0.0f),			//2
			vec2(1.0f, 0.0f),			//4
			vec2(1.0f, 1.0f),			//3

			// Top face
			vec2(0.0f, 1.0f),			//1
			vec2(0.0f, 0.0f),			//2
			vec2(1.0f, 1.0f),			//3

			vec2(0.0f, 0.0f),			//2
			vec2(1.0f, 0.0f),			//4
			vec2(1.0f, 1.0f),			//3

			// Bottom face
			vec2(0.0f, 1.0f),			//1
			vec2(0.0f, 0.0f),			//2
			vec2(1.0f, 1.0f),			//3

			vec2(0.0f, 0.0f),			//2
			vec2(1.0f, 0.0f),			//4
			vec2(1.0f, 1.0f),			//3
		};


		// Eight corners of a big box
		output[0] = vec3(-8192, -8192, -8192);
		output[7] = vec3(8192, 8192, 8192);
		output[1] = vec3(output[0].x, output[0].y, output[7].z);
		output[2] = vec3(output[0].x, output[7].y, output[0].z);
		output[3] = vec3(output[0].x, output[7].y, output[7].z);
		output[4] = vec3(output[7].x, output[0].y, output[0].z);
		output[5] = vec3(output[7].x, output[0].y, output[7].z);
		output[6] = vec3(output[7].x, output[7].y, output[0].z);

		// for each plane
		for (int j = 0; j < 6; j++)
		{
			// for each box point
			for (int k = 0; k < 8; k++)
			{
				intermediate[j][k] = output[k];
			}
		}


		// Good pairs should be (edges) (essentiall 3 axis from point)
		// 0:1, 0:2, 0:4
		// 1:0, 1:3, 1:5
		// 2:0, 2:3, 2:6
		// 3:1, 3:2, 3:7
		// 4:0, 4:5, 4:6
		// 5:1, 5:4, 5:7
		// 6:2, 6:4, 6:7
		// 7:3, 7:5, 7:6

		// 24 edges
		// 12 unique edges
		int set_size = 8;
		for (int j = 0; j < radent[0].brushes[i].num_plane; j++)
		{
			// Eight corners of a big box
			aabb[0] = vec3(-8192, -8192, -8192);
			aabb[7] = vec3(8192, 8192, 8192);
			aabb[1] = vec3(aabb[0].x, aabb[0].y, aabb[7].z);
			aabb[2] = vec3(aabb[0].x, aabb[7].y, aabb[0].z);
			aabb[3] = vec3(aabb[0].x, aabb[7].y, aabb[7].z);
			aabb[4] = vec3(aabb[7].x, aabb[0].y, aabb[0].z);
			aabb[5] = vec3(aabb[7].x, aabb[0].y, aabb[7].z);
			aabb[6] = vec3(aabb[7].x, aabb[7].y, aabb[0].z);

			printf("Plane %d normal %f %f %f d %f\r\n", j,
				quadent.quadbrush[i].quadplane[j].plane.normal.x,
				quadent.quadbrush[i].quadplane[j].plane.normal.y,
				quadent.quadbrush[i].quadplane[j].plane.normal.z,
				quadent.quadbrush[i].quadplane[j].plane.d);


			// (n+r-1!) / (r! * (n-1)!) = 36
			for (int k = 0; k < set_size * set_size; k++)
			{
				int x = k / set_size;
				int y = k % set_size;



				// avoid identical pairs
				if (x == y)
				{
					//					printf("%d:%d skipping identical point\r\n", x, y);
					continue;
				}

				if (DistPointPlane(original[x], quadent.quadbrush[i].quadplane[j].plane.normal, quadent.quadbrush[i].quadplane[j].plane.d) > 0)
				{
					if (DistPointPlane(original[y], quadent.quadbrush[i].quadplane[j].plane.normal, quadent.quadbrush[i].quadplane[j].plane.d) > 0)
					{
						//						printf("%d:%d both points are on front side of plane\r\n", x, y);
						continue;
					}
				}


				if (DistPointPlane(original[x], quadent.quadbrush[i].quadplane[j].plane.normal, quadent.quadbrush[i].quadplane[j].plane.d) < 0)
				{
					if (DistPointPlane(original[y], quadent.quadbrush[i].quadplane[j].plane.normal, quadent.quadbrush[i].quadplane[j].plane.d) < 0)
					{
						//						printf("%d:%d both points are on back side of plane\r\n", x, y);
						continue;
					}
				}

				if (DistPointPlane(original[x], quadent.quadbrush[i].quadplane[j].plane.normal, quadent.quadbrush[i].quadplane[j].plane.d) < 0)
				{
					if (DistPointPlane(original[y], quadent.quadbrush[i].quadplane[j].plane.normal, quadent.quadbrush[i].quadplane[j].plane.d) > 0)
					{
						//						printf("%d:%d crosses plane back:front\r\n", x, y);
						backfront = 1;
					}
				}

				if (DistPointPlane(original[x], quadent.quadbrush[i].quadplane[j].plane.normal, quadent.quadbrush[i].quadplane[j].plane.d) > 0)
				{
					if (DistPointPlane(original[y], quadent.quadbrush[i].quadplane[j].plane.normal, quadent.quadbrush[i].quadplane[j].plane.d) < 0)
					{
						//						printf("%d:%d crosses plane front:back\r\n", x, y);
						backfront = 0;
					}
				}


				vec3 o = original[x] - original[y];
				if (fabs(o.magnitude()) > 2 * 8192 * 1.2) // will be sqrt(2) bigger or more 1.4142
				{
					// avoid diagonal pairs
//					printf("%d:%d vector magnitude greater than a side (edge is a diagonal) %f\r\n", x, y, o.magnitude());
					continue;
				}
				else
				{
					//					printf("%d:%d vector magnitude in range (edge not a diagnol) %f\r\n", x, y, o.magnitude());
				}


				vec3 v = aabb[x] - aabb[y];
				if (fabs(v * quadent.quadbrush[i].quadplane[j].plane.normal) < 0.001f)
				{
					// avoid parallel checks
//					printf("%d:%d vector is parallel to plane %d\r\n", x, y, j);
					continue;
				}

				// move from a to b, result is new b
				vertex_t a;
				vertex_t b;
				vertex_t result;
				float t = -1.0f;

				if (backfront)
				{
					a.position = aabb[x];
					b.position = aabb[y];
				}
				else
				{
					a.position = aabb[y];
					b.position = aabb[x];
				}




				intersection_test_count++;
				if (intersect_two_points_plane(quadent.quadbrush[i].quadplane[j].plane, a, b, result, t) == 0)
				{
					intersection_count++;
					printf("intersection %d:%d with plane %d at time %f\r\n", x, y, j, t);
					b.position = result.position;


					v = a.position - b.position;
					// grid is in increments of 8, so we should have moved some distance (avoid zero length sides)
					if (v.magnitude() > 4.0)
					{
						//aabb[k % set_size] = a.position;
						int save = -1;


						if (backfront)
						{
							save = y;
						}
						else
						{
							save = x;
						}


						aabb[save] = result.position;
						printf("\tintersection found aabb[%d]=(%f %f %f)\r\n", save, b.position.x, b.position.y, b.position.z);


						if (fabs(aabb[save].x) < fabs(intermediate[j][save].x))
							intermediate[j][save].x = aabb[save].x;
						if (fabs(aabb[save].y) < fabs(intermediate[j][save].y))
							intermediate[j][save].y = aabb[save].y;
						if (fabs(aabb[save].z) < fabs(intermediate[j][save].z))
							intermediate[j][save].z = aabb[save].z;



						if (fabs(aabb[save].x) < fabs(output[save].x))
							output[save].x = aabb[save].x;
						if (fabs(aabb[save].y) < fabs(output[save].y))
							output[save].y = aabb[save].y;
						if (fabs(aabb[save].z) < fabs(output[save].z))
							output[save].z = aabb[save].z;
					}
					else
					{
						printf("Warning: Intersection left small delta\r\n");
					}

				}
				else
				{
					//					printf("\tdid not intersect\r\n");
				}
			}


			printf("tested %d intersected %d\r\n", intersection_test_count, intersection_count);
			printf("=============================================\r\n");
			for (int k = 0; k < 8; k++)
			{
				//each aabb should now match brush
				printf("aabb[%d]=(%f %f %f)\r\n", j, intermediate[j][k].x, intermediate[j][k].y, intermediate[j][k].z);
			}
			printf("=============================================\r\n");

		}


		quadent.quadbrush[i].num_vert = 8;
		quadent.quadbrush[i].vert_array = (vec3 *)realloc(quadent.quadbrush[i].vert_array, (quadent.quadbrush[i].num_vert) * sizeof(vec3));

		for (int k = 0; k < quadent.quadbrush[i].num_vert; k++)
		{
			//each aabb should now match brush
			printf("(%f %f %f)\r\n", output[k].x, output[k].y, output[k].z);

			quadent.quadbrush[i].vert_array[k] = output[quadent.quadbrush[i].num_vert - k - 1];

		}

		quadent.quadbrush[i].num_index = 36;
		quadent.quadbrush[i].index_array = (int *)realloc(quadent.quadbrush[i].index_array, (quadent.quadbrush[i].num_index) * sizeof(int));

		for (int k = 0; k < quadent.quadbrush[i].num_index; k++)
		{
			quadent.quadbrush[i].index_array[k] = index_array[k];
		}
		printf("done with brush %d\r\n", i);

	}
}
