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
	gfx.CreateFramebuffer((int)(FBO_RESOLUTION * scale), (int)(FBO_RESOLUTION * scale), fbo, quad_tex, depth_tex, normal_depth, 0, false);
}

void PortalCamera::destroy(Graphics &gfx)
{
	gfx.DeleteFrameBuffer(fbo, quad_tex, depth_tex);
}






