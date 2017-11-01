#include "trigger.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Trigger::Trigger(Entity *entity, Audio &audio)
{
	Trigger::entity = entity;
	active = false;
	client_active = false;
	timeout = 0.0f;
	timeout_value = 30.0f;
	radius = 75.0f;
	action[0] = '\0';
	pickup_buf = -1;
	respawn_buf = -1;

	pickup_index = -1;
	respawn_index = -1;
	explode_index = -1;
	idle_index = -1;



//	idle_timer = 0;


	hide = true;
//	idle = false;
//	explode = false;
	health = false;
	armor = false;
	played = false;

//	explode_type = 0;
//	explode_color.x = 1.0f;
//	explode_color.y = 1.0f;
//	explode_color.z = 1.0f;
//	explode_intensity = 0.0f;
//	explode_timer = 0;
//	splash_damage = 0;
//	splash_radius = 0.0f;
//	knockback = 0.0f;
//	owner = -1;
//	num_bounce = 1;
//	projectile = false;
	noise = false;

	source = -1;
	loop_source = -1;

}

void Trigger::create_sources(Audio &audio)
{
	source = audio.create_source(false, false);
	loop_source = audio.create_source(true, false);

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
