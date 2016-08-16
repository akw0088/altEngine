#include "include.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/*
	Sound occulsion filters out high frequency sounds based on angle of indirect path
between src and listener. We must calculate this angle using pathfinding and set filter
accordingly for each source. Apply generic low pass filter when no line of sight exists.
Environmental audio just adds "effects" based on location. Requires defining bsp leafs with
properties and applying settings. There is also some crazy angle based method on applying
directionality between environments that require determining the start / end angles of portals
between environments.

see Reverb and Reflection Panning Algorithm.
(0,0,1) - straight ahead vector
two settings, reverberations and reflections
depend on "pan vector" magnitude of zero = 360 degrees, magnitude of 1 equals highly directional
two dimensional orientation with (0,0,1) representing forward. angle represents direction
magnitude represents size of envelope
eg: (0,0,0.75) = sound coming from infront of you with 90 degree width 
*/

/*
	deviceList = alcGetString(NULL, ALC_DEVICE_SPECIFIER);
	while (1)
	{
		printf("%s\n", deviceList);
		deviceList += strlen(deviceList) + 1;
		if (*deviceList == NULL)
			break;
	}
*/

char *GetALErrorString(ALenum err)
{
    switch(err)
    {
        case AL_NO_ERROR:
            return "AL_NO_ERROR";

        case AL_INVALID_NAME:
            return "AL_INVALID_NAME";

        case AL_INVALID_ENUM:
            return "AL_INVALID_ENUM";

        case AL_INVALID_VALUE:
            return "AL_INVALID_VALUE";

        case AL_INVALID_OPERATION:
            return "AL_INVALID_OPERATION";

        case AL_OUT_OF_MEMORY:
            return "AL_OUT_OF_MEMORY";
    };
	return "?";
}


// Imported EFX functions
#ifdef EFXUTIL
// Effect objects
LPALGENEFFECTS alGenEffects = NULL;
LPALDELETEEFFECTS alDeleteEffects = NULL;
LPALISEFFECT alIsEffect = NULL;
LPALEFFECTI alEffecti = NULL;
LPALEFFECTIV alEffectiv = NULL;
LPALEFFECTF alEffectf = NULL;
LPALEFFECTFV alEffectfv = NULL;
LPALGETEFFECTI alGetEffecti = NULL;
LPALGETEFFECTIV alGetEffectiv = NULL;
LPALGETEFFECTF alGetEffectf = NULL;
LPALGETEFFECTFV alGetEffectfv = NULL;

//Filter objects
LPALGENFILTERS alGenFilters = NULL;
LPALDELETEFILTERS alDeleteFilters = NULL;
LPALISFILTER alIsFilter = NULL;
LPALFILTERI alFilteri = NULL;
LPALFILTERIV alFilteriv = NULL;
LPALFILTERF alFilterf = NULL;
LPALFILTERFV alFilterfv = NULL;
LPALGETFILTERI alGetFilteri = NULL;
LPALGETFILTERIV alGetFilteriv = NULL;
LPALGETFILTERF alGetFilterf = NULL;
LPALGETFILTERFV alGetFilterfv = NULL;

// Auxiliary slot object
LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots = NULL;
LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots = NULL;
LPALISAUXILIARYEFFECTSLOT alIsAuxiliaryEffectSlot = NULL;
LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti = NULL;
LPALAUXILIARYEFFECTSLOTIV alAuxiliaryEffectSlotiv = NULL;
LPALAUXILIARYEFFECTSLOTF alAuxiliaryEffectSlotf = NULL;
LPALAUXILIARYEFFECTSLOTFV alAuxiliaryEffectSlotfv = NULL;
LPALGETAUXILIARYEFFECTSLOTI alGetAuxiliaryEffectSloti = NULL;
LPALGETAUXILIARYEFFECTSLOTIV alGetAuxiliaryEffectSlotiv = NULL;
LPALGETAUXILIARYEFFECTSLOTF alGetAuxiliaryEffectSlotf = NULL;
LPALGETAUXILIARYEFFECTSLOTFV alGetAuxiliaryEffectSlotfv = NULL;

EAXREVERBPROPERTIES eaxBathroom = REVERB_PRESET_BATHROOM;
EAXREVERBPROPERTIES eaxHangar = REVERB_PRESET_HANGAR;

