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
typedef enum {
	wp_none = WEAPON_NONE,
	wp_melee = WEAPON_MELEE,
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
	BOT_ALERT,
	BOT_GET_ITEM,
	BOT_ATTACK
} bot_state_t;


typedef struct
{
	int kills;
	int deaths;
	float accuracy;
} stats_t;


class Player
{
public:
	Player(Entity *entity, Graphics &gfx, Audio &audio, int model);
	~Player();
	Entity	*entity;


	static void load_sounds(Audio &audio, std::vector<wave_t> &snd_wave);

	void respawn();
	void kill();
	void render_weapon(Graphics &gfx);
	void change_weapon_up();
	void change_weapon_down();
	void best_weapon();

	void bot_search_for_items(vector<Entity *> &entity_list, int self);


	int FindLookAt(vec3 &cameraOrigin, vec3 &cameraDir, vec3 *points, int numPoints);
	float DistanceToLine(vec3 &direction, vec3 &origin, vec3 &point);

	char attack_sound[128];
	char empty_sound[128];
	char weapon_idle_sound[128];
	char death1_sound[128];
	char death2_sound[128];
	char death3_sound[128];

	char pain25_sound[128];
	char pain50_sound[128];
	char pain75_sound[128];
	char pain100_sound[128];

	char jump_sound[128];
	char pad_sound[128];
	char pit_sound[128];
	char fall_sound[128];
	char land_sound[128];
	char weapon_swap_sound[128];

	char step1_sound[128];
	char step2_sound[128];
	char step3_sound[128];
	char step4_sound[128];

	char waterin_sound[128];
	char waterout_sound[128];

	char telein_sound[128];
	char teleout_sound[128];

	char gurp1_sound[128];
	char gurp2_sound[128];

	char gibbed_sound[128];
	char gibimpact1_sound[128];
	char gibimpact2_sound[128];
	char gibimpact3_sound[128];

	char chat_sound[128];
	char model_name[128];
	int model_index;

	int health;
	unsigned int footstep_source;

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
	unsigned int drown_timer;
	unsigned int flight_timer;
	unsigned int teleport_timer; // prevent teleporting out and back rapidly
	unsigned int current_light; // debugging light sources, selected same as weapon
	unsigned int current_face; // debugging light sources, selected same as weapon


	char name[128];
	stats_t stats;
	bool dead;
	bool spawned;

	bot_state_t bot_state;


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
