#include "trigger.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Trigger::Trigger(Entity *entity)
{
	Trigger::entity = entity;
	active = false;
	timeout = 0.0f;
	action[0] = NULL;
}

void Trigger::destroy(Sound &audio)
{
	if (source != -1)
		audio.delete_source(source);
}

Trigger::~Trigger()
{
}