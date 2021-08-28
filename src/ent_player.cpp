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

#include "ent_player.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include <float.h>
#include "quake3.h"

const char EntPlayer::bot_state_name[16][32] = {
	"BOT_IDLE",
	"BOT_ALERT",
	"BOT_ATTACK",
	"BOT_GET_ITEM",
	"BOT_EXPLORE",
	"BOT_DEAD",
};

const char *EntPlayer::models[23] = {
	"anarki",			//0
	"biker",			//1
	"bitterman",		//2
	"bones",			//3
	"crash",			//4
	"doom",				//5
	"grunt",			//6
	"hunter",			//7
	"keel",				//8
	"klesk",			//9
	"lucy",				//10
	"major",			//11
	"mynx",				//12
	"orbb",				//13
	"ranger",			//14
	"razor",			//15
	"sarge",			//16
	"slash",			//17
	"sorlag",			//18
	"tankjr",			//19
	"uriel",			//20
	"visor",			//21
	"xaero"				//22
};



#define NUM_PATH 64 // Should equal map navpoint count

EntPlayer::EntPlayer(Entity *entity, Graphics &gfx, Audio &audio, int model, team_t team, entity_type_t ent_type, EntModel *model_table)
: weapon_gauntlet(entity), weapon_machinegun(entity), weapon_shotgun(entity), weapon_grenade(entity), weapon_rocket(entity),
  weapon_lightning(entity), weapon_railgun(entity), weapon_plasma(entity)
{
	EntPlayer::entity = entity;

	spawned = false;
	godmode = false;
	local = false;
	falling = false;
	immobile = false;
	in_vehicle = -1;
	seat = 1;

	EntPlayer::team = team;

	num_sentry = 0;
	build_timer = 1;

	haste_factor = 2.0f;

	telefragged = NULL;


	if (ent_type == ENT_SERVER)
	{
		type = SERVER;
		entity->ent_type = ENT_SERVER;
	}
	else if (ent_type == ENT_PLAYER)
	{
		type = PLAYER;
		entity->ent_type = ENT_PLAYER;
	}

	zoom_level = 4.0;
	weapon_source = -1;
	weapon_loop_source = -1;
	last_weapon = 0;
	last_state = BOT_IDLE;
	state = PLAYER_IDLE;

	path.path = new int[NUM_PATH];
	get_item = 0;
	ignore[0] = '\0';

	impressive_award_timer = 0;
	excellent_award_timer = 0;
	gauntlet_award_timer = 0;

		
	pm_accel = 10.0f;
	pm_airaccel = 1.0f;
	pm_wateraccel = 4.0f;
	pm_flyaccel = 8.0f;

	pm_max_speed = 30.0f;
	pm_max_air_speed = 55.0f;
	pm_air_control = 2.5f;

	pm_friction = 6.0f;
	pm_waterfriction = 1.0f;
	pm_flightfriction = 3.0f;
	pm_spectatorfriction = 5.0f;


	model_index = model; // default to visor -- ranger isnt in demo files :'( 

	strcpy(model_name, models[model_index]);

	health = 125;
	armor = 0;
	alert_timer = 0;
	idle_timer = (20 + rand() % 10) * TICK_RATE;
	flight_timer = 0;
	teleport_timer = 0;
	drown_timer = 0;
	regen_timer = 0;
	haste_timer = 0;
	quad_timer = 0;
	reload_timer = 0;
	invisibility_timer = 0;
	jumppad_timer = 0;
	excellent_timer = 0;
	click_timer = 0;
	pain_timer = 0;
	impressive_count = 0;
	fall_timer = 0;

	flash_gauntlet = 0;
	flash_machinegun = 0;
	flash_shotgun = 0;
	flash_grenade = 0;
	flash_rocket = 0;
	flash_railgun = 0;
	flash_lightning = 0;
	flash_plasma = 0;
	dead_timer = 0;
	alive_timer = 0;

	holdable_teleporter = false;
	holdable_medikit = false;
	holdable_flag = false;
	quad_damage = false;


	weapon_flags = WEAPON_MACHINEGUN | WEAPON_GAUNTLET;
	current_weapon = wp_machinegun;
	ammo_rockets = 0;
	ammo_grenades = 0;
	ammo_slugs = 0;
	ammo_shells = 0;
	ammo_bullets = 50;
	ammo_lightning = 0;
	ammo_plasma = 0;
	ammo_bfg = 0;
	sprintf(name, "UnnamedPlayer");
	memset(&stats, 0, sizeof(stats_t));

//	weapon_machinegun.load(gfx, "media/models/weapons2/m4/m4s");
	weapon_gauntlet.clone(model_table[MODEL_WEAPON_GAUNTLET]);
	weapon_machinegun.clone(model_table[MODEL_WEAPON_MACHINEGUN]);
	weapon_shotgun.clone(model_table[MODEL_WEAPON_SHOTGUN]);
	weapon_grenade.clone(model_table[MODEL_WEAPON_GRENADE]);
	weapon_rocket.clone(model_table[MODEL_WEAPON_ROCKET]);
	weapon_lightning.clone(model_table[MODEL_WEAPON_LIGHTNING]);
	weapon_railgun.clone(model_table[MODEL_WEAPON_RAILGUN]);
	weapon_plasma.clone(model_table[MODEL_WEAPON_PLASMA]);


	//	weapon_model.center = entity->rigid->center;

	current_light = 0;
	current_face = 0;
	bot_state = BOT_IDLE;

	path.length = 0;
	path.step = 0;


	done_transform = false;
	done_pos = vec3(0.0f, 0.0f, 0.0f);


	frame_limit(IDLE_GUN, LENGTH_IDLE_GUN, IDLE_GUN, LENGTH_IDLE_GUN);
}


