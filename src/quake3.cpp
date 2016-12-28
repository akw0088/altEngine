#include "include.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern char bot_state_name[16][80];

Quake3::Quake3()
{
	blink = false;
}

void Quake3::init(Engine *altEngine)
{
	engine = altEngine;

	box = new Entity();
	box->rigid = new RigidBody(box);
	box->model = box->rigid;
	box->model->load(engine->gfx, "media/models/box");

	ball = new Entity();
	ball->rigid = new RigidBody(ball);
	ball->model = ball->rigid;
	ball->model->load(engine->gfx, "media/models/ball");

	thug22 = new Entity();
	thug22->rigid = new RigidBody(thug22);
	thug22->model = thug22->rigid;
	thug22->model->load(engine->gfx, "media/models/thug22/thug22");

	rocket = new Entity();
	rocket->rigid = new RigidBody(rocket);
	rocket->model = rocket->rigid;
	rocket->rigid->load(engine->gfx, "media/models/weapons2/rocketl/rocket");

	pineapple = new Entity();
	pineapple->rigid = new RigidBody(pineapple);
	pineapple->model = pineapple->rigid;
	pineapple->rigid->load(engine->gfx, "media/models/weapons2/grenadel/pineapple");
}

void Quake3::destroy()
{
	delete box;
	delete ball;
	delete thug22;
	delete rocket;
	delete pineapple;
}

/*
item_health
item_health_large
item_armor_shard
item_health_mega
item_quad
holdable_medkit
holdable_teleporter
item_enviro
item_flight
item_haste
item_invis
item_regen

ammo_bullets
ammo_rockets
ammo_slugs
ammo_shells
ammo_lightning
ammo_plasma

weapon_rocketlauncher
weapon_lightning
weapon_shotgun
weapon_railgun
weapon_plasma
weapon_grenadelauncher
item_armor_combat
item_armor_body
trigger_teleport
*/