bool ALFWIsEFXSupported()
{
	ALCdevice *pDevice = NULL;
	ALCcontext *pContext = NULL;

	pContext = alcGetCurrentContext();
	pDevice = alcGetContextsDevice(pContext);

	if (alcIsExtensionPresent(pDevice, (ALCchar*)ALC_EXT_EFX_NAME))
	{
		// Get function pointers
		alGenEffects = (LPALGENEFFECTS)alGetProcAddress("alGenEffects");
		alDeleteEffects = (LPALDELETEEFFECTS )alGetProcAddress("alDeleteEffects");
		alIsEffect = (LPALISEFFECT )alGetProcAddress("alIsEffect");
		alEffecti = (LPALEFFECTI)alGetProcAddress("alEffecti");
		alEffectiv = (LPALEFFECTIV)alGetProcAddress("alEffectiv");
		alEffectf = (LPALEFFECTF)alGetProcAddress("alEffectf");
		alEffectfv = (LPALEFFECTFV)alGetProcAddress("alEffectfv");
		alGetEffecti = (LPALGETEFFECTI)alGetProcAddress("alGetEffecti");
		alGetEffectiv = (LPALGETEFFECTIV)alGetProcAddress("alGetEffectiv");
		alGetEffectf = (LPALGETEFFECTF)alGetProcAddress("alGetEffectf");
		alGetEffectfv = (LPALGETEFFECTFV)alGetProcAddress("alGetEffectfv");
		alGenFilters = (LPALGENFILTERS)alGetProcAddress("alGenFilters");
		alDeleteFilters = (LPALDELETEFILTERS)alGetProcAddress("alDeleteFilters");
		alIsFilter = (LPALISFILTER)alGetProcAddress("alIsFilter");
		alFilteri = (LPALFILTERI)alGetProcAddress("alFilteri");
		alFilteriv = (LPALFILTERIV)alGetProcAddress("alFilteriv");
		alFilterf = (LPALFILTERF)alGetProcAddress("alFilterf");
		alFilterfv = (LPALFILTERFV)alGetProcAddress("alFilterfv");
		alGetFilteri = (LPALGETFILTERI )alGetProcAddress("alGetFilteri");
		alGetFilteriv= (LPALGETFILTERIV )alGetProcAddress("alGetFilteriv");
		alGetFilterf = (LPALGETFILTERF )alGetProcAddress("alGetFilterf");
		alGetFilterfv= (LPALGETFILTERFV )alGetProcAddress("alGetFilterfv");
		alGenAuxiliaryEffectSlots = (LPALGENAUXILIARYEFFECTSLOTS)alGetProcAddress("alGenAuxiliaryEffectSlots");
		alDeleteAuxiliaryEffectSlots = (LPALDELETEAUXILIARYEFFECTSLOTS)alGetProcAddress("alDeleteAuxiliaryEffectSlots");
		alIsAuxiliaryEffectSlot = (LPALISAUXILIARYEFFECTSLOT)alGetProcAddress("alIsAuxiliaryEffectSlot");
		alAuxiliaryEffectSloti = (LPALAUXILIARYEFFECTSLOTI)alGetProcAddress("alAuxiliaryEffectSloti");
		alAuxiliaryEffectSlotiv = (LPALAUXILIARYEFFECTSLOTIV)alGetProcAddress("alAuxiliaryEffectSlotiv");
		alAuxiliaryEffectSlotf = (LPALAUXILIARYEFFECTSLOTF)alGetProcAddress("alAuxiliaryEffectSlotf");
		alAuxiliaryEffectSlotfv = (LPALAUXILIARYEFFECTSLOTFV)alGetProcAddress("alAuxiliaryEffectSlotfv");
		alGetAuxiliaryEffectSloti = (LPALGETAUXILIARYEFFECTSLOTI)alGetProcAddress("alGetAuxiliaryEffectSloti");
		alGetAuxiliaryEffectSlotiv = (LPALGETAUXILIARYEFFECTSLOTIV)alGetProcAddress("alGetAuxiliaryEffectSlotiv");
		alGetAuxiliaryEffectSlotf = (LPALGETAUXILIARYEFFECTSLOTF)alGetProcAddress("alGetAuxiliaryEffectSlotf");
		alGetAuxiliaryEffectSlotfv = (LPALGETAUXILIARYEFFECTSLOTFV)alGetProcAddress("alGetAuxiliaryEffectSlotfv");

		if (alGenEffects &&	alDeleteEffects && alIsEffect && alEffecti && alEffectiv &&	alEffectf &&
			alEffectfv && alGetEffecti && alGetEffectiv && alGetEffectf && alGetEffectfv &&	alGenFilters &&
			alDeleteFilters && alIsFilter && alFilteri && alFilteriv &&	alFilterf && alFilterfv &&
			alGetFilteri &&	alGetFilteriv && alGetFilterf && alGetFilterfv && alGenAuxiliaryEffectSlots &&
			alDeleteAuxiliaryEffectSlots &&	alIsAuxiliaryEffectSlot && alAuxiliaryEffectSloti &&
			alAuxiliaryEffectSlotiv && alAuxiliaryEffectSlotf && alAuxiliaryEffectSlotfv &&
			alGetAuxiliaryEffectSloti && alGetAuxiliaryEffectSlotiv && alGetAuxiliaryEffectSlotf &&
			alGetAuxiliaryEffectSlotfv)
			return true;
	}

	return false;
}


