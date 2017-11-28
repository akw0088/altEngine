#ifdef OCULUS
#include "include.h"
#include "Extras/OVR_Math.h"
#include "OVR_CAPI.h"
#include "OVR_CAPI_GL.h"

#ifndef OCULUS_H
#define OCULUS_H

class Oculus
{
public:
	Oculus();
	int init(Graphics &gfx);
	void get_pos(matrix3 &head, vec3 &position, matrix3 &lefthand, vec3 &left_pos, matrix3 &righthand, vec3 &right_pos, ovrtouch_t &input);
	void start_frame();
	void end_frame();
	void submit_frame();
	int destroy();
	int get_index(int eye);
	int get_tex(int eye);

	// OpenGL specific
	unsigned int ovr_fbo[2];
	unsigned int ovr_tex[2];
	unsigned int ovr_depth[2];
//	unsigned int eye_tex[2];
	unsigned int texId[2][3];
	ovrSizei   eye_size[2];

//private:
	int buffer(Graphics &gfx, int index);

	ovrSession        session;
	ovrHmdDesc        hmd_desc;
//	ovrEyeRenderDesc  render_desc[2];
//	ovrPosef          render_pose[2];
	ovrVector3f       render_offset[2];

	ovrEyeRenderDesc eyeRenderDesc[2];
	ovrPosef      hmdToEyeViewPose[2];


	ovrMirrorTexture		mirror;
	ovrMirrorTextureDesc	mirror_desc;
	ovrTextureSwapChain		swap_chain[2];

	ovrPosef	eye_pose[2];
	ovrTrackingState track;
	double display_time;
	ovrLayerEyeFov layer;


	unsigned int ovr_time;

	unsigned int frame_num;
	bool initialized;


};

#endif
#endif