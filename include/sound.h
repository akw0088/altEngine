#include "include.h"


#ifndef SOUND_H
#define SOUND_H

class Sound
{
public:
	void init();
	void load(char *file, wave_t *wave);
	void play(wave_t *wave);
	void destroy();
private:
	int checkFormat(char *data, char *format);
	char *findChunk(char *chunk, char *id, int *size, char *end);
	ALenum alFormat(wave_t *wave);

	ALCdevice	*device;
	ALCcontext	*context;
};

#endif