void EntPlayer::frame_limit(int start, int length, int end_start, int end_length)
{
	ani_state.fstart = start;
	ani_state.flength = length;


	ani_state.animation_frame = ani_state.fstart;
	ani_state.done = false;


	if (end_start != -1 && end_length != -1)
	{
		ani_state.fstart_end = end_start;
		ani_state.flength_end = end_length;
	}
	else
	{
		ani_state.fstart_end = 0;
		ani_state.flength_end = INT_MAX;
	}
}



void EntPlayer::respawn()
{
	health = 100;
	armor = 0;
	weapon_flags = WEAPON_MACHINEGUN | WEAPON_GAUNTLET;
	current_weapon = wp_machinegun;
	ammo_rockets = 0;
	ammo_slugs = 0;
	ammo_shells = 0;
	ammo_bullets = 50;
	ammo_lightning = 0;
	ammo_plasma = 0;
	ammo_bfg = 0;
	reload_timer = 0;
	click_timer = 0;
	entity->rigid->velocity = vec3(0.0f, 0.0f, 0.0f);
	entity->rigid->net_force = vec3(0.0f, 0.0f, 0.0f);
	entity->rigid->bsp_trigger_volume = 0;
	entity->rigid->flags.lava = false;
	entity->rigid->flags.slime = false;
	telefragged = NULL;


	state = PLAYER_IDLE;
	bot_state = BOT_IDLE;


	armor = 0;
	flight_timer = 0;
	teleport_timer = 0;
	drown_timer = 0;
	regen_timer = 0;
	haste_timer = 0;
	quad_timer = 0;
	reload_timer = 0;
	invisibility_timer = 0;
	alive_timer = 0;
	falling = false;


	frame_limit(IDLE_GUN, LENGTH_IDLE_GUN, IDLE_GUN, LENGTH_IDLE_GUN);

//	entity->model->make_aabb();
}

void EntPlayer::reset()
{
	respawn();
	memset(&stats, 0, sizeof(stats_t));
}

void EntPlayer::kill()
{
	weapon_flags = 0;
	reload_timer = 120;
	current_weapon = wp_none;
	state = PLAYER_DEAD;
}

