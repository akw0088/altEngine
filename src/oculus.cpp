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

#include "oculus.h"

#ifdef OCULUS

Oculus::Oculus()
{
	initialized = false;
	frame_num = 0;
}

int Oculus::init(Graphics &gfx)
{
	ovrResult result = ovr_Initialize(NULL);
	ovrGraphicsLuid luid;

	if (result != ovrSuccess)
	{
		printf("Unable to initialize ovr\r\n");
		return -1;
	}

	result = ovr_Create(&session, &luid);
	if (result != ovrSuccess)
	{
		printf("Unable to create ovr\r\n");
		return -1;
	}

	hmd_desc = ovr_GetHmdDesc(session);

	printf("Product: %s\n", hmd_desc.ProductName);
	printf("Manufacturer: %s\n", hmd_desc.Manufacturer);
	printf("Vendor Id: %d\n", hmd_desc.VendorId);
	printf("Product Id: %d\n", hmd_desc.ProductId);
	printf("SerialNumber: %s\n", hmd_desc.SerialNumber);
	printf("FirmwareMajor: %d\n", hmd_desc.FirmwareMajor);
	printf("FirmwareMinor: %d\n", hmd_desc.FirmwareMinor);

	ovrSizei resolution = hmd_desc.Resolution;
	printf("Resolution: %dx%d\n", resolution.w, resolution.h);
	printf("DisplayRefreshRate: %f\n", hmd_desc.DisplayRefreshRate);


	buffer(gfx, 0);
	buffer(gfx, 1);

	memset(&mirror_desc, 0, sizeof(mirror_desc));
	mirror_desc.Width = gfx.width;
	mirror_desc.Height = gfx.height;
	mirror_desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;

	ovr_CreateMirrorTextureGL(session, &mirror_desc, &mirror_texture);

	// Configure the mirror read buffer
	GLuint texId;
	ovr_GetMirrorTextureBufferGL(session, mirror_texture, &texId);


	initialized = true;

	return 0;
}

void Oculus::get_pos(matrix3 &head, vec3 &head_pos,
	matrix3 &lefthand, vec3 &left_pos,
	matrix3 &righthand, vec3 &right_pos,
	matrix3 &righteye, vec3 &reye_pos,
	matrix3 &lefteye, vec3 &leye_pos,
	ovrtouch_t &input)
{
	quaternion q;
	ovrInputState    ovrinput;
	ovrResult ret;

	if (initialized == false)
		return;

	display_time = ovr_GetPredictedDisplayTime(session, frame_num);
	track = ovr_GetTrackingState(session, ovr_time, ovrTrue);

	head_pos.x = track.HeadPose.ThePose.Position.x;
	head_pos.y = track.HeadPose.ThePose.Position.y;
	head_pos.z = track.HeadPose.ThePose.Position.z;

	left_pos.x = track.HandPoses[0].ThePose.Position.x;
	left_pos.y = track.HandPoses[0].ThePose.Position.y;
	left_pos.z = track.HandPoses[0].ThePose.Position.z;

	right_pos.x = track.HandPoses[1].ThePose.Position.x;
	right_pos.y = track.HandPoses[1].ThePose.Position.y;
	right_pos.z = track.HandPoses[1].ThePose.Position.z;

	q.x = track.HeadPose.ThePose.Orientation.x;
	q.y = track.HeadPose.ThePose.Orientation.y;
	q.z = track.HeadPose.ThePose.Orientation.z;
	q.s = track.HeadPose.ThePose.Orientation.w;
	head = q.to_matrix();

	q.x = track.HandPoses[0].ThePose.Orientation.x;
	q.y = track.HandPoses[0].ThePose.Orientation.y;
	q.z = track.HandPoses[0].ThePose.Orientation.z;
	q.s = track.HandPoses[0].ThePose.Orientation.w;
	lefthand = q.to_matrix();

	q.x = track.HandPoses[1].ThePose.Orientation.x;
	q.y = track.HandPoses[1].ThePose.Orientation.y;
	q.z = track.HandPoses[1].ThePose.Orientation.z;
	q.s = track.HandPoses[1].ThePose.Orientation.w;
	righthand = q.to_matrix();

	if ( ovr_GetInputState(session, ovrControllerType_Touch, &ovrinput) >= 0)
	{
		if (ovrinput.ControllerType == ovrControllerType_Touch)
		{
			if (ovrinput.Buttons & ovrTouch_A)
				input.a = true;
			else
				input.a = false;

			if (ovrinput.Buttons & ovrTouch_B)
				input.b = true;
			else
				input.b = false;

			if (ovrinput.Buttons & ovrTouch_X)
				input.x = true;
			else
				input.x = false;

			if (ovrinput.Buttons & ovrTouch_Y)
				input.y = true;
			else
				input.y = false;

			if (ovrinput.Buttons & ovrTouch_LThumb)
				input.lthumb = true;
			else
				input.lthumb = false;

			if (ovrinput.Buttons & ovrTouch_RThumb)
				input.rthumb = true;
			else
				input.rthumb = false;

			if (ovrinput.Buttons & ovrTouch_LThumbRest)
				input.lthumb_rest = true;
			else
				input.lthumb_rest = false;

			if (ovrinput.Buttons & ovrTouch_RThumbRest)
				input.rthumb_rest = true;
			else
				input.rthumb_rest = false;

			if (ovrinput.Buttons & ovrTouch_RIndexTrigger)
				input.rindex_trigger = true;
			else
				input.rindex_trigger = false;

			if (ovrinput.Buttons & ovrTouch_LIndexTrigger)
				input.lindex_trigger = true;
			else
				input.lindex_trigger = false;

			if (ovrinput.Buttons & ovrTouch_RIndexPointing)
				input.rindex_pointing = true;
			else
				input.rindex_pointing = false;

			if (ovrinput.Buttons & ovrTouch_LIndexPointing)
				input.lindex_pointing = true;
			else
				input.lindex_pointing = false;

			if (ovrinput.Buttons & ovrTouch_RThumbUp)
				input.rthumb_up = true;
			else
				input.rthumb_up = false;

			if (ovrinput.Buttons & ovrTouch_LThumbUp)
				input.lthumb_up = true;
			else
				input.lthumb_up = false;


			input.lthumbstick.x = ovrinput.Thumbstick[ovrHand_Left].x;
			input.lthumbstick.y = ovrinput.Thumbstick[ovrHand_Left].y;
			input.rthumbstick.x = ovrinput.Thumbstick[ovrHand_Right].x;
			input.rthumbstick.y = ovrinput.Thumbstick[ovrHand_Right].y;

			input.ltrigger = ovrinput.HandTrigger[ovrHand_Left];
			input.rtrigger = ovrinput.HandTrigger[ovrHand_Right];
		}
	}


	

	/*
	track.CalibratedOrigin;
	track.HandPoses;
	track.HandStatusFlags;
	track.HeadPose;
	track.StatusFlags;
	*/



	// Initialize VR structures, filling out description.
	eyeRenderDesc[0] = ovr_GetRenderDesc(session, ovrEye_Left, hmd_desc.DefaultEyeFov[0]);
	eyeRenderDesc[1] = ovr_GetRenderDesc(session, ovrEye_Right, hmd_desc.DefaultEyeFov[1]);
	hmdToEyeViewPose[0] = eyeRenderDesc[0].HmdToEyePose;
	hmdToEyeViewPose[1] = eyeRenderDesc[1].HmdToEyePose;


	ovr_CalcEyePoses(track.HeadPose.ThePose, hmdToEyeViewPose, layer.RenderPose);

	for (int i = 0; i < ovrEye_Count; i++)
	{
		layer.Header.Type = ovrLayerType_EyeFov;
		layer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;
		layer.ColorTexture[i] = swap_chain[i];
		layer.Viewport[i] = OVR::Recti(0, 0, eye_size[i].w, eye_size[i].h);
		layer.Fov[i] = hmd_desc.DefaultEyeFov[i];
//		layer.RenderPose[i] = hmdToEyeViewPose[i];
		layer.SensorSampleTime = ovr_time;
	}

}




