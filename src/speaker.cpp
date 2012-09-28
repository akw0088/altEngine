#include "speaker.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Speaker::Speaker(Entity *entity)
{
	Speaker::entity = entity;
	source = (unsigned int)-1;
	playing = false;
	loop = true;
}

void Speaker::destroy(Sound &audio)
{
	if (source != -1)
		audio.delete_source(source);
}

Speaker::~Speaker()
{
}