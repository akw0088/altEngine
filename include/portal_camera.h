#include "include.h"

#ifndef PORTAL_CAMERA_H
#define PORTAL_CAMERA_H

class PortalCamera
{
public:
	PortalCamera(Entity *entity, Graphics &gfx, float scale, float zNear, float zFar, bool inf);
	void destroy(Graphics &gfx);

	Entity	*entity;
	unsigned int fbo;
	unsigned int quad_tex;
	unsigned int depth_tex;
	matrix4 portal_projection;
};

#endif
