#include "include.h"

#ifndef AUDIO_H
#define AUDIO_H

class Audio
{
public:
	void init();
	void load(wave_t &wave);
	void play(int hSource);
	void stop(int hSource);
	int create_source(bool loop, bool global);
	void source_position(int hSource, float *position);
	void source_velocity(int hSource, float *velocity);
	void listener_position(float *position);
	void listener_velocity(float *velocity);
	void listener_orientation(float *orientation);
	void delete_source(int hSource);
	bool select_buffer(int hSource, int hBuffer);
	void delete_buffer(int hBuffer);
	void destroy();
	void effects(int source);
private:
	int checkFormat(char *data, char *format);
	char *findChunk(char *chunk, char *id, int *size, char *end);

    ALenum alFormat(wave_t *wave);

	ALCdevice		*device;
	ALCcontext		*context;

#ifdef WIN32
	unsigned int	slot, effect, filter;
#endif

};

#endif