ALboolean SetEFXEAXReverbProperties(EFXEAXREVERBPROPERTIES *pEFXEAXReverb, ALuint uiEffect)
{
	ALenum		al_err;
	ALboolean bReturn = AL_FALSE;

	if (pEFXEAXReverb)
	{
		// Clear AL Error code
		alGetError();

		alEffectf(uiEffect, AL_EAXREVERB_DENSITY, pEFXEAXReverb->flDensity);
		alEffectf(uiEffect, AL_EAXREVERB_DIFFUSION, pEFXEAXReverb->flDiffusion);
		alEffectf(uiEffect, AL_EAXREVERB_GAIN, pEFXEAXReverb->flGain);
		alEffectf(uiEffect, AL_EAXREVERB_GAINHF, pEFXEAXReverb->flGainHF);
		alEffectf(uiEffect, AL_EAXREVERB_GAINLF, pEFXEAXReverb->flGainLF);
		alEffectf(uiEffect, AL_EAXREVERB_DECAY_TIME, pEFXEAXReverb->flDecayTime);
		alEffectf(uiEffect, AL_EAXREVERB_DECAY_HFRATIO, pEFXEAXReverb->flDecayHFRatio);
		alEffectf(uiEffect, AL_EAXREVERB_DECAY_LFRATIO, pEFXEAXReverb->flDecayLFRatio);
		alEffectf(uiEffect, AL_EAXREVERB_REFLECTIONS_GAIN, pEFXEAXReverb->flReflectionsGain);
		alEffectf(uiEffect, AL_EAXREVERB_REFLECTIONS_DELAY, pEFXEAXReverb->flReflectionsDelay);
		alEffectfv(uiEffect, AL_EAXREVERB_REFLECTIONS_PAN, pEFXEAXReverb->flReflectionsPan);
		alEffectf(uiEffect, AL_EAXREVERB_LATE_REVERB_GAIN, pEFXEAXReverb->flLateReverbGain);
		alEffectf(uiEffect, AL_EAXREVERB_LATE_REVERB_DELAY, pEFXEAXReverb->flLateReverbDelay);
		alEffectfv(uiEffect, AL_EAXREVERB_LATE_REVERB_PAN, pEFXEAXReverb->flLateReverbPan);
		alEffectf(uiEffect, AL_EAXREVERB_ECHO_TIME, pEFXEAXReverb->flEchoTime);
		alEffectf(uiEffect, AL_EAXREVERB_ECHO_DEPTH, pEFXEAXReverb->flEchoDepth);
		alEffectf(uiEffect, AL_EAXREVERB_MODULATION_TIME, pEFXEAXReverb->flModulationTime);
		alEffectf(uiEffect, AL_EAXREVERB_MODULATION_DEPTH, pEFXEAXReverb->flModulationDepth);
		alEffectf(uiEffect, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, pEFXEAXReverb->flAirAbsorptionGainHF);
		alEffectf(uiEffect, AL_EAXREVERB_HFREFERENCE, pEFXEAXReverb->flHFReference);
		alEffectf(uiEffect, AL_EAXREVERB_LFREFERENCE, pEFXEAXReverb->flLFReference);
		alEffectf(uiEffect, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR, pEFXEAXReverb->flRoomRolloffFactor);
		alEffecti(uiEffect, AL_EAXREVERB_DECAY_HFLIMIT, pEFXEAXReverb->iDecayHFLimit);

		al_err = alGetError();
		if (al_err != AL_NO_ERROR)
		{
			printf("Unable to set effect: %s", GetALErrorString(al_err));
		}
		return 1;
	}

	return bReturn;
}
#else
#endif