void Quake3::step(int frame_step)
{
	static int footstep_num = 0;
	int spawn = engine->spawn;

	if (spawn == -1)
		return;

	if (engine->entity_list.size() == 0)
		return;

	Entity *entity = engine->entity_list[spawn];

	if (engine->server_flag == false && engine->client_flag == false && engine->enemy == -1)
	{
		engine->enemy = engine->get_player();
		Entity *entity = engine->entity_list[engine->enemy];
		debugf("Adding an enemy player\n");
		entity->rigid = new RigidBody(entity);
		entity->model = entity->rigid;
		entity->rigid->clone(*(thug22->model));
		sprintf(entity->type, "NPC");
		entity->player = new Player(entity, engine->gfx, engine->audio, 16);
		entity->speaker->gain(5.0f);
		sprintf(entity->player->name, "thug22");
		entity->position += entity->rigid->center + vec3(0.0f, 50.0f, 0.0f);
		char cmd[80];
		sprintf(cmd, "respawn %d %d", -1, engine->enemy);
		engine->console(cmd);
//		entity->rigid->pursue_flag = true;
//		entity->rigid->set_target(*(engine->entity_list[engine->spawn]));
	}


	//player movement
	if (engine->menu.ingame == false && engine->menu.console == false)
	{
		if (engine->input.control == true)
			engine->camera_frame.update(engine->input);
		else if (spawn != -1)
		{
			if (engine->entity_list[spawn]->player->health > 0)
			{

				// True if jumped
				if (engine->input.up || engine->input.down || engine->input.left || engine->input.right)
				{
					entity->player->state = PLAYER_MOVED;
				}

				if (engine->input.shift)
				{
					entity->player->state = PLAYER_DUCKED;
				}


				if (entity->rigid->move(engine->input))
				{
					entity->player->state = PLAYER_JUMPED;
					engine->select_wave(entity->speaker->source, entity->player->jump_sound);
					engine->audio.play(entity->speaker->source);
				}

			}
			else
			{
				button_t noinput;

				memset(&noinput, 0, sizeof(button_t));
				entity->player->state = PLAYER_DEAD;
				//Makes body hit the floor, need to explore why this hack is needed
				if (entity->player->reload_timer)
				{
					entity->rigid->move(noinput);
				}
			}

			if (engine->entity_list[spawn]->player->teleport_timer > 0)
			{
				engine->entity_list[spawn]->player->teleport_timer--;
			}

			if (engine->entity_list[spawn]->player->flight_timer > 0)
			{
				engine->entity_list[spawn]->player->flight_timer--;
				engine->entity_list[spawn]->rigid->flight = true;
				engine->entity_list[spawn]->rigid->translational_friction = 0.9f;
			}
			else
			{
				engine->entity_list[spawn]->rigid->flight = false;
				engine->entity_list[spawn]->rigid->translational_friction = 0.0f;
			}
		}

		if (engine->enemy != -1)
		{
			Entity *enemy_ent = engine->entity_list[engine->enemy];
			button_t input;


			enemy_ent->player->avoid_walls(engine->map);
			enemy_ent->player->bot_search_for_items(engine->entity_list, engine->enemy);

			switch (enemy_ent->player->bot_state)
			{
			case BOT_ATTACK:
				engine->zcc.select_animation(0);
				memset(&input, 0, sizeof(button_t));
				input.leftbutton = true;
				handle_weapons(*(enemy_ent->player), input, engine->enemy);
				break;
			case BOT_DEAD:
				engine->zcc.select_animation(1);
				enemy_ent->model->clone(*(box->model));
				engine->select_wave(enemy_ent->speaker->source, enemy_ent->player->death1_sound);
				engine->audio.play(enemy_ent->speaker->source);

				enemy_ent->player->respawn();
				char cmd[80];
				sprintf(cmd, "respawn -1 %d", engine->enemy);
				engine->console(cmd);
				break;
			case BOT_IDLE:
			case BOT_ALERT:
				engine->zcc.select_animation(1);
				break;
			}
		}
	}


	if (frame_step % TICK_RATE == 0)
	{
		blink = !blink;
	}


	if (spawn != -1)
	{
		if (frame_step % TICK_RATE == 0)
		{
			if (entity->player->health > 100)
			{
				entity->player->health--;
			}

			if (entity->player->armor > 100)
			{
				entity->player->armor--;
			}
		}

		if (entity->rigid->velocity.y > -1.0f && entity->rigid->velocity.y < 1.0f &&
			entity->rigid->water == false && entity->player->state != PLAYER_DEAD &&
			entity->rigid->noclip == false && entity->rigid->gravity == true )
		{

			if ((entity->position - entity->rigid->old_position).magnitude() > 0.8f && frame_step % 20 == 0)
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

		if (entity->player->health <= 0 && entity->player->state != PLAYER_DEAD)
		{
			bool ret = false;

			if (entity->player->health <= -50)
			{
				debugf("%s was gibbed\n", entity->player->name);

				ret = engine->select_wave(entity->speaker->source, entity->player->gibbed_sound);

				handle_gibs(*(entity->player));
			}
			else
			{
				debugf("%s died\n", entity->player->name);

				switch (frame_step % 3)
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

			entity->player->state = PLAYER_DEAD;
			drop_weapon->trigger = new Trigger(drop_weapon, engine->audio);
			snprintf(drop_weapon->trigger->pickup_sound, LINE_SIZE, "sound/misc/w_pkup.wav");
			snprintf(drop_weapon->trigger->respawn_sound, LINE_SIZE, "sound/items/s_health.wav");
			sprintf(drop_weapon->trigger->action, "%s", weapon_str);

			entity->player->kill();
			entity->model->clone(*(box->model));
		}
		handle_weapons(*(entity->player), engine->input, engine->spawn);
	}
}

void Quake3::handle_plasma(Entity *entity, Player &player)
{
	Frame camera_frame;

	player.entity->model->get_frame(camera_frame);

	sprintf(player.attack_sound, "sound/weapons/plasma/hyprbf1a.wav");

	player.reload_timer = 8;
	player.ammo_plasma--;

	entity->rigid = new RigidBody(entity);
	entity->model = entity->rigid;
	entity->position = camera_frame.pos;
	camera_frame.set(entity->model->morientation);

	entity->rigid->clone(*(ball->model));
	entity->rigid->velocity = camera_frame.forward * -10.0f;
	entity->rigid->net_force = camera_frame.forward * -10.0f;

	entity->rigid->angular_velocity = vec3();
	entity->rigid->gravity = false;
	entity->trigger = new Trigger(entity, engine->audio);
	sprintf(entity->trigger->explode_sound, "sound/weapons/plasma/plasmx1a.wav");
	sprintf(entity->trigger->idle_sound, "sound/weapons/plasma/lasfly.wav");
	sprintf(entity->trigger->action, "damage 20");

	entity->trigger->hide = false;
	entity->trigger->self = false;
	entity->trigger->idle = true;
	entity->trigger->explode = false;
	entity->trigger->explode_timer = 10;
	entity->trigger->explode_color = vec3(0.0f, 0.0f, 1.0f);
	entity->trigger->explode_intensity = 200.0f;
	entity->trigger->splash_damage = 15;
	entity->trigger->splash_radius = 75.0f;
	entity->trigger->knockback = 75.0f;

	entity->light = new Light(entity, engine->gfx, 999);
	entity->light->color = vec3(0.0f, 0.0f, 1.0f);
	entity->light->intensity = 1000.0f;

}

void Quake3::handle_rocketlauncher(Entity *entity, Player &player)
{
	Frame camera_frame;

	player.entity->model->get_frame(camera_frame);


	sprintf(player.attack_sound, "sound/weapons/rocket/rocklf1a.wav");

	player.reload_timer = 100;
	player.ammo_rockets--;

	entity->position = camera_frame.pos;

	entity->trigger = new Trigger(entity, engine->audio);
	sprintf(entity->trigger->explode_sound, "sound/weapons/rocket/rocklx1a.wav");
	sprintf(entity->trigger->idle_sound, "sound/weapons/rocket/rockfly.wav");
	sprintf(entity->trigger->action, "damage 100");

	entity->trigger->hide = false;
	entity->trigger->self = false;
	entity->trigger->idle = true;
	entity->trigger->explode = true;
	entity->trigger->idle_timer = 0;
	entity->trigger->explode_timer = 10;
	entity->trigger->explode_color = vec3(1.0f, 0.0f, 0.0f);
	entity->trigger->explode_intensity = 500.0f;
	entity->trigger->splash_damage = 50;
	entity->trigger->splash_radius = 250.0f;
	entity->trigger->knockback = 250.0f;

	entity->light = new Light(entity, engine->gfx, 999);
	entity->light->color = vec3(1.0f, 1.0f, 1.0f);
	entity->light->intensity = 1000.0f;

	entity->rigid = new RigidBody(entity);
	entity->model = entity->rigid;
	camera_frame.set(entity->rigid->morientation);
	entity->rigid->clone(*(rocket->model));
	entity->rigid->velocity = camera_frame.forward * -6.25f;
	entity->rigid->net_force = camera_frame.forward * -10.0f;
	entity->rigid->angular_velocity = vec3();
	entity->rigid->gravity = false;




	bool ret = engine->select_wave(entity->trigger->loop_source, entity->trigger->idle_sound);
	if (ret)
	{
		engine->audio.play(entity->trigger->loop_source);
	}
	else
	{
		debugf("Unable to find PCM data for %s\n", entity->trigger->idle_sound);
	}
}

void Quake3::handle_grenade(Entity *entity, Player &player)
{
	Frame camera_frame;

	player.entity->model->get_frame(camera_frame);


	sprintf(player.attack_sound, "sound/weapons/grenade/grenlf1a.wav");

	player.reload_timer = 100;
	player.ammo_grenades--;

	entity->rigid = new RigidBody(entity);
	entity->model = entity->rigid;
	entity->position = camera_frame.pos;
	camera_frame.set(entity->model->morientation);

	entity->rigid->clone(*(box->model));
	//entity->rigid->clone(*(pineapple->model));
	entity->rigid->velocity = camera_frame.forward * -5.0f;
	entity->rigid->angular_velocity = vec3(0.1f, 0.1f, 0.1f);
	entity->rigid->gravity = true;
	entity->rigid->rotational_friction_flag = true;
	//entity->rigid->set_target(*(entity_list[spawn]));

	entity->trigger = new Trigger(entity, engine->audio);
	sprintf(entity->trigger->explode_sound, "sound/weapons/rocket/rocklx1a.wav");
	sprintf(entity->trigger->action, "damage 100");

	entity->trigger->hide = false;
	entity->trigger->self = false;
	entity->trigger->idle = true;
	entity->trigger->idle_timer = 120;
	entity->trigger->explode = true;
	entity->trigger->explode_timer = 10;
	entity->trigger->explode_color = vec3(1.0f, 0.0f, 0.0f);
	entity->trigger->explode_intensity = 500.0f;
	entity->trigger->splash_damage = 50;
	entity->trigger->splash_radius = 250.0f;
	entity->trigger->knockback = 250.0f;
}

void Quake3::handle_lightning(Entity *entity, Player &player)
{
	Frame camera_frame;

	player.entity->model->get_frame(camera_frame);


	sprintf(player.attack_sound, "sound/weapons/lightning/lg_fire.wav");

	player.reload_timer = 6;
	player.ammo_lightning--;

	entity->rigid = new RigidBody(entity);
	entity->position = camera_frame.pos;
	entity->rigid->clone(*(box->model));
	entity->rigid->velocity = camera_frame.forward * -1.0f;
	entity->rigid->angular_velocity = vec3();
	entity->rigid->gravity = false;
	entity->rigid->rotational_friction_flag = true;
	entity->model = entity->rigid;
	entity->rigid->set_target(*(engine->entity_list[engine->spawn]));
	camera_frame.set(entity->model->morientation);

	entity->light = new Light(entity, engine->gfx, 999);
	entity->light->color = vec3(1.0f, 1.0f, 1.0f);
	entity->light->intensity = 1000.0f;
}

void Quake3::handle_railgun(Entity *entity, Player &player)
{
	int index[8];
	int num_index;

	Frame camera_frame;

	player.entity->model->get_frame(camera_frame);


	sprintf(player.attack_sound, "sound/weapons/railgun/railgf1a.wav");

	player.reload_timer = 188;
	player.ammo_slugs--;

	entity->rigid = new RigidBody(entity);
	entity->position = camera_frame.pos;
	entity->rigid->clone(*(ball->model));
	entity->rigid->velocity = camera_frame.forward * -100.0f;
	entity->rigid->angular_velocity = vec3();
	entity->rigid->gravity = false;
	entity->model = entity->rigid;
	camera_frame.set(entity->model->morientation);

	vec3 forward;
	player.entity->model->getForwardVector(forward);

	engine->hitscan(player.entity->position, forward, index, num_index, engine->spawn);
	for (int i = 0; i < num_index; i++)
	{
		char cmd[80] = { 0 };

		if (engine->entity_list[index[i]]->player == NULL)
			continue;

		debugf("Player %s hit %s with the railgun for %d damage\n", player.name,
			engine->entity_list[index[i]]->player->name, 100);

		debugf("%s has %d health\n", engine->entity_list[index[i]]->player->name,
			engine->entity_list[index[i]]->player->health);
		sprintf(cmd, "hurt %d %d", index[i], 100);
		engine->console(cmd);
	}
}

void Quake3::handle_machinegun(Player &player, int self)
{
	char cmd[80] = { 0 };
	int index[8];
	int num_index;
	vec3 forward;
	//float distance;

	Frame camera_frame;

	player.entity->model->get_frame(camera_frame);


	sprintf(player.attack_sound, "sound/weapons/machinegun/machgf1b.wav");

	player.reload_timer = 8;
	player.ammo_bullets--;
	player.entity->model->getForwardVector(forward);

	if (self == 1)
	{
		debugf("Player %s hit %s with the machinegun for %d damage\n", engine->entity_list[self]->player->name, player.name, 7);
		sprintf(cmd, "hurt %d %d", engine->spawn, 7);
		engine->console(cmd);
	}

	engine->hitscan(player.entity->position, forward, index, num_index, self);
	for (int i = 0; i < num_index; i++)
	{

		if (engine->entity_list[index[i]]->player == NULL)
			continue;

		debugf("Player %s hit %s with the machinegun for %d damage\n", player.name, engine->entity_list[index[i]]->player->name, 7);
		sprintf(cmd, "hurt %d %d", index[i], 7);
		debugf("%s has %d health\n", engine->entity_list[index[i]]->player->name,
			engine->entity_list[index[i]]->player->health);
		engine->console(cmd);
	}

//	engine->map.hitscan(player.entity->position, forward, distance);
	//vec3 end = player.entity->position + forward * distance;


	//			Entity *entity = new Entity();
	//			entity->decal = new Decal(entity);
	//			entity->position = end;
	//			entity->decal->normal = normal;

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

		entity1->rigid->clone(*(box->model));
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

		entity2->rigid->clone(*(box->model));
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

		entity3->rigid->clone(*(box->model));
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

void Quake3::handle_shotgun(Player &player)
{
	Frame camera_frame;

	player.entity->model->get_frame(camera_frame);


	vec3 forward;
	//float distance;
	int index[8];
	int num_index;


	player.reload_timer = 60;
	player.ammo_shells--;
	player.entity->model->getForwardVector(forward);

	sprintf(player.attack_sound, "sound/weapons/shotgun/sshotf1b.wav");

//	engine->map.hitscan(player.entity->position, forward, distance);

	player.entity->model->getForwardVector(forward);

	engine->hitscan(player.entity->position, forward, index, num_index, engine->spawn);
	for (int i = 0; i < num_index; i++)
	{
		char cmd[80] = { 0 };

		if (engine->entity_list[index[i]]->player == NULL)
			continue;

		debugf("Player %s hit %s with the shotgun for %d damage\n", player.name, engine->entity_list[index[i]]->player->name, 50);
		sprintf(cmd, "hurt %d %d", index[i], 50);
		debugf("%s has %d health\n", engine->entity_list[index[i]]->player->name,
			engine->entity_list[index[i]]->player->health);

		engine->console(cmd);
	}

	//vec3 end = player.entity->position + forward * distance;


	//			Entity *entity = new Entity();
	//			entity->decal = new Decal(entity);
	//			entity->position = end;
	//			entity->decal->normal = normal;

}


void Quake3::handle_weapons(Player &player, button_t &input, int self)
{
	bool fired = false;
	bool empty = false;

	if (player.reload_timer > 0)
	{
		player.reload_timer--;
	}

	if (player.state == PLAYER_DEAD)
	{
		if (input.leftbutton && player.reload_timer == 0)
		{
			engine->console("respawn");
		}
		return;
	}

	if (player.current_weapon != player.last_weapon)
	{
		switch (player.current_weapon)
		{
		case wp_railgun:
			sprintf(player.weapon_idle_sound, "sound/weapons/railgun/rg_hum.wav");
			//		audio.select_buffer(entity->speaker->source, snd_wave[WP_RAILGUN_IDLE].buffer);
			break;
		case wp_lightning:
			//			player.weapon_idle_sound = "sound/weapons/lightning/lg_hum.wav";
			player.weapon_idle_sound[0] = '\0';
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

	if (input.leftbutton && player.reload_timer == 0)
	{
		if (player.current_weapon == wp_rocket)
		{
			if (player.ammo_rockets > 0)
			{
				fired = true;
				Entity *entity = engine->entity_list[engine->get_entity()];
				handle_rocketlauncher(entity, player);
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
				handle_plasma(entity, player);
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
				handle_grenade(entity, player);
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
				handle_lightning(entity, player);
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
				handle_railgun(entity, player);
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
				handle_shotgun(player);
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

	int spawn = engine->spawn;
	Entity *entity = engine->entity_list[spawn];

	engine->projection = engine->identity;
	vec3 color(1.0f, 1.0f, 1.0f);

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
			vec3 color = vec3(1.0f, 1.0f, 1.0f);
			matrix4 trans2;
			matrix4 mvp2;
			matrix4 model;

			engine->camera_frame.set(trans2);

			if (i == (unsigned int)spawn)
			{
				color = vec3(1.0f, 0.0f, 0.0f);
			}

			if (engine->entity_list[i]->rigid == NULL)
				continue;


			mvp2 = trans2.premultiply(engine->entity_list[i]->rigid->get_matrix(model.m)) * real_projection;
			vec4 pos = mvp2 * vec4(0.0f, 0.0f, 0.0f, 1.0f); // model space coordinate

			pos.x = pos.x / pos.w;
			pos.y = -pos.y / pos.w; // negative y? Hey it works
			pos.z = pos.z / pos.w;

			pos.x = 0.5f + (pos.x * 0.5f);
			pos.y = 0.5f + (pos.y * 0.5f);
			//			pos.z = 0.5f + (pos.z * 0.5f);

			if (engine->entity_list[i]->visible)
			{
				draw_name(pos, engine->entity_list[i], engine->menu, color);
			}
		}
	}

	engine->projection = real_projection;
}

void Quake3::draw_name(vec4 &pos, Entity *entity, Menu &menu, vec3 &color)
{
	if (pos.z >= -1.0 && pos.z <= 1.0)
	{
		char data[512];

		menu.draw_text(entity->type, pos.x, pos.y - 0.0625f, 0.02f, color);
		sprintf(data, "bsp_leaf: %d", entity->bsp_leaf);
		menu.draw_text(data, pos.x, pos.y, 0.02f, color);

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

			sprintf(data, "Health %d", entity->player->health);
			menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, red);

			sprintf(data, "Bot State %s", bot_state_name[entity->player->bot_state]);
			menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, red);
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
}
