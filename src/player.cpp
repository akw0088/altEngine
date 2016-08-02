#include "player.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Player::Player(Entity *entity, Graphics &gfx)
: weapon_rocket(entity), weapon_shotgun(entity), weapon_lightning(entity), weapon_railgun(entity)
{
	Player::entity = entity;
	health = 100;
	armor = 0;
	weapon = 0;
	current_weapon = wp_none;
	ammo_rockets = 0;
	ammo_slugs = 0;
	ammo_shells = 0;
	ammo_bullets = 50;
	ammo_lightning = 0;
	ammo_plasma = 0;
	ammo_bfg = 0;

	weapon_rocket.load(gfx,		"media/models/weapons2/rocketl/rocketl");
	weapon_lightning.load(gfx,	"media/models/weapons2/lightning/lightning");
	weapon_railgun.load(gfx,	"media/models/weapons2/railgun/railgun");
	weapon_shotgun.load(gfx,	"media/models/weapons2/shotgun/shotgun");
	//	weapon_model.center = entity->rigid->center;
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
		if (weapon & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		else if (weapon & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		break;
	case wp_shotgun:
		if (weapon & WEAPON_SHOTGUN)
			current_weapon = wp_rocket;
		else if (weapon & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		break;
	case wp_rocket:
		if (weapon & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		break;
	case wp_lightning:
		if (weapon & WEAPON_RAILGUN)
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
		if (weapon & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		else if (weapon & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		break;
	case wp_shotgun:
		break;
	case wp_rocket:
		if (weapon & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		break;
	case wp_lightning:
		if (weapon & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		break;
	case wp_railgun:
		if (weapon & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		break;
	}
}

Player::~Player()
{
}