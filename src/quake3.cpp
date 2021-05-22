//=============================================================================
// This file is part of the altEngine distribution
// (https://github.com/akw0088/altEngine/)
// Copyright (c) 2018 Alexander Wright All Rights Reserved.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON - INFRINGEMENT.IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//=============================================================================

int raster_enabled = 0;
int raster_target = 0;

///============================================================================
/// File: quake3.cpp
///============================================================================
/// Game class that has all the game logic, inherits from basegame, which provides
/// the interface used by engine. Note, this class is a friend of engine, which
/// kind of makes them the same class essentially, ideally the friend relationship
/// will be removed and just access things through the basegame interface
///
/// Called quake3, because it essentially is a recreation of quake3 game logic,
/// but could have assets swapped and become it's own thing pretty easily.
/// Heavily tied to quake3 BSP's for levels and collision detection though
/// (which isnt a bad thing)
///
/// You can make maps using qeradiant / gtk radiant same style as for quake3
///
/// Bunny hopping / movement sadly doesnt feel as good as quake3
///
///============================================================================

#include "include.h"
#include <math.h> // for sin/cos

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef WIN32
extern "C" __declspec(dllexport) BaseGame* __cdecl create_class()
{
	return new Quake3;
}
#else
extern "C" BaseGame* create_class()
{
	return new Quake3;
}
#endif

///=============================================================================
/// Function: Quake3
///=============================================================================
/// Description: Constructor for quake3 class
///
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
Quake3::Quake3()
{
	gametype = GAMETYPE_DEATHMATCH;
	fraglimit = 8;
	timelimit = 10;
	capturelimit = 8;
	blue_flag_caps = 0;
	red_flag_caps = 0;
	notif_timer = 0;
	chat_timer = 0;
	engine = NULL;
	spectator = false;
	flyby = false;

	engine = NULL;
	blink = false;
	spectator = false;
	warmup = true;
	hold_fire = false;
	gametype = GAMETYPE_CTF;
	last_spawn = 0;
	spectator_timer = 0;
	chat_timer = 0;
	notif_timer = 0;
	fraglimit = 10;
	timelimit = 10;
	warmup_time = 30;
	round_time = 0;
	capturelimit = 8;
	weapon_switch_timer = 0;
	win_timer = 0;
	win_msg[0] = '\0';

	crosshair_vbo = 0;
	current_crosshair = 0;
	crosshair_scale = 1.0f;
	icon_vbo = 0;

	num_player = 0;
	num_player_red = 0;
	num_player_blue = 0;
	old_contrast = 2.0f;

	played_one_frag = false;
	played_two_frag = false;
	played_three_frag = false;
	played_prepare = false;
	played_sudden = false;
	faceicon = true;

}

///=============================================================================
/// Function: init
///=============================================================================
/// Description: initializes the quake3 class
///
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Quake3::init(Engine *altEngine)
{
	engine = altEngine;
	create_crosshair();
	crosshair_scale = 1.0f;


	load_icon();
	create_icon();
	load_sounds(engine->audio, engine->snd_wave);
	//sounds/player/watr_un.wav // another water in?
	//sound/player/fry.wav

	load_models(engine->gfx);

//	load_q1_models(engine->gfx);
}

///=============================================================================
/// Function: load
///=============================================================================
/// Description: load function called when a level is loaded
///
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Quake3::load(gametype_t type)
{
	char cmd[128];
	last_spawn = 0;
	gametype = type;

	current_crosshair = engine->menu.data.crosshair;

	sprintf(cmd, "name \"%s\"", engine->menu.data.name);
	engine->console(cmd);
}

///=============================================================================
/// Function: unload
///=============================================================================
/// Description: unload function called when a level is unloaded
///
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Quake3::unload()
{
	last_spawn = 0;
	warmup = true;
	warmup_time = 30;
	round_time = 0;
}

///=============================================================================
/// Function: destroy
///=============================================================================
/// Description: destroy function called when the program is about to terminate
///
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Quake3::destroy()
{
#if 0
	bool dump_obj = false;

	for (unsigned int i = 0; i < model_table.size(); i++)
	{
		if (dump_obj)
		{
			strcat(model_table[i]->name, ".obj");
			WriteObj(model_table[i]->name, model_table[i]->model_vertex_array, model_table[i]->num_vertex, model_table[i]->model_index_array, model_table[i]->num_index);
		}
		model_table[i] = NULL;
	}
#endif
	delete [] model_table;

	// uncomment when created from dll / shared library
	//delete this;
}


///=============================================================================
/// Function: load_models
///=============================================================================
/// Description: loads models such as weapons
///
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Quake3::load_models(Graphics &gfx)
{
	model_table = new EntModel[512];
	num_model = 0;
	#define MODEL_ROCKET 0	
	model_table[num_model++].load(gfx, "media/models/weapons2/rocketl/rocket");

	#define MODEL_GRENADE 1
	model_table[num_model++].load(gfx, "media/models/weapons2/grenadel/pineapple");

	#define MODEL_SHELL 2
	model_table[num_model++].load(gfx, "media/models/weapons2/shells/s_shell");


	#define MODEL_BULLET 3
	model_table[num_model++].load(gfx, "media/models/weapons2/shells/M_shell");


	#define MODEL_BULLET_HIT 4
	model_table[num_model++].load(gfx, "media/models/weaphits/bullet_hit");

	#define MODEL_PLASMA_HIT 5
	model_table[num_model++].load(gfx, "media/models/weaphits/plasma_hit");

	#define MODEL_BOOM 6
	model_table[num_model++].load(gfx, "media/models/weaphits/rlboom");


	#define MODEL_GIB0 7
	model_table[num_model++].load(gfx, "media/models/gibs/abdomen");

	#define MODEL_GIB1 8
	model_table[num_model++].load(gfx, "media/models/gibs/arm");

	#define MODEL_GIB2 9
	model_table[num_model++].load(gfx, "media/models/gibs/brain");

	#define MODEL_GIB3 10
	model_table[num_model++].load(gfx, "media/models/gibs/chest");

	#define MODEL_GIB4 11
	model_table[num_model++].load(gfx, "media/models/gibs/fist");

	#define MODEL_GIB5 12
	model_table[num_model++].load(gfx, "media/models/gibs/foot");

	#define MODEL_GIB6 13
	model_table[num_model++].load(gfx, "media/models/gibs/forearm");

	#define MODEL_GIB7 14
	model_table[num_model++].load(gfx, "media/models/gibs/intestine");

	#define MODEL_GIB8 15
	model_table[num_model++].load(gfx, "media/models/gibs/leg");

	#define MODEL_GIB9 16
	model_table[num_model++].load(gfx, "media/models/gibs/skull");

	#define MODEL_BOX 17
	model_table[num_model++].load(gfx, "media/models/box");

	#define MODEL_BALL 18
	model_table[num_model++].load(gfx, "media/models/ball");

	bool qc_weapons = false;

//	#define MODEL_WEAPON_GAUNTLET 19
	if (qc_weapons)
		model_table[num_model++].load(gfx, "media/models/weapons_qc/qc_gauntlet_fixed");
	else
		model_table[num_model++].load(gfx, "media/models/weapons2/gauntlet/gauntlet");

//	#define MODEL_WEAPON_MACHINEGUN 20
	if (qc_weapons)
		model_table[num_model++].load(gfx, "media/models/weapons_qc/qc_machinegun_small");
	else
		model_table[num_model++].load(gfx, "media/models/weapons2/machinegun/machinegun");

//	#define MODEL_WEAPON_SHOTGUN 21
	
	if (qc_weapons)
		model_table[num_model++].load(gfx, "media/models/weapons_qc/qc_shotgun_small");
	else
		model_table[num_model++].load(gfx, "media/models/weapons2/shotgun/shotgun");

//	#define MODEL_WEAPON_GRENADE 22
	if (qc_weapons)
		model_table[num_model++].load(gfx, "media/models/weapons_qc/qc_tribolt_small");
	else
		model_table[num_model++].load(gfx, "media/models/weapons2/grenadel/grenade");

//	#define MODEL_WEAPON_ROCKET 23
	if (qc_weapons)
		model_table[num_model++].load(gfx, "media/models/weapons_qc/qc_rocketl_small");
	else
		model_table[num_model++].load(gfx, "media/models/weapons2/rocketl/rocketl");

//	#define MODEL_WEAPON_LIGHTNING 24
	if (qc_weapons)
		model_table[num_model++].load(gfx, "media/models/weapons_qc/qc_lightning_small");
	else
		model_table[num_model++].load(gfx, "media/models/weapons2/lightning/lightning");


//	#define MODEL_WEAPON_RAILGUN 25
	if (qc_weapons)
		model_table[num_model++].load(gfx, "media/models/weapons_qc/qc_railgun_small_long");
	else
		model_table[num_model++].load(gfx, "media/models/weapons2/railgun/railgun");

//	#define MODEL_WEAPON_PLASMA 26
	if (qc_weapons)
		model_table[num_model++].load(gfx, "media/models/weapons_qc/qc_nailgun_small");
	else
		model_table[num_model++].load(gfx, "media/models/weapons2/plasma/plasma");

	#define MODEL_ARMOR_SHARD 27
	model_table[num_model++].load(gfx, "media/models/powerups/armor/shard");

	#define MODEL_AMMO_SHELLS 28
	model_table[num_model++].load(gfx, "media/models/powerups/ammo/ammo_shells");

	#define MODEL_AMMO_ROCKETS 29
	model_table[num_model++].load(gfx, "media/models/powerups/ammo/ammo_rockets");

	#define MODEL_AMMO_LIGHTNING 30
	model_table[num_model++].load(gfx, "media/models/powerups/ammo/ammo_lightning");

	#define MODEL_AMMO_GRENADES 31
	model_table[num_model++].load(gfx, "media/models/powerups/ammo/ammo_grenades");

	#define MODEL_AMMO_SLUGS 32
	model_table[num_model++].load(gfx, "media/models/powerups/ammo/ammo_slugs");

	#define MODEL_AMMO_BULLETS 33
	model_table[num_model++].load(gfx, "media/models/powerups/ammo/ammo_bullets");

	#define MODEL_ARMOR_COMBAT 34
	model_table[num_model++].load(gfx, "media/models/powerups/armor/item_armor_combat");

	#define MODEL_ARMOR_BODY 35
	model_table[num_model++].load(gfx, "media/models/powerups/armor/item_armor_body");

	#define MODEL_BLUE_FLAG 36
	model_table[num_model++].load(gfx, "media/models/flags/b_flag");

	#define MODEL_RED_FLAG 37
	model_table[num_model++].load(gfx, "media/models/flags/r_flag");

	#define MODEL_QUAD 38
	model_table[num_model++].load(gfx, "media/models/powerups/instant/quad");

	#define MODEL_REGEN 39
	model_table[num_model++].load(gfx, "media/models/powerups/instant/regen");

	#define MODEL_INVIS 40
	model_table[num_model++].load(gfx, "media/models/powerups/instant/invis");

	#define MODEL_FLIGHT 41
	model_table[num_model++].load(gfx, "media/models/powerups/instant/flight");

	#define MODEL_HASTE 42
	model_table[num_model++].load(gfx, "media/models/powerups/instant/haste");

	#define MODEL_MEDKIT 43
	model_table[num_model++].load(gfx, "media/models/powerups/holdable/medkit");

	#define MODEL_TELEPORTER 44
	model_table[num_model++].load(gfx, "media/models/powerups/holdable/teleporter");

	#define MODEL_HEALTH 45
	model_table[num_model++].load(gfx, "media/models/powerups/health/item_health");

	#define MODEL_HEALTH_LARGE 46
	model_table[num_model++].load(gfx, "media/models/powerups/health/item_health_large");

	#define MODEL_HEALTH_SMALL 47
	model_table[num_model++].load(gfx, "media/models/powerups/health/item_health_small");

	#define MODEL_HEALTH_MEGA 48
	model_table[num_model++].load(gfx, "media/models/powerups/health/item_health_mega");

	#define MODEL_AMMO_PLASMA 49
	model_table[num_model++].load(gfx, "media/models/powerups/ammo/ammo_plasma");

	#define MODEL_TERRAIN 50
	model_table[num_model++].load(gfx, "media/terrain/terrain_big");

	#define MODEL_SENTRY1 51
	model_table[num_model++].load(gfx, "media/models/sentry/sentry1");

	#define MODEL_SENTRY2 52
	model_table[num_model++].load(gfx, "media/models/sentry/sentry2");

	#define MODEL_SENTRY3 53
	model_table[num_model++].load(gfx, "media/models/sentry/sentry3");

	#define MODEL_SENTRY_BASE 54
	model_table[num_model++].load(gfx, "media/models/sentry/sentry_base");

#define MODEL_LUCY 55
	model_table[num_model++].load(gfx, "media/models/lucy2");

}

///=============================================================================
/// Function: load_q1_models
///=============================================================================
/// Description: loads models from quake1, can serve as alternate set of items
///
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Quake3::load_q1_models(Graphics &gfx)
{
	model_table = new EntModel[512];
	num_model = 0;

	#define MODEL_ROCKET 0
	model_table[num_model++].load(gfx, "media/models/quake1/rock/rock");

	#define MODEL_GRENADE 1
	model_table[num_model++].load(gfx, "media/models/quake1/grenade/grenade");

#define MODEL_SHELL 2
	model_table[num_model++].load(gfx, "media/models/weapons2/shells/s_shell");


#define MODEL_BULLET 3
	model_table[num_model++].load(gfx, "media/models/weapons2/shells/M_shell");


#define MODEL_BULLET_HIT 4
	model_table[num_model++].load(gfx, "media/models/weaphits/bullet_hit");

#define MODEL_PLASMA_HIT 5
	model_table[num_model++].load(gfx, "media/models/weaphits/plasma_hit");

#define MODEL_BOOM 6
	model_table[num_model++].load(gfx, "media/models/weaphits/rlboom");

#define MODEL_GIB0 7
	model_table[num_model++].load(gfx, "media/models/gibs/abdomen");

#define MODEL_GIB1 8
	model_table[num_model++].load(gfx, "media/models/gibs/arm");

#define MODEL_GIB2 9
	model_table[num_model++].load(gfx, "media/models/gibs/brain");

#define MODEL_GIB3 10
	model_table[num_model++].load(gfx, "media/models/gibs/chest");

#define MODEL_GIB4 11
	model_table[num_model++].load(gfx, "media/models/gibs/fist");

#define MODEL_GIB5 12
	model_table[num_model++].load(gfx, "media/models/gibs/foot");

#define MODEL_GIB6 13
	model_table[num_model++].load(gfx, "media/models/gibs/forearm");

#define MODEL_GIB7 14
	model_table[num_model++].load(gfx, "media/models/gibs/intestine");

#define MODEL_GIB8 15
	model_table[num_model++].load(gfx, "media/models/gibs/leg");

#define MODEL_GIB9 16
	model_table[num_model++].load(gfx, "media/models/gibs/skull");

#define MODEL_BOX 17
	model_table[num_model++].load(gfx, "media/models/box");

#define MODEL_BALL 18
	model_table[num_model++].load(gfx, "media/models/ball");

	//	#define MODEL_WEAPON_GAUNTLET 19
	model_table[num_model++].load(gfx, "media/models/quake1/v_axe/v_axe");

	//	#define MODEL_WEAPON_MACHINEGUN 20
	model_table[num_model++].load(gfx, "media/models/quake1/g_nail/g_nail");

	//	#define MODEL_WEAPON_SHOTGUN 21
	model_table[num_model++].load(gfx, "media/models/quake1/g_shot/g_shot");

	//	#define MODEL_WEAPON_GRENADE 22
	model_table[num_model++].load(gfx, "media/models/quake1/g_rock/g_rock");

	//	#define MODEL_WEAPON_ROCKET 23
	model_table[num_model++].load(gfx, "media/models/quake1/g_rock2/g_rock2");

	//	#define MODEL_WEAPON_LIGHTNING 24
	model_table[num_model++].load(gfx, "media/models/quake1/g_light/g_light");

	//	#define MODEL_WEAPON_RAILGUN 25
	model_table[num_model++].load(gfx, "media/models/quake1/g_light/g_light");

	//	#define MODEL_WEAPON_PLASMA 26
	model_table[num_model++].load(gfx, "media/models/weapons2/plasma/plasma");

#define MODEL_ARMOR_SHARD 27
	model_table[num_model++].load(gfx, "media/models/powerups/armor/shard");

#define MODEL_AMMO_SHELLS 28
	model_table[num_model++].load(gfx, "media/models/powerups/ammo/ammo_shells");

#define MODEL_AMMO_ROCKETS 29
	model_table[num_model++].load(gfx, "media/models/powerups/ammo/ammo_rockets");

#define MODEL_AMMO_LIGHTNING 30
	model_table[num_model++].load(gfx, "media/models/powerups/ammo/ammo_lightning");

#define MODEL_AMMO_GRENADES 31
	model_table[num_model++].load(gfx, "media/models/powerups/ammo/ammo_grenades");

#define MODEL_AMMO_SLUGS 32
	model_table[num_model++].load(gfx, "media/models/powerups/ammo/ammo_slugs");

#define MODEL_AMMO_BULLETS 33
	model_table[num_model++].load(gfx, "media/models/powerups/ammo/ammo_bullets");

#define MODEL_ARMOR_COMBAT 34
	model_table[num_model++].load(gfx, "media/models/quake1/armor/yellowarmor");

#define MODEL_ARMOR_BODY 35
	model_table[num_model++].load(gfx, "media/models/quake1/armor/redarmor");

#define MODEL_BLUE_FLAG 36
	model_table[num_model++].load(gfx, "media/models/flags/b_flag");

#define MODEL_RED_FLAG 37
	model_table[num_model++].load(gfx, "media/models/flags/r_flag");

#define MODEL_QUAD 38
	model_table[num_model++].load(gfx, "media/models/quake1/quaddama/quaddama");

#define MODEL_REGEN 39
	model_table[num_model++].load(gfx, "media/models/powerups/instant/regen");

#define MODEL_INVIS 40
	model_table[num_model++].load(gfx, "media/models/quake1/invisibl/invisibl");

#define MODEL_FLIGHT 41
	model_table[num_model++].load(gfx, "media/models/powerups/instant/flight");

#define MODEL_HASTE 42
	model_table[num_model++].load(gfx, "media/models/powerups/instant/haste");

#define MODEL_MEDKIT 43
	model_table[num_model++].load(gfx, "media/models/powerups/holdable/medkit");

#define MODEL_TELEPORTER 44
	model_table[num_model++].load(gfx, "media/models/powerups/holdable/teleporter");

#define MODEL_HEALTH 45
	model_table[num_model++].load(gfx, "media/models/powerups/health/item_health");

#define MODEL_HEALTH_LARGE 46
	model_table[num_model++].load(gfx, "media/models/powerups/health/item_health_large");

#define MODEL_HEALTH_SMALL 47
	model_table[num_model++].load(gfx, "media/models/powerups/health/item_health_small");

#define MODEL_HEALTH_MEGA 48
	model_table[num_model++].load(gfx, "media/models/powerups/health/item_health_mega");

#define MODEL_AMMO_PLASMA 49
	model_table[num_model++].load(gfx, "media/models/powerups/ammo/ammo_plasma");

#define MODEL_SHAMBLER 50
	model_table[num_model++].load(gfx, "media/models/quake1/shambler/shambler");

#define MODEL_WIZARD 51
	model_table[num_model++].load(gfx, "media/models/quake1/wizard/wizard");

#define MODEL_OGRE 52
	model_table[num_model++].load(gfx, "media/models/quake1/ogre/ogre");

#define MODEL_DOG 53
	model_table[num_model++].load(gfx, "media/models/quake1/dog/dog");

#define MODEL_ZOMBIE 54
	model_table[num_model++].load(gfx, "media/models/quake1/zombie/zombie");

#define MODEL_PLAYER 55
	model_table[num_model++].load(gfx, "media/models/quake1/player/player");

#define MODEL_DEMON 56
	model_table[num_model++].load(gfx, "media/models/quake1/demon/demon");

#define MODEL_SOLDIER 57
	model_table[num_model++].load(gfx, "media/models/quake1/soldier/soldier");

#define MODEL_KNIGHT 58
	model_table[num_model++].load(gfx, "media/models/quake1/knight/knight");

#define MODEL_REDARMOR 59
	model_table[num_model++].load(gfx, "media/models/quake1/armor/redarmor");

#define MODEL_YELLOWARMOR 60
	model_table[num_model++].load(gfx, "media/models/quake1/armor/yellowarmor");

#define MODEL_GREENARMOR 61
	model_table[num_model++].load(gfx, "media/models/quake1/armor/greenarmor");

#define MODEL_BACKPACK 62
	model_table[num_model++].load(gfx, "media/models/quake1/backpack/backpack");

#define MODEL_BOLT 63
	model_table[num_model++].load(gfx, "media/models/quake1/bolt/bolt");

#define MODEL_BOLT2 64
	model_table[num_model++].load(gfx, "media/models/quake1/bolt2/bolt2");

#define MODEL_BOLT3 65
	model_table[num_model++].load(gfx, "media/models/quake1/bolt3/bolt3");

#define MODEL_BOSS 66
	model_table[num_model++].load(gfx, "media/models/quake1/boss/boss");

#define MODEL_END1 67
	model_table[num_model++].load(gfx, "media/models/quake1/end1/end1");

#define MODEL_EYES 68
	model_table[num_model++].load(gfx, "media/models/quake1/eyes/eyes");

#define MODEL_FLAME 69
	model_table[num_model++].load(gfx, "media/models/quake1/flame/flame");

#define MODEL_FLAME2 70
	model_table[num_model++].load(gfx, "media/models/quake1/flame2/flame2");

#define MODEL_G_LIGHT 71
	model_table[num_model++].load(gfx, "media/models/quake1/g_light/g_light");

#define MODEL_G_NAIL 72
	model_table[num_model++].load(gfx, "media/models/quake1/g_nail/g_nail");

#define MODEL_G_NAIL2 73
	model_table[num_model++].load(gfx, "media/models/quake1/g_nail2/g_nail2");

#define MODEL_G_ROCK 74
	model_table[num_model++].load(gfx, "media/models/quake1/g_rock/g_rock");

#define MODEL_G_ROCK2 75
	model_table[num_model++].load(gfx, "media/models/quake1/g_rock2/g_rock2");

#define MODEL_G_SHOT 76
	model_table[num_model++].load(gfx, "media/models/quake1/g_shot/g_shot");

#define MODEL_Q1_GIB1 77
	model_table[num_model++].load(gfx, "media/models/quake1/gib1/gib1");

#define MODEL_Q1_GIB2 78
	model_table[num_model++].load(gfx, "media/models/quake1/gib2/gib2");

#define MODEL_Q1_GIB3 79
	model_table[num_model++].load(gfx, "media/models/quake1/gib3/gib3");

#define MODEL_Q1_GRENADE 80
	model_table[num_model++].load(gfx, "media/models/quake1/grenade/grenade");

#define MODEL_H_DEMON 81
	model_table[num_model++].load(gfx, "media/models/quake1/h_demon/h_demon");

#define MODEL_H_DOG 82
	model_table[num_model++].load(gfx, "media/models/quake1/h_dog/h_dog");

#define MODEL_H_GUARD 83
	model_table[num_model++].load(gfx, "media/models/quake1/h_guard/h_guard");

#define MODEL_H_KNIGHT 84
	model_table[num_model++].load(gfx, "media/models/quake1/h_knight/h_knight");

#define MODEL_H_OGRE 85
	model_table[num_model++].load(gfx, "media/models/quake1/h_ogre/h_ogre");

#define MODEL_H_PLAYER 86
	model_table[num_model++].load(gfx, "media/models/quake1/h_player/h_player");

#define MODEL_H_SHAMS 87
	model_table[num_model++].load(gfx, "media/models/quake1/h_shams/h_shams");

#define MODEL_H_WIZARD 88
	model_table[num_model++].load(gfx, "media/models/quake1/h_wizard/h_wizard");

#define MODEL_H_ZOMBIE 89
	model_table[num_model++].load(gfx, "media/models/quake1/h_zombie/h_zombie");

#define MODEL_INVISIBL 90
	model_table[num_model++].load(gfx, "media/models/quake1/invisibl/invisibl");

#define MODEL_INVULNER 91
	model_table[num_model++].load(gfx, "media/models/quake1/invulner/invulner");

#define MODEL_LAVABALL 92
	model_table[num_model++].load(gfx, "media/models/quake1/lavaball/lavaball");
	
#define MODEL_M_G_KEY 93
	model_table[num_model++].load(gfx, "media/models/quake1/m_g_key/m_g_key");

#define MODEL_M_S_KEY 94
	model_table[num_model++].load(gfx, "media/models/quake1/m_s_key/m_s_key");

#define MODEL_MISSLE 95
	model_table[num_model++].load(gfx, "media/models/quake1/missle/missle");

#define MODEL_Q1_PLAYER 96
	model_table[num_model++].load(gfx, "media/models/quake1/player/player");

#define MODEL_QUADDAMA 97
	model_table[num_model++].load(gfx, "media/models/quake1/quaddama/quaddama");

#define MODEL_S_LIGHT 98
	model_table[num_model++].load(gfx, "media/models/quake1/s_light/s_light");

#define MODEL_S_SPIKE 99
	model_table[num_model++].load(gfx, "media/models/quake1/s_spike/s_spike");

#define MODEL_SPIKE 100
	model_table[num_model++].load(gfx, "media/models/quake1/spike/spike");

#define MODEL_SUIT 101
	model_table[num_model++].load(gfx, "media/models/quake1/suit/suit");

#define MODEL_V_AXE 102
	model_table[num_model++].load(gfx, "media/models/quake1/v_axe/v_axe");

#define MODEL_V_LIGHT 103
	model_table[num_model++].load(gfx, "media/models/quake1/v_light/v_light");

#define MODEL_V_NAIL 104
	model_table[num_model++].load(gfx, "media/models/quake1/v_nail/v_nail");

#define MODEL_V_NAIL2 105
	model_table[num_model++].load(gfx, "media/models/quake1/v_nail2/v_nail2");

#define MODEL_V_ROCK 106
	model_table[num_model++].load(gfx, "media/models/quake1/v_rock/v_rock");

#define MODEL_V_ROCK2 107
	model_table[num_model++].load(gfx, "media/models/quake1/v_rock2/v_rock2");

#define MODEL_V_SHOT 108
	model_table[num_model++].load(gfx, "media/models/quake1/v_shot/v_shot");

#define MODEL_V_SHOT2 109
	model_table[num_model++].load(gfx, "media/models/quake1/v_shot2/v_shot2");

#define MODEL_W_G_KEY 110
	model_table[num_model++].load(gfx, "media/models/quake1/w_g_key/w_g_key");

#define MODEL_W_S_KEY 111
	model_table[num_model++].load(gfx, "media/models/quake1/w_s_key/w_s_key");

#define MODEL_W_SPIKE 112
	model_table[num_model++].load(gfx, "media/models/quake1/w_spike/w_spike");

#define MODEL_W_ZOM_GIB 113
	model_table[num_model++].load(gfx, "media/models/quake1/zom_gib/zom_gib");
}


///=============================================================================
/// Function: load_sounds
///=============================================================================
/// Description: loads audio files for later use
///
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Quake3::load_sounds(Audio &audio, vector<wave_t> &snd_wave)
{
	wave_t wave;

	//load player sounds
#define SND_PLAYER	10 // +23 * 10
#define SND_DEATH1	0
#define SND_DEATH2	1
#define SND_DEATH3	2
#define SND_PAIN1	3
#define SND_PAIN2	4
#define SND_PAIN3	5
#define SND_PAIN4	6
#define SND_JUMP	7
#define SND_FALL	8
#define SND_FALLING 9


	for (unsigned int i = 0; i < 23; i++)
	{
		sprintf(wave.file, "sound/player/%s/death1.wav", EntPlayer::models[i]);
		audio.load(wave, engine->pk3_list, engine->num_pk3);
		snd_wave.push_back(wave);
		snd_table[SND_PLAYER + i * 11] = snd_wave.size() - 1;


		sprintf(wave.file, "sound/player/%s/death2.wav", EntPlayer::models[i]);
		audio.load(wave, engine->pk3_list, engine->num_pk3);
		snd_wave.push_back(wave);
		snd_table[SND_PLAYER + i * 11 + 1] = snd_wave.size() - 1;


		sprintf(wave.file, "sound/player/%s/death3.wav", EntPlayer::models[i]);
		audio.load(wave, engine->pk3_list, engine->num_pk3);
		snd_wave.push_back(wave);
		snd_table[SND_PLAYER + i * 11 + 2] = snd_wave.size() - 1;


		sprintf(wave.file, "sound/player/%s/pain25_1.wav", EntPlayer::models[i]);
		audio.load(wave, engine->pk3_list, engine->num_pk3);
		snd_wave.push_back(wave);
		snd_table[SND_PLAYER + i * 11 + 3] = snd_wave.size() - 1;


		sprintf(wave.file, "sound/player/%s/pain50_1.wav", EntPlayer::models[i]);
		audio.load(wave, engine->pk3_list, engine->num_pk3);
		snd_wave.push_back(wave);
		snd_table[SND_PLAYER + i * 11 + 4] = snd_wave.size() - 1;

		sprintf(wave.file, "sound/player/%s/pain75_1.wav", EntPlayer::models[i]);
		audio.load(wave, engine->pk3_list, engine->num_pk3);
		snd_wave.push_back(wave);
		snd_table[SND_PLAYER + i * 11 + 5] = snd_wave.size() - 1;


		sprintf(wave.file, "sound/player/%s/pain100_1.wav", EntPlayer::models[i]);
		audio.load(wave, engine->pk3_list, engine->num_pk3);
		snd_wave.push_back(wave);
		snd_table[SND_PLAYER + i * 11 + 6] = snd_wave.size() - 1;

/*
		Cool Quake 1 sounds

		sound/items/health1.wav
		sound/items/r_item1.wav
		sound/misc/r_tele1.wav
		sound/misc/r_tele2.wav
		sound/misc/r_tele3.wav
		sound/misc/r_tele4.wav
		sound/misc/r_tele5.wav
		sound/misc/talk.wav
		sound/misc/water1.wav
		sound/misc/water2.wav
		sound/player/death1.wav
		sound/player/death2.wav
		sound/player/death3.wav
		sound/player/death4.wav
		sound/player/death5.wav
		sound/player/drown1.wav
		sound/player/drown2.wav
		sound/player/gasp1.wav
		sound/player/gasp2.wav
		sound/player/h2odeath.wav
		sound/player/lburn1.wav
		sound/player/lburn2.wav
		sound/player/pain1.wav
		sound/player/pain2.wav
		sound/player/pain3.wav
		sound/player/pain4.wav
		sound/player/pain5.wav
		sound/player/pain6.wav
		sound/player/teledth1.wav

		sound/weapons/lock4.wav
		sound/weapons/pkup.wav
		sound/weapons/lstart.wav

		sound/weapons/sgun1.wav
		sound/weapons/r_exp3.wav

*/

//		sprintf(wave.file, "sound/player/%s/jump1.wav", models[i]);
		sprintf(wave.file, "sound/player/plyrjmp8.wav");
		audio.load(wave, engine->pk3_list, engine->num_pk3);
		snd_wave.push_back(wave);
		snd_table[SND_PLAYER + i * 11 + 7] = snd_wave.size() - 1;


		sprintf(wave.file, "sound/player/%s/fall1.wav", EntPlayer::models[i]);
		audio.load(wave, engine->pk3_list, engine->num_pk3);
		snd_wave.push_back(wave);
		snd_table[SND_PLAYER + i * 11 + 8] = snd_wave.size() - 1;

		sprintf(wave.file, "sound/player/%s/falling1.wav", EntPlayer::models[i]);
		audio.load(wave, engine->pk3_list, engine->num_pk3);
		snd_wave.push_back(wave);
		snd_table[SND_PLAYER + i * 11 + 9] = snd_wave.size() - 1;
	}


	strcpy(wave.file, "sound/weapons/railgun/rg_hum.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_RG_HUM 230
	snd_table[SND_RG_HUM] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/weapons/lightning/lg_hum.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_LG_HUM 231
	snd_table[SND_LG_HUM] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/weapons/shotgun/sshotf1b.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_SHOTGUN 232
	snd_table[SND_SHOTGUN] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/weapons/railgun/railgf1a.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_RAILGUN 233
	snd_table[SND_RAILGUN] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/weapons/lightning/lg_fire.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_LIGHTNING 234
	snd_table[SND_LIGHTNING] = snd_wave.size() - 1;

#ifdef G_COMMANDO
	strcpy(wave.file, "sound/weapons/knife/knife_slash1.wav");
	audio.load(wave);
	if (wave.data != NULL)
		snd_wave.push_back(wave);
#endif


	strcpy(wave.file, "sound/weapons/rocket/rocklf1a.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_ROCKET 235
	snd_table[SND_ROCKET] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/weapons/grenade/grenlf1a.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_GRENADE 236
	snd_table[SND_ROCKET] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/weapons/plasma/hyprbf1a.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_PLASMA 237
	snd_table[SND_PLASMA] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/weapons/machinegun/machgf1b.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_MACHINEGUN 238
	snd_table[SND_MACHINEGUN] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/world/telein.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_TELEIN 239
	snd_table[SND_TELEIN] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/world/teleout.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_TELEOUT 240
	snd_table[SND_TELEOUT] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/teamplay/flagcap_blu.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
	#define SND_FLAGCAP 241
	snd_table[SND_FLAGCAP] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/teamplay/flagtk_blu.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
	#define SND_FLAGTAKE 242
	snd_table[SND_FLAGTAKE] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/teamplay/flagret_blu.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
	#define SND_FLAGRET 243
	snd_table[SND_FLAGRET] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/weapons/grenade/hgrenb1a.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
	#define SND_GRENADE_IMPACT 244
	snd_table[SND_GRENADE_IMPACT] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/weapons/rocket/rocklx1a.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_EXPLODE 245
	snd_table[SND_EXPLODE] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/weapons/rocket/rockfly.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_ROCKETFLY 246
	snd_table[SND_ROCKETFLY] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/weapons/plasma/lasfly.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_PLASMAFLY 247
	snd_table[SND_PLASMAFLY] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/weapons/plasma/plasmx1a.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_PLASMA_EXPLODE 248
	snd_table[SND_PLASMA_EXPLODE] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/weapons/change.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_SWAPWEAP 249
	snd_table[SND_SWAPWEAP] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/items/use_medkit.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_USE_MED 250
	snd_table[SND_USE_MED] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/items/use_nothing.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_USE_EMPTY 251
	snd_table[SND_USE_EMPTY] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/items/regen.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_REGEN 252
	snd_table[SND_REGEN] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/player/footsteps/step1.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_STEP1 253
	snd_table[SND_STEP1] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/player/footsteps/step2.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_STEP2 254
	snd_table[SND_STEP2] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/player/footsteps/step3.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_STEP3 255
	snd_table[SND_STEP3] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/player/footsteps/step4.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_STEP4 256
	snd_table[SND_STEP4] = snd_wave.size() - 1;


	sprintf(wave.file, "sound/player/land1.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_LAND 257
	snd_table[SND_LAND] = snd_wave.size() - 1;


	sprintf(wave.file, "sound/world/jumppad.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_JUMPPAD 258
	snd_table[SND_JUMPPAD] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/weapons/noammo.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_NOAMMO 259
	snd_table[SND_NOAMMO] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/player/watr_in.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_WATER_IN 260
	snd_table[SND_WATER_IN] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/player/watr_out.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_WATER_OUT 261
	snd_table[SND_WATER_OUT] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/player/gurp1.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_GURP1 262
	snd_table[SND_GURP1] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/player/gurp2.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_GURP2 263
	snd_table[SND_GURP2] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/player/gibsplt1.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_GIBBED 264
	snd_table[SND_GIBBED] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/player/gibimp1.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_GIB1 265
	snd_table[SND_GIB1] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/player/gibimp2.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_GIB2 266
	snd_table[SND_GIB2] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/player/gibimp3.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_GIB3 267
	snd_table[SND_GIB3] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/player/talk.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
	#define SND_TALK 268
	snd_table[SND_TALK] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/feedback/blueleads.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_BLUE_LEAD 269
	snd_table[SND_BLUE_LEAD] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/feedback/redleads.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_RED_LEAD 270
	snd_table[SND_RED_LEAD] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/feedback/tiedlead.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_TIED_LEAD 271
	snd_table[SND_TIED_LEAD] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/feedback/lostlead.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_LOST_LEAD 272
	snd_table[SND_LOST_LEAD] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/feedback/takenlead.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_TAKEN_LEAD 273
	snd_table[SND_TAKEN_LEAD] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/teamstied.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_TEAMS_TIED 274
	snd_table[SND_TEAMS_TIED] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/excellent.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_EXCELLENT 275
	snd_table[SND_EXCELLENT] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/feedback/gauntlet.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_GAUNTLET_MEDAL 276
	snd_table[SND_GAUNTLET_MEDAL] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/humiliation.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_HUMILIATION 277
	snd_table[SND_HUMILIATION] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/impressive_a.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_IMPRESSIVEA 278
	snd_table[SND_IMPRESSIVEA] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/accuracy.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_ACCURACY 279
	snd_table[SND_ACCURACY] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/excellent_a.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_EXCELLENTA 280
	snd_table[SND_EXCELLENTA] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/frags.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_FRAGS 281
	snd_table[SND_FRAGS] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/impressive.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_IMPRESSIVE 282
	snd_table[SND_IMPRESSIVE] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/feedback/perfect.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_PERFECT 283
	snd_table[SND_PERFECT] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/hit.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_HIT 284
	snd_table[SND_HIT] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/hit_teammate.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_HITTEAM 285
	snd_table[SND_HITTEAM] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/sudden_death.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_SUDDEN 286
	snd_table[SND_SUDDEN] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/one.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_ONE 287
	snd_table[SND_ONE] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/two.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_TWO 288
	snd_table[SND_TWO] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/three.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_THREE 289
	snd_table[SND_THREE] = snd_wave.size() - 1;



	strcpy(wave.file, "sound/feedback/denied.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_DENIED 290
	snd_table[SND_DENIED] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/fight.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_FIGHT 291
	snd_table[SND_FIGHT] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/feedback/prepare.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_PREPARE 292
	snd_table[SND_PREPARE] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/feedback/1_minute.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_ONE_MINUTE 293
	snd_table[SND_ONE_MINUTE] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/feedback/5_minute.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_FIVE_MINUTE 294
	snd_table[SND_FIVE_MINUTE] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/1_frag.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_ONE_FRAG 295
	snd_table[SND_ONE_FRAG] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/2_frags.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_TWO_FRAG 296
	snd_table[SND_TWO_FRAG] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/3_frags.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_THREE_FRAG 297
	snd_table[SND_THREE_FRAG] = snd_wave.size() - 1;

	strcpy(wave.file, "media/sound/shell_quiet.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_SHELL 298
	snd_table[SND_SHELL] = snd_wave.size() - 1;

	strcpy(wave.file, "media/sound/bullet_quiet.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_BULLET 299
	snd_table[SND_BULLET] = snd_wave.size() - 1;

	strcpy(wave.file, "media/sound/dropgun_quiet.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_DROPGUN 300
	snd_table[SND_DROPGUN] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/soldier/sight1.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_SOLDIER_SIGHT 301
	snd_table[SND_SOLDIER_SIGHT] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/soldier/idle.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_SOLDIER_IDLE 302
	snd_table[SND_SOLDIER_IDLE] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/weapons/melee/fstrun.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_GAUNTLET 303
	snd_table[SND_GAUNTLET] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/weapons/melee/fsthum.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_GAUNTLET_IDLE 304
	snd_table[SND_GAUNTLET_IDLE] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/weapons/melee/fstatck.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_GAUNTLET_HIT 305
	snd_table[SND_GAUNTLET_HIT] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/movers/doors/dr1_strt.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_DOOR_START 306
	snd_table[SND_DOOR_START] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/movers/doors/dr1_end.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_DOOR_END 307
	snd_table[SND_DOOR_END] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/movers/switches/butn2.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_BUTTON 308
	snd_table[SND_BUTTON] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/player/footsteps/clank1.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_METAL_STEP1 309
	snd_table[SND_METAL_STEP1] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/player/footsteps/clank2.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_METAL_STEP2 310
	snd_table[SND_METAL_STEP2] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/player/footsteps/clank3.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_METAL_STEP3 311
	snd_table[SND_METAL_STEP3] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/player/footsteps/clank4.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_METAL_STEP4 312
	snd_table[SND_METAL_STEP4] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/player/footsteps/flesh1.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_FLESH_STEP1 313
	snd_table[SND_FLESH_STEP1] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/player/footsteps/flesh2.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_FLESH_STEP2 314
	snd_table[SND_FLESH_STEP2] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/player/footsteps/flesh3.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_FLESH_STEP3 315
	snd_table[SND_FLESH_STEP3] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/player/footsteps/flesh4.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_FLESH_STEP4 316
	snd_table[SND_FLESH_STEP4] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/player/footsteps/splash1.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_SPLASH_STEP1 317
	snd_table[SND_SPLASH_STEP1] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/player/footsteps/splash2.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_SPLASH_STEP2 318
	snd_table[SND_SPLASH_STEP2] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/player/footsteps/splash3.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_SPLASH_STEP3 319
	snd_table[SND_SPLASH_STEP3] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/player/footsteps/splash4.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_SPLASH_STEP4 320
	snd_table[SND_SPLASH_STEP4] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/player/fry.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_FRY 321
	snd_table[SND_FRY] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/weapons/q3f_sentry_build.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_SENTRY_BUILD 322
	snd_table[SND_SENTRY_BUILD] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/weapons/q3f_sentry_upgrade.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_SENTRY_UPGRADE 323
	snd_table[SND_SENTRY_UPGRADE] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/weapons/q3f_sentry_prepstart.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_SENTRY_PREPSTART 324
	snd_table[SND_SENTRY_PREPSTART] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/weapons/q3f_sentry_prepend.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_SENTRY_PREPEND 325
	snd_table[SND_SENTRY_PREPEND] = snd_wave.size() - 1;


	strcpy(wave.file, "media/sound/constru2.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_CONSTRUCT 326
	snd_table[SND_CONSTRUCT] = snd_wave.size() - 1;

	strcpy(wave.file, "media/sound/country4.wav");
	audio.load(wave, engine->pk3_list, engine->num_pk3);
	snd_wave.push_back(wave);
#define SND_CONSTRUCT_COMPLETE 327
	snd_table[SND_CONSTRUCT_COMPLETE] = snd_wave.size() - 1;

	


}


///=============================================================================
/// Function: get_team
///=============================================================================
/// Description: Selects a team for a new person joining a game based on existing
/// team sizes
///
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
team_t Quake3::get_team()
{
	if (gametype == GAMETYPE_CTF)
	{
		if (num_player_red >= num_player_blue)
		{
			return TEAM_BLUE;
		}
		else
		{
			return TEAM_RED;
		}
	}
	else
	{
		return TEAM_NONE;
	}
}

///=============================================================================
/// Function: add_player
///=============================================================================
/// Description: Adds a player to the map
///
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
void Quake3::add_player(vector<Entity *> &entity_list, playertype_t player_type, int &ent_id, char *player_name)
{
	int spawn_type = -1;
	bool local = false;
	team_t team = get_team();


	if (team == TEAM_RED)
	{
		spawn_type = ENT_TEAM_CTF_REDSPAWN;
		num_player_red++;

		if (local)
		{
			sprintf(engine->menu.data.team, "Red");
		}
	}
	else if (team == TEAM_BLUE)
	{
		spawn_type = ENT_TEAM_CTF_BLUESPAWN;
		num_player_blue++;

		if (local)
		{
			sprintf(engine->menu.data.team, "Blue");
		}
	}
	else if (team == TEAM_NONE)
	{
		spawn_type = ENT_INFO_PLAYER_DEATHMATCH;
		if (local)
		{
			sprintf(engine->menu.data.team, "None");
		}
	}
	else
	{
		spawn_type = ENT_UNKNOWN;

		if (local)
		{
			sprintf(engine->menu.data.team, "Unknown");
		}
	}
	num_player++;

	for (unsigned int i = engine->max_dynamic; i < entity_list.size(); i++)
	{		
		if (spawn_type == entity_list[i]->ent_type //||
			/*strcmp(type, "info_player_start") == 0*/)
		{
			if ((unsigned int)last_spawn == i + 1)
				continue;

			if (player_type == PLAYER ||
				player_type == SERVER)
			{
				// Only set render view position for local clients
				engine->camera_frame.pos = entity_list[i]->position;
				local = true;
			}

			int spawn = engine->get_player();
			printf("player is entity %d\r\n", spawn);
			ent_id = spawn;

			switch (player_type)
			{
			case PLAYER:
				entity_list[spawn]->ent_type = ENT_PLAYER;
				break;
			case SERVER:
				entity_list[spawn]->ent_type = ENT_SERVER;
				break;
			case CLIENT:
				entity_list[spawn]->ent_type = ENT_CLIENT;
				break;
			case BOT:
				entity_list[spawn]->ent_type = ENT_NPC;
				break;
			case SPECTATOR:
				entity_list[spawn]->ent_type = ENT_SPECTATOR;
				break;
			case UNKNOWN:
				entity_list[spawn]->ent_type = ENT_UNKNOWN;
				break;
			}
			Entity *spawn_ent = entity_list[spawn];

			spawn_ent->position = entity_list[i]->position;
			spawn_ent->rigid = new EntRigidBody(entity_list[spawn]);
			spawn_ent->model = entity_list[spawn]->rigid;
			spawn_ent->rigid->clone(*(engine->thug22->model));
			spawn_ent->rigid->flags.step_flag = true;
			spawn_ent->player = new EntPlayer(entity_list[spawn], engine->gfx, engine->audio, 21, team, (entity_type_t)entity_list[spawn]->ent_type, model_table);
			spawn_ent->position += entity_list[spawn]->rigid->center;
			spawn_ent->position += vec3(0.0f, 20.0f, 0.0f); //adding some height
			spawn_ent->player->type = player_type;

			if (player_type == BOT && engine->demo == false)
			{
				spawn_ent->player->model_index = 20;
			}

			strcpy(spawn_ent->player->name, player_name);


			spawn_ent->player->local = local;
			if (player_type == BOT)
				spawn_ent->player->local = false;


			matrix4 matrix;

			if (entity_list[i]->brushinfo)
			{
				//set spawn angle
				switch (entity_list[i]->brushinfo->angle)
				{
				case 0:
				case 45:
				case 360:
				case 325:
					matrix4::mat_left(matrix, spawn_ent->position);
					break;
				case 90:
				case 135:
					matrix4::mat_forward(matrix, spawn_ent->position);
					break;
				case 180:
				case 225:
					matrix4::mat_right(matrix, spawn_ent->position);
					break;
				case 270:
					matrix4::mat_backward(matrix, spawn_ent->position);
					break;
				default:
					matrix4::mat_left(matrix, spawn_ent->position);
					break;
				}
			}
			else
			{
				matrix4::mat_left(matrix, spawn_ent->position);
			}

			if (local)
			{
				engine->camera_frame.forward.x = matrix.m[8];
				engine->camera_frame.forward.y = matrix.m[9];
				engine->camera_frame.forward.z = matrix.m[10];
				engine->camera_frame.up = vec3(0.0f, 1.0f, 0.0f);
			}
			last_spawn = i + 1;
			break;
		}
	}

	if (gametype == GAMETYPE_DEATHMATCH && player_type == BOT)
	{
		char cmd[64];

		// Hack to maintain same spawn behavior for bots
		sprintf(cmd, "respawn %d %d", -1, ent_id);
		console(ent_id, cmd, engine->menu, engine->entity_list);
	}

	if (local)
	{
		engine->audio.listener_position((float *)&(engine->camera_frame.pos));
	}
}


///=============================================================================
/// Function: handle_player
///=============================================================================
/// Description: Handles player input
///
///
/// Parameters:
///		self - entity index for self / player
///
/// Returns:
///		None
///=============================================================================
void Quake3::handle_player(int self, input_t &input)
{
	Entity *entity = engine->entity_list[self];
	static int footstep_num = 0;
	static bool zoomed = false;
#if 0
	static int last_tick = 0;
#endif

#if 0
	// kills frames, yes clients get here ;)
	if (self != 0)
	{
		printf("client movement: up %d down %d left %d right %d\r\n", input.moveup, input.movedown, input.moveleft, input.moveright);
	}
#endif

	if (entity->player == NULL || entity->rigid == NULL)
		return;


	if (entity->player->local && 0)
	{
		quaternion q;
		vec3 temp;

		q.to_quat(entity->rigid->morientation);
		if (q.s < 0)
		{
			temp.x = -q.x;
			temp.y = -q.y;
			temp.z = -q.z;
		}
		else
		{
			temp.x = q.x;
			temp.y = q.y;
			temp.z = q.z;
		}


		q.x = temp.x;
		q.y = temp.y;
		q.z = temp.z;
		q.compute_w();
		q.s = -q.s;

		matrix3 compare = q.to_matrix();

		float error = 0.0f;
		for (int i = 0; i < 9; i++)
		{
			error += compare.m[i] - entity->rigid->morientation.m[i];
		}

		debugf("Error: %f\n", error);

	}


	// make all plasma balls from plasma gun follow path around the player position
	if (0)
	{
		vec3 path_list[4];
		Entity *ref = entity;

		path_list[0] = ref->position + vec3(100.0f, 0.0f, -100.0f);
		path_list[1] = ref->position + vec3(-100.0f, 0.0f, -100.0f);
		path_list[2] = ref->position + vec3(-100.0f, 0.0f, 100.0f);
		path_list[3] = ref->position + vec3(100.0f, 0.0f, 100.0f);

		Entity *projectile = NULL;
		for (unsigned int i = 0; i < engine->max_dynamic; i++)
		{
			if (engine->entity_list[i]->trigger && engine->entity_list[i]->projectile->explode_type == 2)
			{
				projectile = engine->entity_list[i];
				projectile->rigid->pid_follow_path(path_list, 4, 2.0f, 75.0f, 1);
			}
		}
	}


	if (input.duck && entity->rigid->y_offset != -25)
	{
		entity->rigid->y_offset -= 2;
		if (entity->rigid->y_offset < -25)
			entity->rigid->y_offset = -25;
	}


	if (input.duck == false && entity->rigid->y_offset != 0)
	{
		entity->rigid->y_offset += 4;
		if (entity->rigid->y_offset > 0)
			entity->rigid->y_offset = 0;
	}



	if (input.zoom == true && zoomed == false)
	{
		zoomed = true;
		if (spectator)
			engine->zoom(4.0);
		else
			engine->zoom(entity->player->zoom_level);
	}

	if (input.zoom == false && zoomed == true)
	{
		zoomed = false;
		engine->zoom(1.0);
	}

	if (spectator)
		return;


	if (entity->player->pain_timer == 0)
	{
		if (entity->rigid->flags.lava)
		{
			console(self, "damage 20", engine->menu, engine->entity_list);

			if (entity->player->local)
				engine->play_wave_global(SND_FRY);
			else
				engine->play_wave(entity->position, SND_FRY);


			entity->player->pain_timer = (TICK_RATE >> 1);
			entity->rigid->flags.lava = false;

			if (entity->player->health <= 0 && entity->player->state != PLAYER_DEAD)
			{
				char msg[256];

				entity->player->stats.deaths++;

				//  "Player" visits the Volcano God
				//	"Player" turned into hot slag
				//	"Player" burst into flames

				sprintf(msg, "%s turned into hot slag\n", entity->player->name);
				debugf(msg);
				engine->menu.print_notif(msg);
				notif_timer = 3 * TICK_RATE;
			}
		}
		else if (entity->rigid->flags.slime)
		{
			console(self, "damage 10", engine->menu, engine->entity_list);
			entity->player->pain_timer = (TICK_RATE >> 1);
			entity->rigid->flags.slime = false;

			if (entity->player->health <= 0 && entity->player->state != PLAYER_DEAD)
			{
				char msg[256];

				entity->player->stats.deaths++;
				sprintf(msg, "%s took an acid bath\n", entity->player->name);
				debugf(msg);
				engine->menu.print_notif(msg);
				notif_timer = 3 * TICK_RATE;
			}
		}
	}
	

	if (entity->player->telefragged && 0)
	{
		char msg[512];
		Entity *entity2 = entity->player->telefragged;
		entity->player->telefragged = NULL;


		if (round_time > 1)
		{
			entity->player->health = -100;
			entity->player->stats.deaths++;

			if (entity2->player)
			{
				entity2->player->stats.kills++;
				sprintf(msg, "%s telefragged %s\n", entity2->player->name, entity->player->name);
				handle_frags_left(*(entity2->player));
			}
			else
			{
				sprintf(msg, "%s was in the wrong place at the wrong time\n", entity->player->name);
				entity->player->telefragged = NULL;
			}

			debugf(msg);
			engine->menu.print_notif(msg);
			notif_timer = 3 * TICK_RATE;
		}
		else
		{
			console(self, "respawn", engine->menu, engine->entity_list);
		}
	}

	if (entity->player->excellent_timer > 0)
		entity->player->excellent_timer--;

	if (entity->player->pain_timer > 0)
		entity->player->pain_timer--;

	if (entity->rigid->flags.hard_impact)
	{
		if (entity->rigid->impact_velocity < -FALL_DAMAGE_VELOCITY)
		{
			entity->rigid->flags.hard_impact = false;

			if (entity->player->fall_timer == 0)
			{
				entity->player->fall_timer = TICK_RATE >> 4;
				console(self, "dmg 10", engine->menu, engine->entity_list);
				if (entity->player->health <= 0 && entity->player->state != PLAYER_DEAD)
				{
					char msg[256];

					entity->player->stats.deaths++;
					sprintf(msg, "%s fell and cant get up\n", entity->player->name);
					debugf(msg);
					engine->menu.print_notif(msg);
					notif_timer = 3 * TICK_RATE;
				}

				if (entity->player->local)
					engine->play_wave_global(entity->player->model_index * SND_PLAYER + SND_FALL);
				else
					engine->play_wave(entity->position, entity->player->model_index * SND_PLAYER + SND_FALL);
			}
		}
		else if (entity->rigid->impact_velocity < -IMPACT_VELOCITY)
		{
			entity->rigid->flags.hard_impact = false;


			if (entity->player->fall_timer == 0)
			{
				entity->player->fall_timer = TICK_RATE >> 4;

				if (entity->player->local)
					engine->play_wave_global(SND_LAND);
				else
					engine->play_wave(entity->position, SND_LAND);
			}
		}
	}

	if (input.use == true)
	{
		bool click = true;
		if (entity->player->holdable_medikit)
		{
			entity->player->health = 125;
			entity->player->holdable_medikit = false;
			if (entity->player->local)
				engine->play_wave_global(SND_USE_MED);
			else
				engine->play_wave(entity->position, SND_USE_MED);

			click = false;
		}
		if (entity->player->holdable_teleporter)
		{
			console(self, "teleport", engine->menu, engine->entity_list);
			entity->player->holdable_teleporter = false;
			click = false;
		}

		static unsigned int last_check = 0;


		// Check for vehicles and enter them if near by
		if (last_check + TICK_RATE < engine->tick_num)
		{
			if (entity->player->in_vehicle == -1)
			{
				for (unsigned int i = 0; i < engine->entity_list.size(); i++)
				{
					if (engine->entity_list[i]->vehicle)
					{
						vec3 dist = engine->entity_list[i]->position - entity->position;
						if (dist.magnitude() < 200.0f)
						{
							engine->play_wave_global(SND_SENTRY_UPGRADE);
							entity->player->in_vehicle = i;
							click = false;
							last_check = engine->tick_num;
							break;
						}
					}
				}
			}
			else
			{
				engine->play_wave_global(entity->player->model_index * SND_PLAYER + SND_JUMP);
				entity->player->in_vehicle = -1;
			}
		}


		if (click)
		{
			//play click sound
			if (entity->player->click_timer == 0)
			{
				entity->player->click_timer = (int)(0.5f * TICK_RATE);
				if (entity->player->local)
					engine->play_wave_global(SND_USE_EMPTY);
				else
					engine->play_wave(entity->position, SND_USE_EMPTY);
			}
			else
			{
				entity->player->click_timer--;
			}
		}
	}


#if 0
	if (input.use == true)
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


	if (input.numpad7 == true)
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

	if (input.pickup)
	{
		int item = -1;
		float min_distance = FLT_MAX;

		for(unsigned int i = engine->num_player; i < engine->entity_list.size(); i++)
		{
			if (engine->entity_list[i]->rigid == NULL)
				continue;

			if (engine->entity_list[i]->visible == false)
				continue;

			// Dont grab things like trigger_teleport :)
			if (strstr(engine->entity_list[i]->type, "item_") == NULL &&
				strstr(engine->entity_list[i]->type, "weapon_") == NULL)
				continue;

			float distance = (engine->camera_frame.pos - engine->entity_list[i]->position).magnitude();
			if (distance < min_distance)
			{
				min_distance = distance;
				item = i;
			}
		}

		if (item != -1)
		{
			printf("Grabbing %s\n", engine->entity_list[item]->type);
			engine->entity_list[item]->rigid->seek(engine->camera_frame.pos);
		}
		else
		{
			printf("Couldnt find an item to grab\n");
		}
	}
#endif

	if (entity->player->health > 0)
	{
		entity->player->alive_timer++;

		if (entity->player->state == PLAYER_DEAD)
		{
			// network clients can get stuck as walking body
			entity->player->state = PLAYER_IDLE;
			entity->rigid->clone(*(engine->thug22->model));
		}

		if (spectator == false || engine->menu.console == false)
		{
			// True if jumped
			if (input.moveup || input.movedown || input.moveleft || input.moveright)
			{
				entity->player->state = PLAYER_MOVED;
			}

			if (input.duck)
			{
				entity->player->state = PLAYER_DUCKED;
			}

			if ((entity->player && entity->player->type == PLAYER) || (engine->netcode.server_flag && (entity->player && entity->player->type == CLIENT)))
			{
				float speed_scale = 1.0f;

				if (entity->player->haste_timer > 0)
					speed_scale = entity->player->haste_factor;


				if (engine->menu.console == false)
				{
					if (entity->player->in_vehicle == -1)
					{

						if (entity->rigid->move(input, speed_scale)) // player and client movement code
						{
							entity->player->state = PLAYER_JUMPED;
							if (entity->player->local)
								engine->play_wave_global(entity->player->model_index * SND_PLAYER + SND_JUMP);
							else
								engine->play_wave(entity->position, entity->player->model_index * SND_PLAYER + SND_JUMP);

						}
					}
					else
					{
						EntVehicle *vehicle = engine->entity_list[entity->player->in_vehicle]->vehicle;
						Frame car_frame;

						float x = cos(-vehicle->angle_rad);
						float y = sin(-vehicle->angle_rad);

						car_frame.forward.x = y;
						car_frame.forward.y = 0;
						car_frame.forward.z = -x;
						car_frame.forward.normalize();

						car_frame.up = vec3(0.0f, 1.0f, 0.0f);

						matrix3 mat;
						car_frame.set(mat);
						engine->entity_list[entity->player->in_vehicle]->rigid->morientation = mat;
						entity->rigid->morientation = mat;
						engine->camera_frame = car_frame;
						engine->entity_list[entity->player->in_vehicle]->position.y = 0.0f;


						vec3 viewpos = mat * vehicle->info.seat[entity->player->seat];

						entity->position = vehicle->entity->position + viewpos;
						engine->entity_list[entity->player->in_vehicle]->vehicle->move(input, speed_scale, engine->tick_num);
					}
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

		if (entity->player && entity->player->type == PLAYER)
		{
			if (input.attack && entity->player->reload_timer == 0)
			{
				console(self, "respawn", engine->menu, engine->entity_list);
			}
		}
		else
		{
			if (entity->player->reload_timer <= 0)
			{
				if (engine->netcode.client_flag == false)
				{
					char cmd[128];

					sprintf(cmd, "respawn %d %d", -1, self);
					console(self, cmd, engine->menu, engine->entity_list);
				}
			}
		}
	}

	if (entity->player->jumppad_timer > 0)
	{
		entity->player->jumppad_timer--;
	}

	if (entity->player->teleport_timer > 0)
	{
		entity->player->teleport_timer--;
	}

	if (entity->player->fall_timer > 0)
	{
		entity->player->fall_timer--;
	}

	if (entity->player->quad_timer > 0)
	{
		if (entity->light == NULL)
		{
			entity->player->quad_damage = true;
			entity->light = new EntLight(entity, engine->gfx, 999, engine->res_scale);
			entity->light->color = vec3(0.0f, 0.0f, 1.0f);
			entity->light->intensity = 4000.0f;
			entity->light->attenuation = 0.125f;
		}
		entity->player->quad_timer--;
	}
	else
	{
		// Light list wont be updated until the next step, so manually delete
		if (entity->player->quad_damage && entity->light)
		{
			entity->player->quad_damage = false;
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
		entity->model->flags.blend = true;
	}
	else
	{
		entity->model->flags.blend = false;
	}

	if (entity->player->flight_timer > 0)
	{
		entity->player->flight_timer--;
		entity->rigid->flags.flight = true;
		entity->rigid->translational_friction = 0.9f;
	}
	else
	{
		entity->rigid->flags.flight = false;
		entity->rigid->translational_friction = 0.0f;
	}

	// Stuff that updates every second
	if (engine->tick_num % TICK_RATE == 0)
	{
		if (entity->player->regen_timer > 0 && entity->player->state != PLAYER_DEAD)
		{
			if (entity->player->health < 200)
			{
				entity->player->health += 15;
				if (entity->player->local)
					engine->play_wave_global(SND_REGEN);
				else
					engine->play_wave(entity->position, SND_REGEN);

			}

			if (entity->player->health > 200)
			{
				entity->player->health = 200;
			}

		}
		else
		{
			if (entity->player->health > 100)
			{
				entity->player->health--;
			}
		}

		if (entity->player->armor > 100)
		{
			entity->player->armor--;
		}
	}

	if (input.walk == false && entity->rigid->flags.on_ground && entity->rigid->flags.gravity == true &&
		(entity->rigid->flags.water == false && entity->rigid->water_depth > 2047.0f) &&
		entity->player->state != PLAYER_DEAD &&
		entity->rigid->flags.noclip == false &&
		entity->rigid->flags.flight == false)
	{
		// Footstep sounds
		if ((entity->position - entity->rigid->old_position).magnitude() > 0.8f && engine->tick_num % 20 == 0)
		{
			switch (footstep_num++ % 4)
			{
			case 0:
				if (entity->rigid->flags.water && entity->rigid->water_depth < 10.0f)
				{
					if (entity->player->local)
						engine->play_wave_global(SND_SPLASH_STEP1);
					else
						engine->play_wave(entity->position, SND_SPLASH_STEP1);
				}
				else if (entity->rigid->step_type == SURF_METALSTEPS)
				{
					if (entity->player->local)
						engine->play_wave_global(SND_METAL_STEP1);
					else
						engine->play_wave(entity->position, SND_METAL_STEP1);
				}
				else if (entity->rigid->step_type == SURF_FLESH)
				{
					if (entity->player->local)
						engine->play_wave_global(SND_FLESH_STEP1);
					else
						engine->play_wave(entity->position, SND_FLESH_STEP1);
				}
				else
				{
					if (entity->player->local)
						engine->play_wave_global(SND_STEP1);
					else
						engine->play_wave(entity->position, SND_STEP1);
				}
				break;
			case 1:
				if (entity->rigid->flags.water && entity->rigid->water_depth < 10.0f)
				{
					if (entity->player->local)
						engine->play_wave_global(SND_SPLASH_STEP2);
					else
						engine->play_wave(entity->position, SND_SPLASH_STEP2);
				}
				else if (entity->rigid->step_type == SURF_METALSTEPS)
				{
					if (entity->player->local)
						engine->play_wave_global(SND_METAL_STEP2);
					else
						engine->play_wave(entity->position, SND_METAL_STEP2);
				}
				else if (entity->rigid->step_type == SURF_FLESH)
				{
					if (entity->player->local)
						engine->play_wave_global(SND_FLESH_STEP2);
					else
						engine->play_wave(entity->position, SND_FLESH_STEP2);
				}
				else
				{
					if (entity->player->local)
						engine->play_wave_global(SND_STEP2);
					else
						engine->play_wave(entity->position, SND_STEP2);
				}
				break;
			case 2:
				if (entity->rigid->flags.water && entity->rigid->water_depth < 10.0f)
				{
					if (entity->player->local)
						engine->play_wave_global(SND_SPLASH_STEP3);
					else
						engine->play_wave(entity->position, SND_SPLASH_STEP3);
				}
				else if (entity->rigid->step_type == SURF_METALSTEPS)
				{
					if (entity->player->local)
						engine->play_wave_global(SND_METAL_STEP3);
					else
						engine->play_wave(entity->position, SND_METAL_STEP3);
				}
				else if (entity->rigid->step_type == SURF_FLESH)
				{
					if (entity->player->local)
						engine->play_wave_global(SND_FLESH_STEP3);
					else
						engine->play_wave(entity->position, SND_FLESH_STEP3);
				}
				else
				{
					if (entity->player->local)
						engine->play_wave_global(SND_STEP3);
					else
						engine->play_wave(entity->position, SND_STEP3);
				}
				break;
			case 3:
				if (entity->rigid->flags.water && entity->rigid->water_depth < 10.0f)
				{
					if (entity->player->local)
						engine->play_wave_global(SND_SPLASH_STEP4);
					else
						engine->play_wave(entity->position, SND_SPLASH_STEP4);
				}
				else if (entity->rigid->step_type == SURF_METALSTEPS)
				{
					if (entity->player->local)
						engine->play_wave_global(SND_METAL_STEP4);
					else
						engine->play_wave(entity->position, SND_METAL_STEP4);
				}
				else if (entity->rigid->step_type == SURF_FLESH)
				{
					if (entity->player->local)
						engine->play_wave_global(SND_FLESH_STEP4);
					else
						engine->play_wave(entity->position, SND_FLESH_STEP4);
				}
				else
				{
					if (entity->player->local)
						engine->play_wave_global(SND_STEP4);
					else
						engine->play_wave(entity->position, SND_STEP4);
				}
				break;
			}
		}
	}

	//Water sounds
	if (entity->rigid->flags.water && entity->rigid->water_depth < entity->rigid->get_height())
	{
		if (entity->rigid->flags.water != entity->rigid->flags.last_water)
		{
			if (entity->player->local)
				engine->play_wave_global(SND_WATER_IN);
			else
				engine->play_wave(entity->position, SND_WATER_IN);

			entity->rigid->flags.last_water = entity->rigid->flags.water;
		}
	}
	else if (entity->rigid->water_depth > 2047.0f)
	{
		if (entity->rigid->flags.water != entity->rigid->flags.last_water)
		{
			if (entity->player->local)
				engine->play_wave_global(SND_WATER_OUT);
			else
				engine->play_wave(entity->position, SND_WATER_OUT);
			entity->rigid->flags.last_water = entity->rigid->flags.water;
			entity->player->drown_timer = 0;
		}
	}
	else
	{
		if (entity->player->state != PLAYER_DEAD)
		{
			entity->player->drown_timer++;

			if (entity->player->drown_timer % 125 * 30 == 0)
			{
				switch (footstep_num++ % 2)
				{
				case 0:
					if (entity->player->local)
						engine->play_wave_global(SND_GURP1);
					else
						engine->play_wave(entity->position, SND_GURP1);
					break;
				case 1:
					if (entity->player->local)
						engine->play_wave_global(SND_GURP2);
					else
						engine->play_wave(entity->position, SND_GURP2);
					break;
				}

				console(self, "damage 15", engine->menu, engine->entity_list);
				if (entity->player->health < 0 && entity->player->state != PLAYER_DEAD)
				{
					char msg[256];

					entity->player->stats.deaths++;
					entity->player->kill();
					sprintf(msg, "%s swallowed too much water\n", entity->player->name);
					debugf(msg);
					engine->menu.print_notif(msg);
					notif_timer = 3 * TICK_RATE;
				}
			}
		}
	}

	handle_weapons(*(entity->player), input, self, engine->netcode.client_flag);
}

void Quake3::player_died(int index)
{
	Entity *entity = engine->entity_list[index];

	if (entity->player->health <= -50)
	{
		if (entity->player->local)
			engine->play_wave_global(SND_GIBBED);
		else
			engine->play_wave(entity->position, SND_GIBBED);
		handle_gibs(*(entity->player));
	}
	else
	{
		switch (engine->tick_num % 3)
		{
		case 0:
			if (entity->player->local)
				engine->play_wave_global(entity->player->model_index * SND_PLAYER + SND_DEATH1);
			else
				engine->play_wave(entity->position, entity->player->model_index * SND_PLAYER + SND_DEATH1);
			break;
		case 1:
			if (entity->player->local)
				engine->play_wave_global(entity->player->model_index * SND_PLAYER + SND_DEATH2);
			else
				engine->play_wave(entity->position, entity->player->model_index * SND_PLAYER + SND_DEATH2);
			break;
		case 2:
			if (entity->player->local)
				engine->play_wave_global(entity->player->model_index * SND_PLAYER + SND_DEATH3);
			else
				engine->play_wave(entity->position, entity->player->model_index * SND_PLAYER + SND_DEATH3);
			break;
		}
	}

	drop_weapon(index);

	if (entity->player->quad_timer > 0)
		drop_powerup(entity->position, "media/models/powerups/instant/quad", "quaddamage");
	if (entity->player->regen_timer > 0)
		drop_powerup(entity->position, "media/models/powerups/instant/regen", "regeneration");
	if (entity->player->haste_timer > 0)
		drop_powerup(entity->position, "media/models/powerups/instant/haste", "haste");
	if (entity->player->invisibility_timer > 0)
		drop_powerup(entity->position, "media/models/powerups/instant/invis", "invisibility");
	if (entity->player->flight_timer > 0)
		drop_powerup(entity->position, "media/models/powerups/instant/flight", "flight");
	if (entity->player->holdable_flag)
	{
		if (entity->player->team == TEAM_BLUE)
			drop_powerup(entity->position, "media/models/flags/r_flag", "red_flag");
		else if (entity->player->team == TEAM_RED)
			drop_powerup(entity->position, "media/models/flags/b_flag", "blue_flag");
	}

	entity->player->kill();
	if (entity->player->immobile == false)
	{
		entity->model->clone(model_table[MODEL_BOX]);
	}
}

///=============================================================================
/// Function: drop_weapon
///=============================================================================
/// Description: Drops players weapon to the ground (usually on death)
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::drop_weapon(int index)
{
	Entity *entity = engine->entity_list[index];

	if (entity->player->current_weapon == wp_gauntlet)		return;

	Entity *drop_weapon = engine->entity_list[engine->get_entity()];
	char *weapon_str = NULL;



	drop_weapon->rigid = new EntRigidBody(drop_weapon);
	drop_weapon->position = entity->position;
	drop_weapon->model = drop_weapon->rigid;
	drop_weapon->rigid->impact_index = SND_DROPGUN;

	switch (entity->player->current_weapon)
	{
	case wp_machinegun:
		weapon_str = "weapon_machinegun";
		drop_weapon->model->clone(entity->player->weapon_machinegun);
		drop_weapon->nettype = NET_MACHINEGUN;
		break;
	case wp_shotgun:
		weapon_str = "weapon_shotgun";
		drop_weapon->model->clone(entity->player->weapon_shotgun);
		drop_weapon->nettype = NET_SHOTGUN;
		break;
	case wp_grenade:
		weapon_str = "weapon_grenadelauncher";
		drop_weapon->model->clone(entity->player->weapon_grenade);
		drop_weapon->nettype = NET_GRENADE_LAUNCHER;
		break;
	case wp_rocket:
		weapon_str = "weapon_rocketlauncher";
		drop_weapon->model->clone(entity->player->weapon_rocket);
		drop_weapon->nettype = NET_ROCKET_LAUNCHER;
		break;
	case wp_plasma:
		weapon_str = "weapon_plasma";
		drop_weapon->model->clone(entity->player->weapon_plasma);
		drop_weapon->nettype = NET_PLASMAGUN;
		break;
	case wp_lightning:
		weapon_str = "weapon_lightning";
		drop_weapon->model->clone(entity->player->weapon_lightning);
		drop_weapon->nettype = NET_LIGHTNINGGUN;
		break;
	case wp_railgun:
		weapon_str = "weapon_railgun";
		drop_weapon->model->clone(entity->player->weapon_railgun);
		drop_weapon->nettype = NET_RAILGUN;
		break;
	default:
		weapon_str = "weapon_machinegun";		drop_weapon->model->clone(entity->player->weapon_machinegun);		drop_weapon->nettype = NET_MACHINEGUN;		break;
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
	drop_weapon->rigid->flags.translational_friction_flag = true;
	drop_weapon->rigid->flags.rotational_friction_flag = true;



	vec3 forward;
	entity->model->getForwardVector(forward);
	drop_weapon->rigid->velocity = forward *-1.0f;
	drop_weapon->rigid->angular_velocity.x = 10.0f;

	//  Prevent player from picking weapon back up
	entity->player->state = PLAYER_DEAD;

	drop_weapon->trigger = new EntTrigger(drop_weapon, engine->audio);
	drop_weapon->trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
	drop_weapon->trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
	sprintf(drop_weapon->trigger->action, "%s", weapon_str);
}

///=============================================================================
/// Function: drop_powerup
///=============================================================================
/// Description: Drops players powerup to the ground (usually on death)
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::drop_powerup(vec3 &position, char *model, char *action)
{
	Entity *drop = engine->entity_list[engine->get_entity()];
	drop->position = position;
	drop->nettype = NET_QUAD;


	drop->rigid = new EntRigidBody(drop);
	drop->model = drop->rigid;
	drop->model->load(engine->gfx, model);

	drop->rigid->velocity = vec3(0.0f, 2.0f, 0.0);
	drop->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
	drop->trigger = new EntTrigger(drop, engine->audio);
	drop->trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
	drop->trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
	strcpy(drop->trigger->action, action);
}


///=============================================================================
/// Function: step
///=============================================================================
/// Description: Called every 16ms or so to advance game state a single time step
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::step(int frame_step)
{
	unsigned int num_bot = 3;

#ifdef WIN32
	/*
	int fpOld, fpNew;
	fpOld = _controlfp(0, 0);
	//fpNew = fpOld & ~(EM_OVERFLOW | EM_UNDERFLOW | EM_INEXACT | EM_ZERODIVIDE | EM_DENORMAL | EM_INVALID);
	fpNew = fpOld & ~( EM_ZERODIVIDE | EM_INVALID);
	_controlfp(fpNew, MCW_EM);
	*/
#endif


/*	if (enabled)
	{
		static int count = 0;

		count++;

		if (count % 40)
		{
			target++;
		}

		if (target > 2000)
		{
			target = 0;
		}
	}
	*/
	if (engine->entity_list.size() == 0)
		return;

	if (flyby)
	{
		static float t = 0.0f;

		if (engine->input.attack)
		{
			flyby = false;
			engine->camera_frame.reset();
			if (spectator == true)
				engine->console("spectate");

		}
		t += 0.0005f;

		if (t >= 1.0f)
		{
			t = 0.0f;
			flyby = false;

			if (spectator == true)
				engine->console("spectate");
		}

		spline.step(engine->camera_frame, t);
		return;
	}

	if (frame_step % TICK_RATE == 0)
	{
		blink = !blink;
		round_time++;


		if (warmup)
		{
			int timeleft = warmup_time - round_time;
			if (played_prepare == false)
			{
				engine->play_wave_global(SND_PREPARE);
				played_prepare = true;
			}

			if (timeleft == 3)
			{
				engine->play_wave_global(SND_THREE);
			}
			else if ( timeleft == 2)
			{
				engine->play_wave_global(SND_TWO);
			}
			else if (timeleft == 1)
			{
				engine->play_wave_global(SND_ONE);
			}
			if (warmup_time <= round_time)
			{
				warmup = false;
				hold_fire = false;
				console(-1, "reset 0", engine->menu, engine->entity_list);
				for (unsigned int i = 0; i < engine->max_player; i++)
				{
					if (engine->entity_list[i]->player)
					{
						memset(&engine->entity_list[i]->player->stats, 0, sizeof(stats_t));
					}
				}
				engine->input.scores = false;
				console(-1, "reset 1", engine->menu, engine->entity_list);
				engine->play_wave_global(SND_FIGHT);
			}
		}
		else
		{
			if (timelimit * 60 - round_time == 60)
			{
				engine->play_wave_global(SND_ONE_MINUTE);
			}
			else if (timelimit * 60 - round_time == 60 * 5)
			{
				engine->play_wave_global(SND_FIVE_MINUTE);
			}
			else if (timelimit * 60 - round_time <= 0)
			{
				if (gametype == GAMETYPE_CTF && red_flag_caps == blue_flag_caps)
				{
					if (played_sudden == false)
					{
						engine->play_wave_global(SND_SUDDEN);
						played_sudden = true;
					}
				}
				else
				{
					char winner[128];

					if (red_flag_caps > blue_flag_caps)
						sprintf(winner, "Red team wins!");
					else
						sprintf(winner, "Blue team wins!");
					endgame(winner);
				}

				if (gametype == GAMETYPE_DEATHMATCH)
				{
					bool tied = false;
					char *leader = NULL;

					for (unsigned int i = 0; i < engine->entity_list.size(); i++)
					{
						int max_frags = 0;
						EntPlayer *player = engine->entity_list[i]->player;

						if (player != NULL)
						{
							if (player->stats.kills > max_frags)
							{
								max_frags = player->stats.kills;
								leader = player->name;
								tied = false;
							}
							else if (player->stats.kills == max_frags)
							{
								tied = true;
							}
						}
					}

					if (tied)
					{
						if (played_sudden == false)
						{
							engine->play_wave_global(SND_SUDDEN);
							played_sudden = true;
						}
					}
					else
					{
						char winner[128];

						sprintf(winner, "%s wins", leader);
						endgame(winner);
					}
				}

			}
		}
	}


	if (engine->netcode.server_flag == false && engine->netcode.client_flag == false && engine->num_bot < num_bot)
	{
		for (unsigned int i = 0; i < num_bot; i++)
		{
			char bot_name[32];
			int bot_index = -1;

			sprintf(bot_name, "Bot %d", i);
			add_player(engine->entity_list, BOT, bot_index, bot_name);
			engine->num_bot++;
		}
	}

	if (spectator == true && engine->menu.console == false)
	{
		engine->camera_frame.update(engine->input);
	}

	if (engine->input.control && spectator_timer <= 0)
	{
		spectator_timer = TICK_RATE >> 1;
//		engine->console("spectate");

		vec3 zero(0,50,0);
		//build_structure(zero, MODEL_LUCY);
		build_sentry();
	}
	else
	{
		if (spectator_timer > 0)
			spectator_timer--;
	}

	for (unsigned int i = engine->max_dynamic; i < engine->entity_list.size(); i++)
	{
		Entity *ent = engine->entity_list[i];
		if (ent->ent_type <= ENT_FUNC_START || ent->ent_type >= ENT_FUNC_END)
			continue;

		switch (ent->ent_type)
		{
		case ENT_FUNC_BOBBING:
			handle_func_bobbing(ent);
			break;
		case ENT_FUNC_PLAT:
			handle_func_platform(ent);
			break;
		case ENT_FUNC_TRAIN:
//			handle_func_train(ent);
			break;
		}
	}


	for (unsigned int i = 0; i < engine->max_dynamic; i++)
	{
		Entity *entity = engine->entity_list[i];
		bool isplayer = (entity->player && entity->player->type == PLAYER);
		bool isbot = (entity->player && entity->player->type == BOT);
		bool isclient = (entity->player && entity->player->type == CLIENT);
		bool isserver = (entity->player && entity->player->type == SERVER);

		if (isplayer || isbot || isserver)
		{
			if (((isplayer || isserver) &&
				(engine->menu.ingame == true || engine->menu.console == true || engine->menu.stringmode == true || engine->menu.chatmode == true)))
			{
				input_t noinput = { 0 };

				handle_player(i, noinput);
			}
			else
			{
				if (entity->player->build_timer < TICK_RATE >> 2)
					handle_player(i, engine->input);

				if (entity->player->build_timer > 0)
					entity->player->build_timer--;

				if (entity->player->build_timer == 1)
				{
					entity->player->num_sentry++;

					if (entity->player->build_type == CT_AUTOSENTRY)
						engine->play_wave(entity->position, SND_SENTRY_UPGRADE);
					else
						engine->play_wave(entity->position, SND_CONSTRUCT_COMPLETE);
				}

			}
		}

		if (isclient)
		{
			for(unsigned int j = 0; j < engine->netcode.client_list.size(); j++)
			{
				if ((unsigned int)engine->netcode.client_list[j]->ent_id == i)
					handle_player(i, engine->netcode.client_list[j]->input);
				else
					printf("Error: mismatch between client and server entity id's\r\n");
			}
		}

		if (entity->construct)
		{
			if (entity->construct->step(engine->input, engine->entity_list, i, *engine))
			{
				// building cancelled, delete entity
				debugf("building cancelled\n");
				if (entity->construct->owner != -1 && engine->entity_list[entity->construct->owner]->player)
				{
					engine->entity_list[entity->construct->owner]->player->build_timer = 0;

					if (entity->construct->base_index != -1)
						engine->clean_entity(entity->construct->base_index);
					engine->clean_entity(i);
				}
			}
			else
			{
				if (entity->construct->bot_state == BOT_ATTACK)
				{
					if (entity->construct->reload_timer == 0)
					{
						handle_machinegun(*engine->entity_list[entity->construct->owner]->player, entity->construct, i, engine->netcode.client_flag);
						if (entity->construct->ammo_bullets > 0)
						{
							engine->play_wave(entity->position, SND_MACHINEGUN);
						}

					}
					if (entity->construct->level == 3 && entity->construct->reload_timer2 == 0)
					{
						handle_rocketlauncher(*engine->entity_list[entity->construct->owner]->player, entity->construct, entity->construct->owner, engine->netcode.client_flag);
						if (entity->construct->ammo_rockets > 0)
						{
							engine->play_wave(entity->position, SND_ROCKET);
						}

					}

					Frame frame;

					// now that we shot, change to yaw only for rendering
					entity->rigid->get_frame(frame);
					frame.forward.y = 0.0f;
					frame.forward.normalize();
					frame.set(entity->rigid->morientation);
				}
			}

		}

		if (handle_bot(entity, i) == -1)
			continue;

	}

	// handles triggers and the projectile as trigger stuff
	engine->num_light = 0;

	//#pragma omp parallel for num_threads(8)
	for (unsigned int j = engine->max_player; j < engine->entity_list.size(); j++)
	{
		Entity *ent = engine->entity_list[j];

		if (ent->light)
			engine->num_light++;

#ifdef OPENMP
		int thread_num = omp_get_thread_num();
		int num_thread = omp_get_num_threads();

		if (engine->entity_list[i]->bsp_leaf % num_thread != thread_num)
			continue;

//		printf("bsp leaf %d Handled by thread %d of %d\n", engine->entity_list[i]->bsp_leaf, thread_num, num_thread);
#endif
		for (unsigned int i = 0; i < engine->max_player; i++)
		{
			EntPlayer *player = engine->entity_list[i]->player;


			Entity *owner = NULL;

			if (ent->projectile && ent->projectile->owner != -1)
				owner = engine->entity_list[ent->projectile->owner];

			if (player && (player->type == PLAYER || player->type == CLIENT || player->type == BOT || player->type == SERVER))
			{
				check_triggers(player, ent, i, engine->entity_list);
				check_func(player, ent, i, engine->entity_list);
				check_projectiles(player, ent, owner, i, j, engine->entity_list);
			}
		}
		if (ent->projectile)
		{
			ent->projectile->active = ent->projectile->will_be_active;
			ent->projectile->will_be_active = false;
		}
	}

#ifdef WIN32
//	_controlfp(fpOld, MCW_EM);
#endif
}


///=============================================================================
/// Function: handle_bot
///=============================================================================
/// Description: Handles bot player logic
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
int Quake3::handle_bot(Entity *entity, int i)
{
	Entity *bot = engine->entity_list[i];
	input_t input;


	if (entity->player == NULL)
		return -1;

	if (entity->player->type != BOT)
		return -1;

	memset(&input, 0, sizeof(input_t));
	if (bot->player->health <= 0)
	{
		if (bot->player->bot_state != BOT_DEAD)
			bot->player->dead_timer = (int)(random() * (TICK_RATE << 1));
		bot->player->bot_state = BOT_DEAD;
		return -1;
	}

	//bot->player->avoid_walls(engine->q3map);
	bot->player->handle_bot(engine->entity_list, i);
	static int temp = 0;
	if (bot->player->immobile && bot->player->health >= 75 && temp != 3)
	{
		entity->model->clone(model_table[MODEL_SENTRY3]);
		temp = 3;
	}
	else if (bot->player->immobile && bot->player->health < 75 && bot->player->health > 50 && temp != 2)
	{
		entity->model->clone(model_table[MODEL_SENTRY2]);
		temp = 2;
	}
	else if (bot->player->immobile && bot->player->health < 50 && temp != 1)
	{
		entity->model->clone(model_table[MODEL_SENTRY1]);
		temp = 1;
	}

	if (bot->player->bot_state == BOT_ALERT || bot->player->bot_state == BOT_ATTACK)
	{
		//clear path
		bot->player->path.step = 0;
		bot->player->path.length = 0;

		if (bot->player->alert_timer == 0)
		{
			engine->play_wave(bot->position, SND_SOLDIER_SIGHT);
			bot->player->alert_timer = 20 * TICK_RATE;
		}
	}
	else
	{
		if (bot->player->idle_timer > 0)
			bot->player->idle_timer--;

		if (bot->player->alert_timer > 0)
			bot->player->alert_timer--;
	}

	if (bot->player->idle_timer == 0)
	{
		bot->player->idle_timer = (20 + rand() % 10) * TICK_RATE;
		engine->play_wave(bot->position, SND_SOLDIER_IDLE);
	}

	if (bot->player->bot_state == BOT_IDLE || bot->player->bot_state == BOT_EXPLORE)
	{
		bot->player->get_item = bot->player->bot_search_for_items(engine->entity_list, i);

		//clear path just in case
		bot->player->path.step = 0;
		bot->player->path.length = 0;
	}
	else
	{
		if (bot->player->bot_state != BOT_GET_ITEM)
			bot->player->idle_timer = (20 + rand() % 10) * TICK_RATE;
	}

	float speed_scale = 1.0f;

	if (bot->player->haste_timer > 0)
		speed_scale = bot->player->haste_factor;

	switch (bot->player->bot_state)
	{
	case BOT_ATTACK:
		engine->zcc.select_animation(ANIM_ATTACK, true);
		if (bot->player->immobile == false)
			bot->rigid->move_forward(speed_scale);
		break;
	case BOT_DEAD:
		engine->zcc.select_animation(ANIM_DEAD, true); // RagDoll?
		bot->model->clone(model_table[MODEL_BOX]);
		engine->play_wave(bot->position, bot->player->model_index * SND_PLAYER + SND_DEATH1);

		if (bot->player->dead_timer > 0)
			bot->player->dead_timer--;

		if (bot->player->dead_timer == 0)
			bot->player->respawn();

		char cmd[64];
		sprintf(cmd, "respawn -1 %d", i);
		console(i, cmd, engine->menu, engine->entity_list);
		break;
	case BOT_GET_ITEM:
	{
		static int nav_array[64] = { 0 };
		int ret = 0;

		if (bot->player->immobile)
			break;


		bot->player->ammo_bullets = 100; // bots cheat on reloading :)
		engine->zcc.select_animation(1, true);
		if (engine->entity_list[bot->player->get_item]->trigger->active)
		{
			//some one got the item before we did, abort
			bot->player->bot_state = BOT_IDLE;
			bot->player->path.step = 0;
			bot->player->path.length = 0;

		}

		// Need a path to item
		if (bot->player->path.length == 0)
		{
			//probably need to pass Player reference instead of each path param, or make a struct
			ret = bot_get_path(bot->player->get_item, i, nav_array,
				bot->player->path);

			if (bot->player->path.length == -1)
			{
				// Path doesnt exist, give up
				strncat(bot->player->ignore, engine->entity_list[bot->player->get_item]->entstring->type, sizeof(bot->player->ignore) - strlen(bot->player->ignore) - 1);
				strncat(bot->player->ignore, " ", sizeof(bot->player->ignore) - strlen(bot->player->ignore) - 1);

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
		engine->zcc.select_animation(ANIM_ALERT, false);

		if (bot->player->immobile == false)
			bot->rigid->move_forward(speed_scale);
		break;
	case BOT_EXPLORE:
		engine->zcc.select_animation(ANIM_EXPLORE, false);
		bot->player->ignore[0] = '\0';
		break;
	case BOT_IDLE:
		engine->zcc.select_animation(ANIM_IDLE, false);
		break;
	}

	return 0;
}


///=============================================================================
/// Function: build_sentry
///=============================================================================
/// Description: Builds a sentry gun
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::build_sentry()
{
	int owner = engine->find_type(ENT_PLAYER, 0);
	EntPlayer *powner = engine->entity_list[owner]->player;

	if (owner == -1)
	{
		return;
	}

	if (powner->build_timer > 0)
	{
		return;
	}

	if (powner->num_sentry >= 1)
	{
		debugf("Sentry already constructed\n");
		return;
	}


	powner->build_timer = 3 * TICK_RATE;


	int spawn = engine->get_entity();
	Entity *ent = engine->entity_list[spawn];
	ent->ent_type = ENT_SENTRY;
	ent->construct = new EntConstructable(ent, engine->gfx, engine->audio, powner->team, CT_AUTOSENTRY);
	ent->construct->owner = owner;
	ent->rigid = new EntRigidBody(ent);

	ent->position = powner->entity->position + engine->camera_frame.forward * -100.0f + vec3(0.0f, 25.0f, 0.0);
	ent->rigid->clone(model_table[MODEL_SENTRY1]);
	for (int i = 0; i < 8; i++)
	{
		ent->rigid->aabb[i].y += -20.0f; //hack to get height right
	}


	ent->model = ent->rigid;
	ent->construct->immobile = true;
	ent->construct->render_md5 = false;


	int base_index = engine->get_entity();
	Entity *sentry_base = engine->entity_list[base_index];
	sentry_base->rigid = new EntRigidBody(sentry_base);
	sentry_base->model = sentry_base->rigid;
	sentry_base->model->clone(model_table[MODEL_SENTRY_BASE]);
	sentry_base->position = engine->entity_list[spawn]->position + vec3(0.0f, 0.0f, -5.0f);
	sentry_base->flags.visible = true;
	sentry_base->ent_type = ENT_CONSTRUCT;

	ent->construct->base_index = base_index;

	engine->play_wave(engine->entity_list[owner]->position, SND_SENTRY_BUILD);
}



///=============================================================================
/// Function: build_structure
///=============================================================================
/// Description: Builds a structure (essentially model that fades in when constructed
/// can be used to build bridges, guns, buildings, etc. Could be destroyed when taking
/// damage also
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::build_structure(vec3 &position, int model_index)
{
	int owner = engine->find_type(ENT_PLAYER, 0);
	if (owner == -1)
		return;

	EntPlayer *powner = engine->entity_list[owner]->player;

	if (powner->build_timer > 0)
	{
		return;
	}

	powner->build_timer = 3 * TICK_RATE;
	powner->build_type = CT_STRUCTURE;

	int spawn = engine->get_entity();
	Entity *ent = engine->entity_list[spawn];
	ent->ent_type = ENT_CONSTRUCT;
	ent->construct = new EntConstructable(ent, engine->gfx, engine->audio, powner->team, CT_STRUCTURE);
	ent->construct->owner = owner;
	ent->rigid = new EntRigidBody(ent);

	ent->position = position;
	ent->rigid->clone(model_table[model_index]);

	ent->model = ent->rigid;
	ent->construct->immobile = true;
	ent->construct->render_md5 = false;

	engine->play_wave(position, SND_CONSTRUCT);
}

///=============================================================================
/// Function: handle_plasma
///=============================================================================
/// Description: Handles plasma gun logic
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::handle_plasma(EntPlayer &player, int self, bool client)
{
	Frame frame;

	player.entity->rigid->get_frame(frame);
	player.reload_timer = PLASMA_RELOAD;
	player.ammo_plasma--;

	float quad_factor = 1.0f;

	if (player.quad_timer > 0)
		quad_factor = QUAD_FACTOR;

	if (client == false)
	{
		Entity *projectile = engine->entity_list[engine->get_entity()];

		projectile->nettype = NET_PLASMA;
		projectile->rigid = new EntRigidBody(projectile);
		projectile->model = projectile->rigid;
		projectile->position = frame.pos;
		frame.set(projectile->model->morientation);
		projectile->flags.visible = true; // accomodate for low spatial testing rate
		projectile->bsp_leaf = player.entity->bsp_leaf;
		projectile->flags.bsp_visible = true;
		projectile->model->flags.blend = true;

		projectile->rigid->clone(model_table[MODEL_BALL]);
		projectile->rigid->velocity = frame.forward * -10.0f;
		projectile->rigid->net_force = frame.forward * -10.0f;

		projectile->rigid->angular_velocity = vec3();
		projectile->rigid->flags.gravity = false;
		projectile->projectile = new EntProjectile(projectile, engine->audio);
//		projectile->trigger->projectile = true;

		projectile->rigid->impact_index = SND_PLASMA_EXPLODE;
		projectile->projectile->explode_index = SND_PLASMA_EXPLODE;
		projectile->projectile->idle_index = SND_PLASMAFLY;

		sprintf(projectile->projectile->action, "damage %d", (int)(PLASMA_DAMAGE * quad_factor));

		projectile->projectile->hide = false;
		projectile->projectile->radius = 25.0f;
		projectile->projectile->idle = true;
		projectile->projectile->explode = false;
		projectile->projectile->explode_type = 2;
		projectile->projectile->explode_timer = 10;
		projectile->projectile->explode_color = vec3(0.0f, 0.0f, 1.0f);
		projectile->projectile->explode_intensity = 200.0f;
		projectile->projectile->splash_damage = (int)(PLASMA_SPLASH_DAMAGE * quad_factor);
		projectile->projectile->splash_radius = 75.0f;
		projectile->projectile->knockback = 10.0f;
		projectile->projectile->owner = self;


		projectile->light = new EntLight(projectile, engine->gfx, 999, engine->res_scale);
		projectile->light->color = vec3(0.0f, 0.0f, 1.0f);
		projectile->light->intensity = 1000.0f;
	}

	add_decal(frame.pos, frame, NET_PLASMA_HIT, model_table[MODEL_PLASMA_HIT], -40.0f, true, 10);

	Entity *muzzleflash = engine->entity_list[engine->get_entity()];
	muzzleflash->position = player.entity->position + frame.forward * -75.0f;
	muzzleflash->light = new EntLight(muzzleflash, engine->gfx, 999, engine->res_scale);
	muzzleflash->light->color = vec3(0.6f, 0.6f, 1.0f);
	muzzleflash->light->intensity = 2000.0f;
	muzzleflash->light->attenuation = 0.0625f;
	muzzleflash->light->timer_flag = true;
	muzzleflash->light->timer = (int)(0.125f * TICK_RATE);
	muzzleflash->flags.visible = true; // accomodate for low spatial testing rate
	muzzleflash->bsp_leaf = player.entity->bsp_leaf;
	muzzleflash->flags.bsp_visible = true;
}

///=============================================================================
/// Function: handle_rocketlauncher
///=============================================================================
/// Description: Handles rocket launcher gun logic
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::handle_rocketlauncher(EntPlayer &player, EntConstructable *sentry, int self, bool client)
{
	Frame frame;

	if (sentry)
	{
		sentry->entity->rigid->get_frame(frame);
		sentry->reload_timer2 = ROCKET_RELOAD;
		sentry->ammo_rockets--;
	}
	else
	{
		player.entity->rigid->get_frame(frame);
		player.reload_timer = ROCKET_RELOAD;
		player.ammo_rockets--;
	}

	float quad_factor = 1.0f;

	if (player.quad_timer > 0)
		quad_factor = QUAD_FACTOR;


	if (client == false)
	{
		Entity *projectile = engine->entity_list[engine->get_entity()];
		projectile->nettype = NET_ROCKET;
		projectile->position = frame.pos;
		projectile->flags.visible = true; // accomodate for low spatial testing rate
		projectile->flags.bsp_visible = true;
		projectile->bsp_leaf = player.entity->bsp_leaf;

		projectile->projectile = new EntProjectile(projectile, engine->audio);
		projectile->projectile->explode_index = SND_EXPLODE;
		projectile->projectile->idle_index = SND_ROCKETFLY;

		sprintf(projectile->projectile->action, "damage %d", (int)(ROCKET_DAMAGE * quad_factor));

		projectile->projectile->hide = false;
		projectile->projectile->radius = 25.0f;
		projectile->projectile->idle = true;
		projectile->projectile->explode = true;
		projectile->projectile->explode_type = 1;
		projectile->projectile->idle_timer = 0;
		projectile->projectile->num_bounce = 0;
		projectile->projectile->explode_timer = 10;
		projectile->projectile->explode_color = vec3(1.0f, 0.0f, 0.0f);
		projectile->projectile->explode_intensity = 500.0f;
		projectile->projectile->splash_damage = (int)(ROCKET_SPLASH_DAMAGE * quad_factor);
		projectile->projectile->splash_radius = 250.0f;
		projectile->projectile->knockback = 250.0f;
		projectile->projectile->owner = self;


		projectile->flags.particle_on = true;
		projectile->num_particle = 5000;

		projectile->light = new EntLight(projectile, engine->gfx, 999, engine->res_scale);
		projectile->light->color = vec3(1.0f, 1.0f, 1.0f);
		projectile->light->intensity = 1000.0f;

		projectile->rigid = new EntRigidBody(projectile);
		projectile->model = projectile->rigid;
		frame.set(projectile->rigid->morientation);
		projectile->rigid->clone(model_table[MODEL_ROCKET]);
		projectile->rigid->velocity = frame.forward * -6.25f;
		projectile->rigid->net_force = frame.forward * -10.0f;
		projectile->rigid->angular_velocity = vec3();
		projectile->rigid->flags.gravity = false;
		projectile->rigid->impact_index = SND_EXPLODE;


		projectile->projectile->create_sources(engine->audio);

		engine->play_wave_source(projectile->projectile->loop_source, projectile->projectile->idle_index);
	}

	vec3 color(1.0f, 0.75f, 0.0f);
	add_muzzle_flash(frame, player, color, 2000.0f, 0.0625f, 0.125f);
}

///=============================================================================
/// Function: handle_grenade
///=============================================================================
/// Description: Handles grenade launcher gun logic
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::handle_grenade(EntPlayer &player, int self, bool client)
{
	Frame frame;

	player.entity->rigid->get_frame(frame);
	player.reload_timer = GRENADE_RELOAD;
	player.ammo_grenades--;

	if (client == false)
	{
		Entity *projectile = engine->entity_list[engine->get_entity()];
		projectile->nettype = NET_GRENADE;

		projectile->rigid = new EntRigidBody(projectile);
		projectile->model = projectile->rigid;
		projectile->position = frame.pos;
		frame.set(projectile->model->morientation);
		projectile->flags.visible = true; // accomodate for low spatial testing rate
		projectile->bsp_leaf = player.entity->bsp_leaf;
		projectile->flags.bsp_visible = true;

		projectile->rigid->clone(model_table[MODEL_GRENADE]);
		projectile->rigid->velocity = frame.forward * -25.0f;
		projectile->rigid->angular_velocity = vec3(1.1f, 0.1f, 1.1f);
		projectile->rigid->flags.gravity = true;
		projectile->rigid->flags.ground_friction_flag = true;
		projectile->rigid->flags.rotational_friction_flag = true;
		//entity->rigid->set_target(*(entity_list[spawn]));

		projectile->flags.particle_on = true;
		projectile->num_particle = 5000;


		float quad_factor = 1.0f;

		if (player.quad_timer > 0)
			quad_factor = QUAD_FACTOR;


		projectile->projectile = new EntProjectile(projectile, engine->audio);
		projectile->projectile->num_bounce = 20;
		projectile->projectile->explode_index = SND_EXPLODE;
		sprintf(projectile->projectile->action, "damage %d", (int)(GRENADE_DAMAGE * quad_factor));

		projectile->projectile->hide = false;
		projectile->projectile->radius = 50.0f;
		projectile->projectile->idle = true;
		projectile->projectile->idle_timer = 120;
		projectile->projectile->explode = true;
		projectile->projectile->explode_type = 1;
		projectile->projectile->explode_timer = 10;
		projectile->projectile->explode_color = vec3(1.0f, 0.0f, 0.0f);
		projectile->projectile->explode_intensity = 500.0f;
		projectile->projectile->splash_damage = (int)(GRENADE_SPLASH_DAMAGE * quad_factor);
		projectile->projectile->splash_radius = 250.0f;
		projectile->projectile->knockback = 250.0f;
		projectile->projectile->owner = self;
	}

	vec3 color(1.0f, 0.7f, 0.0f);
	add_muzzle_flash(frame, player, color, 2000.0f, 0.0625f, 0.125f);
}

///=============================================================================
/// Function: handle_lightning
///=============================================================================
/// Description: Handles lightning gun logic
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::handle_lightning(EntPlayer &player, int self, bool client)
{
	Frame frame;
	int index[16] = { -1 };
	int num_index = 0;

	player.entity->rigid->get_frame(frame);


	if (player.entity->rigid->flags.water && player.entity->rigid->water_depth >= 25.0f)
	{
		player.health -= player.ammo_lightning * LIGHTNING_DAMAGE;
		if (player.health < 0)
		{
			char msg[256];

			sprintf(msg, "%s discovered water conducts electricity\n", player.name);
			debugf(msg);
			engine->menu.print_notif(msg);
			notif_timer = 3 * TICK_RATE;
		}

		for (unsigned int i = 0; i < engine->max_player; i++)
		{
			if (i == (unsigned int)self)
				continue;

			Entity *ent = engine->entity_list[i];

			if (ent->player && ent->bsp_leaf == player.entity->bsp_leaf)
			{
				EntPlayer *enemy = ent->player;

				if (enemy->godmode == false)
				{
					enemy->health -= enemy->ammo_lightning * LIGHTNING_DAMAGE;
				}
				if (enemy->health < 0)
				{
					char msg[256];

					sprintf(msg, "%s was caught up in %s discharge\n", enemy->name, player.name);
					debugf(msg);
					engine->menu.print_notif(msg);
					notif_timer = 3 * TICK_RATE;
				}
			}
		}
		player.ammo_lightning = 0;
	}

	player.reload_timer = LIGHTNING_RELOAD;
	player.ammo_lightning--;

	vec3 forward;
	player.entity->model->getForwardVector(forward);

	float quad_factor = 1.0f;

	if (player.quad_timer > 0)
		quad_factor = QUAD_FACTOR;



	if (client == false)
	{
		Entity *projectile = engine->entity_list[engine->get_entity()];
		projectile->nettype = NET_LIGHTNING;
		projectile->rigid = new EntRigidBody(projectile);
		projectile->model = projectile->rigid;
		projectile->position = frame.pos;
		projectile->rigid->clone(model_table[MODEL_BOX]);
		projectile->rigid->velocity = vec3();
		projectile->rigid->angular_velocity = vec3();
		projectile->rigid->flags.gravity = false;
		projectile->rigid->bounce = 2;
	//	projectile->rigid->rotational_friction_flag = true;
		projectile->model->flags.lightning_trail = true;
		projectile->model = projectile->rigid;
	//	projectile->rigid->set_target(*(engine->entity_list[self]));
		frame.set(projectile->model->morientation);
		projectile->flags.visible = true; // accomodate for low spatial testing rate
		projectile->rigid->flags.noclip = true;
		projectile->bsp_leaf = player.entity->bsp_leaf;
		projectile->flags.bsp_visible = true;

		/*
		projectile->light = new Light(projectile, engine->gfx, 999);
		projectile->light->color = vec3(1.0f, 1.0f, 1.0f);
		projectile->light->intensity = 1000.0f;
		*/

		projectile->projectile = new EntProjectile(projectile, engine->audio);
		projectile->projectile->action[0] = '\0';

//		projectile->rigid->bounce = 5;
		projectile->projectile->hide = false;
		projectile->projectile->radius = 25.0f;
		projectile->projectile->idle = true;
		projectile->projectile->idle_timer = (int)(0.025 * TICK_RATE);
		projectile->projectile->explode = true;
		projectile->projectile->explode_timer = 5;
		projectile->projectile->owner = self;



		engine->hitscan(frame.pos, forward, index, num_index, self, 768.0f);
		for (int i = 0; i < num_index; i++)
		{
			char cmd[512] = { 0 };
			EntPlayer *target = engine->entity_list[index[i]]->player;

			if (target == NULL)
			{
				EntConstructable *construct = engine->entity_list[index[i]]->construct;
				if (construct && construct->construct_type == CT_AUTOSENTRY)
				{
					int health = construct->health;

					if ( health < 220)
						construct->health += (int)(LIGHTNING_DAMAGE * quad_factor);

					if (health <= 150)
					{
						if (construct->level != 1)
						{
							construct->level = 1;
							construct->entity->rigid->clone(model_table[MODEL_SENTRY1]);
						}
					}
					if (health > 150 && health < 180)
					{
						if (construct->level != 2)
						{
							construct->level = 2;
							construct->entity->rigid->clone(model_table[MODEL_SENTRY2]);
						}
					}
					else if (health >= 180 && health < 220)
					{
						if (construct->level != 3)
						{
							construct->level = 3;
							construct->entity->rigid->clone(model_table[MODEL_SENTRY3]);
						}
					}
				}
				continue;
			}

			if (target->health <= 0)
				continue;

			if (player.team == target->team && gametype != GAMETYPE_DEATHMATCH)
				continue;

			debugf("Player %s hit %s with the lightning gun for %d damage\n", player.name,
				target->name, (int)(LIGHTNING_DAMAGE * quad_factor));
			sprintf(cmd, "hurt %d %d", index[i], (int)(LIGHTNING_DAMAGE * quad_factor));


			console(self, cmd, engine->menu, engine->entity_list);

			debugf("%s has %d health\n", target->name,
				target->health);

			player.stats.hits++;
			if (target->health <= 0 && target->state != PLAYER_DEAD)
			{
				char msg[512];
				char word[32] = { 0 };
				player.stats.kills++;
				target->stats.deaths++;

				if (target->health <= GIB_HEALTH)
					sprintf(word, "%s", "gibbed");
				else
					sprintf(word, "%s", "killed");

				sprintf(msg, "%s %s %s with a lightning gun\n", player.name,
					word,
					target->name);
				debugf(msg);
				engine->menu.print_notif(msg);
				notif_timer = 3 * TICK_RATE;
				handle_frags_left(player);
			}
		}
	}

	vec3 color(0.6f, 0.6f, 1.0f);
	add_muzzle_flash(frame, player, color, 2000.0f, 0.0625f, 0.125f);
}

///=============================================================================
/// Function: handle_railgun
///=============================================================================
/// Description: Handles rail gun logic
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::handle_railgun(EntPlayer &player, int self, bool client)
{
	Frame frame;

	player.entity->rigid->get_frame(frame);
	player.reload_timer = RAILGUN_RELOAD;
	player.ammo_slugs--;


	if (client == false)
	{
		Entity *projectile = engine->entity_list[engine->get_entity()];
		int index[16] = { -1 };
		int num_index = 0;

		projectile->nettype = NET_RAIL;
		projectile->rigid = new EntRigidBody(projectile);
		projectile->position = frame.pos;
		projectile->rigid->clone(model_table[MODEL_BALL]);
		projectile->rigid->velocity = vec3();
		projectile->rigid->angular_velocity = vec3();
		projectile->rigid->flags.gravity = false;
		projectile->rigid->bounce = 5;
		projectile->model = projectile->rigid;
		projectile->model->flags.rail_trail = true;
		projectile->rigid->flags.noclip = true;

		frame.set(projectile->model->morientation);
		projectile->flags.visible = true; // accomodate for low spatial testing rate
		projectile->bsp_leaf = player.entity->bsp_leaf;
		projectile->flags.bsp_visible = true;

		projectile->projectile = new EntProjectile(projectile, engine->audio);
		sprintf(projectile->projectile->action, " ");

		projectile->projectile->hide = false;
		projectile->projectile->radius = 25.0f;
		projectile->projectile->idle = true;
		projectile->projectile->idle_timer = (int)(1.0 * TICK_RATE);
		projectile->projectile->explode = true;
		projectile->projectile->explode_timer = 10;
		projectile->projectile->owner = self;

		vec3 forward;
		player.entity->model->getForwardVector(forward);

		float quad_factor = 1.0f;

		if (player.quad_timer > 0)
			quad_factor = QUAD_FACTOR;

		engine->hitscan(frame.pos, forward, index, num_index, self, -1.0f);
		if (num_index == 0)
			player.impressive_count = 0;


		for (int i = 0; i < num_index; i++)
		{
			EntPlayer *target = engine->entity_list[index[i]]->player;
			char cmd[64] = { 0 };
	
			if (target == NULL)
				continue;

			if (target->health <= 0)
				continue;

			if (player.team == target->team && gametype != GAMETYPE_DEATHMATCH)
				continue;


			debugf("Player %s hit %s with the railgun for %d damage\n", player.name,
				target->name, (int)(RAILGUN_DAMAGE * quad_factor));

			sprintf(cmd, "hurt %d %d", index[i], (int)(RAILGUN_DAMAGE * quad_factor));
			console(self, cmd, engine->menu, engine->entity_list);

			debugf("%s has %d health\n", target->name,
				target->health);

			player.stats.hits++;
			if (target->health <= 0 && target->state != PLAYER_DEAD)
			{
				char msg[256];
				char word[32] = { 0 };
				player.stats.kills++;
				target->stats.deaths++;

				if (target->health <= GIB_HEALTH)
					sprintf(word, "%s", "gibbed");
				else
					sprintf(word, "%s", "killed");

				player.impressive_count++;

				if (player.impressive_count > 1)
				{
					player.impressive_award_timer = 3 * TICK_RATE;
					player.stats.medal_impressive++;
					engine->play_wave_global(SND_IMPRESSIVE);
				}

				if (player.excellent_timer > 0)
				{
					player.excellent_award_timer = 3 * TICK_RATE;
					player.stats.medal_excellent++;
					engine->play_wave_global(SND_EXCELLENT);
				}

				player.excellent_timer = 3 * TICK_RATE;
				sprintf(msg, "%s %s %s with a railgun\n", player.name,
					word,
					target->name);
				debugf(msg);
				engine->menu.print_notif(msg);
				notif_timer = 3 * TICK_RATE;
				handle_frags_left(player);
			}
		}
	}


	vec3 color(1.0f, 0.5f, 0.0f);
	add_muzzle_flash(frame, player, color, 2000.0f, 0.0625f, 0.125f);
}

///=============================================================================
/// Function: handle_gauntlet
///=============================================================================
/// Description: Handles gauntlet melee logic
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::handle_gauntlet(EntPlayer &player, int self, bool client)
{
	char cmd[64] = { 0 };
	int index[16] = { -1 };
	int num_index = 0;

	Frame frame;
	vec3 dir;
	vec3 end;


	player.reload_timer = 5; // prevent doing gauntlet hitscan every frame

	player.entity->rigid->get_frame(frame);
	frame.forward *= -1;
	player.ammo_bullets--;



	vec3 color(1.0f, 1.0f, 1.0f);
	add_muzzle_flash(frame, player, color, 2000.0f, 0.0625f, 0.125f);


	if (client == false)
	{
		engine->hitscan(frame.pos, frame.forward, index, num_index, self, 75.0f);

		float quad_factor = 1.0f;

		if (player.quad_timer > 0)
			quad_factor = QUAD_FACTOR;

		for (int i = 0; i < num_index; i++)
		{
			EntPlayer *target = engine->entity_list[index[i]]->player;

			if (target == NULL)
				continue;

			if (target->health <= 0)
				continue;

			if (player.team == target->team && gametype != GAMETYPE_DEATHMATCH)
				continue;

			player.reload_timer = GAUNTLET_RELOAD;

			if (player.local)
				player.weapon_source = engine->play_wave_global(SND_GAUNTLET_HIT);
			else
				player.weapon_source = engine->play_wave(player.entity->position, SND_GAUNTLET_HIT);

			debugf("Player %s hit %s with the gauntlet for %d damage\n", player.name,
				target->name, (int)(GAUNTLET_DAMAGE * quad_factor));
			sprintf(cmd, "hurt %d %d", index[i], (int)(GAUNTLET_DAMAGE * quad_factor));
			console(self, cmd, engine->menu, engine->entity_list);
			debugf("%s has %d health\n", target->name, target->health);
			player.stats.hits++;

			if (target->health <= 0 && target->state != PLAYER_DEAD)
			{
				char msg[256];
				char word[16] = { 0 };

				player.stats.kills++;
				target->stats.deaths++;

				if (target->health <= GIB_HEALTH)
					sprintf(word, "%s", "gibbed");
				else
					sprintf(word, "%s", "killed");

				player.gauntlet_award_timer = 3 * TICK_RATE;
				player.stats.medal_humiliation++;
				engine->play_wave_global(277);

				sprintf(msg, "%s killed %s with a gauntlet\n", player.name,
					target->name);
				debugf(msg);
				engine->menu.print_notif(msg);
				notif_timer = 3 * TICK_RATE;

				handle_frags_left(player);
			}
		}
	}

}

///=============================================================================
/// Function: handle_machinegun
///=============================================================================
/// Description: Handles machine gun logic
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::handle_machinegun(EntPlayer &player, EntConstructable *sentry, int self, bool client)
{
	char cmd[64] = { 0 };
	int index[16] = { -1 };
	int num_index = 0;

	Frame frame;
	vec3 dir;
	vec3 end;

	if (sentry)
	{
		sentry->entity->rigid->get_frame(frame);
		frame.forward *= -1;

		if (sentry->level > 1)
			sentry->reload_timer = (int)(MACHINEGUN_RELOAD * 0.8f);
		else
			sentry->reload_timer = (int)(MACHINEGUN_RELOAD * 1.4f);
		sentry->ammo_bullets--;
	}
	else
	{
		player.entity->rigid->get_frame(frame);
		frame.forward *= -1;
		player.reload_timer = MACHINEGUN_RELOAD;
		player.ammo_bullets--;
	}


	// Added to end vector
	int spread = 200;
	float r = random() * MY_PI * 2.0f;
	float spread_up = (float)(fsin(r) * crandom() * spread * 16);
	float spread_right = (float)(fcos(r) * crandom() * spread * 16);

	end = frame.pos + frame.forward * 8192 * 16;
	end.x += spread_right;
	end.y += spread_up;
	end.z += spread_right;

	dir = end - frame.pos;
	dir.normalize();

	frame.forward = dir;



	add_decal(frame.pos, frame, NET_BULLET_HIT, model_table[MODEL_BULLET_HIT], 10.0f, true, 10);
	vec3 color(1.0f, 1.0f, 0.0f);
	add_muzzle_flash(frame, player, color, 2000.0f, 0.0625f, 0.125f);
	add_ejection_shell(frame, player, NET_BULLET, MODEL_BULLET, SND_BULLET);


	if (client == false)
	{
		engine->hitscan(frame.pos, frame.forward, index, num_index, self, -1.0f);

		float quad_factor = 1.0f;

		if (player.quad_timer > 0)
			quad_factor = QUAD_FACTOR;

		for (int i = 0; i < num_index; i++)
		{
			EntPlayer *target = engine->entity_list[index[i]]->player;
			if (target == NULL)
				continue;

			if (index[i] == self)
				continue;

			if (target->health <= 0)
				continue;

			if (player.team == target->team && gametype != GAMETYPE_DEATHMATCH)
				continue;

			if (sentry)
			{
				debugf("%s's autosentry hit %s with the machinegun for %d damage\n", player.name,
					target->name, (int)(MACHINEGUN_DAMAGE * quad_factor));
				sprintf(cmd, "hurt %d %d", index[i], (int)(MACHINEGUN_DAMAGE * quad_factor));
				console(self, cmd, engine->menu, engine->entity_list);
				debugf("%s has %d health\n", target->name, target->health);
			}
			else
			{
				debugf("Player %s hit %s with the machinegun for %d damage\n", player.name,
					target->name, (int)(MACHINEGUN_DAMAGE * quad_factor));
				sprintf(cmd, "hurt %d %d", index[i], (int)(MACHINEGUN_DAMAGE * quad_factor));
				console(self, cmd, engine->menu, engine->entity_list);
				debugf("%s has %d health\n", target->name, target->health);
				player.stats.hits++;
			}
	
			if (target->health <= 0 && target->state != PLAYER_DEAD)
			{
				char msg[256];
				char word[16] = { 0 };

				player.stats.kills++;
				target->stats.deaths++;

				if (target->health <= GIB_HEALTH)
					sprintf(word, "%s", "gibbed");
				else
					sprintf(word, "%s", "killed");

				if (sentry)
				{
					sprintf(msg, "%s's autosentry killed %s with a machinegun\n", player.name, target->name);
					debugf(msg);
					engine->menu.print_notif(msg);
					notif_timer = 3 * TICK_RATE;
				}
				else
				{
					sprintf(msg, "%s killed %s with a machinegun\n", player.name, target->name);
					debugf(msg);
					engine->menu.print_notif(msg);
					notif_timer = 3 * TICK_RATE;
				}

				handle_frags_left(player);
			}
		}
	}

}


///=============================================================================
/// Function: handle_frags_left
///=============================================================================
/// Description: Handles frags left countdown logic
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::handle_frags_left(EntPlayer &player)
{
	if (player.stats.kills >= fraglimit)
	{
		char winner[192];

		sprintf(winner, "%s wins", player.name);
		endgame(winner);
		return;
	}
	else if (fraglimit - player.stats.kills == 1 && played_one_frag == false)
	{
		engine->play_wave_global(SND_ONE_FRAG);
	}
	else if (fraglimit - player.stats.kills == 2 && played_two_frag == false)
	{
		engine->play_wave_global(SND_TWO_FRAG);
	}
	else if (fraglimit - player.stats.kills == 3 && played_three_frag == false)
	{
		engine->play_wave_global(SND_THREE_FRAG);
	}
}


///=============================================================================
/// Function: add_muzzle_flash
///=============================================================================
/// Description: Adds light to gun shots
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::add_muzzle_flash(Frame  &frame, EntPlayer &player, vec3 &color, float intensity, float attenuation, float duration)
{
	Entity *muzzleflash = engine->entity_list[engine->get_entity()];

	muzzleflash->position = player.entity->position + frame.forward * 75.0f;
	muzzleflash->light = new EntLight(muzzleflash, engine->gfx, 999, engine->res_scale);
	muzzleflash->light->color = color;
	muzzleflash->light->intensity = intensity;
	muzzleflash->light->attenuation = attenuation;
	muzzleflash->light->timer_flag = true;
	muzzleflash->light->timer = (int)(duration * TICK_RATE);
	muzzleflash->flags.visible = true; // accomodate for low spatial testing rate
	muzzleflash->bsp_leaf = player.entity->bsp_leaf;
	muzzleflash->flags.bsp_visible = true;
}


///=============================================================================
/// Function: add_ejection_shell
///=============================================================================
/// Description: Adds ejection shell to gun shots (shotgun / machinegun)
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::add_ejection_shell(Frame  &frame, EntPlayer &player, net_ent_t type, int model_index, int sound_index)
{
	Entity *shell = engine->entity_list[engine->get_entity()];
	vec3 right = vec3::crossproduct(frame.forward, frame.up);


	shell->nettype = type;
	shell->rigid = new EntRigidBody(shell);
	shell->position = frame.pos;
	shell->position += frame.forward * 3.0f - frame.up * 4.0f + right * 5.0f;
	shell->rigid->clone(model_table[model_index]);
	frame.set(shell->rigid->morientation);
	shell->rigid->velocity += right * random() + frame.up * random();
	shell->rigid->angular_velocity = vec3(1.0f * random(), 2.0f * random(), 3.0f  * random());
	shell->rigid->flags.gravity = true;
	shell->rigid->flags.rotational_friction_flag = true;
	shell->rigid->flags.translational_friction_flag = true;
	shell->rigid->translational_friction = 0.9f;
	shell->rigid->impact_index = SND_SHELL;


	shell->model = shell->rigid;
	frame.set(shell->model->morientation);
	shell->flags.visible = true; // accomodate for low spatial testing rate
	shell->bsp_leaf = player.entity->bsp_leaf;
	shell->flags.bsp_visible = true;

}

///=============================================================================
/// Function: handle_shotgun
///=============================================================================
/// Description: Handles shot gun logic
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::handle_shotgun(EntPlayer &player, int self, bool client)
{
	Frame frame;

	player.entity->rigid->get_frame(frame);

	player.reload_timer = SHOTGUN_RELOAD;
	player.ammo_shells--;

	//	engine->map.hitscan(player.entity->position, forward, distance);

	frame.forward *= -1;

	vec3 color(1.0f, 1.0f, 1.0f);
	add_muzzle_flash(frame, player, color, 1000.0f, 0.125f, 0.125f);
	add_ejection_shell(frame, player, NET_SHELL, MODEL_SHELL, SND_SHELL);
	add_ejection_shell(frame, player, NET_SHELL, MODEL_SHELL, SND_SHELL);

	for (int i = 0; i < 10; i++)
	{
		// Added to end vector
		int spread = 700;
		float r = random() * MY_PI * 2.0f;
		float spread_up = (float)(fsin(r) * crandom() * spread * 16);
		float spread_right = (float)(fcos(r) * crandom() * spread * 16);
		vec3 end;
		vec3 dir;

		end = frame.pos + frame.forward * 8192 * 16;
		end.x += spread_right;
		end.y += spread_up;
		end.z += spread_right;

		dir = end - player.entity->position;
		dir.normalize();

		frame.forward = dir;


		add_decal(frame.pos, frame, NET_BULLET_HIT, model_table[MODEL_BULLET_HIT], 10.0f, true, 10);


		if (client == false)
		{
			int index[16] = { -1 };
			int num_index = 0;
			float quad_factor = 1.0f;

			if (player.quad_timer > 0)
				quad_factor = QUAD_FACTOR;


			engine->hitscan(frame.pos, frame.forward, index, num_index, self, -1.0f);
			for (int i = 0; i < num_index; i++)
			{
				EntPlayer *target = engine->entity_list[index[i]]->player;
				char cmd[64] = { 0 };

				if (target == NULL)
					continue;

				if (target->health <= 0)
					continue;

				if (player.team == target->team && gametype != GAMETYPE_DEATHMATCH)
					continue;

				debugf("Player %s hit %s with the shotgun for %d damage\n", player.name,
					target->name, (int)(SHOTGUN_DAMAGE * quad_factor));

				sprintf(cmd, "hurt %d %d", index[i], (int)(SHOTGUN_DAMAGE * quad_factor));

				console(self, cmd, engine->menu, engine->entity_list);
				debugf("%s has %d health\n", target->name,
					target->health);

				player.stats.hits++;
				if (target->health <= 0 && target->state != PLAYER_DEAD)
				{
					char msg[64];
					char word[32] = { 0 };

					player.stats.kills++;
					target->stats.deaths++;

					if (target->health <= GIB_HEALTH)
						sprintf(word, "%s", "gibbed");
					else
						sprintf(word, "%s", "killed");

					sprintf(msg, "%s %s %s with a shotgun\n", player.name,
						word,
						target->name);
					debugf(msg);
					engine->menu.print_notif(msg);
					notif_timer = 3 * TICK_RATE;

					handle_frags_left(player);
				}
			}
		}
	}
}

///=============================================================================
/// Function: handle_gibs
///=============================================================================
/// Description: Handles exploding player into piles of gibs/guts logic
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::handle_gibs(EntPlayer &player)
{
	Frame camera_frame;

	player.entity->rigid->get_frame(camera_frame);
	player.entity->rigid->velocity += vec3(0.5f, 3.0f, 1.2f);

	{
		Entity *entity0 = engine->entity_list[engine->get_entity()];
		entity0->rigid = new EntRigidBody(entity0);
		entity0->model = entity0->rigid;
		entity0->position = camera_frame.pos;
		camera_frame.set(entity0->model->morientation);

		entity0->rigid->clone(model_table[MODEL_GIB0]);
		entity0->rigid->velocity = vec3(2.0f, 1.2f, 1.6f);
		entity0->rigid->angular_velocity = vec3(3.0f, 1.0f, 2.2f);
		entity0->rigid->flags.gravity = true;
		entity0->rigid->flags.translational_friction_flag = true;
		entity0->rigid->flags.rotational_friction_flag = true;
		entity0->rigid->impact_index = SND_GIB1;
	}

	{
		Entity *entity1 = engine->entity_list[engine->get_entity()];
		entity1->rigid = new EntRigidBody(entity1);
		entity1->model = entity1->rigid;
		entity1->position = camera_frame.pos;
		camera_frame.set(entity1->model->morientation);

		entity1->rigid->clone(model_table[MODEL_GIB1]);
		entity1->rigid->velocity = vec3(0.8f, 1.2f, -1.2f);
		entity1->rigid->angular_velocity = vec3(1.0f, 1.6f, 2.0f);
		entity1->rigid->flags.gravity = true;
		entity1->rigid->flags.translational_friction_flag = true;
		entity1->rigid->flags.rotational_friction_flag = true;
		entity1->rigid->impact_index = SND_GIB2;
	}

	{
		Entity *entity2 = engine->entity_list[engine->get_entity()];

		entity2->rigid = new EntRigidBody(entity2);
		entity2->model = entity2->rigid;
		entity2->position = camera_frame.pos;
		camera_frame.set(entity2->model->morientation);

		entity2->rigid->clone(model_table[MODEL_GIB2]);
		entity2->rigid->velocity = vec3(0.5f, 2.0f, 0.2f);
		entity2->rigid->angular_velocity = vec3(-2.0f, 1.0f, 6.0f);
		entity2->rigid->flags.gravity = true;
		entity2->rigid->flags.translational_friction_flag = true;
		entity2->rigid->flags.rotational_friction_flag = true;
		entity2->rigid->impact_index = SND_GIB3;
	}

	{
		Entity *entity3 = engine->entity_list[engine->get_entity()];

		entity3->rigid = new EntRigidBody(entity3);
		entity3->model = entity3->rigid;
		entity3->position = camera_frame.pos;
		camera_frame.set(entity3->model->morientation);

		entity3->rigid->clone(model_table[MODEL_GIB3]);
		entity3->rigid->velocity = vec3(-2.0f, 3.2f, 1.2f);
		entity3->rigid->angular_velocity = vec3(3.0f, -4.0f, 2.0f);
		entity3->rigid->flags.gravity = true;
		entity3->rigid->flags.rotational_friction_flag = true;
		entity3->rigid->flags.translational_friction_flag = true;
		entity3->rigid->impact_index = SND_GIB1;
	}

	{
		Entity *entity4 = engine->entity_list[engine->get_entity()];

		entity4->rigid = new EntRigidBody(entity4);
		entity4->model = entity4->rigid;
		entity4->position = camera_frame.pos;
		camera_frame.set(entity4->model->morientation);

		entity4->rigid->clone(model_table[MODEL_GIB4]);
		entity4->rigid->velocity = vec3(-1.25f, 1.7f, 1.27f);
		entity4->rigid->angular_velocity = vec3(3.0f, -4.0f, 2.0f);
		entity4->rigid->flags.gravity = true;
		entity4->rigid->flags.rotational_friction_flag = true;
		entity4->rigid->flags.translational_friction_flag = true;
		entity4->rigid->impact_index = SND_GIB2;
	}

	{
		Entity *entity5 = engine->entity_list[engine->get_entity()];

		entity5->rigid = new EntRigidBody(entity5);
		entity5->model = entity5->rigid;
		entity5->position = camera_frame.pos;
		camera_frame.set(entity5->model->morientation);

		entity5->rigid->clone(model_table[MODEL_GIB5]);
		entity5->rigid->velocity = vec3(-1.45f, 3.7f, 1.72f);
		entity5->rigid->angular_velocity = vec3(3.0f, -4.0f, 2.0f);
		entity5->rigid->flags.gravity = true;
		entity5->rigid->flags.rotational_friction_flag = true;
		entity5->rigid->flags.translational_friction_flag = true;
		entity5->rigid->impact_index = SND_GIB1;
	}

	{
		Entity *entity6 = engine->entity_list[engine->get_entity()];

		entity6->rigid = new EntRigidBody(entity6);
		entity6->model = entity6->rigid;
		entity6->position = camera_frame.pos;
		camera_frame.set(entity6->model->morientation);

		entity6->rigid->clone(model_table[MODEL_GIB6]);
		entity6->rigid->velocity = vec3(-1.15f, 1.7f, 1.37f);
		entity6->rigid->angular_velocity = vec3(3.0f, -4.0f, 2.0f);
		entity6->rigid->flags.gravity = true;
		entity6->rigid->flags.translational_friction_flag = true;
		entity6->rigid->flags.rotational_friction_flag = true;
		entity6->rigid->impact_index = SND_GIB2;
	}

	{
		Entity *entity7 = engine->entity_list[engine->get_entity()];

		entity7->rigid = new EntRigidBody(entity7);
		entity7->model = entity7->rigid;
		entity7->position = camera_frame.pos;
		camera_frame.set(entity7->model->morientation);

		entity7->rigid->clone(model_table[MODEL_GIB7]);
		entity7->rigid->velocity = vec3(-1.45f, 2.34f, 1.27f);
		entity7->rigid->angular_velocity = vec3(3.0f, -4.0f, 2.0f);
		entity7->rigid->flags.gravity = true;
		entity7->rigid->flags.translational_friction_flag = true;
		entity7->rigid->flags.rotational_friction_flag = true;
		entity7->rigid->impact_index = SND_GIB3;
	}

	{
		Entity *entity8 = engine->entity_list[engine->get_entity()];

		entity8->rigid = new EntRigidBody(entity8);
		entity8->model = entity8->rigid;
		entity8->position = camera_frame.pos;
		camera_frame.set(entity8->model->morientation);

		entity8->rigid->clone(model_table[MODEL_GIB8]);
		entity8->rigid->velocity = vec3(-1.85f, 1.73f, 2.32f);
		entity8->rigid->angular_velocity = vec3(3.0f, -4.0f, 2.0f);
		entity8->rigid->flags.gravity = true;
		entity8->rigid->flags.translational_friction_flag = true;
		entity8->rigid->flags.rotational_friction_flag = true;
		entity8->rigid->impact_index = SND_GIB1;
	}

	{
		Entity *entity9 = engine->entity_list[engine->get_entity()];

		entity9->rigid = new EntRigidBody(entity9);
		entity9->model = entity9->rigid;
		entity9->position = camera_frame.pos;
		camera_frame.set(entity9->model->morientation);

		entity9->rigid->clone(model_table[MODEL_GIB9]);
		entity9->rigid->velocity = vec3(1.45f, 1.27f, -1.2f);
		entity9->rigid->angular_velocity = vec3(3.0f, -4.0f, 2.0f);
		entity9->rigid->flags.gravity = true;
		entity9->rigid->flags.translational_friction_flag = true;
		entity9->rigid->flags.rotational_friction_flag = true;
		entity9->rigid->impact_index = SND_GIB2;
	}

}

///=============================================================================
/// Function: load_icon
///=============================================================================
/// Description: Loads 2d hud icons
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::load_icon()
{
	icon_t icon;

#define ICON_SELECT 0
	sprintf(icon.filename, "media/gfx/2d/select.tga");
	icon.x = 0.0f; // positive right from center
	icon.y = -0.125f; // positive up from center
	icon_list.push_back(icon);

#define ICON_GAUNTLET 1
	sprintf(icon.filename, "media/icons/iconw_gauntlet.tga");
	icon.x = 0.0f; // positive right from center
	icon.y = -0.125f; // positive up from center
	icon_list.push_back(icon);

#define ICON_MACHINEGUN 2
	sprintf(icon.filename, "media/icons/iconw_machinegun.tga");
	icon.x = 0.0f; // positive right from center
	icon.y = -0.125f; // positive up from center
	icon_list.push_back(icon);

#define ICON_SHOTGUN 3
	sprintf(icon.filename, "media/icons/iconw_shotgun.tga");
	icon.x = 0.0f; // positive right from center
	icon.y = -0.125f; // positive up from center
	icon_list.push_back(icon);


#define ICON_GRENADE 4
	sprintf(icon.filename, "media/icons/iconw_grenade.tga");
	icon.x = 0.0f; // positive right from center
	icon.y = -0.125f; // positive up from center
	icon_list.push_back(icon);

#define ICON_ROCKET 5
	sprintf(icon.filename, "media/icons/iconw_rocket.tga");
	icon.x = 0.0f; // positive right from center
	icon.y = -0.125f; // positive up from center
	icon_list.push_back(icon);

#define ICON_LIGHTNING 6
	sprintf(icon.filename, "media/icons/iconw_lightning.tga");
	icon.x = 0.0f; // positive right from center
	icon.y = -0.125f; // positive up from center
	icon_list.push_back(icon);

#define ICON_RAILGUN 7
	sprintf(icon.filename, "media/icons/iconw_railgun.tga");
	icon.x = 0.0f; // positive right from center
	icon.y = -0.125f; // positive up from center
	icon_list.push_back(icon);

#define ICON_PLASMA 8
	sprintf(icon.filename, "media/icons/iconw_plasma.tga");
	icon.x = 0.0f; // positive right from center
	icon.y = -0.125f; // positive up from center
	icon_list.push_back(icon);

#define ICON_BFG 9
	sprintf(icon.filename, "media/icons/iconw_bfg.tga");
	icon.x = 0.0f; // positive right from center
	icon.y = -0.125f; // positive up from center
	icon_list.push_back(icon);

#define ICON_GRAPPLE 10
	sprintf(icon.filename, "media/icons/iconw_grapple.tga");
	icon.x = 0.0f; // positive right from center
	icon.y = -0.125f; // positive up from center
	icon_list.push_back(icon);

#define ICON_MACHINEGUN_BOX 11
	sprintf(icon.filename, "media/icons/icona_machinegun.tga");
	icon.x = 0.0f;
	icon.y = -10.0f;
	icon_list.push_back(icon);

#define ICON_SHOTGUN_BOX 12
	sprintf(icon.filename, "media/icons/icona_shotgun.tga");
	icon.x = 0.0f;
	icon.y = -10.0f;
	icon_list.push_back(icon);

#define ICON_GRENADE_BOX 13
	sprintf(icon.filename, "media/icons/icona_grenade.tga");
	icon.x = 0.0f;
	icon.y = -10.0f;
	icon_list.push_back(icon);

#define ICON_ROCKET_BOX 14
	sprintf(icon.filename, "media/icons/icona_rocket.tga");
	icon.x = 0.0f;
	icon.y = -10.0f;
	icon_list.push_back(icon);

#define ICON_PLASMA_BOX 15
	sprintf(icon.filename, "media/icons/icona_plasma.tga");
	icon.x = 0.0f;
	icon.y = -10.0f;
	icon_list.push_back(icon);

#define ICON_LIGHTNING_BOX 16
	sprintf(icon.filename, "media/icons/icona_lightning.tga");
	icon.x = 0.0f;
	icon.y = -10.0f;
	icon_list.push_back(icon);

#define ICON_BFG_BOX 17
	sprintf(icon.filename, "media/icons/icona_bfg.tga");
	icon.x = 0.0f;
	icon.y = -10.0f;
	icon_list.push_back(icon);


#define ICON_SUIT 18
	sprintf(icon.filename, "media/icons/envirosuit.tga");
	icon.x = 0.0f; // positive right from center
	icon.y = 0.125f; // positive up from center
	icon_list.push_back(icon);

#define ICON_FLIGHT 19
	sprintf(icon.filename, "media/icons/flight.tga");
	icon.x = 0.0f; // positive right from center
	icon.y = 0.125f; // positive up from center
	icon_list.push_back(icon);

#define ICON_HASTE 20
	sprintf(icon.filename, "media/icons/haste.tga");
	icon.x = 0.0f; // positive right from center
	icon.y = 0.125f; // positive up from center
	icon_list.push_back(icon);

#define ICON_BLUE_FLAG 21
	sprintf(icon.filename, "media/icons/iconf_blu.tga");
	icon.x = 0.0f; // positive right from center
	icon.y = 0.125f; // positive up from center
	icon_list.push_back(icon);

#define ICON_RED_FLAG 22
	sprintf(icon.filename, "media/icons/iconf_red.tga");
	icon.x = 0.0f; // positive right from center
	icon.y = 0.125f; // positive up from center
	icon_list.push_back(icon);

#define ICON_MEGA 23
	sprintf(icon.filename, "media/icons/iconh_mega.tga");
	icon.x = 0.0f; // positive right from center
	icon.y = 0.125f; // positive up from center
	icon_list.push_back(icon);

#define ICON_RED_HEALTH 24
	sprintf(icon.filename, "media/icons/iconh_red.tga");
	icon.x = 0.0f; // positive right from center
	icon.y = 0.125f; // positive up from center
	icon_list.push_back(icon);

#define ICON_YELLOW_HEALTH 25
	sprintf(icon.filename, "media/icons/iconh_yellow.tga");
	icon.x = 0.0f; // positive right from center
	icon.y = 0.125f; // positive up from center
	icon_list.push_back(icon);

#define ICON_RED_ARMOR 26
	sprintf(icon.filename, "media/icons/iconr_red.tga");
	icon.x = 0.0f; // positive right from center
	icon.y = 0.125f; // positive up from center
	icon_list.push_back(icon);

#define ICON_ARMOR_SHARD 27
	sprintf(icon.filename, "media/icons/iconr_shard.tga");
	icon.x = 0.0f; // positive right from center
	icon.y = 0.125f; // positive up from center
	icon_list.push_back(icon);

#define ICON_YELLOW_ARMOR 28
	sprintf(icon.filename, "media/icons/iconr_yellow.tga");
	icon.x = 0.0f; // positive right from center
	icon.y = 0.125f; // positive up from center
	icon_list.push_back(icon);

#define ICON_INVIS 29
	sprintf(icon.filename, "media/icons/invis.tga");
	icon.x = 0.0f; // positive right from center
	icon.y = 0.125f; // positive up from center
	icon_list.push_back(icon);

#define ICON_MEDIKIT 30
	sprintf(icon.filename, "media/icons/medkit.tga");
	icon.x = 0.0f; // positive right from center
	icon.y = 0.125f; // positive up from center
	icon_list.push_back(icon);

#define ICON_NOAMMO 31
	sprintf(icon.filename, "media/icons/noammo.tga");
	icon.x = 0.0f; // positive right from center
	icon.y = -0.125f; // positive up from center
	icon_list.push_back(icon);

#define ICON_QUAD 32
	sprintf(icon.filename, "media/icons/quad.tga");
	icon.x = 0.0f; // positive right from center
	icon.y = 0.125f; // positive up from center
	icon_list.push_back(icon);

#define ICON_REGEN 33
	sprintf(icon.filename, "media/icons/regen.tga");
	icon.x = 0.0f; // positive right from center
	icon.y = 0.125f; // positive up from center
	icon_list.push_back(icon);

#define ICON_TELEPORTER 34
	sprintf(icon.filename, "media/icons/teleporter.tga");
	icon.x = 0.0f; // positive right from center
	icon.y = 0.125f; // positive up from center
	icon_list.push_back(icon);

#define ICON_MEDAL_ACCURACY 35
	sprintf(icon.filename, "media/menu/medals/medal_accuracy.tga");
	icon.x = 0.0f; // positive right from center
	icon.y = 0.125f; // positive up from center
	icon_list.push_back(icon);

#define ICON_MEDAL_EXCELLENT 36
	sprintf(icon.filename, "media/menu/medals/medal_excellent.tga");
	icon.x = 0.0f; // positive right from center
	icon.y = 0.125f; // positive up from center
	icon_list.push_back(icon);

#define ICON_MEDAL_FRAGS 37
	sprintf(icon.filename, "media/menu/medals/medal_frags.tga");
	icon.x = 0.0f; // positive right from center
	icon.y = 0.125f; // positive up from center
	icon_list.push_back(icon);

#define ICON_MEDAL_GAUNTLET 38
	sprintf(icon.filename, "media/menu/medals/medal_gauntlet.tga");
	icon.x = 0.0f; // positive right from center
	icon.y = 0.125f; // positive up from center
	icon_list.push_back(icon);

#define ICON_MEDAL_IMPRESSIVE 39
	sprintf(icon.filename, "media/menu/medals/medal_impressive.tga");
	icon.x = 0.0f; // positive right from center
	icon.y = 0.125f; // positive up from center
	icon_list.push_back(icon);

#define ICON_MEDAL_VICTORY 40
	sprintf(icon.filename, "media/menu/medals/medal_victory.tga");
	icon.x = 0.0f; // positive right from center
	icon.y = 0.125f; // positive up from center
	icon_list.push_back(icon);

#define FLASH_X 0.122f
#define FLASH_Y -0.127f

#define FACE_X	 0.0f
#define FACE_Y	-0.4f


#define ICON_F_MACHINEGUN 41
	sprintf(icon.filename, "media/models/weapons2/machinegun/f_machinegun.jpg");
	icon.x = FLASH_X; // positive right from center
	icon.y = FLASH_Y; // positive up from center
	icon_list.push_back(icon);

#define ICON_F_SHOTGUN 42
	sprintf(icon.filename, "media/models/weapons2/shotgun/f_shotgun.jpg");
	icon.x = FLASH_X; // positive right from center
	icon.y = FLASH_Y; // positive up from center
	icon_list.push_back(icon);

#define ICON_F_GRENADE 43
	sprintf(icon.filename, "media/models/weapons2/grenadel/f_grenadel.jpg");
	icon.x = FLASH_X; // positive right from center
	icon.y = FLASH_Y; // positive up from center
	icon_list.push_back(icon);

#define ICON_F_ROCKET 44
	sprintf(icon.filename, "media/models/weapons2/rocketl/f_rocketl.jpg");
	icon.x = FLASH_X; // positive right from center
	icon.y = FLASH_Y; // positive up from center
	icon_list.push_back(icon);

#define ICON_F_PLASMA 45
	sprintf(icon.filename, "media/models/weapons2/plasma/f_plasma.jpg");
	icon.x = FLASH_X; // positive right from center
	icon.y = FLASH_Y; // positive up from center
	icon_list.push_back(icon);

#define ICON_F_LIGHTNING 46
	sprintf(icon.filename, "media/models/weapons2/lightning/f_lightning.jpg");
	icon.x = FLASH_X; // positive right from center
	icon.y = FLASH_Y; // positive up from center
	icon_list.push_back(icon);

#define ICON_F_RAILGUN 47
	sprintf(icon.filename, "media/models/weapons2/railgun/f_railgun2.jpg");
	icon.x = FLASH_X; // positive right from center
	icon.y = FLASH_Y; // positive up from center
	icon_list.push_back(icon);


#define ICON_FACE0 48
	sprintf(icon.filename, "media/gfx/face0.png");
	icon.x = FACE_X; // positive right from center
	icon.y = FACE_Y; // positive up from center
	icon_list.push_back(icon);

#define ICON_FACE0_PAIN 49
	sprintf(icon.filename, "media/gfx/face0_pain.png");
	icon.x = FACE_X; // positive right from center
	icon.y = FACE_Y; // positive up from center
	icon_list.push_back(icon);

#define ICON_FACE1 50
	sprintf(icon.filename, "media/gfx/face1.png");
	icon.x = FACE_X; // positive right from center
	icon.y = FACE_Y; // positive up from center
	icon_list.push_back(icon);

#define ICON_FACE1_PAIN 51
	sprintf(icon.filename, "media/gfx/face1_pain.png");
	icon.x = FACE_X; // positive right from center
	icon.y = FACE_Y; // positive up from center
	icon_list.push_back(icon);

#define ICON_FACE2 52
	sprintf(icon.filename, "media/gfx/face2.png");
	icon.x = FACE_X; // positive right from center
	icon.y = FACE_Y; // positive up from center
	icon_list.push_back(icon);

#define ICON_FACE3 53
	sprintf(icon.filename, "media/gfx/face3.png");
	icon.x = FACE_X; // positive right from center
	icon.y = FACE_Y; // positive up from center
	icon_list.push_back(icon);

#define ICON_FACE3_PAIN 54
	sprintf(icon.filename, "media/gfx/face3_pain.png");
	icon.x = FACE_X; // positive right from center
	icon.y = FACE_Y; // positive up from center
	icon_list.push_back(icon);

#define ICON_FACE_QUAD 55
	sprintf(icon.filename, "media/gfx/face_quad.png");
	icon.x = FACE_X; // positive right from center
	icon.y = FACE_Y; // positive up from center
	icon_list.push_back(icon);

#define ICON_FACE_PENT 56
	sprintf(icon.filename, "media/gfx/face_pent.png");
	icon.x = FACE_X; // positive right from center
	icon.y = FACE_Y; // positive up from center
	icon_list.push_back(icon);

#define ICON_FACE_PENT_KILL 57
	sprintf(icon.filename, "media/gfx/face_pent_kill.png");
	icon.x = FACE_X; // positive right from center
	icon.y = FACE_Y; // positive up from center
	icon_list.push_back(icon);

#define ICON_F_GAUNTLET 58
	sprintf(icon.filename, "media/models/weapons2/gauntlet/gauntlet3.jpg");
	icon.x = FLASH_X; // positive right from center
	icon.y = -0.15f; // positive up from center
	icon_list.push_back(icon);

#define ICON_RLBOOM1 59
	sprintf(icon.filename, "media/models/weaphits/rlboom/rlboom_1.jpg");
	icon.x = 0.0f; // positive right from center
	icon.y = 0.0f; // positive up from center
	icon_list.push_back(icon);

#define ICON_RLBOOM2 60
	sprintf(icon.filename, "media/models/weaphits/rlboom/rlboom_2.jpg");
	icon.x = 0.0f; // positive right from center
	icon.y = 0.0f; // positive up from center
	icon_list.push_back(icon);

#define ICON_RLBOOM3 61
	sprintf(icon.filename, "media/models/weaphits/rlboom/rlboom_3.jpg");
	icon.x = 0.0f; // positive right from center
	icon.y = 0.0f; // positive up from center
	icon_list.push_back(icon);

#define ICON_RLBOOM4 62
	sprintf(icon.filename, "media/models/weaphits/rlboom/rlboom_4.jpg");
	icon.x = 0.0f; // positive right from center
	icon.y = 0.0f; // positive up from center
	icon_list.push_back(icon);

#define ICON_RLBOOM5 63
	sprintf(icon.filename, "media/models/weaphits/rlboom/rlboom_5.jpg");
	icon.x = 0.0f; // positive right from center
	icon.y = 0.0f; // positive up from center
	icon_list.push_back(icon);

#define ICON_RLBOOM6 64
	sprintf(icon.filename, "media/models/weaphits/rlboom/rlboom_6.jpg");
	icon.x = 0.0f; // positive right from center
	icon.y = 0.0f; // positive up from center
	icon_list.push_back(icon);

#define ICON_RLBOOM7 65
	sprintf(icon.filename, "media/models/weaphits/rlboom/rlboom_7.jpg");
	icon.x = 0.0f; // positive right from center
	icon.y = 0.0f; // positive up from center
	icon_list.push_back(icon);

#define ICON_RLBOOM8 66
	sprintf(icon.filename, "media/models/weaphits/rlboom/rlboom_8.jpg");
	icon.x = 0.0f; // positive right from center
	icon.y = 0.0f; // positive up from center
	icon_list.push_back(icon);



}

///=============================================================================
/// Function: handle_weapons
///=============================================================================
/// Description: Handles player weapon (calls into sub weapon functions)
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::handle_weapons(EntPlayer &player, input_t &input, int self, bool client)
{
	static bool once = false;
	static bool playing = false;
	bool fired = false;
	bool empty = false;
	int attack_sound = -1;

	if (hold_fire)
	{
		if (round_time >= 10)
		{
			for (unsigned int i = 0; i < engine->max_player; i++)
			{
				if (engine->entity_list[i]->player)
				{
					memset(&engine->entity_list[i]->player->stats, 0, sizeof(stats_t));
				}
			}
			engine->input.scores = false;
			hold_fire = false;
			console(-1, "reset 0", engine->menu, engine->entity_list);
		}
		player.flash_gauntlet = 0;
		player.flash_machinegun = 0;
		player.flash_shotgun = 0;
		player.flash_grenade = 0;
		player.flash_rocket = 0;
		player.flash_lightning = 0;
		player.flash_railgun = 0;
		player.flash_plasma = 0;
		return;
	}

	if (input.weapon_up)
	{
		if (self != -1)
		{
			player.change_weapon_up();
		}
		input.weapon_up = false;
	}

	if (input.weapon_down)
	{
		if (self != -1)
		{
			player.change_weapon_down();
		}
		input.weapon_down = false;
	}

	// Only reset flag when they stop clicking for lightning gun
	if (engine->input.attack == false)
	{
		if (once && (player.current_weapon & WEAPON_LIGHTNING))
		{
//			engine->audio.stop(player.entity->speaker->loop_source);
		}

		once = false;
	}

	if (player.reload_timer > 0)
	{
		player.reload_timer--;
	}

	if (weapon_switch_timer > 0)
	{
		weapon_switch_timer--;
	}

	if (player.flash_gauntlet > 0)
		player.flash_gauntlet--;
	if (player.flash_machinegun > 0)
		player.flash_machinegun--;
	if (player.flash_shotgun > 0)
		player.flash_shotgun--;
	if (player.flash_grenade > 0)
		player.flash_grenade--;
	if (player.flash_rocket > 0)
		player.flash_rocket--;
	if (player.flash_railgun > 0)
		player.flash_railgun--;
	if (player.flash_lightning > 0)
		player.flash_lightning--;
	if (player.flash_plasma > 0)
		player.flash_plasma--;

	if (player.health <= 0)
	{
		if (player.local)
		{
			engine->mlight2.set_contrast(-1.0f);
			player.state = PLAYER_DEAD;
		}
		return;
	}
	else
	{
		// network clients might get stuck black and white
		engine->mlight2.set_contrast(old_contrast);
	}

	if (player.current_weapon != player.last_weapon)
	{
		weapon_switch_timer = 2 * TICK_RATE;
		switch (player.current_weapon)
		{
		case wp_railgun:
			player.weapon_loop_source = engine->play_wave_global_loop(SND_RG_HUM);
			break;
		default:
			if (player.weapon_loop_source != -1)
				engine->audio.stop(player.weapon_loop_source);
			break;
		}
//		engine->audio.stop(player.entity->speaker->loop_source);
//		player.entity->speaker->loop_gain(0.25f);

		if (player.spawned)
		{
			if (player.local)
				engine->play_wave_global(SND_SWAPWEAP);
			else
				engine->play_wave(player.entity->position, SND_SWAPWEAP);
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

	if ((input.attack &&
		(player.type == PLAYER || player.type == CLIENT) &&
		player.reload_timer <= 0) ||
		((player.bot_state == BOT_ATTACK) && (player.reload_timer <= 0)))
	{
		if (player.current_weapon == wp_rocket)
		{
			attack_sound = SND_ROCKET;
			if (player.ammo_rockets > 0)
			{
				fired = true;
				handle_rocketlauncher(player, NULL, self, client);
			}
			else
			{
				empty = true;
			}
		}
		else if (player.current_weapon == wp_plasma)
		{
			attack_sound = SND_PLASMA;
			if (player.ammo_plasma > 0)
			{
				fired = true;
				handle_plasma(player, self, client);
			}
			else
			{
				empty = true;
			}
		}
		else if (player.current_weapon == wp_grenade)
		{
			attack_sound = SND_GRENADE;

			if (player.ammo_grenades > 0)
			{
				fired = true;
				handle_grenade(player, self, client);
			}
			else
			{
				empty = true;
			}

		}
		else if (player.current_weapon == wp_lightning)
		{
			attack_sound = SND_LIGHTNING;

			if (player.ammo_lightning > 0)
			{
				fired = true;
				handle_lightning(player, self, client);
			}
			else
			{
				empty = true;
			}

		}
		else if (player.current_weapon == wp_railgun)
		{
			attack_sound = SND_RAILGUN;

			if (player.ammo_slugs > 0)
			{
				fired = true;
				handle_railgun(player, self, client);
			}
			else
			{
				empty = true;
			}

		}
		else if (player.current_weapon == wp_shotgun)
		{
			attack_sound = SND_SHOTGUN;

			if (player.ammo_shells > 0)
			{
				fired = true;
				handle_shotgun(player, self, client);
			}
			else
			{
				empty = true;
			}

		}
		else if (player.current_weapon == wp_machinegun)
		{
			attack_sound = SND_MACHINEGUN;

			if (player.ammo_bullets > 0)
			{
				fired = true;
				handle_machinegun(player, NULL, self, client);
			}
			else
			{
				empty = true;
			}
		}
		else if (player.current_weapon == wp_gauntlet)
		{
			attack_sound = SND_GAUNTLET;
			fired = true;
			handle_gauntlet(player, self, client);
		}

		if (fired)
		{
			player.state = PLAYER_ATTACK;
			player.stats.shots++;

			if (player.current_weapon != WEAPON_RAILGUN)
				player.impressive_count = 0;

			switch (player.current_weapon)
			{
			case wp_gauntlet:
				player.flash_gauntlet = 5;
				break;
			case wp_machinegun:
				player.flash_machinegun = 5;
				break;
			case wp_shotgun:
				player.flash_shotgun = 5;
				break;
			case wp_grenade:
				player.flash_grenade = 5;
				break;
			case wp_rocket:
				player.flash_rocket = 5;
				break;
			case wp_railgun:
				player.flash_railgun = 5;
				break;
			case wp_lightning:
				player.flash_lightning = 5;
				break;
			case wp_plasma:
				player.flash_plasma = 5;
				break;
			}

			if (player.current_weapon & WEAPON_LIGHTNING)
			{
				if (once == false)
				{
					if (player.local)
						engine->play_wave_global(SND_LIGHTNING);
					else
						engine->play_wave(player.entity->position, SND_LIGHTNING);

					//weapon_idle_sound = SND_LG_HUM;

					/*
					engine->audio.stop(player.entity->speaker->loop_source);
					player.entity->speaker->loop_gain(0.25f);
					if (player.weapon_idle_sound[0] != '\0')
					{
						if (player.local)
							engine->play_wave_global(player.weapon_idle_sound);
						else
							engine->play_wave(player.entity->position, player.weapon_idle_sound);
					}
					*/

					once = true;
				}
			}
			else if (player.current_weapon & WEAPON_GAUNTLET)
			{
				if (playing == false)
				{
					if (player.local)
						player.weapon_loop_source = engine->play_wave_global_loop(attack_sound);
					else
						player.weapon_loop_source = engine->play_wave_loop(player.entity->position, attack_sound);
					playing = true;
				}
			}
			else
			{
				if (player.local)
					player.weapon_source = engine->play_wave_global(attack_sound);
				else
					player.weapon_source = engine->play_wave(player.entity->position, attack_sound);
			}
		}
		else if (empty)
		{
			player.reload_timer = 30;

			if (player.local)
				engine->play_wave_global(SND_NOAMMO);
			else
				engine->play_wave(player.entity->position, SND_NOAMMO);
		}
	}

	if ((player.current_weapon & WEAPON_GAUNTLET) && input.attack == false)
	{
		if (player.weapon_loop_source != -1)
			engine->audio.stop(player.weapon_loop_source);

		playing = false;
	}

}

///=============================================================================
/// Function: draw_flash
///=============================================================================
/// Description: Draws first person muzzle flash
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::draw_flash(EntPlayer *player)
{
	engine->gfx.Blend(true);
	engine->gfx.BlendFuncOneOne();
	if (player->flash_gauntlet)
		draw_icon(15.0, ICON_F_GAUNTLET, 0.0f, -0.1f, -0.5f);
	else if (player->flash_machinegun)
		draw_icon(15.0, ICON_F_MACHINEGUN, 0.0f, -0.1f, -0.5f);
	else if (player->flash_shotgun)
		draw_icon(15.0, ICON_F_SHOTGUN, 0.0f, -0.1f, -0.5f);
	else if (player->flash_grenade)
		draw_icon(15.0, ICON_F_GRENADE, 0.0f, -0.1f, -0.5f);
	else if (player->flash_rocket)
		draw_icon(15.0, ICON_F_ROCKET, 0.0f, -0.1f, -0.5f);
	else if (player->flash_lightning)
		draw_icon(15.0, ICON_F_LIGHTNING, 0.0f, -0.1f, -0.5f);
	else if (player->flash_railgun)
		draw_icon(15.0, ICON_F_RAILGUN, 0.0f, -0.1f, -0.5f);
	else if (player->flash_plasma)
		draw_icon(15.0, ICON_F_PLASMA, 0.0f, -0.1f, -0.5f);
	engine->gfx.Blend(false);
}

///=============================================================================
/// Function: render_hud
///=============================================================================
/// Description: Draws first person heads up display
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::render_hud(double last_frametime)
{
	matrix4 real_projection = engine->projection;
	char msg[LINE_SIZE];
	vec3 color(1.0f, 1.0f, 1.0f);



	int spawn = engine->find_type(ENT_PLAYER, 0);
	if (spawn == -1)
	{
		snprintf(msg, LINE_SIZE, "Spectating");
		engine->menu.draw_text(msg, 0.1f, 0.95f, 0.050f, color, true, true);
		return;
	}

	Entity *entity = engine->entity_list[spawn];
	if (entity->player == NULL)
	{
		return;
	}

	engine->projection = engine->identity;


	if (chat_timer > 0 || engine->menu.chatmode)
	{
		engine->menu.render_chat(engine->global);
		chat_timer--;
	}

	if (notif_timer > 0)
	{
		engine->menu.render_notif(engine->global);
		notif_timer--;
	}

	if (faceicon)
	{
		if (entity->player->quad_timer)
		{
			draw_icon(4.0f, ICON_FACE_QUAD, 0.0f, 0.0f);
		}
		else if (entity->player->godmode)
		{
			if (entity->player->pain_timer)
				draw_icon(4.0f, ICON_FACE_PENT_KILL, 0.0f, 0.0f);
			else
				draw_icon(4.0f, ICON_FACE_PENT, 0.0f, 0.0f);
		}
		else if (entity->player->health > 80)
		{
			if (entity->player->pain_timer)
				draw_icon(4.0f, ICON_FACE0_PAIN, 0.0f, 0.0f);
			else
				draw_icon(4.0f, ICON_FACE0, 0.0f, 0.0f);
		}
		else if (entity->player->health > 60)
		{
			if (entity->player->pain_timer)
				draw_icon(4.0f, ICON_FACE1_PAIN, 0.0f, 0.0f);
			else
				draw_icon(4.0f, ICON_FACE1, 0.0f, 0.0f);
		}
		else if (entity->player->health > 40)
		{
			draw_icon(4.0f, ICON_FACE2, 0.0f, 0.0f);
		}
		else if (entity->player->health > 0)
		{
			if (entity->player->pain_timer)
				draw_icon(4.0f, ICON_FACE3_PAIN, 0.0f, 0.0f);
			else
				draw_icon(4.0f, ICON_FACE3, 0.0f, 0.0f);
		}
	}

	engine->menu.draw_text("", 0.15f, 0.95f, 0.050f, color, true, false);

	if (engine->show_hud)
	{
		if (spawn != -1)
		{
			if (warmup)
			{
				if (win_timer > 0)
				{
					engine->menu.draw_text(win_msg, 0.35f, 0.25f, 0.050f, color, false, false);
					win_timer--;
				}
				else
				{
					snprintf(msg, LINE_SIZE, "Warmup: %d", warmup_time - round_time);
					engine->menu.draw_text(msg, 0.35f, 0.25f, 0.050f, color, false, false);
				}
			}
			else
			{
				if (round_time <= 1)
				{
					snprintf(msg, LINE_SIZE, "Fight!");
					engine->menu.draw_text(msg, 0.45f, 0.25f, 0.050f, color, false, false);
				}
			}


			if (entity->player->health > 50)
			{
				snprintf(msg, LINE_SIZE, "%d/%d", entity->player->health, entity->player->armor);
				engine->menu.draw_text(msg, 0.15f, 0.95f, 0.050f, color, false, false);
			}
			else if (entity->player->health <= 50 && blink)
			{
				vec3 red = vec3(1.0f, 0.0f, 0.0f);
				snprintf(msg, LINE_SIZE, "%d/%d", entity->player->health, entity->player->armor);
				engine->menu.draw_text(msg, 0.15f, 0.95f, 0.050f, red, false, false);
			}
			else
			{
				snprintf(msg, LINE_SIZE, "%d/%d", entity->player->health, entity->player->armor);
				engine->menu.draw_text(msg, 0.15f, 0.95f, 0.050f, color, false, false);
			}

			switch (entity->player->current_weapon)
			{
			case wp_machinegun:
				snprintf(msg, LINE_SIZE, "%d", entity->player->ammo_bullets);
				engine->menu.draw_text(msg, 0.7f, 0.95f, 0.050f, color, false, false);
				break;
			case wp_shotgun:
				snprintf(msg, LINE_SIZE, "%d", entity->player->ammo_shells);
				engine->menu.draw_text(msg, 0.7f, 0.95f, 0.050f, color, false, false);
				break;
			case wp_grenade:
				snprintf(msg, LINE_SIZE, "%d", entity->player->ammo_grenades);
				engine->menu.draw_text(msg, 0.7f, 0.95f, 0.050f, color, false, false);
				break;
			case wp_rocket:
				snprintf(msg, LINE_SIZE, "%d", entity->player->ammo_rockets);
				engine->menu.draw_text(msg, 0.7f, 0.95f, 0.050f, color, false, false);
				break;
			case wp_railgun:
				snprintf(msg, LINE_SIZE, "%d", entity->player->ammo_slugs);
				engine->menu.draw_text(msg, 0.7f, 0.95f, 0.050f, color, false, false);
				break;
			case wp_lightning:
				snprintf(msg, LINE_SIZE, "%d", entity->player->ammo_lightning);
				engine->menu.draw_text(msg, 0.7f, 0.95f, 0.050f, color, false, false);
				break;
			case wp_plasma:
				snprintf(msg, LINE_SIZE, "%d", entity->player->ammo_plasma);
				engine->menu.draw_text(msg, 0.7f, 0.95f, 0.050f, color, false, false);
				break;
			}
		}

		engine->global.Select();
	}

	if (engine->input.scores)
	{
		int line = 1;


		if (gametype == GAMETYPE_CTF)
			snprintf(msg, LINE_SIZE, "Scores: Red Team Score %d/%d Blue Team Score %d/%d",
				red_flag_caps, capturelimit,
				blue_flag_caps, capturelimit);
		else
			snprintf(msg, LINE_SIZE, "Scores: Fraglimit %d Timelimit %d Round Time %d:%02d", fraglimit, timelimit, round_time / 60, round_time % 60);
		engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color, true, false);


		for (unsigned int i = 0; i < engine->max_player; i++)
		{
			float accuracy = 0.0f;
			EntPlayer *p = engine->entity_list[i]->player;


			bool player = (p && p->type == PLAYER);
			bool bot = (p && p->type == BOT);

			if (!(bot || player))
				continue;

			if (p->stats.shots != 0)
				accuracy = 100.0f * p->stats.hits / p->stats.shots;

			snprintf(msg, LINE_SIZE, "%-32s %d kills, %d deaths, %3.1f%% accuracy",
				p->name,
				p->stats.kills,
				p->stats.deaths,
				accuracy);
			engine->menu.draw_text(msg, 0.05f, 0.025f * line++, 0.025f, color, false, false);
		}

	}


	if (engine->entity_list[spawn]->player->in_vehicle != -1)
	{
		EntVehicle *vehicle = engine->entity_list[engine->entity_list[spawn]->player->in_vehicle]->vehicle;

		int line = 1;
		line++;
		snprintf(msg, LINE_SIZE, "position: %3.3f %3.3f %3.3f", entity->position.x, entity->position.y, entity->position.z);
		engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color, true, false);

		line++;
		snprintf(msg, LINE_SIZE, "velocity: %3.3f %3.3f %3.3f", entity->rigid->velocity.x, entity->rigid->velocity.y, entity->rigid->velocity.z);
		engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color, true, false);

		line++;
		snprintf(msg, LINE_SIZE, "RPM: %f", vehicle->rpm);
		engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color, true, false);
		line++;
		snprintf(msg, LINE_SIZE, "Gear: %d", vehicle->gear);
		engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color, true, false);
		line++;
		snprintf(msg, LINE_SIZE, "Steer angle: %f", vehicle->steerangle);
		engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color, true, false);
		line++;
		snprintf(msg, LINE_SIZE, "Throttle %f", vehicle->throttle);
		engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color, true, false);
		line++;
		snprintf(msg, LINE_SIZE, "Brake %f", vehicle->brake);
		engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color, true, false);
		line++;
		snprintf(msg, LINE_SIZE, "Speed %f", vehicle->velocity.magnitude());
		engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color, true, false);
		line++;
		snprintf(msg, LINE_SIZE, "Angle: %d", (int)(vehicle->angle_rad * 180.0f / M_PI) % 360);
		engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color, true, false);

	}

	if (engine->show_debug)
	{
		int line = 1;

		snprintf(msg, LINE_SIZE, "Debug Messages: lastframe %.2f ms %.2f fps", last_frametime, 1000.0 / last_frametime);
		engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color, true, false);
		snprintf(msg, LINE_SIZE, "%d active lights.", (int)engine->light_list.size());
		engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color, false, false);
		if (spawn != -1)
		{
			line++;
			snprintf(msg, LINE_SIZE, "position: %3.3f %3.3f %3.3f", entity->position.x, entity->position.y, entity->position.z);
			engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color, false, false);
			snprintf(msg, LINE_SIZE, "velocity: %3.3f %3.3f %3.3f", entity->rigid->velocity.x, entity->rigid->velocity.y, entity->rigid->velocity.z);
			engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color, false, false);
			snprintf(msg, LINE_SIZE, "Speed: %3.3f", entity->rigid->velocity.magnitude());
			engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color, false, false);
			snprintf(msg, LINE_SIZE, "Water: %d depth %lf", entity->rigid->flags.water, entity->rigid->water_depth);
			engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color, false, false);
			snprintf(msg, LINE_SIZE, "drawcalls: %d triangles %d", engine->gfx.gpustat.drawcall, engine->gfx.gpustat.triangle);
			engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color, false, false);
			snprintf(msg, LINE_SIZE, "buffer mb: %d texture mb %d", engine->gfx.gpustat.buffer_size / (1024 * 1024), engine->gfx.gpustat.texture_size / (1024 * 1024));
			engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color, false, false);

#ifdef SERIAL
			snprintf(msg, LINE_SIZE, "ping: %d delta %d size %d/%d num_ents %d dropped %d qsize %d",
				engine->netcode.netinfo.ping,
				engine->netcode.netinfo.sequence_delta,
				engine->netcode.netinfo.size,
				engine->netcode.netinfo.uncompressed_size,
				engine->netcode.netinfo.num_ents,
				engine->netcode.netinfo.dropped,
				engine->netcode.recv_queue.size);
#else
			snprintf(msg, LINE_SIZE, "ping: %d delta %d size %d/%d num_ents %d dropped %d",
				engine->netcode.netinfo.ping,
				engine->netcode.netinfo.sequence_delta,
				engine->netcode.netinfo.size,
				engine->netcode.netinfo.uncompressed_size,
				engine->netcode.netinfo.num_ents,
				engine->netcode.netinfo.dropped);
#endif
			engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color, false, false);
			snprintf(msg, LINE_SIZE, "send_full %d send_partial %d recv_empty %d",
				engine->netcode.netinfo.send_full,
				engine->netcode.netinfo.send_partial,
				engine->netcode.netinfo.recv_empty
			);
			engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color, false, false);

			snprintf(msg, LINE_SIZE, "bsp_platform %d on_ground %d impact velocity %f", entity->rigid->bsp_model_platform, entity->rigid->flags.on_ground, entity->rigid->impact_velocity );
			engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color, false, false);

			snprintf(msg, LINE_SIZE, "terrain height %f x: %d y: %d top %d", engine->terrain.height, engine->terrain.x_index, engine->terrain.y_index, engine->terrain.top);
			engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color, false, false);


			if (engine->netcode.server_flag)
			{
				for (unsigned int i = 0; i < engine->netcode.client_list.size(); i++)
				{
					client_t *client = engine->netcode.client_list[i];
					snprintf(msg, LINE_SIZE, "[client %d]", i );
					engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color, false, false);


					snprintf(msg, LINE_SIZE, "ping: %d delta %d size %d num_ents %d dropped %d",
						client->netinfo.ping,
						client->netinfo.sequence_delta,
						client->netinfo.size,
						client->netinfo.num_ents,
						client->netinfo.dropped);
					engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color, false, false);

					snprintf(msg, LINE_SIZE, "position delta %3.3f", client->position_delta.magnitude());
					engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color, false, false);

					snprintf(msg, LINE_SIZE, "velocity %3.3f %3.3f %3.3f",
						engine->entity_list[client->ent_id]->rigid->velocity.x,
						engine->entity_list[client->ent_id]->rigid->velocity.y, 
						engine->entity_list[client->ent_id]->rigid->velocity.z);
					engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color, false, false);

					snprintf(msg, LINE_SIZE, "position %3.3f %3.3f %3.3f",
						engine->entity_list[client->ent_id]->position.x,
						engine->entity_list[client->ent_id]->position.y,
						engine->entity_list[client->ent_id]->position.z);
					engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color, false, false);


				}

			}

		}
	}

	if (engine->show_names)
	{
		for (unsigned int i = 0; i < engine->entity_list.size(); i++)
		{
			Entity *ent = engine->entity_list[i];
			if (ent->rigid == NULL)
				continue;

			if (ent->flags.visible && ent->flags.nodraw == false)
			{
				draw_name(ent, engine->menu, real_projection, i);
			}
		}
	}

	for (unsigned int i = 0; i < engine->max_player; i++)
	{
		Entity *ent = engine->entity_list[i];

		if (ent->player == NULL)
			continue;

		if (ent->player->type != BOT)
			continue;

		if (ent->flags.visible && ent->flags.nodraw == false)
		{
			draw_name(ent, engine->menu, real_projection, i);
		}
	}


	if (engine->show_lines)
	{
		vec3 color(1.0f, 1.0f, 1.0f);

		for (unsigned int i = 0; i < engine->entity_list.size(); i++)
		{
//			if (engine->entity_list[i]->nodraw == true)
//				continue;

			if ( strlen(engine->entity_list[i]->entstring->target_name) <= 1 )
				continue;

			for (unsigned int j = 0; j < engine->entity_list.size(); j++)
			{
//				if (engine->entity_list[j]->nodraw == true)
	//				continue;

				if (strlen(engine->entity_list[j]->entstring->target) <= 1)
					continue;

				if (strstr(engine->entity_list[i]->entstring->target_name, engine->entity_list[j]->entstring->target) != NULL)
				{
					draw_line(engine->entity_list[i], engine->entity_list[j], engine->menu, color);
				}
			}

		}
	}

	engine->menu.draw_text("", 0.01f, 0.025f, 0.025f, color, false, false);
	engine->projection = real_projection;

	engine->gfx.Blend(true);
	engine->gfx.BlendFuncSrcAlphaOneMinusSrcAlpha();
	draw_crosshair();
	int i = 1;
	int j = -1;

	if (weapon_switch_timer > 0)
	{
#define WEAPON_SPACING 0.1f
		switch (entity->player->current_weapon)
		{
		case wp_none:
			draw_icon(1.0, ICON_NOAMMO);
			break;
		case wp_gauntlet:
			draw_icon(1.0, ICON_GAUNTLET);
			if (entity->player->weapon_flags & WEAPON_MACHINEGUN)
				draw_icon(1.0, ICON_MACHINEGUN, WEAPON_SPACING * i++, 0.0f);
			if (entity->player->weapon_flags & WEAPON_SHOTGUN)
				draw_icon(1.0, ICON_SHOTGUN, WEAPON_SPACING * i++, 0.0f);
			if (entity->player->weapon_flags & WEAPON_GRENADE)
				draw_icon(1.0, ICON_GRENADE, WEAPON_SPACING * i++, 0.0f);
			if (entity->player->weapon_flags & WEAPON_ROCKET)
				draw_icon(1.0, ICON_ROCKET, WEAPON_SPACING * i++, 0.0f);
			if (entity->player->weapon_flags & WEAPON_LIGHTNING)
				draw_icon(1.0, ICON_LIGHTNING, WEAPON_SPACING * i++, 0.0f);
			if (entity->player->weapon_flags & WEAPON_RAILGUN)
				draw_icon(1.0, ICON_RAILGUN, WEAPON_SPACING * i++, 0.0f);
			if (entity->player->weapon_flags & WEAPON_PLASMA)
				draw_icon(1.0, ICON_PLASMA, WEAPON_SPACING * i++, 0.0f);

			break;
		case wp_machinegun:
			if (entity->player->weapon_flags & WEAPON_GAUNTLET)
				draw_icon(1.0, ICON_GAUNTLET, WEAPON_SPACING * j--, 0.0f);
			draw_icon(1.0, ICON_MACHINEGUN);
			if (entity->player->weapon_flags & WEAPON_SHOTGUN)
				draw_icon(1.0, ICON_SHOTGUN, WEAPON_SPACING * i++, 0.0f);
			if (entity->player->weapon_flags & WEAPON_GRENADE)
				draw_icon(1.0, ICON_GRENADE, WEAPON_SPACING * i++, 0.0f);
			if (entity->player->weapon_flags & WEAPON_ROCKET)
				draw_icon(1.0, ICON_ROCKET, WEAPON_SPACING * i++, 0.0f);
			if (entity->player->weapon_flags & WEAPON_LIGHTNING)
				draw_icon(1.0, ICON_LIGHTNING, WEAPON_SPACING * i++, 0.0f);
			if (entity->player->weapon_flags & WEAPON_RAILGUN)
				draw_icon(1.0, ICON_RAILGUN, WEAPON_SPACING * i++, 0.0f);
			if (entity->player->weapon_flags & WEAPON_PLASMA)
				draw_icon(1.0, ICON_PLASMA, WEAPON_SPACING * i++, 0.0f);
			break;
		case wp_shotgun:
			if (entity->player->weapon_flags & WEAPON_MACHINEGUN)
				draw_icon(1.0, ICON_MACHINEGUN, WEAPON_SPACING * j--, 0.0f);
			if (entity->player->weapon_flags & WEAPON_GAUNTLET)
				draw_icon(1.0, ICON_GAUNTLET, WEAPON_SPACING * j--, 0.0f);
			draw_icon(1.0, ICON_SHOTGUN);
			if (entity->player->weapon_flags & WEAPON_GRENADE)
				draw_icon(1.0, ICON_GRENADE, WEAPON_SPACING * i++, 0.0f);
			if (entity->player->weapon_flags & WEAPON_ROCKET)
				draw_icon(1.0, ICON_ROCKET, WEAPON_SPACING * i++, 0.0f);
			if (entity->player->weapon_flags & WEAPON_LIGHTNING)
				draw_icon(1.0, ICON_LIGHTNING, WEAPON_SPACING * i++, 0.0f);
			if (entity->player->weapon_flags & WEAPON_RAILGUN)
				draw_icon(1.0, ICON_RAILGUN, WEAPON_SPACING * i++, 0.0f);
			if (entity->player->weapon_flags & WEAPON_PLASMA)
				draw_icon(1.0, ICON_PLASMA, WEAPON_SPACING * i++, 0.0f);
			break;
		case wp_grenade:
			if (entity->player->weapon_flags & WEAPON_SHOTGUN)
				draw_icon(1.0, ICON_SHOTGUN, WEAPON_SPACING * j--, 0.0f);
			if (entity->player->weapon_flags & WEAPON_MACHINEGUN)
				draw_icon(1.0, ICON_MACHINEGUN, WEAPON_SPACING * j--, 0.0f);
			if (entity->player->weapon_flags & WEAPON_GAUNTLET)
				draw_icon(1.0, ICON_GAUNTLET, WEAPON_SPACING * j--, 0.0f);
			draw_icon(1.0, ICON_GRENADE);
			if (entity->player->weapon_flags & WEAPON_ROCKET)
				draw_icon(1.0, ICON_ROCKET, WEAPON_SPACING * i++, 0.0f);
			if (entity->player->weapon_flags & WEAPON_LIGHTNING)
				draw_icon(1.0, ICON_LIGHTNING, WEAPON_SPACING * i++, 0.0f);
			if (entity->player->weapon_flags & WEAPON_RAILGUN)
				draw_icon(1.0, ICON_RAILGUN, WEAPON_SPACING * i++, 0.0f);
			if (entity->player->weapon_flags & WEAPON_PLASMA)
				draw_icon(1.0, ICON_PLASMA, WEAPON_SPACING * i++, 0.0f);
			break;
		case wp_rocket:
			if (entity->player->weapon_flags & WEAPON_GRENADE)
				draw_icon(1.0, ICON_GRENADE, WEAPON_SPACING * j--, 0.0f);
			if (entity->player->weapon_flags & WEAPON_SHOTGUN)
				draw_icon(1.0, ICON_SHOTGUN, WEAPON_SPACING * j--, 0.0f);
			if (entity->player->weapon_flags & WEAPON_MACHINEGUN)
				draw_icon(1.0, ICON_MACHINEGUN, WEAPON_SPACING * j--, 0.0f);
			if (entity->player->weapon_flags & WEAPON_GAUNTLET)
				draw_icon(1.0, ICON_GAUNTLET, WEAPON_SPACING * j--, 0.0f);
			draw_icon(1.0, ICON_ROCKET);
			if (entity->player->weapon_flags & WEAPON_LIGHTNING)
				draw_icon(1.0, ICON_LIGHTNING, WEAPON_SPACING * i++, 0.0f);
			if (entity->player->weapon_flags & WEAPON_RAILGUN)
				draw_icon(1.0, ICON_RAILGUN, WEAPON_SPACING * i++, 0.0f);
			if (entity->player->weapon_flags & WEAPON_PLASMA)
				draw_icon(1.0, ICON_PLASMA, WEAPON_SPACING * i++, 0.0f);
			break;
		case wp_lightning:
			if (entity->player->weapon_flags & WEAPON_ROCKET)
				draw_icon(1.0, ICON_ROCKET, WEAPON_SPACING * j--, 0.0f);
			if (entity->player->weapon_flags & WEAPON_GRENADE)
				draw_icon(1.0, ICON_GRENADE, WEAPON_SPACING * j--, 0.0f);
			if (entity->player->weapon_flags & WEAPON_SHOTGUN)
				draw_icon(1.0, ICON_SHOTGUN, WEAPON_SPACING * j--, 0.0f);
			if (entity->player->weapon_flags & WEAPON_MACHINEGUN)
				draw_icon(1.0, ICON_MACHINEGUN, WEAPON_SPACING * j--, 0.0f);
			if (entity->player->weapon_flags & WEAPON_GAUNTLET)
				draw_icon(1.0, ICON_GAUNTLET, WEAPON_SPACING * j--, 0.0f);
			draw_icon(1.0, ICON_LIGHTNING);
			if (entity->player->weapon_flags & WEAPON_RAILGUN)
				draw_icon(1.0, ICON_RAILGUN, WEAPON_SPACING * i++, 0.0f);
			if (entity->player->weapon_flags & WEAPON_PLASMA)
				draw_icon(1.0, ICON_PLASMA, WEAPON_SPACING * i++, 0.0f);
			break;
		case wp_railgun:
			if (entity->player->weapon_flags & WEAPON_LIGHTNING)
				draw_icon(1.0, ICON_LIGHTNING, WEAPON_SPACING * j--, 0.0f);
			if (entity->player->weapon_flags & WEAPON_ROCKET)
				draw_icon(1.0, ICON_ROCKET, WEAPON_SPACING * j--, 0.0f);
			if (entity->player->weapon_flags & WEAPON_GRENADE)
				draw_icon(1.0, ICON_GRENADE, WEAPON_SPACING * j--, 0.0f);
			if (entity->player->weapon_flags & WEAPON_SHOTGUN)
				draw_icon(1.0, ICON_SHOTGUN, WEAPON_SPACING * j--, 0.0f);
			if (entity->player->weapon_flags & WEAPON_MACHINEGUN)
				draw_icon(1.0, ICON_MACHINEGUN, WEAPON_SPACING * j--, 0.0f);
			if (entity->player->weapon_flags & WEAPON_GAUNTLET)
				draw_icon(1.0, ICON_GAUNTLET, WEAPON_SPACING * j--, 0.0f);
			draw_icon(1.0, ICON_RAILGUN);
			if (entity->player->weapon_flags & WEAPON_PLASMA)
				draw_icon(1.0, ICON_PLASMA, WEAPON_SPACING * i++, 0.0f);
			break;
		case wp_plasma:
			if (entity->player->weapon_flags & WEAPON_RAILGUN)
				draw_icon(1.0, ICON_RAILGUN, WEAPON_SPACING * j--);
			if (entity->player->weapon_flags & WEAPON_LIGHTNING)
				draw_icon(1.0, ICON_LIGHTNING, WEAPON_SPACING * j--, 0.0f);
			if (entity->player->weapon_flags & WEAPON_ROCKET)
				draw_icon(1.0, ICON_ROCKET, WEAPON_SPACING * j--, 0.0f);
			if (entity->player->weapon_flags & WEAPON_GRENADE)
				draw_icon(1.0, ICON_GRENADE, WEAPON_SPACING * j--, 0.0f);
			if (entity->player->weapon_flags & WEAPON_SHOTGUN)
				draw_icon(1.0, ICON_SHOTGUN, WEAPON_SPACING * j--, 0.0f);
			if (entity->player->weapon_flags & WEAPON_MACHINEGUN)
				draw_icon(1.0, ICON_MACHINEGUN, WEAPON_SPACING * j--, 0.0f);
			if (entity->player->weapon_flags & WEAPON_GAUNTLET)
				draw_icon(1.0, ICON_GAUNTLET, WEAPON_SPACING * j--, 0.0f);
			draw_icon(1.0, ICON_PLASMA);
			break;
		}
		draw_icon(1.0, ICON_SELECT);
	}

	if (entity->player->quad_timer)
	{
		draw_icon(1.0, ICON_QUAD);
	}

	if (entity->player->excellent_award_timer > 0)
	{
		draw_icon(1.0, ICON_MEDAL_EXCELLENT);
		entity->player->excellent_award_timer--;
	}

	if (entity->player->gauntlet_award_timer > 0)
	{
		draw_icon(1.0, ICON_MEDAL_GAUNTLET);
		entity->player->gauntlet_award_timer--;
	}

	if (entity->player->impressive_award_timer > 0)
	{
		draw_icon(1.0, ICON_MEDAL_IMPRESSIVE);
		entity->player->impressive_award_timer--;
	}

	if (entity->player->regen_timer)
	{
		draw_icon(1.0, ICON_REGEN);
	}

	if (entity->player->flight_timer)
	{
		draw_icon(1.0, ICON_FLIGHT);
	}

	if (entity->player->invisibility_timer)
	{
		draw_icon(1.0, ICON_INVIS);
	}

	if (entity->player->haste_timer)
	{
		draw_icon(1.0, ICON_HASTE);
	}

	if (entity->player->holdable_medikit)
	{
		draw_icon(1.0, ICON_MEDIKIT);
	}

	if (entity->player->holdable_teleporter)
	{
		draw_icon(1.0, ICON_TELEPORTER);
	}

}

///=============================================================================
/// Function: draw_name
///=============================================================================
/// Description: Draws player name / info near their 3d model in 3d space
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::draw_name(Entity *entity, Menu &menu, matrix4 &real_projection, int ent_num)
{
	matrix4 trans2;
	matrix4 mvp2;
	matrix4 model;
	vec3 color(1.0f, 1.0f, 1.0f);


	menu.draw_text("", 0, 0, 0.02f, color, true, false);
	engine->camera_frame.set(trans2);
	entity->rigid->get_matrix(model.m);

	mvp2 = (model * trans2) * real_projection;

	vec4 pos = mvp2 * vec4(0.0f, 0.0f, 0.0f, 1.0f); // model space coordinate

	if (abs32(pos.w) < 0.00001f)
	{
		return;
	}

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


		if (engine->show_names)
		{
			if (entity->entstring)
			{
				sprintf(data, "%s", entity->entstring->type);
				menu.draw_text(entity->entstring->type, pos.x, pos.y - 0.0625f, 0.02f, color, false, false);
			}
			sprintf(data, "bsp_leaf: %d", entity->bsp_leaf);
			menu.draw_text(data, pos.x, pos.y, 0.02f, color, false, false);
		}

		if (entity->ent_type == ENT_UNKNOWN)
		{
			int line = 1;

			sprintf(data, "Pos %.3f %.3f %.3f", entity->position.x, entity->position.y, entity->position.z);
			menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, color, false, false);
			sprintf(data, "Vel %.3f %.3f %.3f", entity->rigid->velocity.x, entity->rigid->velocity.y, entity->rigid->velocity.z);
			menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, color, false, false);
			sprintf(data, "State %d", entity->rigid->flags.sleep);
			menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, color, false, false);
		}


		if (entity->ent_type == ENT_FUNC_BOBBING || entity->ent_type == ENT_FUNC_TRAIN)
		{
			int line = 1;

			sprintf(data, "Target %.3f %.3f %.3f",	entity->rigid->path.path_list[entity->rigid->path.index].x,
													entity->rigid->path.path_list[entity->rigid->path.index].y,
													entity->rigid->path.path_list[entity->rigid->path.index].z );
			menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, color, false, false);

			sprintf(data, "path_index %d", entity->rigid->path.index);
			menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, color, false, false);
			sprintf(data, "num_path %d", entity->rigid->path.num_path);
			menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, color, false, false);

		}

		if (entity->ent_type == ENT_SENTRY)
		{
			int line = 1;

			sprintf(data, "Health %d", entity->construct->health);
			menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, color, false, false);
			sprintf(data, "Bullets %d", entity->construct->ammo_bullets);
			menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, color, false, false);
			sprintf(data, "Rockets %d", entity->construct->ammo_rockets);
			menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, color, false, false);

		}

		if (entity->ent_type == ENT_NAVPOINT)
		{
			vec3 blue(0.0f, 0.0f, 1.0f);
			vec3 green(0.0f, 1.0f, 0.0f);
			int line = 1;

			sprintf(data, "targetname %s", entity->entstring->target_name);
			menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.025f, blue, false, false);
			sprintf(data, "target %s", entity->entstring->target);
			menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.025f, green, false, false);
		}

		if (entity->ent_type == ENT_LIGHT)
		{
			int line = 1;

			if (entity->light != NULL)
			{
				sprintf(data, "ent num %d", ent_num);
				menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, color, false, false);

				sprintf(data, "intensity %f", entity->light->intensity);
				menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, color, false, false);

				sprintf(data, "color %.3f %.3f %.3f", entity->light->color.x, entity->light->color.y, entity->light->color.z);
				menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, color, false, false);
			}
		}

		if (entity->player && entity->player->type == BOT)
		{
			int line = 1;
			vec3 red(1.0f, 0.0f, 0.0f);
			vec3 white(1.0f, 1.0f, 1.0f);

			sprintf(data, "Health %d", entity->player->health);
			menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, red, false, false);

			if (engine->show_names)
			{
				sprintf(data, "Bot State %s", EntPlayer::bot_state_name[entity->player->bot_state]);
				menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, red, false, false);

				if (entity->player->bot_state == BOT_GET_ITEM)
				{
					sprintf(data, "Item: %s", engine->entity_list[entity->player->get_item]->entstring->type);
					menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, white, false, false);
				}
			}
		}

		if (entity->ent_type == ENT_FUNC_PLAT || entity->ent_type == ENT_FUNC_BOBBING ||
			entity->ent_type == ENT_FUNC_TRAIN)
		{
			int line = 1;

			if (entity->light != NULL)
			{
				sprintf(data, "target %s", entity->entstring->target);
				menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, color, false, false);
				sprintf(data, "target_name %s", entity->entstring->target_name);
				menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, color, false, false);
			}
		}

		if (entity->ent_type == ENT_FUNC_CLOTH)
		{
			int line = 1;

			if (entity->light != NULL)
			{
				sprintf(data, "ent_num %d", ent_num);
				menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, color, false, false);
			}
		}
	}
	menu.draw_text("", pos.x, pos.y + 0.0625f, 0.02f, color, false, true);
	engine->projection = real_projection;
}


///=============================================================================
/// Function: draw_line
///=============================================================================
/// Description: draws a line between two entities
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::draw_line(Entity *ent_a, Entity *ent_b, Menu &menu, vec3 &color)
{
	static vertex_t vertex[512];
	matrix4 trans2;
	matrix4 mvp2;
	matrix4 model;
	int index[512];
	int i;

	vec3 a = ent_a->position;
	vec3 b = ent_b->position;
	vec3 pos;

	for (i = 0; i < 50; i++)
	{
		lerp(a, b, i / 50.0f, pos);

		memset(&vertex[i], 0, sizeof(vertex_t));
		vertex[i].position = pos;
		vertex[i].color = 0x0000FF00;
		vertex[i].tangent.x = 2500.0f; //life
		vertex[i].tangent.y = 5.0f; //size
		vertex[i].tangent.z = -1.0f; //type
		index[i] = i;
	}

	int line_ibo = engine->gfx.CreateIndexBuffer(index, i);
	int line_vbo = engine->gfx.CreateVertexBuffer(vertex, i);

	matrix4 transformation;
	engine->camera_frame.set(transformation);
	matrix4 mvp = transformation * engine->projection;


	vec3 quad1 = engine->camera_frame.up;
	vec3 quad2 = vec3::crossproduct(engine->camera_frame.forward, engine->camera_frame.up);


	engine->particle_render.Select();
	engine->particle_render.Params(mvp, quad1, quad2, 0.0f, 0.0f, 0.0f);
	engine->gfx.SelectTexture(0, engine->particle_tex);
	engine->gfx.SelectIndexBuffer(line_ibo);
	engine->particle_render.render(engine->gfx, 0, line_vbo, 400);

	// yeah I know
	engine->gfx.DeleteIndexBuffer(line_ibo);
	engine->gfx.DeleteVertexBuffer(line_vbo);
}

///=============================================================================
/// Function: transform_3d_2d
///=============================================================================
/// Description: transforms 3d point (x,y,z) to equivalent 2d screen coordinate
/// (x, y)
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
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
	mvp2 = (matrix * trans2) * projection;

	vec4 pos_4d = mvp2 * vec4(0.0f, 0.0f, 0.0f, 1.0f); // model space coordinate

	pos2d.x = pos_4d.x / pos_4d.w;
	pos2d.y = -pos_4d.y / pos_4d.w;
	pos2d.z = pos_4d.z / pos_4d.w;

	pos2d.x = 0.5f + (pos2d.x * 0.5f);
	pos2d.y = 0.5f + (pos2d.y * 0.5f);
}

///=============================================================================
/// Function: create_crosshair
///=============================================================================
/// Description: creates crosshairs (10 different styles)
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::create_crosshair()
{
	vertex_t vert;
	int num_crosshair = 10;

	memset(&vert, 0, sizeof(vertex_t));
	vert.position = vec3(0.0f, 0.0f, -1.0f);
	vert.color = ~0;
	vert.tangent.x = 5.0f; //life
	vert.tangent.y = 5.0f; //size
	vert.tangent.z = -1.0f; //type

	crosshair_vbo = engine->gfx.CreateVertexBuffer(&vert, 1);

	for (int i = 0; i < num_crosshair; i++)
	{
		char filename[128];

		sprintf(filename, "media/gfx/2d/crosshair%c.tga", 'a' + i);
		crosshair_tex[i] = load_texture_pk3(engine->gfx, filename, engine->pk3_list, engine->num_pk3, true, false, 0);
	}
	current_crosshair = 0;
}




///=============================================================================
/// Function: draw_crosshair
///=============================================================================
/// Description: renders the crosshair on screen
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::draw_crosshair()
{
	matrix4 transformation;

	if (current_crosshair == -1)
		return;

	engine->camera_frame.set(transformation);

	//matrix4 mvp = transformation * engine->projection;
	float scale = crosshair_scale / 150.0f;

	vec3 quad1 = vec3(0.0f, scale, 0.0f);
	vec3 quad2 = vec3(scale, 0.0f, 0.0f);

#ifndef DIRECTX
	engine->particle_render.Select();
	engine->particle_render.Params(engine->projection, quad1, quad2, 0.0f, 0.0f, 0.0f);
	engine->gfx.SelectTexture(0, crosshair_tex[current_crosshair]);
	engine->particle_render.render(engine->gfx, 0, crosshair_vbo, 1);
#endif
}


///=============================================================================
/// Function: create_icon
///=============================================================================
/// Description: creates 2d icons
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::create_icon()
{
	static vertex_t vert[512];

	for (unsigned int i = 0; i < icon_list.size(); i++)
	{
		memset(&vert[i], 0, sizeof(vertex_t));
		vert[i].position = vec3(icon_list[i].x, icon_list[i].y, -1.0f);
		vert[i].color = ~0;
		vert[i].tangent.x = 10.0f; //life
		vert[i].tangent.y = 5.0f; //size
		vert[i].tangent.z = -1.0f; //type
		icon_list[i].tex = load_texture_pk3(engine->gfx, icon_list[i].filename, engine->pk3_list, engine->num_pk3, true, false, 0);

		if (icon_list[i].tex == 0)
		{
			icon_list[i].tex = load_texture(engine->gfx, icon_list[i].filename, false, false, 0);
		}

		if (icon_list[i].tex == 0)
		{
			printf("Failed to load %s\n", icon_list[i].filename);
		}
	}

	icon_vbo = engine->gfx.CreateVertexBuffer(&vert, icon_list.size());
}

///=============================================================================
/// Function: draw_icon
///=============================================================================
/// Description: draws 2d icons
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::draw_icon(float scale, int index, float x, float y, float z)
{
	matrix4 transformation;
	engine->camera_frame.set(transformation);

	//matrix4 mvp = transformation * engine->projection;
	scale /= 150.0f;

	vec3 quad1 = vec3(0.0f, scale, 0.0f);
	vec3 quad2 = vec3(scale, 0.0f, 0.0f);

#ifndef DIRECTX
	engine->gfx.SelectTexture(0, icon_list[index].tex);
	engine->particle_render.Select();
	engine->particle_render.Params(engine->projection, quad1, quad2, icon_list[index].x + x, icon_list[index].y + y, z);
	engine->particle_render.render(engine->gfx, index, icon_vbo, 1);
#endif
}




///=============================================================================
/// Function: bot_get_path
///=============================================================================
/// Description: Finds A* shortest path from bot to specific item on map
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
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
		Entity *ent = engine->entity_list[i];

		if (ent->ent_type == ENT_NAVPOINT)
		{
			float distance_self = (ent->position - self_pos).magnitude();
			float distance_target = (ent->position - target_pos).magnitude();

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


	int start_path = atoi(engine->entity_list[self_index]->entstring->target_name + 3);
	int end_path = atoi(engine->entity_list[target_index]->entstring->target_name + 3);

	if (start_path == end_path)
		return -1;

	engine->find_path(path.path, path.length, start_path, end_path);
	path.step = 0;
	return 0;
}

///=============================================================================
/// Function: bot_follow
///=============================================================================
/// Description: Has bot follow path to desired position in node order
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
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

///=============================================================================
/// Function: console
///=============================================================================
/// Description: console command handler, these will contain console commands
/// that require the game to be loaded and the player to exist
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::console(int self, char *cmd, Menu &menu, vector<Entity *> &entity_list)
{
	char msg[LINE_SIZE] = { 0 };
	char data[LINE_SIZE] = { 0 };
	char data2[LINE_SIZE] = { 0 };
	unsigned int ret;

	if (cmd[0] == '\0')
		return;

	ret = sscanf(cmd, "hurt %s %s", data, data2);
	if (ret == 2)
	{
		snprintf(msg, LINE_SIZE, "hurt %s %s\n", data, data2);
		menu.print(msg);

		unsigned int index = atoi(data);

		if (index >= entity_list.size())
		{
			debugf("hurt given invalid index\n");
			return;
		}

		if (entity_list[index]->player == NULL)
		{
			debugf("hurt given invalid index\n");
			return;
		}

		if (entity_list[index]->player->health <= 0)
			return;



		unsigned int damage = abs32(atoi(data2));
		unsigned int health_damage = damage / 3;
		unsigned int armor_damage = 2 * health_damage;
		EntPlayer *player = entity_list[index]->player;

		if (player->godmode)
			return;

		if (armor_damage > player->armor)
		{
			armor_damage -= player->armor;
			player->armor = 0;
			health_damage += armor_damage;
		}
		else
		{
			player->armor -= armor_damage;
		}

		player->health -= health_damage;


		if (player->pain_timer == 0)
		{
			if (player->health <= 25)
				engine->play_wave(entity_list[index]->position, player->model_index * SND_PLAYER + SND_PAIN1);
			else if (player->health <= 50)
				engine->play_wave(entity_list[index]->position, player->model_index * SND_PLAYER + SND_PAIN2);
			else if (player->health <= 75)
				engine->play_wave(entity_list[index]->position, player->model_index * SND_PLAYER + SND_PAIN3);
			else if (player->health <= 100)
				engine->play_wave(entity_list[index]->position, player->model_index * SND_PLAYER + SND_PAIN4);

			player->pain_timer = TICK_RATE >> 2;
		}

		return;
	}

	vec3 pos;
	ret = sscanf(cmd, "setpos %f %f %f %d", &pos.x, &pos.y, &pos.z, &self);
	if (ret == 4)
	{
		snprintf(msg, LINE_SIZE, "Setting position to %f %f %f for entity %d\n", pos.x, pos.y, pos.z, self);
		if (self >= 0 && self < entity_list.size() - 1)
			entity_list[self]->position = pos;
		return;
	}


	vec3 triangle[3];
	ret = sscanf(cmd, "debug_triangle (%f, %f, %f) (%f, %f, %f) (%f, %f, %f)",
		&triangle[0].x, &triangle[0].y, &triangle[0].z,
		&triangle[1].x, &triangle[1].y, &triangle[1].z,
		&triangle[2].x, &triangle[2].y, &triangle[2].z
	);
	if (ret == 9)
	{
		snprintf(msg, LINE_SIZE, "Debug triangle\n");

		debugf("debug_triangle (%f, %f, %f) (%f, %f, %f) (%f, %f, %f)\n",
			triangle[0].x, triangle[0].y, triangle[0].z,
			triangle[1].x, triangle[1].y, triangle[1].z,
			triangle[2].x, triangle[2].y, triangle[2].z);


		ret = engine->debug_triangle(triangle);

		if (ret == 0)
		{
			debugf("Success\n");
		}
		else
		{
			debugf("Failed to create GPU objects\n");
		}
		return;
	}

	vec3 vec_pos;
	vec3 vec_dir;
	ret = sscanf(cmd, "debug_vector (%f, %f, %f) (%f, %f, %f)",
		&vec_pos.x, &vec_pos.y, &vec_pos.z,
		&vec_dir.x, &vec_dir.y, &vec_dir.z
	);
	if (ret == 6)
	{
		snprintf(msg, LINE_SIZE, "Debug vector\n");

		debugf("debug_vector (%f, %f, %f) (%f, %f, %f)\n",
			vec_pos.x, vec_pos.y, vec_pos.z,
			vec_dir.x, vec_dir.y, vec_dir.z);

		ret = engine->debug_vector(vec_pos, vec_dir);

		if (ret == 0)
		{
			debugf("Success\n");
		}
		else
		{
			debugf("Failed to create GPU objects\n");
		}
		return;
	}
	vec3 point_pos;
	ret = sscanf(cmd, "debug_point (%f, %f, %f)",
		&point_pos.x, &point_pos.y, &point_pos.z
	);
	if (ret == 3)
	{
		snprintf(msg, LINE_SIZE, "Debug point\n");

		debugf("debug_point (%f, %f, %f)\n",
			point_pos.x, point_pos.y, point_pos.z);

		ret = engine->debug_point(point_pos);

		if (ret == 0)
		{
			debugf("Success\n");
		}
		else
		{
			debugf("Failed to create GPU objects\n");
		}
		return;
	}





	ret = sscanf(cmd, "setpos %f %f %f", &pos.x, &pos.y, &pos.z);
	if (ret == 3)
	{
		snprintf(msg, LINE_SIZE, "Setting position to %f %f %f\n", pos.x, pos.y, pos.z);
		if (self != -1)
			entity_list[self]->position = pos;
		else
			engine->camera_frame.pos = pos;
		return;
	}

	ret = sscanf(cmd, "raster_target %s", data);
	if (ret == 1)
	{
		raster_target = (int)atoi(data);
		snprintf(msg, LINE_SIZE, "Set to %d\n", raster_target);
		menu.print(msg);
		debugf(msg);
		return;
	}


	ret = sscanf(cmd, "raster_enabled %s", data);
	if (ret == 1)
	{
		raster_enabled = (int)atoi(data);
		snprintf(msg, LINE_SIZE, "Set to %d\n", raster_enabled);
		menu.print(msg);
		return;
	}


	ret = sscanf(cmd, "cg_crosshairsize %s", data);
	if (ret == 1)
	{
		crosshair_scale = (float)atof(data);
		snprintf(msg, LINE_SIZE, "Set to %f\n", crosshair_scale);
		menu.print(msg);
		return;
	}

	ret = sscanf(cmd, "cg_crosshair %s", data);
	if (ret == 1)
	{
		engine->menu.data.crosshair = (int)atoi(data);
		if (engine->menu.data.crosshair < -1)
			engine->menu.data.crosshair = -1;
		if (engine->menu.data.crosshair > 10)
			engine->menu.data.crosshair = 10;

		current_crosshair = engine->menu.data.crosshair;

		snprintf(msg, LINE_SIZE, "Set to %d\n", engine->menu.data.crosshair);
		menu.print(msg);
		return;
	}


	/*
	ret = sscanf(cmd, "pgain %s", data);
	if (ret == 1)
	{
		pid.pGain = (float)atof(data);
	}

	ret = sscanf(cmd, "igain %s", data);
	if (ret == 1)
	{
		pid.iGain = (float)atof(data);
	}

	ret = sscanf(cmd, "dgain %s", data);
	if (ret == 1)
	{
		pid.dGain = (float)atof(data);
	}
	*/

	ret = sscanf(cmd, "damage %s", data);
	if (ret == 1)
	{
		bool local = entity_list[self]->player->local;
		EntPlayer *player = entity_list[self]->player;

		snprintf(msg, LINE_SIZE, "damage %s\n", data);
		menu.print(msg);


		if (player->health <= 0)
			return;

		unsigned int damage = abs32(atoi(data));
		unsigned int health_damage = damage / 3;
		unsigned int armor_damage = 2 * health_damage;


		if (player->godmode)
			return;

		if (armor_damage > player->armor)
		{
			armor_damage -= player->armor;
			player->armor = 0;
			health_damage += armor_damage;
		}
		else
		{
			player->armor -= armor_damage;
		}

		player->health -= health_damage;

		if (player->pain_timer == 0)
		{
			if (player->health <= 25)
			{
				if (local)
					engine->play_wave_global(player->model_index * SND_PLAYER + SND_PAIN1);
				else
					engine->play_wave(entity_list[self]->position, player->model_index * SND_PLAYER + SND_PAIN1);
			}
			else if (player->health <= 50)
			{
				if (local)
					engine->play_wave_global(player->model_index * SND_PLAYER + SND_PAIN2);
				else
					engine->play_wave(entity_list[self]->position, player->model_index * SND_PLAYER + SND_PAIN2);
			}
			else if (player->health <= 75)
			{
				if (local)
					engine->play_wave_global(player->model_index * SND_PLAYER + SND_PAIN3);
				else
					engine->play_wave(entity_list[self]->position, player->model_index * SND_PLAYER + SND_PAIN3);
			}
			else if (player->health <= 200)
			{
				if (local)
					engine->play_wave_global(player->model_index * SND_PLAYER + SND_PAIN4);
				else
					engine->play_wave(entity_list[self]->position, player->model_index * SND_PLAYER + SND_PAIN4);
			}

			player->pain_timer = TICK_RATE >> 2;
		}

		return;
	}

	ret = sscanf(cmd, "health %s", data);
	if (ret == 1)
	{
		EntPlayer *player = entity_list[self]->player;
		snprintf(msg, LINE_SIZE, "health %s\n", data);
		menu.print(msg);
		player->health += atoi(data);
		if (player->health > 200)
		{
			player->health = 200;
		}

		return;
	}

	if (strstr(cmd, "flyby"))
	{
		flyby = true;
		char file[128];

		if (spectator == false)
			engine->console("spectate");

		sprintf(file, "media/%s.spline", engine->q3map.map_name);
		spline.load(file);
		return;
	}


	ret = sscanf(cmd, "armor %s", data);
	if (ret == 1)
	{
		snprintf(msg, LINE_SIZE, "armor %s\n", data);
		menu.print(msg);
		if (entity_list[self]->player->armor + atoi(data) <= 200)
		{
			entity_list[self]->player->armor += atoi(data);
		}
		else
		{
			entity_list[self]->player->armor = 200;
		}
		return;
	}

	if (strcmp(cmd, "weapon_grenadelauncher") == 0)
	{
		EntPlayer *player = entity_list[self]->player;
		snprintf(msg, LINE_SIZE, "weapon_grenadelauncher\n");
		menu.print(msg);

		weapon_switch_timer = 2 * TICK_RATE;
		if (player->current_weapon == wp_none)
			player->current_weapon = wp_grenade;

		player->weapon_flags |= wp_grenade;
		if (player->ammo_grenades > 10)
		{
			player->ammo_grenades++;
		}
		else
		{
			player->ammo_grenades = 10;
		}
		return;
	}

	if (strcmp(cmd, "weapon_rocketlauncher") == 0)
	{
		EntPlayer *player = entity_list[self]->player;

		snprintf(msg, LINE_SIZE, "weapon_rocketlauncher\n");
		menu.print(msg);

		weapon_switch_timer = 2 * TICK_RATE;
		if (player->current_weapon == wp_none)
			player->current_weapon = wp_rocket;

		player->weapon_flags |= wp_rocket;
		if (player->ammo_rockets > 10)
		{
			player->ammo_rockets++;
		}
		else
		{
			player->ammo_rockets = 10;
		}
		return;
	}

	if (strcmp(cmd, "weapon_shotgun") == 0)
	{
		EntPlayer *player = entity_list[self]->player;

		snprintf(msg, LINE_SIZE, "weapon_shotgun\n");
		menu.print(msg);

		weapon_switch_timer = 2 * TICK_RATE;
		if (player->current_weapon == wp_none)
			player->current_weapon = wp_shotgun;

		player->weapon_flags |= wp_shotgun;
		if (player->ammo_shells > 10)
		{
			player->ammo_shells++;
		}
		else
		{
			player->ammo_shells = 10;
		}
		return;
	}

	if (strcmp(cmd, "weapon_machinegun") == 0)
	{
		EntPlayer *player = entity_list[self]->player;

		snprintf(msg, LINE_SIZE, "weapon_machinegun\n");
		menu.print(msg);

		weapon_switch_timer = 2 * TICK_RATE;
		if (player->current_weapon == wp_none)
			player->current_weapon = wp_machinegun;

		player->weapon_flags |= wp_machinegun;

		if (player->ammo_bullets > 100)
		{
			player->ammo_bullets++;
		}
		else
		{
			player->ammo_bullets = 100;
		}
		return;
	}

	if (strstr(cmd, "weapon "))
	{
		int weapon = atoi(cmd + 7);

		if (self == -1)
			return;

		switch (weapon)
		{
		case 1:
			if (entity_list[self]->player->weapon_flags & WEAPON_GAUNTLET)	
				entity_list[self]->player->current_weapon = WEAPON_GAUNTLET;
			break;
		case 2:
			if (entity_list[self]->player->weapon_flags & WEAPON_MACHINEGUN)
				entity_list[self]->player->current_weapon = WEAPON_MACHINEGUN;
			break;
		case 3:
			if (entity_list[self]->player->weapon_flags & WEAPON_SHOTGUN)
				entity_list[self]->player->current_weapon = WEAPON_SHOTGUN;
			break;
		case 4:
			if (entity_list[self]->player->weapon_flags & WEAPON_GRENADE)
				entity_list[self]->player->current_weapon = WEAPON_GRENADE;
			break;
		case 5:
			if (entity_list[self]->player->weapon_flags & WEAPON_ROCKET)
				entity_list[self]->player->current_weapon = WEAPON_ROCKET;
			break;
		case 6:
			if (entity_list[self]->player->weapon_flags & WEAPON_LIGHTNING)
				entity_list[self]->player->current_weapon = WEAPON_LIGHTNING;
			break;
		case 7:
			if (entity_list[self]->player->weapon_flags & WEAPON_RAILGUN)
				entity_list[self]->player->current_weapon = WEAPON_RAILGUN;
			break;
		case 8:
			if (entity_list[self]->player->weapon_flags & WEAPON_PLASMA)
				entity_list[self]->player->current_weapon = WEAPON_PLASMA;
			break;
		default:
			break;
		}
		return;
	}

	if (strcmp(cmd, "weapon_lightning") == 0)
	{
		EntPlayer *player = entity_list[self]->player;

		snprintf(msg, LINE_SIZE, "weapon_lightning\n");
		menu.print(msg);

		weapon_switch_timer = 2 * TICK_RATE;
		if (player->current_weapon == wp_none)
			player->current_weapon = wp_lightning;

		player->weapon_flags |= wp_lightning;
		if (player->ammo_lightning > 100)
		{
			player->ammo_lightning++;
		}
		else
		{
			player->ammo_lightning = 100;
		}
		return;
	}

	if (strcmp(cmd, "weapon_railgun") == 0)
	{
		EntPlayer *player = entity_list[self]->player;

		snprintf(msg, LINE_SIZE, "weapon_railgun\n");
		menu.print(msg);

		weapon_switch_timer = 2 * TICK_RATE;
		if (player->current_weapon == wp_none)
			player->current_weapon = wp_railgun;

		player->weapon_flags |= wp_railgun;
		if (player->ammo_slugs > 10)
		{
			player->ammo_slugs++;
		}
		else
		{
			player->ammo_slugs = 10;
		}
		return;
	}

	if (strcmp(cmd, "weapon_plasma") == 0)
	{
		EntPlayer *player = entity_list[self]->player;
		snprintf(msg, LINE_SIZE, "weapon_plasma\n");
		menu.print(msg);

		weapon_switch_timer = 2 * TICK_RATE;
		if (player->current_weapon == wp_none)
			player->current_weapon = wp_plasma;

		player->weapon_flags |= wp_plasma;
		if (player->ammo_plasma > 50)
		{
			player->ammo_plasma++;
		}
		else
		{
			player->ammo_plasma = 50;
		}
		return;
	}

	ret = sscanf(cmd, "ammo_rockets %s", data);
	if (ret == 1)
	{
		snprintf(msg, LINE_SIZE, "ammo_rockets %s\n", data);
		menu.print(msg);
		entity_list[self]->player->ammo_rockets += atoi(data);
		return;
	}

	ret = sscanf(cmd, "ammo_grenades %s", data);
	if (ret == 1)
	{
		snprintf(msg, LINE_SIZE, "ammo_grenades %s\n", data);
		menu.print(msg);
		entity_list[self]->player->ammo_grenades += atoi(data);
		return;
	}

	ret = sscanf(cmd, "ammo_slugs %s", data);
	if (ret == 1)
	{
		snprintf(msg, LINE_SIZE, "ammo_slugs %s\n", data);
		menu.print(msg);
		entity_list[self]->player->ammo_slugs += atoi(data);
		return;
	}

	ret = sscanf(cmd, "ammo_shells %s", data);
	if (ret == 1)
	{
		snprintf(msg, LINE_SIZE, "ammo_shells %s\n", data);
		menu.print(msg);
		entity_list[self]->player->ammo_shells += atoi(data);
		return;
	}

	ret = sscanf(cmd, "ammo_bullets %s", data);
	if (ret == 1)
	{
		snprintf(msg, LINE_SIZE, "ammo_bullets %s\n", data);
		menu.print(msg);
		entity_list[self]->player->ammo_bullets += atoi(data);
		return;
	}

	ret = sscanf(cmd, "ammo_lightning %s", data);
	if (ret == 1)
	{
		snprintf(msg, LINE_SIZE, "ammo_lightning %s\n", data);
		menu.print(msg);
		entity_list[self]->player->ammo_lightning += atoi(data);
		return;
	}

	ret = sscanf(cmd, "ammo_plasma %s", data);
	if (ret == 1)
	{
		snprintf(msg, LINE_SIZE, "ammo_plasma %s\n", data);
		menu.print(msg);
		entity_list[self]->player->ammo_plasma += atoi(data);
		return;
	}

	ret = sscanf(cmd, "ammo_bfg %s", data);
	if (ret == 1)
	{
		snprintf(msg, LINE_SIZE, "ammo_bfg %s\n", data);
		menu.print(msg);
		entity_list[self]->player->ammo_bfg += atoi(data);
		return;
	}

	ret = strcmp(cmd, "blueflag");
	if (ret == 0)
	{
		EntPlayer *player = entity_list[self]->player;

		if (player->team == TEAM_RED && player->holdable_flag == false)
		{
			snprintf(msg, LINE_SIZE, "blueflag taken\n");
			menu.print(msg);
			player->holdable_flag = true;
		}
		return;
	}

	ret = strcmp(cmd, "redflag");
	if (ret == 0)
	{
		EntPlayer *player = entity_list[self]->player;
		if (player->team == TEAM_BLUE && player->holdable_flag == false)
		{
			snprintf(msg, LINE_SIZE, "redflag taken\n");
			menu.print(msg);
			player->holdable_flag = true;
		}
		return;
	}

	ret = strcmp(cmd, "holdable_teleporter");
	if (ret == 0)
	{
		if (entity_list[self]->player->holdable_medikit == false)
		{
			snprintf(msg, LINE_SIZE, "holdable_teleporter\n");
			menu.print(msg);
			entity_list[self]->player->holdable_teleporter = true;
		}
		return;
	}

	ret = strcmp(cmd, "holdable_medkit");
	if (ret == 0)
	{
		if (entity_list[self]->player->holdable_teleporter == false)
		{
			snprintf(msg, LINE_SIZE, "holdable_medkit\n");
			menu.print(msg);
			entity_list[self]->player->holdable_medikit = true;
		}
		return;
	}

	ret = strcmp(cmd, "teleport");
	if (ret == 0 && engine->netcode.client_flag == false)
	{
		bool local = entity_list[self]->player->local;

		// Find a spawn point
		for (unsigned int i = last_spawn; i < entity_list.size(); i++)
		{
			if (entity_list[i]->ent_type == ENT_INFO_PLAYER_DEATHMATCH ||
				entity_list[i]->ent_type == ENT_INFO_PLAYER_START)
			{
				matrix4 matrix;

				Entity *ent = entity_list[self];
				// Set position and orientation
				ent->position = ent->position + vec3(0.0f, 50.0f, 0.0f);
				if (ent->brushinfo)
				{
					switch (ent->brushinfo->angle)
					{
					case 0:
						matrix4::mat_left(matrix, ent->position);
						break;
					case 90:
						matrix4::mat_forward(matrix, ent->position);
						break;
					case 180:
						matrix4::mat_right(matrix, ent->position);
						break;
					case 270:
						matrix4::mat_backward(matrix, ent->position);
						break;
					default:
						matrix4::mat_forward(matrix, ent->position);
						break;
					}
				}
				else
				{
					matrix4::mat_forward(matrix, ent->position);
				}

				ent->model->morientation.m[0] = matrix.m[0];
				ent->model->morientation.m[1] = matrix.m[1];
				ent->model->morientation.m[2] = matrix.m[2];

				ent->model->morientation.m[3] = matrix.m[4];
				ent->model->morientation.m[4] = matrix.m[5];
				ent->model->morientation.m[5] = matrix.m[6];

				ent->model->morientation.m[6] = matrix.m[8];
				ent->model->morientation.m[7] = matrix.m[9];
				ent->model->morientation.m[8] = matrix.m[10];


				if (local)
				{
					// Set frame if player (or else frame will override model position)
					engine->camera_frame.up.x = matrix.m[4];
					engine->camera_frame.up.y = matrix.m[5];
					engine->camera_frame.up.z = matrix.m[6];
					engine->camera_frame.forward.x = matrix.m[8];
					engine->camera_frame.forward.y = matrix.m[9];
					engine->camera_frame.forward.z = matrix.m[10];
				}

				// Set last spawn position so we spawn in the next point
				last_spawn = i + 1;
				debugf("Teleporting on entity %d\n", i);

				// Play teleport sound
				if (local)
					engine->play_wave_global(SND_TELEIN);
				else
					engine->play_wave(ent->position, SND_TELEIN);

				break;
			}
		}
		return;
	}


	if (strstr(cmd, "kill"))
	{
		if (entity_list[self]->player)
		{
			snprintf(msg, LINE_SIZE, "killed\n");
			menu.print(msg);
			entity_list[self]->player->health = -100;
		}
	}

	ret = sscanf(cmd, "team %s", data);
	if (ret == 1)
	{
		if (strcmp(data, "red") == 0)
		{
			entity_list[self]->player->team = TEAM_RED;
			engine->console("respawn");
		}
		else if (strcmp(data, "blue") == 0)
		{
			entity_list[self]->player->team = TEAM_BLUE;
			engine->console("respawn");
		}
		else
		{
			debugf("Invalid team: [red, blue]");
		}
		return;
	}

	ret = sscanf(cmd, "seat %s", data);
	if (ret == 1)
	{
		entity_list[self]->player->seat = atoi(data);
		return;
	}

	if (strstr(cmd, "spectate"))
	{
		spectator = !spectator;
		debugf("spectator is %d\n", (int)spectator);

		if (spectator == false)
		{
			float min_distance = FLT_MAX;
			int index = -1;

			for (unsigned int i = 0; i < engine->max_player; i++)
			{
				float distance = (engine->camera_frame.pos - engine->entity_list[i]->position).magnitude();

				if (distance < min_distance)
				{

					if (engine->entity_list[i]->player)
					{
						min_distance = distance;
						index = i;
					}
				}
			}
			int spec = engine->find_type(ENT_SPECTATOR, 0);

			if (spec != -1)
			{
				engine->entity_list[spec]->ent_type = ENT_NPC;
			}
			engine->entity_list[index]->ent_type = ENT_PLAYER;
		}
		else
		{
			int spawn = engine->find_type(ENT_PLAYER, 0);

			engine->entity_list[spawn]->ent_type = ENT_SPECTATOR;
		}
		return;
	}

	ret = sscanf(cmd, "teleport %s %s", data, data2);
	if (ret == 2 && engine->netcode.client_flag == false)
	{
		snprintf(msg, LINE_SIZE, "target %s\n", data);
		menu.print(msg);

		for (unsigned int i = engine->max_dynamic; i < entity_list.size(); i++)
		{
			if ( !(entity_list[i]->ent_type == ENT_MISC_TELEPORTER_DEST ||
				   entity_list[i]->ent_type == ENT_TARGET_POSITION ||
				   entity_list[i]->ent_type == ENT_TARGET_TELEPORTER ||
				   entity_list[i]->ent_type == ENT_Q1_INFO_TELEPORT_DESTINATION))
				continue;

			if (!strcmp(entity_list[i]->entstring->target_name, data))
			{
				matrix4 matrix;
				unsigned int index = atoi(data2);
				Entity *ent = entity_list[self];

				if (ent->player->teleport_timer > 0)
					return;

				ent->player->teleport_timer = TICK_RATE >> 1;
				ent->position = entity_list[i]->position + vec3(0.0f, 50.0f, 0.0f);
				ent->rigid->velocity = vec3(0.0f, 0.0f, 0.0f);


				if (index < entity_list.size())
				{
					engine->play_wave(entity_list[index]->position, SND_TELEOUT);
				}

				if (ent->player->local)
					engine->play_wave_global(SND_TELEIN);
				else
					engine->play_wave(ent->position, SND_TELEIN);

				switch (entity_list[i]->brushinfo->angle)
				{
				case 0:
				case 45:
				case 360:
				case 315:
					matrix4::mat_left(matrix, ent->position);
					break;
				case 90:
					matrix4::mat_forward(matrix, ent->position);
					break;
				case 135:
				case 180:
					matrix4::mat_right(matrix, ent->position);
					break;
				case 225:
				case 270:
					matrix4::mat_backward(matrix, ent->position);
					break;
				}

				if (ent->player->local && spectator == 0)
				{
					engine->camera_frame.forward.x = matrix.m[8];
					engine->camera_frame.forward.y = matrix.m[9];
					engine->camera_frame.forward.z = matrix.m[10];
					engine->camera_frame.up = vec3(0.0f, 1.0f, 0.0f);
				}
				break;

			}
		}
		return;
	}

	char *pret = NULL; // linux didnt like pointer to int cast
	pret = strstr(cmd, "respawn");
	if (pret && engine->netcode.client_flag == false)
	{
		unsigned int i = last_spawn;
		bool spawned = false;
		unsigned int index = i;

		if (self == -1)
			return;

		// Respawn command needs to be rewritten really
		// param one spawns player on entity index given
		// param two spawns a different player entity
		// (give -1 as first param in thise case)
		ret = sscanf(cmd, "respawn %s %s", data, data2);
		if (ret == 2)
		{
			self = atoi(data2);
			if (self >= (int)entity_list.size() || entity_list[self]->player == NULL)
			{
				debugf("respawn given invalid player index\n");
				return;
			}
		}
		else if (ret == 1)
		{
			index = atoi(data);

			if (index >= entity_list.size())
			{
				debugf("respawn given invalid entity index\n");
				return;
			}
			i = index;
		}

		if (ret >= 0 && index != i)
		{
			matrix4 matrix;

			entity_list[self]->position = entity_list[index]->position + vec3(0.0f, 50.0f, 0.0f);

			entity_list[self]->rigid->bsp_trigger_volume = 0;

			switch (entity_list[i]->brushinfo->angle)
			{
			case 0:
				matrix4::mat_left(matrix, entity_list[self]->position);
				break;
			case 90:
				matrix4::mat_forward(matrix, entity_list[self]->position);
				break;
			case 180:
				matrix4::mat_right(matrix, entity_list[self]->position);
				break;
			case 270:
				matrix4::mat_backward(matrix, entity_list[self]->position);
				break;
			default:
				matrix4::mat_forward(matrix, entity_list[self]->position);
				break;
			}

			if (self == engine->find_type(ENT_PLAYER, 0) && spectator == 0)
			{
				engine->camera_frame.up.x = matrix.m[4];
				engine->camera_frame.up.y = matrix.m[5];
				engine->camera_frame.up.z = matrix.m[6];
				engine->camera_frame.forward.x = matrix.m[8];
				engine->camera_frame.forward.y = matrix.m[9];
				engine->camera_frame.forward.z = matrix.m[10];
			}

			debugf("Spawning on entity %d\n", index);
			entity_list[self]->player->respawn();
			if (entity_list[self]->player->local)
			{
				engine->mlight2.set_contrast(old_contrast);
				old_contrast = engine->mlight2.m_contrast;
			}

			entity_list[self]->rigid->clone(*(engine->thug22->model));

			engine->play_wave(entity_list[self]->position, SND_TELEIN);
			return;
		}

		while (spawned == false)
		{
			for (i = last_spawn; i < entity_list.size(); i++)
			{
				if (entity_list[i]->ent_type == ENT_INFO_PLAYER_DEATHMATCH ||
					entity_list[i]->ent_type == ENT_INFO_PLAYER_START)
				{
					matrix4 matrix;
					Entity *ent = entity_list[self];
					//					camera_frame.set(matrix);
					ent->position = entity_list[i]->position + vec3(0.0f, 50.0f, 0.0f);

					if (entity_list[i]->brushinfo)
					{
						switch (entity_list[i]->brushinfo->angle)
						{
						case 0:
							matrix4::mat_left(matrix, ent->position);
							break;
						case 90:
							matrix4::mat_forward(matrix, ent->position);
							break;
						case 180:
							matrix4::mat_right(matrix, ent->position);
							break;
						case 270:
							matrix4::mat_backward(matrix, ent->position);
							break;
						default:
							matrix4::mat_forward(matrix, ent->position);
							break;
						}
					}
					else
					{
						matrix4::mat_forward(matrix, ent->position);
					}
					EntModel *model = entity_list[self]->model;

					model->morientation.m[0] = matrix.m[0];
					model->morientation.m[1] = matrix.m[1];
					model->morientation.m[2] = matrix.m[2];

					model->morientation.m[3] = matrix.m[4];
					model->morientation.m[4] = matrix.m[5];
					model->morientation.m[5] = matrix.m[6];

					model->morientation.m[6] = matrix.m[8];
					model->morientation.m[7] = matrix.m[9];
					model->morientation.m[8] = matrix.m[10];

					if (self == engine->find_type(ENT_PLAYER, 0) && spectator == 0)
					{
						engine->camera_frame.up.x = matrix.m[4];
						engine->camera_frame.up.y = matrix.m[5];
						engine->camera_frame.up.z = matrix.m[6];
						engine->camera_frame.forward.x = matrix.m[8];
						engine->camera_frame.forward.y = matrix.m[9];
						engine->camera_frame.forward.z = matrix.m[10];
					}

					last_spawn = i + 1;
					debugf("Spawning on entity %d\n", i);
					entity_list[self]->player->respawn();
					entity_list[self]->rigid->clone(*(engine->thug22->model));
					if (entity_list[self]->player->local)
					{
						engine->mlight2.set_contrast(old_contrast);
						old_contrast = engine->mlight2.m_contrast;
					}


					engine->play_wave(entity_list[self]->position, SND_TELEIN);
					spawned = true;
					break;

				}
			}

			if (i == entity_list.size())
			{
				if (last_spawn != 0)
				{
					last_spawn = 0;
				}
				else
				{
					debugf("Failed to find a spawn point");
					break;
				}
			}
		}
		return;
	}

	ret = sscanf(cmd, "push %s", data);
	if (ret == 1)
	{
		bool local = entity_list[self]->player->local;

		snprintf(msg, LINE_SIZE, "push %s\n", data);
		menu.print(msg);

		for (unsigned int i = engine->max_dynamic; i < entity_list.size(); i++)
		{
			if (entity_list[i]->entstring == NULL)
				continue;

			if (!strcmp(entity_list[i]->entstring->target_name, data))
			{
				//target - origin
				vec3 dir = entity_list[i]->position - entity_list[self]->position;

				// clear out any negative velocity so if you are falling it still works
				if (engine->entity_list[self]->rigid->velocity.y < 0.0f)
					engine->entity_list[self]->rigid->velocity.y = 0.0f;

				//add velocity towards target
				engine->entity_list[self]->rigid->velocity += dir * JUMPPAD_SCALE * GRAVITY_SCALE;


				entity_list[self]->player->jumppad_timer = TICK_RATE >> 1;
				if (local)
					engine->play_wave(entity_list[self]->position, SND_JUMPPAD);
				else
					engine->play_wave_global(SND_JUMPPAD);
				break;
			}
		}
		return;
	}

	ret = sscanf(cmd, "name \"%[^\"]s", data);
	if (ret == 1)
	{
		bool valid = true;
		unsigned int data_length = strlen(data);

		for (unsigned int i = 0; i < data_length; i++)
		{
			if (data[i] >= 'A' && data[i] <= 'Z')
				continue;
			if (data[i] >= 'a' && data[i] <= 'z')
				continue;
			if (data[i] >= '0' && data[i] <= '9')
				continue;
			if (data[i] == ' ')
				continue;

			valid = false;
		}
		if (valid)
		{
			if (engine->netcode.server_flag)
			{
				engine->netcode.server_rename(entity_list[self]->player->name, data, self);
			}

			snprintf(entity_list[self]->player->name, 127, "%s", data);
			debugf("Player name: %s\n", data);
			sprintf(menu.data.name, "%s", data);

			if (engine->netcode.client_flag)
			{
				engine->netcode.client_rename();
			}
		}
		else
		{
			debugf("Invalid name, must be alphanumeric + space\n");
		}
		return;
	}

	ret = sscanf(cmd, "say \"%[^\"]s", data);
	if (ret == 1)
	{
		engine->netcode.chat(entity_list[self]->player->name, cmd);
		return;
	}

	ret = strcmp(cmd, "sv_list");
	if (ret == 0)
	{
		unsigned int current = (unsigned int)time(NULL);

		snprintf(msg, LINE_SIZE, "Client list\n");
		menu.print(msg);

		snprintf(msg, LINE_SIZE, "s: %s %d kills %d deaths %s %d idle\n", entity_list[self]->player->name,
			entity_list[self]->player->stats.kills,
			entity_list[self]->player->stats.deaths,
			"127.0.0.1:65535",
			0);
		menu.print(msg);


		for (unsigned int i = 0; i < engine->netcode.client_list.size(); i++)
		{
			EntPlayer *player = entity_list[engine->netcode.client_list[i]->ent_id]->player;
			snprintf(msg, LINE_SIZE, "%d: %s %d kills %d deaths %s %d idle\n", i, player->name,
				player->stats.kills,
				player->stats.deaths,
				engine->netcode.client_list[i]->socketname,
				current - engine->netcode.client_list[i]->last_time);
			menu.print(msg);
		}
		return;
	}

	ret = sscanf(cmd, "kick %s", data);
	if (ret == 1)
	{
		engine->netcode.kick(atoi(data));
		return;
	}

	ret = strcmp(cmd, "noclip");
	if (ret == 0 && engine->netcode.client_flag == false)
	{
		if (self != -1)
		{
			EntRigidBody *rigid = entity_list[self]->rigid;
			snprintf(msg, LINE_SIZE, "noclip\n");
			menu.print(msg);

			rigid->flags.noclip = !rigid->flags.noclip;
//			rigid->velocity.y = 0.0f; // stop initial sinking into floor from gravity
			//rigid->translational_friction = 0.9f;
		}
		return;
	}

	/*
	haste tempted to double rate of fire too
	*/

	ret = strcmp(cmd, "regeneration");
	if (ret == 0)
	{
		if (self != -1)
		{
			snprintf(msg, LINE_SIZE, "regeneration\n");
			menu.print(msg);
			entity_list[self]->player->regen_timer = 60 * TICK_RATE;
		}
		return;
	}

	ret = strcmp(cmd, "haste");
	if (ret == 0)
	{
		if (self != -1)
		{
			snprintf(msg, LINE_SIZE, "haste\n");
			menu.print(msg);
			entity_list[self]->player->haste_timer = 60 * TICK_RATE;
		}
		return;
	}

	ret = sscanf(cmd, "haste_factor %s", data);
	if (ret == 1)
	{
		if (self != -1)
		{
			snprintf(msg, LINE_SIZE, "haste_factor %s\n", data);
			menu.print(msg);
			float haste_factor = (float)atof(data);
			entity_list[self]->player->haste_factor = haste_factor;
		}
		return;
	}


	ret = strcmp(cmd, "quaddamage");
	if (ret == 0)
	{
		if (self != -1)
		{
			snprintf(msg, LINE_SIZE, "quaddamage\n");
			menu.print(msg);
			entity_list[self]->player->quad_timer = 60 * TICK_RATE;
		}
		return;
	}

	ret = strcmp(cmd, "invisibility");
	if (ret == 0)
	{
		if (self != -1)
		{
			snprintf(msg, LINE_SIZE, "invisibility\n");
			menu.print(msg);
			entity_list[self]->player->invisibility_timer = 60 * TICK_RATE;
		}
		return;
	}

	ret = strcmp(cmd, "flight");
	if (ret == 0)
	{
		if (self != -1)
		{
			snprintf(msg, LINE_SIZE, "flight\n");
			menu.print(msg);
			entity_list[self]->player->flight_timer = 30 * TICK_RATE;
		}
		return;
	}

	ret = sscanf(cmd, "flight %s", data);
	if (ret == 1)
	{
		if (self != -1)
		{
			snprintf(msg, LINE_SIZE, "flight %d\n", atoi(data));
			menu.print(msg);
			entity_list[self]->player->flight_timer = atoi(data) * TICK_RATE;
		}
		return;
	}

	ret = strcmp(cmd, "shownames");
	if (ret == 0)
	{
		snprintf(msg, LINE_SIZE, "shownames\n");
		menu.print(msg);
		engine->show_names = !engine->show_names;
		snprintf(msg, LINE_SIZE, "%s %d\n", cmd, engine->show_names);
		menu.print(msg);
		return;
	}

	ret = strcmp(cmd, "showbox");
	if (ret == 0)
	{
		snprintf(msg, LINE_SIZE, "showbox\n");
		menu.print(msg);
		engine->show_box = !engine->show_box;
		snprintf(msg, LINE_SIZE, "%s %d\n", cmd, engine->show_box);
		menu.print(msg);
		return;
	}

	ret = strcmp(cmd, "showlines");
	if (ret == 0)
	{
		snprintf(msg, LINE_SIZE, "showlines\n");
		menu.print(msg);
		engine->show_lines = !engine->show_lines;
		snprintf(msg, LINE_SIZE, "%s %d\n", cmd, engine->show_lines);
		menu.print(msg);
		return;
	}

	ret = strcmp(cmd, "showdebug");
	if (ret == 0)
	{
		snprintf(msg, LINE_SIZE, "showdebug\n");
		menu.print(msg);
		engine->show_debug = !engine->show_debug;
		snprintf(msg, LINE_SIZE, "%s %d\n", cmd, engine->show_debug);
		menu.print(msg);
		return;
	}

	ret = strcmp(cmd, "showhud");
	if (ret == 0)
	{
		snprintf(msg, LINE_SIZE, "showhud\n");
		menu.print(msg);
		engine->show_hud = !engine->show_hud;
		snprintf(msg, LINE_SIZE, "%s %d\n", cmd, engine->show_hud);
		menu.print(msg);
		return;
	}

	ret = strcmp(cmd, "r_all_lights");
	if (ret == 0)
	{
		snprintf(msg, LINE_SIZE, "r_all_lights\n");
		menu.print(msg);
		engine->all_lights = !engine->all_lights;
		snprintf(msg, LINE_SIZE, "%s %d\n", cmd, engine->all_lights);
		menu.print(msg);
		return;
	}

	ret = sscanf(cmd, "animation %s", data);
	if (ret == 1)
	{
		snprintf(msg, LINE_SIZE, "Selected animation %d\n", atoi(data));
		menu.print(msg);
		snprintf(msg, LINE_SIZE, "%s\n", cmd);
		menu.print(msg);
		engine->zcc.select_animation(atoi(data), false);

		if (atoi(data) == ANIM_WALK ||
			atoi(data) == ANIM_RUN)
		{
			int spawn = engine->find_type(ENT_PLAYER, 0);

			if (spawn == -1)
			{
				spawn = engine->find_type(ENT_SPECTATOR, 0);
			}

			if (spawn != -1)
			{
				entity_list[spawn]->player->done_transform = true;

				vec3 forward;

				// note this completely ignores collision detection
				entity_list[spawn]->rigid->getForwardVector(forward);
				entity_list[spawn]->player->done_pos = forward * 120.0f;
			}
		}
		return;
	}

	ret = strcmp(cmd, "godmode");
	if (ret == 0)
	{
		snprintf(msg, LINE_SIZE, "godmode\n");
		menu.print(msg);
		if (self != -1)
		{
			entity_list[self]->player->godmode = true;
		}
		return;
	}

	ret = strcmp(cmd, "give all");
	if (ret == 0)
	{
		snprintf(msg, LINE_SIZE, "give all\n");
		menu.print(msg);
		if (self != -1)
		{
			entity_list[self]->player->ammo_bfg = 999;
			entity_list[self]->player->ammo_bullets = 999;
			entity_list[self]->player->ammo_lightning = 999;
			entity_list[self]->player->ammo_plasma = 999;
			entity_list[self]->player->ammo_grenades = 999;
			entity_list[self]->player->ammo_rockets = 999;
			entity_list[self]->player->ammo_shells = 999;
			entity_list[self]->player->ammo_slugs = 999;
			entity_list[self]->player->armor = 200;
			entity_list[self]->player->health = 100;
			entity_list[self]->player->weapon_flags = ~0;
		}
		return;
	}


	if (strcmp(cmd, "disconnect") == 0)
	{
		snprintf(msg, LINE_SIZE, "disconnecting\n");
		menu.print(msg);
		engine->unload();
		return;
	}

	if (strstr(cmd, "reset"))
	{
		int reset_score = 0;

		snprintf(msg, LINE_SIZE, "reset\n");
		sscanf(cmd, "reset %d", &reset_score);

		for (unsigned int i = 0; i < engine->max_player; i++)
		{
			if (entity_list[i]->player)
			{
				char respawn[128];

				sprintf(respawn, "respawn -1 %d", i);
				if (reset_score)
					entity_list[i]->player->reset();
				console(i, respawn, menu, entity_list);
			}

			round_time = 0;
			played_one_frag = false;
			played_two_frag = false;
			played_three_frag = false;
			played_prepare = false;
			played_sudden = false;
		}

		return;
	}


	if (sscanf(cmd, "g_gametype %s", data) == 1)
	{
		menu.print(msg);

		if (strstr(data, "ctf"))
		{
			gametype = GAMETYPE_CTF;
		}
		else if (strstr(data, "deathmatch"))
		{
			gametype = GAMETYPE_DEATHMATCH;
		}
		return;
	}

	if (sscanf(cmd, "cg_faceicon %s", data) == 1)
	{
		int flag = atoi(data);
		menu.print(msg);

		if (flag)
			faceicon = true;
		else
			faceicon = false;


		return;
	}


	if (sscanf(cmd, "g_fraglimit %s", data) == 1)
	{
		menu.print(msg);
		fraglimit = atoi(data);
		return;
	}

	if (sscanf(cmd, "g_timelimit %s", data) == 1)
	{
		menu.print(msg);
		timelimit = atoi(data);
		return;
	}

	if (sscanf(cmd, "g_warmup %s", data) == 1)
	{
		menu.print(msg);
		warmup_time = atoi(data);

		if (warmup_time == 0)
		{
			for (unsigned int i = 0; i < engine->max_player; i++)
			{
				if (engine->entity_list[i]->player)
				{
					memset(&engine->entity_list[i]->player->stats, 0, sizeof(stats_t));
				}
			}
			engine->input.scores = false;
			warmup = false;
			hold_fire = false;
			console(-1, "reset 0", engine->menu, engine->entity_list);
		}

		return;
	}

	if (sscanf(cmd, "g_capturelimit %s", data) == 1)
	{
		menu.print(msg);
		capturelimit = atoi(data);

		return;
	}


	snprintf(msg, LINE_SIZE, "Unknown command: %s\n", cmd);
	menu.print(msg);
}


///=============================================================================
/// Function: setup_func
///=============================================================================
/// Description: setups func_mover entities (BSP brushes treated like entities)
/// such as elevators, pendulums, func_bobbing, func_train, func_* type objects
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::setup_func(vector<Entity *> &entity_list, Bsp *q3map)
{
	for (unsigned int i = engine->max_dynamic; i < entity_list.size(); i++)
	{
		Entity *ent = entity_list[i];
		Entity *ref = ent;

		if (ent->brush_ref != -1)
		{
			q3map->model_type[ent->brush_ref] = ent->ent_type;
			ent->position = q3map->model_origin(ent->brush_ref);
		}

		if (ent->brushinfo)
		{
			ent->brushinfo->origin = ent->position;
		}


		switch (ent->ent_type)
		{
		case ENT_TRIGGER_PUSH:
			sprintf(ent->trigger->action, "push %s", ent->entstring->target);
			break;
		case ENT_FUNC_PENDULUM:
// PENDULUM start at origin, need to offset
//			q3map.model_offset[entity_list[i]->model_ref] = entity_list[i]->position;
ent->rigid->angular_velocity = vec3(10.0f, 10.0f, 10.0f);
break;
		case ENT_FUNC_BOBBING:
			switch (ent->brushinfo->angle)
			{
			case 0:
			case 360:
				ent->rigid->path.path_list[0] = ent->position + vec3(-ent->brushinfo->func_height, 0.0f, 0.0f);
				ent->rigid->path.path_list[1] = ent->position + vec3(ent->brushinfo->func_height, 0.0f, 0.0f);
				break;
			case 90:
				ent->rigid->path.path_list[0] = ent->position + vec3(0.0f, 0.0f, -ent->brushinfo->func_height);
				ent->rigid->path.path_list[1] = ent->position + vec3(0.0f, 0.0f, ent->brushinfo->func_height);
				break;
			case 180:
				ent->rigid->path.path_list[0] = ent->position + vec3(ent->brushinfo->func_height, 0.0f, 0.0f);
				ent->rigid->path.path_list[1] = ent->position + vec3(-ent->brushinfo->func_height, 0.0f, 0.0f);
				break;
			case 270:
				ent->rigid->path.path_list[0] = ent->position + vec3(0.0f, 0.0f, ent->brushinfo->func_height);
				ent->rigid->path.path_list[1] = ent->position + vec3(0.0f, 0.0f, -ent->brushinfo->func_height);
				break;
			case -1:
				ent->rigid->path.path_list[0] = ent->position + vec3(0.0f, ent->brushinfo->func_height, 0.0f);
				ent->rigid->path.path_list[1] = ent->position + vec3(0.0f, -ent->brushinfo->func_height, 0.0f);
				break;
			case -2:
				ent->rigid->path.path_list[0] = ent->position + vec3(0.0f, -ent->brushinfo->func_height, 0.0f);
				ent->rigid->path.path_list[1] = ent->position + vec3(0.0f, ent->brushinfo->func_height, 0.0f);
				break;
			default:
				ent->rigid->path.path_list[0] = ent->position + vec3(0.0f, ent->brushinfo->func_height, 0.0f);
				ent->rigid->path.path_list[1] = ent->position + vec3(0.0f, -ent->brushinfo->func_height, 0.0f);
				break;
			}
			ent->rigid->path.num_path = 2;
			break;
		case ENT_FUNC_TRAIN:
			q3map->model_offset[entity_list[i]->brush_ref] = entity_list[i]->position;
			for (unsigned int i = engine->max_dynamic; i < engine->entity_list.size(); i++)
			{
				int ret = add_train_path(ent, ref, engine->entity_list[i]);
				if (ret == 1)
				{
					//target found, set new reference to target
					ref = engine->entity_list[i];

					// restart loop from begining
					i = engine->max_dynamic - 1;
					continue;
				}
				else if (ret == 2)
				{
					ent->rigid->path.loop = 1;
					break;
				}
			}

			//entity defaults at origin, move to first path_corner
			ent->position += ent->rigid->path.path_list[0];

			break;
		case ENT_PATH_CORNER:
			entity_list[i]->flags.visible = true;
			entity_list[i]->flags.bsp_visible = true;
			entity_list[i]->rigid->flags.gravity = false;
			break;
		case ENT_MISC_PORTAL_CAMERA:
			engine->q3map.portal_tex = entity_list[i]->portal_camera->quad_tex;
			break;
		case ENT_MISC_PORTAL_SURFACE:
			if (entity_list[i]->entstring->target[0] != '\0')
			{
				// If we have a target, then use corresponding portal camera
				delete entity_list[i]->portal_camera;
				entity_list[i]->portal_camera = NULL;
			}
			else
			{
				// mirror
				engine->q3map.portal_tex = entity_list[i]->portal_camera->quad_tex;
			}
			break;
		case ENT_TRIGGER_TELEPORT:
			// Reset action because of ordering issues
			sprintf(entity_list[i]->trigger->action, "teleport %s %d", entity_list[i]->entstring->target, i);
			break;
		case ENT_FUNC_DOOR:
			if (entity_list[i]->trigger)
			{
				if (strstr(entity_list[i]->trigger->action, "damage"))
				{
					entity_list[i]->trigger->action[0] = '\0';
				}
			}
			break;
		case ENT_TARGET_SPEAKER:
			if (entity_list[i]->entstring && strlen(entity_list[i]->entstring->target_name) > 2)
			{
				entity_list[i]->trigger->noise = false;
			}
			break;

		case ENT_FUNC_CLOTH:
		{
			cloth::Cloth *cloth = new cloth::Cloth();
			engine->cloth.push_back(cloth);


			matrix4 matrix;
			if (entity_list[i]->brushinfo)
			{
				//set spawn angle
				switch (entity_list[i]->brushinfo->angle)
				{
				case 0:
				case 45:
				case 360:
				case 325:
					matrix4::mat_left(matrix, entity_list[i]->position);
					break;
				case 90:
				case 135:
					matrix4::mat_forward(matrix, entity_list[i]->position);
					break;
				case 180:
				case 225:
					matrix4::mat_right(matrix, entity_list[i]->position);
					break;
				case 270:
					matrix4::mat_backward(matrix, entity_list[i]->position);
					break;
				default:
					matrix4::mat_left(matrix, entity_list[i]->position);
					break;
				}
			}

			float *m = &entity_list[i]->rigid->morientation.m[0];

			m[0] = matrix.m[0];
			m[1] = matrix.m[1];
			m[2] = matrix.m[2];

			m[3] = matrix.m[4];
			m[4] = matrix.m[5];
			m[5] = matrix.m[6];

			m[6] = matrix.m[7];
			m[7] = matrix.m[8];
			m[8] = matrix.m[10];


			cloth->init(130, 220, 10, 20);
			cloth->create_buffers(engine->gfx);

			// just sticking cloth index someplace for reference by entity
			entity_list[i]->brushinfo->opening = engine->cloth.size() - 1;
			cloth->ent_index = i;

			cloth->tex = load_texture_pk3(engine->gfx, entity_list[i]->entstring->target, engine->pk3_list, engine->num_pk3, false, false, 0);
			break;
		}
		}

		if ((entity_list[i]->ent_type > ENT_FUNC_START && entity_list[i]->ent_type < ENT_FUNC_END) ||
			entity_list[i]->ent_type == ENT_INFO_PLAYER_INTERMISSION ||
			entity_list[i]->ent_type == ENT_TARGET_POSITION ||
			entity_list[i]->ent_type == ENT_INFO_NOTNULL ||
			entity_list[i]->ent_type == ENT_TRIGGER_PUSH)
		{
			entity_list[i]->rigid->flags.gravity = false;
		}

	}
}


///=============================================================================
/// Function: make_dynamic_ent
///=============================================================================
/// Description: in a network game, when the player is a client, they repeatedly
/// get entity position and orientation information from the server.
/// This function will create a local only entity for items like ejection
/// shells, rockets, plasma balls, etc so that they appear sooner than the
/// round trip time of the network. They will be replaced with "real" server
/// objects when they are received over the network
///
/// Old comments:
///		Used to create entities for network clients
///		Network clients are more interested in visual properties,
///		they get the position and orientation over the network
///		and let server handle damage etc
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::make_dynamic_ent(net_ent_t item, int ent_id)
{
	Entity *ent = engine->entity_list[ent_id];
	engine->clean_entity(ent_id);

	switch (item)
	{
	case NET_NONE:
		engine->clean_entity(ent_id);
		break;
	case NET_BULLET_HIT:
		ent->nettype = NET_BULLET_HIT;
		ent->rigid = new EntRigidBody(ent);
		ent->rigid->clone(model_table[MODEL_BULLET_HIT]);
		ent->rigid->flags.gravity = false;
		ent->rigid->bounce = 2;
		ent->model = ent->rigid;
		ent->rigid->flags.noclip = true;
		ent->flags.visible = true; // accomodate for low spatial testing rate
		ent->bsp_leaf = true;
		ent->model->flags.blend = true;
		ent->model->flags.cull_none = true;

		ent->projectile = new EntProjectile(ent, engine->audio);
		ent->projectile->idle = true;
		ent->projectile->explode_timer = 10;
		ent->projectile->idle_timer = 10;
		ent->projectile->hide = false;
		break;
	case NET_PLASMA_HIT:
		ent->nettype = NET_PLASMA_HIT;
		ent->rigid = new EntRigidBody(ent);
		ent->rigid->clone(model_table[MODEL_PLASMA_HIT]);
		ent->rigid->flags.gravity = false;
		ent->rigid->bounce = 2;
		ent->model = ent->rigid;
		ent->rigid->flags.noclip = true;
		ent->flags.visible = true; // accomodate for low spatial testing rate
		ent->bsp_leaf = true;
		ent->model->flags.blend = true;
		ent->model->flags.cull_none = true;

		ent->projectile = new EntProjectile(ent, engine->audio);
		ent->projectile->idle = true;
		ent->projectile->explode_timer = 10;
		ent->projectile->idle_timer = 10;
		ent->projectile->hide = false;
		break;
	case NET_BULLET:
		ent->nettype = NET_BULLET;
		ent->rigid = new EntRigidBody(ent);
		ent->rigid->clone(model_table[MODEL_BULLET]);
		ent->rigid->flags.gravity = false;
		ent->rigid->bounce = 2;
		ent->model = ent->rigid;
		ent->rigid->flags.noclip = false;
		ent->flags.visible = true; // accomodate for low spatial testing rate
		ent->bsp_leaf = true;
		ent->rigid->impact_index = SND_BULLET;
		break;
	case NET_SHELL:
		ent->nettype = NET_SHELL;
		ent->rigid = new EntRigidBody(ent);
		ent->rigid->clone(model_table[MODEL_SHELL]);
		ent->rigid->flags.gravity = false;
		ent->rigid->bounce = 2;
		ent->model = ent->rigid;
		ent->rigid->flags.noclip = false;
		ent->flags.visible = true; // accomodate for low spatial testing rate
		ent->bsp_leaf = true;
		ent->rigid->impact_index = SND_SHELL;
		break;
	case NET_BOOM:
		ent->nettype = NET_BOOM;
		ent->rigid = new EntRigidBody(ent);
		ent->rigid->clone(model_table[MODEL_BOOM]);
		ent->rigid->flags.gravity = false;
		ent->rigid->bounce = 2;
		ent->model = ent->rigid;
		ent->rigid->flags.noclip = true;
		ent->flags.visible = true; // accomodate for low spatial testing rate
		ent->bsp_leaf = true;
		ent->model->flags.blend = true;
		ent->model->flags.cull_none = true;

		ent->projectile = new EntProjectile(ent, engine->audio);
		ent->projectile->idle = true;
		ent->projectile->explode_timer = 10;
		ent->projectile->idle_timer = 10;
		ent->projectile->hide = false;
		break;
	case NET_ROCKET:
		ent->nettype = NET_ROCKET;
		ent->projectile = new EntProjectile(ent, engine->audio);
		ent->projectile->explode_index = SND_EXPLODE;
		ent->projectile->idle_index = SND_ROCKETFLY;

		ent->projectile->hide = false;
		ent->projectile->radius = 25.0f;
		ent->projectile->idle = true;
		ent->projectile->explode = true;
		ent->projectile->idle_timer = 0;
		ent->projectile->explode_timer = 10;
		ent->projectile->explode_color = vec3(1.0f, 0.0f, 0.0f);
		ent->projectile->explode_intensity = 500.0f;
		ent->projectile->splash_radius = 250.0f;
		ent->projectile->knockback = 250.0f;
		ent->projectile->splash_damage = 0;
		ent->num_particle = 5000;
		ent->flags.particle_on = true;


		ent->light = new EntLight(ent, engine->gfx, 999, engine->res_scale);
		ent->light->color = vec3(1.0f, 1.0f, 1.0f);
		ent->light->intensity = 1000.0f;

		ent->rigid = new EntRigidBody(ent);
		ent->model = ent->rigid;
		ent->rigid->clone(model_table[MODEL_ROCKET]);
		ent->rigid->flags.gravity = false;
		break;
	case NET_ROCKET_LAUNCHER:
		ent->rigid = new EntRigidBody(ent);
		ent->model = ent->rigid;
		ent->model->load(engine->gfx, "media/models/weapons2/rocketl/rocketl");
		ent->nettype = NET_ROCKET_LAUNCHER;
		ent->rigid->flags.translational_friction_flag = true;
		ent->rigid->flags.rotational_friction_flag = true;

		ent->trigger = new EntTrigger(ent, engine->audio);

		ent->trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
		ent->trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
		sprintf(ent->trigger->action, "weapon_rocketlauncer");
		break;
	case NET_GRENADE:
		ent->nettype = NET_GRENADE;

		ent->rigid = new EntRigidBody(ent);
		ent->model = ent->rigid;
		ent->rigid->clone(model_table[MODEL_GRENADE]);
		ent->rigid->flags.gravity = true;
		ent->rigid->flags.rotational_friction_flag = true;
		ent->rigid->flags.translational_friction_flag = true;
		ent->rigid->translational_friction = 0.9f;
		ent->num_particle = 5000;
		ent->flags.particle_on = true;

		ent->projectile = new EntProjectile(ent, engine->audio);
		ent->projectile->explode_index = SND_EXPLODE;
		ent->projectile->splash_damage = 0;
		ent->projectile->hide = false;
		ent->projectile->radius = 25.0f;
		ent->projectile->idle = true;
		ent->projectile->idle_timer = 120;
		ent->projectile->explode = true;
		ent->projectile->explode_timer = 10;
		ent->projectile->explode_color = vec3(1.0f, 0.0f, 0.0f);
		ent->projectile->explode_intensity = 500.0f;
		ent->projectile->splash_radius = 250.0f;
		ent->projectile->knockback = 250.0f;
		break;
	case NET_GRENADE_LAUNCHER:
		ent->rigid = new EntRigidBody(ent);
		ent->model = ent->rigid;
		ent->model->load(engine->gfx, "media/models/weapons2/grenadel/grenade");
		ent->nettype = NET_GRENADE_LAUNCHER;
		ent->rigid->flags.translational_friction_flag = true;
		ent->rigid->flags.rotational_friction_flag = true;

		ent->trigger = new EntTrigger(ent, engine->audio);
		ent->trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
		ent->trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
		sprintf(ent->trigger->action, "weapon_grenadelauncer");
		break;
	case NET_LIGHTNING:
		ent->nettype = NET_LIGHTNING;
		ent->rigid = new EntRigidBody(ent);
		ent->rigid->clone(model_table[MODEL_BOX]);
		ent->rigid->velocity = vec3();
		ent->rigid->angular_velocity = vec3();
		ent->rigid->flags.gravity = false;
		ent->model = ent->rigid;
		ent->model->flags.lightning_trail = true;
		ent->rigid->bounce = 5;
		ent->rigid->flags.noclip = true;


		ent->projectile = new EntProjectile(ent, engine->audio);
		sprintf(ent->projectile->action, " ");
		ent->projectile->splash_damage = 0;
		ent->projectile->hide = false;
		ent->projectile->radius = 25.0f;
		ent->projectile->idle = true;
		ent->projectile->idle_timer = (int)(0.1 * TICK_RATE);
		ent->projectile->explode = true;
		ent->projectile->explode_timer = 20;
		break;
	case NET_LIGHTNINGGUN:
		ent->rigid = new EntRigidBody(ent);
		ent->model = ent->rigid;
		ent->model->load(engine->gfx, "media/models/weapons2/lightning/lightning");
		ent->nettype = NET_LIGHTNINGGUN;
		ent->rigid->flags.translational_friction_flag = true;
		ent->rigid->flags.rotational_friction_flag = true;

		ent->trigger = new EntTrigger(ent, engine->audio);
		ent->trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
		ent->trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
		sprintf(ent->trigger->action, "weapon_lightninggun");
		break;
	case NET_RAIL:
		ent->nettype = NET_RAIL;
		ent->rigid = new EntRigidBody(ent);
		ent->rigid->clone(model_table[MODEL_BALL]);
		ent->rigid->velocity = vec3();
		ent->rigid->angular_velocity = vec3();
		ent->rigid->flags.gravity = false;
		ent->rigid->bounce = 5;
		ent->model = ent->rigid;
		ent->model->flags.rail_trail = true;
		ent->rigid->flags.noclip = true;

		ent->projectile = new EntProjectile(ent, engine->audio);
		sprintf(ent->projectile->action, " ");
		ent->projectile->hide = false;
		ent->projectile->radius = 25.0f;
		ent->projectile->idle = true;
		ent->projectile->idle_timer = (int)(5.0 * TICK_RATE);
		ent->projectile->explode = true;
		ent->projectile->explode_timer = 10;
		break;
	case NET_RAILGUN:
		ent->rigid = new EntRigidBody(ent);
		ent->model = ent->rigid;
		ent->model->load(engine->gfx, "media/models/weapons2/railgun/railgun");
		ent->nettype = NET_RAILGUN;
		ent->rigid->flags.translational_friction_flag = true;
		ent->rigid->flags.rotational_friction_flag = true;

		ent->trigger = new EntTrigger(ent, engine->audio);
		ent->trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
		ent->trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
		sprintf(ent->trigger->action, "weapon_railgun");
		break;
	case NET_PLASMA:
		ent->nettype = NET_PLASMA;
		ent->rigid = new EntRigidBody(ent);
		ent->model = ent->rigid;

		ent->rigid->clone(model_table[MODEL_BALL]);
		ent->rigid->flags.gravity = false;
		ent->projectile = new EntProjectile(ent, engine->audio);
		ent->projectile->explode_index = engine->get_load_wave("sound/weapons/plasma/plasmx1a.wav");
		ent->projectile->idle_index = engine->get_load_wave("sound/weapons/plasma/lasfly.wav");

		ent->projectile->splash_damage = 0;
		ent->projectile->hide = false;
		ent->projectile->radius = 25.0f;
		ent->projectile->idle = true;
		ent->projectile->explode = false;
		ent->projectile->explode_timer = 10;
		ent->projectile->explode_color = vec3(0.0f, 0.0f, 1.0f);
		ent->projectile->explode_intensity = 200.0f;
		ent->projectile->splash_radius = 75.0f;
		ent->projectile->knockback = 10.0f;


		ent->light = new EntLight(ent, engine->gfx, 999, engine->res_scale);
		ent->light->color = vec3(0.0f, 0.0f, 1.0f);
		ent->light->intensity = 1000.0f;

		break;
	case NET_PLASMAGUN:
		ent->rigid = new EntRigidBody(ent);
		ent->model = ent->rigid;
		ent->model->load(engine->gfx, "media/models/weapons2/plasmagun/plasmagun");
		ent->nettype = NET_PLASMAGUN;
		ent->rigid->flags.translational_friction_flag = true;
		ent->rigid->flags.rotational_friction_flag = true;

		ent->trigger = new EntTrigger(ent, engine->audio);
		ent->trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
		ent->trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
		sprintf(ent->trigger->action, "weapon_plasma");
		break;
	case NET_SHOTGUN:
		ent->rigid = new EntRigidBody(ent);
		ent->model = ent->rigid;
		ent->model->load(engine->gfx, "media/models/weapons2/shotgun/shotgun");
		ent->nettype = NET_SHOTGUN;
		ent->rigid->flags.translational_friction_flag = true;
		ent->rigid->flags.rotational_friction_flag = true;

		ent->trigger = new EntTrigger(ent, engine->audio);
		ent->trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
		ent->trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
		sprintf(ent->trigger->action, "weapon_shotgun");
		break;
	case NET_MACHINEGUN:
		ent->rigid = new EntRigidBody(ent);
		ent->model = ent->rigid;
		ent->model->load(engine->gfx, "media/models/weapons2/machinegun/machinegun");
		ent->nettype = NET_MACHINEGUN;
		ent->rigid->flags.translational_friction_flag = true;
		ent->rigid->flags.rotational_friction_flag = true;

		ent->trigger = new EntTrigger(ent, engine->audio);
		ent->trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
		ent->trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
		sprintf(ent->trigger->action, "weapon_machinegun");
		break;
	case NET_QUAD:
		ent->rigid = new EntRigidBody(ent);
		ent->model = ent->rigid;
		ent->model->load(engine->gfx, "media/models/powerups/instant/quad");
		ent->nettype = NET_QUAD;
		ent->rigid->flags.translational_friction_flag = true;
		ent->rigid->flags.rotational_friction_flag = true;

		ent->trigger = new EntTrigger(ent, engine->audio);
		ent->trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
		ent->trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
		sprintf(ent->trigger->action, "quaddamage");
		break;
	case NET_REGEN:
		ent->rigid = new EntRigidBody(ent);
		ent->model = ent->rigid;
		ent->model->load(engine->gfx, "media/models/powerups/instant/regen");
		ent->nettype = NET_REGEN;
		ent->rigid->flags.translational_friction_flag = true;
		ent->rigid->flags.rotational_friction_flag = true;

		ent->trigger = new EntTrigger(ent, engine->audio);
		ent->trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
		ent->trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
		sprintf(ent->trigger->action, "regeneration");
		break;
	case NET_INVIS:
		ent->rigid = new EntRigidBody(ent);
		ent->model = ent->rigid;
		ent->model->load(engine->gfx, "media/models/powerups/instant/invis");
		ent->nettype = NET_INVIS;
		ent->rigid->flags.translational_friction_flag = true;
		ent->rigid->flags.rotational_friction_flag = true;

		ent->trigger = new EntTrigger(ent, engine->audio);
		ent->trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
		ent->trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
		sprintf(ent->trigger->action, "invisibility");
		break;
	case NET_FLIGHT:
		ent->rigid = new EntRigidBody(ent);
		ent->model = ent->rigid;
		ent->model->load(engine->gfx, "media/models/powerups/instant/flight");
		ent->nettype = NET_FLIGHT;
		ent->rigid->flags.translational_friction_flag = true;
		ent->rigid->flags.rotational_friction_flag = true;

		ent->trigger = new EntTrigger(ent, engine->audio);
		ent->trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
		ent->trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
		sprintf(ent->trigger->action, "flight");
		break;
	case NET_HASTE:
		ent->rigid = new EntRigidBody(ent);
		ent->model = ent->rigid;
		ent->model->load(engine->gfx, "media/models/powerups/instant/haste");
		ent->nettype = NET_HASTE;
		ent->rigid->flags.translational_friction_flag = true;
		ent->rigid->flags.rotational_friction_flag = true;

		ent->trigger = new EntTrigger(ent, engine->audio);
		ent->trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
		ent->trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
		sprintf(ent->trigger->action, "haste");
		break;
	case NET_BLUE_FLAG:
		ent->rigid = new EntRigidBody(ent);
		ent->model = ent->rigid;
		ent->model->load(engine->gfx, "media/models/flags/b_flag");
		ent->nettype = NET_BLUE_FLAG;
		ent->rigid->flags.translational_friction_flag = true;
		ent->rigid->flags.rotational_friction_flag = true;

		ent->trigger = new EntTrigger(ent, engine->audio);
		ent->trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
		ent->trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
		sprintf(ent->trigger->action, "blue_flag");
		break;
	case NET_RED_FLAG:
		ent->rigid = new EntRigidBody(ent);
		ent->model = ent->rigid;
		ent->model->load(engine->gfx, "media/models/flags/r_flag");
		ent->nettype = NET_RED_FLAG;
		ent->rigid->flags.translational_friction_flag = true;
		ent->rigid->flags.rotational_friction_flag = true;

		ent->trigger = new EntTrigger(ent, engine->audio);
		ent->trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
		ent->trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");

		sprintf(ent->trigger->action, "red_flag");
		break;
	}
}


///=============================================================================
/// Function: endgame
///=============================================================================
/// Description: when someone hits the frag limit, time limit expires, etc
/// display the winner's name and such for a period of time before next game
/// starts
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::endgame(char *winner)
{
	engine->input.scores = true;

	win_timer = 3 * TICK_RATE;
	strcpy(win_msg, winner);

	warmup = true;
	hold_fire = true;
	round_time = 0;
}


///=============================================================================
/// Function: check_target
///=============================================================================
/// Description: Called to check for player entering various trigger volumes
/// defined in the map (teleporters, damage volumes, jump pads etc)
/// triggers can have targets, which are other triggers, and can have other targets
/// etc causing a chaining of events
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::check_target(vector<Entity *> &entity_list, Entity *ent, Entity *target, int self)
{

	if (ent->entstring && target->entstring && strcmp(ent->entstring->target, target->entstring->target_name) == 0)
	{
		if (strlen(ent->entstring->target) < 2 || strlen(target->entstring->target_name) < 2)
			return;

		printf("%s bsp volume triggered %s with targetname %s\n",
			ent->entstring->type, target->entstring->type, ent->entstring->target);

		if (target->trigger)
		{
			if (target->ent_type == ENT_TRIGGER_TELEPORT ||
				strcmp(target->trigger->action, "teleport") == 0)
			{
				// bots will teleport player
				return;
			}


			if (target->trigger->active == false)
			{
				target->trigger->active = true;

				if (strstr(target->trigger->action, "map"))
					engine->console(target->trigger->action);
				else
					console(self, target->trigger->action, engine->menu, engine->entity_list);
			}
			else
			{
				printf("trigger has already been hit\n");
				return;
			}
		}

		switch (target->ent_type)
		{
		case ENT_TARGET_SPEAKER:
			//hack we know it's *falling
				if (target->trigger)
				{
					EntPlayer *player = entity_list[self]->player;
					if (player->falling == false)
					{
						if (strstr(target->trigger->noise_str, "*falling"))
						{
							printf("Ahhhh...\n");
							engine->play_wave(entity_list[self]->position, player->model_index * SND_PLAYER + SND_FALLING);
							player->falling = true;
						}
					}

					if (target->trigger->played  == false)
					{
						engine->play_wave(target->position, target->trigger->respawn_index);
						target->trigger->played = true;
						printf("trigger played %s\n", target->trigger->noise_str);
					}
				}

			break;
		case ENT_TARGET_REMOVE_POWERUPS:
			//hack for q3tourney3, just kill them
			console(self, "damage 1000", engine->menu, entity_list);
			break;
		case ENT_TARGET_RELAY:
			// search again, great
			if (strlen(ent->entstring->target) > 1)
			{
				for (unsigned int k = engine->max_dynamic; k < entity_list.size(); k++)
				{
					check_target(entity_list, target, entity_list[k], self);
				}
			}
			return;
		}
	}
}


///=============================================================================
/// Function: handle_model_trigger
///=============================================================================
/// Description: Handles BSP trigger volume logic
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::handle_model_trigger(vector<Entity *> &entity_list, Entity *ent, int self)
{
	int model_index = ent->brush_ref;

	for (unsigned int j = 0; j < engine->max_player; j++)
	{
		if (entity_list[j]->player == NULL || entity_list[j]->rigid == NULL)
		{
			continue;
		}

		if (entity_list[j]->player)
		{
			// dont let dead people trigger anything
			if (entity_list[j]->player->health <= 0)
				continue;
		}

		if (entity_list[j]->rigid->bsp_trigger_volume != model_index)
		{
			continue;
		}
		entity_list[j]->rigid->bsp_trigger_volume = 0;


		if (ent->trigger && ent->trigger->active == false)
		{
			printf("Triggered bsp volume %d type %s\n", ent->brush_ref, ent->entstring->type);
			ent->trigger->active = true;
			console(j, ent->trigger->action, engine->menu, entity_list);
		}
		else
		{
			printf("bsp volume %d trigger already hit\n", ent->brush_ref);
		}

		if (ent->entstring != NULL)
		{
			for (unsigned int j = engine->max_dynamic; j < entity_list.size(); j++)
			{
				check_target(entity_list, ent, entity_list[j], self);
			}
		}
	}
}


///=============================================================================
/// Function: check_triggers
///=============================================================================
/// Description: Checks BSP trigger volume logic
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::check_triggers(EntPlayer *player, Entity *ent, int self, vector<Entity *> &entity_list)
{
	bool inside = false;
	EntRigidBody *rigid = ent->rigid;

	if (rigid && rigid->flags.hard_impact)
	{
		if (rigid->impact_velocity <= -RIGID_IMPACT)
		{
			rigid->flags.hard_impact = false;

			engine->play_wave(ent->position, rigid->impact_index);
		}
	}

	// Not a trigger
	EntTrigger *trigger = ent->trigger;

	if (trigger == NULL)
		return;

	float distance = (ent->position - player->entity->position).magnitude();

	if (distance < trigger->radius)
		inside = true;

	if (inside && ent->ent_type == ENT_TEAM_CTF_BLUEFLAG)
	{
		if (player->team == TEAM_BLUE)
		{
			if (player->holdable_flag)
			{
				player->holdable_flag = false;
				blue_flag_caps++;

				engine->play_wave(ent->position, SND_FLAGCAP);

				if (blue_flag_caps >= capturelimit)
				{
					endgame("Blue team wins");
				}
				else if (blue_flag_caps == red_flag_caps)
				{
					engine->play_wave_global(SND_TEAMS_TIED);
				}
				else if (blue_flag_caps > red_flag_caps)
				{
					engine->play_wave_global(SND_BLUE_LEAD);
				}

			}
			return;
		}
	}

	if (inside && ent->ent_type == ENT_TEAM_CTF_REDFLAG)
	{
		if (player->team == TEAM_RED)
		{
			if (player->holdable_flag)
			{
				player->holdable_flag = false;
				red_flag_caps++;

				if (player->local)
					engine->play_wave_global(SND_FLAGTAKE);
				else
					engine->play_wave(ent->position, SND_FLAGTAKE);

				if (red_flag_caps >= capturelimit)
				{
					endgame("Red team wins");
				}
				else if (blue_flag_caps == red_flag_caps)
				{
					engine->play_wave_global(SND_TEAMS_TIED);
				}
				else if (blue_flag_caps < red_flag_caps)
				{
					engine->play_wave_global(SND_RED_LEAD);
				}
			}
			return;
		}
	}

	if (inside == true && trigger->active == false)
	{
		int pickup = true;

		if (trigger->armor && player->armor >= 200)
			pickup = false;

		if (trigger->health && player->health >= 100)
			pickup = false;

		if (player->state == PLAYER_DEAD)
			pickup = false;

		if (player->teleport_timer > 0 && ent->ent_type == ENT_TRIGGER_TELEPORT)
			pickup = false;



		if (pickup)
		{
			if (trigger->action[0] != '\0' && trigger->client_active == false)
			{
				if (strstr(trigger->action, "map"))
				{
					engine->console(trigger->action);
					return;
				}
				else
				{
					console(self, trigger->action, engine->menu, entity_list);
				}
			}

			trigger->active = true;
			trigger->client_active = true;

			ent->flags.visible = false;
			trigger->timeout = ent->trigger->timeout_value;

			if (player->local)
				engine->play_wave_global(trigger->pickup_index);
			else
				engine->play_wave(ent->position, trigger->pickup_index);

		}
	}


	if (trigger->timeout > 0)
	{
		trigger->timeout -= 0.016f;
	}
	else
	{
		trigger->played = false;
		if (trigger->active && trigger->noise)
		{
			// play periodic sound
			engine->play_wave(ent->position, trigger->respawn_index);
		}

		if (trigger->noise == false)
		{
			// means this sound must be triggered, reset timeout so it's not trigger continously
			trigger->active = false;
			trigger->client_active = false;
		}
		else
		{
			trigger->timeout = trigger->timeout_value;
		}
	}
}

///=============================================================================
/// Function: check_func
///=============================================================================
/// Description: Checks func_* entities for various triggers/targets etc
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::check_func(EntPlayer *player, Entity *ent, int self, vector<Entity *> &entity_list)
{
	//bool inside = false;
	//RigidBody *rigid = ent->rigid;

	if ((ent->ent_type > ENT_FUNC_START && ent->ent_type < ENT_FUNC_END) || ent->ent_type == ENT_TRIGGER_MULTIPLE)
	{
		float distance = (player->entity->position - ent->position).magnitude();

		if (ent->ent_type == ENT_FUNC_STATIC)
			return;


		if (ent->ent_type == ENT_TRIGGER_MULTIPLE)
		{
			float distance = (ent->position - player->entity->position).magnitude();

			if (distance < 75.0f)
			{
				for (unsigned int j = engine->max_dynamic; j < entity_list.size(); j++)
				{
					if (ent == entity_list[j])
						continue;

					if (strcmp(ent->entstring->target, entity_list[j]->entstring->target_name) == 0)
					{
						printf("trigger_multiple triggered target %s of type %s\n", ent->entstring->target, entity_list[j]->entstring->type);
						if (entity_list[j]->trigger)
						{
							console(self, entity_list[j]->trigger->action, engine->menu, engine->entity_list);

							if (entity_list[j]->ent_type == ENT_TARGET_DELAY ||
								entity_list[j]->ent_type == ENT_FUNC_TIMER ||
								entity_list[j]->ent_type == ENT_TARGET_GIVE )
							{

								if (entity_list[j]->ent_type == ENT_TARGET_DELAY && entity_list[j]->trigger->timeout == 0)
								{
									for (unsigned int k = engine->max_dynamic; k < entity_list.size(); k++)
									{
										check_target(entity_list, entity_list[j], entity_list[k], self);
									}
								}
								else if (entity_list[j]->ent_type != ENT_TARGET_DELAY)
								{
									for (unsigned int k = engine->max_dynamic; k < entity_list.size(); k++)
									{
										check_target(entity_list, entity_list[j], entity_list[k], self);
									}
								}
							}

							if (entity_list[j]->ent_type == ENT_TARGET_SPEAKER)
							{
								if (entity_list[j]->trigger->played == false)
								{
									if (player->local)
										engine->play_wave_global(entity_list[j]->trigger->respawn_index);
									else
										engine->play_wave(ent->position, entity_list[j]->trigger->respawn_index);

									entity_list[j]->trigger->played = true;
								}
							}
						}
					}
				}
			}
			return;
		}


		if (ent->brushinfo == NULL)
			return;

		// start closing a distance further than we started opening
		if (ent->brushinfo && distance > 300.0f && ent->brushinfo->opening > 0)
		{
			ent->brushinfo->opening = false;
			if (ent->brushinfo->model_lerp > 0.99f)
			{
				engine->play_wave(ent->position, SND_DOOR_END);
			}
		}

		if (ent->brushinfo->opening == false && ent->brushinfo->model_lerp > 0.0)
		{

			if (ent->ent_type == ENT_FUNC_PLAT)
			{
				ent->brushinfo->model_offset = ent->brushinfo->model_offset * (1.0f - ent->brushinfo->model_lerp);
			}
			else
			{
				ent->brushinfo->model_offset = ent->brushinfo->model_offset * ent->brushinfo->model_lerp;
			}
			engine->q3map.model_offset[ent->brush_ref] = ent->brushinfo->model_offset;


			if (ent->brushinfo->model_lerp > 0.0f)
				ent->brushinfo->model_lerp -= 0.0001f;
			else
				ent->brushinfo->model_lerp = 0.0f;
		}


		// only open if very close
		if (distance > 100.0f && ent->brushinfo->opening == false)
		{
			return;
		}

		ent->brushinfo->opening = true;


		if (ent->ent_type == ENT_FUNC_DOOR || ent->ent_type == ENT_FUNC_BUTTON || ent->ent_type == ENT_FUNC_PLAT)
		{
			//				float amount = entity_list[i]->height;
			float amount = 50.0f;
			float half_x = 1.0f;
			float half_y = 1.0f;
			float half_z = 1.0f;

			if (ent->ent_type == ENT_FUNC_BUTTON)
			{
				amount = 10.0f; // buttons are tiny doors ;)
			}
			else if (ent->ent_type == ENT_FUNC_PLAT)
			{
				amount = (engine->q3map.data.Model[ent->brush_ref].max[1]
					- engine->q3map.data.Model[ent->brush_ref].min[1]);

				amount *= 0.9f;
				if (amount < 0)
					amount *= -1;
			}



			if (ent->brushinfo->model_lerp < 0.01f)
			{
				if (ent->ent_type == ENT_FUNC_BUTTON)
				{
					engine->play_wave(ent->position, SND_BUTTON);
					for (unsigned int j = engine->max_dynamic; j < entity_list.size(); j++)
					{
						if (ent == entity_list[j])
							continue;

						if (strcmp(ent->entstring->target, entity_list[j]->entstring->target_name) == 0)
						{
							printf("func_button triggered target %s of type %s\n", ent->entstring->target, entity_list[j]->entstring->type);
						}
					}
				}
				else
				{
					engine->play_wave(ent->position, SND_DOOR_START);
				}
			}

			if (ent->brushinfo->model_lerp < 1.0f)
				ent->brushinfo->model_lerp += 0.01f;

			if (abs32(amount) < 0.001f)
			{
				half_x = (engine->q3map.data.Model[ent->brush_ref].max[0]
					- engine->q3map.data.Model[ent->brush_ref].min[0]);

				if (half_x < 0)
					half_x *= -1;

				half_y = (engine->q3map.data.Model[ent->brush_ref].max[1]
					- engine->q3map.data.Model[ent->brush_ref].min[1]);

				if (half_y < 0)
					half_y *= -1;

				half_z = (engine->q3map.data.Model[ent->brush_ref].max[2]
					- engine->q3map.data.Model[ent->brush_ref].min[2]);

				if (half_z < 0)
					half_z *= -1;
			}


			amount = amount * ent->brushinfo->model_lerp;

			// platforms start up (so lightmaps are generated)
			// so invert lerp value
			if (ent->ent_type == ENT_FUNC_PLAT)
				amount = amount * (1.0f - ent->brushinfo->model_lerp);



			vec3 end;
			switch (ent->brushinfo->angle)
			{
			case 0:
			case 360:
				ent->brushinfo->model_offset = vec3(amount, 0.0f, 0.0f);
				break;
			case 90:
				ent->brushinfo->model_offset = vec3(0.0f, 0.0f, -amount);
				break;
			case 180:
				ent->brushinfo->model_offset = vec3(-amount, 0.0f, 0.0f);
				break;
			case 270:
				ent->brushinfo->model_offset = vec3(0.0f, 0.0f, amount);
				break;
			case -1://up
				ent->brushinfo->model_offset = vec3(0.0f, amount, 0.0f);
				break;
			case -2://down
				ent->brushinfo->model_offset = vec3(0.0f, -amount, 0.0f);
				break;
			}


			engine->q3map.model_offset[ent->brush_ref] = ent->brushinfo->model_offset;


		}
	}

	if (ent->brush_ref > 0 && (unsigned int)ent->brush_ref < engine->q3map.data.num_model)
	{
		handle_model_trigger(entity_list, ent, self);
	}

}

///=============================================================================
/// Function: check_projectiles
///=============================================================================
/// Description: Check something like a rocket / plasma ball if it needs to explode
/// or richochet or something
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::check_projectiles(EntPlayer *player, Entity *ent, Entity *owner, int self, int proj_id, vector<Entity *> &entity_list)
{
	bool inside = false;
	//RigidBody *rigid = ent->rigid;

	// Not a trigger
	EntProjectile *projectile = ent->projectile;

	if (projectile == NULL)
		return;

	// Delete when not moving
	if (projectile->idle == true)
	{
		if (ent->rigid)
		{
			if (ent->rigid->bounce > projectile->num_bounce || ent->rigid->velocity.magnitude() < 0.0001f)
			{
				ent->flags.particle_on = false;
				if (projectile->explode == false)
				{
					if (projectile->explode_timer <= 0)
					{
						engine->clean_entity(proj_id);
						ent->~Entity();
						return;
					}
					else
					{

						if (projectile->explode_type == 1)
						{
							int sprite_index = MIN(7, projectile->explode_timer);
							if (ent->model->model_index != model_table[MODEL_BOOM].model_index)
							{
								ent->nettype = NET_BOOM;
								ent->model->clone(model_table[MODEL_BOOM]);
								ent->model->flags.blend = true;
							}
							ent->model->model_tex = icon_list[ICON_RLBOOM8 - sprite_index].tex;
						}
						else if (projectile->explode_type == 2)
						{
							if (ent->model->model_index != model_table[MODEL_PLASMA_HIT].model_index)
							{
								ent->nettype = NET_PLASMA_HIT;
								ent->model->clone(model_table[MODEL_PLASMA_HIT]);
								ent->model->flags.blend = true;
							}
						}
						projectile->explode_timer--;
					}
				}
				else
				{
					// Explode after being idle for idle_timer time (usually zero)
					if (projectile->idle_timer <= 0)
					{
						projectile->radius = projectile->splash_radius;
						sprintf(projectile->action, "damage %d", projectile->splash_damage);
						if (ent->light == NULL)
						{
							ent->light = new EntLight(ent, engine->gfx, 999, engine->res_scale);
						}
						ent->light->intensity = projectile->explode_intensity;
						ent->light->color = projectile->explode_color;
						projectile->explode = false;

						engine->play_wave(ent->position, projectile->explode_index);
						return;
					}
					else
					{
						projectile->idle_timer--;
					}
				}
			}
		}
	}


	// Only other players can pick up
	if (owner == player->entity && ent->rigid && ent->rigid->bounce == 0)
		return;


	if (projectile->owner >= 0 &&
		gametype != GAMETYPE_DEATHMATCH &&
		owner->player->team == player->team)
		return;

	float distance = (ent->position - player->entity->position).magnitude();


	if (distance < projectile->radius)
		inside = true;

	if (inside && ent->ent_type == ENT_TEAM_CTF_BLUEFLAG)
	{
		if (player->team == TEAM_BLUE)
		{
			if (player->holdable_flag)
			{
				player->holdable_flag = false;
				blue_flag_caps++;

				engine->play_wave(ent->position, SND_FLAGCAP);

				if (blue_flag_caps >= capturelimit)
				{
					endgame("Blue team wins");
				}
				else if (blue_flag_caps == red_flag_caps)
				{
					engine->play_wave_global(SND_TEAMS_TIED);
				}
				else if (blue_flag_caps > red_flag_caps)
				{
					engine->play_wave_global(SND_BLUE_LEAD);
				}

			}
			return;
		}
	}


	if (inside == true && projectile->active == false)
	{
		int pickup = true;

		if (pickup)
		{
			if (projectile->action[0] != '\0' && projectile->client_active == false)
			{
				console(self, projectile->action, engine->menu, entity_list);
			}

			projectile->will_be_active = true;
			projectile->client_active = true;

			if (projectile)
			{
				if (projectile->explode_type == 1)
				{
					ent->rigid->velocity *= 0.5f;
					int sprite_index = MIN(7, projectile->explode_timer);
					if (ent->model->model_index != model_table[MODEL_BOOM].model_index)
					{
						ent->model->clone(model_table[MODEL_BOOM]);
						ent->model->flags.blend = true;
					}
					ent->model->model_tex = icon_list[ICON_RLBOOM8 - sprite_index].tex;
				}
				else if (projectile->explode_type == 2)
				{
					ent->rigid->velocity = vec3();
					if (ent->model->model_index != model_table[MODEL_PLASMA_HIT].model_index)
					{
						ent->model->clone(model_table[MODEL_PLASMA_HIT]);
						ent->model->flags.blend = true;
					}
				}

				if (player->health <= 0 && player->state != PLAYER_DEAD)
				{
					char word[32] = "";
					char weapon[32] = "";

					player->state = PLAYER_DEAD;
					player->stats.deaths++;
					if (owner != NULL)
					{
						EntPlayer *powner = owner->player;
						powner->stats.kills++;
						powner->stats.hits++;

						if (powner->current_weapon == wp_rocket)
						{
							sprintf(weapon, "rocket launcher");
						}
						else if (powner->current_weapon == wp_grenade)
						{
							sprintf(weapon, "grenade launcher");
						}
						else if (powner->current_weapon == wp_plasma)
						{
							sprintf(weapon, "plasma gun");
						}
						else if (powner->current_weapon == wp_lightning)
						{
							sprintf(weapon, "lightning gun");
						}
						else if (powner->current_weapon == wp_shotgun)
						{
							sprintf(weapon, "shotgun");
						}
						else if (powner->current_weapon == wp_machinegun)
						{
							sprintf(weapon, "machinegun");
						}
						else if (powner->current_weapon == wp_railgun)
						{
							sprintf(weapon, "railgun");
						}
						else if (powner->current_weapon == wp_gauntlet)
						{
							sprintf(weapon, "gauntlet");
						}


						if (player->health <= GIB_HEALTH)
							sprintf(word, "%s", "gibbed");
						else
							sprintf(word, "%s", "killed");

						char msg[256];

						if (powner == player)
						{
							sprintf(msg, "%s killed themselves with a %s\n",
								player->name, weapon);
						}
						else
						{
							sprintf(msg, "%s %s %s with a %s\n",
								powner->name,
								word,
								player->name,
								weapon);
						}

						// allow rocket jumping to pass max air speed
						if (player->entity->rigid->flags.on_ground == false)
							player->pm_max_air_speed *= 2.0f;
						debugf(msg);
						engine->menu.print_notif(msg);
						notif_timer = 3 * TICK_RATE;
						handle_frags_left(*(owner->player));
					}

				}
			}

			ent->flags.visible = false;
			projectile->timeout = ent->projectile->timeout_value;

			if (projectile->explode_timer)
			{
				vec3 distance = player->entity->position - ent->position;
				float mag = MIN(distance.magnitude(), 50.0f);

				if (abs32(mag) > 0.0001f)
				{
					//add knockback to explosions
					if (player->entity->rigid)
					{
						player->entity->rigid->velocity += (distance.normalize() * projectile->knockback) / mag;
					}
				}
			}

			if (player->local)
				engine->play_wave_global(projectile->pickup_index);
			else
				engine->play_wave(ent->position, projectile->pickup_index);

		}
	}


	if (projectile->timeout > 0)
	{
		projectile->timeout -= 0.016f;
	}
	else
	{
		projectile->played = false;
		if (projectile->active && projectile->noise)
		{
			// play periodic sound
			engine->play_wave(ent->position, projectile->respawn_index);
		}

		if (projectile->noise == false)
		{
			// means this sound must be triggered, reset timeout so it's not trigger continously
			projectile->active = false;
			projectile->client_active = false;
			//				trigger->timeout = trigger->timeout_value;;
		}
		else
		{
			projectile->timeout = projectile->timeout_value;
		}
	}
}

///=============================================================================
/// Function: get_state
///=============================================================================
/// Description: Gets game state that exists outside of entities
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::get_state(serverdata_t *data)
{
	memset(data, 0, sizeof(serverdata_t));
	memcpy(data->header, "<data>", 6);
	memcpy(data->footer, "</data>", 8);

	if (warmup)
		data->warmup = 0xFF;

	data->warmup_time = warmup_time;
	data->red_flag_caps = red_flag_caps;
	data->blue_flag_caps = blue_flag_caps;
	data->fraglimit = fraglimit;
	data->timelimit = timelimit;
	data->round_time = round_time;
}

///=============================================================================
/// Function: set_state
///=============================================================================
/// Description: Sets game state that exists outside of entities
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::set_state(serverdata_t *data)
{
	if (memcmp(data->header, "<data>", 6) == 0 && memcmp(data->footer, "</data>", 7) == 0)
	{
		if (data->warmup)
		{
			warmup = true;
		}
		else
		{
			for (unsigned int i = 0; i < engine->max_player; i++)
			{
				if (engine->entity_list[i]->player)
				{
					memset(&engine->entity_list[i]->player->stats, 0, sizeof(stats_t));
				}
			}
			engine->input.scores = false;
			warmup = false;
			hold_fire = false;
			console(-1, "reset 0", engine->menu, engine->entity_list);
		}
		warmup_time = data->warmup_time;
		red_flag_caps = data->red_flag_caps;
		blue_flag_caps = data->blue_flag_caps;
		fraglimit = data->fraglimit;
		timelimit = data->timelimit;
		round_time = data->round_time;
	}
}

///=============================================================================
/// Function: add_decal
///=============================================================================
/// Description: Add's a decal (such as a rocket explosion burn mark) to the map
/// and/or walls (called decals cause they are like stickers if that makes sense)
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::add_decal(vec3 &start, Frame &camera_frame, net_ent_t nettype, EntModel &decal_model, float offset, bool idle, int idle_timer)
{
	plane_t plane;
	float depth;
	float water_depth;
	vec3 clip;
	vec3 velocity;
	bool ret = false;
	bool hit = false;
	vec3 pos;
	vec3 step = start;
	int nstep = 0;
	int model_trigger = 0;
	int model_platform = 0;
	content_flag_t flag;


	vec3 end = step + camera_frame.forward;
	
//	pos = engine->q3map.trace(start, end, normal);
//	engine->q3map.collision = false;

	vec3 tri[3];

	do
	{
		ret = engine->q3map.collision_detect(end, step, &plane, &depth, water_depth, engine->surface_list, false, clip, velocity, model_trigger, model_platform, flag, tri);
		if (ret)
		{
			hit = true;
			pos = end + camera_frame.forward.normalize() * -depth;
		}
		else
		{
			step = end;
			end += camera_frame.forward;
			nstep++;
		}
	} while (hit == false && nstep < 10000);





	if (hit)
	{
//		printf("decal added at x %4.3f y %4.3f z %4.3f\n", pos.x, pos.y, pos.z);
		Entity *decal = engine->entity_list[engine->get_entity()];
		decal->rigid = new EntRigidBody(decal);
		decal->position = pos + plane.normal * offset;
		decal->rigid->clone(decal_model);
		decal->rigid->flags.gravity = false;
		decal->rigid->bounce = 2;
		decal->model = decal->rigid;
		decal->rigid->flags.noclip = true;
		decal->flags.visible = true; // accomodate for low spatial testing rate
		decal->bsp_leaf = true;
		decal->model->flags.blend = true;
		decal->model->flags.cull_none = true;
		decal->nettype = nettype;

		decal->projectile = new EntProjectile(decal, engine->audio);
		decal->projectile->idle = idle;
		decal->projectile->explode_timer = idle_timer;
		decal->projectile->idle_timer = idle_timer;
		decal->projectile->hide = false;




		// make point axial to plane (change forward vector and normalze a bunch)
		vec3 up = camera_frame.up;
		vec3 forward = plane.normal;

		vec3 right = vec3::crossproduct(forward, up);
		right.normalize();
		up = vec3::crossproduct(right, forward);
		up.normalize();
		right = vec3::crossproduct(up, forward);
		right.normalize();

		decal->model->morientation.m[0] = right.x;
		decal->model->morientation.m[1] = right.y;
		decal->model->morientation.m[2] = right.z;

		decal->model->morientation.m[3] = up.x;
		decal->model->morientation.m[4] = up.y;
		decal->model->morientation.m[5] = up.z;

		decal->model->morientation.m[6] = forward.x;
		decal->model->morientation.m[7] = forward.y;
		decal->model->morientation.m[8] = forward.z;

		/*
		Entity *decal2 = engine->entity_list[engine->get_entity()];
		decal2->rigid = new RigidBody(decal2);
		decal2->model = decal->rigid;
		decal2->position = pos + plane.normal * offset;
		decal2->rigid->clone(*(engine->mark->model));
		decal2->visible = true; // accomodate for low spatial testing rate
		decal2->bsp_leaf = -1;
		decal2->rigid->gravity = false;
		decal2->rigid->noclip = true;
		decal2->rigid->bounce = 2;
		decal2->rigid->blend = true;

		decal2->trigger = new Trigger(decal, engine->audio);
		decal2->trigger->idle = idle;
		decal2->trigger->explode_timer = idle_timer * 2000;
		decal2->trigger->idle_timer = idle_timer * 2000;
		decal2->trigger->projectile = true;
		decal2->trigger->hide = false;



		decal2->model->morientation.m[0] = right.x * 0.01f;
		decal2->model->morientation.m[1] = right.y;
		decal2->model->morientation.m[2] = right.z;

		decal2->model->morientation.m[3] = up.x;
		decal2->model->morientation.m[4] = up.y * 0.01f;
		decal2->model->morientation.m[5] = up.z;

		decal2->model->morientation.m[6] = forward.x;
		decal2->model->morientation.m[7] = forward.y;
		decal2->model->morientation.m[8] = forward.z * 0.01f;

		decal2->model->morientation.m[15] *= 0.01f;
		*/

	}
}

///=============================================================================
/// Function: map_model
///=============================================================================
/// Description: So we have models loaded in memory, which may be used by multiple
/// entities (say two piece of armor on a map of same type) We dont want to duplicate
/// the vertex data, so we "clone" the data into each entity
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::map_model(Entity *ent)
{
	if (ent->ent_type == ENT_ITEM_ARMOR_SHARD)
	{
		debugf("Loading item_armor_shard\n");
		ent->model->clone(model_table[MODEL_ARMOR_SHARD]);
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->rigid->flags.gravity = false;
	}
	else if (ent->ent_type == ENT_WEAPON_ROCKETLAUNCHER)
	{
		debugf("Loading weapon_rocketlauncher\n");
		ent->model->clone(model_table[MODEL_WEAPON_ROCKET]);
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->rigid->flags.gravity = false;
	}
	else if (ent->ent_type == ENT_AMMO_SHELLS)
	{
		debugf("Loading ammo_shells\n");
		ent->model->clone(model_table[MODEL_AMMO_SHELLS]);
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->rigid->flags.gravity = false;
	}
	else if (ent->ent_type == ENT_AMMO_ROCKETS)
	{
		debugf("Loading ammo_rockets\n");
		ent->model->clone(model_table[MODEL_AMMO_ROCKETS]);
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->rigid->flags.gravity = false;
	}
	else if (ent->ent_type == ENT_AMMO_CELLS)
	{
		debugf("Loading ammo_cells\n");
		ent->model->clone(model_table[MODEL_AMMO_PLASMA]);
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->rigid->flags.gravity = false;
	}
	else if (ent->ent_type == ENT_AMMO_BFG)
	{
		debugf("Loading ammo_bfg\n");
		ent->model->clone(model_table[MODEL_AMMO_LIGHTNING]);
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->rigid->flags.gravity = false;
	}
	else if (ent->ent_type == ENT_AMMO_GRENADES)
	{
		debugf("Loading ammo_bfg\n");
		ent->model->clone(model_table[MODEL_AMMO_GRENADES]);
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->rigid->flags.gravity = false;
	}
	else if (ent->ent_type == ENT_AMMO_LIGHTNING)
	{
		debugf("Loading ammo_lightning\n");
		ent->model->clone(model_table[MODEL_AMMO_LIGHTNING]);
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->rigid->flags.gravity = false;
	}
	else if (ent->ent_type == ENT_AMMO_SLUGS)
	{
		debugf("Loading ammo_slugs\n");
		ent->model->clone(model_table[MODEL_AMMO_SLUGS]);
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->rigid->flags.gravity = false;
	}
	else if (ent->ent_type == ENT_AMMO_BULLETS)
	{
		debugf("Loading ammo_bullets\n");
		ent->model->clone(model_table[MODEL_AMMO_BULLETS]);
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->rigid->flags.gravity = false;
	}
	else if (ent->ent_type == ENT_ITEM_ARMOR_COMBAT)
	{
		debugf("Loading item_armor_combat\n");
		ent->model->clone(model_table[MODEL_ARMOR_COMBAT]);
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->position.y += 15.0f;
		ent->rigid->flags.gravity = false;
	}
	else if (ent->ent_type == ENT_ITEM_ARMOR_BODY)
	{
		debugf("Loading item_armor_body\n");
		ent->model->clone(model_table[MODEL_ARMOR_BODY]);
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->position.y += 15.0f;
		ent->rigid->flags.gravity = false;
	}
	else if (ent->ent_type == ENT_TEAM_CTF_BLUEFLAG)
	{
		debugf("Loading team_CTF_blueflag\n");
		ent->model->clone(model_table[MODEL_BLUE_FLAG]);
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->position += vec3(0.0f, 50.0f, 0.0);
		ent->rigid->flags.gravity = false;
	}
	else if (ent->ent_type == ENT_TEAM_CTF_REDFLAG)
	{
		debugf("Loading team_CTF_redflag\n");
		ent->model->clone(model_table[MODEL_RED_FLAG]);
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->position += vec3(0.0f, 50.0f, 0.0);
		ent->rigid->flags.gravity = false;
	}
	else if (ent->ent_type == ENT_ITEM_QUAD)
	{
		debugf("Loading item_quad\n");
		ent->model->clone(model_table[MODEL_QUAD]);
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->rigid->flags.gravity = false;
		ent->position.y += 15.0f;
	}
	else if (ent->ent_type == ENT_ITEM_REGEN)
	{
		debugf("Loading item_regen\n");
		ent->model->clone(model_table[MODEL_REGEN]);
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->rigid->flags.gravity = false;
		ent->position.y += 15.0f;
	}
	else if (ent->ent_type == ENT_ITEM_INVIS)
	{
		debugf("Loading item_invis\n");
		ent->model->clone(model_table[MODEL_INVIS]);
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->rigid->flags.gravity = false;
		ent->position.y += 15.0f;
	}
	else if (ent->ent_type == ENT_ITEM_FLIGHT)
	{
		debugf("Loading item_flight\n");
		ent->model->clone(model_table[MODEL_FLIGHT]);
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->rigid->flags.gravity = false;
		ent->position.y += 15.0f;
	}
	else if (ent->ent_type == ENT_ITEM_HASTE)
	{
		debugf("Loading item_haste\n");
		ent->model->clone(model_table[MODEL_HASTE]);
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->rigid->flags.gravity = false;
		ent->position.y += 15.0f;
	}
	else if (ent->ent_type == ENT_HOLDABLE_MEDKIT)
	{
		debugf("Loading holdable_medkit\n");
		ent->model->clone(model_table[MODEL_MEDKIT]);
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->rigid->flags.gravity = false;
		ent->position.y += 15.0f;
	}
	else if (ent->ent_type == ENT_HOLDABLE_TELEPORTER)
	{
		debugf("Loading holdable_teleporter\n");
		ent->model->clone(model_table[MODEL_TELEPORTER]);
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->rigid->flags.gravity = false;
		ent->position.y += 15.0f;
	}
	else if (ent->ent_type == ENT_FUNC_TERRAIN)
	{
		debugf("Loading func_terrain\n");
		ent->model->clone(model_table[MODEL_TERRAIN]);
		ent->position += vec3(0.0f, -5000.0f, 0.0f);
		ent->rigid->flags.gravity = false;
	}
	else if (ent->ent_type == ENT_WEAPON_LIGHTNING)
	{
		debugf("Loading weapon_lightning\n");
		ent->model->clone(model_table[MODEL_WEAPON_LIGHTNING]);
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->rigid->flags.gravity = false;
	}
	else if (ent->ent_type == ENT_WEAPON_BFG)
	{
		debugf("Loading weapon_bfg\n");
		ent->model->clone(model_table[MODEL_WEAPON_LIGHTNING]);
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->rigid->flags.gravity = false;
	}
	else if (ent->ent_type == ENT_WEAPON_SHOTGUN)
	{
		debugf("Loading weapon_shotgun\n");
		ent->model->clone(model_table[MODEL_WEAPON_SHOTGUN]);
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->rigid->flags.gravity = false;
	}
	else if (ent->ent_type == ENT_WEAPON_RAILGUN)
	{
		debugf("Loading weapon_railgun\n");
		ent->model->clone(model_table[MODEL_WEAPON_RAILGUN]);
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->rigid->flags.gravity = false;
	}
	else if (ent->ent_type == ENT_WEAPON_GRENADELAUNCHER)
	{
		debugf("Loading weapon_grenadelauncher\n");
		ent->model->clone(model_table[MODEL_WEAPON_GRENADE]);
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->rigid->flags.gravity = false;
	}
	else if (ent->ent_type == ENT_WEAPON_PLASMA)
	{
		debugf("Loading weapon_plasmagun\n");
		ent->model->clone(model_table[MODEL_WEAPON_PLASMA]);
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->rigid->flags.gravity = false;
	}
	else if (ent->ent_type == ENT_ITEM_HEALTH)
	{
		debugf("Loading item_health\n");
		ent->model->clone(model_table[MODEL_HEALTH]);
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->rigid->flags.gravity = false;
	}
	else if (ent->ent_type == ENT_ITEM_HEALTH_LARGE)
	{
		debugf("Loading item_health_large\n");
		ent->model->clone(model_table[MODEL_HEALTH_LARGE]);
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->rigid->flags.gravity = false;
	}
	else if (ent->ent_type == ENT_ITEM_HEALTH_SMALL)
	{
		debugf("Loading item_health_large\n");
		ent->model->clone(model_table[MODEL_HEALTH_SMALL]);
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->rigid->flags.gravity = false;
	}
	else if (ent->ent_type == ENT_ITEM_HEALTH_MEGA)
	{
		debugf("Loading item_health_mega\n");
		ent->model->clone(model_table[MODEL_HEALTH_MEGA]);
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->rigid->flags.gravity = false;
	}
	else if (ent->ent_type == ENT_INFO_PLAYER_DEATHMATCH)
	{
		debugf("Loading info_player_deathmatch\n");
		ent->model->clone(model_table[MODEL_BALL]);
		ent->rigid->flags.gravity = false;
		ent->flags.nodraw = true;
	}
	else if (ent->ent_type == ENT_TEAM_CTF_BLUESPAWN)
	{
		debugf("Loading team_CTF_bluespawn\n");
		ent->model->clone(model_table[MODEL_BALL]);
		ent->rigid->flags.gravity = false;
		ent->flags.nodraw = true;
	}
	else if (ent->ent_type == ENT_TEAM_CTF_REDSPAWN)
	{
		debugf("Loading team_CTF_redspawn\n");
		ent->model->clone(model_table[MODEL_BALL]);
		ent->rigid->flags.gravity = false;
		ent->flags.nodraw = true;
	}
	else if (ent->ent_type == ENT_TEAM_CTF_BLUEPLAYER)
	{
		debugf("Loading team_CTF_blueplayer\n");
		ent->model->clone(model_table[MODEL_BALL]);
		ent->rigid->flags.gravity = false;
		ent->flags.nodraw = true;
	}
	else if (ent->ent_type == ENT_TEAM_CTF_REDPLAYER)
	{
		debugf("Loading team_CTF_redplayer\n");
		ent->model->clone(model_table[MODEL_BALL]);
		ent->rigid->flags.gravity = false;
		ent->flags.nodraw = true;
	}
	else if (ent->ent_type > ENT_FUNC_START && ent->ent_type < ENT_FUNC_END)
	{

		if (!(ent->ent_type == ENT_FUNC_DOOR || ent->ent_type == ENT_FUNC_BUTTON || ent->ent_type == ENT_FUNC_PLAT))
		{
			debugf("Loading func item\n");
			ent->model->clone(model_table[MODEL_BOX]);
			ent->rigid->flags.gravity = false;
			ent->flags.nodraw = false;
		}
	}
	else if (ent->ent_type == ENT_PATH_CORNER)
	{
		debugf("Loading path_corner\n");
		ent->model->clone(model_table[MODEL_BOX]);
		ent->rigid->flags.gravity = false;
		ent->flags.nodraw = false;
	}
	else if (ent->ent_type == ENT_Q1_MONSTER_DEMON1)
	{
		debugf("Loading monster_demon1\n");
		ent->model->clone(model_table[MODEL_DEMON]);
		ent->rigid->flags.gravity = true;
		ent->flags.nodraw = false;
	}
	else if (ent->ent_type == ENT_Q1_MONSTER_SHAMBLER)
	{
		debugf("Loading monster_shambler\n");
		ent->model->clone(model_table[MODEL_SHAMBLER]);
		ent->rigid->flags.gravity = true;
		ent->flags.nodraw = false;
	}
	else if (ent->ent_type == ENT_Q1_MONSTER_DOG)
	{
		debugf("Loading monster_dog\n");
		ent->model->clone(model_table[MODEL_DOG]);
		ent->rigid->flags.gravity = true;
		ent->flags.nodraw = false;
	}
	else if (ent->ent_type == ENT_Q1_MONSTER_ARMY)
	{
		debugf("Loading monster_army\n");
		ent->model->clone(model_table[MODEL_SOLDIER]);
		ent->rigid->flags.gravity = true;
		ent->flags.nodraw = false;
	}
	else if (ent->ent_type == ENT_Q1_MONSTER_WIZARD)
	{
		debugf("Loading monster_wizard\n");
		ent->model->clone(model_table[MODEL_WIZARD]);
		ent->rigid->flags.gravity = true;
		ent->flags.nodraw = false;
	}
	else if (ent->ent_type == ENT_Q1_MONSTER_OGRE)
	{
		debugf("Loading monster_ogre\n");
		ent->model->clone(model_table[MODEL_OGRE]);
		ent->rigid->flags.gravity = true;
		ent->flags.nodraw = false;
	}
	else if (ent->ent_type == ENT_Q1_MONSTER_KNIGHT)
	{
		debugf("Loading monster_knight\n");
		ent->model->clone(model_table[MODEL_KNIGHT]);
		ent->rigid->flags.gravity = true;
		ent->flags.nodraw = false;
	}
	else if (ent->ent_type == ENT_Q1_MONSTER_ZOMBIE)
	{
		debugf("Loading monster_knight\n");
		ent->model->clone(model_table[MODEL_ZOMBIE]);
		ent->rigid->flags.gravity = true;
		ent->flags.nodraw = false;
	}
	else if (ent->ent_type == ENT_Q1_LIGHT_FLUORO)
	{
		debugf("Loading light_fluoro\n");
		ent->model->clone(model_table[MODEL_V_LIGHT]);
		ent->rigid->flags.gravity = true;
		ent->flags.nodraw = false;
	}
	else if (ent->ent_type == ENT_Q1_LIGHT_FLUOROSPARK)
	{
		debugf("Loading light_fluorospark\n");
		ent->model->clone(model_table[MODEL_V_LIGHT]);
		ent->rigid->flags.gravity = true;
		ent->flags.nodraw = false;
	}
	else if (ent->ent_type == ENT_Q1_ITEM_ARTIFACT_SUPER_DAMAGE)
	{
		debugf("Loading item_artifact_super_damage\n");
		ent->model->clone(model_table[MODEL_QUADDAMA]);
		ent->rigid->flags.gravity = false;
		ent->flags.nodraw = false;
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->position.y += 15.0f;
	}
	else if (ent->ent_type == ENT_Q1_ITEM_ARTIFACT_INVULNERABILITY)
	{
		debugf("Loading item_artifact_invulnerability\n");
		ent->model->clone(model_table[MODEL_INVULNER]);
		ent->rigid->flags.gravity = false;
		ent->flags.nodraw = false;
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->position.y += 15.0f;
	}
	else if (ent->ent_type == ENT_Q1_ITEM_ARTIFACT_ENVIROSUIT)
	{
		debugf("Loading item_artifact_envirosuit\n");
		ent->model->clone(model_table[MODEL_SUIT]);
		ent->rigid->flags.gravity = false;
		ent->flags.nodraw = false;
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->position.y += 15.0f;
	}
	else if (ent->ent_type == ENT_Q1_WEAPON_NAILGUN)
	{
		debugf("Loading weapon_nailgun\n");
		ent->model->clone(model_table[MODEL_G_NAIL]);
		ent->rigid->flags.gravity = false;
		ent->flags.nodraw = false;
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->position.y += 15.0f;
	}
	else if (ent->ent_type == ENT_Q1_WEAPON_SUPERNAILGUN)
	{
		debugf("Loading weapon_supernailgun\n");
		ent->model->clone(model_table[MODEL_G_NAIL2]);
		ent->rigid->flags.gravity = false;
		ent->flags.nodraw = false;
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->position.y += 15.0f;
	}
	else if (ent->ent_type == ENT_Q1_WEAPON_SUPERSHOTGUN)
	{
		debugf("Loading weapon_supershotgun\n");
		ent->model->clone(model_table[MODEL_G_SHOT]);
		ent->rigid->flags.gravity = false;
		ent->flags.nodraw = false;
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->position.y += 15.0f;
	}
	else if (ent->ent_type == ENT_Q1_MISC_EXPLOBOX)
	{
		/*
		debugf("Loading misc_explobox\n");
		ent->model->clone(*model_table[MODEL_EXPLOBOX]);
		ent->rigid->gravity = true;
		ent->nodraw = false;
		*/
	}
	else if (ent->ent_type == ENT_Q1_LIGHT_TORCH_SMALL_WALLTORCH)
	{
		debugf("Loading light_torch_small_walltorch\n");
		ent->model->clone(model_table[MODEL_FLAME]);
		ent->rigid->flags.gravity = false;
		ent->flags.nodraw = false;
	}
	else if (ent->ent_type == ENT_Q1_LIGHT_FLAME_SMALL_YELLOW)
	{
		debugf("Loading light_flame_small_yellow\n");
		ent->model->clone(model_table[MODEL_FLAME2]);
		ent->rigid->flags.gravity = false;
		ent->flags.nodraw = false;
	}
	else if (ent->ent_type == ENT_Q1_LIGHT_FLAME_SMALL_WHITE)
	{
		debugf("Loading light_flame_small_yellow\n");
		ent->model->clone(model_table[MODEL_FLAME2]);
		ent->rigid->flags.gravity = false;
		ent->flags.nodraw = false;
	}
	else if (ent->ent_type == ENT_Q1_ITEM_KEY1)
	{
		debugf("Loading item_key1\n");
		ent->model->clone(model_table[MODEL_M_G_KEY]);
		ent->rigid->flags.gravity = false;
		ent->flags.nodraw = false;
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->position.y += 15.0f;
	}
	else if (ent->ent_type == ENT_Q1_ITEM_KEY2)
	{
		debugf("Loading item_key2\n");
		ent->model->clone(model_table[MODEL_M_S_KEY]);
		ent->rigid->flags.gravity = false;
		ent->flags.nodraw = false;
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->position.y += 15.0f;
	}
	else if (ent->ent_type == ENT_Q1_ITEM_ARMOR1)
	{
		debugf("Loading item_armor1\n");
		ent->model->clone(model_table[MODEL_YELLOWARMOR]);
		ent->rigid->flags.gravity = false;
		ent->flags.nodraw = false;
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->position.y += 15.0f;
	}
	else if (ent->ent_type == ENT_Q1_ITEM_ARMOR2)
	{
		debugf("Loading item_armor2\n");
		ent->model->clone(model_table[MODEL_REDARMOR]);
		ent->rigid->flags.gravity = false;
		ent->flags.nodraw = false;
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->position.y += 15.0f;
	}
	else if (ent->ent_type == ENT_Q1_ITEM_SIGIL)
	{
		debugf("Loading item_sigil\n");
		ent->model->clone(model_table[MODEL_END1]);
		ent->rigid->flags.gravity = false;
		ent->flags.nodraw = false;
		ent->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent->position.y += 15.0f;
	}

	/*
	else if (strcmp(value, "ambient_comp_hum") == 0)
	else if (strcmp(value, "ambient_drone") == 0)
	else if (strcmp(value, "ambient_drip") == 0)
	else if (strcmp(value, "ambient_swamp1") == 0)
	else if (strcmp(value, "ambient_swamp2") == 0)
	else if (strcmp(value, "trap_spikeshooter") == 0)


	0 - normal
	1 - flicker (first variety)
	2 - slow strong pulse
	3 - candle (first variety)
	4 - fast strobe
	5 - gentle pulse
	6 - flicker (second variety)
	7 - candle (second variety)
	8 - candle (third variety)
	9 - slow strobe
	10 - flourescent flicker
	11 - slow pulse, not fading to black
	*/
}


///=============================================================================
/// Function: ~Quake3
///=============================================================================
/// Description: Class destructor, only defined to satisfy base class virtual
/// destructor
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
Quake3::~Quake3()
{

}



///=============================================================================
/// Function: handle_func_platform
///=============================================================================
/// Description: Handles func platforms, which sway horizontally periodically
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::handle_func_platform(Entity *ent)
{
}

///=============================================================================
/// Function: handle_func_bobbing
///=============================================================================
/// Description: Handles func bobbing, which sway vertically periodically
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::handle_func_bobbing(Entity *entity)
{
	//Entity *ref = entity;
	int wait = 10;

	sprintf(entity->entstring->target, " ");
	engine->q3map.model_offset[entity->brush_ref] = entity->position - entity->brushinfo->origin;
	entity->rigid->path.loop = 1;

	if (entity->trigger)
	{
		wait = (int)entity->trigger->timeout_value;
	}

	entity->rigid->pid_follow_path(entity->rigid->path.path_list, entity->rigid->path.num_path, 3.0f, 75.0f, wait);
}

///=============================================================================
/// Function: handle_func_train
///=============================================================================
/// Description: Handles func train, which follow a path of nodes, may or may not
/// be in a loop
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::handle_func_train(Entity *entity)
{
	//bool escort = true;
	//Entity *ref = entity;

	engine->q3map.model_offset[entity->brush_ref] = entity->position - entity->brushinfo->origin;


	if (entity->rigid->train.escort)
	{
		vec3 dist;

		int index = engine->find_type(ENT_PLAYER, 0);
		if (index == -1)
			return;

		dist = engine->entity_list[index]->position - entity->position;
		if (dist.magnitude() < 300.0f)
		{
			entity->rigid->pid_follow_path(entity->rigid->path.path_list, entity->rigid->path.num_path,
				entity->rigid->train.speed, entity->rigid->train.path_min_dist, entity->rigid->train.wait);
		}
	}
	else
	{
		entity->rigid->pid_follow_path(entity->rigid->path.path_list, entity->rigid->path.num_path, entity->rigid->train.speed,
			entity->rigid->train.path_min_dist, entity->rigid->train.wait);
	}
	engine->q3map.model_vel[entity->brush_ref] = entity->rigid->velocity;

	for (unsigned int i = 0; i < engine->max_player; i++)
	{
		if (engine->entity_list[i]->rigid == NULL)
			continue;

		// Right now players dont move when on top of train objects, need to find nice way to match velocity
		if (engine->entity_list[i]->rigid->bsp_model_platform == entity->brush_ref)
		{
			//engine->entity_list[i]->rigid->net_force = entity->rigid->net_force;
		}
	}

}

///=============================================================================
/// Function: add_train_path
///=============================================================================
/// Description: Add path node to func train
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
int Quake3::add_train_path(Entity *original, Entity *ref, Entity *target)
{
	if (strlen((ref)->entstring->target) <= 1)
		return 0;

	if (original->rigid->path.num_path == 8)
		return 0;

	if (strcmp(ref->entstring->target, target->entstring->target_name) == 0)
	{
		// Loop detected
		if ((strcmp(ref->entstring->target, original->entstring->target) == 0) && original->rigid->path.num_path != 0)
		{
			return 2;
		}

		original->rigid->path.path_list[original->rigid->path.num_path++] = target->position;
		return 1;
	}
	return 0;
}

///=============================================================================
/// Function: get_gametype
///=============================================================================
/// Description: Get current gametype (for advertising on server browsers)
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
gametype_t Quake3::get_gametype()
{
	return gametype;
}

///=============================================================================
/// Function: get_fraglimit
///=============================================================================
/// Description: Get current frag limit (for advertising on server browsers)
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
int Quake3::get_fraglimit()
{
	return fraglimit;
}

///=============================================================================
/// Function: get_timelimit
///=============================================================================
/// Description: Get current time limit (for advertising on server browsers)
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
int Quake3::get_timelimit()
{
	return timelimit;
}

///=============================================================================
/// Function: get_capturelimit
///=============================================================================
/// Description: Get current capture limit (for advertising on server browsers)
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
int Quake3::get_capturelimit()
{
	return capturelimit;
}

///=============================================================================
/// Function: get_blue_flag_caps
///=============================================================================
/// Description: Get current blue team capture score (for advertising on server browsers)
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
int Quake3::get_blue_flag_caps()
{
	return blue_flag_caps;
}

///=============================================================================
/// Function: get_red_flag_caps
///=============================================================================
/// Description: Get current red team capture score (for advertising on server browsers)
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
int Quake3::get_red_flag_caps()
{
	return red_flag_caps;
}

///=============================================================================
/// Function: get_notif_timer
///=============================================================================
/// Description: gets a notif timer (dont quite remember, but some counter value)
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
int Quake3::get_notif_timer()
{
	return notif_timer;
}

///=============================================================================
/// Function: get_spectator
///=============================================================================
/// Description: gets a current spectator count (for advertising on server browsers)
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
bool Quake3::get_spectator()
{
	return spectator;
}

///=============================================================================
/// Function: get_model_table
///=============================================================================
/// Description: gets model table for rendering in Engine class
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
EntModel *Quake3::get_model_table()
{
	return model_table;
}

///=============================================================================
/// Function: get_num_model
///=============================================================================
/// Description: gets model table size for rendering in Engine class
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
int Quake3::get_num_model()
{
	return num_model;
}

///=============================================================================
/// Function: get_icon_list
///=============================================================================
/// Description: gets icon list for rendering in Engine class
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
vector<icon_t> Quake3::get_icon_list()
{
	return icon_list;
}

///=============================================================================
/// Function: set_gametype
///=============================================================================
/// Description: Set the game type
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::set_gametype(gametype_t &gt)
{
	gametype = gt;
}

///=============================================================================
/// Function: set_fraglimit
///=============================================================================
/// Description: Set the frag limit
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::set_fraglimit(unsigned int limit)
{
	chat_timer = limit;
}

///=============================================================================
/// Function: set_timelimit
///=============================================================================
/// Description: Set the time limit
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::set_timelimit(unsigned int limit)
{
	chat_timer = limit;
}

///=============================================================================
/// Function: set_capturelimit
///=============================================================================
/// Description: Set the capture limit
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::set_capturelimit(unsigned int limit)
{
	chat_timer = limit;
}

///=============================================================================
/// Function: set_chat_timer
///=============================================================================
/// Description: Set the chat timer (minimum delay between chat messages)
///
///
/// Parameters:
///
/// Returns:
///		None
///=============================================================================
void Quake3::set_chat_timer(unsigned int limit)
{
	chat_timer = limit;
}
