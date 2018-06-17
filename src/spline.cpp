#include "spline.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Spline::Spline()
{
	num_control = 0;
}

void Spline::load(char *file)
{
	char *list[32];
	char *fp = NULL;
	unsigned int num_spline = 0;

	newlinelist(file, list, num_spline, &fp);

	unsigned int j = 0;
	for (int i = 0; i < num_spline; i++)
	{
		quaternion q;
		vec3 pos;

		if (7 != sscanf(list[i], "pos [%X, %X, %X] quat [%X, i %X, j %X, k %X]",
			(unsigned int *)&pos.x,
			(unsigned int *)&pos.y,
			(unsigned int *)&pos.z,
			(unsigned int *)&q.s,
			(unsigned int *)&q.x,
			(unsigned int *)&q.y,
			(unsigned int *)&q.z))
		{
			debugf("parse spline failed\r\n");
			break;
		}

		controlq[j] = q;
		control[j] = pos;
		j++;
	}
	num_control = j;

	delete[] fp;
}

void Spline::step(Frame &camera_frame, float t)
{
	vec3 pos = para_spline(control, num_control, t);
	quaternion result;

	int seg = t * (num_control - 1);
	float nt = (t * (num_control - 1)) - ((int)(t * (num_control - 1)) % (num_control - 1));
	quaternion::slerp(controlq[seg], controlq[seg + 1], nt, result);
	matrix3 orientation = result.to_matrix();

	camera_frame.up.x = orientation.m[3];
	camera_frame.up.y = orientation.m[4];
	camera_frame.up.z = orientation.m[5];
	camera_frame.forward.x = orientation.m[6];
	camera_frame.forward.y = orientation.m[7];
	camera_frame.forward.z = orientation.m[8];
	camera_frame.pos = pos;
}

void Spline::destroy()
{
	num_control = 0;
}

Spline::~Spline()
{
}
