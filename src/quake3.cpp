#include "include.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define BOT_ENABLE

#define MACHINEGUN_DAMAGE 7
#define SHOTGUN_DAMAGE 50
#define GRENADE_DAMAGE 100
#define GRENADE_SPLASH_DAMAGE 50
#define ROCKET_DAMAGE 100
#define ROCKET_SPLASH_DAMAGE 50
#define PLASMA_DAMAGE 20
#define PLASMA_SPLASH_DAMAGE 15
#define LIGHTNING_DAMAGE 
#define RAILGUN_DAMAGE 100



extern char bot_state_name[16][80];

Quake3::Quake3()
{
	blink = false;
}

void Quake3::init(Engine *altEngine)
{
	engine = altEngine;


}

void Quake3::destroy()
{

}


void Quake3::handle_player(int self)
{
	Entity *entity = engine->entity_list[self];
	static int footstep_num = 0;
	static int last_tick = 0;


	if (engine->input.use == true)
	{
		int nav_num = 0;

		if (last_tick == 0)
		{
			printf("NavMesh point:\n");
			printf("{\n");
			printf("\"classname\" \"navpoint\"\n");
			printf("\"origin\" \"%d %d %d\"\n", (int)entity->position.x, (int)entity->position.y, (int)entity->position.z);
			printf("\"targetname\" \"nav%d\"\n", nav_num);
			printf("\"target\" \"nav1 nav2 nav3\"\n");
			printf("}\n");

			navpoint_t navpoint;

			navpoint.position.x = (float)entity->position.x;
			navpoint.position.y = (float)entity->position.y;
			navpoint.position.z = (float)entity->position.z;
			sprintf(navpoint.targetname, "nav%d", nav_num);
			if (navmesh.size() > 0)
				sprintf(navpoint.target, "%s ", navmesh[0].targetname);
			navmesh.push_back(navpoint);
			last_tick = 125;
		}
	}


	if (engine->input.numpad7 == true)
	{
		if (last_tick == 0)
		{
			for (unsigned int i = 0; i < navmesh.size(); i++)
			{
				printf("{\n");
				printf("\"classname\" \"navpoint\"\n");
				printf("\"origin\" \"%d %d %d\"\n", (int)navmesh[i].position.x, (int)navmesh[i].position.y, (int)navmesh[i].position.z);
				printf("\"targetname\" \"%s\"\n", navmesh[i].targetname);
				printf("\"target\" \"%s\"\n", navmesh[i].target);
				printf("}\n");
			}
			last_tick = 250;
		}
	}
	if (last_tick > 0)
		last_tick--;


	if (entity->player->health > 0)
	{
		if (engine->input.control == false)
		{
			// True if jumped
			if (engine->input.moveup || engine->input.movedown || engine->input.moveleft || engine->input.moveright)
			{
				entity->player->state = PLAYER_MOVED;
			}

			if (engine->input.duck)
			{
				entity->player->state = PLAYER_DUCKED;
			}

			if (strcmp(entity->type, "player") == 0)
			{
				float speed_scale = 1.0f;

				if (entity->player->haste_timer > 0)
					speed_scale = 2.0f;

				if (entity->rigid->move(engine->input, speed_scale))
				{
					entity->player->state = PLAYER_JUMPED;
					engine->select_wave(entity->speaker->source, entity->player->jump_sound);
					engine->audio.play(entity->speaker->source);
				}
			}
		}

	}
	else
	{
		input_t noinput;

		memset(&noinput, 0, sizeof(input_t));
		//Makes body hit the floor, need to explore why this hack is needed
		if (entity->player->reload_timer)
		{
			entity->rigid->move(noinput, 1.0f);
		}


		if (entity->player->health <= 0 && entity->player->state != PLAYER_DEAD)
		{
			player_died(self);
		}

		if (strcmp(entity->type, "player") == 0)
		{
			if (engine->input.attack && entity->player->reload_timer == 0)
			{
				engine->console(self, "respawn");
			}
		}
		else
		{
			if (entity->player->reload_timer <= 0)
			{
				engine->console(self, "respawn");
			}
		}
	}

	if (entity->player->teleport_timer > 0)
	{
		entity->player->teleport_timer--;
	}

	if (entity->player->quad_timer > 0)
	{
		if (entity->light == NULL)
		{
			entity->light = new Light(entity, engine->gfx, 999);
			entity->light->color = vec3(0.0f, 0.0f, 1.0f);
			entity->light->intensity = 4000.0f;
			entity->light->attenuation = 0.125f;
		}
		entity->player->quad_timer--;
	}
	else
	{
		// Light list wont be updated until the next step, so manually delete
		if (entity->light)
		{
			for (unsigned int i = 0; i < engine->light_list.size(); i++)
			{
				if (engine->light_list[i]->entity == entity)
				{
					engine->light_list.erase(engine->light_list.begin() + i);
				}
			}
		}
	}

	if (entity->player->regen_timer > 0)
	{
		entity->player->regen_timer--;
	}

	if (entity->player->invisibility_timer > 0)
	{
		entity->player->invisibility_timer--;
		entity->nodraw = true;
	}
	else
	{
		entity->nodraw = false;
	}

	if (entity->player->flight_timer > 0)
	{
		entity->player->flight_timer--;
		entity->rigid->flight = true;
		entity->rigid->translational_friction = 0.9f;
	}
	else
	{
		entity->rigid->flight = false;
		entity->rigid->translational_friction = 0.0f;
	}

	if (engine->tick_num % TICK_RATE == 0)
	{
		if (entity->player->health > 100)
		{
			if (entity->player->regen_timer > 0)
			{
				entity->player->health += 15;
				//play regen bump sound
			}
			else
			{
				entity->player->health--;
			}
		}

		if (entity->player->armor > 100)
		{
			entity->player->armor--;
		}
	}

	if (entity->rigid->velocity.y > -1.0f && entity->rigid->velocity.y < 1.0f &&
		entity->rigid->water == false && entity->player->state != PLAYER_DEAD &&
		entity->rigid->noclip == false && entity->rigid->gravity == true)
	{

		if ((entity->position - entity->rigid->old_position).magnitude() > 0.8f && engine->tick_num % 20 == 0)
		{
			bool ret = false;

			switch (footstep_num++ % 4)
			{
			case 0:
				ret = engine->select_wave(entity->player->footstep_source, entity->player->step1_sound);
				break;
			case 1:
				ret = engine->select_wave(entity->player->footstep_source, entity->player->step2_sound);
				break;
			case 2:
				ret = engine->select_wave(entity->player->footstep_source, entity->player->step3_sound);
				break;
			case 3:
				ret = engine->select_wave(entity->player->footstep_source, entity->player->step4_sound);
				break;
			}

			if (ret)
			{
				engine->audio.play(entity->player->footstep_source);
			}
			else
			{
				debugf("Failed to find PCM data for footstep sound\n");
			}
		}
	}

	if (entity->rigid->water && entity->rigid->water_depth < entity->rigid->get_height())
	{
		if (entity->rigid->water != entity->rigid->last_water)
		{
			bool ret = engine->select_wave(entity->speaker->source, entity->player->waterin_sound);

			if (ret)
			{
				engine->audio.play(entity->speaker->source);
			}
			else
			{
				debugf("Failed to find PCM data for water entry sound\n");
			}
			entity->rigid->last_water = entity->rigid->water;
		}
	}
	else if (entity->rigid->water == false)
	{
		if (entity->rigid->water != entity->rigid->last_water)
		{
			bool ret = engine->select_wave(entity->speaker->source, entity->player->waterout_sound);

			if (ret)
			{
				engine->audio.play(entity->speaker->source);
			}
			else
			{
				debugf("Failed to find PCM data for water exit sound\n");
			}
			entity->rigid->last_water = entity->rigid->water;
			entity->player->drown_timer = 0;
		}
	}
	else
	{
		entity->player->drown_timer++;

		if (entity->player->drown_timer % 125 * 30 == 0)
		{
			bool ret = false;

			switch (footstep_num++ % 2)
			{
			case 0:
				ret = engine->select_wave(entity->speaker->source, entity->player->gurp1_sound);
				break;
			case 1:
				ret = engine->select_wave(entity->speaker->source, entity->player->gurp2_sound);
				break;
			}

			if (ret)
			{
				engine->audio.play(entity->speaker->source);
				entity->player->health -= 15;
			}
			else
			{
				debugf("Failed to find PCM data for water exit sound\n");
			}
		}
	}

	handle_weapons(*(entity->player), engine->input, self);
}

