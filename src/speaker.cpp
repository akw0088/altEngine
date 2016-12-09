#include "speaker.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Speaker::Speaker(Entity *entity, Audio &audio)
{
	Speaker::entity = entity;

	source = audio.create_source(false, false);
	loop_source = audio.create_source(true, false);


#ifndef __OBJC__
	alSourcef(source, AL_GAIN, 8.0f);
	alSourcef(loop_source, AL_GAIN, 8.0f);
#endif
	audio.effects(loop_source);

	audio.play(loop_source);
}

void Speaker::destroy(Audio &audio)
{
	if (source != -1)
		audio.delete_source(source);
}

Speaker::~Speaker()
{
	source = -1;
}