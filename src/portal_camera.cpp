#include "portal_camera.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

PortalCamera::PortalCamera(Entity *entity, Graphics &gfx)
{
	PortalCamera::entity = entity;
	quad_tex = -1;
	depth_tex = -1;
	fbo = -1;

	portal_projection.perspective(90.0, 1.0, 1.0, 2001.0, true);
#ifdef OPENGL
	gfx.setupFramebuffer((int)(1024), (int)(1024), fbo, quad_tex, depth_tex, 0);
#endif

}

void PortalCamera::destroy(Graphics &gfx)
{
	gfx.DeleteFrameBuffer(fbo);
	gfx.DeleteTexture(quad_tex);
	gfx.DeleteTexture(depth_tex);
}






