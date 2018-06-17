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