void Audio::init()
{
#ifndef __linux__
	EFXEAXREVERBPROPERTIES efxReverb;
	int attrib[] = {ALC_MAX_AUXILIARY_SENDS, 4};
#endif
	int sends;
	ALenum al_err;

	printf("Using default audio device: %s\n", alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER));
	device = alcOpenDevice(NULL);
	if (device == NULL)
	{
		printf("No sound device/driver has been found.\n");
		return;
	}

#ifdef __linux__
	context = alcCreateContext(device, NULL);
#else
	context = alcCreateContext(device, attrib);
#endif
	if (context == NULL)
	{
		printf("alcCreateContext failed.\n");
	}

	if ( alcMakeContextCurrent(context) == ALC_FALSE )
	{
		ALCenum error = alcGetError(device);

		switch (error)
		{
		case ALC_NO_ERROR:
			printf("alcMakeContextCurrent failed: No error.\n");
			break;
		case ALC_INVALID_DEVICE:
			printf("alcMakeContextCurrent failed: Invalid device.\n");
			break;
		case ALC_INVALID_CONTEXT:
			printf("alcMakeContextCurrent failed: Invalid context.\n");
			break;
		case ALC_INVALID_ENUM:
			printf("alcMakeContextCurrent failed: Invalid enum.\n");
			break;
		case ALC_INVALID_VALUE:
			printf("alcMakeContextCurrent failed: Invalid value.\n");
			break;
		case ALC_OUT_OF_MEMORY:
			printf("alcMakeContextCurrent failed: Out of memory.\n");
			break;
		}
		return;
	}
#ifndef __linux__
	alcGetIntegerv(device, ALC_MAX_AUXILIARY_SENDS, 1, &sends);
	printf("%d sends per audio source\n", sends);
#endif
//	alListenerf(AL_REFERENCE_DISTANCE, 100.0f);
	alDistanceModel(AL_EXPONENT_DISTANCE);
	alListenerf(AL_ROLLOFF_FACTOR, 0.001f);
//	alListenerf(AL_MAX_DISTANCE, 10000.0f);

	alDopplerFactor(1.0f);
//	alDopplerVelocity(8.0f);
//	alSpeedOfSound(343.3f * UNITS_TO_METERS);
#ifndef __linux__
	alListenerf(AL_METERS_PER_UNIT, 0.3f);

	ALFWIsEFXSupported();
	alGenAuxiliaryEffectSlots(1, &slot);
	al_err = alGetError();
	if (al_err != AL_NO_ERROR)
	{
		printf("Unable to generate slot: %s\n", GetALErrorString(al_err));
		return;
	}

	alGenEffects(1, &effect);
	al_err = alGetError();
	if (al_err != AL_NO_ERROR)
	{
		printf("Unable to generate effect: %s\n", GetALErrorString(al_err));
		return;
	}
	alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);
	al_err = alGetError();
	if (al_err != AL_NO_ERROR)
	{
		printf("Unable to set effect: %s\n", GetALErrorString(al_err));
		return;
	}
	ConvertReverbParameters(&eaxBathroom, &efxReverb);
	SetEFXEAXReverbProperties(&efxReverb, effect);

//	alEffectf(effect, AL_REVERB_GAIN, 1.0f);
//	alEffectf(effect, AL_REVERB_DECAY_TIME, 20.0f);
//	alEffectf(effect, AL_REVERB_DENSITY, 0.25f);

	alGenFilters(1, &filter);
	al_err = alGetError();
	if (al_err != AL_NO_ERROR)
	{
		printf("Unable to generate filter: %s\n", GetALErrorString(al_err));
		return;
	}

	alFilteri(filter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
	alFilterf(filter, AL_LOWPASS_GAIN, 0.5f);
	alFilterf(filter, AL_LOWPASS_GAINHF, 0.5f);
#endif
}

void Audio::effects(int source)
{
#ifndef __linux__
	ALenum al_err;

	alAuxiliaryEffectSloti(slot, AL_EFFECTSLOT_EFFECT, effect);
	al_err = alGetError();
	if (al_err != AL_NO_ERROR)
	{
		printf("Unable to associate effect: %s\n", GetALErrorString(al_err));
		return;
	}

	alSource3i(source, AL_AUXILIARY_SEND_FILTER, slot, 0, AL_FILTER_NULL);
	al_err = alGetError();
	if (al_err != AL_NO_ERROR)
	{
		printf("Unable to associate slot: %s\n", GetALErrorString(al_err));
	}
#endif

}

