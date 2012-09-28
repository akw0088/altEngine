#include "include.h"

#ifndef PLAYER_H
#define PLAYER_H

#define WEAPON_MELEE			1
#define WEAPON_MACHINEGUN		2
#define WEAPON_SHOTGUN			4
#define WEAPON_PLASMAGUN		8
#define WEAPON_ROCKET			16
#define WEAPON_RAILGUN			32
#define WEAPON_BFG				64
#define WEAPON_LIGHTNING		128

class Player
{
public:
	Player(Entity *entity, Graphics &gfx);
	~Player();
	Entity	*entity;

	int health;
	int armor;
	int	weapon;

	int current_weapon;
	int ammo_rockets;
	int ammo_slugs;
	int ammo_shells;
	int ammo_bullets;
	int ammo_lightning;
	int ammo_plasma;
	int ammo_bfg;

	Model weapon_model;
};

#endif