void Quake3::player_died(int index)
{
	Entity *entity = engine->entity_list[index];
	bool ret = false;

	if (entity->player->health <= -50)
	{
		char msg[80];
		sprintf(msg, "%s was gibbed\n", entity->player->name);
		debugf(msg);
		engine->menu.print_notif(msg);

		ret = engine->select_wave(entity->speaker->source, entity->player->gibbed_sound);
		handle_gibs(*(entity->player));
	}
	else
	{
		char msg[80];
		sprintf(msg,"%s died\n", entity->player->name);
		debugf(msg);
		engine->menu.print_notif(msg);

		switch (engine->tick_num % 3)
		{
		case 0:
			ret = engine->select_wave(entity->speaker->source, entity->player->death1_sound);
			break;
		case 1:
			ret = engine->select_wave(entity->speaker->source, entity->player->death2_sound);
			break;
		case 2:
			ret = engine->select_wave(entity->speaker->source, entity->player->death3_sound);
			break;
		}
	}

	if (ret)
	{
		engine->audio.play(entity->speaker->source);
	}
	else
	{
		debugf("Failed to find PCM data for death sound\n");
	}

	drop_weapon(index);

	if (entity->player->quad_timer > 0)
		drop_quaddamage(entity->position);

	entity->player->kill();
	entity->model->clone(*(engine->box->model));
}

void Quake3::drop_weapon(int index)
{
	Entity *entity = engine->entity_list[index];
	Entity *drop_weapon = engine->entity_list[engine->get_entity()];
	char *weapon_str = NULL;


	drop_weapon->rigid = new RigidBody(drop_weapon);
	drop_weapon->position = entity->position;
	drop_weapon->model = drop_weapon->rigid;

	switch (entity->player->current_weapon)
	{
	case wp_machinegun:
		weapon_str = "weapon_machinegun";
		drop_weapon->model->clone(entity->player->weapon_machinegun);
		break;
	case wp_shotgun:
		weapon_str = "weapon_shotgun";
		drop_weapon->model->clone(entity->player->weapon_shotgun);
		break;
	case wp_grenade:
		weapon_str = "weapon_grenadelauncher";
		drop_weapon->model->clone(entity->player->weapon_grenade);
		break;
	case wp_rocket:
		weapon_str = "weapon_rocketlauncher";
		drop_weapon->model->clone(entity->player->weapon_rocket);
		break;
	case wp_plasma:
		weapon_str = "weapon_plasmagun";
		drop_weapon->model->clone(entity->player->weapon_plasma);
		break;
	case wp_lightning:
		weapon_str = "weapon_lightning";
		drop_weapon->model->clone(entity->player->weapon_lightning);
		break;
	case wp_railgun:
		weapon_str = "weapon_railgun";
		drop_weapon->model->clone(entity->player->weapon_railgun);
		break;
	}

	// it will have the players view direction, resetting
	drop_weapon->model->morientation.m[0] = 1.0f;
	drop_weapon->model->morientation.m[1] = 0.0f;
	drop_weapon->model->morientation.m[2] = 0.0f;

	drop_weapon->model->morientation.m[3] = 0.0f;
	drop_weapon->model->morientation.m[4] = 1.0f;
	drop_weapon->model->morientation.m[5] = 0.0f;

	drop_weapon->model->morientation.m[6] = 0.0f;
	drop_weapon->model->morientation.m[7] = 0.0f;
	drop_weapon->model->morientation.m[8] = 1.0f;

	drop_weapon->rigid->velocity = vec3(0.0f, 2.0f, 0.0);
	//			drop_weapon->position += vec3(0.0f, 20.0f, 0.0);


	drop_weapon->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);


	vec3 forward;
	entity->model->getForwardVector(forward);
	drop_weapon->rigid->velocity = forward *-1.0f;
	drop_weapon->rigid->angular_velocity.x = 10.0f;

	//  Prevent player from picking weapon back up
	entity->player->state = PLAYER_DEAD;

	drop_weapon->trigger = new Trigger(drop_weapon, engine->audio);
	snprintf(drop_weapon->trigger->pickup_sound, LINE_SIZE, "sound/misc/w_pkup.wav");
	snprintf(drop_weapon->trigger->respawn_sound, LINE_SIZE, "sound/items/s_health.wav");
	sprintf(drop_weapon->trigger->action, "%s", weapon_str);
}

void Quake3::drop_quaddamage(vec3 &position)
{
	Entity *drop_quad = engine->entity_list[engine->get_entity()];
	drop_quad->position = position;


	drop_quad->rigid = new RigidBody(drop_quad);
	drop_quad->model = drop_quad->rigid;
	drop_quad->model->clone((*engine->box->model));

	drop_quad->rigid->velocity = vec3(0.0f, 2.0f, 0.0);
	drop_quad->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
	drop_quad->trigger = new Trigger(drop_quad, engine->audio);
	snprintf(drop_quad->trigger->pickup_sound, LINE_SIZE, "sound/misc/w_pkup.wav");
	snprintf(drop_quad->trigger->respawn_sound, LINE_SIZE, "sound/items/s_health.wav");
	sprintf(drop_quad->trigger->action, "quaddamage");
}



void Quake3::add_bot(int &index)
{
	index = engine->get_player();

	Entity *entity = engine->entity_list[index];

	debugf("Adding a bot\n");
	entity->rigid = new RigidBody(entity);
	entity->model = entity->rigid;
	entity->rigid->clone(*(engine->thug22->model));
	sprintf(entity->type, "NPC");
	entity->player = new Player(entity, engine->gfx, engine->audio, 16);
	entity->speaker->gain(5.0f);
	sprintf(entity->player->name, "Bot %d", index);
	entity->position += entity->rigid->center + vec3(0.0f, 50.0f, 0.0f);
	entity->rigid->step_flag = true;


	char cmd[80];
	sprintf(cmd, "respawn %d %d", -1, index);
	engine->console(index, cmd);
}


