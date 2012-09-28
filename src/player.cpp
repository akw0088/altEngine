#include "player.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Player::Player(Entity *entity, Graphics &gfx)
: weapon_model(entity)
{
	Player::entity = entity;
	health = 100;
	armor = 0;
	weapon = 3;
	current_weapon = 1;
	ammo_rockets = 0;
	ammo_slugs = 0;
	ammo_shells = 0;
	ammo_bullets = 50;
	ammo_lightning = 0;
	ammo_plasma = 0;
	ammo_bfg = 0;

	weapon_model.load(gfx, "media/models/weapons2/rocketl/rocketl");
//	weapon_model.center = entity->rigid->center;
}

Player::~Player()
{
}