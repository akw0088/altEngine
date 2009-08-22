#include "include.h"

void Sound::init()
{
	device = alcOpenDevice(NULL);

	if (device == NULL)
		throw "No sound device/driver has been found.";

	context = alcCreateContext(device, NULL);
	if (context == NULL)
		throw "alcCreateContext failed.";

	if ( alcMakeContextCurrent(context) == ALC_FALSE )
	{
		ALCenum error = alcGetError(device);

		switch (error)
		{
		case ALC_NO_ERROR:
			throw "alcMakeContextCurrent failed: No error.";
			break;
		case ALC_INVALID_DEVICE:
			throw "alcMakeContextCurrent failed: Invalid device.";
			break;
		case ALC_INVALID_CONTEXT:
			throw "alcMakeContextCurrent failed: Invalid context.";
			break;
		case ALC_INVALID_ENUM:
			throw "alcMakeContextCurrent failed: Invalid enum.";
			break;
		case ALC_INVALID_VALUE:
			throw "alcMakeContextCurrent failed: Invalid value.";
			break;
		case ALC_OUT_OF_MEMORY:
			throw "alcMakeContextCurrent failed: Out of memory.";
			break;
		}
	}
}

void Sound::load(char *file, wave_t *wave)
{
	char	err[80];
	char	*end;

	wave->file = getFile(file);

	if (wave->file == NULL)
	{
		snprintf(err, 80, "Couldnt load wave file %s.", file);
		throw err;
	}

	if ( checkFormat(wave->file, "WAVE") )
	{
		snprintf(err, 80, "%s is not a wave file.\n", file);
		throw err;
	}

	end = wave->file + 4 + *((int *)(wave->file + 4));
	wave->format = (waveFormat_t *)findChunk( wave->file + 12, "fmt ", &(wave->dataSize), end);
	wave->pcmData = findChunk( wave->file + 12, "data", &(wave->dataSize), end);
	wave->duration = wave->dataSize / (wave->format->sampleRate * wave->format->channels * (wave->format->sampleSize / 8));
}

void Sound::play(wave_t *wave)
{
	ALuint		hBuffer[2], hSource[2];

	alGenBuffers(2, hBuffer);
	if (alGetError() != AL_NO_ERROR)
		return;

	alBufferData(hBuffer[0], alFormat(wave), wave->pcmData, wave->dataSize, wave->format->sampleRate);
	if (alGetError() != AL_NO_ERROR)
		return;

	alGenSources(2, hSource);
	if (alGetError() != AL_NO_ERROR)
		return;
	alSourcei(hSource[0], AL_BUFFER, hBuffer[0]);
	if (alGetError() != AL_NO_ERROR)
		return;
	alSourcePlay(hSource[0]);
}

void Sound::destroy()
{
	alcMakeContextCurrent(NULL);
	alcDestroyContext(context);
	alcCloseDevice(device);
}

int Sound::checkFormat(char *data, char *format)
{
	return memcmp(&data[8], format, 4);
}

char *Sound::findChunk(char *chunk, char *id, int *size, char *end)
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

ALenum Sound::alFormat(wave_t *wave)
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

