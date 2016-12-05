#include "trigger.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Trigger::Trigger(Entity *entity)
{
	Trigger::entity = entity;
	active = false;
	timeout = 0.0f;
	action[0] = '\0';
	pickup_snd[0] = '\0';
	respawn_snd[0] = '\0';
	hide = true;
	self = true;
	idle = false;
}

void Trigger::destroy(Audio &audio)
{
	if (source != -1)
		audio.delete_source(source);
}

Trigger::~Trigger()
{
}
