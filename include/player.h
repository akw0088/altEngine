#include "include.h"

#ifndef PLAYER_H
#define PLAYER_H

#define WEAPON_NONE				0
#define WEAPON_MELEE			1
#define WEAPON_MACHINEGUN		2
#define WEAPON_SHOTGUN			4
#define WEAPON_PLASMAGUN		8
#define WEAPON_ROCKET			16
#define WEAPON_RAILGUN			32
#define WEAPON_BFG				64
#define WEAPON_LIGHTNING		128


//multiple types exist because I forgot above code existed D:
enum wp_weapon {
	wp_none = WEAPON_NONE,
	wp_shotgun = WEAPON_SHOTGUN,
	wp_rocket = WEAPON_ROCKET,
	wp_lightning = WEAPON_LIGHTNING,
	wp_railgun = WEAPON_RAILGUN
};


class Player
{
public:
	Player(Entity *entity, Graphics &gfx, Audio &audio);
	~Player();
	Entity	*entity;

	void render_weapon(Graphics &gfx);
	void change_weapon_up();
	void change_weapon_down();


	int FindLookAt(vec3 &cameraOrigin, vec3 &cameraDir, vec3 *points, int numPoints);
	float DistanceToLine(vec3 &direction, vec3 &origin, vec3 &point);

	char *attack_sound;
	char *weapon_idle_sound;
	int health;
	int armor;
	int	weapon_flags;

	int current_weapon;
	int ammo_rockets;
	int ammo_slugs;
	int ammo_shells;
	int ammo_bullets;
	int ammo_lightning;
	int ammo_plasma;
	int ammo_bfg;

	int reload_timer;
	int current_light; // debugging light sources, selected same as weapon

private:
	Model weapon_shotgun;
	Model weapon_rocket;
	Model weapon_lightning;
	Model weapon_railgun;
};

#endif
