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


#include "include.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include "stb_vorbis.h" // for ogg compressed audio

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

#ifdef EFXUTIL
EAXREVERBPROPERTIES environment[] = {
	REVERB_PRESET_GENERIC,
	REVERB_PRESET_PADDEDCELL,
	REVERB_PRESET_ROOM,
	REVERB_PRESET_BATHROOM,
	REVERB_PRESET_LIVINGROOM,
	REVERB_PRESET_STONEROOM,
	REVERB_PRESET_AUDITORIUM,
	REVERB_PRESET_CONCERTHALL,
	REVERB_PRESET_CAVE,
	REVERB_PRESET_ARENA,
	REVERB_PRESET_HANGAR, // 10
	REVERB_PRESET_CARPETTEDHALLWAY,
	REVERB_PRESET_HALLWAY,
	REVERB_PRESET_STONECORRIDOR,
	REVERB_PRESET_ALLEY,
	REVERB_PRESET_FOREST,
	REVERB_PRESET_CITY,
	REVERB_PRESET_MOUNTAINS,
	REVERB_PRESET_QUARRY,
	REVERB_PRESET_PLAIN,
	REVERB_PRESET_PARKINGLOT, //20
	REVERB_PRESET_SEWERPIPE,
	REVERB_PRESET_UNDERWATER,
	REVERB_PRESET_DRUGGED,
	REVERB_PRESET_DIZZY,
	REVERB_PRESET_PSYCHOTIC,
	REVERB_PRESET_CASTLE_SMALLROOM,
	REVERB_PRESET_CASTLE_SHORTPASSAGE,
	REVERB_PRESET_CASTLE_MEDIUMROOM,
	REVERB_PRESET_CASTLE_LONGPASSAGE,
	REVERB_PRESET_CASTLE_LARGEROOM, // 30
	REVERB_PRESET_CASTLE_HALL,
	REVERB_PRESET_CASTLE_CUPBOARD,
	REVERB_PRESET_CASTLE_COURTYARD,
	REVERB_PRESET_CASTLE_ALCOVE,
	REVERB_PRESET_FACTORY_ALCOVE,
	REVERB_PRESET_FACTORY_SHORTPASSAGE,
	REVERB_PRESET_FACTORY_MEDIUMROOM,
	REVERB_PRESET_FACTORY_LONGPASSAGE,
	REVERB_PRESET_FACTORY_LARGEROOM,
	REVERB_PRESET_FACTORY_HALL, // 40
	REVERB_PRESET_FACTORY_CUPBOARD,
	REVERB_PRESET_FACTORY_COURTYARD,
	REVERB_PRESET_FACTORY_SMALLROOM,
	REVERB_PRESET_ICEPALACE_ALCOVE,
	REVERB_PRESET_ICEPALACE_SHORTPASSAGE,
	REVERB_PRESET_ICEPALACE_MEDIUMROOM,
	REVERB_PRESET_ICEPALACE_LONGPASSAGE,
	REVERB_PRESET_ICEPALACE_LARGEROOM,
	REVERB_PRESET_ICEPALACE_HALL,
	REVERB_PRESET_ICEPALACE_CUPBOARD, //50
	REVERB_PRESET_ICEPALACE_COURTYARD,
	REVERB_PRESET_ICEPALACE_SMALLROOM,
	REVERB_PRESET_SPACESTATION_ALCOVE,
	REVERB_PRESET_SPACESTATION_MEDIUMROOM,
	REVERB_PRESET_SPACESTATION_SHORTPASSAGE,
	REVERB_PRESET_SPACESTATION_LONGPASSAGE,
	REVERB_PRESET_SPACESTATION_LARGEROOM,
	REVERB_PRESET_SPACESTATION_HALL,
	REVERB_PRESET_SPACESTATION_CUPBOARD,
	REVERB_PRESET_SPACESTATION_SMALLROOM, //60
	REVERB_PRESET_WOODEN_ALCOVE,
	REVERB_PRESET_WOODEN_SHORTPASSAGE,
	REVERB_PRESET_WOODEN_MEDIUMROOM,
	REVERB_PRESET_WOODEN_LONGPASSAGE,
	REVERB_PRESET_WOODEN_LARGEROOM,
	REVERB_PRESET_WOODEN_HALL,
	REVERB_PRESET_WOODEN_CUPBOARD,
	REVERB_PRESET_WOODEN_SMALLROOM,
	REVERB_PRESET_WOODEN_COURTYARD,
	REVERB_PRESET_SPORT_EMPTYSTADIUM, //70
	REVERB_PRESET_SPORT_SQUASHCOURT,
	REVERB_PRESET_SPORT_SMALLSWIMMINGPOOL,
	REVERB_PRESET_SPORT_LARGESWIMMINGPOOL,
	REVERB_PRESET_SPORT_GYMNASIUM,
	REVERB_PRESET_SPORT_FULLSTADIUM,
	REVERB_PRESET_SPORT_STADIUMTANNOY,
	REVERB_PRESET_PREFAB_WORKSHOP,
	REVERB_PRESET_PREFAB_SCHOOLROOM,
	REVERB_PRESET_PREFAB_PRACTISEROOM,
	REVERB_PRESET_PREFAB_OUTHOUSE, // 80
	REVERB_PRESET_PREFAB_CARAVAN,
	REVERB_PRESET_DOME_TOMB,
	REVERB_PRESET_PIPE_SMALL,
	REVERB_PRESET_DOME_SAINTPAULS,
	REVERB_PRESET_PIPE_LONGTHIN,
	REVERB_PRESET_PIPE_LARGE,
	REVERB_PRESET_PIPE_RESONANT,
	REVERB_PRESET_OUTDOORS_BACKYARD, 
	REVERB_PRESET_OUTDOORS_ROLLINGPLAINS,
	REVERB_PRESET_OUTDOORS_DEEPCANYON, //90
	REVERB_PRESET_OUTDOORS_CREEK,
	REVERB_PRESET_OUTDOORS_VALLEY,
	REVERB_PRESET_MOOD_HEAVEN,
	REVERB_PRESET_MOOD_HELL,
	REVERB_PRESET_MOOD_MEMORY,
	REVERB_PRESET_DRIVING_COMMENTATOR,
	REVERB_PRESET_DRIVING_PITGARAGE,
	REVERB_PRESET_DRIVING_INCAR_RACER, 
	REVERB_PRESET_DRIVING_INCAR_SPORTS,
	REVERB_PRESET_DRIVING_INCAR_LUXURY, //100
	REVERB_PRESET_DRIVING_FULLGRANDSTAND,
	REVERB_PRESET_DRIVING_EMPTYGRANDSTAND,
	REVERB_PRESET_DRIVING_TUNNEL,
	REVERB_PRESET_CITY_STREETS,
	REVERB_PRESET_CITY_SUBWAY,
	REVERB_PRESET_CITY_MUSEUM,
	REVERB_PRESET_CITY_LIBRARY,
	REVERB_PRESET_CITY_UNDERPASS,
	REVERB_PRESET_CITY_ABANDONED,
	REVERB_PRESET_DUSTYROOM, //110
	REVERB_PRESET_CHAPEL,
	REVERB_PRESET_SMALLWATERROOM
};
#endif

