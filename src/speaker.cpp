#include "speaker.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Speaker::Speaker(Entity *entity, Audio &audio, bool looping)
{
	Speaker::entity = entity;

	source = audio.create_source(looping, false);
	playing = false;
	loop = looping;
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