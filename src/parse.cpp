#include "include.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include <ctype.h> // for isspace (crappy space compression function)

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
			snprintf(entity.trigger->action, LINE_SIZE, "quad");
			entity.trigger->health = false;
		}
		else if (strcmp(value, "holdable_medkit") == 0)
		{
			entity.trigger = new Trigger(&entity, audio);
			snprintf(entity.trigger->pickup_sound, LINE_SIZE, "sound/items/holdable.wav");
			snprintf(entity.trigger->respawn_sound, LINE_SIZE, "sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "medkit");
			entity.trigger->health = false;
		}
		else if (strcmp(value, "holdable_teleporter") == 0)
		{
			entity.trigger = new Trigger(&entity, audio);
			snprintf(entity.trigger->pickup_sound, LINE_SIZE, "sound/items/holdable.wav");
			snprintf(entity.trigger->respawn_sound, LINE_SIZE, "sound/items/s_health.wav");
			snprintf(entity.trigger->action, LINE_SIZE, "teleporter");
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
			snprintf(entity.trigger->action, LINE_SIZE, "flight");
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
		else if (strcmp(value, "worldspawn") == 0)
		{
			//yeap
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
		else if (strcmp(value, "trigger_teleport") == 0)
		{
			if (entity.trigger == NULL)
				entity.trigger = new Trigger(&entity, audio);


			sprintf(entity.type, "trigger_teleport");
			sprintf(entity.trigger->action, "teleport %s %d", entity.target, entity_num);
		}
		else if (strcmp(value, "target_teleporter") == 0)
		{
			//trigger targets this guy, this guy targets destination
			// useless middleman?
		}
		else if (strcmp(value, "trigger_push") == 0)
		{
			if (entity.trigger == NULL)
				entity.trigger = new Trigger(&entity, audio);

			sprintf(entity.trigger->action, "push %s", entity.target);
		}
		else if (strcmp(value, "trigger_hurt") == 0)
		{
			if (entity.trigger == NULL)
				entity.trigger = new Trigger(&entity, audio);

			snprintf(entity.trigger->action, LINE_SIZE, "damage 666");
		}
	}
	else if (strcmp(key, "light") == 0)
	{
		int inten;
		float intensity;

		if (entity.light == NULL && light_num < 100)
		{
			entity.light = new Light(&entity, gfx, light_num++);
		}
		sscanf(value, "%d", &inten);
		intensity = (float)inten;

		while (intensity < 750.0f)
			intensity *= 2.0f;

		while (intensity > 1750.0f)
			intensity /= 2.0f;

		if (entity.light)
			entity.light->intensity = intensity;
	}
	else if (strcmp(key, "_color") == 0)
	{
		float r, g, b;

		if (entity.light == NULL && light_num < 100)
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


void compress_spaces(char *str)
{
	char *dst = str;

	for (; *str; ++str) {
		*dst++ = *str;

		if (isspace(*str)) {
			do ++str;

			while (isspace(*str));

			--str;
		}
	}

	*dst = 0;
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

void parse_shader(char *input, vector<Surface *> &surface_list, char *filename)
{
	char	basecmd[512] = { 0 };
	char	stagecmd[512] = { 0 };
	int		old_pos = 0;
	int		stage_num = 0;
	int		num_cmd = 0;
	int		j = 0;
	int		length = strlen(input);
	char	state = 'S';
	char	prevstate = 'S';
	bool	first = true;

	//        printf("%s\n", input);

	Surface *surface = new Surface;
	surface->num_cmd = 0;
	surface->num_stage = 0;
	memcpy(surface->file, filename, strlen(filename) + 1);

	for (int i = 0; i < length; i++)
	{
		prevstate = state;

		//completely skip line returns
		if (input[i] == '\r')
		{
			input[i] = '\n';
			continue;
		}

		state = machine_shader(input[i], state);
		//            printf("basecmd %s\n", basecmd);

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
				if (first == false)
				{
					surface->stage.stage[0] = NULL;
					surface_list.push_back(surface);
				}
				first = false;

				surface = new Surface;
				memcpy(surface->file, filename, strlen(filename) + 1);
				// bad naming, mixed lines in a stage from number of texture stages, will fix
				surface->stage.stage_num = 0;
				surface->num_cmd = 0;
				surface->num_stage = 0;
				num_cmd = 0;
				stage_num = 0;

				memset(surface->name, 0, sizeof(surface->name));
//				memcpy(name, &input[old_pos + 1], i - old_pos - 1);
				memcpy(surface->name, &input[old_pos + 1], i - old_pos - 1);
				surface->name[i - old_pos - 2] = '\0';
				//printf("name is [%s]\n", name);
				old_pos = i;
				stage_num = 0;
				num_cmd = 0;
				j = 0;
			}
			break;
		case 'F':
			if (input[i] == '\n' && input[i - 1] != '{')
			{
				int size = strlen(basecmd) + 1;
				basecmd[j++] = '\0';
//				printf("basecmd is [%s]\n", basecmd);
				surface->cmd[num_cmd] = new char[size];
				memset(surface->cmd[num_cmd], 0, size);
				memcpy(surface->cmd[num_cmd++], basecmd, size);
				surface->num_cmd++;
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
				int size = strlen(stagecmd) + 1;
//				printf("stagecmd is [%s] stage_num is %d\n", stagecmd, stage_num);
				surface->stage.stage[stage_num] = new char[size];
				memset(surface->stage.stage[stage_num], 0, size);
//				memcpy(surface->stage.stage[stage_num], stagecmd, size);
				memcpy(surface->stage.stage[stage_num], &input[old_pos + 1], i - old_pos - 1);
				surface->stage.stage[stage_num][i - old_pos - 2] = '\0';
				old_pos = i;
				stage_num++;
				surface->num_stage++;
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