void Quake3::step(int frame_step)
{
	unsigned int num_bot = 3;

	if (engine->entity_list.size() == 0)
		return;

	if (frame_step % TICK_RATE == 0)
	{
		blink = !blink;
	}

	if (engine->server_flag == false && engine->client_flag == false && engine->num_bot < num_bot)
	{
		for (unsigned int i = 0; i < num_bot; i++)
		{
			int bot_index = engine->get_player();
			add_bot(bot_index);
			engine->num_bot++;
		}
	}


	if (engine->menu.ingame == false && engine->menu.console == false)
	{
		if (engine->input.control == true)
		{
			engine->camera_frame.update(engine->input);
		}


		for (unsigned int i = 0; i < engine->num_player; i++)
		{
			Entity *entity = engine->entity_list[i];

			if (strcmp(entity->type, "player") == 0 || strcmp(entity->type, "NPC") == 0)
			{
				handle_player(i);
			}

			if (strcmp(entity->type, "NPC") != 0)
				continue;

#ifdef BOT_ENABLE

			Entity *bot = engine->entity_list[i];
			button_t input;

			memset(&input, 0, sizeof(button_t));
			if (bot->player->health <= 0)
			{
				bot->player->bot_state = BOT_DEAD;
				continue;
			}

			bot->player->avoid_walls(engine->map);

			bot->player->handle_bot(engine->entity_list, i);

			if (bot->player->bot_state == BOT_ALERT || bot->player->bot_state == BOT_ATTACK)
			{
				//clear path
				bot->player->path.step = 0;
				bot->player->path.length = 0;
			}


			if (bot->player->bot_state == BOT_IDLE || bot->player->bot_state == BOT_EXPLORE)
			{
				bot->player->get_item = bot->player->bot_search_for_items(engine->entity_list, i);

				//clear path just in case
				bot->player->path.step = 0;
				bot->player->path.length = 0;
			}

			float speed_scale = 1.0f;

			if (bot->player->haste_timer > 0)
				speed_scale = 2.0f;

			switch (bot->player->bot_state)
			{
			case BOT_ATTACK:
				engine->zcc.select_animation(0);
//				handle_machinegun(*(bot->player), i);
				bot->rigid->move_backward(speed_scale);
				break;
			case BOT_DEAD:
				engine->zcc.select_animation(1);
				bot->model->clone(*(engine->box->model));
				engine->select_wave(bot->speaker->source, bot->player->death1_sound);
				engine->audio.play(bot->speaker->source);

				bot->player->respawn();
				char cmd[80];
				sprintf(cmd, "respawn -1 %d", i);
				engine->console(i, cmd);
				break;
			case BOT_GET_ITEM:
			{
				static int nav_array[64] = { 0 };
				int ret = 0;

				bot->player->ammo_bullets = 100; // bots cheat on reloading :)
				engine->zcc.select_animation(1);
				if (engine->entity_list[bot->player->get_item]->trigger->active)
				{
					//some one got the item before we did, abort
					bot->player->bot_state = BOT_IDLE;
					bot->player->path.step = 0;
					bot->player->path.length = 0;

				}

				// Need a path to item
				if (engine->entity_list[i]->player->path.length == 0)
				{
					//probably need to pass Player reference instead of each path param, or make a struct
					ret = bot_get_path(engine->entity_list[i]->player->get_item, i, nav_array,
						engine->entity_list[i]->player->path);

					if (engine->entity_list[i]->player->path.length == -1)
					{
						// Path doesnt exist, give up
						strncat(bot->player->ignore,  engine->entity_list[bot->player->get_item]->type, 1023);
						strncat(bot->player->ignore, " ", 1023);

						if (strlen(bot->player->ignore) >= 1000)
							bot->player->ignore[0] = '\0';

						bot->player->bot_state = BOT_IDLE;
						bot->player->path.step = 0;
						bot->player->path.length = 0;
					}
					else
					{
						bot->player->ignore[0] = '\0';
					}
				}

				// We are already at the closest nav point to item, fake a empty path
				if (ret == -1)
				{
					bot->player->path.step = 1;
					bot->player->path.length = 1;
				}

				// At last step in path list, go directly to item
				if (bot->player->path.step == bot->player->path.length)
				{
					vec3 delta;
					bot->rigid->lookat_yaw(engine->entity_list[bot->player->get_item]->position);
					bot->rigid->move_forward(speed_scale);

					float distance = (engine->entity_list[i]->position - engine->entity_list[bot->player->get_item]->position).magnitude();

					if (distance < 10.0f)
					{
						// Finally got where the item is, exit get state
						bot->player->bot_state = BOT_IDLE;
						bot->player->path.step = 0;
						bot->player->path.length = 0;
					}
				}
				else if (bot->player->path.length != 0)
				{
					// Go through path steps until we get to navpoint where item is
					if (bot_follow(bot->player->path,
						nav_array, bot, speed_scale) == 0)
					{
						(bot->player->path.step)++;
						// momentum makes him miss pretty bad
						bot->rigid->velocity = vec3(0.0f, 0.0f, 0.0f);
					}
				}
				break;
			}
			case BOT_ALERT:
				engine->zcc.select_animation(0);
				bot->rigid->move_forward(speed_scale);
				break;
			case BOT_EXPLORE:
				engine->zcc.select_animation(1);
				bot->player->ignore[0] = '\0';
				break;
			case BOT_IDLE:
				engine->zcc.select_animation(1);
				break;
			}
#endif

		}
	}
}

void Quake3::handle_plasma(Player &player, int self)
{
	Frame camera_frame;

	player.entity->model->get_frame(camera_frame);

	//forward is right for the bots, need to fix it as hacks are piling up
	if (strcmp(engine->entity_list[self]->type, "NPC") == 0)
	{
		camera_frame.forward.x = -player.entity->model->morientation.m[0];
		camera_frame.forward.y = -player.entity->model->morientation.m[1];
		camera_frame.forward.z = -player.entity->model->morientation.m[2];
	}

	sprintf(player.attack_sound, "sound/weapons/plasma/hyprbf1a.wav");

	player.reload_timer = 8;
	player.ammo_plasma--;

	float quad_factor = 1.0f;

	if (player.quad_timer > 0)
		quad_factor = 3.0f;


	Entity *projectile = engine->entity_list[engine->get_entity()];
	projectile->rigid = new RigidBody(projectile);
	projectile->model = projectile->rigid;
	projectile->position = camera_frame.pos;
	camera_frame.set(projectile->model->morientation);

	projectile->rigid->clone(*(engine->ball->model));
	projectile->rigid->velocity = camera_frame.forward * -10.0f;
	projectile->rigid->net_force = camera_frame.forward * -10.0f;

	projectile->rigid->angular_velocity = vec3();
	projectile->rigid->gravity = false;
	projectile->trigger = new Trigger(projectile, engine->audio);
	sprintf(projectile->trigger->explode_sound, "sound/weapons/plasma/plasmx1a.wav");
	sprintf(projectile->trigger->idle_sound, "sound/weapons/plasma/lasfly.wav");
	sprintf(projectile->trigger->action, "damage %d", (int)(PLASMA_DAMAGE * quad_factor));

	projectile->trigger->hide = false;
	projectile->trigger->self = false;
	projectile->trigger->idle = true;
	projectile->trigger->explode = false;
	projectile->trigger->explode_timer = 10;
	projectile->trigger->explode_color = vec3(0.0f, 0.0f, 1.0f);
	projectile->trigger->explode_intensity = 200.0f;
	projectile->trigger->splash_damage = (int)(PLASMA_SPLASH_DAMAGE * quad_factor);
	projectile->trigger->splash_radius = 75.0f;
	projectile->trigger->knockback = 75.0f;
	projectile->trigger->owner = self;


	projectile->light = new Light(projectile, engine->gfx, 999);
	projectile->light->color = vec3(0.0f, 0.0f, 1.0f);
	projectile->light->intensity = 1000.0f;


	Entity *muzzleflash = engine->entity_list[engine->get_entity()];
	muzzleflash->position = player.entity->position + camera_frame.forward * -75.0f;
	muzzleflash->light = new Light(muzzleflash, engine->gfx, 999);
	muzzleflash->light->color = vec3(0.6f, 0.6f, 1.0f);
	muzzleflash->light->intensity = 2000.0f;
	muzzleflash->light->attenuation = 0.0625f;
	muzzleflash->light->timer_flag = true;
	muzzleflash->light->timer = (int)(0.125f * TICK_RATE);

}

