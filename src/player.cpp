#include "player.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include <float.h>

Player::Player(Entity *entity, Graphics &gfx, Audio &audio)
: weapon_shotgun(entity), weapon_rocket(entity), weapon_lightning(entity), weapon_railgun(entity)
{
	Player::entity = entity;

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
	memcpy(name, "UnnamedPlayer", strlen("UnnamedPlayer") + 1);
	memset(&stats, 0, sizeof(stats_t));
	dead = false;

	weapon_rocket.load(gfx,		"media/models/weapons2/rocketl/rocketl");
	weapon_lightning.load(gfx,	"media/models/weapons2/lightning/lightning");
	weapon_railgun.load(gfx,	"media/models/weapons2/railgun/railgun");
	weapon_shotgun.load(gfx,	"media/models/weapons2/shotgun/shotgun");


	entity->speaker = new Speaker(entity);
	strcpy(entity->speaker->file, "info_player_deathmatch");
	entity->speaker->loop = false;
	entity->speaker->source = audio.create_source(entity->speaker->loop, false);
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
	dead = false;


//	entity->model->make_aabb();
}

void Player::kill()
{
	entity->model->aabb[0] = vec3(-10.0f, -10.0f, -10.0f);
	entity->model->aabb[1] = vec3(-10.0f, -10.0f, 10.0f);
	entity->model->aabb[2] = vec3(-10.0f, 10.0f, 0.0f);
	entity->model->aabb[3] = vec3(-10.0f, 10.0f, 10.0f);
	entity->model->aabb[4] = vec3(10.0f, -10.0f, -10.0f);
	entity->model->aabb[5] = vec3(10.0f, -10.0f, 10.0f);
	entity->model->aabb[6] = vec3(10.0f, 10.0f, -10.0f);
	entity->model->aabb[7] = vec3(10.0f, 10.0f, 10.0f);
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
	case wp_shotgun:
		weapon_shotgun.render(gfx);
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
	}
}

void Player::change_weapon_up()
{
	switch (current_weapon)
	{
	case wp_none:
		if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		else if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		break;
	case wp_shotgun:
		if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		break;
	case wp_rocket:
		if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		break;
	case wp_lightning:
		if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		break;
	case wp_railgun:
		break;
	}
}

void Player::change_weapon_down()
{
	switch (current_weapon)
	{
	case wp_none:
		if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		else if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		break;
	case wp_shotgun:
		break;
	case wp_rocket:
		if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		break;
	case wp_lightning:
		if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		break;
	case wp_railgun:
		if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
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