char environment_names[][128] = {
	"REVERB_PRESET_GENERIC",
	"REVERB_PRESET_PADDEDCELL",
	"REVERB_PRESET_ROOM",
	"REVERB_PRESET_BATHROOM",
	"REVERB_PRESET_LIVINGROOM",
	"REVERB_PRESET_STONEROOM",
	"REVERB_PRESET_AUDITORIUM",
	"REVERB_PRESET_CONCERTHALL",
	"REVERB_PRESET_CAVE",
	"REVERB_PRESET_ARENA",
	"REVERB_PRESET_HANGAR", //10
	"REVERB_PRESET_CARPETTEDHALLWAY",
	"REVERB_PRESET_HALLWAY",
	"REVERB_PRESET_STONECORRIDOR",
	"REVERB_PRESET_ALLEY",
	"REVERB_PRESET_FOREST",
	"REVERB_PRESET_CITY",
	"REVERB_PRESET_MOUNTAINS",
	"REVERB_PRESET_QUARRY",
	"REVERB_PRESET_PLAIN",
	"REVERB_PRESET_PARKINGLOT", //20
	"REVERB_PRESET_SEWERPIPE",
	"REVERB_PRESET_UNDERWATER",
	"REVERB_PRESET_DRUGGED",
	"REVERB_PRESET_DIZZY",
	"REVERB_PRESET_PSYCHOTIC",
	"REVERB_PRESET_CASTLE_SMALLROOM",
	"REVERB_PRESET_CASTLE_SHORTPASSAGE",
	"REVERB_PRESET_CASTLE_MEDIUMROOM",
	"REVERB_PRESET_CASTLE_LONGPASSAGE",
	"REVERB_PRESET_CASTLE_LARGEROOM", // 30
	"REVERB_PRESET_CASTLE_HALL",
	"REVERB_PRESET_CASTLE_CUPBOARD",
	"REVERB_PRESET_CASTLE_COURTYARD",
	"REVERB_PRESET_CASTLE_ALCOVE",
	"REVERB_PRESET_FACTORY_ALCOVE",
	"REVERB_PRESET_FACTORY_SHORTPASSAGE",
	"REVERB_PRESET_FACTORY_MEDIUMROOM",
	"REVERB_PRESET_FACTORY_LONGPASSAGE",
	"REVERB_PRESET_FACTORY_LARGEROOM",
	"REVERB_PRESET_FACTORY_HALL", // 40
	"REVERB_PRESET_FACTORY_CUPBOARD",
	"REVERB_PRESET_FACTORY_COURTYARD",
	"REVERB_PRESET_FACTORY_SMALLROOM",
	"REVERB_PRESET_ICEPALACE_ALCOVE",
	"REVERB_PRESET_ICEPALACE_SHORTPASSAGE",
	"REVERB_PRESET_ICEPALACE_MEDIUMROOM",
	"REVERB_PRESET_ICEPALACE_LONGPASSAGE",
	"REVERB_PRESET_ICEPALACE_LARGEROOM",
	"REVERB_PRESET_ICEPALACE_HALL",
	"REVERB_PRESET_ICEPALACE_CUPBOARD", //50
	"REVERB_PRESET_ICEPALACE_COURTYARD",
	"REVERB_PRESET_ICEPALACE_SMALLROOM",
	"REVERB_PRESET_SPACESTATION_ALCOVE",
	"REVERB_PRESET_SPACESTATION_MEDIUMROOM",
	"REVERB_PRESET_SPACESTATION_SHORTPASSAGE",
	"REVERB_PRESET_SPACESTATION_LONGPASSAGE",
	"REVERB_PRESET_SPACESTATION_LARGEROOM",
	"REVERB_PRESET_SPACESTATION_HALL",
	"REVERB_PRESET_SPACESTATION_CUPBOARD",
	"REVERB_PRESET_SPACESTATION_SMALLROOM", //60
	"REVERB_PRESET_WOODEN_ALCOVE",
	"REVERB_PRESET_WOODEN_SHORTPASSAGE",
	"REVERB_PRESET_WOODEN_MEDIUMROOM",
	"REVERB_PRESET_WOODEN_LONGPASSAGE",
	"REVERB_PRESET_WOODEN_LARGEROOM",
	"REVERB_PRESET_WOODEN_HALL",
	"REVERB_PRESET_WOODEN_CUPBOARD",
	"REVERB_PRESET_WOODEN_SMALLROOM",
	"REVERB_PRESET_WOODEN_COURTYARD", //70
	"REVERB_PRESET_SPORT_EMPTYSTADIUM",
	"REVERB_PRESET_SPORT_SQUASHCOURT",
	"REVERB_PRESET_SPORT_SMALLSWIMMINGPOOL",
	"REVERB_PRESET_SPORT_LARGESWIMMINGPOOL",
	"REVERB_PRESET_SPORT_GYMNASIUM",
	"REVERB_PRESET_SPORT_FULLSTADIUM",
	"REVERB_PRESET_SPORT_STADIUMTANNOY",
	"REVERB_PRESET_PREFAB_WORKSHOP",
	"REVERB_PRESET_PREFAB_SCHOOLROOM",
	"REVERB_PRESET_PREFAB_PRACTISEROOM", //80
	"REVERB_PRESET_PREFAB_OUTHOUSE",
	"REVERB_PRESET_PREFAB_CARAVAN",
	"REVERB_PRESET_DOME_TOMB",
	"REVERB_PRESET_PIPE_SMALL",
	"REVERB_PRESET_DOME_SAINTPAULS",
	"REVERB_PRESET_PIPE_LONGTHIN",
	"REVERB_PRESET_PIPE_LARGE",
	"REVERB_PRESET_PIPE_RESONANT",
	"REVERB_PRESET_OUTDOORS_BACKYARD",
	"REVERB_PRESET_OUTDOORS_ROLLINGPLAINS", //90
	"REVERB_PRESET_OUTDOORS_DEEPCANYON",
	"REVERB_PRESET_OUTDOORS_CREEK",
	"REVERB_PRESET_OUTDOORS_VALLEY",
	"REVERB_PRESET_MOOD_HEAVEN",
	"REVERB_PRESET_MOOD_HELL",
	"REVERB_PRESET_MOOD_MEMORY",
	"REVERB_PRESET_DRIVING_COMMENTATOR",
	"REVERB_PRESET_DRIVING_PITGARAGE",
	"REVERB_PRESET_DRIVING_INCAR_RACER",
	"REVERB_PRESET_DRIVING_INCAR_SPORTS",
	"REVERB_PRESET_DRIVING_INCAR_LUXURY", // 100
	"REVERB_PRESET_DRIVING_FULLGRANDSTAND",
	"REVERB_PRESET_DRIVING_EMPTYGRANDSTAND",
	"REVERB_PRESET_DRIVING_TUNNEL",
	"REVERB_PRESET_CITY_STREETS",
	"REVERB_PRESET_CITY_SUBWAY",
	"REVERB_PRESET_CITY_MUSEUM",
	"REVERB_PRESET_CITY_LIBRARY",
	"REVERB_PRESET_CITY_UNDERPASS",
	"REVERB_PRESET_CITY_ABANDONED",
	"REVERB_PRESET_DUSTYROOM", //110
	"REVERB_PRESET_CHAPEL",
	"REVERB_PRESET_SMALLWATERROOM"
};

