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
	vec3 control[32];
	quaternion controlq[32];
	int num_control;
};

#endif
