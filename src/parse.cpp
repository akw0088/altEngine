#include "include.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WEAPON_TIMEOUT 5
#define AMMO_TIMEOUT 10
#define HEALTH_TIMEOUT 10

#include <ctype.h>

void add_key(Engine *engine, Entity &entity, char *key, char *value, Graphics &gfx, Audio &audio, int entity_num)
{
	static int light_num = 0;

	if (strcmp(key, "origin") == 0)
	{
		int x, y, z;

		sscanf(value, "%d %d %d", &x, &y, &z);
		entity.position.x = (float)x;
		entity.position.y = (float)z;
		entity.position.z = (float)-y;

		if (strstr(entity.type, "navpoint") != NULL)
		{
			entity.position.x = (float)x;
			entity.position.y = (float)y;
			entity.position.z = (float)z;
		}
	}
	else if (strcmp(key, "model") == 0)
	{
		int num;

		// Reference into BSP model lump for triggers / movers
		if (strchr(value, '*') != NULL)
		{
			int ret = sscanf(value + 1, "%d", &num);
			if (ret == 1)
				entity.model_ref = num;

			if (entity.rigid)
				entity.rigid->gravity = false;
		}
	}
	else if (strcmp(key, "classname") == 0)
	{
		size_t size = strlen(value) + 1;
		memcpy(entity.type, value, size);

		if (strcmp(value, "item_armor_shard") == 0)
		{
			entity.ent_type = ENT_ITEM_ARMOR_SHARD;
			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/items/s_health.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");

			//snprintf(entity.trigger->respawn_sound, LINE_SIZE, "sound/items/respawn1.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "armor 5");
			entity.trigger->timeout_value = AMMO_TIMEOUT;
		}
		else if (strcmp(value, "item_health") == 0)
		{
			entity.ent_type = ENT_ITEM_HEALTH;
			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/items/n_health.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "health 25");
			entity.trigger->health = true;
			entity.trigger->timeout_value = HEALTH_TIMEOUT;
		}
		else if (strcmp(value, "item_health_large") == 0)
		{
			entity.ent_type = ENT_ITEM_HEALTH_LARGE;
			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/items/l_health.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "health 50");
			entity.trigger->health = true;
			entity.trigger->timeout_value = HEALTH_TIMEOUT;
		}
		else if (strcmp(value, "item_health_small") == 0)
		{
			entity.ent_type = ENT_ITEM_HEALTH_SMALL;
			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/items/n_health.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "health 5");
			entity.trigger->health = false;
			entity.trigger->timeout_value = HEALTH_TIMEOUT;
		}
		else if (strcmp(value, "item_health_mega") == 0)
		{
			entity.ent_type = ENT_ITEM_HEALTH_MEGA;
			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/items/m_health.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "health 100");
			entity.trigger->health = false;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
		}
		else if (strcmp(value, "item_quad") == 0)
		{
			entity.ent_type = ENT_ITEM_QUAD;
			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/items/quaddamage.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/poweruprespawn.wav");

			snprintf(entity.trigger->action, LINE_SIZE, "quaddamage");
			entity.trigger->health = false;
		}
		else if (strcmp(value, "holdable_medkit") == 0)
		{
			entity.ent_type = ENT_HOLDABLE_MEDKIT;
			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/items/holdable.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "holdable_medkit");
			entity.trigger->health = false;
		}
		else if (strcmp(value, "holdable_teleporter") == 0)
		{
			entity.ent_type = ENT_HOLDABLE_TELEPORTER;
			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/items/holdable.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "holdable_teleporter");
			entity.trigger->health = false;
		}
		else if (strcmp(value, "item_enviro") == 0)
		{
			entity.ent_type = ENT_ITEM_ENVIRO;
			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/items/protect.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "bodysuit");
			entity.trigger->health = false;
		}
		else if (strcmp(value, "item_flight") == 0)
		{
			entity.ent_type = ENT_ITEM_FLIGHT;
			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/items/holdable.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "flight 30");
			entity.trigger->health = false;
		}
		else if (strcmp(value, "item_haste") == 0)
		{
			entity.ent_type = ENT_ITEM_HASTE;
			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/items/haste.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "haste");
			entity.trigger->health = false;
		}
		else if (strcmp(value, "item_invis") == 0)
		{
			entity.ent_type = ENT_ITEM_INVIS;
			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/items/invisibility.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "invisibility");
			entity.trigger->health = false;
		}
		else if (strcmp(value, "item_regen") == 0)
		{
			entity.ent_type = ENT_ITEM_REGEN;
			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/items/regeneration.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/poweruprespawn.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "regeneration");
			entity.trigger->health = false;
		}
		else if (strcmp(value, "team_CTF_blueflag") == 0)
		{
			entity.ent_type = ENT_TEAM_CTF_BLUEFLAG;
			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/teamplay/flagtk_blu.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/poweruprespawn.wav");

			snprintf(entity.trigger->action, LINE_SIZE, "blueflag");
			entity.trigger->health = false;
		}
		else if (strcmp(value, "team_CTF_redflag") == 0)
		{
			entity.ent_type = ENT_TEAM_CTF_REDFLAG;
			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/teamplay/flagtk_red.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/poweruprespawn.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "redflag");
			entity.trigger->health = false;
		}
		else if (strcmp(value, "ammo_bullets") == 0)
		{
			entity.ent_type = ENT_AMMO_BULLETS;
			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/misc/am_pkup.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "ammo_bullets 25");
			entity.trigger->timeout_value = AMMO_TIMEOUT;
		}
		else if (strcmp(value, "ammo_rockets") == 0)
		{
			entity.ent_type = ENT_AMMO_ROCKETS;
			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/misc/am_pkup.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "ammo_rockets 10");
			entity.trigger->timeout_value = AMMO_TIMEOUT;
		}
		else if (strcmp(value, "ammo_slugs") == 0)
		{
			entity.ent_type = ENT_AMMO_SLUGS;
			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/misc/am_pkup.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "ammo_slugs 10");
			entity.trigger->timeout_value = AMMO_TIMEOUT;
		}
		else if (strcmp(value, "ammo_shells") == 0)
		{
			entity.ent_type = ENT_AMMO_SHELLS;
			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/misc/am_pkup.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "ammo_shells 10");
			entity.trigger->timeout_value = AMMO_TIMEOUT;
		}
		else if (strcmp(value, "ammo_cells") == 0)
		{
			entity.ent_type = ENT_AMMO_CELLS;
			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/misc/am_pkup.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "ammo_plasma 50");
			entity.trigger->timeout_value = AMMO_TIMEOUT;
		}
		else if (strcmp(value, "ammo_lightning") == 0)
		{
			entity.ent_type = ENT_AMMO_LIGHTNING;
			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/misc/am_pkup.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "ammo_lightning 50");
			entity.trigger->timeout_value = AMMO_TIMEOUT;
		}
		else if (strcmp(value, "ammo_bfg") == 0)
		{
			entity.ent_type = ENT_AMMO_BFG;
			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/misc/am_pkup.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "ammo_lightning 50");
			entity.trigger->timeout_value = AMMO_TIMEOUT;
		}
		else if (strcmp(value, "ammo_grenades") == 0)
		{
			entity.ent_type = ENT_AMMO_GRENADES;
			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/misc/am_pkup.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "ammo_grenades 10");
			entity.trigger->timeout_value = AMMO_TIMEOUT;
		}
		else if (strstr(value, "weapon_rocketlauncher"))
		{
			entity.ent_type = ENT_WEAPON_ROCKETLAUNCHER;
			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
			entity.trigger->timeout_value = WEAPON_TIMEOUT;


			snprintf(entity.trigger->action, LINE_SIZE, "weapon_rocketlauncher");
		}
		else if (strstr(value, "weapon_lightning"))
		{
			entity.ent_type = ENT_WEAPON_LIGHTNING;
			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "weapon_lightning");
			entity.trigger->timeout_value = WEAPON_TIMEOUT;
		}
		else if (strstr(value, "weapon_bfg"))
		{
			entity.ent_type = ENT_WEAPON_BFG;
			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "weapon_lightning");
			entity.trigger->timeout_value = WEAPON_TIMEOUT;
		}
		else if (strstr(value, "weapon_shotgun"))
		{
			entity.ent_type = ENT_WEAPON_SHOTGUN;
			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "weapon_shotgun");
			entity.trigger->timeout_value = WEAPON_TIMEOUT;
		}
		else if (strstr(value, "weapon_machinegun"))
		{
			entity.ent_type = ENT_WEAPON_MACHINEGUN;
			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "weapon_machinegun");
			entity.trigger->timeout_value = WEAPON_TIMEOUT;
		}
		else if (strstr(value, "weapon_railgun"))
		{
			entity.ent_type = ENT_WEAPON_RAILGUN;
			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "weapon_railgun");
			entity.trigger->timeout_value = WEAPON_TIMEOUT;
		}
		else if (strstr(value, "weapon_plasma"))
		{
			entity.ent_type = ENT_WEAPON_PLASMA;
			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "weapon_plasma");
			entity.trigger->timeout_value = WEAPON_TIMEOUT;
		}
		else if (strstr(value, "weapon_grenadelauncher"))
		{
			entity.ent_type = ENT_WEAPON_GRENADELAUNCHER;
			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "weapon_grenadelauncher");
			entity.trigger->timeout_value = WEAPON_TIMEOUT;
		}
		else if (strcmp(value, "item_armor_combat") == 0)
		{
			entity.ent_type = ENT_ITEM_ARMOR_COMBAT;
			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/misc/ar2_pkup.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");

			snprintf(entity.trigger->action, LINE_SIZE, "armor 50");
			entity.trigger->armor = true;
		}
		else if (strcmp(value, "item_armor_body") == 0)
		{
			entity.ent_type = ENT_ITEM_ARMOR_BODY;
			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/misc/ar2_pkup.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "armor 100");
			entity.trigger->armor = true;
		}
		else if (strcmp(value, "func_bobbing") == 0)
		{
			entity.ent_type = ENT_FUNC_BOBBING;
			/*
			speed : amount of time in seconds for one complete oscillation cycle (default 4).
			height : sets the amount of travel of the oscillation movement (default 32).
			phase : sets the start offset of the oscillation cycle. Values must be 0 < phase < 1. Any integer phase value is the same as no offset (default 0).
			noise : path/name of .wav file to play. Use looping sounds only (eg. sound/world/drone6.wav - See Notes).
			*/
			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->flight = true;
			}
		}
		else if (strcmp(value, "func_button") == 0)
		{
			entity.ent_type = ENT_FUNC_BUTTON;
			/*
			angle : determines the direction in which the button will move (up = -1, down = -2).
			target : all entities with a matching targetname will be triggered.
			speed : speed of button's displacement (default 40).
			wait : number of seconds button stays pressed (default 1, -1 = return immediately).
			lip : lip remaining at end of move (default 4 units).
			health : if set to a non-zero value, the button must be damaged by "health" amount of points to operate.
			*/

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->flight = true;
//				entity.nodraw = true;
			}
		}
		else if (strcmp(value, "func_door") == 0)
		{
			entity.ent_type = ENT_FUNC_DOOR;
			/*
			angle : determines the opening direction of door (up = -1, down = -2).
			speed : determines how fast the door moves (default 100).
			wait : number of seconds before door returns (default 2, -1 = return immediately)
			lip : lip remaining at end of move (default 8)
			targetname : if set, a func_button or trigger is required to activate the door.
			health : if set to a non-zero value, the door must be damaged by "health" amount of points to activate (default 0).
			dmg : damage to inflict on player when he blocks operation of door (default 4). Door will reverse direction when blocked unless CRUSHER spawnflag is set.
			team: assign the same team name to multiple doors that should operate together (see Notes).
			*/

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->flight = true;
			}

		}
		else if (strcmp(value, "func_plat") == 0)
		{
			entity.ent_type = ENT_FUNC_PLAT;
			/*
			speed : determines how fast the plat moves (default 150).
			lip : lip remaining at end of move (default 16). Has no effect if "height" is set.
			height : if set, this will determine the total amount of vertical travel of the plat.
			dmg : damage to inflict on player when he blocks operation of plat (default 4). Plat will reverse direction when blocked.
			*/

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->flight = true;
			}

		}
		else if (strcmp(value, "func_pendulum") == 0)
		{
			entity.ent_type = ENT_FUNC_PENDULUM;
			/*
			angle: angle offset of axis of rotation from default X axis(default 0).
			speed : angle of swing arc in either direction from initial position(default 30).
			phase : sets the start offset of the swinging cycle.Values must be 0 < phase < 1. Any integer phase value is the same as no offset(default 0).
			noise : path / name of.wav file to play.Use looping sounds only(eg.sound / world / drone6.wav).
			*/

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->flight = true;
				entity.rigid->angular_velocity.x = 10.0f;
			}

		}
		else if (strcmp(value, "func_rotating") == 0)
		{
			entity.ent_type = ENT_FUNC_ROTATING;
			/*
			speed: determines how fast entity rotates(default 100).
			noise : path / name of.wav file to play.Use looping sounds only(eg.sound / world / drone6.wav).
			*/

			if (entity.rigid)
			{
				entity.rigid->angular_velocity.y = 1.0f;
				entity.rigid->gravity = false;
				entity.rigid->flight = true;
			}

		}
		else if (strcmp(value, "func_train") == 0)
		{
			entity.ent_type = ENT_FUNC_TRAIN;

			/*
			speed : speed of displacement of train (default 100 or overridden by speed value of path).
			target : this points to the first path_corner of the path which is also the spawn location of the train's origin.
			model2 : path/name of model to include (eg: models/mapobjects/pipe/pipe02.md3).
			*/
			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->flight = true;
			}

		}
		else if (strcmp(value, "trigger_teleport") == 0)
		{
			entity.ent_type = ENT_TRIGGER_TELEPORT;

			if (entity.trigger == NULL)
				entity.trigger = new Trigger(&entity, audio);

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
				entity.nodraw = true;
			}

			sprintf(entity.type, "trigger_teleport");
			entity.trigger->timeout = 1.0f;
			entity.trigger->timeout_value = 1.0f;
			sprintf(entity.trigger->action, "teleport %s %d", entity.target, entity_num);
		}
		else if (strcmp(value, "target_teleporter") == 0)
		{
			entity.ent_type = ENT_TARGET_TELEPORTER;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
				entity.nodraw = true;
			}
		}
		else if (strcmp(value, "trigger_push") == 0)
		{
			entity.ent_type = ENT_TRIGGER_PUSH;

			if (entity.trigger == NULL)
				entity.trigger = new Trigger(&entity, audio);

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
				entity.nodraw = true;
			}

			sprintf(entity.trigger->action, "push %s", entity.target);
			entity.trigger->timeout = 1.0f;
			entity.trigger->timeout_value = 1.0f;
		}
		else if (strcmp(value, "trigger_hurt") == 0)
		{
			entity.ent_type = ENT_TRIGGER_HURT;

			if (entity.trigger == NULL)
				entity.trigger = new Trigger(&entity, audio);

			entity.trigger->timeout = 1.0f;
			entity.trigger->timeout_value = 1.0f;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->flight = true;
				entity.nodraw = true;
			}

			snprintf(entity.trigger->action, LINE_SIZE, "damage 666");
		}
		else if (strcmp(value, "trigger_changelevel") == 0)
		{
			entity.ent_type = ENT_TRIGGER_CHANGELEVEL;

			if (entity.trigger == NULL)
				entity.trigger = new Trigger(&entity, audio);

			entity.trigger->timeout = 1.0f;
			entity.trigger->timeout_value = 1.0f;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->flight = true;
				entity.nodraw = true;
			}
		}
		//trigger_setskill
		else if (strcmp(value, "misc_model") == 0)
		{
			entity.ent_type = ENT_MISC_MODEL;
			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "info_spectator_start") == 0)
		{
			entity.ent_type = ENT_INFO_SPECTATOR_START;
			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "item_botroam") == 0)
		{
			entity.ent_type = ENT_ITEM_BOT_ROAM;
			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "info_firstplace") == 0)
		{
			entity.ent_type = ENT_INFO_FIRSTPLACE;
			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "info_secondplace") == 0)
		{
			entity.ent_type = ENT_INFO_SECONDPLACE;
			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "info_thirdplace") == 0)
		{
			entity.ent_type = ENT_INFO_THIRDPLACE;
			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "shooter_grenade") == 0)
		{
			entity.ent_type = ENT_SHOOTER_GRENADE;
			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "light") == 0)
		{
			entity.ent_type = ENT_LIGHT;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
//			entity.nodraw = true;
		}
		else if (strcmp(value, "func_static") == 0)
		{
			entity.ent_type = ENT_FUNC_STATIC;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "target_remove_powerups") == 0)
		{
			entity.ent_type = ENT_TARGET_REMOVE_POWERUPS;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "func_timer") == 0)
		{
			entity.ent_type = ENT_FUNC_TIMER;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
				entity.nodraw = true;
			}
		}
		else if (strcmp(value, "target_location") == 0)
		{
			entity.ent_type = ENT_TARGET_LOCATION;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "info_camp") == 0)
		{
			entity.ent_type = ENT_INFO_CAMP;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "misc_portal_camera") == 0)
		{
			entity.ent_type = ENT_MISC_PORTAL_CAMERA;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}

			entity.portal_camera = new PortalCamera(&entity, gfx, engine->res_scale, engine->zNear, engine->zFar, engine->inf);
			entity.nodraw = true;
		}
		else if (strcmp(value, "misc_portal_surface") == 0)
		{
			entity.ent_type = ENT_MISC_PORTAL_SURFACE;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}

			entity.portal_camera = new PortalCamera(&entity, gfx, engine->res_scale, engine->zNear, engine->zFar, engine->inf);
			entity.nodraw = true;
		}
		else if (strcmp(value, "worldspawn") == 0)
		{
			entity.ent_type = ENT_WORLDSPAWN;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "info_notnull") == 0)
		{
			entity.ent_type = ENT_INFO_NOTNULL;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "target_position") == 0)
		{
			entity.ent_type = ENT_TARGET_POSITION;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "target_give") == 0)
		{
			entity.ent_type = ENT_TARGET_GIVE;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "info_null") == 0)
		{
			entity.ent_type = ENT_INFO_NULL;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;

		}
		else if (strcmp(value, "target_relay") == 0)
		{
			entity.ent_type = ENT_TARGET_RELAY;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
//				entity.nodraw = true;
			}
		}
		else if (strcmp(value, "trigger_always") == 0)
		{
			entity.ent_type = ENT_TRIGGER_ALWAYS;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
				entity.nodraw = true;
			}
		}
		else if (strcmp(value, "info_player_intermission") == 0)
		{
			entity.ent_type = ENT_INFO_PLAYER_INTERMISSION;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
				entity.nodraw = true;
			}
		}
		else if (strcmp(value, "info_player_deathmatch") == 0)
		{
			entity.ent_type = ENT_INFO_PLAYER_DEATHMATCH;
		}
		else if (strcmp(value, "info_player_start") == 0)
		{
			entity.ent_type = ENT_INFO_PLAYER_START;
		}
		else if (strcmp(value, "team_CTF_bluespawn") == 0)
		{
			entity.ent_type = ENT_TEAM_CTF_BLUESPAWN;
		}
		else if (strcmp(value, "team_CTF_redspawn") == 0)
		{
			entity.ent_type = ENT_TEAM_CTF_REDSPAWN;
		}
		else if (strcmp(value, "team_CTF_blueplayer") == 0)
		{
			entity.ent_type = ENT_TEAM_CTF_BLUEPLAYER;
		}
		else if (strcmp(value, "team_CTF_redplayer") == 0)
		{
			entity.ent_type = ENT_TEAM_CTF_REDPLAYER;
		}
		else if (strcmp(value, "misc_teleporter_dest") == 0)
		{
			entity.ent_type = ENT_MISC_TELEPORTER_DEST;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
				entity.nodraw = true;
			}
		}
		else if (strcmp(value, "target_delay") == 0)
		{
			entity.ent_type = ENT_TARGET_DELAY;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
				entity.nodraw = true;
			}
		}
		else if (strcmp(value, "target_speaker") == 0)
		{
			entity.ent_type = ENT_TARGET_SPEAKER;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
				entity.nodraw = true;

				if (entity.trigger)
				{
					if (entity.trigger->timeout_value < 0.001f)
					{
						entity.trigger->timeout = 1.0f;
						entity.trigger->timeout_value = 1.0f;
					}
					else
					{
						entity.trigger->timeout = 15.0f;
						entity.trigger->timeout_value = 15.0f;
					}

				}
			}
		}
		else if (strcmp(value, "trigger_multiple") == 0)
		{
			entity.ent_type = ENT_TRIGGER_MULTIPLE;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
//				entity.nodraw = true;
			}
		}
		else if (strcmp(value, "navpoint") == 0)
		{
			entity.ent_type = ENT_NAVPOINT;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;

		}
		//quake 1 ents
		if (strcmp(value, "monster_army") == 0)
		{
			entity.ent_type = ENT_Q1_MONSTER_ARMY;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "monster_dog") == 0)
		{
			entity.ent_type = ENT_Q1_MONSTER_DOG;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "monster_ogre") == 0)
		{
			entity.ent_type = ENT_Q1_MONSTER_OGRE;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "monster_zombie") == 0)
		{
			entity.ent_type = ENT_Q1_MONSTER_ZOMBIE;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "monster_shambler") == 0)
		{
			entity.ent_type = ENT_Q1_MONSTER_SHAMBLER;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "monster_wizard") == 0)
		{
			entity.ent_type = ENT_Q1_MONSTER_WIZARD;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "monster_knight") == 0)
		{
			entity.ent_type = ENT_Q1_MONSTER_KNIGHT;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "monster_demon1") == 0)
		{
			entity.ent_type = ENT_Q1_MONSTER_DEMON1;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "monster_enforcer") == 0)
		{
			entity.ent_type = ENT_Q1_MONSTER_ENFORCER;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "monster_fish") == 0)
		{
			entity.ent_type = ENT_Q1_MONSTER_FISH;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "monster_hell_knight") == 0)
		{
			entity.ent_type = ENT_Q1_MONSTER_HELL_KNIGHT;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "monster_shalrath") == 0)
		{
			entity.ent_type = ENT_Q1_MONSTER_SHALRATH;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "monster_tarbaby") == 0)
		{
			entity.ent_type = ENT_Q1_MONSTER_TARBABY;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "light_fluoro") == 0)
		{
			entity.ent_type = ENT_Q1_LIGHT_FLUORO;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "light_fluorospark") == 0)
		{
			entity.ent_type = ENT_Q1_LIGHT_FLUOROSPARK;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "func_door_secret") == 0)
		{
			entity.ent_type = ENT_Q1_FUNC_DOOR_SECRET;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "func_train") == 0)
		{
			entity.ent_type = ENT_FUNC_TRAIN;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "info_teleport_destination") == 0)
		{
			entity.ent_type = ENT_Q1_INFO_TELEPORT_DESTINATION;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "info_intermission") == 0)
		{
			entity.ent_type = ENT_Q1_INFO_INTERMISSION;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "info_player_coop") == 0)
		{
			entity.ent_type = ENT_Q1_INFO_PLAYER_COOP;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "trigger_once") == 0)
		{
			entity.ent_type = ENT_TRIGGER_ONCE;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "trigger_counter") == 0)
		{
			entity.ent_type = ENT_Q1_TRIGGER_COUNTER;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "trigger_secret") == 0)
		{
			entity.ent_type = ENT_Q1_TRIGGER_SECRET;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "path_corner") == 0)
		{
			entity.ent_type = ENT_PATH_CORNER;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
//				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = false;
		}
		else if (strcmp(value, "item_spikes") == 0)
		{
			entity.ent_type = ENT_Q1_ITEM_SPIKES;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "item_artifact_super_damage") == 0)
		{
			entity.ent_type = ENT_Q1_ITEM_ARTIFACT_SUPER_DAMAGE;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "item_artifact_invulnerability") == 0)
		{
			entity.ent_type = ENT_Q1_ITEM_ARTIFACT_INVULNERABILITY;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "item_artifact_envirosuit") == 0)
		{
			entity.ent_type = ENT_Q1_ITEM_ARTIFACT_ENVIROSUIT;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "weapon_nailgun") == 0)
		{
			entity.ent_type = ENT_Q1_WEAPON_NAILGUN;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = false;

			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
			entity.trigger->timeout_value = WEAPON_TIMEOUT;


			snprintf(entity.trigger->action, LINE_SIZE, "weapon_machinegun");
		}
		else if (strcmp(value, "weapon_supernailgun") == 0)
		{
			entity.ent_type = ENT_Q1_WEAPON_SUPERNAILGUN;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = false;


			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
			entity.trigger->timeout_value = WEAPON_TIMEOUT;


			snprintf(entity.trigger->action, LINE_SIZE, "weapon_plasma");
		}
		else if (strcmp(value, "weapon_supershotgun") == 0)
		{
			entity.ent_type = ENT_Q1_WEAPON_SUPERSHOTGUN;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = false;


			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
			entity.trigger->timeout_value = WEAPON_TIMEOUT;

			snprintf(entity.trigger->action, LINE_SIZE, "weapon_shotgun");
		}
		else if (strcmp(value, "misc_explobox") == 0)
		{
			entity.ent_type = ENT_Q1_MISC_EXPLOBOX;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "ambient_comp_hum") == 0)
		{
			entity.ent_type = ENT_Q1_AMBIENT_COMP_HUM;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "ambient_drone") == 0)
		{
			entity.ent_type = ENT_Q1_AMBIENT_DRONE;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "ambient_drip") == 0)
		{
			entity.ent_type = ENT_Q1_AMBIENT_DRIP;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "ambient_swamp1") == 0)
		{
			entity.ent_type = ENT_Q1_AMBIENT_SWAMP1;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "ambient_swamp2") == 0)
		{
			entity.ent_type = ENT_Q1_AMBIENT_SWAMP2;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "func_wall") == 0)
		{
			entity.ent_type = ENT_Q1_FUNC_WALL;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "light_torch_small_walltorch") == 0)
		{
			entity.ent_type = ENT_Q1_LIGHT_TORCH_SMALL_WALLTORCH;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "light_flame_small_yellow") == 0)
		{
			entity.ent_type = ENT_Q1_LIGHT_FLAME_SMALL_YELLOW;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "item_key1") == 0)
		{
			entity.ent_type = ENT_Q1_ITEM_KEY1;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "item_key2") == 0)
		{
			entity.ent_type = ENT_Q1_ITEM_KEY2;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "item_armor1") == 0)
		{
			entity.ent_type = ENT_Q1_ITEM_ARMOR1;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;

			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/misc/ar2_pkup.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");

			snprintf(entity.trigger->action, LINE_SIZE, "armor 50");
			entity.trigger->armor = true;
		}
		else if (strcmp(value, "item_armor2") == 0)
		{
			entity.ent_type = ENT_Q1_ITEM_ARMOR2;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;

			entity.trigger = new Trigger(&entity, audio);
			entity.trigger->pickup_index = engine->get_load_wave("sound/misc/ar2_pkup.wav");
			entity.trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");

			snprintf(entity.trigger->action, LINE_SIZE, "armor 100");
			entity.trigger->armor = true;

		}
		else if (strcmp(value, "trap_spikeshooter") == 0)
		{
			entity.ent_type = ENT_Q1_TRAP_SPIKESHOOTER;

			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
	}
	else if (strcmp(key, "light") == 0)
	{
		int inten;
		float intensity;

		if (entity.light == NULL)
		{
			entity.light = new Light(&entity, gfx, light_num++, engine->res_scale);
		}
		sscanf(value, "%d", &inten);
		intensity = (float)inten;

		while (intensity < 750.0f)
			intensity *= 2.0f;

		while (intensity > 1750.0f)
			intensity /= 2.0f;

		// uncomment to draw light positions
//		entity.nodraw = true;

		if (entity.light)
			entity.light->intensity = intensity;
	}
	else if (strcmp(key, "_color") == 0)
	{
		float r, g, b;

		if (entity.light == NULL)
		{
			// generates cubemaps for light, this sucks up memory on large maps
			entity.light = new Light(&entity, gfx, light_num++, engine->res_scale);
		}
		sscanf(value, "%f %f %f", &r, &g, &b);
		if (entity.light)
		{
			vec3 color_temp = vec3(r, g, b);

			color_temp.normalize();
			entity.light->color = color_temp;
		}
	}
	else if (strcmp(key, "music") == 0)
	{
		entity.speaker = new Speaker(&entity, audio);
		entity.speaker->gain(0.3f);
		entity.speaker->loop_gain(0.3f);
		entity.speaker->index = engine->get_load_wave(value);
	}
	else if (strcmp(key, "map") == 0)
	{
		if (entity.trigger == NULL)
			entity.trigger = new Trigger(&entity, audio);

		// changel level, should only exist in trigger_changelevel
		sprintf(entity.trigger->action, "map maps/%s.bsp", value);
	}
	else if (strcmp(key, "message") == 0)
	{
		if (entity.trigger == NULL)
			entity.trigger = new Trigger(&entity, audio);

		// print message when triggered, quake1 style
		sprintf(entity.trigger->message, "%s", value);
	}
	else if (strcmp(key, "noise") == 0)
	{
		if (entity.trigger == NULL)
			entity.trigger = new Trigger(&entity, audio);
		entity.trigger->respawn_index = engine->get_load_wave(value);
		entity.trigger->noise = true;

		strcpy(entity.trigger->noise_str, value);
		if (entity.trigger->timeout_value < 0.001f)
		{
			if (strstr("*falling", key))
			{
				entity.trigger->timeout = 1.0f;
				entity.trigger->timeout_value = 1.0f;
			}
			else
			{
				if (strlen(entity.target_name) > 1)
				{
					// suppose to be triggered
					entity.trigger->noise = false;
					entity.trigger->timeout = 15.0f;
					entity.trigger->timeout_value = 15.0f;
				}
				else
				{
					entity.trigger->timeout = 15.0f;
					entity.trigger->timeout_value = 15.0f;
				}
			}
		}
		entity.trigger->active = true;
	}
	else if (strcmp(key, "wait") == 0)
	{
		if (entity.trigger == NULL)
			entity.trigger = new Trigger(&entity, audio);
		entity.trigger->timeout_value = (float)atoi(value);
	}
	else if (strcmp(key, "target") == 0)
	{
		strcpy(entity.target, value);
	}
	else if (strcmp(key, "angle") == 0)
	{
		entity.angle = atoi(value);
	}
	else if (strcmp(key, "targetname") == 0)
	{
		strcpy(entity.target_name, value);
	}
	else if (strcmp(key, "dmg") == 0)
	{
		if (entity.trigger == NULL)
			entity.trigger = new Trigger(&entity, audio);
		sprintf(entity.trigger->action, "damage %s", value);
	}
	else if (strcmp(key, "speed") == 0)
	{
		// Need to loop through all entities after parsing and set func_train targets and will use pursue()
	}
	else if (strcmp(key, "height") == 0)
	{
		if (atoi(value) != 0)
			entity.func_height = (float)atoi(value);
		else
			entity.func_height = 0.0f;
//		entity.rigid->velocity = vec3(0.0f, atoi(value), 0.0f);
	}

}

