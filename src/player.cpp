#include "player.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include <float.h>

Player::Player(Entity *entity, Graphics &gfx, Audio &audio)
: weapon_machinegun(entity), weapon_shotgun(entity), weapon_grenade(entity), weapon_rocket(entity),
  weapon_lightning(entity), weapon_railgun(entity), weapon_plasma(entity)
{
	Player::entity = entity;

	// Player sound sets
	char *models[]{
		"anarki",			//0
		"biker",			//1
		"bitterman",		//2
		"bones",			//3
		"crash",			//4
		"doom",				//5
		"grunt",			//6
		"hunter",			//7
		"keel",				//8
		"klesk",			//9
		"lucy",				//10
		"major",			//11
		"mynx",				//12
		"orbb",				//13
		"ranger",			//14
		"razor",			//15
		"sarge",			//16
		"slash",			//17
		"sorlag",			//18
		"tankjr",			//19
		"uriel",			//20
		"visor",			//21
		"xaero"				//22
	};

	model_index = 14; // default to ranger

	sprintf(model_name, models[model_index]);
	attack_sound[0] = '\0';
	weapon_idle_sound[0] = '\0';
	sprintf(death1_sound, "sound/player/%s/death1.wav", model_name);
	sprintf(death2_sound, "sound/player/%s/death2.wav", model_name);
	sprintf(death3_sound, "sound/player/%s/death3.wav", model_name);

	sprintf(pain25_sound, "sound/player/%s/pain25_1.wav", model_name);
	sprintf(pain50_sound, "sound/player/%s/pain50_1.wav", model_name);
	sprintf(pain75_sound, "sound/player/%s/pain75_1.wav", model_name);
	sprintf(pain100_sound, "sound/player/%s/pain100_1.wav", model_name);

	sprintf(jump_sound, "sound/player/%s/jump1.wav", model_name);
	sprintf(land_sound, "sound/player/%s/fall1.wav", model_name);

	sprintf(empty_sound, "sound/weapons/noammo.wav");
	sprintf(weapon_swap_sound, "sound/weapons/change.wav");

	sprintf(step1_sound, "sound/player/footsteps/step1.wav");
	sprintf(step2_sound, "sound/player/footsteps/step2.wav");
	sprintf(step3_sound, "sound/player/footsteps/step3.wav");
	sprintf(step4_sound, "sound/player/footsteps/step4.wav");

	sprintf(waterin_sound,  "sound/player/watr_in.wav");
	sprintf(waterout_sound, "sound/player/watr_out.wav");

	sprintf(gurp1_sound, "sound/player/gurp1.wav");
	sprintf(gurp2_sound, "sound/player/gurp2.wav");

	sprintf(gibbed_sound, "sound/player/gibsplt1.wav");
	sprintf(gibimpact1_sound, "sound/player/gibimp1.wav");
	sprintf(gibimpact2_sound, "sound/player/gibimp2.wav");
	sprintf(gibimpact3_sound, "sound/player/gibimp3.wav");


	sprintf(telein_sound, "sound/world/telein.wav");
	sprintf(teleout_sound, "sound/world/teleout.wav");
	


	// Probably should be global and not tied to player entity
	sprintf(chat_sound, "sound/player/talk.wav");
	

	//sounds/player/watr_un.wav // another water in?
	//sound/player/fry.wav

	health = 100;
	armor = 0;
	weapon_flags = 0;
	current_weapon = wp_none;
	ammo_rockets = 0;
	ammo_grenades = 0;
	ammo_slugs = 0;
	ammo_shells = 0;
	ammo_bullets = 50;
	ammo_lightning = 0;
	ammo_plasma = 0;
	ammo_bfg = 0;
	reload_timer = 0;
	drown_timer++;
	memcpy(name, "UnnamedPlayer", strlen("UnnamedPlayer") + 1);
	memset(&stats, 0, sizeof(stats_t));
	dead = false;

	weapon_machinegun.load(gfx, "media/models/weapons2/machinegun/machinegun");
	weapon_shotgun.load(gfx,	"media/models/weapons2/shotgun/shotgun");
	weapon_grenade.load(gfx,	"media/models/weapons2/grenadel/grenade");
	weapon_rocket.load(gfx,		"media/models/weapons2/rocketl/rocketl");
	weapon_lightning.load(gfx,	"media/models/weapons2/lightning/lightning");
	weapon_railgun.load(gfx,	"media/models/weapons2/railgun/railgun");
	weapon_plasma.load(gfx,		"media/models/weapons2/plasma/plasma");


	entity->speaker = new Speaker(entity, audio);
	strcpy(entity->speaker->file, "info_player_deathmatch");
#ifndef __OBJC__
    alSourcef(entity->speaker->source, AL_GAIN, 4.0f);
#endif
	audio.effects(entity->speaker->source);

	//	weapon_model.center = entity->rigid->center;

	current_light = 0;
	current_face = 0;
}

