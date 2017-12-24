#include "constructable.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include <float.h>
#include "quake3.h"


Constructable::Constructable(Entity *entity, Graphics &gfx, Audio &audio)
{
	Constructable::entity = entity;

	spawned = false;
	render_md5 = true;
	immobile = false;

	Constructable::team = team;
	owner = -1;


	telefragged = NULL;


	weapon_source = -1;
	weapon_loop_source = -1;
	last_weapon = 0;
	last_state = BOT_IDLE;
	state = PLAYER_IDLE;

	strcpy(model_name, "Autosentry");

	health = 125;
	armor = 0;
	alert_timer = 0;
	idle_timer = (20 + rand() % 10) * TICK_RATE;
	reload_timer = 0;
	click_timer = 0;
	pain_timer = 0;

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



void Constructable::handle_bot(vector<Entity *> &entity_list, int self)
{
	for (unsigned int i = 0; i < entity_list.size(); i++)
	{
		if (i == (unsigned int)self)
			continue;

		Player *player = entity_list[i]->player;


		if (player && player->type == PLAYER)
		{
			if (team != TEAM_NONE && player->team == team)
				continue;

			if (team == TEAM_NONE || owner == i)
			{
				continue;
			}

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