void EntPlayer::render_weapon(Graphics &gfx)
{
	if (invisibility_timer > 0)
	{
		gfx.Blend(true);
		gfx.BlendFuncOneOne();
	}

	switch (current_weapon)
	{
	case wp_none:
		break;
	case wp_gauntlet:
		weapon_gauntlet.render(gfx);
		break;
	case wp_machinegun:
		weapon_machinegun.render(gfx);
		break;
	case wp_shotgun:
		weapon_shotgun.render(gfx);
		break;
	case wp_grenade:
		weapon_grenade.render(gfx);
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
	case wp_plasma:
		weapon_plasma.render(gfx);
		break;
	}

	if (invisibility_timer > 0)
	{
		gfx.Blend(false);
	}
}

void EntPlayer::change_weapon_up()
{
	if (reload_timer != 0)
		return;

	switch (current_weapon)
	{
	case wp_none:
		if (weapon_flags & WEAPON_GAUNTLET)
			current_weapon = wp_machinegun;
		else if (weapon_flags & WEAPON_MACHINEGUN)
			current_weapon = wp_machinegun;
		else if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		else if (weapon_flags & WEAPON_GRENADE)
			current_weapon = wp_grenade;
		else if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		else if (weapon_flags & WEAPON_PLASMA)
			current_weapon = wp_plasma;
		break;
	case wp_gauntlet:
		if (weapon_flags & WEAPON_MACHINEGUN)
			current_weapon = wp_machinegun;
		else if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		else if (weapon_flags & WEAPON_GRENADE)
			current_weapon = wp_grenade;
		else if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		else if (weapon_flags & WEAPON_PLASMA)
			current_weapon = wp_plasma;
		break;
	case wp_machinegun:
		if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		else if (weapon_flags & WEAPON_GRENADE)
			current_weapon = wp_grenade;
		else if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		else if (weapon_flags & WEAPON_PLASMA)
			current_weapon = wp_plasma;
		break;
	case wp_shotgun:
		if (weapon_flags & WEAPON_GRENADE)
			current_weapon = wp_grenade;
		else if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		else if (weapon_flags & WEAPON_PLASMA)
			current_weapon = wp_plasma;
		break;
	case wp_grenade:
		if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		else if (weapon_flags & WEAPON_PLASMA)
			current_weapon = wp_plasma;
		break;
	case wp_rocket:
		if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		else if (weapon_flags & WEAPON_PLASMA)
			current_weapon = wp_plasma;
		break;
	case wp_lightning:
		if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		else if (weapon_flags & WEAPON_PLASMA)
			current_weapon = wp_plasma;
		break;
	case wp_railgun:
		if (weapon_flags & WEAPON_PLASMA)
			current_weapon = wp_plasma;
		break;
	case wp_plasma:
		break;
	}
}

void EntPlayer::change_weapon_down()
{
	if (reload_timer != 0)
		return;

	switch (current_weapon)
	{
	case wp_none:
		if (weapon_flags & WEAPON_PLASMA)
			current_weapon = wp_plasma;
		else if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		else if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_GRENADE)
			current_weapon = wp_grenade;
		else if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		else if (weapon_flags & WEAPON_MACHINEGUN)
			current_weapon = wp_machinegun;
		else if (weapon_flags & WEAPON_GAUNTLET)
			current_weapon = wp_machinegun;
		break;
	case wp_gauntlet:
		break;
	case wp_machinegun:
		if (weapon_flags & WEAPON_GAUNTLET)
			current_weapon = wp_gauntlet;
		break;
	case wp_shotgun:
		if (weapon_flags & WEAPON_MACHINEGUN)
			current_weapon = wp_machinegun;
		else if (weapon_flags & WEAPON_GAUNTLET)
			current_weapon = wp_machinegun;
		break;
	case wp_grenade:
		if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		else if (weapon_flags & WEAPON_MACHINEGUN)
			current_weapon = wp_machinegun;
		else if (weapon_flags & WEAPON_GAUNTLET)
			current_weapon = wp_machinegun;
		break;
	case wp_rocket:
		if (weapon_flags & WEAPON_GRENADE)
			current_weapon = wp_grenade;
		else if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		else if (weapon_flags & WEAPON_MACHINEGUN)
			current_weapon = wp_machinegun;
		else if (weapon_flags & WEAPON_GAUNTLET)
			current_weapon = wp_machinegun;
		break;
	case wp_lightning:
		if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		else if (weapon_flags & WEAPON_MACHINEGUN)
			current_weapon = wp_machinegun;
		else if (weapon_flags & WEAPON_GAUNTLET)
			current_weapon = wp_machinegun;
		break;
	case wp_railgun:
		if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		else if (weapon_flags & WEAPON_MACHINEGUN)
			current_weapon = wp_machinegun;
		else if (weapon_flags & WEAPON_GAUNTLET)
			current_weapon = wp_machinegun;
		break;
	case wp_plasma:
		if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		else if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		else if (weapon_flags & WEAPON_MACHINEGUN)
			current_weapon = wp_machinegun;
		else if (weapon_flags & WEAPON_GAUNTLET)
			current_weapon = wp_machinegun;
		break;
	}
}