bool parse_entity(Engine *engine, const char *input, vector<Entity *> &entity_list, Graphics &gfx, Audio &audio)
{
	Entity *entity = NULL;
	char state = 'S';
	char prevstate = 'S';
	char key[LINE_SIZE], val[LINE_SIZE];
	char stack[LINE_SIZE] = {0};
	int sp = 0;
	unsigned int i, j = 0;
	unsigned int input_length = strlen(input);

	for(i = 0; state != 'F' && state != '\0' && i < input_length; i++)
	{		
		prevstate = state;
		state = machine_entity(state, input[i], stack, sp);

		switch (state)
		{
		case 'K': //key
			if (prevstate == 'K')
			{
				key[j++] = input[i];
				key[j] = '\0';
			}
			break;
		case 'V': //value
			if (prevstate == 'V')
			{
				val[j++] = input[i];
				val[j] = '\0';
			}
			break;
		case 'A':
			if (prevstate == 'S')
			{
				entity = NULL;
				entity = new Entity();
				entity->rigid = new RigidBody(entity);
				entity->model = entity->rigid;

				entity_list.push_back(entity);
			}
			j = 0;
			break;
		case 'B':
			j = 0;
			break;
		case 'R':
			i--;
			add_key(engine, *entity, key, val, gfx, audio, entity_list.size() - 1);
			break;
		}
	}

	if (state == 'F' && sp == 0)
		return true;
	else
		return false;
}

