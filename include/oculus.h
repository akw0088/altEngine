#include "include.h"
#include "Extras/OVR_Math.h"
#include "OVR_CAPI.h"
#include "OVR_CAPI_GL.h"

#ifndef OVR_H

class Oculus
{
public:
	int init(Graphics &gfx);
	void get_pos(matrix3 &head, vec3 &position, matrix3 &lefthand, vec3 &left_pos, matrix3 &righthand, vec3 &right_pos);
	void submit_frame();
	int destroy();


private:
	int buffer(Graphics &gfx, int index);

	ovrSession        session;
	ovrHmdDesc        hmd_desc;
	ovrEyeRenderDesc  render_desc[2];
	ovrPosef          render_pose[2];
	ovrVector3f       render_offset[2];

	ovrMirrorTexture		mirror;
	ovrMirrorTextureDesc	mirror_desc;
	ovrTextureSwapChain		swap_chain;

	ovrSizei   eye_size[2];
	ovrPosef	eye_pose[2];
	ovrTrackingState track;
	double display_time;

	unsigned int ovr_time;

	// OpenGL specific
	unsigned int ovr_fbo;
	unsigned int ovr_depth;
	unsigned int depth_tex;
	unsigned int eye_tex[2];
};

#endif
