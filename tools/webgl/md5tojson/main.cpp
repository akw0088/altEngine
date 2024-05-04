#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <climits>
#include "vector.h"
#include "matrix.h"
#include "quaternion.h"
#include "md5.h"
#include "md5model.h"

#define M_PI 3.14159265359

void rotate_vector(float rad, vec3 &vec, vec3 &axis)
{
	matrix3	m;
	float	sin_val, cos_val, minus_val;

	sin_val = (float)sin(rad);
	cos_val = (float)cos(rad);
	minus_val = 1.0f - cos_val;

	m.m[0] = cos_val + minus_val * axis.x * axis.x;
	m.m[1] = minus_val * axis.x * axis.y - sin_val * axis.z;
	m.m[2] = minus_val * axis.x * axis.z + sin_val * axis.y;

	m.m[3] = minus_val * axis.y * axis.x + sin_val * axis.z;
	m.m[4] = cos_val + minus_val * axis.y * axis.y;
	m.m[5] = minus_val * axis.y * axis.z - sin_val * axis.x;

	m.m[6] = minus_val * axis.z * axis.x - sin_val * axis.y;
	m.m[7] = minus_val * axis.z * axis.y + sin_val * axis.x;
	m.m[8] = cos_val + minus_val * axis.z * axis.z;

	vec = m * vec;
}

void frame_limit(animation_state_t &ani_state, int start, int length, int end_start, int end_length)
{
	ani_state.fstart = start;
	ani_state.flength = length;


	ani_state.animation_frame = ani_state.fstart;
	ani_state.done = false;


	if (end_start != -1 && end_length != -1)
	{
		ani_state.fstart_end = end_start;
		ani_state.flength_end = end_length;
	}
	else
	{
		ani_state.fstart_end = 0;
		ani_state.flength_end = INT_MAX;
	}
}


int main(int argc, char *argv[])
{

	if (argc < 3)
	{
		printf("Usage: md5_to_json file.md5mesh file.md5anim\r\n");
		return 0;
	}

	char *anim[] = {
		argv[2]
	};

	static MD5Model md5model;
	animation_state_t state;

	md5model.load(argv[1], anim, 1, 0);

	return 0;
}