int Oculus::buffer(Graphics &gfx, int index)
{
	ovrTextureSwapChainDesc desc = {};
	int num_tex = 0;

	eye_size[index] = ovr_GetFovTextureSize(session, (ovrEyeType)index, hmd_desc.DefaultEyeFov[index], 1.0f);

	desc.Type = ovrTexture_2D;
	desc.ArraySize = 1;
	desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
	desc.Width = eye_size[index].w;
	desc.Height = eye_size[index].h;
	desc.MipLevels = 1;
	desc.SampleCount = 1;
	desc.StaticImage = ovrFalse;

	ovrResult result = ovr_CreateTextureSwapChainGL(session, &desc, &swap_chain[index]);
	if (result < 0)
	{
		printf("ovr_CreateTextureSwapChainGL failed\n");
		return -1;
	}

	ovr_GetTextureSwapChainLength(session, swap_chain[index], &num_tex);
	for (int i = 0; i < num_tex; i++)
	{
		ovr_GetTextureSwapChainBufferGL(session, swap_chain[index], i, &texId[index][i]);
		glBindTexture(GL_TEXTURE_2D, texId[index][i]);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	}

	unsigned int normal_depth = 0;
	gfx.CreateFramebuffer(1.5 * eye_size[index].w, 1.5 *eye_size[index].h, ovr_fbo[index], ovr_tex[index], ovr_depth[index], normal_depth, 0, false);
	return 0;
}

int Oculus::get_index(int eye)
{
	int index;
	ovr_GetTextureSwapChainCurrentIndex(session, swap_chain[eye], &index);
	return index;
}


int Oculus::get_tex(int eye)
{
	unsigned int index;
	int i = get_index(eye);

	ovr_GetTextureSwapChainBufferGL(session, swap_chain[eye], i, &index);
	return index;
}

void Oculus::start_frame()
{
}

void Oculus::end_frame()
{
//	ovr_EndFrame(session, frame_num, viewScaleDesc, layer_header, layer_count);
}

void Oculus::submit_frame()
{
	ovrViewScaleDesc viewScaleDesc;
	viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;
	viewScaleDesc.HmdToEyePose[0] = hmdToEyeViewPose[0];
	viewScaleDesc.HmdToEyePose[1] = hmdToEyeViewPose[1];

	// append all the layers to global list
	ovrLayerHeader* layerList = &layer.Header;



//	ovr_WaitToBeginFrame(session, frame_num);
//	ovr_BeginFrame(session, frame_num);
//	ovr_EndFrame(session, frame_num, &viewScaleDesc, &layerList, 1);

	ovrResult result = ovr_SubmitFrame(session, frame_num, NULL, &layerList, 1);
	frame_num++;


}


int Oculus::destroy()
{
	ovr_DestroyTextureSwapChain(session, swap_chain[0]);
	ovr_DestroyTextureSwapChain(session, swap_chain[1]);
	ovr_Destroy(session);
	ovr_Shutdown();
	return 0;
}


#endif