#ifndef DEDICATED
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
#endif




#ifdef EFXUTIL

// Imported EFX functions
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
			debugf("Unable to set effect: %s", GetALErrorString(al_err));
		}
		return 1;
	}

	return bReturn;
}
#endif

#ifndef DEDICATED
void Audio::init()
{
#ifdef WIN32
	EFXEAXREVERBPROPERTIES efxReverb;
	int attrib[] = {ALC_MAX_AUXILIARY_SENDS, 4};
	int sends;
	ALenum al_err;
#endif
	selected_effect = 0;

	debugf("Using default audio device: %s\n", alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER));
	device = alcOpenDevice(NULL);
	if (device == NULL)
	{
		debugf("No sound device/driver has been found.\n");
		return;
	}

	// step is every 8 ms, 48khz per second, or 48 samples per ms, which is 384 samples per step give or take
	microphone = alcCaptureOpenDevice(NULL, VOICE_SAMPLE_RATE, AL_FORMAT_MONO16, MIC_BUFFER_SIZE);
	if (microphone == NULL)
	{
		debugf("No microphone has been found.\n");
		return;
	}


#ifdef WIN32
    context = alcCreateContext(device, attrib);
#else
    context = alcCreateContext(device, NULL);
#endif
	if (context == NULL)
	{
		debugf("alcCreateContext failed.\n");
	}

	if ( alcMakeContextCurrent(context) == ALC_FALSE )
	{
		ALCenum error = alcGetError(device);

		switch (error)
		{
		case ALC_NO_ERROR:
			debugf("alcMakeContextCurrent failed: No error.\n");
			break;
		case ALC_INVALID_DEVICE:
			debugf("alcMakeContextCurrent failed: Invalid device.\n");
			break;
		case ALC_INVALID_CONTEXT:
			debugf("alcMakeContextCurrent failed: Invalid context.\n");
			break;
		case ALC_INVALID_ENUM:
			debugf("alcMakeContextCurrent failed: Invalid enum.\n");
			break;
		case ALC_INVALID_VALUE:
			debugf("alcMakeContextCurrent failed: Invalid value.\n");
			break;
		case ALC_OUT_OF_MEMORY:
			debugf("alcMakeContextCurrent failed: Out of memory.\n");
			break;
		}
		return;
	}
