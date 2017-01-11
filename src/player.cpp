#include "player.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include <float.h>

char bot_state_name[16][80] = {
	"BOT_IDLE",
	"BOT_ALERT",
	"BOT_ATTACK",
	"BOT_GET_ITEM",
	"BOT_EXPLORE",
	"BOT_DEAD",
};

// Player sound sets
const char *models[23] = {
	"anarki",			//0
	"biker",			//1
	"bitterman",		//2
	"bones",			//3
	"crash",			//4
	"doom",				//5
	"grunt",			//6
	"hunter",			//7
	"keel",				//8
	"klesk",			//9
	"lucy",				//10
	"major",			//11
	"mynx",				//12
	"orbb",				//13
	"ranger",			//14
	"razor",			//15
	"sarge",			//16
	"slash",			//17
	"sorlag",			//18
	"tankjr",			//19
	"uriel",			//20
	"visor",			//21
	"xaero"				//22
};

#define NUM_PATH 64 // Should equal map navpoint count

Player::Player(Entity *entity, Graphics &gfx, Audio &audio, int model)
: weapon_machinegun(entity), weapon_shotgun(entity), weapon_grenade(entity), weapon_rocket(entity),
  weapon_lightning(entity), weapon_railgun(entity), weapon_plasma(entity)
{
	Player::entity = entity;

	spawned = false;

	zoom_level = 4.0;

	path.path = new int[NUM_PATH];
	get_item = 0;
	ignore[0] = '\0';

	model_index = model; // default to visor -- ranger isnt in demo files :'( 

	strcpy(model_name, models[model_index]);
	attack_sound[0] = '\0';
	weapon_idle_sound[0] = '\0';
	sprintf(death1_sound, "sound/player/%s/death1.wav", model_name);
	sprintf(death2_sound, "sound/player/%s/death2.wav", model_name);
	sprintf(death3_sound, "sound/player/%s/death3.wav", model_name);

	sprintf(pain25_sound, "sound/player/%s/pain25_1.wav", model_name);
	sprintf(pain50_sound, "sound/player/%s/pain50_1.wav", model_name);
	sprintf(pain75_sound, "sound/player/%s/pain75_1.wav", model_name);
	sprintf(pain100_sound, "sound/player/%s/pain100_1.wav", model_name);

	sprintf(jump_sound, "sound/player/%s/jump1.wav", model_name);
	sprintf(fall_sound, "sound/player/%s/fall1.wav", model_name);
	sprintf(pit_sound, "sound/player/%s/falling1.wav", model_name);
	sprintf(land_sound, "sound/player/land1.wav");
	sprintf(pad_sound, "sound/world/jumppad.wav");

	sprintf(empty_sound, "sound/weapons/noammo.wav");
	sprintf(weapon_swap_sound, "sound/weapons/change.wav");

	sprintf(medikit_sound, "sound/items/use_medkit.wav");
	sprintf(noitem_sound, "sound/items/use_nothing.wav");
	sprintf(regen_bump_sound, "sound/items/regen.wav");
	

	sprintf(step1_sound, "sound/player/footsteps/step1.wav");
	sprintf(step2_sound, "sound/player/footsteps/step2.wav");
	sprintf(step3_sound, "sound/player/footsteps/step3.wav");
	sprintf(step4_sound, "sound/player/footsteps/step4.wav");

	sprintf(waterin_sound,  "sound/player/watr_in.wav");
	sprintf(waterout_sound, "sound/player/watr_out.wav");

	sprintf(gurp1_sound, "sound/player/gurp1.wav");
	sprintf(gurp2_sound, "sound/player/gurp2.wav");

	sprintf(gibbed_sound, "sound/player/gibsplt1.wav");
	sprintf(gibimpact1_sound, "sound/player/gibimp1.wav");
	sprintf(gibimpact2_sound, "sound/player/gibimp2.wav");
	sprintf(gibimpact3_sound, "sound/player/gibimp3.wav");


	sprintf(telein_sound, "sound/world/telein.wav");
	sprintf(teleout_sound, "sound/world/teleout.wav");
	


	// Probably should be global and not tied to player entity
	sprintf(chat_sound, "sound/player/talk.wav");
	
	//sounds/player/watr_un.wav // another water in?
	//sound/player/fry.wav

	health = 100;
	armor = 0;
	flight_timer = 0;
	teleport_timer = 0;
	drown_timer = 0;
	regen_timer = 0;
	haste_timer = 0;
	quad_timer = 0;
	reload_timer = 0;
	invisibility_timer = 0;
	teleport_timer = 0;
	click_timer = 0;

	holdable_teleporter = false;
	holdable_medikit = false;


	weapon_flags = WEAPON_MACHINEGUN;
	current_weapon = wp_machinegun;
	ammo_rockets = 0;
	ammo_grenades = 0;
	ammo_slugs = 0;
	ammo_shells = 0;
	ammo_bullets = 50;
	ammo_lightning = 0;
	ammo_plasma = 0;
	ammo_bfg = 0;
	reload_timer = 0;
	drown_timer++;
	sprintf(name, "UnnamedPlayer");
	memset(&stats, 0, sizeof(stats_t));

//	weapon_machinegun.load(gfx, "media/models/weapons2/m4/m4s");
	weapon_machinegun.load(gfx, "media/models/weapons2/machinegun/machinegun");
	weapon_shotgun.load(gfx,	"media/models/weapons2/shotgun/shotgun");
	weapon_grenade.load(gfx,	"media/models/weapons2/grenadel/grenade");
	weapon_rocket.load(gfx,		"media/models/weapons2/rocketl/rocketl");
	weapon_lightning.load(gfx,	"media/models/weapons2/lightning/lightning");
	weapon_railgun.load(gfx,	"media/models/weapons2/railgun/railgun");
	weapon_plasma.load(gfx,		"media/models/weapons2/plasma/plasma");


	entity->speaker = new Speaker(entity, audio);
	strcpy(entity->speaker->file, "info_player_deathmatch");
#ifndef __OBJC__
    alSourcef(entity->speaker->source, AL_GAIN, 4.0f);
#endif
	audio.effects(entity->speaker->source);

	footstep_source = audio.create_source(false, true);
#ifndef __OBJC__
	alSourcef(footstep_source, AL_GAIN, 4.0f);
#endif
	audio.effects(footstep_source);


	//	weapon_model.center = entity->rigid->center;

	current_light = 0;
	current_face = 0;
	bot_state = BOT_IDLE;

	path.length = 0;
	path.step = 0;
}