void Quake3::handle_rocketlauncher(Player &player, int self)
{
	Frame camera_frame;

	player.entity->model->get_frame(camera_frame);

	//forward is right for the bots, need to fix it as hacks are piling up
	if (strcmp(engine->entity_list[self]->type, "NPC") == 0)
	{
		camera_frame.forward.x = -player.entity->model->morientation.m[0];
		camera_frame.forward.y = -player.entity->model->morientation.m[1];
		camera_frame.forward.z = -player.entity->model->morientation.m[2];
	}


	sprintf(player.attack_sound, "sound/weapons/rocket/rocklf1a.wav");
	player.reload_timer = 100;
	player.ammo_rockets--;

	float quad_factor = 1.0f;

	if (player.quad_timer > 0)
		quad_factor = 3.0f;



	Entity *projectile = engine->entity_list[engine->get_entity()];
	projectile->position = camera_frame.pos;

	projectile->trigger = new Trigger(projectile, engine->audio);
	sprintf(projectile->trigger->explode_sound, "sound/weapons/rocket/rocklx1a.wav");
	sprintf(projectile->trigger->idle_sound, "sound/weapons/rocket/rockfly.wav");
	sprintf(projectile->trigger->action, "damage %d", (int)(ROCKET_DAMAGE * quad_factor));

	projectile->trigger->hide = false;
	projectile->trigger->self = false;
	projectile->trigger->idle = true;
	projectile->trigger->explode = true;
	projectile->trigger->idle_timer = 0;
	projectile->trigger->explode_timer = 10;
	projectile->trigger->explode_color = vec3(1.0f, 0.0f, 0.0f);
	projectile->trigger->explode_intensity = 500.0f;
	projectile->trigger->splash_damage = (int)(ROCKET_SPLASH_DAMAGE * quad_factor);
	projectile->trigger->splash_radius = 250.0f;
	projectile->trigger->knockback = 250.0f;
	projectile->trigger->owner = self;

	projectile->light = new Light(projectile, engine->gfx, 999);
	projectile->light->color = vec3(1.0f, 1.0f, 1.0f);
	projectile->light->intensity = 1000.0f;

	projectile->rigid = new RigidBody(projectile);
	projectile->model = projectile->rigid;
	camera_frame.set(projectile->rigid->morientation);
	projectile->rigid->clone(*(engine->rocket->model));
	projectile->rigid->velocity = camera_frame.forward * -6.25f;
	projectile->rigid->net_force = camera_frame.forward * -10.0f;
	projectile->rigid->angular_velocity = vec3();
	projectile->rigid->gravity = false;




	bool ret = engine->select_wave(projectile->trigger->loop_source, projectile->trigger->idle_sound);
	if (ret)
	{
		engine->audio.play(projectile->trigger->loop_source);
	}
	else
	{
		debugf("Unable to find PCM data for %s\n", projectile->trigger->idle_sound);
	}

	Entity *muzzleflash = engine->entity_list[engine->get_entity()];
	muzzleflash->position = player.entity->position + camera_frame.forward * -75.0f;
	muzzleflash->light = new Light(muzzleflash, engine->gfx, 999);
	muzzleflash->light->color = vec3(1.0f, 0.75f, 0.0f);
	muzzleflash->light->intensity = 2000.0f;
	muzzleflash->light->attenuation = 0.0625f;
	muzzleflash->light->timer_flag = true;
	muzzleflash->light->timer = (int)(0.125f * TICK_RATE);
}

void Quake3::handle_grenade(Player &player, int self)
{
	Frame camera_frame;

	player.entity->model->get_frame(camera_frame);

	//forward is right for the bots, need to fix it as hacks are piling up
	if (strcmp(engine->entity_list[self]->type, "NPC") == 0)
	{
		camera_frame.forward.x = -player.entity->model->morientation.m[0];
		camera_frame.forward.y = -player.entity->model->morientation.m[1];
		camera_frame.forward.z = -player.entity->model->morientation.m[2];
	}


	sprintf(player.attack_sound, "sound/weapons/grenade/grenlf1a.wav");

	player.reload_timer = 100;
	player.ammo_grenades--;

	Entity *projectile = engine->entity_list[engine->get_entity()];

	projectile->rigid = new RigidBody(projectile);
	projectile->model = projectile->rigid;
	projectile->position = camera_frame.pos;
	camera_frame.set(projectile->model->morientation);

	projectile->rigid->clone(*(engine->box->model));
	//entity->rigid->clone(*(pineapple->model));
	projectile->rigid->velocity = camera_frame.forward * -5.0f;
	projectile->rigid->angular_velocity = vec3(0.1f, 0.1f, 0.1f);
	projectile->rigid->gravity = true;
	projectile->rigid->rotational_friction_flag = true;
	//entity->rigid->set_target(*(entity_list[spawn]));

	float quad_factor = 1.0f;

	if (player.quad_timer > 0)
		quad_factor = 3.0f;


	projectile->trigger = new Trigger(projectile, engine->audio);
	sprintf(projectile->trigger->explode_sound, "sound/weapons/rocket/rocklx1a.wav");
	sprintf(projectile->trigger->action, "damage %d", (int)(GRENADE_DAMAGE * quad_factor));

	projectile->trigger->hide = false;
	projectile->trigger->self = false;
	projectile->trigger->idle = true;
	projectile->trigger->idle_timer = 120;
	projectile->trigger->explode = true;
	projectile->trigger->explode_timer = 10;
	projectile->trigger->explode_color = vec3(1.0f, 0.0f, 0.0f);
	projectile->trigger->explode_intensity = 500.0f;
	projectile->trigger->splash_damage = (int)(GRENADE_SPLASH_DAMAGE * quad_factor);
	projectile->trigger->splash_radius = 250.0f;
	projectile->trigger->knockback = 250.0f;
	projectile->trigger->owner = self;


	Entity *muzzleflash = engine->entity_list[engine->get_entity()];
	muzzleflash->position = player.entity->position + camera_frame.forward * -75.0f;
	muzzleflash->light = new Light(muzzleflash, engine->gfx, 999);
	muzzleflash->light->color = vec3(1.0f, 0.7f, 0.0f);
	muzzleflash->light->intensity = 2000.0f;
	muzzleflash->light->attenuation = 0.0625f;
	muzzleflash->light->timer_flag = true;
	muzzleflash->light->timer = (int)(0.125f * TICK_RATE);
}

