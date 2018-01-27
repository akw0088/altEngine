#include "constructable.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include <float.h>
#include "quake3.h"


Constructable::Constructable(Entity *entity, Graphics &gfx, Audio &audio)
{
	Constructable::entity = entity;
	Constructable::team = team;

	spawned = false;
	render_md5 = true;
	immobile = false;

	owner = -1;
	telefragged = NULL;

	weapon_source = -1;
	weapon_loop_source = -1;
	last_weapon = 0;
	last_state = BOT_IDLE;
	state = PLAYER_IDLE;

	health = SENTRY_HEALTH_LVL1;
	armor = 0;
	alert_timer = 0;
	idle_timer = (20 + rand() % 10) * TICK_RATE;
	reload_timer = 0;
	reload_timer2 = 0;
	click_timer = 0;
	pain_timer = 0;
	dead_timer = 0;
	alive_timer = 0;

	build_timer = 3 * TICK_RATE;

	flash_gauntlet = 0;
	flash_machinegun = 0;
	flash_shotgun = 0;
	flash_grenade = 0;
	flash_rocket = 0;
	flash_railgun = 0;
	flash_lightning = 0;
	flash_plasma = 0;

	weapon_flags = WEAPON_MACHINEGUN;
	current_weapon = wp_machinegun;
	ammo_rockets = 30;
	ammo_grenades = 0;
	ammo_slugs = 0;
	ammo_shells = 0;
	ammo_bullets = 1000;
	ammo_lightning = 0;
	ammo_plasma = 0;
	ammo_bfg = 0;
	sprintf(name, "Autosentry");
	memset(&stats, 0, sizeof(stats_t));

	//	weapon_model.center = entity->rigid->center;

	current_light = 0;
	current_face = 0;
	bot_state = BOT_IDLE;
}

void Constructable::reset()
{
	memset(&stats, 0, sizeof(stats_t));
}


Constructable::~Constructable()
{
}

void Constructable::destroy()
{
	if (entity->rigid)
	{
		delete entity->rigid;
		entity->rigid = NULL;
		entity->model = NULL;
	}

	if (entity->model)
	{
		delete entity->model;
		entity->model = NULL;
	}
}



int Constructable::step(input_t &input, vector<Entity *> &entity_list, int self, Engine &engine)
{
	if (level != 3)
		engine.entity_list[base_index]->position = entity->position - vec3(0, 30.0, 5.0);

	if (build_timer > 0)
	{
		if (input.control == false && build_timer > 125)
		{
			//building cancelled, delete stuff
			build_timer--;
			return 1;
		}

		build_timer--;
		return 0;
	}


	if (reload_timer > 0)
		reload_timer--;

	if (reload_timer2 > 0)
		reload_timer2--;


	for (unsigned int i = 0; i < entity_list.size(); i++)
	{
		if (i == (unsigned int)self)
			continue;

		Player *player = entity_list[i]->player;


		if (player)
		{
			if (team != TEAM_NONE && player->team == team)
				continue;

			if ((unsigned int)owner == i)
			{
				continue;
			}

			float distance = (entity_list[i]->position - entity->position).magnitude();

			if (distance < 1500.0f)
			{
				int index[8] = { 0 };
				int num_index = 0;
				vec3 dir = (entity_list[i]->position - entity->position);
				dir.normalize();
				engine.hitscan(entity->position, dir, index, num_index, self, -1.0f);
				if (num_index > 0)
				{
					entity->rigid->lookat(entity_list[i]->position);
					if (bot_state != BOT_ATTACK)
						bot_state = BOT_ALERT;
				}
			}
			else
			{
				if (bot_state == BOT_ALERT)
				{
					bot_state = BOT_IDLE;
				}
			}

			if (distance < 1400.0f)
			{
				if (reload_timer <= 0 && player->state != PLAYER_DEAD)
				{
					int index[8] = { 0 };
					int num_index = 0;
					vec3 dir = (entity_list[i]->position - entity->position);
					dir.normalize();
					engine.hitscan(entity->position, dir, index, num_index, self, -1.0f);
					if (num_index > 0)
					{
						entity->rigid->lookat(entity_list[i]->position);
						bot_state = BOT_ATTACK;
					}
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

			// already have a target
			if (bot_state == BOT_ATTACK)
				break;

			continue;
		}
	}

	return 0;
}
