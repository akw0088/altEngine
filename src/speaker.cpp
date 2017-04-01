#include "speaker.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Speaker::Speaker(Entity *entity, Audio &audio)
{
	Speaker::entity = entity;

	source = audio.create_source(false, false);
	loop_source = audio.create_source(true, false);

	audio.effects(loop_source);

	audio.play(loop_source);
}

void Speaker::gain(float value)
{
	alSourcef(source, AL_GAIN, value);
}

void Speaker::loop_gain(float value)
{
	alSourcef(loop_source, AL_GAIN, value);
}

void Speaker::destroy(Audio &audio)
{
	if (source >= 0)
		audio.delete_source(source);

	if (loop_source >= 0)
		audio.delete_source(loop_source);

}

Speaker::~Speaker()
{
	source = -1;
}
