#include "include.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include <ctype.h>

void add_key(Entity &entity, char *key, char *value, Graphics &gfx, Audio &audio, int entity_num)
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
		if (strstr(value, "*") != NULL)
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
			entity.trigger = new Trigger(&entity, audio);
			snprintf(entity.trigger->pickup_sound, LINE_SIZE, "sound/items/s_health.wav");
			snprintf(entity.trigger->respawn_sound, LINE_SIZE, "sound/items/s_health.wav");
			//snprintf(entity.trigger->respawn_sound, LINE_SIZE, "sound/items/respawn1.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "armor 5");
		}
		else if (strcmp(value, "item_health") == 0)
		{
			entity.trigger = new Trigger(&entity, audio);
			snprintf(entity.trigger->pickup_sound, LINE_SIZE, "sound/items/n_health.wav");
			snprintf(entity.trigger->respawn_sound, LINE_SIZE, "sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "health 25");
			entity.trigger->health = true;
		}
		else if (strcmp(value, "item_health_large") == 0)
		{
			entity.trigger = new Trigger(&entity, audio);
			snprintf(entity.trigger->pickup_sound, LINE_SIZE, "sound/items/l_health.wav");
			snprintf(entity.trigger->respawn_sound, LINE_SIZE, "sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "health 50");
			entity.trigger->health = true;
		}
		else if (strcmp(value, "item_health_mega") == 0)
		{
			entity.trigger = new Trigger(&entity, audio);
			snprintf(entity.trigger->pickup_sound, LINE_SIZE, "sound/items/m_health.wav");
			snprintf(entity.trigger->respawn_sound, LINE_SIZE, "sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "health 100");
			entity.trigger->health = false;
		}
		else if (strcmp(value, "item_quad") == 0)
		{
			entity.trigger = new Trigger(&entity, audio);
			snprintf(entity.trigger->pickup_sound, LINE_SIZE, "sound/items/quaddamage.wav");
			snprintf(entity.trigger->respawn_sound, LINE_SIZE, "sound/items/poweruprespawn.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "quaddamage");
			entity.trigger->health = false;
		}
		else if (strcmp(value, "holdable_medkit") == 0)
		{
			entity.trigger = new Trigger(&entity, audio);
			snprintf(entity.trigger->pickup_sound, LINE_SIZE, "sound/items/holdable.wav");
			snprintf(entity.trigger->respawn_sound, LINE_SIZE, "sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "holdable_medkit");
			entity.trigger->health = false;
		}
		else if (strcmp(value, "holdable_teleporter") == 0)
		{
			entity.trigger = new Trigger(&entity, audio);
			snprintf(entity.trigger->pickup_sound, LINE_SIZE, "sound/items/holdable.wav");
			snprintf(entity.trigger->respawn_sound, LINE_SIZE, "sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "holdable_teleporter");
			entity.trigger->health = false;
		}
		else if (strcmp(value, "item_enviro") == 0)
		{
			entity.trigger = new Trigger(&entity, audio);
			snprintf(entity.trigger->pickup_sound, LINE_SIZE, "sound/items/protect.wav");
			snprintf(entity.trigger->respawn_sound, LINE_SIZE, "sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "bodysuit");
			entity.trigger->health = false;
		}
		else if (strcmp(value, "item_flight") == 0)
		{
			entity.trigger = new Trigger(&entity, audio);
			snprintf(entity.trigger->pickup_sound, LINE_SIZE, "sound/items/holdable.wav");
			snprintf(entity.trigger->respawn_sound, LINE_SIZE, "sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "flight 30");
			entity.trigger->health = false;
		}
		else if (strcmp(value, "item_haste") == 0)
		{
			entity.trigger = new Trigger(&entity, audio);
			snprintf(entity.trigger->pickup_sound, LINE_SIZE, "sound/items/haste.wav");
			snprintf(entity.trigger->respawn_sound, LINE_SIZE, "sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "haste");
			entity.trigger->health = false;
		}
		else if (strcmp(value, "item_invis") == 0)
		{
			entity.trigger = new Trigger(&entity, audio);
			snprintf(entity.trigger->pickup_sound, LINE_SIZE, "sound/items/invisibility.wav");
			snprintf(entity.trigger->respawn_sound, LINE_SIZE, "sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "invisibility");
			entity.trigger->health = false;
		}
		else if (strcmp(value, "item_regen") == 0)
		{
			entity.trigger = new Trigger(&entity, audio);
			snprintf(entity.trigger->pickup_sound, LINE_SIZE, "sound/items/regeneration.wav");
			snprintf(entity.trigger->respawn_sound, LINE_SIZE, "sound/items/poweruprespawn.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "regeneration");
			entity.trigger->health = false;
		}
		else if (strcmp(value, "team_CTF_blueflag") == 0)
		{
			entity.trigger = new Trigger(&entity, audio);
			snprintf(entity.trigger->pickup_sound, LINE_SIZE, "sound/teamplay/flagtk_blu.wav");
			snprintf(entity.trigger->respawn_sound, LINE_SIZE, "sound/items/poweruprespawn.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "blueflag");
			entity.trigger->health = false;
		}
		else if (strcmp(value, "team_CTF_redflag") == 0)
		{
			entity.trigger = new Trigger(&entity, audio);
			snprintf(entity.trigger->pickup_sound, LINE_SIZE, "sound/teamplay/flagtk_red.wav");
			snprintf(entity.trigger->respawn_sound, LINE_SIZE, "sound/items/poweruprespawn.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "redflag");
			entity.trigger->health = false;
		}
		else if (strcmp(value, "ammo_bullets") == 0)
		{
			entity.trigger = new Trigger(&entity, audio);
			snprintf(entity.trigger->pickup_sound, LINE_SIZE, "sound/misc/am_pkup.wav");
			snprintf(entity.trigger->respawn_sound, LINE_SIZE, "sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "ammo_bullets 25");
		}
		else if (strcmp(value, "ammo_rockets") == 0)
		{
			entity.trigger = new Trigger(&entity, audio);
			snprintf(entity.trigger->pickup_sound, LINE_SIZE, "sound/misc/am_pkup.wav");
			snprintf(entity.trigger->respawn_sound, LINE_SIZE, "sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "ammo_rockets 10");
		}
		else if (strcmp(value, "ammo_slugs") == 0)
		{
			entity.trigger = new Trigger(&entity, audio);
			snprintf(entity.trigger->pickup_sound, LINE_SIZE, "sound/misc/am_pkup.wav");
			snprintf(entity.trigger->respawn_sound, LINE_SIZE, "sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "ammo_slugs 10");
		}
		else if (strcmp(value, "ammo_shells") == 0)
		{
			entity.trigger = new Trigger(&entity, audio);
			snprintf(entity.trigger->pickup_sound, LINE_SIZE, "sound/misc/am_pkup.wav");
			snprintf(entity.trigger->respawn_sound, LINE_SIZE, "sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "ammo_shells 10");
		}
		else if (strcmp(value, "ammo_lightning") == 0)
		{
			entity.trigger = new Trigger(&entity, audio);
			snprintf(entity.trigger->pickup_sound, LINE_SIZE, "sound/misc/am_pkup.wav");
			snprintf(entity.trigger->respawn_sound, LINE_SIZE, "sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "ammo_lightning 10");
		}
		else if (strcmp(value, "ammo_plasma") == 0)
		{
			entity.trigger = new Trigger(&entity, audio);
			snprintf(entity.trigger->pickup_sound, LINE_SIZE, "sound/misc/am_pkup.wav");
			snprintf(entity.trigger->respawn_sound, LINE_SIZE, "sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "ammo_plasma 10");
		}
		else if (strstr(value, "weapon_rocketlauncher"))
		{
			entity.trigger = new Trigger(&entity, audio);
			snprintf(entity.trigger->pickup_sound, LINE_SIZE, "sound/misc/w_pkup.wav");
			snprintf(entity.trigger->respawn_sound, LINE_SIZE, "sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "weapon_rocketlauncher");
		}
		else if (strstr(value, "weapon_lightning"))
		{
			entity.trigger = new Trigger(&entity, audio);
			snprintf(entity.trigger->pickup_sound, LINE_SIZE, "sound/misc/w_pkup.wav");
			snprintf(entity.trigger->respawn_sound, LINE_SIZE, "sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "weapon_lightning");
		}
		else if (strstr(value, "weapon_shotgun"))
		{
			entity.trigger = new Trigger(&entity, audio);
			snprintf(entity.trigger->pickup_sound, LINE_SIZE, "sound/misc/w_pkup.wav");
			snprintf(entity.trigger->respawn_sound, LINE_SIZE, "sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "weapon_shotgun");
		}
		else if (strstr(value, "weapon_machinegun"))
		{
			entity.trigger = new Trigger(&entity, audio);
			snprintf(entity.trigger->pickup_sound, LINE_SIZE, "sound/misc/w_pkup.wav");
			snprintf(entity.trigger->respawn_sound, LINE_SIZE, "sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "weapon_machinegun");
		}
		else if (strstr(value, "weapon_railgun"))
		{
			entity.trigger = new Trigger(&entity, audio);
			snprintf(entity.trigger->pickup_sound, LINE_SIZE, "sound/misc/w_pkup.wav");
			snprintf(entity.trigger->respawn_sound, LINE_SIZE, "sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "weapon_railgun");
		}
		else if (strstr(value, "weapon_plasma"))
		{
			entity.trigger = new Trigger(&entity, audio);
			snprintf(entity.trigger->pickup_sound, LINE_SIZE, "sound/misc/w_pkup.wav");
			snprintf(entity.trigger->respawn_sound, LINE_SIZE, "sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "weapon_plasma");
		}
		else if (strstr(value, "weapon_grenadelauncher"))
		{
			entity.trigger = new Trigger(&entity, audio);
			snprintf(entity.trigger->pickup_sound, LINE_SIZE, "sound/misc/w_pkup.wav");
			snprintf(entity.trigger->respawn_sound, LINE_SIZE, "sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "weapon_grenadelauncher");
		}
		else if (strcmp(value, "item_armor_combat") == 0)
		{
			entity.trigger = new Trigger(&entity, audio);
			snprintf(entity.trigger->pickup_sound, LINE_SIZE, "sound/misc/ar2_pkup.wav");
			snprintf(entity.trigger->respawn_sound, LINE_SIZE, "sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "armor 50");
			entity.trigger->armor = true;
		}
		else if (strcmp(value, "item_armor_body") == 0)
		{
			entity.trigger = new Trigger(&entity, audio);
			snprintf(entity.trigger->pickup_sound, LINE_SIZE, "sound/misc/ar2_pkup.wav");
			snprintf(entity.trigger->respawn_sound, LINE_SIZE, "sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "armor 100");
			entity.trigger->armor = true;
		}
		else if (strcmp(value, "func_bobbing") == 0)
		{
			/*
			speed : amount of time in seconds for one complete oscillation cycle (default 4).
			height : sets the amount of travel of the oscillation movement (default 32).
			phase : sets the start offset of the oscillation cycle. Values must be 0 < phase < 1. Any integer phase value is the same as no offset (default 0).
			noise : path/name of .wav file to play. Use looping sounds only (eg. sound/world/drone6.wav - See Notes).
			*/
		}
		else if (strcmp(value, "func_button") == 0)
		{
			/*
			angle : determines the direction in which the button will move (up = -1, down = -2).
			target : all entities with a matching targetname will be triggered.
			speed : speed of button's displacement (default 40).
			wait : number of seconds button stays pressed (default 1, -1 = return immediately).
			lip : lip remaining at end of move (default 4 units).
			health : if set to a non-zero value, the button must be damaged by "health" amount of points to operate.
			*/
		}
		else if (strcmp(value, "func_door") == 0)
		{
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
		}
		else if (strcmp(value, "func_plat") == 0)
		{
			/*
			speed : determines how fast the plat moves (default 150).
			lip : lip remaining at end of move (default 16). Has no effect if "height" is set.
			height : if set, this will determine the total amount of vertical travel of the plat.
			dmg : damage to inflict on player when he blocks operation of plat (default 4). Plat will reverse direction when blocked.
			*/
		}
		else if (strcmp(value, "func_pendulum") == 0)
		{
			/*
			angle: angle offset of axis of rotation from default X axis(default 0).
			speed : angle of swing arc in either direction from initial position(default 30).
			phase : sets the start offset of the swinging cycle.Values must be 0 < phase < 1. Any integer phase value is the same as no offset(default 0).
			noise : path / name of.wav file to play.Use looping sounds only(eg.sound / world / drone6.wav).
			*/
		}
		else if (strcmp(value, "func_rotating") == 0)
		{
			entity.rigid->angular_velocity.y = 1.0f;
			/*
			speed: determines how fast entity rotates(default 100).
			noise : path / name of.wav file to play.Use looping sounds only(eg.sound / world / drone6.wav).
			*/
		}
		else if (strcmp(value, "func_train") == 0)
		{
			/*
			speed : speed of displacement of train (default 100 or overridden by speed value of path).
			target : this points to the first path_corner of the path which is also the spawn location of the train's origin.
			model2 : path/name of model to include (eg: models/mapobjects/pipe/pipe02.md3).
			*/
		}
		else if (strcmp(value, "trigger_teleport") == 0)
		{
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
			if (entity.trigger == NULL)
				entity.trigger = new Trigger(&entity, audio);

			snprintf(entity.trigger->action, LINE_SIZE, "damage 666");
		}
		else if (strcmp(value, "misc_model") == 0)
		{
			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
			}
			entity.nodraw = true;
		}
		else if (strcmp(value, "worldspawn") == 0)
		{
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
			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
				entity.nodraw = true;
			}
		}
		else if (strcmp(value, "trigger_always") == 0)
		{
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
			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
				entity.nodraw = true;
			}
		}
		else if (strcmp(value, "misc_teleporter_dest") == 0)
		{
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
			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
				entity.nodraw = true;
			}
		}
		else if (strcmp(value, "trigger_multiple") == 0)
		{
			if (entity.rigid)
			{
				entity.rigid->gravity = false;
				entity.rigid->noclip = true;
				entity.rigid->flight = true;
				entity.nodraw = true;
			}
		}
		else if (strcmp(value, "navpoint") == 0)
		{
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
			entity.light = new Light(&entity, gfx, light_num++);
		}
		sscanf(value, "%d", &inten);
		intensity = (float)inten;

		while (intensity < 750.0f)
			intensity *= 2.0f;

		while (intensity > 1750.0f)
			intensity /= 2.0f;

		entity.nodraw = true;

		if (entity.light)
			entity.light->intensity = intensity;
	}
	else if (strcmp(key, "_color") == 0)
	{
		float r, g, b;

		if (entity.light == NULL)
		{
			// generates cubemaps for light, this sucks up memory on large maps
			entity.light = new Light(&entity, gfx, light_num++);
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
		snprintf(entity.speaker->file, LINE_SIZE, "%s", value);
	}
	else if (strcmp(key, "noise") == 0)
	{
		entity.speaker = new Speaker(&entity, audio);
		snprintf(entity.speaker->file, LINE_SIZE, "%s", value);
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
			entity.height = (float)atoi(value);
		else
			entity.height = 1.0f;
//		entity.rigid->velocity = vec3(0.0f, atoi(value), 0.0f);
	}

}

bool parse_entity(const char *input, vector<Entity *> &entity_list, Graphics &gfx, Audio &audio)
{
	Entity *entity = NULL;
	char state = 'S';
	char prevstate = 'S';
	char key[LINE_SIZE], val[LINE_SIZE];
	char stack[LINE_SIZE] = {0};
	int sp = 0;
	unsigned int i, j = 0;

	for(i = 0; state != 'F' && state != '\0' && i < strlen(input); i++)
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
				break;
			}
		case 'V': //value
			if (prevstate == 'V')
			{
				val[j++] = input[i];
				val[j] = '\0';
				break;
			}
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
			add_key(*entity, key, val, gfx, audio, entity_list.size() - 1);
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


void handle_stage(char *stagecmd, stage_t *stage)
{
	char *ret = NULL;


	

	if (strstr(stagecmd, "$lightmap"))
	{
		stage->lightmap = true;
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
		/*
		if (strcmp(stage->map_tex, "textures/gothic_block/evil2ckillblockglow.tga") == 0)
		{
			printf("break point\n");
		}

		if (strstr(stage->map_tex, "beam") != 0)
		{
			printf("break point\n");
		}
		*/
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
		else if (strstr(ret, "gl_dst_color gl_one"))
		{
			stage->blend_dst_color_one = true;
		}
		else if (strstr(ret, "gl_dst_color gl_zero"))
		{
			stage->blend_dst_color_zero = true;
		}
		else if (strstr(ret, "gl_dst_color gl_one_minus_dst_alpha"))
		{
			stage->blend_dst_color_one_minus_dst_alpha = true;
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
				surface->name[i - old_pos - 2] = '\0';
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

				if (strstr(stagecmd, "{") != NULL)
				{
					surface->num_stage++;
					num_stage++;
				}
				handle_stage(stagecmd, &(surface->stage[num_stage - 1]));
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
