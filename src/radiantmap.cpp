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


///=============================================================================
/// File: radiantmap.cpp
///=============================================================================
/// This class will load a .map file from gtkradiant qeradiant or similar
/// Works with quake1, quake2, quake3 .map files function load_v1
/// Works with doom3, quake4 .map files function load_v2
/// Works with rage .map files load_v3
///
/// Note that this will only load brushes and triangulate brushes
/// if you are interested in bezier surfaces (patchDef2/patchDef3)
/// the q3bsp class will triangulate those, but from the BSP file
/// 
/// My intent for this was really to do the BSP / VIS process myself
/// But having convex volumes for shadow volume generation is a nice side effect
/// I could probably work with the brush definitions in the .bsp file though
///
/// But q3map2 can convert a .bsp file to a .map file pretty well
/// saving again with gtkradiant helps as q3map2 does things a bit differently
///
/// radent contains binary data as read from the .map file
///
/// quadent contains triangulated output, I wanted to keep generated items
/// separate from what was read in
///
/// quadbrush_t has an vbo and ibo index, but all GPU buffers should be
/// generated *outside* of this class in order to keep it portable and
/// not tied to a specific graphics library
///
/// Accepts .map file
/// outputs triangulated brushes
///=============================================================================

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "radiantmap.h"
#include "vector.h"
#include <vector>


using namespace std;

#define MAX_LINE 4096
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define MIN(x,y) ((x) < (y) ? (x) : (y))


///=============================================================================
/// Function: RadiantMap
///=============================================================================
/// Description: Constructor for RadiantMap class
///
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
RadiantMap::RadiantMap()
{
	radent = NULL;
	num_ent = 0;
}

///=============================================================================
/// Function: trim
///=============================================================================
/// Description: trims whitespace for parsing lines in place
///
///
/// Parameters:
///		data	- string to trim
///     length	-  length of string
///
/// Returns:
///		None
///=============================================================================
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


///=============================================================================
/// Function: trim_copy
///=============================================================================
/// Description: trims whitespace for parsing lines makes copy
///
///
/// Parameters:
///		data	- string to trim
///     length	- length of string
///     output	- output string
///
/// Returns:
///		None
///=============================================================================
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

///=============================================================================
/// Function: trim_edges_copy
///=============================================================================
/// Description: trims whitespace for parsing lines
///		this one will only trim from start and end of string
///     generates a copy so original string is intact
///
///
/// Parameters:
///		data	- string to trim
///     length	- length of string
///     output	- output string
///
/// Returns:
///		None
///=============================================================================
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