void Quake3::handle_lightning(Player &player, int self)
{
	Frame camera_frame;

	player.entity->model->get_frame(camera_frame);

	//forward is right for the bots, need to fix it as hacks are piling up
	if (strcmp(engine->entity_list[self]->type, "NPC") == 0)
	{
		camera_frame.forward.x = -player.entity->model->morientation.m[0];
		camera_frame.forward.y = -player.entity->model->morientation.m[1];
		camera_frame.forward.z = -player.entity->model->morientation.m[2];
	}


	sprintf(player.attack_sound, "sound/weapons/lightning/lg_fire.wav");
	player.reload_timer = 6;
	player.ammo_lightning--;


	Entity *projectile = engine->entity_list[engine->get_entity()];
	projectile->rigid = new RigidBody(projectile);
	projectile->position = camera_frame.pos;
	projectile->rigid->clone(*(engine->box->model));
	projectile->rigid->velocity = camera_frame.forward * -1.0f;
	projectile->rigid->angular_velocity = vec3();
	projectile->rigid->gravity = false;
	projectile->rigid->rotational_friction_flag = true;
	projectile->model = projectile->rigid;
	projectile->rigid->set_target(*(engine->entity_list[self]));
	camera_frame.set(projectile->model->morientation);

	projectile->light = new Light(projectile, engine->gfx, 999);
	projectile->light->color = vec3(1.0f, 1.0f, 1.0f);
	projectile->light->intensity = 1000.0f;
//	entity->trigger->owner = self;


	Entity *muzzleflash = engine->entity_list[engine->get_entity()];
	muzzleflash->position = player.entity->position + camera_frame.forward * -75.0f;
	muzzleflash->light = new Light(muzzleflash, engine->gfx, 999);
	muzzleflash->light->color = vec3(0.6f, 0.6f, 1.0f);
	muzzleflash->light->intensity = 2000.0f;
	muzzleflash->light->attenuation = 0.0625f;
	muzzleflash->light->timer_flag = true;
	muzzleflash->light->timer = (int)(0.125f * TICK_RATE);

}

void Quake3::handle_railgun(Player &player, int self)
{
	int index[8];
	int num_index;

	Frame camera_frame;

	player.entity->model->get_frame(camera_frame);


	sprintf(player.attack_sound, "sound/weapons/railgun/railgf1a.wav");

	player.reload_timer = 188;
	player.ammo_slugs--;

	Entity *projectile = engine->entity_list[engine->get_entity()];
	projectile->rigid = new RigidBody(projectile);
	projectile->position = camera_frame.pos;
	projectile->rigid->clone(*(engine->ball->model));
	projectile->rigid->velocity = camera_frame.forward * -100.0f;
	projectile->rigid->angular_velocity = vec3();
	projectile->rigid->gravity = false;
	projectile->model = projectile->rigid;
	camera_frame.set(projectile->model->morientation);

	vec3 forward;
	player.entity->model->getForwardVector(forward);

	float quad_factor = 1.0f;

	if (player.quad_timer > 0)
		quad_factor = 3.0f;


	engine->hitscan(player.entity->position, forward, index, num_index, self);
	for (int i = 0; i < num_index; i++)
	{
		char cmd[80] = { 0 };

		if (engine->entity_list[index[i]]->player == NULL)
			continue;

		debugf("Player %s hit %s with the railgun for %d damage\n", player.name,
			engine->entity_list[index[i]]->player->name, (int)(RAILGUN_DAMAGE * quad_factor));

		debugf("%s has %d health\n", engine->entity_list[index[i]]->player->name,
			engine->entity_list[index[i]]->player->health);
		sprintf(cmd, "hurt %d %d", index[i], (int)(RAILGUN_DAMAGE * quad_factor));
		engine->console(self, cmd);
	}


	Entity *muzzleflash = engine->entity_list[engine->get_entity()];
	muzzleflash->position = player.entity->position + camera_frame.forward * -75.0f;
	muzzleflash->light = new Light(muzzleflash, engine->gfx, 999);
	muzzleflash->light->color = vec3(1.0f, 0.5f, 0.0f);
	muzzleflash->light->intensity = 2000.0f;
	muzzleflash->light->attenuation = 0.0625f;
	muzzleflash->light->timer_flag = true;
	muzzleflash->light->timer = (int)(0.125f * TICK_RATE);
}

void Quake3::handle_machinegun(Player &player, int self)
{
	char cmd[80] = { 0 };
	int index[8];
	int num_index;
	//float distance;

	Frame camera_frame;

	player.entity->model->get_frame(camera_frame);


	sprintf(player.attack_sound, "sound/weapons/machinegun/machgf1b.wav");



	player.reload_timer = 8;
	player.ammo_bullets--;

	//forward is right for the bots, need to fix it as hacks are piling up
	if (strcmp(engine->entity_list[self]->type, "NPC") == 0)
	{
		camera_frame.forward.x = player.entity->model->morientation.m[0];
		camera_frame.forward.y = player.entity->model->morientation.m[1];
		camera_frame.forward.z = player.entity->model->morientation.m[2];
	}
	else
	{
		camera_frame.forward *= -1;
	}


	Entity *muzzleflash = engine->entity_list[engine->get_entity()];
	muzzleflash->position = player.entity->position + camera_frame.forward * 75.0f;
	muzzleflash->light = new Light(muzzleflash, engine->gfx, 999);
	muzzleflash->light->color = vec3(1.0f, 1.0f, 0.0f);
	muzzleflash->light->intensity = 2000.0f;
	muzzleflash->light->attenuation = 0.0625f;
	muzzleflash->light->timer_flag = true;
	muzzleflash->light->timer = (int)(0.125f * TICK_RATE);

	engine->hitscan(player.entity->position, camera_frame.forward, index, num_index, self);

	float quad_factor = 1.0f;

	if (player.quad_timer > 0)
		quad_factor = 3.0f;

	for (int i = 0; i < num_index; i++)
	{

		if (engine->entity_list[index[i]]->player == NULL)
			continue;

		debugf("Player %s hit %s with the machinegun for %d damage\n", player.name,
			engine->entity_list[index[i]]->player->name, (int)(MACHINEGUN_DAMAGE * quad_factor));
		sprintf(cmd, "hurt %d %d", index[i], (int)(MACHINEGUN_DAMAGE * quad_factor));
		debugf("%s has %d health\n", engine->entity_list[index[i]]->player->name,
			engine->entity_list[index[i]]->player->health);
		engine->console(self, cmd);
	}

}

void Quake3::handle_shotgun(Player &player, int self)
{
	Frame camera_frame;

	player.entity->model->get_frame(camera_frame);

	int index[8];
	int num_index;


	player.reload_timer = 60;
	player.ammo_shells--;

	sprintf(player.attack_sound, "sound/weapons/shotgun/sshotf1b.wav");

	//	engine->map.hitscan(player.entity->position, forward, distance);

	//forward is right for the bots, need to fix it as hacks are piling up
	if (strcmp(engine->entity_list[self]->type, "NPC") == 0)
	{
		camera_frame.forward.x = -player.entity->model->morientation.m[0];
		camera_frame.forward.y = -player.entity->model->morientation.m[1];
		camera_frame.forward.z = -player.entity->model->morientation.m[2];
	}

	Entity *muzzleflash = engine->entity_list[engine->get_entity()];
	muzzleflash->position = player.entity->position + camera_frame.forward * 75.0f;
	muzzleflash->light = new Light(muzzleflash, engine->gfx, 999);
	muzzleflash->light->color = vec3(1.0f, 1.0f, 0.75f);
	muzzleflash->light->intensity = 3000.0f;
	muzzleflash->light->attenuation = 0.125f;
	muzzleflash->light->timer_flag = true;
	muzzleflash->light->timer = (int)(0.125f * TICK_RATE);

	float quad_factor = 1.0f;

	if (player.quad_timer > 0)
		quad_factor = 3.0f;


	engine->hitscan(player.entity->position, -camera_frame.forward, index, num_index, self);
	for (int i = 0; i < num_index; i++)
	{
		char cmd[80] = { 0 };

		if (engine->entity_list[index[i]]->player == NULL)
			continue;

		debugf("Player %s hit %s with the shotgun for %d damage\n", player.name,
			engine->entity_list[index[i]]->player->name, (int)(SHOTGUN_DAMAGE * quad_factor));
		sprintf(cmd, "hurt %d %d", index[i], (int)(SHOTGUN_DAMAGE * quad_factor));
		debugf("%s has %d health\n", engine->entity_list[index[i]]->player->name,
			engine->entity_list[index[i]]->player->health);

		engine->console(self, cmd);
	}

}