#ifdef WIN32
	alcGetIntegerv(device, ALC_MAX_AUXILIARY_SENDS, 1, &sends);
	debugf("%d sends per audio source\n", sends);
#endif

	//gain = 	(distance / AL_REFERENCE_DISTANCE) ^ (-AL_ROLLOFF_FACTOR
	alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
//	alListenerf(AL_MAX_DISTANCE, 2000.0f);
//	alListenerf(AL_REFERENCE_DISTANCE, 75.0f);
//	alListenerf(AL_ROLLOFF_FACTOR, 0.0001);
	

	alDopplerFactor(1.0f);
//	alDopplerVelocity(8.0f);
//	alSpeedOfSound(343.3f * UNITS_TO_METERS);
#ifdef WIN32
	// 8 units = 1 foot, 1 foot = 0.3 meters
	// 1 unit = 0.3 / 8 meters
	alListenerf(AL_METERS_PER_UNIT, 0.375f);
#ifdef EFXUTIL
	if (ALFWIsEFXSupported() == false)
	{
		printf("Environmental Audio Effects not supported\n");
	}

	alGenAuxiliaryEffectSlots(1, &slot);
	al_err = alGetError();
	if (al_err != AL_NO_ERROR)
	{
		debugf("Unable to generate slot: %s\n", GetALErrorString(al_err));
		return;
	}

	alGenEffects(1, &effect);
	al_err = alGetError();
	if (al_err != AL_NO_ERROR)
	{
		debugf("Unable to generate effect: %s\n", GetALErrorString(al_err));
		return;
	}
	alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);
	al_err = alGetError();
	if (al_err != AL_NO_ERROR)
	{
		debugf("Unable to set effect: %s\n", GetALErrorString(al_err));
		return;
	}

	ConvertReverbParameters(&environment[selected_effect], &efxReverb);
	SetEFXEAXReverbProperties(&efxReverb, effect);

