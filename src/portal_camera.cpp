#include "portal_camera.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

PortalCamera::PortalCamera(Entity *entity, Graphics &gfx, float scale)
{
	PortalCamera::entity = entity;
	unsigned int normal_depth = -1;
	quad_tex = -1;
	depth_tex = -1;
	fbo = -1;

	portal_projection.perspective(90.0, 1.0, 1.0, 2001.0, true);
	gfx.setupFramebuffer((int)(1024 * scale), (int)(1024 * scale), fbo, quad_tex, depth_tex, normal_depth, 0, false);
}

void PortalCamera::destroy(Graphics &gfx)
{
	gfx.DeleteFrameBuffer(fbo);
	gfx.DeleteTexture(quad_tex);
	gfx.DeleteTexture(depth_tex);
}