void Quake3::handle_gibs(Player &player)
{

	Frame camera_frame;

	player.entity->model->get_frame(camera_frame);


	player.entity->rigid->velocity += vec3(0.5f, 3.0f, 1.2f);

	{
		Entity *entity1 = engine->entity_list[engine->get_entity()];
		entity1->rigid = new RigidBody(entity1);
		entity1->model = entity1->rigid;
		entity1->position = camera_frame.pos;
		camera_frame.set(entity1->model->morientation);

		entity1->rigid->clone(*(engine->box->model));
		//entity->rigid->clone(*(pineapple->model));
		entity1->rigid->velocity = vec3(2.0f, 1.2f, -1.2f);
		entity1->rigid->angular_velocity = vec3(5.0f, 3.0f, 2.f);
		entity1->rigid->gravity = true;
		entity1->rigid->rotational_friction_flag = true;

		entity1->trigger = new Trigger(entity1, engine->audio);
		sprintf(entity1->trigger->explode_sound, "sound/player/gibimp1.wav");
		sprintf(entity1->trigger->action, "gib1 impact");
		entity1->trigger->idle = true;
		entity1->trigger->idle_timer = 0;
		entity1->trigger->explode = false;
	}

	{
		Entity *entity2 = engine->entity_list[engine->get_entity()];

		entity2->rigid = new RigidBody(entity2);
		entity2->model = entity2->rigid;
		entity2->position = camera_frame.pos;
		camera_frame.set(entity2->model->morientation);

		entity2->rigid->clone(*(engine->box->model));
		//entity->rigid->clone(*(pineapple->model));
		entity2->rigid->velocity = vec3(0.5f, 2.0f, 0.2f);
		entity2->rigid->angular_velocity = vec3(-5.0f, -1.0f, 6.0f);
		entity2->rigid->gravity = true;
		entity2->rigid->rotational_friction_flag = true;

		entity2->trigger = new Trigger(entity2, engine->audio);
		sprintf(entity2->trigger->explode_sound, "sound/player/gibimp2.wav");
		sprintf(entity2->trigger->action, "gib2 impact");
		entity2->trigger->idle = true;
		entity2->trigger->idle_timer = 0;
		entity2->trigger->explode = false;
	}

	{
		Entity *entity3 = engine->entity_list[engine->get_entity()];

		entity3->rigid = new RigidBody(entity3);
		entity3->model = entity3->rigid;
		entity3->position = camera_frame.pos;
		camera_frame.set(entity3->model->morientation);

		entity3->rigid->clone(*(engine->box->model));
		//entity->rigid->clone(*(pineapple->model));
		entity3->rigid->velocity = vec3(-2.0f, 3.2f, 1.2f);
		entity3->rigid->angular_velocity = vec3(3.0f, -4.0f, 2.0f);
		entity3->rigid->gravity = true;
		entity3->rigid->rotational_friction_flag = true;

		entity3->trigger = new Trigger(entity3, engine->audio);
		sprintf(entity3->trigger->explode_sound, "sound/player/gibimp3.wav");
		sprintf(entity3->trigger->action, "gib3 impact");
		entity3->trigger->idle = true;
		entity3->trigger->idle_timer = 0;
		entity3->trigger->explode = false;
	}


}


void Quake3::handle_weapons(Player &player, input_t &input, int self)
{
	bool fired = false;
	bool empty = false;

	if (player.reload_timer > 0)
	{
		player.reload_timer--;
	}

	if (player.health <= 0)
	{
		player.state = PLAYER_DEAD;
		return;
	}

	if (player.current_weapon != player.last_weapon)
	{
		switch (player.current_weapon)
		{
		case wp_railgun:
			sprintf(player.weapon_idle_sound, "sound/weapons/railgun/rg_hum.wav");
			break;
		case wp_lightning:
//			sprintf(player.weapon_idle_sound, "sound/weapons/lightning/lg_hum.wav");
			break;
		default:
			player.weapon_idle_sound[0] = '\0';
			break;
		}
		engine->audio.stop(player.entity->speaker->loop_source);
		player.entity->speaker->loop_gain(0.25f);
		if (player.weapon_idle_sound[0] != '\0')
		{
			bool ret = engine->select_wave(player.entity->speaker->loop_source, player.weapon_idle_sound);
			if (ret)
			{
				engine->audio.play(player.entity->speaker->loop_source);
			}
			else
			{
				debugf("Unable to find PCM data for %s\n", player.weapon_idle_sound);
			}
		}

		if (player.spawned)
		{
			bool ret = engine->select_wave(player.entity->speaker->source, player.weapon_swap_sound);
			if (ret)
			{
				engine->audio.play(player.entity->speaker->source);
			}
			else
			{
				debugf("Unable to find PCM data for %s\n", player.weapon_idle_sound);
			}
		}
		player.spawned = true;
		player.last_weapon = player.current_weapon;
	}

	/*
	Weapon notes:
	MG 10  rps 5/7 dmg						[6 ticks]
	SG 1   rps 10 pellets, 10 dmg each		[60 ticks]
	GL 0.8 spr 100dmg larger SD than RL		[48 ticks]
	RL 0.8 spr 100dmg						[48 ticks]
	LG  20 rps 8 dmg (160dmg per second)	[3 ticks]
	RG 1.5 rps 100dmg						[90 ticks]
	PG  10 rps 20dmg 15 splash				[6 ticks]

	// reload delay in ticks/steps (1/60th of a second)
	ticks = 60 * spr
	ticks = 60 * (1 / rps) -- railgun didnt seem right
	*/

	if (player.bot_state == BOT_ATTACK)
	{
		player.best_weapon();
	}

	if ((input.attack && (strcmp(player.entity->type, "player") == 0) && player.reload_timer <= 0) ||
		(player.bot_state == BOT_ATTACK) && player.reload_timer <= 0)
	{
		if (player.current_weapon == wp_rocket)
		{
			if (player.ammo_rockets > 0)
			{
				fired = true;
				Entity *entity = engine->entity_list[engine->get_entity()];
				handle_rocketlauncher(player, self);
			}
			else
			{
				empty = true;
			}
		}
		else if (player.current_weapon == wp_plasma)
		{
			if (player.ammo_plasma > 0)
			{
				fired = true;
				Entity *entity = engine->entity_list[engine->get_entity()];
				handle_plasma(player, self);
			}
			else
			{
				empty = true;
			}
		}
		else if (player.current_weapon == wp_grenade)
		{
			if (player.ammo_grenades > 0)
			{
				fired = true;
				Entity *entity = engine->entity_list[engine->get_entity()];
				handle_grenade(player, self);
			}
			else
			{
				empty = true;
			}

		}
		else if (player.current_weapon == wp_lightning)
		{
			if (player.ammo_lightning > 0)
			{
				fired = true;
				Entity *entity = engine->entity_list[engine->get_entity()];
				handle_lightning(player, self);
			}
			else
			{
				empty = true;
			}

		}
		else if (player.current_weapon == wp_railgun)
		{
			if (player.ammo_slugs > 0)
			{
				fired = true;
				Entity *entity = engine->entity_list[engine->get_entity()];
				handle_railgun(player, self);
			}
			else
			{
				empty = true;
			}

		}
		else if (player.current_weapon == wp_shotgun)
		{
			if (player.ammo_shells > 0)
			{
				fired = true;
				handle_shotgun(player, self);
			}
			else
			{
				empty = true;
			}

		}
		else if (player.current_weapon == wp_machinegun)
		{
			if (player.ammo_bullets > 0)
			{
				fired = true;
				handle_machinegun(player, self);
			}
			else
			{
				empty = true;
			}
		}

		if (fired)
		{
			bool ret = false;

			player.state = PLAYER_ATTACK;
			ret = engine->select_wave(player.entity->speaker->source, player.attack_sound);

			if (ret)
			{
				engine->audio.play(player.entity->speaker->source);
			}
			else
			{
				debugf("Failed to find PCM data for %s\n", player.attack_sound);
			}
		}
		else if (empty)
		{
			bool ret = false;

			player.reload_timer = 30;
			ret = engine->select_wave(player.entity->speaker->source, player.empty_sound);

			if (ret)
			{
				engine->audio.play(player.entity->speaker->source);
			}
			else
			{
				debugf("Failed to find PCM data for %s\n", player.empty_sound);
			}
		}
	}


}

