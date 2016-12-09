#include "player.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include <float.h>

Player::Player(Entity *entity, Graphics &gfx, Audio &audio)
: weapon_machinegun(entity), weapon_shotgun(entity), weapon_grenade(entity), weapon_rocket(entity),
  weapon_lightning(entity), weapon_railgun(entity), weapon_plasma(entity)
{
	Player::entity = entity;

	attack_sound = "";
	weapon_idle_sound = "";
	death1_sound = "sound/player/ranger/death1.wav";
	death2_sound = "sound/player/ranger/death2.wav";
	death3_sound = "sound/player/ranger/death3.wav";

	pain25_sound = "sound/player/ranger/pain25_1.wav";
	pain50_sound = "sound/player/ranger/pain50_1.wav";
	pain75_sound = "sound/player/ranger/pain75_1.wav";
	pain100_sound = "sound/player/ranger/pain100_1.wav";

	jump_sound = "sound/player/ranger/jump1.wav";
	land_sound = "sound/player/ranger/fall1.wav";
	weapon_swap_sound = "sound/weapons/change.wav";

	step1_sound = "sound/player/footsteps/step1.wav";
	step2_sound = "sound/player/footsteps/step2.wav";
	step3_sound = "sound/player/footsteps/step3.wav";
	step4_sound = "sound/player/footsteps/step4.wav";

	//sounds/player/watr_in.wav
	//sounds/player/watr_out.wav
	//sounds/player/watr_un.wav
	//sounds/player/gurp1.wav
	//sounds/player/gurp2.wav
	//sounds/player/talk.wav

	//sound/player/gibsplt1.wav
	//sound/player/gibimp1.wav
	//sound/player/gibimp2.wav
	//sound/player/gibimp3.wav
	//sound/player/fry.wav

	/*
	// Player sound sets
	anarki
	biker
	bitterman
	bones
	crash
	doom
	grunt
	hunter
	keel
	klesk
	lucy
	major
	mynx
	orbb
	ranger
	razor
	sarge
	slash
	sorlag
	tankjr
	uriel
	visor
	*/

	health = 100;
	armor = 0;
	weapon_flags = 0;
	current_weapon = wp_none;
	ammo_rockets = 0;
	ammo_grenades = 0;
	ammo_slugs = 0;
	ammo_shells = 0;
	ammo_bullets = 50;
	ammo_lightning = 0;
	ammo_plasma = 0;
	ammo_bfg = 0;
	reload_timer = 0;
	memcpy(name, "UnnamedPlayer", strlen("UnnamedPlayer") + 1);
	memset(&stats, 0, sizeof(stats_t));
	dead = false;

	weapon_machinegun.load(gfx, "media/models/machinegun/machinegun");
	weapon_shotgun.load(gfx, "media/models/weapons2/shotgun/shotgun");
	weapon_grenade.load(gfx, "media/models/grenade/grenade");
	weapon_rocket.load(gfx,		"media/models/weapons2/rocketl/rocketl");
	weapon_lightning.load(gfx,	"media/models/weapons2/lightning/lightning");
	weapon_railgun.load(gfx,	"media/models/weapons2/railgun/railgun");
	weapon_plasma.load(gfx, "media/models/plasma/plasma");


	entity->speaker = new Speaker(entity, audio);
	strcpy(entity->speaker->file, "info_player_deathmatch");
#ifndef __OBJC__
    alSourcef(entity->speaker->source, AL_GAIN, 4.0f);
#endif
	audio.effects(entity->speaker->source);

	//	weapon_model.center = entity->rigid->center;

	current_light = 0;
	current_face = 0;
}


void Player::respawn()
{
	attack_sound = "";
	weapon_idle_sound = "";
	health = 100;
	armor = 0;
	weapon_flags = 0;
	current_weapon = wp_none;
	ammo_rockets = 0;
	ammo_slugs = 0;
	ammo_shells = 0;
	ammo_bullets = 50;
	ammo_lightning = 0;
	ammo_plasma = 0;
	ammo_bfg = 0;
	reload_timer = 0;
	entity->rigid->velocity = vec3(0.0f, 0.0f, 0.0f);
	entity->rigid->net_force = vec3(0.0f, 0.0f, 0.0f);
	dead = false;


//	entity->model->make_aabb();
}

void Player::kill()
{
	stats.deaths++;
	weapon_flags = 0;
	reload_timer = 120;
	current_weapon = wp_none;
	dead = true;
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
}