void Player::load_sounds(Audio &audio, std::vector<wave_t> &snd_wave)
{
	wave_t wave;

	// Player sound sets
	char *models[]
	{
		"anarki",			//0
		"biker",			//1
		"bitterman",		//2
		"bones",			//3
		"crash",			//4
		"doom",				//5
		"grunt",			//6
		"hunter",			//7
		"keel",				//8
		"klesk",			//9
		"lucy",				//10
		"major",			//11
		"mynx",				//12
		"orbb",				//13
		"ranger",			//14
		"razor",			//15
		"sarge",			//16
		"slash",			//17
		"sorlag",			//18
		"tankjr",			//19
		"uriel",			//20
		"visor",			//21
		"xaero"				//22
	};


	//load player sounds
	strcpy(wave.file, "sound/weapons/railgun/rg_hum.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/weapons/lightning/lg_hum.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/weapons/shotgun/sshotf1b.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/weapons/railgun/railgf1a.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/weapons/lightning/lg_fire.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/weapons/rocket/rocklf1a.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/weapons/grenade/grenlf1a.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/weapons/plasma/hyprbf1a.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/weapons/machinegun/machgf1b.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/world/telein.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/world/teleout.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);


	for (unsigned int i = 0; i < 23; i++)
	{
		sprintf(wave.file, "sound/player/%s/death1.wav", models[i]);
		audio.load(wave);
		if (wave.data != NULL)
			snd_wave.push_back(wave);

		sprintf(wave.file, "sound/player/%s/death2.wav", models[i]);
		audio.load(wave);
		if (wave.data != NULL)
			snd_wave.push_back(wave);

		sprintf(wave.file, "sound/player/%s/death3.wav", models[i]);
		audio.load(wave);
		if (wave.data != NULL)
			snd_wave.push_back(wave);


		sprintf(wave.file, "sound/player/%s/pain25_1.wav", models[i]);
		audio.load(wave);
		if (wave.data != NULL)
			snd_wave.push_back(wave);

		sprintf(wave.file, "sound/player/%s/pain50_1.wav", models[i]);
		audio.load(wave);
		if (wave.data != NULL)
			snd_wave.push_back(wave);

		sprintf(wave.file, "sound/player/%s/pain75_1.wav", models[i]);
		audio.load(wave);
		if (wave.data != NULL)
			snd_wave.push_back(wave);

		sprintf(wave.file, "sound/player/%s/pain100_1.wav", models[i]);
		audio.load(wave);
		if (wave.data != NULL)
			snd_wave.push_back(wave);

		sprintf(wave.file, "sound/player/%s/jump1.wav", models[i]);
		audio.load(wave);
		if (wave.data != NULL)
			snd_wave.push_back(wave);

		sprintf(wave.file, "sound/player/%s/fall1.wav", models[i]);
		audio.load(wave);
		if (wave.data != NULL)
			snd_wave.push_back(wave);
	}

	strcpy(wave.file, "sound/weapons/rocket/rockfly.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/weapons/rocket/rocklx1a.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/weapons/plasma/lasfly.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);


	strcpy(wave.file, "sound/weapons/plasma/plasmx1a.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/weapons/change.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/player/footsteps/step1.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/player/footsteps/step2.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/player/footsteps/step3.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/player/footsteps/step4.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/weapons/noammo.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/player/watr_in.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/player/watr_out.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/player/gurp1.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/player/gurp2.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/player/gibsplt1.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/player/gibimp1.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/player/gibimp2.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/player/gibimp3.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);

	strcpy(wave.file, "sound/player/talk.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);
}


void Player::respawn()
{
	attack_sound[0] = '\0';
	weapon_idle_sound[0] = '\0';
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
	entity->rigid->velocity = vec3(0.0f, 0.0f, 0.0f);
	entity->rigid->net_force = vec3(0.0f, 0.0f, 0.0f);
	dead = false;


//	entity->model->make_aabb();
}

void Player::kill()
{
	stats.deaths++;
	weapon_flags = 0;
	reload_timer = 120;
	current_weapon = wp_none;
	dead = true;
}

void Player::render_weapon(Graphics &gfx)
{
	switch (current_weapon)
	{
	case wp_none:
		break;
	case wp_machinegun:
		weapon_machinegun.render(gfx);
		break;
	case wp_shotgun:
		weapon_shotgun.render(gfx);
		break;
	case wp_grenade:
		weapon_grenade.render(gfx);
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
	case wp_plasma:
		weapon_plasma.render(gfx);
		break;
	}
}

void Player::change_weapon_up()
{
	if (reload_timer != 0)
		return;

	switch (current_weapon)
	{
	case wp_none:
		if (weapon_flags & WEAPON_MACHINEGUN)
			current_weapon = wp_machinegun;
		else if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		else if (weapon_flags & WEAPON_GRENADE)
			current_weapon = wp_grenade;
		else if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		else if (weapon_flags & WEAPON_PLASMA)
			current_weapon = wp_plasma;
		break;
	case wp_machinegun:
		if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		else if (weapon_flags & WEAPON_GRENADE)
			current_weapon = wp_grenade;
		else if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		else if (weapon_flags & WEAPON_PLASMA)
			current_weapon = wp_plasma;
		break;
	case wp_shotgun:
		if (weapon_flags & WEAPON_GRENADE)
			current_weapon = wp_grenade;
		else if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		else if (weapon_flags & WEAPON_PLASMA)
			current_weapon = wp_plasma;
		break;
	case wp_grenade:
		if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		else if (weapon_flags & WEAPON_PLASMA)
			current_weapon = wp_plasma;
		break;
	case wp_rocket:
		if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		else if (weapon_flags & WEAPON_PLASMA)
			current_weapon = wp_plasma;
		break;
	case wp_lightning:
		if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		else if (weapon_flags & WEAPON_PLASMA)
			current_weapon = wp_plasma;
		break;
	case wp_railgun:
		if (weapon_flags & WEAPON_PLASMA)
			current_weapon = wp_plasma;
		break;
	case wp_plasma:
		break;
	}
}

void Player::change_weapon_down()
{
	if (reload_timer != 0)
		return;

	switch (current_weapon)
	{
	case wp_none:
		if (weapon_flags & WEAPON_PLASMA)
			current_weapon = wp_plasma;
		else if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		else if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_GRENADE)
			current_weapon = wp_grenade;
		else if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		else if (weapon_flags & WEAPON_MACHINEGUN)
			current_weapon = wp_machinegun;
		break;
	case wp_machinegun:
		break;
	case wp_shotgun:
		if (weapon_flags & WEAPON_MACHINEGUN)
			current_weapon = wp_machinegun;
		break;
	case wp_grenade:
		if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		else if (weapon_flags & WEAPON_MACHINEGUN)
			current_weapon = wp_machinegun;
		break;
	case wp_rocket:
		if (weapon_flags & WEAPON_GRENADE)
			current_weapon = wp_grenade;
		else if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		else if (weapon_flags & WEAPON_MACHINEGUN)
			current_weapon = wp_machinegun;
		break;
	case wp_lightning:
		if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		else if (weapon_flags & WEAPON_MACHINEGUN)
			current_weapon = wp_machinegun;
		break;
	case wp_railgun:
		if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		else if (weapon_flags & WEAPON_MACHINEGUN)
			current_weapon = wp_machinegun;
		break;
	case wp_plasma:
		if (weapon_flags & WEAPON_RAILGUN)
			current_weapon = wp_railgun;
		else if (weapon_flags & WEAPON_LIGHTNING)
			current_weapon = wp_lightning;
		else if (weapon_flags & WEAPON_ROCKET)
			current_weapon = wp_rocket;
		else if (weapon_flags & WEAPON_SHOTGUN)
			current_weapon = wp_shotgun;
		else if (weapon_flags & WEAPON_MACHINEGUN)
			current_weapon = wp_machinegun;
		break;
	}
}


float Player::DistanceToLine(vec3 &direction, vec3 &origin, vec3 &point)
{
	return vec3::crossproduct(direction, point - origin).magnitude();
}

int Player::FindLookAt(vec3 &cameraOrigin, vec3 &cameraDir, vec3 *points, int numPoints)
{
	int index = -1;
	float min = FLT_MAX;

	for (int i = 0; i < numPoints; i++)
	{
		float distance = DistanceToLine(cameraDir, cameraOrigin, points[i]);

		if (distance < min)
		{
			min = distance;
			index = i;
		}
	}
	return index;
}


Player::~Player()
{
}