void Quake3::render_hud(double last_frametime)
{
	matrix4 real_projection = engine->projection;
	char msg[LINE_SIZE];

	int spawn = engine->find_player();
	Entity *entity = engine->entity_list[spawn];

	engine->projection = engine->identity;
	vec3 color(1.0f, 1.0f, 1.0f);

	engine->menu.render_chat(engine->global);
	engine->menu.render_notif(engine->global);

	if (engine->show_hud)
	{
		if (spawn != -1)
		{
			if (entity->player->health > 50)
			{
				snprintf(msg, LINE_SIZE, "%d/%d", entity->player->health, entity->player->armor);
				engine->menu.draw_text(msg, 0.15f, 0.95f, 0.050f, color);
			}
			else if (entity->player->health <= 50 && blink)
			{
				vec3 red = vec3(1.0f, 0.0f, 0.0f);
				snprintf(msg, LINE_SIZE, "%d/%d", entity->player->health, entity->player->armor);
				engine->menu.draw_text(msg, 0.15f, 0.95f, 0.050f, red);
			}
			else
			{
				snprintf(msg, LINE_SIZE, "%d/%d", entity->player->health, entity->player->armor);
				engine->menu.draw_text(msg, 0.15f, 0.95f, 0.050f, color);
			}

			switch (entity->player->current_weapon)
			{
			case wp_machinegun:
				snprintf(msg, LINE_SIZE, "%d", entity->player->ammo_bullets);
				engine->menu.draw_text(msg, 0.7f, 0.95f, 0.050f, color);
				break;
			case wp_shotgun:
				snprintf(msg, LINE_SIZE, "%d", entity->player->ammo_shells);
				engine->menu.draw_text(msg, 0.7f, 0.95f, 0.050f, color);
				break;
			case wp_grenade:
				snprintf(msg, LINE_SIZE, "%d", entity->player->ammo_grenades);
				engine->menu.draw_text(msg, 0.7f, 0.95f, 0.050f, color);
				break;
			case wp_rocket:
				snprintf(msg, LINE_SIZE, "%d", entity->player->ammo_rockets);
				engine->menu.draw_text(msg, 0.7f, 0.95f, 0.050f, color);
				break;
			case wp_railgun:
				snprintf(msg, LINE_SIZE, "%d", entity->player->ammo_slugs);
				engine->menu.draw_text(msg, 0.7f, 0.95f, 0.050f, color);
				break;
			case wp_lightning:
				snprintf(msg, LINE_SIZE, "%d", entity->player->ammo_lightning);
				engine->menu.draw_text(msg, 0.7f, 0.95f, 0.050f, color);
				break;
			case wp_plasma:
				snprintf(msg, LINE_SIZE, "%d", entity->player->ammo_plasma);
				engine->menu.draw_text(msg, 0.7f, 0.95f, 0.050f, color);
				break;
			}
		}
	}

	if (engine->show_debug)
	{
		int line = 1;

		snprintf(msg, LINE_SIZE, "Debug Messages: lastframe %.2f ms %.2f fps", last_frametime, 1000.0 / last_frametime);
		engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color);
		snprintf(msg, LINE_SIZE, "%d active lights.", (int)engine->light_list.size());
		engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color);
		if (spawn != -1)
		{
			line++;
			snprintf(msg, LINE_SIZE, "position: %3.3f %3.3f %3.3f", entity->position.x, entity->position.y, entity->position.z);
			engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color);
			snprintf(msg, LINE_SIZE, "velocity: %3.3f %3.3f %3.3f", entity->rigid->velocity.x, entity->rigid->velocity.y, entity->rigid->velocity.z);
			engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color);
			snprintf(msg, LINE_SIZE, "Water: %d depth %lf", entity->rigid->water, entity->rigid->water_depth);
			engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color);
		}
	}

	if (engine->show_names)
	{
		for (unsigned int i = 0; i < engine->entity_list.size(); i++)
		{
			if (engine->entity_list[i]->rigid == NULL)
				continue;

			if (engine->entity_list[i]->visible && engine->entity_list[i]->nodraw == false)
			{
				draw_name(engine->entity_list[i], engine->menu, real_projection);
			}
		}
	}

	for (unsigned int i = 0; i < engine->num_player; i++)
	{
		if (strcmp(engine->entity_list[i]->type, "NPC") != 0)
			continue;

		if (engine->entity_list[i]->visible && engine->entity_list[i]->nodraw == false)
		{
			draw_name(engine->entity_list[i], engine->menu, real_projection);
		}
	}


	if (engine->show_lines)
	{
		vec3 color(1.0f, 1.0f, 1.0f);

		for (unsigned int i = 0; i < engine->entity_list.size(); i++)
		{
			if (engine->entity_list[i]->nodraw == true)
				continue;

			if ( strlen(engine->entity_list[i]->target_name) <= 1 )
				continue;

			for (unsigned int j = 0; j < engine->entity_list.size(); j++)
			{
				if (engine->entity_list[j]->nodraw == true)
					continue;

				if (strlen(engine->entity_list[j]->target) <= 1)
					continue;

				if (strstr(engine->entity_list[i]->target_name, engine->entity_list[j]->target) != NULL)
				{
					draw_line(engine->entity_list[i], engine->entity_list[j], engine->menu, color, real_projection);
				}
			}

		}
	}

	engine->projection = real_projection;
}

