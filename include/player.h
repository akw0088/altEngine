//=============================================================================
// This file is part of the altEngine distribution
// (https://github.com/akw0088/altEngine/)
// Copyright (c) 2018 Alexander Wright All Rights Reserved.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON - INFRINGEMENT.IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//=============================================================================

#include "include.h"

#ifndef PLAYER_H
#define PLAYER_H

class Player
{
public:
	Player(Entity *entity, Graphics &gfx, Audio &audio, int model, team_t team, entity_type_t ent_type, Model *model_table);
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
	bool holdable_teleporter;
	bool holdable_medikit;
	bool holdable_flag;
	Entity *telefragged;

	unsigned short int reload_timer;
	unsigned short int fall_timer;
	unsigned short int drown_timer;
	unsigned short int flight_timer;
	unsigned short int regen_timer;
	unsigned short int haste_timer;
	unsigned short int quad_timer;
	unsigned short int click_timer;
	unsigned short int invisibility_timer;
	unsigned short int jumppad_timer;
	unsigned short int teleport_timer; // prevent teleporting out and back rapidly
	unsigned short int excellent_timer;
	unsigned short int impressive_count;
	unsigned short int impressive_award_timer;
	unsigned short int excellent_award_timer;
	unsigned short int gauntlet_award_timer;
	unsigned short int pain_timer;
	unsigned short int alert_timer;
	unsigned short int idle_timer;
	unsigned short int dead_timer;
	unsigned short int alive_timer;
	unsigned short int build_timer;
	unsigned short int build_type;



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


	int num_sentry;

	float pm_friction;
	float pm_waterfriction;
	float pm_flightfriction;
	float pm_spectatorfriction;

	float pm_accel;
	float pm_airaccel;
	float pm_wateraccel;
	float pm_flyaccel;

	float pm_max_speed;
	float pm_max_air_speed;
	float pm_air_control;
	float haste_factor;


	int weapon_source;
	int weapon_loop_source;



	bool godmode;
	bool local;
	bool quad_damage;
	bool falling;


	path_t path;

	char name[128];
	stats_t stats;
	bool spawned;
	bool immobile;

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


	static const char bot_state_name[16][32];
	static const char *models[23];

	bool done_transform;
	vec3 done_pos;
	int in_vehicle;
	int seat;

private:
};

#endif