//	alEffectf(effect, AL_REVERB_GAIN, 1.0f);
//	alEffectf(effect, AL_REVERB_DECAY_TIME, 20.0f);
//	alEffectf(effect, AL_REVERB_DENSITY, 0.25f);

	alGenFilters(8, filter);
	al_err = alGetError();
	if (al_err != AL_NO_ERROR)
	{
		debugf("Unable to generate filter: %s\n", GetALErrorString(al_err));
		return;
	}

	alFilteri(filter[0], AL_FILTER_TYPE, AL_FILTER_LOWPASS);
	alFilterf(filter[0], AL_LOWPASS_GAIN, 0.5f);
	alFilterf(filter[0], AL_LOWPASS_GAINHF, 0.5f);

	alFilteri(filter[1], AL_FILTER_TYPE, AL_FILTER_LOWPASS);
	alFilterf(filter[1], AL_LOWPASS_GAIN, 0.25f);
	alFilterf(filter[1], AL_LOWPASS_GAINHF, 0.25f);

	alFilteri(filter[2], AL_FILTER_TYPE, AL_FILTER_LOWPASS);
	alFilterf(filter[2], AL_LOWPASS_GAIN, 0.75f);
	alFilterf(filter[2], AL_LOWPASS_GAINHF, 0.75f);

	alFilteri(filter[3], AL_FILTER_TYPE, AL_FILTER_BANDPASS);
	alFilterf(filter[3], AL_BANDPASS_GAIN, 0.5f);
	alFilterf(filter[3], AL_BANDPASS_GAINLF, 0.5f);
	alFilterf(filter[3], AL_BANDPASS_GAINHF, 0.25f);

	alFilteri(filter[4], AL_FILTER_TYPE, AL_FILTER_BANDPASS);
	alFilterf(filter[4], AL_BANDPASS_GAIN, 0.5f);
	alFilterf(filter[4], AL_BANDPASS_GAINLF, 0.25f);
	alFilterf(filter[4], AL_BANDPASS_GAINHF, 0.5f);

	alFilteri(filter[5], AL_FILTER_TYPE, AL_FILTER_HIGHPASS);
	alFilterf(filter[5], AL_HIGHPASS_GAIN, 0.5f);
	alFilterf(filter[5], AL_HIGHPASS_GAINLF, 0.5f);

	alFilteri(filter[6], AL_FILTER_TYPE, AL_FILTER_HIGHPASS);
	alFilterf(filter[6], AL_HIGHPASS_GAIN, 0.25f);
	alFilterf(filter[6], AL_HIGHPASS_GAINLF, 0.25f);
	
	alFilteri(filter[7], AL_FILTER_TYPE, AL_FILTER_HIGHPASS);
	alFilterf(filter[7], AL_HIGHPASS_GAIN, 0.75f);
	alFilterf(filter[7], AL_HIGHPASS_GAINLF, 0.75f);