char machine_entity(char state, char input, char *stack, int &sp)
{
	switch (state)
	{
	case 'S':
		switch (input)
		{
		case '{':
			push('}', stack, sp);
			return 'A';
		case '\n':
			return 'S';
		case '\0':
			return 'F';
		}
		return '\0';

	case 'A':
		switch (input)
		{
		case '"':
			push('"', stack, sp);
			return 'K';
		case '}':
			pop('}', stack, sp);
			return 'S';
		case ' ':
			return 'A';
		case '\n':
			return 'A';
		case '\t':
			return 'A';
		case '\r':
			return 'A';
		}
		return '\0';

	case 'B':
		switch (input)
		{
		case '"':
			push('"', stack, sp);
			return 'V';
		case ' ':
			return 'B';
		case '\t':
			return 'B';
		case '\r':
			return 'B';
		case '\n':
			return 'B';
		}
		return '\0';

	case 'K':
		switch (input)
		{
		case '"':
			pop('"', stack, sp);
			return 'B';
		}
		return 'K';

	case 'V':
		switch (input)
		{
		case '"':
			pop('"', stack, sp);
			return 'R';

		}
		return 'V';

	case 'R':
		return 'A';

	case 'F':
		return 'F';
	}
	return '\0';
}

// These are state machines from a piece of paper, pretty hard to decipher without the state diagram
char machine_shader(char input, char state)
{
//	printf("State: %c Input: %c\n", state, input);

	switch (state)
	{
	case 'Y':
		switch (input)
		{
		case '/':
			return 'A';
		case '\n':
			return 'Y';
		}
		return 'S';
	case 'S':

		switch (input)
		{
		case '/':
			return 'A';
		case '\n':
			return 'C';
		}
		return 'S';
	case 'A':
		switch (input)
		{
		case '/':
			return 'B';
		}
		return 'S';
	case 'B':
		switch (input)
		{
		case '\n':
			return 'Z';
		}
		return 'B';
	case 'Z':
		switch (input)
		{
		case '\n':
			return 'Z';
		case ' ':
			return 'Z';
		case '\t':
			return 'Z';
		case '/':
			return 'A';
		default:
			return 'S';
		}
		return 'Z';
	case 'C':
		switch (input)
		{
		case '/':
			return 'D';
		case '{':
			return 'F';
		case '}':
			return 'S';
		}
		return 'C';
	case 'D':
		switch (input)
		{
		case '/':
			return 'E';
		}
		return 'C';
	case 'E':
		switch (input)
		{
		case '\n':
			return 'C';
		}
		return 'E';
	case 'F':
		switch (input)
		{
		case '/':
			return 'G';
		case '}':
			return 'Y';
		case '{':
			return 'I';
		}
		return 'F';
	case 'G':
		switch (input)
		{
		case '/':
			return 'H';
		}
		return 'F';
	case 'H':
		switch (input)
		{
		case '\n':
			return 'F';
		}
		return 'H';
	case 'I':
		switch (input)
		{
		case '}':
			return 'F';
		case '/':
			return 'J';
		}
		return 'I';
	case 'J':
		switch (input)
		{
		case '/':
			return 'K';
		}
		return 'I';
	case 'K':
		switch (input)
		{
		case '\n':
			return 'I';
		}
		return 'K';
	}

	return 'Z';
}

