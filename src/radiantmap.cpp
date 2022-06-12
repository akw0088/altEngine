#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "RadiantMap.h"

#define MAX_LINE 2048



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

		if (line_num == 1898)
		{
			printf("break\r\n");
		}

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