#endif

#endif
}


void Audio::set_effect(int selected_effect)
{
#ifdef EFXUTIL
	EFXEAXREVERBPROPERTIES efxReverb;


	ConvertReverbParameters(&environment[selected_effect], &efxReverb);
	SetEFXEAXReverbProperties(&efxReverb, effect);

	printf("Setting effect %s\r\n", environment_names[selected_effect]);
#endif
}

void Audio::set_audio_model(int model)
{
	switch (model)
	{
	case 0:
		alDistanceModel(AL_INVERSE_DISTANCE);
		break;
	case 1:
		alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
		break;
	case 2:
		alDistanceModel(AL_LINEAR_DISTANCE);
		break;
	case 3:
		alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
		break;
	case 4:
		alDistanceModel(AL_EXPONENT_DISTANCE);
		break;
	case 5:
		alDistanceModel(AL_EXPONENT_DISTANCE_CLAMPED);
		break;
	}
}


void Audio::effects(int source, int enable_filter, int filter_index)
{
#ifdef WIN32
#ifdef EFXUTIL
	ALenum al_err;

	alAuxiliaryEffectSloti(slot, AL_EFFECTSLOT_EFFECT, effect);
	al_err = alGetError();
	if (al_err != AL_NO_ERROR)
	{
		debugf("Unable to associate effect: %s\n", GetALErrorString(al_err));
		return;
	}


	if (enable_filter)
		alSource3i(source, AL_AUXILIARY_SEND_FILTER, slot, 0, filter[filter_index]);
	else
		alSource3i(source, AL_AUXILIARY_SEND_FILTER, slot, 0, AL_FILTER_NULL);
	al_err = alGetError();
	if (al_err != AL_NO_ERROR)
	{
		debugf("Unable to associate slot: %s\n", GetALErrorString(al_err));
	}
#endif
#endif
}

int Audio::load_ogg(char *filename, wave_t &wave)
{
	short *data = NULL;
	int channels = 0;
	int sample_rate = 0;
	int datasize = 0;
	int length;

	length = stb_vorbis_decode_filename(filename, &channels, &sample_rate, &data);

	wave.format->channels = channels;
	wave.format->sampleRate = sample_rate;
	wave.format->sampleSize = 8 * channels;
	wave.format->align = 2 * channels;
	datasize = length * 2;

	//wave.buffer = data;
	wave.pcmData = (char *)data;
	wave.data = (char *)data;

	alGenBuffers(1, (unsigned int *)&wave.buffer);
	alBufferData(wave.buffer, alFormat(&wave), wave.pcmData, datasize, wave.format->sampleRate);
	free((void *)wave.pcmData);
	return wave.buffer;
}