void handle_command(char *basecmd, surface_t *surface)
{
	char *ret = NULL;

	if (basecmd == NULL || surface == NULL)
		return;

	if (strstr(basecmd, "nomipmaps"))
	{
		surface->nomipmaps = true;
	}
	else if (strstr(basecmd, "nopicmip"))
	{
		surface->nopicmip = true;
	}
	else if (strstr(basecmd, "polygonOffset"))
	{
		surface->polygon_offset = true;
	}
	else if (strstr(basecmd, "portal"))
	{
		surface->portal = true;
	}
	else if (strstr(basecmd, "surfaceparm"))
	{
		if (strstr(basecmd, "trans"))
			surface->surfaceparm_trans = true;
		else if (strstr(basecmd, "nonsolid"))
			surface->surfaceparm_nonsolid = true;
		else if (strstr(basecmd, "water"))
			surface->surfaceparm_water = true;
		else if (strstr(basecmd, "slime"))
			surface->surfaceparm_slime = true;
		else if (strstr(basecmd, "lava"))
			surface->surfaceparm_lava = true;
		else if (strstr(basecmd, "slick"))
			surface->surfaceparm_slick = true;
		else if (strstr(basecmd, "structural"))
			surface->surfaceparm_structural = true;
		else if (strstr(basecmd, "fog"))
			surface->surfaceparm_fog = true;
		else if (strstr(basecmd, "sky"))
			surface->surfaceparm_sky = true;
		else if (strstr(basecmd, "nolightmap"))
			surface->surfaceparm_nolightmap = true;
		else if (strstr(basecmd, "nodamage"))
			surface->surfaceparm_nodamage = true;
		else if (strstr(basecmd, "noimpact"))
			surface->surfaceparm_noimpact = true;
		else if (strstr(basecmd, "nomarks"))
			surface->surfaceparm_nomarks = true;
		else if (strstr(basecmd, "nodraw"))
			surface->surfaceparm_nodraw = true;
		else if (strstr(basecmd, "nodrop"))
			surface->surfaceparm_nodrop = true;
		else if (strstr(basecmd, "nodlight"))
			surface->surfaceparm_nodlight = true;
		else if (strstr(basecmd, "clusterportal"))
			surface->surfaceparm_clusterportal = true;
		else if (strstr(basecmd, "donotenter"))
			surface->surfaceparm_donotenter = true;
		else if (strstr(basecmd, "origin"))
			surface->surfaceparm_clusterportal = true;
		else if (strstr(basecmd, "areaportal"))
			surface->surfaceparm_areaportal = true;
		else if (strstr(basecmd, "detail"))
			surface->surfaceparm_detail = true;
		else if (strstr(basecmd, "playerclip"))
			surface->surfaceparm_playerclip = true;
		else if (strstr(basecmd, "metalsteps"))
			surface->surfaceparm_metalsteps = true;
		else if (strstr(basecmd, "alphashadow"))
			surface->surfaceparm_alphashadow = true;
		else
			printf("Unknown surfaceparm [%s]\n", basecmd);
	}
	else if ( (ret = strstr(basecmd, "q3map_surfacelight")) )
	{
		surface->q3map_surfacelight = true;
		surface->q3map_surfacelight_value = atoi(ret + 18);
	}
	else if ((ret = strstr(basecmd, "q3map_sun")))
	{
		int match = 0;
		float r, g, b;
		float intensity;
		float degrees;
		float elevation;

//		surface->q3map_surfacelight_value = atoi(ret + 18);
		match = sscanf(ret, "q3map_sun %f %f %f %f %f %f",
			&r, &g, &b, &intensity, &degrees, &elevation);

		if (match == 6)
		{
			surface->q3map_sun = true;
			surface->q3map_sun_value[0].x = r;
			surface->q3map_sun_value[0].y = g;
			surface->q3map_sun_value[0].z = b;
			surface->q3map_sun_value[1].x = intensity;
			surface->q3map_sun_value[1].y = degrees;
			surface->q3map_sun_value[1].z = elevation;
		}
	}
	else if (strstr(basecmd, "cull"))
	{
		if (strstr(basecmd, "disable"))
			surface->cull_disable = true;
		else if (strstr(basecmd, "none"))
			surface->cull_none = true;
		else if (strstr(basecmd, "backside"))
			surface->cull_backside = true;
		else if (strstr(basecmd, "twosided"))
			surface->cull_twosided = true;
	}
	else if ( (ret = strstr(basecmd, "deformVertexes") ))
	{
		char wave[32] = { 0 };
		float div, func, base, amplitude, phase, freq;
		int match = 0;
		//deformVertexes wave <div> <func> <base> <amplitude> <phase> <freq>

		match = sscanf(ret, "deformVertexes %s %f %f %f %f %f %f",
			&wave[0], &div, &func, &base, &amplitude, &phase, &freq);
		if (match == 7)
		{
			surface->deformVertexes = true;
			strcpy(surface->deform.wave, wave);
			surface->deform.div = div;
			surface->deform.func = func;
			surface->deform.base = base;
			surface->deform.amplitude = amplitude;
			surface->deform.phase = phase;
			surface->deform.freq = freq;
		}
	}
}
/*
vec3 q3map_sun_value[2]; //rgb + intensity degrees elevation
bool deformVertexes;
*/