void Audio::load(wave_t &wave)
{
	char	*end;

	wave.data = get_file(wave.file);
	if (wave.data == NULL)
	{
		printf("Unable to load wave file %s.\n", wave.file);
		memset(&wave, 0, sizeof(wave_t));
		return;
	}

	if ( checkFormat(wave.data, "WAVE") )
	{
		printf("%s is not a wave file.\n", wave.file);
		memset(&wave, 0, sizeof(wave_t));
		return;
	}

	end = wave.data + 4 + *((int *)(wave.data + 4));
	wave.format = (waveFormat_t *)findChunk( wave.data + 12, "fmt ", &(wave.dataSize), end);
	wave.pcmData = findChunk( wave.data + 12, "data", &(wave.dataSize), end);
	wave.duration = wave.dataSize / (wave.format->sampleRate * wave.format->channels * (wave.format->sampleSize / 8));


	alGenBuffers(1, (unsigned int *)&wave.buffer);
	alBufferData(wave.buffer, alFormat(&wave), wave.pcmData, wave.dataSize, wave.format->sampleRate);
	return;
}

int Audio::create_source(bool loop, bool global)
{
	ALuint hSource;
	ALenum		al_err;

	alGenSources(1, &hSource);
	al_err = alGetError();
	if (al_err != AL_NO_ERROR)
	{
/*
		char err[LINE_SIZE];

		snprintf(err, LINE_SIZE, "Unable to generate audio source: %s\n", GetALErrorString(al_err));
		throw err;
*/
	}

	if (loop)
		alSourcei(hSource, AL_LOOPING, AL_TRUE);
	else
		alSourcei(hSource, AL_LOOPING, AL_FALSE);

	if (global)
		alSourcei(hSource, AL_SOURCE_RELATIVE, AL_TRUE); 
	else
		alSourcei(hSource, AL_SOURCE_RELATIVE, AL_FALSE);

	return hSource;
}

void Audio::source_position(int hSource, float *position)
{
	alSourcefv(hSource, AL_POSITION, position);
}

void Audio::source_velocity(int hSource, float *velocity)
{
	alSourcefv(hSource, AL_VELOCITY, velocity);
}

void Audio::listener_position(float *position)
{
	alListenerfv(AL_POSITION, position);
}

void Audio::listener_velocity(float *velocity)
{
	alListenerfv(AL_VELOCITY, velocity);
}

void Audio::listener_orientation(float *orientation)
{
	alListenerfv(AL_ORIENTATION, orientation);
}

void Audio::delete_source(int hSource)
{
	ALenum		al_err;

    alDeleteSources(1, (ALuint *)&hSource);
	al_err = alGetError();
	if (al_err != AL_NO_ERROR)
	{
		printf("Unable to delete audio source: %s\n", GetALErrorString(al_err));
	}
}

void Audio::select_buffer(int hSource, int hBuffer)
{
	ALenum		al_err;

	alSourcei(hSource, AL_BUFFER, hBuffer);
	al_err = alGetError();
	if (al_err != AL_NO_ERROR)
	{
		printf("Unable to add buffer to source: %s\n", GetALErrorString(al_err));
		return;
	}
}

void Audio::delete_buffer(int hBuffer)
{
	ALenum		al_err;

	alDeleteBuffers(1, (ALuint *)&hBuffer);
	al_err = alGetError();
	if (al_err != AL_NO_ERROR)
	{
		printf("Unable to delete buffer: %s\n", GetALErrorString(al_err));
	}
}

void Audio::play(int hSource)
{
	alSourcePlay(hSource);
}

void Audio::stop(int hSource)
{
	alSourceStop(hSource);
}

void Audio::destroy()
{
	alcMakeContextCurrent(NULL);
	alcDestroyContext(context);
	alcCloseDevice(device);
	context = NULL;
	device = NULL;
}

int Audio::checkFormat(char *data, char *format)
{
	return memcmp(&data[8], format, 4);
}

char *Audio::findChunk(char *chunk, char *id, int *size, char *end)
{
	while (chunk < end)
	{
		*size = *((int *)(chunk + 4));

		if ( memcmp(chunk, id, 4) == 0 )
			return chunk + 8;
		else
			chunk += *size + 8;
	}
	return NULL;
}

ALenum Audio::alFormat(wave_t *wave)
{
	if (wave->format->channels == 2)
	{
		if (wave->format->sampleSize == 16)
			return AL_FORMAT_STEREO16;
		else
			return AL_FORMAT_STEREO8;
	}
	else
	{
		if (wave->format->sampleSize == 16)
			return AL_FORMAT_MONO16;
		else
			return AL_FORMAT_MONO8;
	}
}

