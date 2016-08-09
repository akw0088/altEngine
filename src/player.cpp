#include "player.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Player::Player(Entity *entity, Graphics &gfx, Sound &audio)
: weapon_rocket(entity), weapon_shotgun(entity), weapon_lightning(entity), weapon_railgun(entity)
{
	current_light = 0;
	Player::entity = entity;

	attack_sound = "";
	weapon_idle_sound = "";
	health = 100;
	armor = 0;
	weapon_flags = 0;
	current_weapon = wp_none;
	ammo_rockets = 0;
	ammo_slugs = 0;
	ammo_shells = 0;
	ammo_bullets = 50;
	ammo_lightning = 0;
	ammo_plasma = 0;
	ammo_bfg = 0;
	reload_timer = 0;

	weapon_rocket.load(gfx,		"media/models/weapons2/rocketl/rocketl");
	weapon_lightning.load(gfx,	"media/models/weapons2/lightning/lightning");
	weapon_railgun.load(gfx,	"media/models/weapons2/railgun/railgun");
	weapon_shotgun.load(gfx,	"media/models/weapons2/shotgun/shotgun");


	entity->speaker = new Speaker(entity);

	entity->speaker->loop = false;
	entity->speaker->source = audio.create_source(entity->speaker->loop, false);
	alSourcef(entity->speaker->source, AL_GAIN, 4.0f);
	audio.effects(entity->speaker->source);

	//	weapon_model.center = entity->rigid->center;
}




void Player::handle_weapons(button_t &keyboard, Frame &camera, vector<Entity *> &entity_list, int spawn, Graphics &gfx, Sound &audio, vector<wave_t> &snd_wave)
{
	if (reload_timer > 0)
		reload_timer--;

	switch (current_weapon)
	{
	case wp_railgun:
		weapon_idle_sound = "media/sound/weapons/railgun/rg_hum.wav";
//		audio.select_buffer(entity->speaker->source, snd_wave[WP_RAILGUN_IDLE].buffer);
		break;
	case wp_lightning:
		weapon_idle_sound = "media/sound/weapons/lightning/lg_hum.wav";
		break;
	default:
		weapon_idle_sound = "";
		break;
	}

	if (keyboard.leftbutton && reload_timer == 0)
	{

		if (current_weapon == wp_rocket && ammo_rockets > 0)
		{
			reload_timer = 120; // two seconds

			Entity *entity = new Entity();
			entity->rigid = new RigidBody(entity);
			entity->position = camera.pos;
			entity->rigid->clone(*(entity_list[0]->model));
			entity->rigid->velocity = camera.forward * -1.0f;
			entity->rigid->angular_velocity = vec3();
			entity->rigid->gravity = false;
			entity->model = entity->rigid;
			entity->rigid->set_target( *(entity_list[spawn]) );
			camera.set(entity->model->morientation);
			entity_list.push_back(entity);
			ammo_rockets--;

			attack_sound = "media/sound/weapons/rocket/rocklf1a.wav";
		}
		else if (current_weapon == wp_lightning && ammo_lightning > 0)
		{
			reload_timer = 10;

			Entity *entity = new Entity();
			entity->rigid = new RigidBody(entity);
			entity->position = camera.pos;
			entity->rigid->load(gfx, "media/models/ball");
			entity->rigid->velocity = camera.forward * -125.0f;
			entity->rigid->angular_velocity = vec3();
			entity->rigid->gravity = false;
			entity->model = entity->rigid;
//			entity->rigid->set_target(*(entity_list[spawn]));
			camera.set(entity->model->morientation);
			entity_list.push_back(entity);
			ammo_lightning--;

			attack_sound = "media/sound/weapons/lightning/lg_fire.wav";
		}
		else if (current_weapon == wp_railgun && ammo_slugs > 0)
		{
			reload_timer = 120; // two seconds

			Entity *entity = new Entity();
			entity->rigid = new RigidBody(entity);
			entity->position = camera.pos;
			entity->rigid->clone(*(entity_list[1]->model));
			entity->rigid->velocity = camera.forward * -100.0f;
			entity->rigid->angular_velocity = vec3();
			entity->rigid->gravity = false;
			entity->model = entity->rigid;
			camera.set(entity->model->morientation);
			entity_list.push_back(entity);
			ammo_slugs--;

			attack_sound = "media/sound/weapons/railgun/railgf1a.wav";
		}
		else if (current_weapon == wp_shotgun && ammo_shells > 0)
		{
			reload_timer = 60; // one seconds

			ammo_shells--;

			attack_sound = "media/sound/weapons/shotgun/sshotf1b.wav";
		}

		
		for (int i = 0; i < snd_wave.size(); i++)
		{
			if (strcmp(snd_wave[i].file, attack_sound) == 0)
			{
				audio.select_buffer(entity->speaker->source, snd_wave[i].buffer);
				break;
			}
		}
		audio.play(entity->speaker->source);

	}
}

void Player::render_weapon(Graphics &gfx)
{
	switch (current_weapon)
	{
	case wp_none:
		break;
	case wp_shotgun:
		weapon_shotgun.render(gfx);
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
	}
}

void Player::change_weapon_up()
{

	current_light++;
	switch (current_weapon)
	{
	case wp_none:
		if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		else if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		break;
	case wp_shotgun:
		if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		break;
	case wp_rocket:
		if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		break;
	case wp_lightning:
		if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		break;
	case wp_railgun:
		break;
	}
}

void Player::change_weapon_down()
{
	current_light--;
	switch (current_weapon)
	{
	case wp_none:
		if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		else if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		break;
	case wp_shotgun:
		break;
	case wp_rocket:
		if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		break;
	case wp_lightning:
		if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		break;
	case wp_railgun:
		if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		break;
	}
}

Player::~Player()
{
}