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

#include "spline.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;

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
	for (unsigned int i = 0; i < num_spline; i++)
	{
		quaternion q;
		vec3 pos;

		if (7 != sscanf(list[i], "pos [%f, %f, %f] quat [%f, i %f, j %f, k %f]",
			&pos.x,
			&pos.y,
			&pos.z,
			&q.s,
			&q.x,
			&q.y,
			&q.z))
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

	int seg = (int)(t * (num_control - 1));
	float nt = (t * (num_control - 1)) - ((int)(t * (num_control - 1)) % (num_control - 1));
	quaternion::slerp(controlq[seg], controlq[seg + 1], nt, result);
	matrix3 orientation = result.to_matrix();

	if (isnan(orientation.m[0] * orientation.m[1] * orientation.m[2] *
		orientation.m[3] * orientation.m[4] * orientation.m[5] *
		orientation.m[6] * orientation.m[7] * orientation.m[8]) == false)
	{
		camera_frame.up.x = orientation.m[3];
		camera_frame.up.y = orientation.m[4];
		camera_frame.up.z = orientation.m[5];
		camera_frame.forward.x = orientation.m[6];
		camera_frame.forward.y = orientation.m[7];
		camera_frame.forward.z = orientation.m[8];
	}
	camera_frame.pos = pos;
}

float Spline::HermiteInterpolate(float y0, float y1, float y2, float y3,
	float mu, float tension, float bias)
{
	float m0, m1, mu2, mu3;
	float a0, a1, a2, a3;

	mu2 = mu * mu;
	mu3 = mu2 * mu;
	m0 = (y1 - y0)*(1 + bias)*(1 - tension) / 2;
	m0 += (y2 - y1)*(1 - bias)*(1 - tension) / 2;
	m1 = (y2 - y1)*(1 + bias)*(1 - tension) / 2;
	m1 += (y3 - y2)*(1 - bias)*(1 - tension) / 2;
	a0 = 2 * mu3 - 3 * mu2 + 1;
	a1 = mu3 - 2 * mu2 + mu;
	a2 = mu3 - mu2;
	a3 = -2 * mu3 + 3 * mu2;

	return(a0*y1 + a1*m0 + a2*m1 + a3*y2);
}


vec3 Spline::HermiteInterp(vec3 &a, vec3 &b, vec3 &c, vec3 &d, float t)
{
	vec3 result;

	result.x = HermiteInterpolate(a.x, b.x, c.x, d.x, t, 0.0f, 0.0f);
	result.y = HermiteInterpolate(a.y, b.y, c.y, d.y, t, 0.0f, 0.0f);
	result.z = HermiteInterpolate(a.z, b.z, c.z, d.z, t, 0.0f, 0.0f);
	return result;
}


vec3 Spline::para_spline(vec3 *control, int num_control, float t)
{
	int seg = (int)(t * (num_control - 1));

	t = clamp(t, 0.0f, 1.0f);
	float nt = (t * (num_control - 1)) - ((int)(t * (num_control - 1)) % (num_control - 1));


	if (seg == 0)
	{
		return HermiteInterp(control[0], control[0], control[1], control[2], nt);
	}
	else if (seg < num_control - 2)
	{
		return HermiteInterp(control[seg - 1], control[seg], control[seg + 1], control[seg + 2], nt);
	}
	else
	{
		return HermiteInterp(control[num_control - 3], control[num_control - 2], control[num_control - 1], control[num_control - 1], nt);
	}
}

void Spline::destroy()
{
	num_control = 0;
}

Spline::~Spline()
{
}