void handle_stage(char *stagecmd, stage_t *stage, int &stage_num)
{
	char *ret = NULL;


	

	if (strstr(stagecmd, "$lightmap"))
	{
		stage->lightmap = true;
//		if (stage_num - 1 > 0)
//			stage->blendfunc_filter = true;
	}
	else if ((ret = strstr(stagecmd, "animmap ")))
	{
		int match = 0;
		char tex[512] = { 0 };
		float freq;

		match = sscanf(ret, "animmap %f %s", &freq, tex);

		if (match == 2)
		{
			stage->anim_map = true;
			stage->anim_map_freq = freq;
			strcpy(stage->anim_map_tex, ret + 8);
		}
	}
	else if ((ret = strstr(stagecmd, "clampmap ")))
	{
		stage->clampmap = true;
		strcpy(stage->clampmap_tex, ret + 9);
	}
	else if ( (ret = strstr(stagecmd, "map ")) )
	{
		stage->map = true;
		strcpy(stage->map_tex, ret + 4);
	}
	else if ((ret = strstr(stagecmd, "blendfunc")))
	{
		if (strstr(ret, "add"))
		{
			stage->blendfunc_add = true;
		}
		else if (strstr(ret, "filter"))
		{
			stage->blendfunc_filter = true;
		}
		else if (strstr(ret, "blendfunc blend"))
		{
			stage->blendfunc_blend = true;
		}
		else if (strstr(ret, "blendfunc gl_src_alpha gl_one_minus_src_alpha"))
		{
			stage->blendfunc_blend = true;
		}
		else if (strstr(ret, "gl_zero gl_one"))
		{
			stage->blend_zero_one = true;
		}
		else if (strstr(ret, "gl_one gl_zero"))
		{
			stage->blend_one_zero = true;
		}
		else if (strstr(ret, "gl_one gl_one"))
		{
			stage->blend_one_one = true;
		}
		else if (strstr(ret, "gl_one gl_src_color"))
		{
			stage->blend_one_src_color = true;
		}
		else if (strstr(ret, "gl_dst_color gl_zero"))
		{
			stage->blend_dst_color_zero = true;
		}
		else if (strstr(ret, "gl_dst_color gl_one_minus_dst_alpha"))
		{
			stage->blend_dst_color_one_minus_dst_alpha = true;
		}
		else if (strstr(ret, "gl_dst_color gl_one"))
		{
			stage->blend_dst_color_one = true;
		}
		else if (strstr(ret, "gl_dst_color gl_src_alpha"))
		{
			stage->blend_dst_color_src_alpha = true;
		}
		else if (strstr(ret, "gl_one_minus_src_alpha gl_src_alpha"))
		{
			stage->blend_one_minus_src_alpha_src_alpha = true;
		}
		else if (strstr(ret, "gl_src_alpha gl_one_minus_src_alpha"))
		{
			stage->blend_src_alpha_one_minus_src_alpha = true;
		}
		else if (strstr(ret, "gl_one gl_src_alpha"))
		{
			stage->blend_one_src_alpha = true;
		}
		else if (strstr(ret, "gl_one_minus_dst_color gl_zero"))
		{
			stage->blend_one_minus_dst_color_zero = true;
		}
		else if (strstr(ret, "gl_zero gl_src_color"))
		{
			stage->blend_zero_src_color = true;
		}
		else if (strstr(ret, "gl_dst_color gl_src_color"))
		{
			stage->blend_dst_color_src_color = true;
		}
		else if (strstr(ret, "gl_zero gl_src_alpha"))
		{
			stage->blend_zero_src_alpha = true;
		}
		else
		{
			printf("Unsupported blendfunc %s\n", ret);
		}
	}
	else if (strstr(stagecmd, "alphafunc"))
	{
		//alphaFunc GE128
		if (strstr(stagecmd, "gt0") != NULL)
			stage->alpha_gt0 = true;
		else if (strstr(stagecmd, "ge128") != NULL)
			stage->alpha_ge128 = true;
		else if (strstr(stagecmd, "lt128") != NULL)
			stage->alpha_lt128 = true;
		else
			printf("Unsupported alpha func %s\n", stagecmd);

	}
	else if (strstr(stagecmd, "skyparms"))
	{
		char farbox[256];
		char cloudheight[64];
		char nearbox[64];
		sscanf(stagecmd, "skyparms %s %s %s", &farbox[0], &cloudheight[0], &nearbox[0]);
	}
	else if (strstr(stagecmd, "depthwrite"))
	{
		stage->depth_write = true;
	}
	else if ( (ret = strstr(stagecmd, "tcmod")) )
	{
		int match = 0;
		float value;
		float x, y;
		float base, amplitude, phase, freq;

		match = sscanf(ret, "tcmod rotate %f", &value);
		if (match == 1)
		{
			stage->tcmod_rotate = true;
			stage->tcmod_rotate_value = value;
			return;
		}

		match = sscanf(ret, "tcmod scale %f %f", &x, &y);
		if (match == 2)
		{
			stage->tcmod_scale = true;
			stage->tcmod_scale_value.x = x;
			stage->tcmod_scale_value.y = y;
			return;
		}

		match = sscanf(ret, "tcmod scroll %f %f", &x, &y);
		if (match == 2)
		{
			stage->tcmod_scroll = true;
			stage->tcmod_scroll_value.x = x;
			stage->tcmod_scroll_value.y = y;
			return;
		}

		match = sscanf(ret, "tcmod turb %f %f %f %f", &base, &amplitude, &phase, &freq);
		if (match == 4)
		{
			stage->tcmod_turb = true;
			stage->tcmod_turb_value.x = base;
			stage->tcmod_turb_value.y = amplitude;
			stage->tcmod_turb_value.z = phase;
			stage->tcmod_turb_value.w = freq;
			return;
		}

		match = sscanf(ret, "tcmod stretch sin %f %f %f %f", &base, &amplitude, &phase, &freq);
		if (match == 4)
		{
			stage->tcmod_stretch_sin = true;
			stage->tcmod_stretch_value.x = base;
			stage->tcmod_stretch_value.y = amplitude;
			stage->tcmod_stretch_value.z = phase;
			stage->tcmod_stretch_value.w = freq;
			return;
		}

		match = sscanf(ret, "tcmod stretch square  %f %f %f %f", &base, &amplitude, &phase, &freq);
		if (match == 4)
		{
			stage->tcmod_stretch_square = true;
			stage->tcmod_stretch_value.x = base;
			stage->tcmod_stretch_value.y = amplitude;
			stage->tcmod_stretch_value.z = phase;
			stage->tcmod_stretch_value.w = freq;
			return;
		}

		match = sscanf(ret, "tcmod stretch triangle %f %f %f %f", &base, &amplitude, &phase, &freq);
		if (match == 4)
		{
			stage->tcmod_stretch_triangle = true;
			stage->tcmod_stretch_value.x = base;
			stage->tcmod_stretch_value.y = amplitude;
			stage->tcmod_stretch_value.z = phase;
			stage->tcmod_stretch_value.w = freq;
			return;
		}

		match = sscanf(ret, "tcmod stretch sawtooth %f %f %f %f", &base, &amplitude, &phase, &freq);
		if (match == 4)
		{
			stage->tcmod_stretch_sawtooth = true;
			stage->tcmod_stretch_value.x = base;
			stage->tcmod_stretch_value.y = amplitude;
			stage->tcmod_stretch_value.z = phase;
			stage->tcmod_stretch_value.w = freq;
			return;
		}

		match = sscanf(ret, "tcmod stretch inversesawtooth %f %f %f %f", &base, &amplitude, &phase, &freq);
		if (match == 4)
		{
			stage->tcmod_stretch_inverse_sawtooth = true;
			stage->tcmod_stretch_value.x = base;
			stage->tcmod_stretch_value.y = amplitude;
			stage->tcmod_stretch_value.z = phase;
			stage->tcmod_stretch_value.w = freq;
			return;
		}

	}
	else if (strstr(stagecmd, "tcgen environment"))
	{
		stage->tcgen_env = true;
	}
	else if ((ret = strstr(stagecmd, "rgbgen")))
	{
		int match = 0;
		float base, amplitude, phase, freq;

		if (strstr(stagecmd, "identity"))
		{
			stage->rgbgen_identity = true;
			return;
		}

		match = sscanf(ret, "rgbgen wave sin %f %f %f %f", &base, &amplitude, &phase, &freq);
		if (match == 4)
		{
			stage->rgbgen_wave_sin = true;
			stage->rgbgen_wave_value.x = base;
			stage->rgbgen_wave_value.y = amplitude;
			stage->rgbgen_wave_value.z = phase;
			stage->rgbgen_wave_value.w = freq;
			return;
		}

		match = sscanf(ret, "rgbgen wave square  %f %f %f %f", &base, &amplitude, &phase, &freq);
		if (match == 4)
		{
			stage->rgbgen_wave_square = true;
			stage->rgbgen_wave_value.x = base;
			stage->rgbgen_wave_value.y = amplitude;
			stage->rgbgen_wave_value.z = phase;
			stage->rgbgen_wave_value.w = freq;
			return;
		}

		match = sscanf(ret, "rgbgen wave triangle %f %f %f %f", &base, &amplitude, &phase, &freq);
		if (match == 4)
		{
			stage->rgbgen_wave_triangle = true;
			stage->rgbgen_wave_value.x = base;
			stage->rgbgen_wave_value.y = amplitude;
			stage->rgbgen_wave_value.z = phase;
			stage->rgbgen_wave_value.w = freq;
			return;
		}

		match = sscanf(ret, "rgbgen wave sawtooth %f %f %f %f", &base, &amplitude, &phase, &freq);
		if (match == 4)
		{
			stage->rgbgen_wave_sawtooth = true;
			stage->rgbgen_wave_value.x = base;
			stage->rgbgen_wave_value.y = amplitude;
			stage->rgbgen_wave_value.z = phase;
			stage->rgbgen_wave_value.w = freq;
			return;
		}

		match = sscanf(ret, "rgbgen wave inversesawtooth %f %f %f %f", &base, &amplitude, &phase, &freq);
		if (match == 4)
		{
			stage->rgbgen_wave_inverse_sawtooth = true;
			stage->rgbgen_wave_value.x = base;
			stage->rgbgen_wave_value.y = amplitude;
			stage->rgbgen_wave_value.z = phase;
			stage->rgbgen_wave_value.w = freq;
			return;
		}

	}
}