///=============================================================================
/// Function: trim_edges
///=============================================================================
/// Description: trims whitespace for parsing lines
///		this one will only trim from start and end of string
///
///
/// Parameters:
///		data	- string to trim
///     length	- length of string
///
/// Returns:
///		None
///=============================================================================
int RadiantMap::trim_edges(char *data, int length)
{
	int pos = 0;
	int i = 0;
	int leading = 1;

	for (i = 0; i < length; i++)
	{
		char c = data[i];

		if (leading && (c == '\r' || c == '\n' || c == ' ' || c == '\t'))
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



///=============================================================================
/// Function: parse_brushdef
///=============================================================================
/// Description: this will match a brushDef3 token
///
///
/// Parameters:
///		line	- line from file
///
/// Returns:
///		zero match, non-zero no match
///=============================================================================
int RadiantMap::parse_brushdef(char *line)
{
	if (strstr(line, "brushDef3"))
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

///=============================================================================
/// Function: parse_patch2
///=============================================================================
/// Description: this will match a patchDef2 token
///
///
/// Parameters:
///		line	- line from file
///
/// Returns:
///		zero match, non-zero no match
///=============================================================================
int RadiantMap::parse_patch2(char *line)
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

///=============================================================================
/// Function: parse_patch3
///=============================================================================
/// Description: this will match a patchDef3 token
///
///
/// Parameters:
///		line	- line from file
///
/// Returns:
///		zero match, non-zero no match
///=============================================================================
int RadiantMap::parse_patch3(char *line)
{
	if (strstr(line, "patchDef3") != 0)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}



///=============================================================================
/// Function: parse_left_brace
///=============================================================================
/// Description: this will match a left brace token
///
///
/// Parameters:
///		line	- line from file
///
/// Returns:
///		zero match, non-zero no match
///=============================================================================
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

///=============================================================================
/// Function: parse_right_brace
///=============================================================================
/// Description: this will match a right brace token
///
///
/// Parameters:
///		line	- line from file
///
/// Returns:
///		zero match, non-zero no match
///=============================================================================
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

///=============================================================================
/// Function: parse_left_paren
///=============================================================================
/// Description: this will match a left paren token
///
///
/// Parameters:
///		line	- line from file
///
/// Returns:
///		zero match, non-zero no match
///=============================================================================
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

///=============================================================================
/// Function: parse_right_paren
///=============================================================================
/// Description: this will match a right paren token
///
///
/// Parameters:
///		line	- line from file
///
/// Returns:
///		zero match, non-zero no match
///=============================================================================
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

///=============================================================================
/// Function: parse_version
///=============================================================================
/// Description: this will match a version line
///
/// Version2 - Doom3
/// Version3 - Quake4 or Rage
///
///
/// Parameters:
///		line	- line from file
///
/// Returns:
///		zero match, non-zero no match
///=============================================================================
int RadiantMap::parse_version(char *line)
{
	if (strstr(line, "Version 2"))
	{
		map_type = 4;
		return 0;
	}
	else if (strstr(line, "Version 3"))
	{
		map_type = 5;
		return 0;
	}
	else
	{
		return -1;
	}
}



///=============================================================================
/// Function: parse_name
///=============================================================================
/// Description: this will match a "name" which is really a comment line
///
/// Usually indicates brush number or entity number as output by radiant .map
/// really kept around for informational purposes, but not extremely necessary
///
///
/// Parameters:
///		line	- line from file
///
/// Returns:
///		zero match, non-zero no match
///=============================================================================
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

///=============================================================================
/// Function: parse_keyval
///=============================================================================
/// Description: this will match a keyvalue pair
///
/// So Entities will have keyvalue pairs which are used for a lot of purposes
///
///
/// Parameters:
///		line	- line from file
///
/// Returns:
///		zero match, non-zero no match
///=============================================================================
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

///=============================================================================
/// Function: parse_plane
///=============================================================================
/// Description: this will match a brush plane
///
/// Brush planes should have three points on the plane, a texture/shader name
/// and texture scale/offset information
/// contnets, flags, values are used for things like CONTENTS_NONSOLID or other
/// items that are most likely populated by the map compiler
///
/// Note that the brush points may not actually be brush verticies
/// brush points should be integers, but q3map2 converts bsp to floats
///
/// idea being we do not definte float/double if we use integer positions
///
/// Doom3/Quake4 changed from points to just giving us the plane equation directly
///
///
/// Parameters:
///		line	- line from file
///
/// Returns:
///		zero match, non-zero no match
///=============================================================================
int RadiantMap::parse_plane(char *line, brushplane_t *brush)
{
	// planex planey planez texture xoff yoff rotatation scalex scaley 
	//(1816 2080 72) (1744 2264 72) (1744 2080 72) q3f_military/tin -16 0 0 0.500000 0.500000 134217728 0 0

	// quake 3 style
    // really the points should be ints, but q3map2 convert makes them floats, opening and saving with radiant fixes it, but floats work too I guess
	//		( -1024.000 784.000 136.000 ) ( -1024.000 784.000 128.000 ) ( -1024.000 800.000 128.000 ) gothic_trim/pitted_rust 0 0 0 0.5 0.5 0 0 0
	int ret = sscanf(line, "( %f %f %f ) ( %f %f %f ) ( %f %f %f ) %s %d %d %d %f %f %d %d %d",
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
		map_type = 3;
		return 0;
	}

	// quake1 style
	ret = sscanf(line, "( %f %f %f ) ( %f %f %f ) ( %f %f %f ) %s %d %d %d %f %f",
		&brush->v1[0], &brush->v1[1], &brush->v1[2],
		&brush->v2[0], &brush->v2[1], &brush->v2[2],
		&brush->v3[0], &brush->v3[1], &brush->v3[2],
		brush->name,
		&brush->xoffset,
		&brush->yoffset,
		&brush->rotation,
		&brush->xscale,
		&brush->yscale
	);

	if (ret == 15)
	{
		map_type = 1;
		return 0;
	}

	//  ( 0 0 -1 48 ) ( ( 0.0625 0 0 ) ( 0 0.0625 0 ) ) "textures/common/clip" 0 0 0
	// doom3 style
	// (plane equation) ( ( xxscale xyscale xoffset ) ( yxscale yyscale yoffset ) ) "material" ?

	ret = sscanf(line, "( %f %f %f %f ) ( ( %f %f %f ) ( %f %f %f ) ) \"%[^\"]\" %d %d %d",
		&brush->p.x, &brush->p.y, &brush->p.z, &brush->d,
		&brush->xxscale, &brush->xyscale, &brush->xoffsetf,
		&brush->yxscale, &brush->yyscale, &brush->yoffsetf,
		brush->name,
		&brush->contents,
		&brush->flags,
		&brush->values
	);

	if (ret == 14)
	{
		map_type = 4;
		return 0;
	}


	//quake4 style
	//(0 1 0 - 4160) ((0.03125 0 0) (0 0.03125 0)) "textures/common/player_clip"
	ret = sscanf(line, "( %f %f %f %f ) ( ( %f %f %f ) ( %f %f %f ) ) \"%[^\"]\"",
		&brush->p.x, &brush->p.y, &brush->p.z, &brush->d,
		&brush->xxscale, &brush->xyscale, &brush->xoffsetf,
		&brush->yxscale, &brush->yyscale, &brush->yoffsetf,
		brush->name
	);

	if (ret == 11)
	{
		map_type = 5;
		return 0;
	}


	return -1;
}


///=============================================================================
/// Function: parse_texture
///=============================================================================
/// Description: This will parse a texture for the bezier curves / patchDef
///
/// 
///
///
/// Parameters:
///		line	- line from file
///
/// Returns:
///		zero match, non-zero no match
///=============================================================================
int RadiantMap::parse_texture(char *line, char *texture)
{
	if (sscanf(line, "%s", texture) == 1)
		return 0;
	else
		return -1;
}

///=============================================================================
/// Function: parse_patch_control
///=============================================================================
/// Description: This will parse data from a bezier curve
///
/// Should probably rename function as this just defines the width / height
/// and not the control points themselves
/// 
///
///
/// Parameters:
///		line	- line from file
///
/// Returns:
///		zero match, non-zero no match
///=============================================================================
int RadiantMap::parse_patch_control(char *line, patch_control_t *control)
{

	if (patch_type == 2)
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
	}
	else
	{
	//	( xoffset yoffset rotation xscale yscale )
		int ret = sscanf(line, "( %d %d %d %d %d %d %d )",
			&control->width,
			&control->height,
			&control->contents,
			&control->flags,
			&control->value,
			&control->value,
			&control->value
		);

		if (ret == 7)
		{
			return 0;
		}
	}

	return -1;
}


///=============================================================================
/// Function: parse_patch_points
///=============================================================================
/// Description: This will parse data from a bezier curve
///
/// This function will read the patch control points
///
/// They will be x, y, z, u, v for each point
/// 
/// The number of points will be defined by the previously read width / height
/// where height is the number of points per line
/// and width is the number of lines
///
/// Note: for quake3 at least, all of these can be subdivided into multiple 3x3 patches
/// sort of like a image kernel / convolution
///
///
/// Parameters:
///		line	- line from file
///
/// Returns:
///		zero match, non-zero no match
///=============================================================================
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

///=============================================================================
/// Function: print_patch_points
///=============================================================================
/// Description: This will output the parsed patch points
///
/// We print out in save and while parsing, allows you to do a diff on the output
/// to be sure that it matches the input
///
///
/// Parameters:
///		line	- line from file
///
/// Returns:
///		zero success, non-zero failure
///=============================================================================
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


///=============================================================================
/// Function: indent
///=============================================================================
/// Description: outputs tabs based on level
///
/// So this just prints tabs based on the level, which was a count of brace
/// nesting depth, not necessary, but makes reading the file a bit easier
///
///
/// Parameters:
///		line	- line from file
///
/// Returns:
///		None
///=============================================================================
void RadiantMap::indent(int level, FILE *output)
{
	for (int i = 0; i < level; i++)
	{
		fprintf(output, "\t");
	}
}


///=============================================================================
/// Function: load
///=============================================================================
/// Description: loads a .map file
///
/// So this is the first stage of loading, originally this function did everything
/// but not it sort of detects the map type and then hands it off to a map specific
/// loader
///
///
/// Parameters:
///		map	- file to parse
///     output - file to output what was read 
///       (output should match input .map file when done)
///
/// Returns:
///		zero success, non-zero failure
///=============================================================================
int RadiantMap::load(char *map, FILE *output)
{
	FILE *fp = NULL;


	fp = fopen(map, "r");
	if (fp == NULL)
	{
		printf("Unable to open %s\r\n", map);
		return -1;
	}

	char line[MAX_LINE] = { 0 };

	unsigned int line_num = 0;


	// assume a quake1/quake2/quake3 map
	map_type = 3;


	while (fgets(line, MAX_LINE + 1, fp) != NULL)
	{
		// version line will mean doom3/quake4 or rage
		parse_version(line);


		// if we have inherit, definitely a rage map
		if (strstr(line, "inherit"))
		{
			map_type = 6;
			break;
		}

		line_num++;
			
		if (line_num > 10)
		{
			break;
		}
	}
	fclose(fp);


	if (map_type == 4 || map_type == 5)
	{
		// doom3 or quake4
		load_v2(map, output);
	}
	else if (map_type == 6)
	{
		// rage map
		load_v3(map, output);
	}
	else
	{
		// quake1/quake2/quake3
		load_v1(map, output);
	}


	return 0;
}


///=============================================================================
/// Function: load_v1
///=============================================================================
/// Description: loads a .map file
///
/// This will load quake1, quake2, and quake3 .map files
///
///
/// Parameters:
///		map	- file to parse
///     output - file to output what was read 
///       (output should match input .map file when done)
///
/// Returns:
///		zero success, non-zero failure
///=============================================================================
int RadiantMap::load_v1(char *map, FILE *output)
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


	patch_type = 2;

	radent = NULL;
	num_ent = 0;

	char line[MAX_LINE] = { 0 };
	while (fgets(line, MAX_LINE + 1, fp) != NULL)
	{
		brushplane_t brushplane = { 0 };
		brushpatch_t brushpatch = { 0 };
		keyval_t keyval = { 0 };
		brush_name_t name = { 0 };
		patch_control_t control = { 0 };
		patch_point_t point = { 0 };

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

				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].patch[radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_patch - 1].control = control;
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

				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].patch[radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_patch - 1].num_point++;


				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].patch[radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_patch - 1].points = (patch_point_t *)
					realloc(radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].patch[radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_patch - 1].points,
						radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].patch[radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_patch - 1].num_point * sizeof(patch_point_t));

				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].patch[radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_patch - 1].points[radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].patch[radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_patch - 1].num_point - 1] = point;
			}
			else if (parse_texture(line, texture) == 0)
			{
				indent(level, output);
				fprintf(output, "%s\r\n", texture);



				sprintf(radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].patch[radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_patch - 1].name,
					"%s", texture);
			}



		}
		else if (state == P_BRUSH)
		{
			if (parse_patch2(line) == 0)
			{
				indent(level, output);
				fprintf(output, "patchDef2\r\n");
				state = P_PATCH;



				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_patch++;
				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].patch = (brushpatch_t *)
					realloc(radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].patch,
						radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_patch * sizeof(brushpatch_t));


				brushpatch.name[0] = '\0';
				brushpatch.num_point = 0;
				brushpatch.points = NULL;
				brushpatch.control.width = 0;
				brushpatch.control.height = 0;
				brushpatch.control.contents = 0;
				brushpatch.control.flags = 0;
				brushpatch.control.value = 0;

				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].patch[radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_patch - 1] = brushpatch;

			}
			else if (parse_plane(line, &brushplane) == 0)
			{
				indent(level, output);
				fprintf(output, "( %g %g %g ) ( %g %g %g ) ( %g %g %g ) %s %d %d %d %f %f %d %d %d\r\n",
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

				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_plane++;
				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].plane = (brushplane_t *)
					realloc(radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].plane,
						radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_plane * sizeof(brushplane_t));

				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].plane[radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_plane - 1] = brushplane;


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
				state = P_BRUSH;

				radent[num_ent - 1].num_brush++;

				radent[num_ent - 1].brush = (brush_t *)realloc(radent[num_ent - 1].brush, radent[num_ent - 1].num_brush * sizeof(brush_t));


				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_plane = 0;
				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].plane = NULL;
				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_patch = 0;
				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].patch = NULL;
				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].brush_num = radent[num_ent - 1].num_brush;
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

				radent[num_ent - 1].brush = (brush_t *)realloc(radent[num_ent - 1].brush, radent[num_ent - 1].num_brush * sizeof(brush_t));


				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_plane = 0;
				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].plane = NULL;
				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_patch = 0;
				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].patch = NULL;
				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].brush_num = name.number;
			}
		}
		else
		{
			if (parse_name(line, &name) == 0)
			{
				indent(level, output);
				fprintf(output, "// %s %d\r\n", name.name, name.number);
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
					radent[num_ent - 1].brush = NULL;
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

///=============================================================================
/// Function: load_v2
///=============================================================================
/// Description: loads a .map file
///
/// This will load doom3 or quake4 .map files
///
///
/// Parameters:
///		map	- file to parse
///     output - file to output what was read 
///       (output should match input .map file when done)
///
/// Returns:
///		zero success, non-zero failure
///=============================================================================
int RadiantMap::load_v2(char *map, FILE *output)
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
	int brushdef = 0;


	radent = NULL;
	num_ent = 0;

	char line[MAX_LINE] = { 0 };
	while (fgets(line, MAX_LINE + 1, fp) != NULL)
	{
		brushplane_t brushplane = { 0 };
		brushpatch_t brushpatch = { 0 };
		keyval_t keyval = { 0 };
		brush_name_t name = { 0 };
		patch_control_t control = { 0 };
		patch_point_t point = { 0 };

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
				state = P_PRIMITIVE;
			}
			else if (parse_patch_control(line, &control) == 0)
			{
				indent(level, output);

				if (patch_type == 2)
				{
					fprintf(output, "( %d %d %d %d %d )\r\n",
						control.width,
						control.height,
						control.contents,
						control.flags,
						control.value
					);
				}
				else
				{
					fprintf(output, "( %d %d %d %d %d %d %d )\r\n",
						control.width,
						control.height,
						control.contents,
						control.flags,
						control.value,
						control.value,
						control.value
					);
				}

				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].patch[radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_patch - 1].control = control;
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

				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].patch[radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_patch - 1].num_point++;


				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].patch[radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_patch - 1].points = (patch_point_t *)
					realloc(radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].patch[radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_patch - 1].points,
						radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].patch[radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_patch - 1].num_point * sizeof(patch_point_t));

				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].patch[radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_patch - 1].points[radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].patch[radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_patch - 1].num_point - 1] = point;
			}
			else if (parse_texture(line, texture) == 0)
			{
				indent(level, output);
				fprintf(output, "%s\r\n", texture);



				sprintf(radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].patch[radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_patch - 1].name,
					"%s", texture);
			}



		}
		else if (state == P_BRUSH)
		{
			if (parse_plane(line, &brushplane) == 0)
			{
				indent(level, output);


				fprintf(output, "( %g %g %g %g ) ( ( %g %g %g ) ( %g %g %g ) ) \"%s\" %d %d %d\r\n",
					brushplane.p.x, brushplane.p.y, brushplane.p.z, brushplane.d,
					brushplane.xxscale, brushplane.xyscale, brushplane.xoffsetf,
					brushplane.yxscale, brushplane.yyscale, brushplane.yoffsetf,
					brushplane.name,
					brushplane.contents,
					brushplane.flags,
					brushplane.values
				);

				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_plane++;
				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].plane = (brushplane_t *)
					realloc(radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].plane,
						radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_plane * sizeof(brushplane_t));

				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].plane[radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_plane - 1] = brushplane;


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

				state = P_PRIMITIVE;
			}
		}
		else if (state == P_PRIMITIVE)
		{
			if (parse_patch2(line) == 0)
			{
				indent(level, output);

				fprintf(output, "patchDef2\r\n");
				state = P_PATCH;

				patch_type = 2;

				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_patch++;
				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].patch = (brushpatch_t *)
					realloc(radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].patch,
						radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_patch * sizeof(brushpatch_t));


				brushpatch.name[0] = '\0';
				brushpatch.num_point = 0;
				brushpatch.points = NULL;
				brushpatch.control.width = 0;
				brushpatch.control.height = 0;
				brushpatch.control.contents = 0;
				brushpatch.control.flags = 0;
				brushpatch.control.value = 0;

				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].patch[radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_patch - 1] = brushpatch;

			}
			else if (parse_patch3(line) == 0)
			{
				indent(level, output);

				fprintf(output, "patchDef3\r\n");
				state = P_PATCH;

				patch_type = 3;

				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_patch++;
				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].patch = (brushpatch_t *)
					realloc(radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].patch,
						radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_patch * sizeof(brushpatch_t));


				brushpatch.name[0] = '\0';
				brushpatch.num_point = 0;
				brushpatch.points = NULL;
				brushpatch.control.width = 0;
				brushpatch.control.height = 0;
				brushpatch.control.contents = 0;
				brushpatch.control.flags = 0;
				brushpatch.control.value = 0;

				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].patch[radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_patch - 1] = brushpatch;

			}
			else if (parse_brushdef(line) == 0)
			{
				fprintf(output, "brushDef3\r\n");
				state = P_BRUSH;

				radent[num_ent - 1].num_brush++;

				radent[num_ent - 1].brush = (brush_t *)realloc(radent[num_ent - 1].brush, radent[num_ent - 1].num_brush * sizeof(brush_t));


				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_plane = 0;
				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].plane = NULL;
				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_patch = 0;
				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].patch = NULL;
				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].brush_num = radent[num_ent - 1].num_brush;
				brushdef = 1;
			}
			else if (parse_name(line, &name) == 0)
			{
				indent(level, output);
				fprintf(output, "// %s %d\r\n", name.name, name.number);
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
				state = P_PRIMITIVE;


				radent[num_ent - 1].num_brush++;

				radent[num_ent - 1].brush = (brush_t *)realloc(radent[num_ent - 1].brush, radent[num_ent - 1].num_brush * sizeof(brush_t));


				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_plane = 0;
				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].plane = NULL;
				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_patch = 0;
				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].patch = NULL;
				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].brush_num = radent[num_ent - 1].num_brush;
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
			}
		}
		else
		{
			if (parse_name(line, &name) == 0)
			{
				indent(level, output);
				fprintf(output, "// %s %d\r\n", name.name, name.number);
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
					radent[num_ent - 1].brush = NULL;
					radent[num_ent - 1].keyval = NULL;
				}

			}
			else if (parse_right_brace(line) == 0)
			{
				indent(--level, output);
				fprintf(output, "}\r\n");
			}
			else if (parse_version(line) == 0)
			{
				if (map_type == 4)
				{
					fprintf(output, "Version 2\r\n");
				}
				else
				{
					fprintf(output, "Version 3\r\n");
				}
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

///=============================================================================
/// Function: load_v3
///=============================================================================
/// Description: loads a .map file
///
/// This will load rage .map files (note this parser is pretty bare bones)
///
///
/// Parameters:
///		map	- file to parse
///     output - file to output what was read 
///       (output should match input .map file when done)
///
/// Returns:
///		zero success, non-zero failure
///=============================================================================
int RadiantMap::load_v3(char *map, FILE *output)
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
	int brushdef = 0;


	radent = NULL;
	num_ent = 1;

	char line[MAX_LINE] = { 0 };

	state = P_PRIMITIVE;
	radent = (radent_t *)realloc(radent, num_ent * sizeof(radent_t));



	radent[num_ent - 1].num_brush = 0;
	radent[num_ent - 1].num_keyval = 0;
	radent[num_ent - 1].ent_number = 0;
	radent[num_ent - 1].brush = NULL;
	radent[num_ent - 1].keyval = NULL;

	while (fgets(line, MAX_LINE + 1, fp) != NULL)
	{
		brushplane_t brushplane = { 0 };
		brushpatch_t brushpatch = { 0 };
		keyval_t keyval = { 0 };
		brush_name_t name = { 0 };
		patch_control_t control = { 0 };
		patch_point_t point = { 0 };

		line_num++;


		trim_edges(line, strlen(line));


		// since rage maps changed the format so much, we are just going to look for BrushDef3

		// So just two states, brush, or not a brush
		// P_PRIMITIVE or P_BRUSH






		if (state == P_BRUSH)
		{
			if (parse_plane(line, &brushplane) == 0)
			{
				indent(level, output);


				fprintf(output, "( %g %g %g %g ) ( ( %g %g %g ) ( %g %g %g ) ) \"%s\" %d %d %d\r\n",
					brushplane.p.x, brushplane.p.y, brushplane.p.z, brushplane.d,
					brushplane.xxscale, brushplane.xyscale, brushplane.xoffsetf,
					brushplane.yxscale, brushplane.yyscale, brushplane.yoffsetf,
					brushplane.name,
					brushplane.contents,
					brushplane.flags,
					brushplane.values
				);

				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_plane++;
				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].plane = (brushplane_t *)
					realloc(radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].plane,
						radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_plane * sizeof(brushplane_t));

				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].plane[radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_plane - 1] = brushplane;


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

				state = P_PRIMITIVE;
			}
		}
		else if (state == P_PRIMITIVE)
		{
			if (parse_brushdef(line) == 0)
			{
				fprintf(output, "brushDef3\r\n");
				state = P_BRUSH;

				radent[num_ent - 1].num_brush++;

				radent[num_ent - 1].brush = (brush_t *)realloc(radent[num_ent - 1].brush, radent[num_ent - 1].num_brush * sizeof(brush_t));


				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_plane = 0;
				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].plane = NULL;
				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].num_patch = 0;
				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].patch = NULL;
				radent[num_ent - 1].brush[radent[num_ent - 1].num_brush - 1].brush_num = radent[num_ent - 1].num_brush;
				brushdef = 1;
			}
			else
			{
				fprintf(output, line);
			}
		}
	}

	fclose(fp);

	return 0;
}