void Player::load_sounds(Audio &audio, std::vector<wave_t> &snd_wave)
{
	wave_t wave;

	//load player sounds
	strcpy(wave.file, "sound/weapons/railgun/rg_hum.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/weapons/lightning/lg_hum.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/weapons/shotgun/sshotf1b.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/weapons/railgun/railgf1a.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/weapons/lightning/lg_fire.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/weapons/rocket/rocklf1a.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/weapons/grenade/grenlf1a.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/weapons/plasma/hyprbf1a.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/weapons/machinegun/machgf1b.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/world/telein.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/world/teleout.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);


	for (unsigned int i = 0; i < 23; i++)
	{
		sprintf(wave.file, "sound/player/%s/death1.wav", models[i]);
		audio.load(wave);
		if (wave.data != NULL)
			snd_wave.push_back(wave);

		sprintf(wave.file, "sound/player/%s/death2.wav", models[i]);
		audio.load(wave);
		if (wave.data != NULL)
			snd_wave.push_back(wave);

		sprintf(wave.file, "sound/player/%s/death3.wav", models[i]);
		audio.load(wave);
		if (wave.data != NULL)
			snd_wave.push_back(wave);


		sprintf(wave.file, "sound/player/%s/pain25_1.wav", models[i]);
		audio.load(wave);
		if (wave.data != NULL)
			snd_wave.push_back(wave);

		sprintf(wave.file, "sound/player/%s/pain50_1.wav", models[i]);
		audio.load(wave);
		if (wave.data != NULL)
			snd_wave.push_back(wave);

		sprintf(wave.file, "sound/player/%s/pain75_1.wav", models[i]);
		audio.load(wave);
		if (wave.data != NULL)
			snd_wave.push_back(wave);

		sprintf(wave.file, "sound/player/%s/pain100_1.wav", models[i]);
		audio.load(wave);
		if (wave.data != NULL)
			snd_wave.push_back(wave);

		sprintf(wave.file, "sound/player/%s/jump1.wav", models[i]);
		audio.load(wave);
		if (wave.data != NULL)
			snd_wave.push_back(wave);

		sprintf(wave.file, "sound/player/%s/fall1.wav", models[i]);
		audio.load(wave);
		if (wave.data != NULL)
			snd_wave.push_back(wave);

		sprintf(wave.file, "sound/player/%s/falling1.wav", models[i]);
		audio.load(wave);
		if (wave.data != NULL)
			snd_wave.push_back(wave);
	}

	strcpy(wave.file, "sound/weapons/rocket/rockfly.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/weapons/rocket/rocklx1a.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/weapons/plasma/lasfly.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);


	strcpy(wave.file, "sound/weapons/plasma/plasmx1a.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/weapons/change.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/items/use_medkit.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/items/use_nothing.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/items/regen.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/player/footsteps/step1.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/player/footsteps/step2.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/player/footsteps/step3.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/player/footsteps/step4.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	sprintf(wave.file, "sound/player/land1.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	sprintf(wave.file, "sound/world/jumppad.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);


	strcpy(wave.file, "sound/weapons/noammo.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/player/watr_in.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/player/watr_out.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/player/gurp1.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/player/gurp2.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/player/gibsplt1.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/player/gibimp1.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/player/gibimp2.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/player/gibimp3.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/player/talk.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

}


void Player::respawn()
{
	attack_sound[0] = '\0';
	weapon_idle_sound[0] = '\0';
	health = 100;
	armor = 0;
	weapon_flags = WEAPON_MACHINEGUN;
	current_weapon = wp_machinegun;
	ammo_rockets = 0;
	ammo_slugs = 0;
	ammo_shells = 0;
	ammo_bullets = 50;
	ammo_lightning = 0;
	ammo_plasma = 0;
	ammo_bfg = 0;
	reload_timer = 0;
	click_timer = 0;
	entity->rigid->velocity = vec3(0.0f, 0.0f, 0.0f);
	entity->rigid->net_force = vec3(0.0f, 0.0f, 0.0f);
	state = PLAYER_IDLE;
	bot_state = BOT_IDLE;


	armor = 0;
	flight_timer = 0;
	teleport_timer = 0;
	drown_timer = 0;
	regen_timer = 0;
	haste_timer = 0;
	quad_timer = 0;
	reload_timer = 0;
	invisibility_timer = 0;
	teleport_timer = 0;


//	entity->model->make_aabb();
}

void Player::kill()
{
	weapon_flags = 0;
	reload_timer = 120;
	current_weapon = wp_none;
	state = PLAYER_DEAD;
}

void Player::render_weapon(Graphics &gfx)
{
	switch (current_weapon)
	{
	case wp_none:
		break;
	case wp_machinegun:
		weapon_machinegun.render(gfx);
		break;
	case wp_shotgun:
		weapon_shotgun.render(gfx);
		break;
	case wp_grenade:
		weapon_grenade.render(gfx);
		break;
	case wp_rocket:
		weapon_rocket.render(gfx);
		break;
	case wp_lightning:
		weapon_lightning.render(gfx);
		break;
	case wp_railgun:
		weapon_railgun.render(gfx);
		break;
	case wp_plasma:
		weapon_plasma.render(gfx);
		break;
	}
}

void Player::change_weapon_up()
{
	if (reload_timer != 0)
		return;

	switch (current_weapon)
	{
	case wp_none:
		if (weapon_flags & WEAPON_MACHINEGUN)
			current_weapon = wp_machinegun;
		else if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		else if (weapon_flags & WEAPON_GRENADE)
			current_weapon = wp_grenade;
		else if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		else if (weapon_flags & WEAPON_PLASMA)
			current_weapon = wp_plasma;
		break;
	case wp_machinegun:
		if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		else if (weapon_flags & WEAPON_GRENADE)
			current_weapon = wp_grenade;
		else if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		else if (weapon_flags & WEAPON_PLASMA)
			current_weapon = wp_plasma;
		break;
	case wp_shotgun:
		if (weapon_flags & WEAPON_GRENADE)
			current_weapon = wp_grenade;
		else if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		else if (weapon_flags & WEAPON_PLASMA)
			current_weapon = wp_plasma;
		break;
	case wp_grenade:
		if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		else if (weapon_flags & WEAPON_PLASMA)
			current_weapon = wp_plasma;
		break;
	case wp_rocket:
		if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		else if (weapon_flags & WEAPON_PLASMA)
			current_weapon = wp_plasma;
		break;
	case wp_lightning:
		if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		else if (weapon_flags & WEAPON_PLASMA)
			current_weapon = wp_plasma;
		break;
	case wp_railgun:
		if (weapon_flags & WEAPON_PLASMA)
			current_weapon = wp_plasma;
		break;
	case wp_plasma:
		break;
	}
}

void Player::change_weapon_down()
{
	if (reload_timer != 0)
		return;

	switch (current_weapon)
	{
	case wp_none:
		if (weapon_flags & WEAPON_PLASMA)
			current_weapon = wp_plasma;
		else if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		else if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_GRENADE)
			current_weapon = wp_grenade;
		else if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		else if (weapon_flags & WEAPON_MACHINEGUN)
			current_weapon = wp_machinegun;
		break;
	case wp_machinegun:
		break;
	case wp_shotgun:
		if (weapon_flags & WEAPON_MACHINEGUN)
			current_weapon = wp_machinegun;
		break;
	case wp_grenade:
		if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		else if (weapon_flags & WEAPON_MACHINEGUN)
			current_weapon = wp_machinegun;
		break;
	case wp_rocket:
		if (weapon_flags & WEAPON_GRENADE)
			current_weapon = wp_grenade;
		else if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		else if (weapon_flags & WEAPON_MACHINEGUN)
			current_weapon = wp_machinegun;
		break;
	case wp_lightning:
		if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		else if (weapon_flags & WEAPON_MACHINEGUN)
			current_weapon = wp_machinegun;
		break;
	case wp_railgun:
		if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		else if (weapon_flags & WEAPON_MACHINEGUN)
			current_weapon = wp_machinegun;
		break;
	case wp_plasma:
		if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		else if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		else if (weapon_flags & WEAPON_MACHINEGUN)
			current_weapon = wp_machinegun;
		break;
	}
}


void Player::best_weapon()
{
	if (weapon_flags & wp_railgun && ammo_slugs > 0)
		current_weapon = wp_railgun;
	else if (weapon_flags & wp_rocket  && ammo_rockets > 0)
		current_weapon = wp_rocket;
	else if (weapon_flags & wp_plasma && ammo_plasma > 0)
		current_weapon = wp_plasma;
	else if (weapon_flags & wp_lightning && ammo_lightning > 0)
		current_weapon = wp_lightning;
	else if (weapon_flags & wp_grenade && ammo_grenades > 0)
		current_weapon = wp_grenade;
	else if (weapon_flags & wp_shotgun && ammo_shells > 0)
		current_weapon = wp_shotgun;
	else if (weapon_flags & wp_machinegun && ammo_bullets > 0)
		current_weapon = wp_machinegun;

}


float Player::DistanceToLine(vec3 &direction, vec3 &origin, vec3 &point)
{
	return vec3::crossproduct(direction, point - origin).magnitude();
}

int Player::FindLookAt(vec3 &cameraOrigin, vec3 &cameraDir, vec3 *points, int numPoints)
{
	int index = -1;
	float min = FLT_MAX;

	for (int i = 0; i < numPoints; i++)
	{
		float distance = DistanceToLine(cameraDir, cameraOrigin, points[i]);

		if (distance < min)
		{
			min = distance;
			index = i;
		}
	}
	return index;
}


Player::~Player()
{
	delete[] path.path;
}


void Player::avoid_walls(Bsp &map)
{
	Frame frame;
	vec3 rightv;

	entity->model->get_frame(frame);

	vec3 forward = entity->position + frame.forward * 100.0f;
	vec3 backward = entity->position + frame.forward * -100.0f;

	rightv = vec3::crossproduct(frame.up, forward);

	vec3 right = entity->position + rightv * 100.0f;
	vec3 left = entity->position + rightv * -100.0f;

	vec3 down = entity->position + frame.up * -100.0f + frame.forward * 25.0f;


	plane_t plane;
	float depth;
	bool water;
	float water_depth;
	vector<surface_t *> surface_list;
	vec3 clip;
	vec3 vel;

	input_t input;

	float speed_scale = 1.0f;

	if (haste_timer > 0)
		speed_scale = 2.0f;


	map.collision_detect(forward, entity->position, &plane, &depth, water, water_depth, surface_list, false, clip, vel);
	if ( depth < 0 )
	{
		input.moveright = true;
	}

	map.collision_detect(backward, entity->position, &plane, &depth, water, water_depth, surface_list, false, clip, vel);
	if (depth < 0)
	{
		input.moveleft = true;
	}

	map.collision_detect(left, entity->position, &plane, &depth, water, water_depth, surface_list, false, clip, vel);
	if (depth < 0)
	{
		input.moveup = true;
	}

	map.collision_detect(right, entity->position, &plane, &depth, water, water_depth, surface_list, false, clip, vel);
	if (depth < 0)
	{
		input.movedown = true;
	}

	map.collision_detect(down, entity->position, &plane, &depth, water, water_depth, surface_list, false, clip, vel);
	if (depth > 0)
	{
		input.jump = true;
	}

	entity->rigid->move(input, speed_scale);

}

void Player::handle_bot(vector<Entity *> &entity_list, int self)
{
	for (unsigned int i = 0; i < entity_list.size(); i++)
	{
		if (i == (unsigned int)self)
			continue;

		if (strcmp(entity_list[i]->type, "player") == 0)
		{
			float distance = (entity_list[i]->position - entity->position).magnitude();

			if (distance < 500.0f)
			{
				entity->rigid->lookat(entity_list[i]->position);
				bot_state = BOT_ALERT;
			}
			else
			{
				if (bot_state == BOT_ALERT)
					bot_state = BOT_IDLE;
			}

			if (distance < 400.0f)
			{
				if (reload_timer <= 0 && entity_list[i]->player->state != PLAYER_DEAD)
				{
					entity->rigid->lookat(entity_list[i]->position);
					bot_state = BOT_ATTACK;
				}

				if (entity_list[i]->player->state == PLAYER_DEAD)
				{
					bot_state = BOT_IDLE;
				}
			}
			else
			{
				if (bot_state == BOT_ATTACK)
					bot_state = BOT_ALERT;
			}

			continue;
		}
	}
}


//#define DEBUG_BOT
int Player::bot_search_for_items(vector<Entity *> &entity_list, int self)
{
#ifdef DEBUG_BOT
	printf("handle_bot() ignore %s\n", ignore);
#endif

	best_weapon();

//	entity_list[self]->rigid->move_forward();
//	return;

#ifdef DEBUG_BOT
	printf("bot_search_for_items() state is %d\n", bot_state);
#endif

	bool need_health =			(health < 100);
	bool need_armor =			(armor < 100);
	bool need_shotgun =			((weapon_flags & wp_shotgun) == 0);
	bool need_rocketlauncher =	((weapon_flags & wp_shotgun) == 0);
	bool need_lightning =		((weapon_flags & wp_shotgun) == 0);
	bool need_railgun =			((weapon_flags & wp_shotgun) == 0);
	bool need_plasma =			((weapon_flags & wp_shotgun) == 0);
	bool need_grenadelauncher = ((weapon_flags & wp_shotgun) == 0);
	bool need_ammo =			(current_weapon == wp_machinegun);

	if (last_state == BOT_IDLE)
	{
		need_ammo = true;
	}

	for (unsigned int i = 0; i < entity_list.size(); i++)
	{
		if (i == (unsigned int)self)
			continue;


		if (strlen(ignore) >= 1)
		{
			if (strstr(ignore, entity_list[i]->type) != NULL)
				continue;
		}

		last_state = bot_state;

		if (entity_list[i]->trigger == NULL)
			continue;

		if (entity_list[i]->trigger->active == true)
			continue;

		/*
		// Idea is to prefer nearby items without doing expense of distance check first
		if (last_state != BOT_IDLE && last_state != BOT_GET_ITEM && last_state != BOT_EXPLORE &&
			entity_list[i]->bsp_leaf != entity_list[self]->bsp_leaf)
			continue;
			*/

		float distance = (entity_list[i]->position - entity->position).magnitude();

		// Skip far away items unless we hit explore state (in which case nothing good nearby)
		if (distance > 1500.0f && bot_state != BOT_EXPLORE)
			continue;

#ifdef DEBUG_BOT
		printf("Bot is near entity %d type %s distance %3.3f\n", i, entity_list[i]->type, distance);
#endif
		if (strcmp(entity_list[i]->type, "item_quad") == 0)
		{
//			printf("bot %d wants %s\n", i, entity_list[i]->type);
			bot_state = BOT_GET_ITEM;
			return i;
		}
		else if (strcmp(entity_list[i]->type, "item_health_mega") == 0)
		{
//			printf("bot %d wants %s\n", i, entity_list[i]->type);
			bot_state = BOT_GET_ITEM;
			return i;
		}
		else if (need_health)
		{
			if (strcmp(entity_list[i]->type, "item_health_large") == 0)
			{
//				printf("bot %d wants %s\n", i, entity_list[i]->type);
				bot_state = BOT_GET_ITEM;
				return i;
			}
			else if (strcmp(entity_list[i]->type, "item_health_large") == 0)
			{
//				printf("bot %d wants %s\n", i, entity_list[i]->type);
				bot_state = BOT_GET_ITEM;
				return i;
			}
		}
		else if ((strcmp(entity_list[i]->type, "weapon_rocketlauncher") == 0) && need_rocketlauncher)
		{
//			printf("bot %d wants %s\n", i, entity_list[i]->type);
			bot_state = BOT_GET_ITEM;
			return i;
		}
		else if ((strcmp(entity_list[i]->type, "weapon_lightning") == 0) && need_lightning)
		{
//			printf("bot %d wants %s\n", i, entity_list[i]->type);
			bot_state = BOT_GET_ITEM;
			return i;
		}
		else if ((strcmp(entity_list[i]->type, "weapon_railgun") == 0) && need_railgun)
		{
//			printf("bot %d wants %s\n", i, entity_list[i]->type);
			bot_state = BOT_GET_ITEM;
			return i;
		}
		else if ((strcmp(entity_list[i]->type, "weapon_plasma") == 0) && need_plasma)
		{
//			printf("bot %d wants %s\n", i, entity_list[i]->type);
			bot_state = BOT_GET_ITEM;
			return i;
		}
		else if ((strcmp(entity_list[i]->type, "weapon_grenadelauncher") == 0) && need_grenadelauncher)
		{
//			printf("bot %d wants %s\n", i, entity_list[i]->type);
			bot_state = BOT_GET_ITEM;
			return i;
		}
		else if ((strcmp(entity_list[i]->type, "weapon_shotgun") == 0) && need_shotgun)
		{
//			printf("bot %d wants %s\n", i, entity_list[i]->type);
			bot_state = BOT_GET_ITEM;
			return i;
		}
		else if (need_armor)
		{
			if (strcmp(entity_list[i]->type, "item_armor_body") == 0)
			{
//				printf("bot %d wants %s\n", i, entity_list[i]->type);
				bot_state = BOT_GET_ITEM;
				return i;
			}
			else if (strcmp(entity_list[i]->type, "item_armor_combat") == 0)
			{
//				printf("bot %d wants %s\n", i, entity_list[i]->type);
				bot_state = BOT_GET_ITEM;
				return i;
			}
			else if (strcmp(entity_list[i]->type, "item_armor_shard") == 0)
			{
//				printf("bot %d wants %s\n", i, entity_list[i]->type);
				bot_state = BOT_GET_ITEM;
				return i;
			}
		}
		else if (strcmp(entity_list[i]->type, "trigger_teleport") == 0)
		{
//			printf("bot %d wants %s\n", i, entity_list[i]->type);
			bot_state = BOT_GET_ITEM;
			return i;
		}

		if ((strstr(entity_list[i]->type, "ammo_") != NULL) && need_ammo)
		{
//			printf("bot %d wants %s\n", i, entity_list[i]->type);
			bot_state = BOT_GET_ITEM;
			return i;
		}
		last_state = bot_state;

	}

	// Exhausted nearby items
	if (bot_state == BOT_IDLE)
		bot_state = BOT_EXPLORE;
	return -1;
}