void parse_shader(char *input, vector<surface_t *> &surface_list, char *filename)
{
	char		basecmd[512] = { 0 };
	char		stagecmd[512] = { 0 };
	int			old_pos = 0;
	int			num_stage = 0;
	int			j = 0;
	const int	length = strlen(input);
	char		state = 'S';
	char		prevstate = 'S';


	surface_t *surface = new surface_t;
	memset(surface, 0, sizeof(surface_t));
	sprintf(surface->file, "%s", filename);
	surface_list.push_back(surface);


	for (int i = 0; i < length; i++)
	{
		prevstate = state;

		//completely skip line returns
		if (input[i] == '\r')
		{
			input[i] = '\n';
			continue;
		}

		// Force lower case
		input[i] = tolower(input[i]);


		state = machine_shader(input[i], state);

		switch (state)
		{
		case 'B':
		case 'E':
		case 'H':
		case 'K':
			old_pos = i;
			break;
		case 'Z':
			old_pos = i;
			break;

		case 'S':
			if (prevstate == 'F' || prevstate == 'B' || prevstate == 'E' || prevstate == 'H' || prevstate == 'K')
			{
				old_pos = i;
			}
			if (prevstate == 'Z' || prevstate == 'Y')
			{
				old_pos = i - 1;
				i--;
			}
			break;
		case 'C':
			if (prevstate == 'S')
			{
				if (surface->num_stage > 4)
					surface->num_stage = 4;

				surface = new surface_t;
				memset(surface, 0, sizeof(surface_t));
				sprintf(surface->file, "%s",  filename);
				surface_list.push_back(surface);


				memset(surface->name, 0, sizeof(surface->name));
				memcpy(surface->name, &input[old_pos + 1], i - old_pos - 1);
				if (i - old_pos - 2 > 0)
					surface->name[i - old_pos - 2] = '\0';
				else
					surface->name[0] = '\0';
				old_pos = i;
				num_stage = 0;
				j = 0;
			}
			break;
		case 'F':
			if (input[i] == '\n' && input[i - 1] != '{')
			{
				basecmd[j++] = '\0';
				handle_command(basecmd, surface);
				old_pos = i;
				j = 0;
			}
			if (input[i - 1] == '{')
			{
				j = 0;
			}

			if (input[i] != '\n' && input[i] != '{' && input[i-1] != '{' && input[i] != '\t')
				basecmd[j++] = input[i];
			break;
		case 'I':
			if (input[i] == '\n' && input[i - 1] != '{')
			{
				memcpy(stagecmd, &input[old_pos + 1], i - old_pos - 1);
				stagecmd[i - old_pos - 2] = '\0';

				if (strchr(stagecmd, '{') != NULL)
				{
					surface->num_stage++;
					num_stage++;
				}
				handle_stage(stagecmd, &(surface->stage[num_stage - 1]), num_stage);
				old_pos = i;
				j = 0;
			}
			if (input[i - 1] == '{')
			{
				j = 0;
			}
			if (prevstate == 'J')
				stagecmd[j++] = '/';
			if (input[i] != '\n' && input[i] != '{')
				stagecmd[j++] = input[i];
			break;
		}
	}

}

void push(char input, char *stack, int &sp)
{
	stack[sp] = input;
	sp++;
}

void pop(char input, char *stack, int &sp)
{
	if (stack[sp - 1] == input)
		sp--;
}
