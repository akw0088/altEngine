#include "oculus.h"

#ifdef OCULUS


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

	frame_num = 0;

	return 0;
}

void Oculus::get_pos(matrix3 &head, vec3 &head_pos, matrix3 &lefthand, vec3 &left_pos, matrix3 &righthand, vec3 &right_pos, ovrtouch_t &input)
{
	quaternion q;
	ovrInputState    ovrinput;
	ovrResult ret;

	display_time = ovr_GetPredictedDisplayTime(session, 0);
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
	ovrEyeRenderDesc eyeRenderDesc[2];
	ovrPosef      hmdToEyeViewPose[2];
	ovrHmdDesc hmdDesc = ovr_GetHmdDesc(session);
	eyeRenderDesc[0] = ovr_GetRenderDesc(session, ovrEye_Left, hmdDesc.DefaultEyeFov[0]);
	eyeRenderDesc[1] = ovr_GetRenderDesc(session, ovrEye_Right, hmdDesc.DefaultEyeFov[1]);
	hmdToEyeViewPose[0] = eyeRenderDesc[0].HmdToEyePose;
	hmdToEyeViewPose[1] = eyeRenderDesc[1].HmdToEyePose;

	// Initialize our single full screen Fov layer.
	ovrLayerEyeFov layer;
	ovrRecti left;
	ovrRecti right;

	left.Pos.x = 0;
	left.Pos.y = 0;
	left.Size.w = eye_size[0].w / 2;
	left.Size.h = eye_size[0].h;

	right.Pos.x = eye_size[1].w / 2;
	right.Pos.y = 0;
	right.Size.w = eye_size[1].w / 2;
	right.Size.h = eye_size[1].h;

	layer.Header.Type = ovrLayerType_EyeFov;
	layer.Header.Flags = 0;
	layer.ColorTexture[0] = swap_chain;
	layer.ColorTexture[1] = swap_chain;
	layer.Fov[0] = eyeRenderDesc[0].Fov;
	layer.Fov[1] = eyeRenderDesc[1].Fov;
	layer.Viewport[0] = left;
	layer.Viewport[1] = right;

	ovr_CalcEyePoses(track.HeadPose.ThePose, hmdToEyeViewPose, layer.RenderPose);
}



int Oculus::buffer(Graphics &gfx, int index)
{
	ovrTextureSwapChainDesc desc = {};
	int num_tex = 0;

	eye_size[index] = ovr_GetFovTextureSize(session, (ovrEyeType)0, hmd_desc.DefaultEyeFov[index], 1.0f);

	desc.Type = ovrTexture_2D;
	desc.ArraySize = 1;
	desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
	desc.Width = eye_size[index].w;
	desc.Height = eye_size[index].h;
	desc.MipLevels = 1;
	desc.SampleCount = 1;
	desc.StaticImage = ovrFalse;

	ovrResult result = ovr_CreateTextureSwapChainGL(session, &desc, &swap_chain);
	if (result < 0)
	{
		printf("ovr_CreateTextureSwapChainGL failed\n");
		return -1;
	}

	ovr_GetTextureSwapChainLength(session, swap_chain, &num_tex);
	for (int i = 0; i < num_tex; i++)
	{
		unsigned int texId;
		ovr_GetTextureSwapChainBufferGL(session, swap_chain, i, &texId);
		glBindTexture(GL_TEXTURE_2D, texId);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	}

	glGenFramebuffers(1, &ovr_fbo);
	glGenTextures(1, &ovr_depth);
	glBindTexture(GL_TEXTURE_2D, depth_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, eye_size[index].w, eye_size[index].h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);

	return 0;
}

void Oculus::start_frame()
{
	ovr_WaitToBeginFrame(session, frame_num);
	ovr_BeginFrame(session, frame_num);


}

void Oculus::end_frame()
{
	//ovr_EndFrame(session, frame_num, viewScaleDesc, layer_header, layer_count);
}

void Oculus::submit_frame()
{
	ovrViewScaleDesc viewScaleDesc;
	viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;

	ovrLayerEyeFov eyeLayer;
	eyeLayer.Header.Type = ovrLayerType_EyeFov;
	eyeLayer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;

	for (int i = 0; i < ovrEye_Count; i++)
	{
		eyeLayer.ColorTexture[i] = swap_chain;
		eyeLayer.Viewport[i] = OVR::Recti(0, 0, eye_size[i].w, eye_size[i].h);
		eyeLayer.Fov[i] = hmd_desc.DefaultEyeFov[i];
		eyeLayer.RenderPose[i] = eye_pose[i];
		eyeLayer.SensorSampleTime = ovr_time;
	}

	// append all the layers to global list
	ovrLayerHeader* layerList = &eyeLayer.Header;

	ovrResult result = ovr_SubmitFrame(session, ovr_time, NULL, &layerList, 1);
	frame_num++;
}


int Oculus::destroy()
{
	ovr_DestroyTextureSwapChain(session, swap_chain);
	ovr_Destroy(session);
	ovr_Shutdown();
	return 0;
}


#endif