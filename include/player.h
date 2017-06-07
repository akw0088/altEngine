#include "include.h"

#ifndef PLAYER_H
#define PLAYER_H

#define WEAPON_NONE				0
#define WEAPON_GAUNTLET			1
#define WEAPON_MACHINEGUN		2
#define WEAPON_SHOTGUN			4
#define WEAPON_GRENADE			8
#define WEAPON_ROCKET			16
#define WEAPON_RAILGUN			32
#define WEAPON_LIGHTNING		64
#define WEAPON_PLASMA			128


//multiple types exist because I forgot above code existed D:
typedef enum {
	wp_none = WEAPON_NONE,
	wp_gauntlet = WEAPON_GAUNTLET,
	wp_machinegun = WEAPON_MACHINEGUN,
	wp_shotgun = WEAPON_SHOTGUN,
	wp_grenade = WEAPON_GRENADE,
	wp_rocket = WEAPON_ROCKET,
	wp_railgun = WEAPON_RAILGUN,
	wp_lightning = WEAPON_LIGHTNING,
	wp_plasma = WEAPON_PLASMA
} weapon_t;

typedef enum {
	BOT_IDLE,
	BOT_ALERT,		// 1
	BOT_ATTACK,		// 2
	BOT_GET_ITEM,	// 3
	BOT_EXPLORE,	// 4
	BOT_DEAD		// 5
} bot_state_t;




// Seems like a good idea to handle animations
typedef enum {
	PLAYER_IDLE,
	PLAYER_MOVED,
	PLAYER_JUMPED,
	PLAYER_DUCKED,
	PLAYER_ATTACK,
	PLAYER_DEAD
} player_state_t;


typedef struct
{
	int kills;
	int deaths;
	int hits;
	int shots;
	int medal_impressive;
	int medal_excellent;
	int medal_frags;
	int medal_humiliation;
} stats_t;


typedef enum
{
	UNKNOWN,
	PLAYER,
	BOT,
	CLIENT,
	SERVER,
	SPECTATOR
} playertype_t;

class Player
{
public:
	Player(Entity *entity, Graphics &gfx, Audio &audio, int model, team_t team);
	~Player();
	Entity	*entity;


	void respawn();
	void reset();
	void kill();
	void render_weapon(Graphics &gfx);
	void change_weapon_up();
	void change_weapon_down();
	void best_weapon();

	void handle_bot(vector<Entity *> &entity_list, int self);
	int bot_search_for_items(vector<Entity *> &entity_list, int self);
	void avoid_walls(Bsp &map);


	int FindLookAt(vec3 &cameraOrigin, vec3 &cameraDir, vec3 *points, int numPoints);
	float DistanceToLine(vec3 &direction, vec3 &origin, vec3 &point);

	char model_name[128];
	int model_index;

	playertype_t type;

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
	bool holdable_teleporter;
	bool holdable_medikit;
	bool holdable_flag;
	Entity *telefragged;

	unsigned int reload_timer;
	unsigned int drown_timer;
	unsigned int flight_timer;
	unsigned int regen_timer;
	unsigned int haste_timer;
	unsigned int quad_timer;
	unsigned int click_timer;
	unsigned int invisibility_timer;
	unsigned int jumppad_timer;
	unsigned int teleport_timer; // prevent teleporting out and back rapidly
	unsigned int excellent_timer;
	unsigned int impressive_count;
	unsigned int impressive_award_timer;
	unsigned int excellent_award_timer;
	unsigned int pain_timer;
	unsigned int alert_timer;
	unsigned int idle_timer;



	unsigned int current_light; // debugging light sources, selected same as weapon
	unsigned int current_face; // debugging light sources, selected same as weapon

	unsigned int flash_gauntlet;
	unsigned int flash_machinegun;
	unsigned int flash_shotgun;
	unsigned int flash_grenade;
	unsigned int flash_rocket;
	unsigned int flash_railgun;
	unsigned int flash_lightning;
	unsigned int flash_plasma;



	bool godmode;
	bool local;
	bool quad_damage;


	path_t path;

	char name[128];
	stats_t stats;
	bool spawned;

	bot_state_t bot_state;
	bot_state_t last_state;
	player_state_t state;
	int get_item;
	char ignore[1024];

	float zoom_level;
	team_t team;

	Model weapon_gauntlet;
	Model weapon_machinegun;
	Model weapon_shotgun;
	Model weapon_grenade;
	Model weapon_rocket;
	Model weapon_lightning;
	Model weapon_railgun;
	Model weapon_plasma;
private:
};

#endif
