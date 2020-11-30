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

#ifndef AUDIO_H
#define AUDIO_H

class Audio
{
public:
	void init();
	void load(wave_t &wave, char **pk3_list, int num_pk3);
	int load_ogg(char *filename, wave_t &wave);
	void load_doom(wave_t &wave, int *buffer);
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
	void effects(int source, int enable_filter, int filter_index);

	void set_audio_model(int model);
	void capture_start();
	void capture_sample(unsigned short *pcm, int &size);
	void capture_stop();
	void set_effect(int selected_effect);

	ALCdevice		*microphone;

private:
	int checkFormat(char *data, char *format);
	char *findChunk(char *chunk, char *id, int *size, char *end);

    ALenum alFormat(wave_t *wave);

	ALCdevice		*device;
	ALCcontext		*context;

#ifdef WIN32
	unsigned int slot;
	unsigned int effect;
	unsigned int filter[8];
#endif

	int selected_effect;

};

#endif
