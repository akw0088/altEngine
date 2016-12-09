#include "include.h"

#ifndef PLAYER_H
#define PLAYER_H

#define WEAPON_NONE				0
#define WEAPON_MELEE			1
#define WEAPON_MACHINEGUN		2
#define WEAPON_SHOTGUN			4
#define WEAPON_GRENADE			8
#define WEAPON_ROCKET			16
#define WEAPON_RAILGUN			32
#define WEAPON_LIGHTNING		64
#define WEAPON_PLASMA			128


//multiple types exist because I forgot above code existed D:
enum wp_weapon {
	wp_none = WEAPON_NONE,
	wp_melee = WEAPON_MELEE,
	wp_machinegun = WEAPON_MACHINEGUN,
	wp_shotgun = WEAPON_SHOTGUN,
	wp_grenade = WEAPON_GRENADE,
	wp_rocket = WEAPON_ROCKET,
	wp_railgun = WEAPON_RAILGUN,
	wp_lightning = WEAPON_LIGHTNING,
	wp_plasma = WEAPON_PLASMA
};

typedef struct
{
	int kills;
	int deaths;
	float accuracy;
} stats_t;


class Player
{
public:
	Player(Entity *entity, Graphics &gfx, Audio &audio);
	~Player();
	Entity	*entity;

	void respawn();
	void kill();
	void render_weapon(Graphics &gfx);
	void change_weapon_up();
	void change_weapon_down();


	int FindLookAt(vec3 &cameraOrigin, vec3 &cameraDir, vec3 *points, int numPoints);
	float DistanceToLine(vec3 &direction, vec3 &origin, vec3 &point);

	char *attack_sound;
	char *weapon_idle_sound;
	char *death1_sound;
	char *death2_sound;
	char *death3_sound;

	char *pain25_sound;
	char *pain50_sound;
	char *pain75_sound;
	char *pain100_sound;

	char *jump_sound;
	char *land_sound;
	char *weapon_swap_sound;

	char *step1_sound;
	char *step2_sound;
	char *step3_sound;
	char *step4_sound;

	int health;

	unsigned int armor;
	unsigned int weapon_flags;

	unsigned int current_weapon;
	unsigned int last_weapon;
	unsigned int ammo_rockets;
	unsigned int ammo_grenades;
	unsigned int ammo_slugs;
	unsigned int ammo_shells;
	unsigned int ammo_bullets;
	unsigned int ammo_lightning;
	unsigned int ammo_plasma;
	unsigned int ammo_bfg;

	unsigned int reload_timer;
	unsigned int current_light; // debugging light sources, selected same as weapon
	unsigned int current_face; // debugging light sources, selected same as weapon


	char name[128];
	stats_t stats;
	bool dead;

private:

	Model weapon_machinegun;
	Model weapon_shotgun;
	Model weapon_rocket;
	Model weapon_grenade;
	Model weapon_lightning;
	Model weapon_railgun;
	Model weapon_plasma;
};

#endif