void Audio::load_doom(wave_t &wave, int *buffer)
{
	alGenBuffers(1, (unsigned int *)buffer);
	alBufferData(*buffer, AL_FORMAT_MONO8, wave.pcmData, wave.dataSize, wave.format->sampleRate);
}

void Audio::load(wave_t &wave, char **pk3_list, int num_pk3)
{
	char	*end;

	wave.data = NULL;
	if (strstr(wave.file, "media/"))
	{
		wave.data = get_file(wave.file, NULL);
	}
	else
	{
		for (int i = 0; i < num_pk3; i++)
		{
			get_zipfile(pk3_list[i] + FILE_OFFSET, wave.file, (unsigned char **)&wave.data, NULL);
			if (wave.data != NULL)
			{
				break;
			}
		}
	}

	if (wave.data == NULL)
	{
		char *pak = "media/PAK0.PAK";
		wave.data = get_pakfile(pak, wave.file);
	}

	if (wave.data == NULL)
	{
		debugf("Unable to load wave %s\n", wave.file);
		memset(&wave, 0, sizeof(wave_t));
		return;
	}

	if ( checkFormat(wave.data, "WAVE") )
	{
		debugf("%s is not a wave file.\n", wave.file);
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
	ALuint hSource = -1;
	ALenum		al_err;
	bool enable_filter = false;

	alGenSources(1, &hSource);
	al_err = alGetError();
	if (al_err != AL_NO_ERROR)
	{
		debugf("Unable to generate audio source: %s\n", GetALErrorString(al_err));
		return hSource;
	}

	if (loop)
		alSourcei(hSource, AL_LOOPING, AL_TRUE);
	else
		alSourcei(hSource, AL_LOOPING, AL_FALSE);

	if (global)
		alSourcei(hSource, AL_SOURCE_RELATIVE, AL_TRUE); 
	else
		alSourcei(hSource, AL_SOURCE_RELATIVE, AL_FALSE);

	effects(hSource, enable_filter, 0);

	return hSource;
}

void Audio::source_position(int hSource, float *position)
{
	if (hSource == -1)
	{
		//debugf("Attempting to position unallocated source\n");
		return;
	}

	alSourcefv(hSource, AL_POSITION, position);
	ALenum al_err = alGetError();
	if (al_err != AL_NO_ERROR)
	{
		debugf("Error alSourcefv position : %s\n", GetALErrorString(al_err));
		hSource = -1;
	}
}

void Audio::source_velocity(int hSource, float *velocity)
{

	if (hSource == -1)
	{
		//debugf("Attempting to add velocity to unallocated source\n");
		return;
	}

	alSourcefv(hSource, AL_VELOCITY, velocity);
	ALenum al_err = alGetError();
	if (al_err != AL_NO_ERROR)
	{
		debugf("Error alSourcefv velocity : %s\n", GetALErrorString(al_err));
		hSource = -1;
	}
}

void Audio::listener_position(float *position)
{
	ALenum al_err;

	alListenerfv(AL_POSITION, position);
	al_err = alGetError();
	if (al_err != AL_NO_ERROR)
	{
		debugf("Error alListenerfv : %s\n", GetALErrorString(al_err));
	}
}

void Audio::listener_velocity(float *velocity)
{
	ALenum al_err;

	alListenerfv(AL_VELOCITY, velocity);

	al_err = alGetError();
	if (al_err != AL_NO_ERROR)
	{
		debugf("Error alListenerfv velocity: %s\n", GetALErrorString(al_err));
	}
}

void Audio::listener_orientation(float *orientation)
{
	ALenum al_err;

	alListenerfv(AL_ORIENTATION, orientation);

	al_err = alGetError();
	if (al_err != AL_NO_ERROR)
	{
		debugf("Error alListenerfv orientation: %s\n", GetALErrorString(al_err));
	}
}

void Audio::delete_source(int hSource)
{
	ALenum		al_err;

    alDeleteSources(1, (ALuint *)&hSource);
	al_err = alGetError();
	if (al_err != AL_NO_ERROR)
	{
		debugf("Unable to delete audio source: %s\n", GetALErrorString(al_err));
	}
}

bool Audio::select_buffer(int hSource, int hBuffer)
{
	ALenum		al_err;

	alSourceStop(hSource);
	alSourcei(hSource, AL_BUFFER, hBuffer);
	al_err = alGetError();
	if (al_err != AL_NO_ERROR)
	{
		debugf("Unable to add buffer to source: %s\n", GetALErrorString(al_err));
		return true;
	}
	return true;
}

void Audio::delete_buffer(int hBuffer)
{
	ALenum		al_err;

	alDeleteBuffers(1, (ALuint *)&hBuffer);
	al_err = alGetError();
	if (al_err != AL_NO_ERROR)
	{
		debugf("Unable to delete buffer: %s\n", GetALErrorString(al_err));
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
	alcCaptureCloseDevice(microphone);
	context = NULL;
	device = NULL;
	microphone = NULL;
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

void Audio::capture_start()
{
	if (microphone == NULL)
		return;

	alcCaptureStart(microphone);
}

int Audio::capture_sample(unsigned short *pcm, int &size)
{
	// Samples are not bytes, 2 bytes per sample.
	// Use bytes everywhere but where API's dictate samples
	unsigned int samples = 0;

	if (microphone == NULL)
		return -1;

	static int max_recv = 0;

	alcGetIntegerv(microphone, ALC_CAPTURE_SAMPLES, sizeof(int), (int *)&samples);
	if (samples * 2 > MIC_BUFFER_SIZE)
	{
		// we have more than a buffer, only get one buffer out
		size = MIC_BUFFER_SIZE;
	}
	else if (samples * 2 < MIC_BUFFER_SIZE)
	{
		// if we have less than one buffer, leave it in the buffer
		size = 0;
		return 0;
	}

	// Get one buffer full of samples
	alcCaptureSamples(microphone, pcm, (MIC_BUFFER_SIZE >> 1));

	if (size > max_recv)
	{
		max_recv = size;
	}
	return size;
}


void Audio::capture_stop()
{
	if (microphone == NULL)
		return;

	alcCaptureStop(microphone);
}
#endif

#ifdef DEDICATED
bool ALFWIsEFXSupported()
{
	return true;
}

void Audio::init()
{
}

void Audio::set_effect(int selected_effect)
{
}

void Audio::effects(int source, int enable_filter, int filter_index)
{
}

int Audio::load_ogg(char *filename, wave_t &wave)
{
	return 0;
}


void Audio::load(wave_t &wave, char **pk3_list, int num_pk3)
{
}

int Audio::create_source(bool loop, bool global)
{
	return 1;
}

void Audio::source_position(int hSource, float *position)
{
}

void Audio::source_velocity(int hSource, float *velocity)
{
}

void Audio::listener_position(float *position)
{
}

void Audio::listener_velocity(float *velocity)
{
}

void Audio::listener_orientation(float *orientation)
{
}

void Audio::delete_source(int hSource)
{
}

bool Audio::select_buffer(int hSource, int hBuffer)
{
	return true;
}

void Audio::delete_buffer(int hBuffer)
{
}

void Audio::play(int hSource)
{
}

void Audio::stop(int hSource)
{
}

void Audio::destroy()
{
}

int Audio::checkFormat(char *data, char *format)
{
	return 0;
}

char *Audio::findChunk(char *chunk, char *id, int *size, char *end)
{
	return (char *)0xDEADBEEF;
}

#ifndef DEDICATED
ALenum Audio::alFormat(wave_t *wave)
{
	return AL_FORMAT_STEREO16;
}
#endif

void Audio::set_audio_model(int model)
{

}

void Audio::load_doom(wave_t &wave, int *buffer)
{
}

void Audio::capture_start()
{
}

void Audio::capture_sample(unsigned short *pcm, int &size)
{
}


void Audio::capture_stop()
{
}




#endif