void EntPlayer::best_weapon()
{
	if ((weapon_flags & wp_railgun) && ammo_slugs > 0)
		current_weapon = wp_railgun;
	else if ((weapon_flags & wp_rocket) && ammo_rockets > 0)
		current_weapon = wp_rocket;
	else if ((weapon_flags & wp_plasma) && ammo_plasma > 0)
		current_weapon = wp_plasma;
	else if ((weapon_flags & wp_lightning) && ammo_lightning > 0)
		current_weapon = wp_lightning;
	else if ((weapon_flags & wp_grenade) && ammo_grenades > 0)
		current_weapon = wp_grenade;
	else if ((weapon_flags & wp_shotgun) && ammo_shells > 0)
		current_weapon = wp_shotgun;
	else if ((weapon_flags & wp_machinegun) && ammo_bullets > 0)
		current_weapon = wp_machinegun;

}

float EntPlayer::DistanceToLine(vec3 &direction, vec3 &origin, vec3 &point)
{
	return vec3::crossproduct(direction, point - origin).magnitude();
}

int EntPlayer::FindLookAt(vec3 &cameraOrigin, vec3 &cameraDir, vec3 *points, int numPoints)
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


EntPlayer::~EntPlayer()
{
	delete[] path.path;
}


void EntPlayer::avoid_walls(Bsp &map)
{
	Frame frame;
	vec3 rightv;

	entity->model->get_frame(frame);

	vec3 forward = entity->position + frame.forward * 100.0f;
	vec3 backward = entity->position + frame.forward * -100.0f;

	rightv = vec3::crossproduct(frame.up, forward);

	vec3 right = entity->position + rightv * 100.0f;
	vec3 left = entity->position + rightv * -100.0f;

	vec3 down = entity->position + frame.up * -100.0f + frame.forward * 25.0f;


	plane_t plane;
	float depth;
	float water_depth;
	vector<surface_t *> surface_list;
	vec3 clip;
	vec3 vel;
	int model_trigger;
	int model_platform;
	//int surf_flags = 0;

	input_t input;

	memset(&input, 0, sizeof(input_t));
	float speed_scale = 1.0f;
	content_flag_t flag;;

	if (haste_timer > 0)
		speed_scale = entity->player->haste_factor;


	vec3 tri[3];

	map.collision_detect(forward, entity->position, &plane, &depth, water_depth, surface_list, false, clip, vel, model_trigger, model_platform, flag, tri);
	if ( depth < 0 )
	{
		input.moveright = true;
	}

	map.collision_detect(backward, entity->position, &plane, &depth, water_depth, surface_list, false, clip, vel, model_trigger, model_platform, flag, tri);
	if (depth < 0)
	{
		input.moveleft = true;
	}

	map.collision_detect(left, entity->position, &plane, &depth, water_depth, surface_list, false, clip, vel, model_trigger, model_platform, flag, tri);
	if (depth < 0)
	{
		input.moveup = true;
	}

	map.collision_detect(right, entity->position, &plane, &depth, water_depth, surface_list, false, clip, vel, model_trigger, model_platform, flag, tri);
	if (depth < 0)
	{
		input.movedown = true;
	}

	map.collision_detect(down, entity->position, &plane, &depth, water_depth, surface_list, false, clip, vel, model_trigger, model_platform, flag, tri);
	if (depth > 0)
	{
		input.jump = true;
	}

	entity->rigid->move(input, speed_scale);

}

