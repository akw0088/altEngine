#include "oculus.h"



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

	buffer(gfx, 0);
	buffer(gfx, 1);

	return 0;
}

void Oculus::get_pos(matrix3 &head, vec3 &head_pos, matrix3 &lefthand, vec3 &left_pos, matrix3 &righthand, vec3 &right_pos)
{
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


	//track.CalibratedOrigin.Orientation.x -- quaternion to matrix (matrix conversion on my other computer, tbd)

	/*
	track.CalibratedOrigin;
	track.HandPoses;
	track.HandStatusFlags;
	track.HeadPose;
	track.StatusFlags;
	*/
}



int Oculus::buffer(Graphics &gfx, int index)
{
	ovrTextureSwapChainDesc desc;
	int num_tex = 0;

	eye_size[index] = ovr_GetFovTextureSize(session, (ovrEyeType)0, hmd_desc.DefaultEyeFov[index], 1.0f);

	memset(&desc, 0, sizeof(desc));
	desc.Type = ovrTexture_2D;
	desc.ArraySize = 1;
	desc.Width = eye_size[index].w;
	desc.Height = eye_size[index].h;
	desc.MipLevels = 1;
	desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
	desc.SampleCount = 1;
	desc.StaticImage = ovrFalse;

	ovrResult result = ovr_CreateTextureSwapChainGL(session, &desc, &swap_chain);

	ovr_GetTextureSwapChainLength(session, swap_chain, &num_tex);
	for (int i = 0; i < num_tex; i++)
	{
		unsigned int chain_tex_id;
		ovr_GetTextureSwapChainBufferGL(session, swap_chain, i, &chain_tex_id);
		glBindTexture(GL_TEXTURE_2D, chain_tex_id);

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
}


int Oculus::destroy()
{
	ovr_DestroyTextureSwapChain(session, swap_chain);
	ovr_Destroy(session);
	ovr_Shutdown();
	return 0;
}