///=============================================================================
/// Function: save
///=============================================================================
/// Description: saves a .map file
///
/// This will save a .map file from the binary radmap structure
///
///
/// Parameters:
///		map	- file name to save
///     output - file pointer
///
/// Returns:
///		zero success, non-zero failure
///=============================================================================
int RadiantMap::save(char *map, FILE *output)
{
	// save from binary representation


	for (unsigned int i = 0; i < num_ent; i++)
	{
		fprintf(output, "// %s %d\r\n", "entity", radent[i].ent_number);
		fprintf(output, "{\r\n");
		for (unsigned int j = 0; j < radent[i].num_keyval; j++)
		{
			fprintf(output, "\"%s\" \"%s\"\r\n", radent[i].keyval[j].key, radent[i].keyval[j].value);
		}


		// for each brush
		for (unsigned int j = 0; j < radent[i].num_brush; j++)
		{
			fprintf(output, "// brush %d\r\n", radent[i].brush[j].brush_num);
			fprintf(output, "{\r\n");


			// for each plane 
			for (unsigned int k = 0; k < radent[i].brush[j].num_plane; k++)
			{

				brushplane_t *brushplane = &radent[i].brush[j].plane[k];

				fprintf(output, "( %d %d %d ) ( %d %d %d ) ( %d %d %d ) %s %d %d %d %f %f %d %d %d\r\n",
					(int)brushplane->v1[0], (int)brushplane->v1[1], (int)brushplane->v1[2],
					(int)brushplane->v2[0], (int)brushplane->v2[1], (int)brushplane->v2[2],
					(int)brushplane->v3[0], (int)brushplane->v3[1], (int)brushplane->v3[2],
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
			for (unsigned int k = 0; k < radent[i].brush[j].num_patch; k++)
			{
				brushpatch_t *brushpatch = &radent[i].brush[j].patch[k];

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
				for (unsigned int l = 0; l < radent[i].brush[j].patch[k].num_point; l++)
				{
					print_patch_points(&radent[i].brush[j].patch[k].control, &radent[i].brush[j].patch[k].points[l], output);
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



///=============================================================================
/// Function: intersect_three_planes
///=============================================================================
/// Description: Finds the intersection of three planes
///
/// Assuming the three planes intersect, we return true and the point
///
///
/// Parameters:
///		p1, p2, p3 - the planes
///     point - the output point
///
/// Returns:
///		boolean true if a intersection was found, false if not
///=============================================================================
bool RadiantMap::intersect_three_planes(plane_t &p1, plane_t &p2, plane_t &p3, vec3 &point)
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


///=============================================================================
/// Function: DistPointPlane
///=============================================================================
/// Description: Finds the distance of a point to a plane
///
/// I used to inline this code, but it's nice to keep the sign consistent
///
///
/// Parameters:
///		q - the point to check
///     normal - the plane normal
///     d - the distance from the origin (plane equation d)
///
/// Returns:
///		float distance of point from plane, negative if on the backside
///=============================================================================
float RadiantMap::DistPointPlane(const vec3 &q, const vec3 &normal, const float d)
{
	return q * normal - d;
}

///=============================================================================
/// Function: intersect_two_points_plane2
///=============================================================================
/// Description: This intersects a plane with a line segment
///
///
///
/// Parameters:
///		plane - the plane
///		a - start of line segment
///		b - end of line segment
///     result - the point the line segment intersects the plane if it exists
///
/// Returns:
///		zero success, non-zerp failure
///=============================================================================
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


///=============================================================================
/// Function: intersect_two_points_plane
///=============================================================================
/// Description: This intersects a plane with a line segment
///
/// Same as above, just a different way of doing it, both work
///
///
/// Parameters:
///		plane - the plane
///		a - start of line segment
///		b - end of line segment
///     result - the point the line segment intersects the plane if it exists
///
/// Returns:
///		zero success, non-zerp failure
///=============================================================================
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





///=============================================================================
/// Function: intersect_two_points_plane
///=============================================================================
/// Description: finds the signed area of a triangle
///
/// Can be used to tell triangle winding if clock wise or counter clockwise
///
///
/// Parameters:
///		a - triangle point
///		b - triangle point
///		c - triangle point
///
/// Returns:
///      returns positive if CCW negative if CW winding
///=============================================================================
float RadiantMap::Signed2DTriArea(const vec3 &a, const vec3 &b, const vec3 &c)
{
	return (a.x - c.x) * (b.y - c.y) - (a.y - c.y) * (b.x - c.x);
}


///=============================================================================
/// Function: intersect_triangle_plane
///=============================================================================
/// Description: intersects a triangle with a plane
///
/// Used for clipping a mesh one triangle at a time
///
///
/// Parameters:
///		p - plane normal and d component
///		a - triangle point
///		b - triangle point
///		c - triangle point
///     result - The new triangle points, may result in two triangles, no triangles, or one triangle
///
/// Returns:
///      zero success, non-zero failure
///=============================================================================
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


///=============================================================================
/// Function: allocate_quads
///=============================================================================
/// Description: Allocates memory for quadbrush triangulation
///
/// After we read the map, we want to triangulate to quad brush
/// This function will allocate memory for each brush prior to doing so
///
///
/// Returns:
///      None
///=============================================================================
void RadiantMap::allocate_quads()
{
	quadent.num_brush = radent[0].num_brush;
	quadent.quadbrush = NULL;
	quadent.quadbrush = (quadbrush_t *)realloc(quadent.quadbrush, quadent.num_brush * sizeof(quadbrush_t));

	for (int i = 0; i < quadent.num_brush; i++)
	{
		quadent.quadbrush[i].num_quadplane = radent[0].brush[i].num_plane;
		quadent.quadbrush[i].quadplane = NULL;
		quadent.quadbrush[i].num_vert = 0;
		quadent.quadbrush[i].vert_array = NULL;
		quadent.quadbrush[i].num_index = 0;
		quadent.quadbrush[i].index_array = NULL;
		quadent.quadbrush[i].quadplane = (quadplane_t *)realloc(quadent.quadbrush[i].quadplane, quadent.quadbrush[i].num_quadplane * sizeof(quadplane_t));
		memset(quadent.quadbrush[i].quadplane, 0, quadent.quadbrush[i].num_quadplane * sizeof(quadplane_t));
	}
}

///=============================================================================
/// Function: generate_quads
///=============================================================================
/// Description:
///
/// So this will generate planes from the point definitions
/// it also generates a quad that represents the plane and expands it in size
///
///
/// Returns:
///      None
///=============================================================================
void RadiantMap::generate_quads()
{
	// Only generate quads for entity 0 (worldspawn)
	for (unsigned int i = 0; i < radent[0].num_brush; i++)
	{
		for (unsigned int j = 0; j < radent[0].brush[i].num_plane; j++)
		{
			if (map_type < 4)
			{
				const vec3 a((float)radent[0].brush[i].plane[j].v1[0], (float)radent[0].brush[i].plane[j].v1[1], (float)radent[0].brush[i].plane[j].v1[2]);
				const vec3 b((float)radent[0].brush[i].plane[j].v2[0], (float)radent[0].brush[i].plane[j].v2[1], (float)radent[0].brush[i].plane[j].v2[2]);
				const vec3 c((float)radent[0].brush[i].plane[j].v3[0], (float)radent[0].brush[i].plane[j].v3[1], (float)radent[0].brush[i].plane[j].v3[2]);


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
			else
			{
				quadent.quadbrush[i].quadplane[j].plane.normal = radent[0].brush[i].plane[j].p;
				quadent.quadbrush[i].quadplane[j].plane.d = radent[0].brush[i].plane[j].d;
			}

		}
	}
}


#if 0
///=============================================================================
/// Function: clip_quads
///=============================================================================
/// Description:
///
/// So this was intentended to be the approach where you clip a large box into
/// the final brush mesh, but the intersect method works pretty darn good I'm
/// not sure if I will come back and finish this approach
///
///
/// Returns:
///      None
///=============================================================================
void RadiantMap::clip_quads()
{
	for (unsigned int i = 0; i < radent[0].num_brush; i++)
	{
		for (unsigned int j = 0; j < radent[0].brush[i].num_plane; j++)
		{
			for (unsigned int l = 0; l < quadent.quadbrush[i].quadplane[j].num_triangle; l += 3)
			{
				vertex_t result[6];
				vertex_t a;
				vertex_t b;
				vertex_t c;

				a.position = quadent.quadbrush[i].quadplane[j].triangle_list[l];
				a.texCoord0.x = radent[0].brush[i].plane[j].xoffset / radent[0].brush[i].plane[j].xscale;
				a.texCoord0.y = radent[0].brush[i].plane[j].yoffset / radent[0].brush[i].plane[j].yscale;

				b.position = quadent.quadbrush[i].quadplane[j].triangle_list[l + 1];
				b.texCoord0.x = radent[0].brush[i].plane[j].xoffset / radent[0].brush[i].plane[j].xscale;
				b.texCoord0.y = radent[0].brush[i].plane[j].yoffset / radent[0].brush[i].plane[j].yscale;


				c.position = quadent.quadbrush[i].quadplane[j].triangle_list[l + 2];
				c.texCoord0.x = radent[0].brush[i].plane[j].xoffset / radent[0].brush[i].plane[j].xscale;
				c.texCoord0.y = radent[0].brush[i].plane[j].yoffset / radent[0].brush[i].plane[j].yscale;

				for (unsigned int k = 0; k < quadent.quadbrush[i].num_quadplane; k++)
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
#endif

///=============================================================================
/// Function: intersect_quads
///=============================================================================
/// Description:
///
/// So this will triangulate the brushes by intersecting all unique pairs of
/// three brush planes. It then triangulates them with Bowyer Watson triangulation
/// This works 100% perfect, the non-bowyer watson triangulation is faster, but not
/// correct for all cases (like 95% correct though)
///
///
/// Returns:
///      None
///=============================================================================
void RadiantMap::intersect_quads()
{
	unsigned int max_brush_planes = 0;
	unsigned int max_points_per_brush = 0;
	unsigned int max_points_per_plane = 0;
	unsigned int max_output = 0;
	unsigned int num_triangles = 0;

	for (unsigned int i = 0; i < radent[0].num_brush; i++)
	{
		static vec3 point_array[MAX_POINT_PER_PLANE];
		unsigned int num_point = 0;
		static vec3 triangle_array[MAX_POINT_PER_BRUSH];
		unsigned int num_brush_point = 0;
		static vec3 plane_face_array[MAX_BRUSH_PLANE][MAX_POINT_PER_PLANE];
		static vec2 plane_face_tex[MAX_BRUSH_PLANE][MAX_POINT_PER_PLANE];
		static vec2 tex_array[MAX_POINT_PER_BRUSH];
		unsigned int num_brush_tex = 0;
		unsigned int num_plane_face[MAX_BRUSH_PLANE] = { 0 };
		unsigned int arr[MAX_OUTPUT] = { 0 }; // max plane
		unsigned int output[MAX_OUTPUT] = { 0 };
		unsigned int num_output = 0;
		vec3 point(0.0f, 0.0f, 0.0f);



		if (radent[0].brush[i].num_plane > MAX_BRUSH_PLANE)
		{
			printf("Brush %d exceeded max planes %d\r\n", i, radent[0].brush[i].num_plane);
			max_brush_planes = MAX(max_brush_planes, radent[0].brush[i].num_plane);
			continue;
		}

		max_brush_planes = MAX(max_brush_planes, radent[0].brush[i].num_plane);



		bool skip = false;

		for (unsigned int x = 0; x < radent[0].brush[i].num_plane; x++)
		{
			// skip brushes we dont want to turn into geometry
			// things like clip brushes, triggers, etc
			if (strcmp(radent[0].brush[i].plane[x].name, "common/clip") == 0)
			{
				skip = true;
			}
			else if (strcmp(radent[0].brush[i].plane[x].name, "common/weapclip") == 0)
			{
				skip = true;
			}
			else if (strcmp(radent[0].brush[i].plane[x].name, "common/clusterportal") == 0)
			{
				skip = true;
			}
			else if (strcmp(radent[0].brush[i].plane[x].name, "common/cushion") == 0)
			{
				skip = true;
			}
			else if (strcmp(radent[0].brush[i].plane[x].name, "common/donotenter") == 0)
			{
				skip = true;
			}
			else if (strcmp(radent[0].brush[i].plane[x].name, "common/hint") == 0)
			{
				skip = true;
			}
			else if (strcmp(radent[0].brush[i].plane[x].name, "common/missleclip") == 0)
			{
				skip = true;
			}
			else if (strcmp(radent[0].brush[i].plane[x].name, "common/nodrawnonsolid") == 0)
			{
				skip = true;
			}
			else if (strcmp(radent[0].brush[i].plane[x].name, "common/nodrop") == 0)
			{
				skip = true;
			}
			else if (strcmp(radent[0].brush[i].plane[x].name, "common/origin") == 0)
			{
				skip = true;
			}
			else if (strcmp(radent[0].brush[i].plane[x].name, "common/portal") == 0)
			{
				skip = true;
			}
			else if (strcmp(radent[0].brush[i].plane[x].name, "common/trigger") == 0)
			{
				skip = true;
			}
			else if (strcmp(radent[0].brush[i].plane[x].name, "common/botclip") == 0)
			{
				skip = true;
			}
			else if (strcmp(radent[0].brush[i].plane[x].name, "common/weapclip") == 0)
			{
				skip = true;
			}

		}

		if (skip)
		{
			continue;
		}

		for (unsigned int x = 0; x < radent[0].brush[i].num_plane; x++)
		{
			arr[x] = x;
		}
		int n = radent[0].brush[i].num_plane;
		int r = 3;

		// nCr
		Combination(arr, n, r, output, num_output);


		max_output = MAX(max_output, num_output);


#ifdef DEBUG
		printf("Plane nCr combinations of 3 (%dC%d)\r\n", n, r);
		for (int x = 0; x < num_output; x += r)
		{
			printf("\t");
			for (int y = 0; y < r; y++)
			{
				printf("%C ", output[x + y] + 'A');
			}
			printf("\r\n");
		}
#endif

		for (unsigned int t = 0; t < num_output; t += 3)
		{
			unsigned int index1 = output[t + 0];
			unsigned int index2 = output[t + 1];
			unsigned int index3 = output[t + 2];

			if (index1 > num_output ||
				index2 > num_output ||
				index3 > num_output)
			{
				printf("index exceeded num_output\r\n");
				break;
			}

			// Unique Combinations nC3 of planes (ABCDEF for a box)
			// ABC ABD ABE ABF ACD ACE ACF ADE ADF AEF BCD BCE BCF BDE BDF BEF CDE CDF CEF DEF 
			if (intersect_three_planes(
				quadent.quadbrush[i].quadplane[index1].plane,
				quadent.quadbrush[i].quadplane[index2].plane,
				quadent.quadbrush[i].quadplane[index3].plane,
				point))
			{
				bool inside = true;
#ifdef DEBUG
				printf("\tintersection %C%C%C %f %f %f\r\n",
					index1 + 'A',
					index2 + 'A',
					index3 + 'A',
					point.x, point.y, point.z);
#endif

				// sometimes we can have planes intersect outside the volume of the brush (pentagon)
				// so test to be sure it's inside the volume
				for (unsigned int j = 0; j < radent[0].brush[i].num_plane; j++)
				{
					float dist = DistPointPlane(point, quadent.quadbrush[i].quadplane[j].plane.normal, quadent.quadbrush[i].quadplane[j].plane.d);

					if (dist < -0.01)
					{
#ifdef DEBUG
						printf("\Dropping point %f %f %f as not inside the volume from plane %d %d %d dist %f\r\n",
							point.x, point.y, point.z,
							index1,
							index2,
							index3,
							dist
						);
#endif
						inside = false;
						break;
					}
				}


				if (inside)
				{

					if (num_point < MAX_POINT_PER_PLANE)
					{
						point_array[num_point++] = point;
					}
					else
					{
						printf("Max points per plane exceeded on brush %d num_plane %d\r\n", i, radent[0].brush[i].num_plane);
						break;
					}

					// Add this vertex to each of the planes that contain it
					// each array of points will be the points on that plane
					plane_face_array[index1][num_plane_face[index1]] = point;
					plane_face_array[index2][num_plane_face[index2]] = point;
					plane_face_array[index3][num_plane_face[index3]] = point;

					num_plane_face[index1]++;
					num_plane_face[index2]++;
					num_plane_face[index3]++;
				}


			}
		}

#ifdef DEBUG
		// print out points for each plane for debugging
		for (int j = 0; j < radent[0].brush[i].num_plane; j++)
		{
			printf("plane %d brush %d:\r\n", j, i);
			for (int k = 0; k < num_plane_face[j]; k++)
			{
				printf("\t (%f, %f, %f)\r\n", plane_face_array[j][k].x, plane_face_array[j][k].y, plane_face_array[j][k].z);;
			}
		}
#endif

		// Seems we have duplicate points, so remove any that are beside each other
		for (unsigned int j = 0; j < radent[0].brush[i].num_plane; j++)
		{
#ifdef DEBUG
			printf("\r\nremove duplicates from plane %d:\r\n", j);
#endif
			for (unsigned int k = 0; k < num_plane_face[j]; k++)
			{
				for (unsigned int l = 0; l < num_plane_face[j]; l++)
				{
					if (k == l)
					{
						continue;
					}

					if (
						(fabs(plane_face_array[j][k].x - plane_face_array[j][l].x) < 0.001) &&
						(fabs(plane_face_array[j][k].y - plane_face_array[j][l].y) < 0.001) &&
						(fabs(plane_face_array[j][k].z - plane_face_array[j][l].z) < 0.001)
						)
					{
						//						printf("\t (%f, %f, %f) is duplicate\r\n", plane_face_array[j][k].x, plane_face_array[j][k].y, plane_face_array[j][k].z);;

						// replace point with last point in array
						if (l != num_plane_face[j] - 1)
						{
							plane_face_array[j][l] = plane_face_array[j][num_plane_face[j] - 1];
							num_plane_face[j]--;
						}
						else
						{
							// if it was the last one, just remove it
							num_plane_face[j]--;
						}
					}
				}
			}
		}


#ifdef DEBUG
		// print out points for each plane for debugging, no duplicates this time

		for (int j = 0; j < radent[0].brush[i].num_plane; j++)
		{
			if (num_plane_face[j] < 3)
			{
				printf("============= Plane with less than three points on brush %d ===============\r\n", i);

				printf("\r\nplane %d:\r\n", j);
				for (int k = 0; k < num_plane_face[j]; k++)
				{
					printf("\t (%f, %f, %f)\r\n", plane_face_array[j][k].x, plane_face_array[j][k].y, plane_face_array[j][k].z);
				}
			}
		}
#endif



		// We now have all the points, but no ordering, but we can match each point with it's corresponding plane
		// and the points in the plane make up a convex polygon
		// convex polygons can be rendered as triangle fans

		// Instead of testing each point with each plane, we just added them above based on which three planes intersected
		for (unsigned int j = 0; j < radent[0].brush[i].num_plane; j++)
		{
			unsigned int num_points_from_plane = 0;

			// So we can now convert point_array to a regular set of indexed triangles so we can render the whole thing at once
			// Technically you could merge duplicate vertices, but I dont think the benefits outweigh the complexity there

			//sort_point(&plane_face_array[j][0], num_plane_face[j], quadent.quadbrush[i].quadplane[j].plane.normal);



			vec3 x(1.0, 0.0, 0.0);
			vec3 y(0.0, 1.0, 0.0);
			vec3 z(0.0, 0.0, 1.0);

			// just set some texture coordinates along the plane
			for (int k = 0; k < num_plane_face[j]; k++)
			{
				float dotx = fabs(quadent.quadbrush[i].quadplane[j].plane.normal * x);
				float doty = fabs(quadent.quadbrush[i].quadplane[j].plane.normal * y);
				float dotz = fabs(quadent.quadbrush[i].quadplane[j].plane.normal * z);

				if ( dotx >= 0.98)
				{
					plane_face_tex[j][k].x = point.x / 128;
					plane_face_tex[j][k].y = point.y / 128;
				}

				if ( doty >= 0.98)
				{
					plane_face_tex[j][k].x = point.x / 128;
					plane_face_tex[j][k].y = point.y / 128;
				}

				if ( dotz >= 0.98)
				{
					plane_face_tex[j][k].x = point.x / 128;
					plane_face_tex[j][k].y = point.y / 128;
				}

				tex_array[num_brush_tex++] = plane_face_tex[j][k];

			}

			// BowyerWatson can be slow, so dont use it on basic stuff (using it on everything cause the first method just isn't always correct)
			if (num_plane_face[j] <= 3)
			{
				triangle_fan_to_array(&plane_face_array[j][0], num_plane_face[j], &triangle_array[num_brush_point], num_points_from_plane, quadent.quadbrush[i].quadplane[j].plane.normal);
			}
			else
			{
				triangulate.BowyerWatson(&plane_face_array[j][0], num_plane_face[j], &triangle_array[num_brush_point], num_points_from_plane);
				fix_winding(&triangle_array[num_brush_point], num_points_from_plane, quadent.quadbrush[i].quadplane[j].plane.normal);
			}


#ifdef DEBUG
			printf("\r\ntriangulated plane %d:\r\n", j);
			for (unsigned int k = 0; k < num_points_from_plane; k++)
			{
				printf("\tpoint %f %f %f\r\n", triangle_array[num_brush_point + k].x, triangle_array[num_brush_point + k].y, triangle_array[num_brush_point + k].z);
			}
#endif
			num_brush_point += num_points_from_plane;

			max_points_per_plane = MAX(max_points_per_plane, num_points_from_plane);

		}


		// Generate buffers 

		max_points_per_brush = MAX(max_points_per_brush, num_brush_point);

		num_triangles += max_points_per_brush / 3;

		quadent.quadbrush[i].num_vert = num_brush_point;
		quadent.quadbrush[i].vert_array = (vertex_t *)realloc(quadent.quadbrush[i].vert_array, (quadent.quadbrush[i].num_vert) * sizeof(vertex_t));

		for (unsigned int k = 0; k < quadent.quadbrush[i].num_vert; k++)
		{
			// swap from Z up to opengl Y up
			quadent.quadbrush[i].vert_array[k].position.x = triangle_array[k].x;
			quadent.quadbrush[i].vert_array[k].position.y = triangle_array[k].z;
			quadent.quadbrush[i].vert_array[k].position.z = -triangle_array[k].y;

			quadent.quadbrush[i].vert_array[k].texCoord0 = tex_array[k];
			quadent.quadbrush[i].vert_array[k].texCoord1 = tex_array[k];

		}

		quadent.quadbrush[i].num_index = num_brush_point;
		quadent.quadbrush[i].index_array = (unsigned int *)realloc(quadent.quadbrush[i].index_array, (quadent.quadbrush[i].num_index) * sizeof(int));

		for (unsigned int k = 0; k < quadent.quadbrush[i].num_index; k++)
		{
			quadent.quadbrush[i].index_array[k] = k;
		}


	}

	printf("Map had %d planes maximum on brushes\r\n", max_brush_planes);
	printf("Map had %d points maximum on brush plane\r\n", max_points_per_plane);
	printf("Map had %d points maximum on brushes\r\n", max_points_per_brush);
	printf("Map had %d triangles\r\n", num_triangles);
	printf("Map had %d maximum nCr outputs\r\n", max_output);
}


///=============================================================================
/// Function: combination_recurse
///=============================================================================
/// Description:
///
/// To generate the unique combinations of three planes we use nCr
/// this implements that using a recursive function, this being the recursive part
///
///
/// Returns:
///      None
///=============================================================================
void RadiantMap::combination_recurse(unsigned int arr[], unsigned int data[], unsigned int start, unsigned int end, unsigned int index, unsigned int r, unsigned int *output, unsigned int &num_out)
{
	if (index == r)
	{
		for (unsigned int j = 0; j < r; j++)
		{
			//            printf("%c ",  data[j] + 'A');
			output[num_out++] = data[j];
		}
		//      printf("\r\n");
		return;
	}

	for (unsigned int i = start; i <= end && end - i + 1 >= r - index; i++)
	{
		data[index] = arr[i];
		combination_recurse(arr, data, i + 1, end, index + 1, r, output, num_out);
	}
}

///=============================================================================
/// Function: Combination
///=============================================================================
/// Description:
///
/// To generate the unique combinations of three planes we use nCr
/// this implements that using a recursive function
///
///
/// Returns:
///      None
///=============================================================================
void RadiantMap::Combination(unsigned int *arr, unsigned int n, unsigned int r, unsigned int *output, unsigned int &num_out)
{
	unsigned int *data = (unsigned int *)malloc(r * sizeof(int));;

	combination_recurse(arr, data, 0, n - 1, 0, r, output, num_out);
	free((void *)data);
}


///=============================================================================
/// Function: triangle_fan_to_array
///=============================================================================
/// Description:
///
/// This will convert a triangle fan to an array
///
/// Note that this function was a bit modified to better work with the vertex
/// ordering we got from the intersection output. So change it back for a generic
/// conversion of fan to triangle array
///
///
///
/// Returns:
///      None
///=============================================================================
void RadiantMap::triangle_fan_to_array(vec3 *point_array, unsigned int num_point, vec3 *triangle_array, unsigned int &num_triangle, vec3 &normal)
{
	num_triangle = 0;

	vec3 center(0.0f, 0.0f, 0.0f);

	for (unsigned int i = 0; i < num_point; i++)
	{
		center += point_array[i];
	}
	center = center / (float)num_point;


	vec3 a = point_array[0];
	unsigned int i = 0;


	if (num_point == 0)
		return;

	i = 1;
	if (i == 1)
	{
		// First point is kind of bad, so we treat is specially
		int index1 = i % num_point;
		int index2 = (i + 1) % num_point;
		vec3 b = point_array[index1];
		vec3 c = point_array[num_point - 1];


		vec3 ba = b - a;
		vec3 ca = c - a;
		vec3 norm = vec3::crossproduct(ba, ca);

		// make sure all the triangle windings match the passed normal
		if (norm * normal > 0)
		{
			triangle_array[num_triangle++] = a;
			triangle_array[num_triangle++] = b;
			triangle_array[num_triangle++] = c;
		}
		else
		{
			triangle_array[num_triangle++] = c;
			triangle_array[num_triangle++] = b;
			triangle_array[num_triangle++] = a;
		}
		i++;
	}



	for (i = 2; i < num_point;)
	{
		// rest will be one new point and two previous

		int index1 = i % num_point;
		int index2 = (i + 1) % num_point;
		vec3 b = point_array[index1];
		vec3 c = point_array[index2];


		vec3 ba = b - a;
		vec3 ca = c - a;
		vec3 norm = vec3::crossproduct(ba, ca);

		// make sure all the triangle windings match the passed normal
		if (norm * normal > 0)
		{
			triangle_array[num_triangle++] = a;
			triangle_array[num_triangle++] = b;
			triangle_array[num_triangle++] = c;
		}
		else
		{
			triangle_array[num_triangle++] = c;
			triangle_array[num_triangle++] = b;
			triangle_array[num_triangle++] = a;
		}
		i++;
	}
}


///=============================================================================
/// Function: triangle_strip_to_array
///=============================================================================
/// Description:
///
/// This will convert a triangle strip to an array
///
/// A triangle strip will reuse the previous two verticies in creation of the next
/// vertex this undoes that and just generates 3 points defining a single triangle
///
///
///
/// Returns:
///      None
///=============================================================================
void RadiantMap::triangle_strip_to_array(vec3 *point_array, unsigned int num_point, vec3 *triangle_array, unsigned int &num_triangle, vec3 &normal)
{
	num_triangle = 0;


	for (unsigned int i = 0; i < num_point;)
	{
		if (i == 0)
		{
			// Triangle fan, first set will have 3 points
			vec3 a = point_array[i + 0];
			vec3 b = point_array[i + 1];
			vec3 c = point_array[i + 2];


			vec3 ba = b - a;
			vec3 ca = c - a;
			vec3 norm = vec3::crossproduct(ba, ca);

			// make sure all the triangle windings match the passed normal
			if (norm * normal > 0)
			{
				triangle_array[num_triangle++] = a;
				triangle_array[num_triangle++] = b;
				triangle_array[num_triangle++] = c;
			}
			else
			{
				triangle_array[num_triangle++] = c;
				triangle_array[num_triangle++] = b;
				triangle_array[num_triangle++] = a;
			}
			i += 3;
		}
		else
		{
			// rest will be one new point and two previous
			vec3 a = point_array[i - 2];
			vec3 b = point_array[i - 1];
			vec3 c = point_array[i];


			vec3 ba = b - a;
			vec3 ca = c - a;
			vec3 norm = vec3::crossproduct(ba, ca);

			// make sure all the triangle windings match the passed normal
			if (norm * normal > 0)
			{
				triangle_array[num_triangle++] = a;
				triangle_array[num_triangle++] = b;
				triangle_array[num_triangle++] = c;
			}
			else
			{
				triangle_array[num_triangle++] = c;
				triangle_array[num_triangle++] = b;
				triangle_array[num_triangle++] = a;
			}
			i++;
		}
	}
}


///=============================================================================
/// Function: fix_winding
///=============================================================================
/// Description:
///
/// This will ensure vertex windings match the passed normal
///
/// Done just in case, I'm not 100% sure if BowyerWatson's output tracks
/// the windings as nicely as it probably should
///
///
///
/// Returns:
///      None
///=============================================================================
void RadiantMap::fix_winding(vec3 *triangle_array, int num_triangle, vec3 &normal)
{
	for (int k = 0; k < num_triangle; k += 3)
	{
		vec3 a = triangle_array[k + 0];
		vec3 b = triangle_array[k + 1];
		vec3 c = triangle_array[k + 2];


		vec3 ba = b - a;
		vec3 ca = c - a;
		vec3 norm = vec3::crossproduct(ba, ca);

		// make sure all the triangle windings match the passed normal
		if (norm * normal > 0)
		{
			triangle_array[k + 0] = a;
			triangle_array[k + 1] = b;
			triangle_array[k + 2] = c;
		}
		else
		{
			triangle_array[k + 0] = c;
			triangle_array[k + 1] = b;
			triangle_array[k + 2] = a;
		}
	}
}
