#include "trigger.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Trigger::Trigger(Entity *entity, Audio &audio)
{
	Trigger::entity = entity;
	active = false;
	timeout = 0.0f;
	timeout_value = 30.0f;
	radius = 75.0f;
	action[0] = '\0';
	pickup_sound[0] = '\0';
	respawn_sound[0] = '\0';
	explode_sound[0] = '\0';
	idle_sound[0] = '\0';
	hide = true;
	self = true;
	idle = false;
	explode = false;
	health = false;
	armor = false;
	explode_timer = 0;

	source = audio.create_source(false, false);
	loop_source = audio.create_source(true, false);

#ifndef __OBJC__
	alSourcef(source, AL_GAIN, 30.0f);
	alSourcef(loop_source, AL_GAIN, 30.0f);
#endif
	audio.effects(source);
	audio.effects(loop_source);

}

void Trigger::destroy(Audio &audio)
{
	if (source != -1)
		audio.delete_source(source);
	if (loop_source != -1)
		audio.delete_source(loop_source);
}

Trigger::~Trigger()
{

}
