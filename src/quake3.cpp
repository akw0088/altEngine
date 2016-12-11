#include "include.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Quake3::Quake3()
{
	blink = false;
}

void Quake3::init(Engine *altEngine)
{
	engine = altEngine;
}

void Quake3::step(int frame_step)
{
	static int footstep_num = 0;
	int spawn = engine->spawn;
	Entity *entity = engine->entity_list[spawn];

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
			entity->rigid->water == false && entity->player->dead == false &&
			entity->rigid->noclip == false)
		{

			if ((entity->position - entity->rigid->old_position).magnitude() > 1.0f && frame_step % 20 == 0)
			{
				bool ret;

				switch (footstep_num++ % 4)
				{
				case 0:
					ret = engine->select_wave(entity->speaker->source, entity->player->step1_sound);
					break;
				case 1:
					ret = engine->select_wave(entity->speaker->source, entity->player->step2_sound);
					break;
				case 2:
					ret = engine->select_wave(entity->speaker->source, entity->player->step3_sound);
					break;
				case 3:
					ret = engine->select_wave(entity->speaker->source, entity->player->step4_sound);
					break;
				}

				if (ret)
				{
					engine->audio.play(entity->speaker->source);
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
				bool ret;

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

		if (entity->player->health <= 0 && entity->player->dead == false)
		{
			bool ret = false;

			if (entity->player->health <= -50)
			{
				debugf("%s was gibbed\n", entity->player->name);

				ret = engine->select_wave(entity->speaker->source, entity->player->gibbed_sound);

				handle_gibs(*(entity->player), engine->camera_frame);
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

			entity->player->kill();
			entity->model->clone(*(engine->box->model));
		}
		handle_weapons(*(entity->player), engine->camera_frame, engine->input);
	}
}

void Quake3::handle_plasma(Entity *entity, Player &player, Frame &camera_frame)
{
	sprintf(player.attack_sound, "sound/weapons/plasma/hyprbf1a.wav");

	player.reload_timer = 8;
	player.ammo_plasma--;

	entity->rigid = new RigidBody(entity);
	entity->model = entity->rigid;
	entity->position = camera_frame.pos;
	camera_frame.set(entity->model->morientation);

	entity->rigid->clone(*(engine->ball->model));
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

void Quake3::handle_rocketlauncher(Entity *entity, Player &player, Frame &camera_frame)
{
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
	entity->rigid->clone(*(engine->rocket->model));
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

void Quake3::handle_grenade(Entity *entity, Player &player, Frame &camera_frame)
{
	sprintf(player.attack_sound, "sound/weapons/grenade/grenlf1a.wav");

	player.reload_timer = 100;
	player.ammo_grenades--;

	entity->rigid = new RigidBody(entity);
	entity->model = entity->rigid;
	entity->position = camera_frame.pos;
	camera_frame.set(entity->model->morientation);

	entity->rigid->clone(*(engine->box->model));
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

void Quake3::handle_lightning(Entity *entity, Player &player, Frame &camera_frame)
{
	sprintf(player.attack_sound, "sound/weapons/lightning/lg_fire.wav");

	player.reload_timer = 6;
	player.ammo_lightning--;

	entity->rigid = new RigidBody(entity);
	entity->position = camera_frame.pos;
	entity->rigid->clone(*(engine->box->model));
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

void Quake3::handle_railgun(Entity *entity, Player &player, Frame &camera_frame)
{
	int index[8];
	int num_index;

	sprintf(player.attack_sound, "sound/weapons/railgun/railgf1a.wav");

	player.reload_timer = 188;
	player.ammo_slugs--;

	entity->rigid = new RigidBody(entity);
	entity->position = camera_frame.pos;
	entity->rigid->clone(*(engine->ball->model));
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

		debugf("Player %s hit %s with the railgun for %d damage\n", player.name, engine->entity_list[index[i]]->player->name, 100);
		sprintf(cmd, "hurt %d %d", index[i], 100);
		engine->console(cmd);
	}
}

void Quake3::handle_machinegun(Player &player, Frame &camera_frame)
{
	int index[8];
	int num_index;
	vec3 forward;
	float distance;


	sprintf(player.attack_sound, "sound/weapons/machinegun/machgf1b.wav");

	player.reload_timer = 8;
	player.ammo_bullets--;
	player.entity->model->getForwardVector(forward);

	engine->hitscan(player.entity->position, forward, index, num_index, engine->spawn);
	for (int i = 0; i < num_index; i++)
	{
		char cmd[80] = { 0 };

		if (engine->entity_list[index[i]]->player == NULL)
			continue;

		debugf("Player %s hit %s with the machinegun for %d damage\n", player.name, engine->entity_list[index[i]]->player->name, 7);
		sprintf(cmd, "hurt %d %d", index[i], 7);
		engine->console(cmd);
	}

	engine->map.hitscan(player.entity->position, forward, distance);
	//vec3 end = player.entity->position + forward * distance;


	//			Entity *entity = new Entity();
	//			entity->decal = new Decal(entity);
	//			entity->position = end;
	//			entity->decal->normal = normal;

}

void Quake3::handle_gibs(Player &player, Frame &camera_frame)
{

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

void Quake3::handle_shotgun(Player &player, Frame &camera_frame)
{
	vec3 forward;
	float distance;
	//	int index[8];
	//	int num_index;


	player.reload_timer = 60;
	player.ammo_shells--;
	player.entity->model->getForwardVector(forward);

	sprintf(player.attack_sound, "sound/weapons/shotgun/sshotf1b.wav");

	engine->map.hitscan(player.entity->position, forward, distance);
	//vec3 end = player.entity->position + forward * distance;


	//			Entity *entity = new Entity();
	//			entity->decal = new Decal(entity);
	//			entity->position = end;
	//			entity->decal->normal = normal;

}


void Quake3::handle_weapons(Player &player, Frame &frame, button_t &input)
{
	bool fired = false;
	bool empty = false;

	if (player.reload_timer > 0)
	{
		player.reload_timer--;
	}

	if (player.dead)
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

		bool ret = engine->select_wave(player.entity->speaker->source, player.weapon_swap_sound);
		if (ret)
		{
			engine->audio.play(player.entity->speaker->source);
		}
		else
		{
			debugf("Unable to find PCM data for %s\n", player.weapon_idle_sound);
		}
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
				handle_rocketlauncher(entity, player, frame);
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
				handle_plasma(entity, player, frame);
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
				handle_grenade(entity, player, frame);
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
				handle_lightning(entity, player, frame);
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
				handle_railgun(entity, player, frame);
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
				handle_shotgun(player, frame);
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
				handle_machinegun(player, frame);
			}
			else
			{
				empty = true;
			}
		}

		if (fired)
		{
			bool ret = false;

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