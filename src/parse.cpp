#include "include.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void add_key(Entity &entity, char *key, char *value, Graphics &gfx)
{
	static int light_num = 0;

	if (strcmp(key, "origin") == 0)
	{
		int x, y, z;

		sscanf(value, "%d %d %d", &x, &y, &z);
		entity.position.x = (float)x;
		entity.position.y = (float)z;
		entity.position.z = (float)-y;
	}
	else if (strcmp(key, "classname") == 0)
	{
		size_t size = strlen(value) + 1;
		entity.type = new char [size];
		memcpy(entity.type, value, size);

		if (strcmp(value, "item_armor_shard") == 0)
		{
			entity.trigger = new Trigger(&entity);
			snprintf(entity.trigger->pickup_snd, LINE_SIZE, "media/sound/items/s_health.wav");
			snprintf(entity.trigger->respawn_snd, LINE_SIZE, "media/sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "armor 5");
		}

		if (strcmp(value, "item_health") == 0)
		{
			entity.trigger = new Trigger(&entity);
			snprintf(entity.trigger->pickup_snd, LINE_SIZE, "media/sound/items/n_health.wav");
			snprintf(entity.trigger->respawn_snd, LINE_SIZE, "media/sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "health 25");
		}

		if (strcmp(value, "item_health_large") == 0)
		{
			entity.trigger = new Trigger(&entity);
			snprintf(entity.trigger->pickup_snd, LINE_SIZE, "media/sound/items/l_health.wav");
			snprintf(entity.trigger->respawn_snd, LINE_SIZE, "media/sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "health 50");
		}


		if (strcmp(value, "ammo_bullets") == 0)
		{
			entity.trigger = new Trigger(&entity);
			snprintf(entity.trigger->pickup_snd, LINE_SIZE, "media/sound/misc/am_pkup.wav");
			snprintf(entity.trigger->respawn_snd, LINE_SIZE, "media/sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "ammo_bullets 25");
		}

		if (strcmp(value, "ammo_rockets") == 0)
		{
			entity.trigger = new Trigger(&entity);
			snprintf(entity.trigger->pickup_snd, LINE_SIZE, "media/sound/misc/am_pkup.wav");
			snprintf(entity.trigger->respawn_snd, LINE_SIZE, "media/sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "ammo_rockets 10");
		}

		if (strcmp(value, "ammo_slugs") == 0)
		{
			entity.trigger = new Trigger(&entity);
			snprintf(entity.trigger->pickup_snd, LINE_SIZE, "media/sound/misc/am_pkup.wav");
			snprintf(entity.trigger->respawn_snd, LINE_SIZE, "media/sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "ammo_slugs 10");
		}

		if (strcmp(value, "ammo_shells") == 0)
		{
			entity.trigger = new Trigger(&entity);
			snprintf(entity.trigger->pickup_snd, LINE_SIZE, "media/sound/misc/am_pkup.wav");
			snprintf(entity.trigger->respawn_snd, LINE_SIZE, "media/sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "ammo_shells 10");
		}

		if (strcmp(value, "ammo_lightning") == 0)
		{
			entity.trigger = new Trigger(&entity);
			snprintf(entity.trigger->pickup_snd, LINE_SIZE, "media/sound/misc/am_pkup.wav");
			snprintf(entity.trigger->respawn_snd, LINE_SIZE, "media/sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "ammo_lightning 10");
		}

		if (strcmp(value, "ammo_plasma") == 0)
		{
			entity.trigger = new Trigger(&entity);
			snprintf(entity.trigger->pickup_snd, LINE_SIZE, "media/sound/misc/am_pkup.wav");
			snprintf(entity.trigger->respawn_snd, LINE_SIZE, "media/sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "ammo_plasma 10");
		}

		if (strstr(value, "weapon_rocketlauncher"))
		{
			entity.trigger = new Trigger(&entity);
			snprintf(entity.trigger->pickup_snd, LINE_SIZE, "media/sound/misc/w_pkup.wav");
			snprintf(entity.trigger->respawn_snd, LINE_SIZE, "media/sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "weapon_rocketlauncher");
		}

		if (strstr(value, "weapon_lightning"))
		{
			entity.trigger = new Trigger(&entity);
			snprintf(entity.trigger->pickup_snd, LINE_SIZE, "media/sound/misc/w_pkup.wav");
			snprintf(entity.trigger->respawn_snd, LINE_SIZE, "media/sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "weapon_lightning");
		}

		if (strstr(value, "weapon_shotgun"))
		{
			entity.trigger = new Trigger(&entity);
			snprintf(entity.trigger->pickup_snd, LINE_SIZE, "media/sound/misc/w_pkup.wav");
			snprintf(entity.trigger->respawn_snd, LINE_SIZE, "media/sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "weapon_shotgun");
		}

		if (strcmp(value, "item_armor_combat") == 0)
		{
			entity.trigger = new Trigger(&entity);
			snprintf(entity.trigger->pickup_snd, LINE_SIZE, "media/sound/misc/ar2_pkup.wav");
			snprintf(entity.trigger->respawn_snd, LINE_SIZE, "media/sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "armor 50");
		}
	}
	else if (strcmp(key, "light") == 0)
	{
			int intensity;

			if (entity.light == NULL)
				entity.light = new Light(&entity, gfx, light_num++);
			sscanf(value, "%d", &intensity);
			entity.light->intensity = intensity;
	}
	else if (strcmp(key, "_color") == 0)
	{
		float r, g, b;

		if (entity.light == NULL)
			entity.light = new Light(&entity, gfx, light_num++);
		sscanf(value, "%f %f %f", &r, &g, &b);
		entity.light->color = vec3(r,g,b);
	}
	else if (strcmp(key, "noise") == 0)
	{
		entity.speaker = new Speaker(&entity);
		snprintf(entity.speaker->file, LINE_SIZE, "media/%s", value);
		entity.rigid->angular_velocity.x = 10.0f;
	}
}

bool parse_entity(const char *input, vector<Entity *> &entity_list, Graphics &gfx)
{
	Entity *entity;
	char state = 'S';
	char prevstate = 'S';
	char key[LINE_SIZE], val[LINE_SIZE];
	char stack[LINE_SIZE] = {0};
	int sp = 0;
	unsigned int i, j;

	for(i = 0; state != 'F' && state != '\0' && i < strlen(input); i++)
	{		
		prevstate = state;
		state = machine(state, input[i], stack, sp);

		switch (state)
		{
		case 'K':
			if (prevstate == 'K')
			{
				key[j++] = input[i];
				key[j] = '\0';
				break;
			}
		case 'V':
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
			add_key(*entity, key, val, gfx);
			break;
		}
	}

	if (state == 'F' && sp == 0)
		return true;
	else
		return false;
}

char machine(char state, char input, char *stack, int &sp)
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