void EntPlayer::handle_bot(vector<Entity *> &entity_list, int self)
{
	for (unsigned int i = 0; i < entity_list.size(); i++)
	{
		if (i == (unsigned int)self)
			continue;

		EntPlayer *player = entity_list[i]->player;


		if (player && player->type == PLAYER)
		{
			if (team != TEAM_NONE && player->team == team)
				continue;

			float distance = (entity_list[i]->position - entity->position).magnitude();

			if (distance < 500.0f)
			{
				entity->rigid->lookat(entity_list[i]->position);
				bot_state = BOT_ALERT;
			}
			else
			{
				if (bot_state == BOT_ALERT)
				{
					bot_state = BOT_IDLE;
				}
			}

			if (distance < 400.0f)
			{
				if (reload_timer <= 0 && player->state != PLAYER_DEAD)
				{
					entity->rigid->lookat(entity_list[i]->position);
					bot_state = BOT_ATTACK;
				}

				if (player->state == PLAYER_DEAD)
				{
					bot_state = BOT_IDLE;
				}
			}
			else
			{
				if (bot_state == BOT_ATTACK)
					bot_state = BOT_ALERT;
			}

			continue;
		}
	}
}


//#define DEBUG_BOT
int EntPlayer::bot_search_for_items(vector<Entity *> &entity_list, int self)
{
#ifdef DEBUG_BOT
	printf("handle_bot() ignore %s\n", ignore);
#endif

	best_weapon();

#ifdef DEBUG_BOT
	printf("bot_search_for_items() state is %d\n", bot_state);
#endif

	bool need_health =			(health < 100);
	bool need_armor =			(armor < 100);
	bool need_shotgun =			((weapon_flags & wp_shotgun) == 0);
	bool need_rocketlauncher =	((weapon_flags & wp_shotgun) == 0);
	bool need_lightning =		((weapon_flags & wp_shotgun) == 0);
	bool need_railgun =			((weapon_flags & wp_shotgun) == 0);
	bool need_plasma =			((weapon_flags & wp_shotgun) == 0);
	bool need_grenadelauncher = ((weapon_flags & wp_shotgun) == 0);
	bool need_ammo =			(current_weapon == wp_machinegun);

	if (last_state == BOT_IDLE)
	{
		need_ammo = true;
	}

	int ignore_length = strlen(ignore);

	for (unsigned int i = 0; i < entity_list.size(); i++)
	{
		if (i == (unsigned int)self)
			continue;


		if (ignore_length >= 1)
		{
			if (entity_list[i]->entstring)
			{
				if (strstr(ignore, entity_list[i]->entstring->type) != NULL)
					continue;
			}
		}

		last_state = bot_state;

		if (entity_list[i]->trigger == NULL)
			continue;

		if (entity_list[i]->trigger->active == true)
			continue;

		/*
		// Idea is to prefer nearby items without doing expense of distance check first
		if (last_state != BOT_IDLE && last_state != BOT_GET_ITEM && last_state != BOT_EXPLORE &&
			entity_list[i]->bsp_leaf != entity_list[self]->bsp_leaf)
			continue;
			*/

		float distance = (entity_list[i]->position - entity->position).magnitude();

		// Skip far away items unless we hit explore state (in which case nothing good nearby)
		if (distance > 1500.0f && bot_state != BOT_EXPLORE)
			continue;

#ifdef DEBUG_BOT
		printf("Bot is near entity %d type %s distance %3.3f\n", i, entity_list[i]->type, distance);
#endif
		if (entity_list[i]->ent_type == ENT_ITEM_QUAD)
		{
//			printf("bot %d wants %s\n", i, entity_list[i]->type);
			bot_state = BOT_GET_ITEM;
			return i;
		}
		else if (entity_list[i]->ent_type == ENT_ITEM_HEALTH_MEGA)
		{
//			printf("bot %d wants %s\n", i, entity_list[i]->type);
			bot_state = BOT_GET_ITEM;
			return i;
		}
		else if (need_health)
		{
			if (entity_list[i]->ent_type == ENT_ITEM_HEALTH_LARGE)
			{
//				printf("bot %d wants %s\n", i, entity_list[i]->type);
				bot_state = BOT_GET_ITEM;
				return i;
			}
			else if (entity_list[i]->ent_type == ENT_ITEM_HEALTH_SMALL)
			{
//				printf("bot %d wants %s\n", i, entity_list[i]->type);
				bot_state = BOT_GET_ITEM;
				return i;
			}
		}
		else if (entity_list[i]->ent_type == ENT_WEAPON_ROCKETLAUNCHER && need_rocketlauncher)
		{
//			printf("bot %d wants %s\n", i, entity_list[i]->type);
			bot_state = BOT_GET_ITEM;
			return i;
		}
		else if (entity_list[i]->ent_type == ENT_WEAPON_LIGHTNING && need_lightning)
		{
//			printf("bot %d wants %s\n", i, entity_list[i]->type);
			bot_state = BOT_GET_ITEM;
			return i;
		}
		else if (entity_list[i]->ent_type == ENT_WEAPON_RAILGUN && need_railgun)
		{
//			printf("bot %d wants %s\n", i, entity_list[i]->type);
			bot_state = BOT_GET_ITEM;
			return i;
		}
		else if (entity_list[i]->ent_type == ENT_WEAPON_PLASMA && need_plasma)
		{
//			printf("bot %d wants %s\n", i, entity_list[i]->type);
			bot_state = BOT_GET_ITEM;
			return i;
		}
		else if (entity_list[i]->ent_type == ENT_WEAPON_GRENADELAUNCHER && need_grenadelauncher)
		{
//			printf("bot %d wants %s\n", i, entity_list[i]->type);
			bot_state = BOT_GET_ITEM;
			return i;
		}
		else if (entity_list[i]->ent_type == ENT_WEAPON_SHOTGUN && need_shotgun)
		{
//			printf("bot %d wants %s\n", i, entity_list[i]->type);
			bot_state = BOT_GET_ITEM;
			return i;
		}
		else if (need_armor)
		{
			if (entity_list[i]->ent_type == ENT_ITEM_ARMOR_BODY)
			{
//				printf("bot %d wants %s\n", i, entity_list[i]->type);
				bot_state = BOT_GET_ITEM;
				return i;
			}
			else if (entity_list[i]->ent_type == ENT_ITEM_ARMOR_COMBAT)
			{
//				printf("bot %d wants %s\n", i, entity_list[i]->type);
				bot_state = BOT_GET_ITEM;
				return i;
			}
			else if (entity_list[i]->ent_type == ENT_ITEM_ARMOR_SHARD)
			{
//				printf("bot %d wants %s\n", i, entity_list[i]->type);
				bot_state = BOT_GET_ITEM;
				return i;
			}
		}
		else if (entity_list[i]->ent_type == ENT_TRIGGER_TELEPORT)
		{
//			printf("bot %d wants %s\n", i, entity_list[i]->type);
			bot_state = BOT_GET_ITEM;
			return i;
		}

		if ((entity_list[i]->ent_type > ENT_START_AMMO && entity_list[i]->ent_type < ENT_END_AMMO)  && need_ammo)
		{
//			printf("bot %d wants %s\n", i, entity_list[i]->type);
			bot_state = BOT_GET_ITEM;
			return i;
		}
		last_state = bot_state;

	}

	// Exhausted nearby items
	if (bot_state == BOT_IDLE)
		bot_state = BOT_EXPLORE;
	return -1;
}
