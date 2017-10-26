#include "portal_camera.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

PortalCamera::PortalCamera(Entity *entity, Graphics &gfx, float scale, float zNear, float zFar, bool inf)
{
	PortalCamera::entity = entity;
	unsigned int normal_depth = -1;
	quad_tex = -1;
	depth_tex = -1;
	fbo = -1;

	portal_projection.perspective(90.0, 1.0, zNear, zFar, inf);
	gfx.setupFramebuffer((int)(FBO_RESOLUTION * scale), (int)(FBO_RESOLUTION * scale), fbo, quad_tex, depth_tex, normal_depth, 0, false);
}

void PortalCamera::destroy(Graphics &gfx)
{
	gfx.DeleteFrameBuffer(fbo);
	gfx.DeleteTexture(quad_tex);
	gfx.DeleteTexture(depth_tex);
}