void Quake3::draw_name(Entity *entity, Menu &menu, matrix4 &real_projection)
{
	matrix4 trans2;
	matrix4 mvp2;
	matrix4 model;
	vec3 color(1.0f, 1.0f, 1.0f);


	engine->camera_frame.set(trans2);
	mvp2 = trans2.premultiply(entity->rigid->get_matrix(model.m)) * real_projection;
	vec4 pos = mvp2 * vec4(0.0f, 0.0f, 0.0f, 1.0f); // model space coordinate

	pos.x = pos.x / pos.w;
	pos.y = -pos.y / pos.w; // negative y? Hey it works
	pos.z = pos.z / pos.w;

	pos.x = 0.5f + (pos.x * 0.5f);
	pos.y = 0.5f + (pos.y * 0.5f);
	//			pos.z = 0.5f + (pos.z * 0.5f);

	engine->projection = engine->identity;

	if (pos.z >= -1.0 && pos.z <= 1.0)
	{
		char data[512];

		/*
		menu.draw_text(entity->type, pos.x, pos.y - 0.0625f, 0.02f, color);
		sprintf(data, "bsp_leaf: %d", entity->bsp_leaf);
		menu.draw_text(data, pos.x, pos.y, 0.02f, color);
		*/

		if (strcmp(entity->type, "free") == 0)
		{
			int line = 1;

			sprintf(data, "Pos %.3f %.3f %.3f", entity->position.x, entity->position.y, entity->position.z);
			menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, color);
			sprintf(data, "Vel %.3f %.3f %.3f", entity->rigid->velocity.x, entity->rigid->velocity.y, entity->rigid->velocity.z);
			menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, color);
			sprintf(data, "State %d", entity->rigid->sleep);
			menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, color );
		}

		if (strcmp(entity->type, "navpoint") == 0)
		{
			vec3 blue(0.0f, 0.0f, 1.0f);
			vec3 green(0.0f, 1.0f, 0.0f);
			int line = 1;

			sprintf(data, "targetname %s", entity->target_name);
			menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.025f, blue);
			sprintf(data, "target %s", entity->target);
			menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.025f, green);
		}

		if (strcmp(entity->type, "light") == 0)
		{
			int line = 1;

			if (entity->light != NULL)
			{
				sprintf(data, "intensity %f", entity->light->intensity);
				menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, color);

				sprintf(data, "color %.3f %.3f %.3f", entity->light->color.x, entity->light->color.y, entity->light->color.z);
				menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, color);
			}
		}

		if (strcmp(entity->type, "NPC") == 0)
		{
			int line = 1;
			vec3 red(1.0f, 0.0f, 0.0f);
			vec3 white(1.0f, 1.0f, 1.0f);

			sprintf(data, "Health %d", entity->player->health);
			menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, red);

//			sprintf(data, "Bot State %s", bot_state_name[entity->player->bot_state]);
//			menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, red);

//			if (entity->player->bot_state == BOT_GET_ITEM)
//			{
//				sprintf(data, "Item: %s", engine->entity_list[entity->player->get_item]->type);
//				menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, white);
//			}
		}

		if (strcmp(entity->type, "func_plat") == 0 || strcmp(entity->type, "func_bobbing") == 0 ||
			strcmp(entity->type, "func_train") == 0)
		{
			int line = 1;

			if (entity->light != NULL)
			{
				sprintf(data, "target %s", entity->target);
				menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, color);
				sprintf(data, "target_name %s", entity->target_name);
				menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, color);
			}
		}
	}

	engine->projection = real_projection;
}


void Quake3::draw_line(Entity *ent_a, Entity *ent_b, Menu &menu, vec3 &color, matrix4 &real_projection)
{
	matrix4 trans2;
	matrix4 mvp2;
	matrix4 model;

	vec3 a;
	vec3 b;
	vec3 pos;


	transform_3d_2d(ent_a->position, a, real_projection);
	transform_3d_2d(ent_b->position, b, real_projection);

	if ((a.z >= -1.0 && a.z <= 1.0) || (b.z >= -1.0 && b.z <= 1.0))
	{
		engine->projection = engine->identity;

		for (int i = 0; i < 50; i++)
		{
			lerp(a, b, 1.0f / i, pos);

			menu.draw_text("o", pos.x, pos.y, 0.02f, color);
		}
		engine->projection = real_projection;
	}
}

void Quake3::transform_3d_2d(vec3 &position, vec3 &pos2d, matrix4 &projection)
{
	matrix4 matrix;
	matrix4 trans2;
	matrix4 mvp2;

	matrix.m[0] = 1.0f;
	matrix.m[1] = 0.0f;
	matrix.m[2] = 0.0f;
	matrix.m[3] = 0.0f;

	matrix.m[4] = 0.0f;
	matrix.m[5] = 1.0f;
	matrix.m[6] = 0.0f;
	matrix.m[7] = 0.0f;

	matrix.m[8] = 0.0f;
	matrix.m[9] = 0.0f;
	matrix.m[10] = 1.0f;
	matrix.m[11] = 0.0f;

	matrix.m[12] = position.x;
	matrix.m[13] = position.y;
	matrix.m[14] = position.z;
	matrix.m[15] = 1.0f;



	engine->camera_frame.set(trans2);
	mvp2 = trans2.premultiply(matrix.m) * projection;
	vec4 pos_4d = mvp2 * vec4(0.0f, 0.0f, 0.0f, 1.0f); // model space coordinate

	pos2d.x = pos_4d.x / pos_4d.w;
	pos2d.y = -pos_4d.y / pos_4d.w;
	pos2d.z = pos_4d.z / pos_4d.w;

	pos2d.x = 0.5f + (pos2d.x * 0.5f);
	pos2d.y = 0.5f + (pos2d.y * 0.5f);
}


int Quake3::bot_get_path(int item, int self, int *nav_array, path_t &path)
{
	vec3 target_pos = engine->entity_list[item]->position;
	vec3 self_pos = engine->entity_list[self]->position;
	float min_target = FLT_MAX;
	float min_self = FLT_MAX;
	int target_index = -1;
	int self_index = -1;


	int j = 0;

	for (unsigned int i = 0; i < engine->entity_list.size(); i++)
	{
		if (strcmp(engine->entity_list[i]->type, "navpoint") == 0)
		{
			float distance_self = (engine->entity_list[i]->position - self_pos).magnitude();
			float distance_target = (engine->entity_list[i]->position - target_pos).magnitude();

			if (distance_target < min_target)
			{
				min_target = distance_target;
				target_index = i;
			}

			if (distance_self < min_self)
			{
				min_self = distance_self;
				self_index = i;
			}

			nav_array[j++] = i;
		}
	}


	if (target_index == -1 || self_index == -1)
	{
//		printf("bot_find: No nav points!\n");
		return -2;
	}


	int start_path = atoi(engine->entity_list[self_index]->target_name + 3);
	int end_path = atoi(engine->entity_list[target_index]->target_name + 3);

	if (start_path == end_path)
		return -1;

	engine->find_path(path.path, path.length, start_path, end_path);
	path.step = 0;
	return 0;
}

int Quake3::bot_follow(path_t &path, int *nav_array, Entity *entity, float speed_scale)
{
	static int timer = 0;

	timer++;

	if (timer == 15 * TICK_RATE)
	{
		// Taking too long to follow path, force new path selection
//		printf("Giving up on path, probably stuck\n");
		path.length = -1;
		timer = 0;
		return 1;
	}

	for (int i = path.step; i < path.length; i++)
	{
		int nav = nav_array[path.path[i]];

		if ((entity->position - engine->entity_list[nav]->position).magnitude() > 15.0f)
		{
			//static int jitter = 0;
			static vec3 last_position = entity->position;

			entity->rigid->lookat_yaw(engine->entity_list[nav]->position);
			entity->rigid->move_forward(speed_scale);

			if ((last_position - entity->position).magnitude() > 800.0f)
			{
				//probably teleported, give up and get new path
				path.length = -1;
				return 1;
			}

			last_position = entity->position;

			if (rand() % 200 == 0)
				entity->rigid->move_left(speed_scale);
			if (rand() % 114 == 0)
				entity->rigid->move_right(speed_scale);


			//moving towards step
			return 1;
		}
		else
		{
//			printf("Bot arrived at nav point nav%d\n", path.path[i]);
			timer = 0;
			return 0;
		}
	}

	// Couldnt find nav node
	return 0;
}
