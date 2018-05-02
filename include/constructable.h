#include "include.h"

#ifndef CONSTRUCTABLE_H
#define CONSTRUCTABLE_H

#define SENTRY_HEALTH_LVL1 150
#define SENTRY_HEALTH_LVL2 180
#define SENTRY_HEALTH_LVL3 220


// sentry guns,
// enemy terrirotry constructable bridges / brushes (change opacity on % complete)
// fixed mounted guns enemy territory style
// Other things?

class Engine;

class Constructable
{
public:
	Constructable(Entity *entity, Graphics &gfx, Audio &audio, team_t team);
	~Constructable();
	Entity	*entity;

	void reset();
	int step(input_t &input, vector<Entity *> &entity_list, int self, Engine &engine);
	void destroy();

	playertype_t type;

	int health;
	int level;


	unsigned short int armor;
	unsigned char weapon_flags;

	unsigned short int current_weapon;
	unsigned short int last_weapon;
	unsigned short int ammo_rockets;
	unsigned short int ammo_grenades;
	unsigned short int ammo_slugs;
	unsigned short int ammo_shells;
	unsigned short int ammo_bullets;
	unsigned short int ammo_lightning;
	unsigned short int ammo_plasma;
	unsigned short int ammo_bfg;
	Entity *telefragged;

	unsigned short int reload_timer;
	unsigned short int reload_timer2;
	unsigned short int click_timer;
	unsigned short int pain_timer;
	unsigned short int alert_timer;
	unsigned short int idle_timer;
	unsigned short int dead_timer;
	unsigned short int alive_timer;
	unsigned short int build_timer;


	unsigned int current_light; // debugging light sources, selected same as weapon
	unsigned int current_face; // debugging light sources, selected same as weapon

	unsigned short int flash_gauntlet;
	unsigned short int flash_machinegun;
	unsigned short int flash_shotgun;
	unsigned short int flash_grenade;
	unsigned short int flash_rocket;
	unsigned short int flash_railgun;
	unsigned short int flash_lightning;
	unsigned short int flash_plasma;

	int weapon_source;
	int weapon_loop_source;

	char name[128];
	stats_t stats;
	bool spawned;
	bool render_md5;
	bool immobile;

	bot_state_t bot_state;
	bot_state_t last_state;
	player_state_t state;

	team_t team;
	int owner;
	int base_index; // index of sentry base for deletion

private:
};

#endif
