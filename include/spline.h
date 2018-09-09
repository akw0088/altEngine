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

#include "include.h"

#ifndef SPLINE_H
#define SPLINE_H

class Spline
{
public:
	Spline();
	void load(char *file);
	void destroy();
	void step(Frame &camera_frame, float t);

	~Spline();

private:
	float HermiteInterpolate(float y0, float y1, float y2, float y3,
		float mu, float tension, float bias);
	vec3 HermiteInterp(vec3 &a, vec3 &b, vec3 &c, vec3 &d, float t);
	vec3 para_spline(vec3 *control, int num_control, float t);


	vec3 control[32];
	quaternion controlq[32];
	int num_control;
};

#endif
