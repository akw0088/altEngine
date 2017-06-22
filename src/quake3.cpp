#include "include.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define BOT_ENABLE

#define MACHINEGUN_DAMAGE 7
#define MACHINEGUN_RELOAD 8

#define GAUNTLET_DAMAGE 50
#define GAUNTLET_RELOAD 40

#define SHOTGUN_DAMAGE 10
#define SHOTGUN_RELOAD 60

#define GRENADE_DAMAGE 100
#define GRENADE_SPLASH_DAMAGE 50
#define GRENADE_RELOAD 100

#define ROCKET_DAMAGE 100
#define ROCKET_SPLASH_DAMAGE 50
#define ROCKET_RELOAD 100

#define PLASMA_DAMAGE 20
#define PLASMA_SPLASH_DAMAGE 15
#define PLASMA_RELOAD 8

#define LIGHTNING_DAMAGE 8
#define LIGHTNING_RELOAD 6

#define RAILGUN_DAMAGE 100
#define RAILGUN_RELOAD 188

#define QUAD_FACTOR 3.0f


extern char bot_state_name[16][32];
extern const char *models[23];


Quake3::Quake3()
{
	engine = NULL;
	blink = false;
	spectator = false;
	warmup = true;
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

	played_one_frag = false;
	played_two_frag = false;
	played_three_frag = false;
	played_prepare = false;
	played_sudden = false;
	faceicon = true;

}

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
}

void Quake3::load(gametype_t type)
{
	last_spawn = 0;
	gametype = type;
}

void Quake3::unload()
{
	last_spawn = 0;
	warmup = true;
	warmup_time = 30;
	round_time = 0;
}

void Quake3::destroy()
{

}


void Quake3::load_models(Graphics &gfx)
{
	Model *model;


	#define MODEL_ROCKET 0
	model = new Model;
	model->load(gfx, "media/models/weapons2/rocketl/rocket");
        model_table.push_back(model);

	#define MODEL_GRENADE 1
	model = new Model;
	model->load(gfx, "media/models/weapons2/grenadel/pineapple");
        model_table.push_back(model);

	#define MODEL_SHELL 2
	model = new Model;
	model->load(gfx, "media/models/weapons2/shells/s_shell");
        model_table.push_back(model);


	#define MODEL_BULLET 3
	model = new Model;
	model->load(gfx, "media/models/weapons2/shells/M_shell");
        model_table.push_back(model);


	#define MODEL_BULLET_HIT 4
	model = new Model;
	model->load(gfx, "media/models/weaphits/bullet_hit");
        model_table.push_back(model);

	#define MODEL_PLASMA_HIT 5
	model = new Model;
	model->load(gfx, "media/models/weaphits/plasma_hit");
        model_table.push_back(model);

	#define MODEL_BOOM 6
	model = new Model;
	model->load(gfx, "media/models/weaphits/boom");
        model_table.push_back(model);


	#define MODEL_GIB0 7
	model = new Model;
	model->load(gfx, "media/models/gibs/abdomen");
        model_table.push_back(model);

	#define MODEL_GIB1 8
	model = new Model;
	model->load(gfx, "media/models/gibs/arm");
        model_table.push_back(model);

	#define MODEL_GIB2 9
	model = new Model;
	model->load(gfx, "media/models/gibs/brain");
        model_table.push_back(model);

	#define MODEL_GIB3 10
	model = new Model;
	model->load(gfx, "media/models/gibs/chest");
        model_table.push_back(model);

	#define MODEL_GIB4 11
	model = new Model;
	model->load(gfx, "media/models/gibs/fist");
        model_table.push_back(model);

	#define MODEL_GIB5 12
	model = new Model;
	model->load(gfx, "media/models/gibs/foot");
        model_table.push_back(model);

	#define MODEL_GIB6 13
	model = new Model;
	model->load(gfx, "media/models/gibs/forearm");
        model_table.push_back(model);

	#define MODEL_GIB7 14
	model = new Model;
	model->load(gfx, "media/models/gibs/intestine");
        model_table.push_back(model);

	#define MODEL_GIB8 15
	model = new Model;
	model->load(gfx, "media/models/gibs/leg");
        model_table.push_back(model);

	#define MODEL_GIB9 16
	model = new Model;
	model->load(gfx, "media/models/gibs/skull");
        model_table.push_back(model);

	#define MODEL_BOX 17
	model = new Model;
	model->load(gfx, "media/models/box");
        model_table.push_back(model);

	#define MODEL_BALL 18
	model = new Model;
	model->load(gfx, "media/models/box");
        model_table.push_back(model);

/*
        weapon_gauntlet.load(gfx,   "media/models/weapons2/gauntlet/gauntlet");
        weapon_machinegun.load(gfx, "media/models/weapons2/machinegun/machinegun");
        weapon_shotgun.load(gfx,        "media/models/weapons2/shotgun/shotgun");
        weapon_grenade.load(gfx,        "media/models/weapons2/grenadel/grenade");
        weapon_rocket.load(gfx,         "media/models/weapons2/rocketl/rocketl");
        weapon_lightning.load(gfx,      "media/models/weapons2/lightning/lightning");
        weapon_railgun.load(gfx,        "media/models/weapons2/railgun/railgun");
        weapon_plasma.load(gfx,         "media/models/weapons2/plasma/plasma");
*/

}

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
		if (engine->demo && i != 21)
			continue;


		sprintf(wave.file, "sound/player/%s/death1.wav", models[i]);
		audio.load(wave);
		snd_wave.push_back(wave);
		snd_table[SND_PLAYER + i * 11] = snd_wave.size() - 1;


		sprintf(wave.file, "sound/player/%s/death2.wav", models[i]);
		audio.load(wave);
		snd_wave.push_back(wave);
		snd_table[SND_PLAYER + i * 11 + 1] = snd_wave.size() - 1;


		sprintf(wave.file, "sound/player/%s/death3.wav", models[i]);
		audio.load(wave);
		snd_wave.push_back(wave);
		snd_table[SND_PLAYER + i * 11 + 2] = snd_wave.size() - 1;


		sprintf(wave.file, "sound/player/%s/pain25_1.wav", models[i]);
		audio.load(wave);
		snd_wave.push_back(wave);
		snd_table[SND_PLAYER + i * 11 + 3] = snd_wave.size() - 1;


		sprintf(wave.file, "sound/player/%s/pain50_1.wav", models[i]);
		audio.load(wave);
		snd_wave.push_back(wave);
		snd_table[SND_PLAYER + i * 11 + 4] = snd_wave.size() - 1;

		sprintf(wave.file, "sound/player/%s/pain75_1.wav", models[i]);
		audio.load(wave);
		snd_wave.push_back(wave);
		snd_table[SND_PLAYER + i * 11 + 5] = snd_wave.size() - 1;


		sprintf(wave.file, "sound/player/%s/pain100_1.wav", models[i]);
		audio.load(wave);
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
		audio.load(wave);
		snd_wave.push_back(wave);
		snd_table[SND_PLAYER + i * 11 + 7] = snd_wave.size() - 1;


		sprintf(wave.file, "sound/player/%s/fall1.wav", models[i]);
		audio.load(wave);
		snd_wave.push_back(wave);
		snd_table[SND_PLAYER + i * 11 + 8] = snd_wave.size() - 1;

		sprintf(wave.file, "sound/player/%s/falling1.wav", models[i]);
		audio.load(wave);
		snd_wave.push_back(wave);
		snd_table[SND_PLAYER + i * 11 + 9] = snd_wave.size() - 1;
	}


	strcpy(wave.file, "sound/weapons/railgun/rg_hum.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_RG_HUM 230
	snd_table[SND_RG_HUM] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/weapons/lightning/lg_hum.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_LG_HUM 231
	snd_table[SND_LG_HUM] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/weapons/shotgun/sshotf1b.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_SHOTGUN 232
	snd_table[SND_SHOTGUN] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/weapons/railgun/railgf1a.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_RAILGUN 233
	snd_table[SND_RAILGUN] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/weapons/lightning/lg_fire.wav");
	audio.load(wave);
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
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_ROCKET 235
	snd_table[SND_ROCKET] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/weapons/grenade/grenlf1a.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_GRENADE 236
	snd_table[SND_ROCKET] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/weapons/plasma/hyprbf1a.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_PLASMA 237
	snd_table[SND_PLASMA] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/weapons/machinegun/machgf1b.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_MACHINEGUN 238
	snd_table[SND_MACHINEGUN] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/world/telein.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_TELEIN 239
	snd_table[SND_TELEIN] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/world/teleout.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_TELEOUT 240
	snd_table[SND_TELEOUT] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/teamplay/flagcap_blu.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
	#define SND_FLAGCAP 241
	snd_table[SND_FLAGCAP] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/teamplay/flagtk_blu.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
	#define SND_FLAGTAKE 242
	snd_table[SND_FLAGTAKE] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/teamplay/flagret_blu.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
	#define SND_FLAGRET 243
	snd_table[SND_FLAGRET] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/weapons/grenade/hgrenb1a.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
	#define SND_GRENADE_IMPACT 244
	snd_table[SND_GRENADE_IMPACT] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/weapons/rocket/rocklx1a.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_EXPLODE 245
	snd_table[SND_EXPLODE] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/weapons/rocket/rockfly.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_ROCKETFLY 246
	snd_table[SND_ROCKETFLY] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/weapons/plasma/lasfly.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_PLASMAFLY 247
	snd_table[SND_PLASMAFLY] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/weapons/plasma/plasmx1a.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_PLASMA_EXPLODE 248
	snd_table[SND_PLASMA_EXPLODE] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/weapons/change.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_SWAPWEAP 249
	snd_table[SND_SWAPWEAP] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/items/use_medkit.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_USE_MED 250
	snd_table[SND_USE_MED] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/items/use_nothing.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_USE_EMPTY 251
	snd_table[SND_USE_EMPTY] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/items/regen.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_REGEN 252
	snd_table[SND_REGEN] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/player/footsteps/step1.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_STEP1 253
	snd_table[SND_STEP1] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/player/footsteps/step2.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_STEP2 254
	snd_table[SND_STEP2] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/player/footsteps/step3.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_STEP3 255
	snd_table[SND_STEP3] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/player/footsteps/step4.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_STEP4 256
	snd_table[SND_STEP4] = snd_wave.size() - 1;


	sprintf(wave.file, "sound/player/land1.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_LAND 257
	snd_table[SND_LAND] = snd_wave.size() - 1;


	sprintf(wave.file, "sound/world/jumppad.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_JUMPPAD 258
	snd_table[SND_JUMPPAD] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/weapons/noammo.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_NOAMMO 259
	snd_table[SND_NOAMMO] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/player/watr_in.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_WATER_IN 260
	snd_table[SND_WATER_IN] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/player/watr_out.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_WATER_OUT 261
	snd_table[SND_WATER_OUT] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/player/gurp1.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_GURP1 262
	snd_table[SND_GURP1] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/player/gurp2.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_GURP2 263
	snd_table[SND_GURP2] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/player/gibsplt1.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_GIBBED 264
	snd_table[SND_GIBBED] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/player/gibimp1.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_GIB1 265
	snd_table[SND_GIB1] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/player/gibimp2.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_GIB2 266
	snd_table[SND_GIB2] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/player/gibimp3.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_GIB3 267
	snd_table[SND_GIB3] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/player/talk.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
	#define SND_TALK 268
	snd_table[SND_TALK] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/feedback/blueleads.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_BLUE_LEAD 269
	snd_table[SND_BLUE_LEAD] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/feedback/redleads.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_RED_LEAD 270
	snd_table[SND_RED_LEAD] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/feedback/tiedlead.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_TIED_LEAD 271
	snd_table[SND_TIED_LEAD] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/feedback/lostlead.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_LOST_LEAD 272
	snd_table[SND_LOST_LEAD] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/feedback/takenlead.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_TAKEN_LEAD 273
	snd_table[SND_TAKEN_LEAD] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/teamstied.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_TEAMS_TIED 274
	snd_table[SND_TEAMS_TIED] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/excellent.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_EXCELLENT 275
	snd_table[SND_EXCELLENT] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/feedback/gauntlet.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_GAUNTLET_MEDAL 276
	snd_table[SND_GAUNTLET_MEDAL] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/humiliation.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_HUMILIATION 277
	snd_table[SND_HUMILIATION] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/impressive_a.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_IMPRESSIVEA 278
	snd_table[SND_IMPRESSIVEA] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/accuracy.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_ACCURACY 279
	snd_table[SND_ACCURACY] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/excellent_a.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_EXCELLENTA 280
	snd_table[SND_EXCELLENTA] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/frags.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_FRAGS 281
	snd_table[SND_FRAGS] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/impressive.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_IMPRESSIVE 282
	snd_table[SND_IMPRESSIVE] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/feedback/perfect.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_PERFECT 283
	snd_table[SND_PERFECT] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/hit.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_HIT 284
	snd_table[SND_HIT] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/hit_teammate.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_HITTEAM 285
	snd_table[SND_HITTEAM] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/sudden_death.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_SUDDEN 286
	snd_table[SND_SUDDEN] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/one.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_ONE 287
	snd_table[SND_ONE] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/two.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_TWO 288
	snd_table[SND_TWO] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/three.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_THREE 289
	snd_table[SND_THREE] = snd_wave.size() - 1;



	strcpy(wave.file, "sound/feedback/denied.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_DENIED 290
	snd_table[SND_DENIED] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/fight.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_FIGHT 291
	snd_table[SND_FIGHT] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/feedback/prepare.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_PREPARE 292
	snd_table[SND_PREPARE] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/feedback/1_minute.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_ONE_MINUTE 293
	snd_table[SND_ONE_MINUTE] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/feedback/5_minute.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_FIVE_MINUTE 294
	snd_table[SND_FIVE_MINUTE] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/1_frag.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_ONE_FRAG 295
	snd_table[SND_ONE_FRAG] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/2_frags.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_TWO_FRAG 296
	snd_table[SND_TWO_FRAG] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/feedback/3_frags.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_THREE_FRAG 297
	snd_table[SND_THREE_FRAG] = snd_wave.size() - 1;

	strcpy(wave.file, "media/sound/shell_quiet.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_SHELL 298
	snd_table[SND_SHELL] = snd_wave.size() - 1;

	strcpy(wave.file, "media/sound/bullet_quiet.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_BULLET 299
	snd_table[SND_BULLET] = snd_wave.size() - 1;

	strcpy(wave.file, "media/sound/dropgun_quiet.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_DROPGUN 300
	snd_table[SND_DROPGUN] = snd_wave.size() - 1;


	strcpy(wave.file, "sound/soldier/sight1.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_SOLDIER_SIGHT 301
	snd_table[SND_SOLDIER_SIGHT] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/soldier/idle.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_SOLDIER_IDLE 302
	snd_table[SND_SOLDIER_IDLE] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/weapons/melee/fstrun.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_GAUNTLET 303
	snd_table[SND_GAUNTLET] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/weapons/melee/fsthum.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_GAUNTLET_IDLE 304
	snd_table[SND_GAUNTLET_IDLE] = snd_wave.size() - 1;

	strcpy(wave.file, "sound/weapons/melee/fstatck.wav");
	audio.load(wave);
	snd_wave.push_back(wave);
#define SND_GAUNTLET_HIT 305
	snd_table[SND_GAUNTLET_HIT] = snd_wave.size() - 1;

}

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

void Quake3::add_player(vector<Entity *> &entity_list, playertype_t player_type, int &ent_id, char *player_name)
{
	int spawn_type = -1;
	bool local = false;
	team_t team = get_team();


	if (team == TEAM_RED)
	{
		spawn_type = ENT_TEAM_CTF_REDSPAWN;
		num_player_red++;
	}
	else if (team == TEAM_BLUE)
	{
		spawn_type = ENT_TEAM_CTF_BLUESPAWN;
		num_player_blue++;
	}
	else if (team == TEAM_NONE)
	{
		spawn_type = ENT_INFO_PLAYER_DEATHMATCH;
	}
	else
	{
		spawn_type = ENT_UNKNOWN;
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
				engine->entity_list[spawn]->ent_type = ENT_SPECTATOR;
				break;
			case UNKNOWN:
				strcpy(entity_list[spawn]->type, "unknown");
				break;
			}
			Entity *spawn_ent = entity_list[spawn];

			spawn_ent->position = entity_list[i]->position;
			spawn_ent->rigid = new RigidBody(entity_list[spawn]);
			spawn_ent->model = entity_list[spawn]->rigid;
			spawn_ent->rigid->clone(*(engine->thug22->model));
			spawn_ent->rigid->step_flag = true;
			spawn_ent->player = new Player(entity_list[spawn], engine->gfx, engine->audio, 21, team);
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

			//set spawn angle
			switch (entity_list[i]->angle)
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
			}
			engine->camera_frame.forward.x = matrix.m[8];
			engine->camera_frame.forward.y = matrix.m[9];
			engine->camera_frame.forward.z = matrix.m[10];
			engine->camera_frame.up = vec3(0.0f, 1.0f, 0.0f);
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


void Quake3::handle_player(int self, input_t &input)
{
	Entity *entity = engine->entity_list[self];
	static int footstep_num = 0;
	static bool zoomed = false;
#if 0
	static int last_tick = 0;
#endif

	if (entity->player == NULL)
		return;

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


	entity->player->alive_timer++;

	if (entity->player->pain_timer == 0)
	{
		if (entity->rigid->lava)
		{
			console(self, "damage 20", engine->menu, engine->entity_list);
			entity->player->pain_timer = (TICK_RATE >> 1);
		}
		else if (entity->rigid->slime)
		{
			console(self, "damage 10", engine->menu, engine->entity_list);
			entity->player->pain_timer = (TICK_RATE >> 1);
		}
	}

	if (entity->player->telefragged)
	{
		char msg[512];
		Entity *entity2 = entity->player->telefragged;
		entity->player->telefragged = NULL;


		if (round_time > 1)
		{
			entity->player->health = -100;
			entity->player->stats.deaths++;
			entity2->player->stats.kills++;

			sprintf(msg, "%s telefragged %s\n", entity2->player->name, entity->player->name);
			debugf(msg);
			engine->menu.print_notif(msg);
			notif_timer = 3 * TICK_RATE;
			handle_frags_left(*(entity2->player));
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

	if (entity->rigid->hard_impact)
	{
		if (entity->rigid->impact_velocity < -FALL_DAMAGE_VELOCITY)
		{
			entity->rigid->hard_impact = false;

			if (entity->player->fall_timer == 0)
			{
				entity->player->fall_timer = TICK_RATE >> 4;
				entity->player->health -= 10;
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
			entity->rigid->hard_impact = false;


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

			if ((entity->player && entity->player->type == PLAYER) || (engine->server_flag && (entity->player && entity->player->type == CLIENT)))
			{
				float speed_scale = 1.0f;

				if (entity->player->haste_timer > 0)
					speed_scale = 2.0f;

				if (entity->rigid->move(input, speed_scale))
				{
					entity->player->state = PLAYER_JUMPED;
					if (entity->player->local)
						engine->play_wave_global(entity->player->model_index * SND_PLAYER + SND_JUMP);
					else
						engine->play_wave(entity->position, entity->player->model_index * SND_PLAYER + SND_JUMP);

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
				char cmd[128] = { 0 };

				sprintf(cmd, "respawn %d %d", -1, self);
				console(self, cmd, engine->menu, engine->entity_list);
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
			entity->light = new Light(entity, engine->gfx, 999, engine->res_scale);
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
		entity->rigid->blend = true;
	}
	else
	{
		entity->rigid->blend = false;
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

	if (entity->rigid->on_ground && entity->rigid->gravity == true &&
		entity->rigid->water == false && entity->player->state != PLAYER_DEAD &&
		entity->rigid->noclip == false)
	{
		// Footstep sounds
		if ((entity->position - entity->rigid->old_position).magnitude() > 0.8f && engine->tick_num % 20 == 0)
		{
			switch (footstep_num++ % 4)
			{
			case 0:
				if (entity->player->local)
					engine->play_wave_global(SND_STEP1);
				else
					engine->play_wave(entity->position, SND_STEP1);
				break;
			case 1:
				if (entity->player->local)
					engine->play_wave_global(SND_STEP2);
				else
					engine->play_wave(entity->position, SND_STEP2);
				break;
			case 2:
				if (entity->player->local)
					engine->play_wave_global(SND_STEP3);
				else
					engine->play_wave(entity->position, SND_STEP3);
				break;
			case 3:
				if (entity->player->local)
					engine->play_wave_global(SND_STEP4);
				else
					engine->play_wave(entity->position, SND_STEP4);
				break;
			}
		}
	}

	//Water sounds
	if (entity->rigid->water && entity->rigid->water_depth < entity->rigid->get_height())
	{
		if (entity->rigid->water != entity->rigid->last_water)
		{
			if (entity->player->local)
				engine->play_wave_global(SND_WATER_IN);
			else
				engine->play_wave(entity->position, SND_WATER_IN);

			entity->rigid->last_water = entity->rigid->water;
		}
	}
	else if (entity->rigid->water == false)
	{
		if (entity->rigid->water != entity->rigid->last_water)
		{
			if (entity->player->local)
				engine->play_wave_global(SND_WATER_OUT);
			else
				engine->play_wave(entity->position, SND_WATER_OUT);
			entity->rigid->last_water = entity->rigid->water;
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

				entity->player->health -= 15;
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

	handle_weapons(*(entity->player), input, self, engine->client_flag);

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
	entity->model->clone(*(model_table[MODEL_BOX]));
}

void Quake3::drop_weapon(int index)
{
	Entity *entity = engine->entity_list[index];
	Entity *drop_weapon = engine->entity_list[engine->get_entity()];
	char *weapon_str = NULL;


	drop_weapon->rigid = new RigidBody(drop_weapon);
	drop_weapon->position = entity->position;
	drop_weapon->model = drop_weapon->rigid;
	drop_weapon->rigid->impact_index = SND_DROPGUN;

	switch (entity->player->current_weapon)
	{
	case wp_machinegun:
		weapon_str = "weapon_machinegun";
		drop_weapon->model->clone(entity->player->weapon_machinegun);
		drop_weapon->nettype = NT_MACHINEGUN;
		break;
	case wp_shotgun:
		weapon_str = "weapon_shotgun";
		drop_weapon->model->clone(entity->player->weapon_shotgun);
		drop_weapon->nettype = NT_SHOTGUN;
		break;
	case wp_grenade:
		weapon_str = "weapon_grenadelauncher";
		drop_weapon->model->clone(entity->player->weapon_grenade);
		drop_weapon->nettype = NT_GRENADE_LAUNCHER;
		break;
	case wp_rocket:
		weapon_str = "weapon_rocketlauncher";
		drop_weapon->model->clone(entity->player->weapon_rocket);
		drop_weapon->nettype = NT_ROCKET_LAUNCHER;
		break;
	case wp_plasma:
		weapon_str = "weapon_plasmagun";
		drop_weapon->model->clone(entity->player->weapon_plasma);
		drop_weapon->nettype = NT_PLASMAGUN;
		break;
	case wp_lightning:
		weapon_str = "weapon_lightning";
		drop_weapon->model->clone(entity->player->weapon_lightning);
		drop_weapon->nettype = NT_LIGHTNINGGUN;
		break;
	case wp_railgun:
		weapon_str = "weapon_railgun";
		drop_weapon->model->clone(entity->player->weapon_railgun);
		drop_weapon->nettype = NT_RAILGUN;
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
	drop_weapon->rigid->translational_friction_flag = true;
	drop_weapon->rigid->rotational_friction_flag = true;



	vec3 forward;
	entity->model->getForwardVector(forward);
	drop_weapon->rigid->velocity = forward *-1.0f;
	drop_weapon->rigid->angular_velocity.x = 10.0f;

	//  Prevent player from picking weapon back up
	entity->player->state = PLAYER_DEAD;

	drop_weapon->trigger = new Trigger(drop_weapon, engine->audio);
	drop_weapon->trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
	drop_weapon->trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
	sprintf(drop_weapon->trigger->action, "%s", weapon_str);
}

void Quake3::drop_powerup(vec3 &position, char *model, char *action)
{
	Entity *drop = engine->entity_list[engine->get_entity()];
	drop->position = position;
	drop->nettype = NT_QUAD;


	drop->rigid = new RigidBody(drop);
	drop->model = drop->rigid;
	drop->model->load(engine->gfx, model);

	drop->rigid->velocity = vec3(0.0f, 2.0f, 0.0);
	drop->rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
	drop->trigger = new Trigger(drop, engine->audio);
	drop->trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
	drop->trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
	strcpy(drop->trigger->action, action);
}

void Quake3::step(int frame_step)
{
	unsigned int num_bot = 3;

	if (engine->entity_list.size() == 0)
		return;

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
						Player *player = engine->entity_list[i]->player;

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


	if (engine->server_flag == false && engine->client_flag == false && engine->num_bot < num_bot)
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

	if (spectator == true)
	{
		engine->camera_frame.update(engine->input);
	}

	if (engine->input.control && spectator_timer <= 0)
	{
		spectator_timer = TICK_RATE;
		spectator = !spectator;
		printf("spectator is %d\n", (int)spectator);

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
	}
	else
	{
		if (spectator_timer > 0)
			spectator_timer--;
	}


	for (unsigned int i = 0; i < engine->max_player; i++)
	{
		Entity *entity = engine->entity_list[i];
		bool isplayer = (entity->player && entity->player->type == PLAYER);
		bool isbot = (entity->player && entity->player->type == BOT);
		bool isclient = (entity->player && entity->player->type == CLIENT);
		bool isserver = (entity->player && entity->player->type == SERVER);


		if (isplayer || isbot || isserver)
		{
			if (((isplayer || isserver) &&
				(engine->menu.ingame == true || engine->menu.console == true || engine->menu.chatmode == true)))
			{
				input_t noinput = { 0 };

				handle_player(i, noinput);
			}
			else
			{
				handle_player(i, engine->input);
			}
		}

		if (isclient)
		{
			for(unsigned int j = 0; j < engine->client_list.size(); j++)
			{
				if ((unsigned int)engine->client_list[j]->ent_id == i)
					handle_player(i, engine->client_list[j]->input);
			}
		}

		if (entity->player == NULL)
			continue;

		if (entity->player->type != BOT)
			continue;

#ifdef BOT_ENABLE

		Entity *bot = engine->entity_list[i];
		input_t input;

		memset(&input, 0, sizeof(input_t));
		if (bot->player->health <= 0)
		{
			if (bot->player->bot_state != BOT_DEAD)
				bot->player->dead_timer = random() * (TICK_RATE << 1);
			bot->player->bot_state = BOT_DEAD;
			continue;
		}

		//bot->player->avoid_walls(engine->q3map);

		bot->player->handle_bot(engine->entity_list, i);

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
			speed_scale = 2.0f;

		switch (bot->player->bot_state)
		{
		case BOT_ATTACK:
			engine->zcc.select_animation(0);
			bot->rigid->move_forward(speed_scale);
			break;
		case BOT_DEAD:
			engine->zcc.select_animation(1);
			bot->model->clone(*(model_table[MODEL_BOX]));
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
			if (bot->player->path.length == 0)
			{
				//probably need to pass Player reference instead of each path param, or make a struct
				ret = bot_get_path(bot->player->get_item, i, nav_array,
					bot->player->path);

				if (bot->player->path.length == -1)
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

	// handles triggers and the projectile as trigger stuff
	#pragma omp parallel for num_threads(8)
	for (unsigned int i = 0; i < engine->max_player; i++)
	{
		Player *player = engine->entity_list[i]->player;
#ifdef OPENMP
		int thread_num = omp_get_thread_num();
		int num_thread = omp_get_num_threads();

		if (engine->entity_list[i]->bsp_leaf % num_thread != thread_num)
			continue;

//		printf("bsp leaf %d Handled by thread %d of %d\n", engine->entity_list[i]->bsp_leaf, thread_num, num_thread);
#endif



		if (player && player->type == PLAYER)
			check_triggers(i, engine->entity_list);
		if (engine->server_flag && player && player->type == CLIENT)
			check_triggers(i, engine->entity_list);
		else if (player && player->type == BOT)
			check_triggers(i, engine->entity_list);
	}

}

void Quake3::handle_plasma(Player &player, int self, bool client)
{
	Frame camera_frame;

	player.entity->model->get_frame(camera_frame);
	player.reload_timer = PLASMA_RELOAD;
	player.ammo_plasma--;

	float quad_factor = 1.0f;

	if (player.quad_timer > 0)
		quad_factor = QUAD_FACTOR;

	if (client == false)
	{
		Entity *projectile = engine->entity_list[engine->get_entity()];

		projectile->nettype = NT_PLASMA;
		projectile->rigid = new RigidBody(projectile);
		projectile->model = projectile->rigid;
		projectile->position = camera_frame.pos;
		camera_frame.set(projectile->model->morientation);
		projectile->visible = true; // accomodate for low spatial testing rate
		projectile->bsp_leaf = player.entity->bsp_leaf;
		projectile->bsp_visible = player.entity->bsp_visible = true;
		projectile->rigid->blend = true;

		projectile->rigid->clone(*(model_table[MODEL_BALL]));
		projectile->rigid->velocity = camera_frame.forward * -10.0f;
		projectile->rigid->net_force = camera_frame.forward * -10.0f;

		projectile->rigid->angular_velocity = vec3();
		projectile->rigid->gravity = false;
		projectile->trigger = new Trigger(projectile, engine->audio);
		projectile->trigger->projectile = true;

		projectile->rigid->impact_index = SND_PLASMA_EXPLODE;
		projectile->trigger->explode_index = SND_PLASMA_EXPLODE;
		projectile->trigger->idle_index = SND_PLASMAFLY;

		sprintf(projectile->trigger->action, "damage %d", (int)(PLASMA_DAMAGE * quad_factor));

		projectile->trigger->hide = false;
		projectile->trigger->radius = 25.0f;
		projectile->trigger->idle = true;
		projectile->trigger->explode = false;
		projectile->trigger->explode_timer = 10;
		projectile->trigger->explode_color = vec3(0.0f, 0.0f, 1.0f);
		projectile->trigger->explode_intensity = 200.0f;
		projectile->trigger->splash_damage = (int)(PLASMA_SPLASH_DAMAGE * quad_factor);
		projectile->trigger->splash_radius = 75.0f;
		projectile->trigger->knockback = 10.0f;
		projectile->trigger->owner = self;


		projectile->light = new Light(projectile, engine->gfx, 999, engine->res_scale);
		projectile->light->color = vec3(0.0f, 0.0f, 1.0f);
		projectile->light->intensity = 1000.0f;
	}

	add_decal(player.entity->position, camera_frame, *(model_table[MODEL_PLASMA_HIT]), -40.0f, true, 10);

	Entity *muzzleflash = engine->entity_list[engine->get_entity()];
	muzzleflash->position = player.entity->position + camera_frame.forward * -75.0f;
	muzzleflash->light = new Light(muzzleflash, engine->gfx, 999, engine->res_scale);
	muzzleflash->light->color = vec3(0.6f, 0.6f, 1.0f);
	muzzleflash->light->intensity = 2000.0f;
	muzzleflash->light->attenuation = 0.0625f;
	muzzleflash->light->timer_flag = true;
	muzzleflash->light->timer = (int)(0.125f * TICK_RATE);
	muzzleflash->visible = true; // accomodate for low spatial testing rate
	muzzleflash->bsp_leaf = player.entity->bsp_leaf;
	muzzleflash->bsp_visible = player.entity->bsp_visible = true;
}

void Quake3::handle_rocketlauncher(Player &player, int self, bool client)
{
	Frame camera_frame;

	player.entity->model->get_frame(camera_frame);
	player.reload_timer = ROCKET_RELOAD;
	player.ammo_rockets--;

	float quad_factor = 1.0f;

	if (player.quad_timer > 0)
		quad_factor = QUAD_FACTOR;


	if (client == false)
	{
		Entity *projectile = engine->entity_list[engine->get_entity()];
		projectile->nettype = NT_ROCKET;
		projectile->position = camera_frame.pos;
		projectile->visible = true; // accomodate for low spatial testing rate
		projectile->bsp_leaf = player.entity->bsp_leaf;
		projectile->bsp_visible = player.entity->bsp_visible = true;

		projectile->trigger = new Trigger(projectile, engine->audio);
		projectile->trigger->projectile = true;
		projectile->trigger->explode_index = SND_EXPLODE;
		projectile->trigger->idle_index = SND_ROCKETFLY;

		sprintf(projectile->trigger->action, "damage %d", (int)(ROCKET_DAMAGE * quad_factor));

		projectile->trigger->hide = false;
		projectile->trigger->radius = 25.0f;
		projectile->trigger->idle = true;
		projectile->trigger->explode = true;
		projectile->trigger->idle_timer = 0;
		projectile->trigger->num_bounce = 0;
		projectile->trigger->explode_timer = 10;
		projectile->trigger->explode_color = vec3(1.0f, 0.0f, 0.0f);
		projectile->trigger->explode_intensity = 500.0f;
		projectile->trigger->splash_damage = (int)(ROCKET_SPLASH_DAMAGE * quad_factor);
		projectile->trigger->splash_radius = 250.0f;
		projectile->trigger->knockback = 250.0f;
		projectile->trigger->owner = self;

		projectile->particle_on = true;
		projectile->num_particle = 5000;

		projectile->light = new Light(projectile, engine->gfx, 999, engine->res_scale);
		projectile->light->color = vec3(1.0f, 1.0f, 1.0f);
		projectile->light->intensity = 1000.0f;

		projectile->rigid = new RigidBody(projectile);
		projectile->model = projectile->rigid;
		camera_frame.set(projectile->rigid->morientation);
		projectile->rigid->clone(*(model_table[MODEL_ROCKET]));
		projectile->rigid->velocity = camera_frame.forward * -6.25f;
		projectile->rigid->net_force = camera_frame.forward * -10.0f;
		projectile->rigid->angular_velocity = vec3();
		projectile->rigid->gravity = false;

		projectile->trigger->create_sources(engine->audio);

		engine->play_wave_source(projectile->trigger->loop_source, projectile->trigger->idle_index);
	}

	Entity *muzzleflash = engine->entity_list[engine->get_entity()];
	muzzleflash->position = player.entity->position + camera_frame.forward * -75.0f;
	muzzleflash->light = new Light(muzzleflash, engine->gfx, 999, engine->res_scale);
	muzzleflash->light->color = vec3(1.0f, 0.75f, 0.0f);
	muzzleflash->light->intensity = 2000.0f;
	muzzleflash->light->attenuation = 0.0625f;
	muzzleflash->light->timer_flag = true;
	muzzleflash->light->timer = (int)(0.125f * TICK_RATE);
	muzzleflash->visible = true; // accomodate for low spatial testing rate
	muzzleflash->bsp_leaf = player.entity->bsp_leaf;
	muzzleflash->bsp_visible = player.entity->bsp_visible = true;

}

void Quake3::handle_grenade(Player &player, int self, bool client)
{
	Frame camera_frame;

	player.entity->model->get_frame(camera_frame);
	player.reload_timer = GRENADE_RELOAD;
	player.ammo_grenades--;

	if (client == false)
	{
		Entity *projectile = engine->entity_list[engine->get_entity()];
		projectile->nettype = NT_GRENADE;

		projectile->rigid = new RigidBody(projectile);
		projectile->model = projectile->rigid;
		projectile->position = camera_frame.pos;
		camera_frame.set(projectile->model->morientation);
		projectile->visible = true; // accomodate for low spatial testing rate
		projectile->bsp_leaf = player.entity->bsp_leaf;
		projectile->bsp_visible = player.entity->bsp_visible = true;

		projectile->rigid->clone(*(model_table[MODEL_GRENADE]));
		projectile->rigid->velocity = camera_frame.forward * -25.0f;
		projectile->rigid->angular_velocity = vec3(1.1f, 0.1f, 1.1f);
		projectile->rigid->gravity = true;
		projectile->rigid->ground_friction_flag = true;
		projectile->rigid->rotational_friction_flag = true;
		//entity->rigid->set_target(*(entity_list[spawn]));

		projectile->particle_on = true;
		projectile->num_particle = 5000;


		float quad_factor = 1.0f;

		if (player.quad_timer > 0)
			quad_factor = QUAD_FACTOR;


		projectile->trigger = new Trigger(projectile, engine->audio);
		projectile->trigger->projectile = true;
		projectile->trigger->num_bounce = 20;
		projectile->trigger->explode_index = SND_EXPLODE;
		sprintf(projectile->trigger->action, "damage %d", (int)(GRENADE_DAMAGE * quad_factor));

		projectile->trigger->hide = false;
		projectile->trigger->radius = 50.0f;
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
	}


	Entity *muzzleflash = engine->entity_list[engine->get_entity()];
	muzzleflash->position = player.entity->position + camera_frame.forward * -75.0f;
	muzzleflash->light = new Light(muzzleflash, engine->gfx, 999, engine->res_scale);
	muzzleflash->light->color = vec3(1.0f, 0.7f, 0.0f);
	muzzleflash->light->intensity = 2000.0f;
	muzzleflash->light->attenuation = 0.0625f;
	muzzleflash->light->timer_flag = true;
	muzzleflash->light->timer = (int)(0.125f * TICK_RATE);
	muzzleflash->visible = true; // accomodate for low spatial testing rate
	muzzleflash->bsp_leaf = player.entity->bsp_leaf;
	muzzleflash->bsp_visible = player.entity->bsp_visible = true;

}

void Quake3::handle_lightning(Player &player, int self, bool client)
{
	Frame camera_frame;
	int index[16] = { -1 };
	int num_index = 0;

	player.entity->model->get_frame(camera_frame);


	if (player.entity->rigid->water && player.entity->rigid->water_depth >= 25.0f)
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
				Player *enemy = ent->player;

				enemy->health -= enemy->ammo_lightning * LIGHTNING_DAMAGE;
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
		projectile->nettype = NT_LIGHTNING;
		projectile->rigid = new RigidBody(projectile);
		projectile->position = camera_frame.pos;
		projectile->rigid->clone(*(model_table[MODEL_BOX]));
		projectile->rigid->velocity = vec3();
		projectile->rigid->angular_velocity = vec3();
		projectile->rigid->gravity = false;
		projectile->rigid->bounce = 2;
	//	projectile->rigid->rotational_friction_flag = true;
		projectile->rigid->lightning_trail = true;
		projectile->model = projectile->rigid;
	//	projectile->rigid->set_target(*(engine->entity_list[self]));
		camera_frame.set(projectile->model->morientation);
		projectile->visible = true; // accomodate for low spatial testing rate
		projectile->rigid->noclip = true;
		projectile->bsp_leaf = player.entity->bsp_leaf;
		projectile->bsp_visible = player.entity->bsp_visible = true;

		/*
		projectile->light = new Light(projectile, engine->gfx, 999);
		projectile->light->color = vec3(1.0f, 1.0f, 1.0f);
		projectile->light->intensity = 1000.0f;
		*/

		projectile->trigger = new Trigger(projectile, engine->audio);
		projectile->trigger->projectile = true;
		projectile->trigger->action[0] = '\0';

//		projectile->rigid->bounce = 5;
		projectile->trigger->hide = false;
		projectile->trigger->radius = 25.0f;
		projectile->trigger->idle = true;
		projectile->trigger->idle_timer = (int)(0.025 * TICK_RATE);
		projectile->trigger->explode = true;
		projectile->trigger->explode_timer = 5;
		projectile->trigger->owner = self;



		engine->hitscan(player.entity->position, forward, index, num_index, self, 768.0f);
		for (int i = 0; i < num_index; i++)
		{
			char cmd[512] = { 0 };
			Player *target = engine->entity_list[index[i]]->player;

			if (target == NULL)
				continue;

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

				if (target->health <= -50)
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

	Entity *muzzleflash = engine->entity_list[engine->get_entity()];
	muzzleflash->position = player.entity->position + camera_frame.forward * -75.0f;
	muzzleflash->light = new Light(muzzleflash, engine->gfx, 999, engine->res_scale);
	muzzleflash->light->color = vec3(0.6f, 0.6f, 1.0f);
	muzzleflash->light->intensity = 2000.0f;
	muzzleflash->light->attenuation = 0.0625f;
	muzzleflash->light->timer_flag = true;
	muzzleflash->light->timer = (int)(0.125f * TICK_RATE);
	muzzleflash->visible = true; // accomodate for low spatial testing rate
	muzzleflash->bsp_leaf = player.entity->bsp_leaf;
	muzzleflash->bsp_visible = player.entity->bsp_visible = true;


}

void Quake3::handle_railgun(Player &player, int self, bool client)
{
	int index[16] = { -1 };
	int num_index = 0;

	Frame camera_frame;

	player.entity->model->get_frame(camera_frame);
	player.reload_timer = RAILGUN_RELOAD;
	player.ammo_slugs--;


	if (client == false)
	{
		Entity *projectile = engine->entity_list[engine->get_entity()];
		projectile->nettype = NT_RAIL;
		projectile->rigid = new RigidBody(projectile);
		projectile->position = camera_frame.pos;
		projectile->rigid->clone(*(model_table[MODEL_BALL]));
		projectile->rigid->velocity = vec3();
		projectile->rigid->angular_velocity = vec3();
		projectile->rigid->gravity = false;
		projectile->rigid->bounce = 5;
		projectile->model = projectile->rigid;
		projectile->model->rail_trail = true;
		projectile->rigid->noclip = true;

		camera_frame.set(projectile->model->morientation);
		projectile->visible = true; // accomodate for low spatial testing rate
		projectile->bsp_leaf = player.entity->bsp_leaf;
		projectile->bsp_visible = player.entity->bsp_visible = true;

		projectile->trigger = new Trigger(projectile, engine->audio);
		projectile->trigger->projectile = true;
		sprintf(projectile->trigger->action, " ");

		projectile->trigger->hide = false;
		projectile->trigger->radius = 25.0f;
		projectile->trigger->idle = true;
		projectile->trigger->idle_timer = (int)(1.0 * TICK_RATE);
		projectile->trigger->explode = true;
		projectile->trigger->explode_timer = 10;
		projectile->trigger->owner = self;

		vec3 forward;
		player.entity->model->getForwardVector(forward);

		float quad_factor = 1.0f;

		if (player.quad_timer > 0)
			quad_factor = QUAD_FACTOR;

		engine->hitscan(player.entity->position, forward, index, num_index, self, -1.0f);
		if (num_index == 0)
			player.impressive_count = 0;


		for (int i = 0; i < num_index; i++)
		{
			Player *target = engine->entity_list[index[i]]->player;
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

				if (target->health <= -50)
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


	Entity *muzzleflash = engine->entity_list[engine->get_entity()];
	muzzleflash->position = player.entity->position + camera_frame.forward * -75.0f;
	muzzleflash->light = new Light(muzzleflash, engine->gfx, 999, engine->res_scale);
	muzzleflash->light->color = vec3(1.0f, 0.5f, 0.0f);
	muzzleflash->light->intensity = 2000.0f;
	muzzleflash->light->attenuation = 0.0625f;
	muzzleflash->light->timer_flag = true;
	muzzleflash->light->timer = (int)(0.125f * TICK_RATE);
	muzzleflash->visible = true; // accomodate for low spatial testing rate
	muzzleflash->bsp_leaf = player.entity->bsp_leaf;
	muzzleflash->bsp_visible = player.entity->bsp_visible = true;

}

void Quake3::handle_gauntlet(Player &player, int self, bool client)
{
	char cmd[64] = { 0 };
	int index[16] = { -1 };
	int num_index = 0;

	Frame camera_frame;
	vec3 dir;
	vec3 end;


	player.reload_timer = 5; // prevent doing gauntlet hitscan every frame

	player.entity->model->get_frame(camera_frame);
	camera_frame.forward *= -1;
	player.ammo_bullets--;



	Entity *muzzleflash = engine->entity_list[engine->get_entity()];
	muzzleflash->position = player.entity->position + camera_frame.forward * 75.0f;
	muzzleflash->light = new Light(muzzleflash, engine->gfx, 999, engine->res_scale);
	muzzleflash->light->color = vec3(1.0f, 1.0f, 0.0f);
	muzzleflash->light->intensity = 2000.0f;
	muzzleflash->light->attenuation = 0.0625f;
	muzzleflash->light->timer_flag = true;
	muzzleflash->light->timer = (int)(0.125f * TICK_RATE);
	muzzleflash->visible = true; // accomodate for low spatial testing rate
	muzzleflash->bsp_leaf = player.entity->bsp_leaf;
	muzzleflash->bsp_visible = player.entity->bsp_visible = true;



	if (client == false)
	{
		engine->hitscan(player.entity->position, camera_frame.forward, index, num_index, self, 75.0f);

		float quad_factor = 1.0f;

		if (player.quad_timer > 0)
			quad_factor = QUAD_FACTOR;

		for (int i = 0; i < num_index; i++)
		{
			Player *target = engine->entity_list[index[i]]->player;

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
			debugf("%s has %d health\n", target->name,
				target->health);
			player.stats.hits++;

			if (target->health <= 0 && target->state != PLAYER_DEAD)
			{
				char msg[256];
				char word[16] = { 0 };

				player.stats.kills++;
				target->stats.deaths++;

				if (target->health <= -50)
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

void Quake3::handle_machinegun(Player &player, int self, bool client)
{
	char cmd[64] = { 0 };
	int index[16] = { -1 };
	int num_index = 0;

	Frame camera_frame;
	vec3 dir;
	vec3 end;


	player.entity->model->get_frame(camera_frame);
	camera_frame.forward *= -1;
	player.reload_timer = MACHINEGUN_RELOAD;
	player.ammo_bullets--;


	// Added to end vector
	int spread = 200;
	float r = random() * MY_PI * 2.0f;
	float spread_up = (float)(fsin(r) * crandom() * spread * 16);
	float spread_right = (float)(fcos(r) * crandom() * spread * 16);

	end = player.entity->position + camera_frame.forward * 8192 * 16;
	end.x += spread_right;
	end.y += spread_up;
	end.z += spread_right;

	dir = end - player.entity->position;
	dir.normalize();

	camera_frame.forward = dir;



	add_decal(player.entity->position, camera_frame, *(model_table[MODEL_BULLET_HIT]), 10.0f, true, 10);

	Entity *muzzleflash = engine->entity_list[engine->get_entity()];
	muzzleflash->position = player.entity->position + camera_frame.forward * 75.0f;
	muzzleflash->light = new Light(muzzleflash, engine->gfx, 999, engine->res_scale);
	muzzleflash->light->color = vec3(1.0f, 1.0f, 0.0f);
	muzzleflash->light->intensity = 2000.0f;
	muzzleflash->light->attenuation = 0.0625f;
	muzzleflash->light->timer_flag = true;
	muzzleflash->light->timer = (int)(0.125f * TICK_RATE);
	muzzleflash->visible = true; // accomodate for low spatial testing rate
	muzzleflash->bsp_leaf = player.entity->bsp_leaf;
	muzzleflash->bsp_visible = player.entity->bsp_visible = true;



	Entity *bullet = engine->entity_list[engine->get_entity()];
	bullet->rigid = new RigidBody(bullet);
	bullet->position = camera_frame.pos;
	bullet->rigid->clone(*(model_table[MODEL_BULLET]));
	camera_frame.set(bullet->rigid->morientation);
	vec3 right = vec3::crossproduct(camera_frame.forward, camera_frame.up);
	bullet->position += camera_frame.forward * 3.0f - camera_frame.up * 4.0f + right * 5.0f;
	bullet->rigid->velocity += right * random() + camera_frame.up * random();
	bullet->rigid->angular_velocity = vec3(1.0 * random(), 2.0 * random(), 3.0 * random());
	bullet->rigid->gravity = true;
	bullet->model = bullet->rigid;
	bullet->rigid->impact_index = SND_BULLET;
	bullet->rigid->rotational_friction_flag = true;
	bullet->rigid->translational_friction_flag = true;
	bullet->rigid->translational_friction = 0.9f;
	camera_frame.set(bullet->model->morientation);
	bullet->visible = true; // accomodate for low spatial testing rate
	bullet->bsp_leaf = player.entity->bsp_leaf;
	bullet->bsp_visible = player.entity->bsp_visible = true;


	if (client == false)
	{
		engine->hitscan(player.entity->position, camera_frame.forward, index, num_index, self, -1.0f);

		float quad_factor = 1.0f;

		if (player.quad_timer > 0)
			quad_factor = QUAD_FACTOR;

		for (int i = 0; i < num_index; i++)
		{
			Player *target = engine->entity_list[index[i]]->player;
			if (target == NULL)
				continue;

			if (target->health <= 0)
				continue;

			if (player.team == target->team && gametype != GAMETYPE_DEATHMATCH)
				continue;

			debugf("Player %s hit %s with the machinegun for %d damage\n", player.name,
				target->name, (int)(MACHINEGUN_DAMAGE * quad_factor));
			sprintf(cmd, "hurt %d %d", index[i], (int)(MACHINEGUN_DAMAGE * quad_factor));
			console(self, cmd, engine->menu, engine->entity_list);
			debugf("%s has %d health\n", target->name,
				target->health);
			player.stats.hits++;
	
			if (target->health <= 0 && target->state != PLAYER_DEAD)
			{
				char msg[256];
				char word[16] = { 0 };

				player.stats.kills++;
				target->stats.deaths++;

				if (target->health <= -50)
					sprintf(word, "%s", "gibbed");
				else
					sprintf(word, "%s", "killed");

				sprintf(msg, "%s killed %s with a machinegun\n", player.name,
					target->name);
				debugf(msg);
				engine->menu.print_notif(msg);
				notif_timer = 3 * TICK_RATE;

				handle_frags_left(player);
			}
		}
	}

}


void Quake3::handle_frags_left(Player &player)
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

void Quake3::handle_shotgun(Player &player, int self, bool client)
{
	Frame camera_frame;
	int index[16] = { -1 };
	int num_index = 0;

	player.entity->model->get_frame(camera_frame);

	player.reload_timer = SHOTGUN_RELOAD;
	player.ammo_shells--;

	//	engine->map.hitscan(player.entity->position, forward, distance);

	camera_frame.forward *= -1;

	Entity *muzzleflash = engine->entity_list[engine->get_entity()];
	muzzleflash->position = player.entity->position + camera_frame.forward * 75.0f;
	muzzleflash->light = new Light(muzzleflash, engine->gfx, 999, engine->res_scale);
	muzzleflash->light->color = vec3(1.0f, 1.0f, 0.75f);
	muzzleflash->light->intensity = 3000.0f;
	muzzleflash->light->attenuation = 0.125f;
	muzzleflash->light->timer_flag = true;
	muzzleflash->light->timer = (int)(0.125f * TICK_RATE);
	muzzleflash->visible = true; // accomodate for low spatial testing rate
	muzzleflash->bsp_leaf = player.entity->bsp_leaf;
	muzzleflash->bsp_visible = player.entity->bsp_visible = true;

	vec3 right = vec3::crossproduct(camera_frame.forward, camera_frame.up);

	Entity *shell = engine->entity_list[engine->get_entity()];
	shell->rigid = new RigidBody(shell);
	shell->position = camera_frame.pos;
	shell->position += camera_frame.forward * 3.0f - camera_frame.up * 4.0f + right * 5.0f;
	shell->rigid->clone(*(model_table[MODEL_SHELL]));
	camera_frame.set(shell->rigid->morientation);
	shell->rigid->velocity += right * random() + camera_frame.up * random();
	shell->rigid->angular_velocity = vec3(1.0 * random(), 2.0 * random(), 3.0  * random());
	shell->rigid->gravity = true;
	shell->rigid->rotational_friction_flag = true;
	shell->rigid->translational_friction_flag = true;
	shell->rigid->translational_friction = 0.9f;
	shell->rigid->impact_index = SND_SHELL;


	shell->model = shell->rigid;
	camera_frame.set(shell->model->morientation);
	shell->visible = true; // accomodate for low spatial testing rate
	shell->bsp_leaf = player.entity->bsp_leaf;
	shell->bsp_visible = player.entity->bsp_visible = true;


	Entity *shell2 = engine->entity_list[engine->get_entity()];
	shell2->rigid = new RigidBody(shell2);
	shell2->position = camera_frame.pos;
	shell2->position += camera_frame.forward * 3.0f - camera_frame.up * 4.0f + right * 5.0f;
	shell2->rigid->clone(*(model_table[MODEL_SHELL]));
	camera_frame.set(shell->rigid->morientation);

	shell2->position += camera_frame.forward * 2.0f;
	shell2->rigid->velocity += right * 1.25f * random() + camera_frame.up * random();
	shell2->rigid->rotational_friction_flag = true;
	shell2->rigid->translational_friction_flag = true;
	shell2->rigid->translational_friction = 0.9f;
	shell2->rigid->angular_velocity = vec3(-1.0 * random(), 2.0 * random(), -3.0 * random());
	shell2->rigid->gravity = true;
	shell2->rigid->impact_index = SND_SHELL;
	shell2->model = shell2->rigid;
	camera_frame.set(shell2->model->morientation);
	shell2->visible = true; // accomodate for low spatial testing rate
	shell2->bsp_leaf = player.entity->bsp_leaf;
	shell2->bsp_visible = player.entity->bsp_visible = true;


	for (int i = 0; i < 10; i++)
	{
		// Added to end vector
		int spread = 700;
		float r = random() * MY_PI * 2.0f;
		float spread_up = (float)(fsin(r) * crandom() * spread * 16);
		float spread_right = (float)(fcos(r) * crandom() * spread * 16);
		vec3 end;
		vec3 dir;

		end = player.entity->position + camera_frame.forward * 8192 * 16;
		end.x += spread_right;
		end.y += spread_up;
		end.z += spread_right;

		dir = end - player.entity->position;
		dir.normalize();

		camera_frame.forward = dir;


		add_decal(player.entity->position, camera_frame, *(model_table[MODEL_BULLET_HIT]), 10.0f, true, 10);


		if (client == false)
		{
			float quad_factor = 1.0f;

			if (player.quad_timer > 0)
				quad_factor = QUAD_FACTOR;


			engine->hitscan(player.entity->position, camera_frame.forward, index, num_index, self, -1.0f);
			for (int i = 0; i < num_index; i++)
			{
				Player *target = engine->entity_list[index[i]]->player;
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

					if (target->health <= -50)
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

void Quake3::handle_gibs(Player &player)
{

	Frame camera_frame;

	player.entity->model->get_frame(camera_frame);


	player.entity->rigid->velocity += vec3(0.5f, 3.0f, 1.2f);


	{
		Entity *entity0 = engine->entity_list[engine->get_entity()];
		entity0->rigid = new RigidBody(entity0);
		entity0->model = entity0->rigid;
		entity0->position = camera_frame.pos;
		camera_frame.set(entity0->model->morientation);

		entity0->rigid->clone(*(model_table[MODEL_GIB0]));
		entity0->rigid->velocity = vec3(2.0f, 1.2f, 1.6f);
		entity0->rigid->angular_velocity = vec3(3.0f, 1.0f, 2.2f);
		entity0->rigid->gravity = true;
		entity0->rigid->translational_friction_flag = true;
		entity0->rigid->rotational_friction_flag = true;
		entity0->rigid->impact_index = SND_GIB1;
	}

	{
		Entity *entity1 = engine->entity_list[engine->get_entity()];
		entity1->rigid = new RigidBody(entity1);
		entity1->model = entity1->rigid;
		entity1->position = camera_frame.pos;
		camera_frame.set(entity1->model->morientation);

		entity1->rigid->clone(*(model_table[MODEL_GIB1]));
		entity1->rigid->velocity = vec3(0.8f, 1.2f, -1.2f);
		entity1->rigid->angular_velocity = vec3(1.0f, 1.6f, 2.0f);
		entity1->rigid->gravity = true;
		entity1->rigid->translational_friction_flag = true;
		entity1->rigid->rotational_friction_flag = true;
		entity1->rigid->impact_index = SND_GIB2;
	}

	{
		Entity *entity2 = engine->entity_list[engine->get_entity()];

		entity2->rigid = new RigidBody(entity2);
		entity2->model = entity2->rigid;
		entity2->position = camera_frame.pos;
		camera_frame.set(entity2->model->morientation);

		entity2->rigid->clone(*(model_table[MODEL_GIB2]));
		entity2->rigid->velocity = vec3(0.5f, 2.0f, 0.2f);
		entity2->rigid->angular_velocity = vec3(-2.0f, 1.0f, 6.0f);
		entity2->rigid->gravity = true;
		entity2->rigid->translational_friction_flag = true;
		entity2->rigid->rotational_friction_flag = true;
		entity2->rigid->impact_index = SND_GIB3;
	}

	{
		Entity *entity3 = engine->entity_list[engine->get_entity()];

		entity3->rigid = new RigidBody(entity3);
		entity3->model = entity3->rigid;
		entity3->position = camera_frame.pos;
		camera_frame.set(entity3->model->morientation);

		entity3->rigid->clone(*(model_table[MODEL_GIB3]));
		entity3->rigid->velocity = vec3(-2.0f, 3.2f, 1.2f);
		entity3->rigid->angular_velocity = vec3(3.0f, -4.0f, 2.0f);
		entity3->rigid->gravity = true;
		entity3->rigid->rotational_friction_flag = true;
		entity3->rigid->translational_friction_flag = true;
		entity3->rigid->impact_index = SND_GIB1;
	}

	{
		Entity *entity4 = engine->entity_list[engine->get_entity()];

		entity4->rigid = new RigidBody(entity4);
		entity4->model = entity4->rigid;
		entity4->position = camera_frame.pos;
		camera_frame.set(entity4->model->morientation);

		entity4->rigid->clone(*(model_table[MODEL_GIB4]));
		entity4->rigid->velocity = vec3(-1.25f, 1.7f, 1.27f);
		entity4->rigid->angular_velocity = vec3(3.0f, -4.0f, 2.0f);
		entity4->rigid->gravity = true;
		entity4->rigid->rotational_friction_flag = true;
		entity4->rigid->translational_friction_flag = true;
		entity4->rigid->impact_index = SND_GIB2;
	}

	{
		Entity *entity5 = engine->entity_list[engine->get_entity()];

		entity5->rigid = new RigidBody(entity5);
		entity5->model = entity5->rigid;
		entity5->position = camera_frame.pos;
		camera_frame.set(entity5->model->morientation);

		entity5->rigid->clone(*(model_table[MODEL_GIB5]));
		entity5->rigid->velocity = vec3(-1.45f, 3.7f, 1.72f);
		entity5->rigid->angular_velocity = vec3(3.0f, -4.0f, 2.0f);
		entity5->rigid->gravity = true;
		entity5->rigid->rotational_friction_flag = true;
		entity5->rigid->translational_friction_flag = true;
		entity5->rigid->impact_index = SND_GIB1;
	}

	{
		Entity *entity6 = engine->entity_list[engine->get_entity()];

		entity6->rigid = new RigidBody(entity6);
		entity6->model = entity6->rigid;
		entity6->position = camera_frame.pos;
		camera_frame.set(entity6->model->morientation);

		entity6->rigid->clone(*(model_table[MODEL_GIB6]));
		entity6->rigid->velocity = vec3(-1.15f, 1.7f, 1.37f);
		entity6->rigid->angular_velocity = vec3(3.0f, -4.0f, 2.0f);
		entity6->rigid->gravity = true;
		entity6->rigid->translational_friction_flag = true;
		entity6->rigid->rotational_friction_flag = true;
		entity6->rigid->impact_index = SND_GIB2;
	}

	{
		Entity *entity7 = engine->entity_list[engine->get_entity()];

		entity7->rigid = new RigidBody(entity7);
		entity7->model = entity7->rigid;
		entity7->position = camera_frame.pos;
		camera_frame.set(entity7->model->morientation);

		entity7->rigid->clone(*(model_table[MODEL_GIB7]));
		entity7->rigid->velocity = vec3(-1.45f, 2.34f, 1.27f);
		entity7->rigid->angular_velocity = vec3(3.0f, -4.0f, 2.0f);
		entity7->rigid->gravity = true;
		entity7->rigid->translational_friction_flag = true;
		entity7->rigid->rotational_friction_flag = true;
		entity7->rigid->impact_index = SND_GIB3;
	}

	{
		Entity *entity8 = engine->entity_list[engine->get_entity()];

		entity8->rigid = new RigidBody(entity8);
		entity8->model = entity8->rigid;
		entity8->position = camera_frame.pos;
		camera_frame.set(entity8->model->morientation);

		entity8->rigid->clone(*(model_table[MODEL_GIB8]));
		entity8->rigid->velocity = vec3(-1.85f, 1.73f, 2.32f);
		entity8->rigid->angular_velocity = vec3(3.0f, -4.0f, 2.0f);
		entity8->rigid->gravity = true;
		entity8->rigid->translational_friction_flag = true;
		entity8->rigid->rotational_friction_flag = true;
		entity8->rigid->impact_index = SND_GIB1;
	}

	{
		Entity *entity9 = engine->entity_list[engine->get_entity()];

		entity9->rigid = new RigidBody(entity9);
		entity9->model = entity9->rigid;
		entity9->position = camera_frame.pos;
		camera_frame.set(entity9->model->morientation);

		entity9->rigid->clone(*(model_table[MODEL_GIB9]));
		entity9->rigid->velocity = vec3(1.45f, 1.27f, -1.2f);
		entity9->rigid->angular_velocity = vec3(3.0f, -4.0f, 2.0f);
		entity9->rigid->gravity = true;
		entity9->rigid->translational_friction_flag = true;
		entity9->rigid->rotational_friction_flag = true;
		entity9->rigid->impact_index = SND_GIB2;
	}

}

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



}

void Quake3::handle_weapons(Player &player, input_t &input, int self, bool client)
{
	static bool once = false;
	static bool playing = false;
	bool fired = false;
	bool empty = false;
	int attack_sound = -1;

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
		player.state = PLAYER_DEAD;
		return;
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
				handle_rocketlauncher(player, self, client);
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
				handle_machinegun(player, self, client);
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

void Quake3::draw_flash(Player &player)
{
	engine->gfx.Blend(true);
	engine->gfx.BlendFuncOneOne();
	if (player.flash_gauntlet)
		draw_icon(15.0, ICON_F_GAUNTLET);
	else if (player.flash_machinegun)
		draw_icon(15.0, ICON_F_MACHINEGUN);
	else if (player.flash_shotgun)
		draw_icon(15.0, ICON_F_SHOTGUN);
	else if (player.flash_grenade)
		draw_icon(15.0, ICON_F_GRENADE);
	else if (player.flash_rocket)
		draw_icon(15.0, ICON_F_ROCKET);
	else if (player.flash_lightning)
		draw_icon(15.0, ICON_F_LIGHTNING);
	else if (player.flash_railgun)
		draw_icon(15.0, ICON_F_RAILGUN);
	else if (player.flash_plasma)
		draw_icon(15.0, ICON_F_PLASMA);
	engine->gfx.Blend(false);
}

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
			Player *p = engine->entity_list[i]->player;


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
			snprintf(msg, LINE_SIZE, "Water: %d depth %lf", entity->rigid->water, entity->rigid->water_depth);
			engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color, false, false);
			snprintf(msg, LINE_SIZE, "drawcalls: %d triangles %d", engine->gfx.gpustat.drawcall, engine->gfx.gpustat.triangle);
			engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color, false, false);
			snprintf(msg, LINE_SIZE, "netinfo: ping: %d delta %d size %d num_ents %d dropped %d",
				engine->netinfo.ping,
				engine->netinfo.sequence_delta,
				engine->netinfo.size,
				engine->netinfo.num_ents,
				engine->netinfo.dropped);
			engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color, false, false);
			snprintf(msg, LINE_SIZE, "send_full %d send_partial %d recv_empty %d",
				engine->netinfo.send_full,
				engine->netinfo.send_partial,
				engine->netinfo.recv_empty
			);
			engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color, false, false);

			snprintf(msg, LINE_SIZE, "on_ground %d impact velocity %f", entity->rigid->on_ground, entity->rigid->impact_velocity );
			engine->menu.draw_text(msg, 0.01f, 0.025f * line++, 0.025f, color, false, false);


			if (engine->server_flag)
			{
				for (unsigned int i = 0; i < engine->client_list.size(); i++)
				{
					client_t *client = engine->client_list[i];
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

			if (ent->visible && ent->nodraw == false)
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

		if (ent->visible && ent->nodraw == false)
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

			if ( strlen(engine->entity_list[i]->target_name) <= 1 )
				continue;

			for (unsigned int j = 0; j < engine->entity_list.size(); j++)
			{
//				if (engine->entity_list[j]->nodraw == true)
	//				continue;

				if (strlen(engine->entity_list[j]->target) <= 1)
					continue;

				if (strstr(engine->entity_list[i]->target_name, engine->entity_list[j]->target) != NULL)
				{
					draw_line(engine->entity_list[i], engine->entity_list[j], engine->menu, color);
				}
			}

		}
	}

	engine->menu.draw_text("", 0.01f, 0.025f, 0.025f, color, false, false);
	engine->projection = real_projection;

	engine->gfx.Blend(true);
	engine->gfx.BlendFunc(NULL, NULL);
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
			sprintf(data, "%s", entity->type);
			menu.draw_text(entity->type, pos.x, pos.y - 0.0625f, 0.02f, color, false, false);
			sprintf(data, "bsp_leaf: %d", entity->bsp_leaf);
			menu.draw_text(data, pos.x, pos.y, 0.02f, color, false, false);
		}

		if (strcmp(entity->type, "free") == 0)
		{
			int line = 1;

			sprintf(data, "Pos %.3f %.3f %.3f", entity->position.x, entity->position.y, entity->position.z);
			menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, color, false, false);
			sprintf(data, "Vel %.3f %.3f %.3f", entity->rigid->velocity.x, entity->rigid->velocity.y, entity->rigid->velocity.z);
			menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, color, false, false);
			sprintf(data, "State %d", entity->rigid->sleep);
			menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, color, false, false);
		}

		if (entity->ent_type == ENT_NAVPOINT)
		{
			vec3 blue(0.0f, 0.0f, 1.0f);
			vec3 green(0.0f, 1.0f, 0.0f);
			int line = 1;

			sprintf(data, "targetname %s", entity->target_name);
			menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.025f, blue, false, false);
			sprintf(data, "target %s", entity->target);
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
				sprintf(data, "Bot State %s", bot_state_name[entity->player->bot_state]);
				menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, red, false, false);

				if (entity->player->bot_state == BOT_GET_ITEM)
				{
					sprintf(data, "Item: %s", engine->entity_list[entity->player->get_item]->type);
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
				sprintf(data, "target %s", entity->target);
				menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, color, false, false);
				sprintf(data, "target_name %s", entity->target_name);
				menu.draw_text(data, pos.x, pos.y + 0.0625f * line++, 0.02f, color, false, false);
			}
		}
	}
	menu.draw_text("", pos.x, pos.y + 0.0625f, 0.02f, color, false, true);
	engine->projection = real_projection;
}


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
	engine->particle_render.Params(mvp, quad1, quad2, 0.0f, 0.0f);
	engine->gfx.SelectTexture(0, engine->particle_tex);
	engine->gfx.SelectIndexBuffer(line_ibo);
	engine->particle_render.render(engine->gfx, 0, line_vbo, 400);

	// yeah I know
	engine->gfx.DeleteIndexBuffer(line_ibo);
	engine->gfx.DeleteVertexBuffer(line_vbo);
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
	mvp2 = (matrix * trans2) * projection;

	vec4 pos_4d = mvp2 * vec4(0.0f, 0.0f, 0.0f, 1.0f); // model space coordinate

	pos2d.x = pos_4d.x / pos_4d.w;
	pos2d.y = -pos_4d.y / pos_4d.w;
	pos2d.z = pos_4d.z / pos_4d.w;

	pos2d.x = 0.5f + (pos2d.x * 0.5f);
	pos2d.y = 0.5f + (pos2d.y * 0.5f);
}

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
		crosshair_tex[i] = load_texture_pk3(engine->gfx, filename, engine->pk3_list, engine->num_pk3, true, false);
	}
	current_crosshair = 0;
}




void Quake3::draw_crosshair()
{
	matrix4 transformation;
	engine->camera_frame.set(transformation);

	//matrix4 mvp = transformation * engine->projection;
	float scale = crosshair_scale / 150.0f;

	vec3 quad1 = vec3(0.0f, scale, 0.0f);
	vec3 quad2 = vec3(scale, 0.0f, 0.0f);

#ifndef DIRECTX
	engine->particle_render.Select();
	engine->particle_render.Params(engine->projection, quad1, quad2, 0.0f, 0.0f);
	engine->gfx.SelectTexture(0, crosshair_tex[current_crosshair]);
	engine->particle_render.render(engine->gfx, 0, crosshair_vbo, 1);
#endif
}


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
		icon_list[i].tex = load_texture_pk3(engine->gfx, icon_list[i].filename, engine->pk3_list, engine->num_pk3, true, false);

		if (icon_list[i].tex == 0)
		{
			icon_list[i].tex = load_texture(engine->gfx, icon_list[i].filename, false, false);
		}

		if (icon_list[i].tex == 0)
		{
			printf("Failed to load %s\n", icon_list[i].filename);
		}
	}

	icon_vbo = engine->gfx.CreateVertexBuffer(&vert, icon_list.size());
}

void Quake3::draw_icon(float scale, int index, float x, float y)
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
	engine->particle_render.Params(engine->projection, quad1, quad2, icon_list[index].x + x, icon_list[index].y + y);
	engine->particle_render.render(engine->gfx, index, icon_vbo, 1);
#endif
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

void Quake3::console(int self, char *cmd, Menu &menu, vector<Entity *> &entity_list)
{
	char msg[LINE_SIZE] = { 0 };
	char data[LINE_SIZE] = { 0 };
	char data2[LINE_SIZE] = { 0 };
	int ret;

	if (cmd[0] == '\0')
		return;

	debugf("Console: %s\n", cmd);

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


		unsigned int damage = abs32(atoi(data2));
		unsigned int health_damage = damage / 3;
		unsigned int armor_damage = 2 * health_damage;
		Player *player = entity_list[index]->player;

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

	ret = sscanf(cmd, "cg_crosshairsize %s", data);
	if (ret == 1)
	{
		crosshair_scale = (float)atof(data);
	}

	ret = sscanf(cmd, "damage %s", data);
	if (ret == 1)
	{
		bool local = entity_list[self]->player->local;
		Player *player = entity_list[self]->player;

		snprintf(msg, LINE_SIZE, "damage %s\n", data);
		menu.print(msg);

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
			else if (player->health <= 100)
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
		Player *player = entity_list[self]->player;
		snprintf(msg, LINE_SIZE, "health %s\n", data);
		menu.print(msg);
		player->health += atoi(data);
		if (player->health > 200)
		{
			player->health = 200;
		}

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
		Player *player = entity_list[self]->player;
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
		Player *player = entity_list[self]->player;

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
		Player *player = entity_list[self]->player;

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
		Player *player = entity_list[self]->player;

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

	if (strcmp(cmd, "weapon_lightning") == 0)
	{
		Player *player = entity_list[self]->player;

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
		Player *player = entity_list[self]->player;

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
		Player *player = entity_list[self]->player;
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
		Player *player = entity_list[self]->player;

		if (player->team == TEAM_RED && player->holdable_flag == false)
			player->holdable_flag = true;
		return;
	}

	ret = strcmp(cmd, "redflag");
	if (ret == 0)
	{
		if (entity_list[self]->player->team == TEAM_BLUE && entity_list[self]->player->holdable_flag == false)
			entity_list[self]->player->holdable_flag = true;
		return;
	}

	ret = strcmp(cmd, "holdable_teleporter");
	if (ret == 0)
	{
		if (entity_list[self]->player->holdable_medikit == false)
			entity_list[self]->player->holdable_teleporter = true;
		return;
	}

	ret = strcmp(cmd, "holdable_medikit");
	if (ret == 0)
	{
		if (entity_list[self]->player->holdable_teleporter == false)
			entity_list[self]->player->holdable_medikit = true;
		return;
	}

	ret = strcmp(cmd, "teleport");
	if (ret == 0 && engine->client_flag == false)
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

				switch (ent->angle)
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


	ret = sscanf(cmd, "teleport %s %s", data, data2);
	if (ret == 2 && engine->client_flag == false)
	{
		snprintf(msg, LINE_SIZE, "target %s\n", data);
		menu.print(msg);

		for (unsigned int i = engine->max_dynamic; i < entity_list.size(); i++)
		{
			if ( !(entity_list[i]->ent_type == ENT_MISC_TELEPORTER_DEST ||
				   entity_list[i]->ent_type == ENT_TARGET_POSITION ||
				   entity_list[i]->ent_type == ENT_TARGET_TELEPORTER) )
				continue;

			if (!strcmp(entity_list[i]->target_name, data))
			{
				matrix4 matrix;
				unsigned int index = atoi(data2);

				if (entity_list[self]->player->teleport_timer > 0)
					return;

				entity_list[self]->player->teleport_timer = TICK_RATE >> 1;
				entity_list[self]->position = entity_list[i]->position + vec3(0.0f, 50.0f, 0.0f);
				entity_list[self]->rigid->velocity = vec3(0.0f, 0.0f, 0.0f);


				if (index < entity_list.size())
				{
					engine->play_wave(entity_list[index]->position, SND_TELEOUT);
				}

				if (entity_list[self]->player->local)
					engine->play_wave_global(SND_TELEIN);
				else
					engine->play_wave(entity_list[self]->position, SND_TELEIN);

				switch (entity_list[i]->angle)
				{
				case 0:
				case 45:
				case 360:
				case 315:
					matrix4::mat_left(matrix, entity_list[self]->position);
					break;
				case 90:
					matrix4::mat_forward(matrix, entity_list[self]->position);
					break;
				case 135:
				case 180:
					matrix4::mat_right(matrix, entity_list[self]->position);
					break;
				case 225:
				case 270:
					matrix4::mat_backward(matrix, entity_list[self]->position);
					break;
				}

				if (entity_list[self]->player->local)
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
	if (pret && engine->client_flag == false)
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

			switch (entity_list[i]->angle)
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

			if (self == engine->find_type(ENT_PLAYER, 0))
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

					switch (entity_list[i]->angle)
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
					Model *model = entity_list[self]->model;

					model->morientation.m[0] = matrix.m[0];
					model->morientation.m[1] = matrix.m[1];
					model->morientation.m[2] = matrix.m[2];

					model->morientation.m[3] = matrix.m[4];
					model->morientation.m[4] = matrix.m[5];
					model->morientation.m[5] = matrix.m[6];

					model->morientation.m[6] = matrix.m[8];
					model->morientation.m[7] = matrix.m[9];
					model->morientation.m[8] = matrix.m[10];

					if (self == engine->find_type(ENT_PLAYER, 0))
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

		for (unsigned int i = 0; i < entity_list.size(); i++)
		{
			if (!strcmp(entity_list[i]->target_name, data))
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
			snprintf(entity_list[self]->player->name, 127, "%s", data);
			debugf("Player name: %s\n", data);
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
		engine->chat(entity_list[self]->player->name, cmd);
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


		for (unsigned int i = 0; i < engine->client_list.size(); i++)
		{
			Player *player = entity_list[engine->client_list[i]->ent_id]->player;
			snprintf(msg, LINE_SIZE, "%d: %s %d kills %d deaths %s %d idle\n", i, player->name,
				player->stats.kills,
				player->stats.deaths,
				engine->client_list[i]->socketname,
				current - engine->client_list[i]->last_time);
			menu.print(msg);
		}
		return;
	}

	ret = sscanf(cmd, "kick %s", data);
	if (ret == 1)
	{
		engine->kick(atoi(data));
		return;
	}

	ret = strcmp(cmd, "noclip");
	if (ret == 0 && engine->client_flag == false)
	{
		if (self != -1)
		{
			RigidBody *rigid = entity_list[self]->rigid;

			rigid->noclip = !entity_list[self]->rigid->noclip;
			rigid->velocity.y = 0.0f; // stop initial sinking into floor from gravity
			rigid->translational_friction = 0.9f;
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
			entity_list[self]->player->regen_timer = 60 * TICK_RATE;
		}
		return;
	}

	ret = strcmp(cmd, "haste");
	if (ret == 0)
	{
		if (self != -1)
		{
			entity_list[self]->player->haste_timer = 60 * TICK_RATE;
		}
		return;
	}

	ret = strcmp(cmd, "quaddamage");
	if (ret == 0)
	{
		if (self != -1)
		{
			entity_list[self]->player->quad_timer = 60 * TICK_RATE;
		}
		return;
	}

	ret = strcmp(cmd, "invisibility");
	if (ret == 0)
	{
		if (self != -1)
		{
			entity_list[self]->player->invisibility_timer = 60 * TICK_RATE;
		}
		return;
	}

	ret = strcmp(cmd, "flight");
	if (ret == 0)
	{
		if (self != -1)
		{
			entity_list[self]->player->flight_timer = 60 * 60 * 24 * TICK_RATE;
		}
		return;
	}

	ret = sscanf(cmd, "flight %s", data);
	if (ret == 1)
	{
		if (self != -1)
		{
			entity_list[self]->player->flight_timer = atoi(data) * TICK_RATE;
		}
		return;
	}

	ret = strcmp(cmd, "shownames");
	if (ret == 0)
	{
		engine->show_names = !engine->show_names;
		snprintf(msg, LINE_SIZE, "%s %d\n", cmd, engine->show_names);
		menu.print(msg);
		return;
	}

	ret = strcmp(cmd, "showlines");
	if (ret == 0)
	{
		engine->show_lines = !engine->show_lines;
		snprintf(msg, LINE_SIZE, "%s %d\n", cmd, engine->show_lines);
		menu.print(msg);
		return;
	}

	ret = strcmp(cmd, "showdebug");
	if (ret == 0)
	{
		engine->show_debug = !engine->show_debug;
		snprintf(msg, LINE_SIZE, "%s %d\n", cmd, engine->show_debug);
		menu.print(msg);
		return;
	}

	ret = strcmp(cmd, "showhud");
	if (ret == 0)
	{
		engine->show_hud = !engine->show_hud;
		snprintf(msg, LINE_SIZE, "%s %d\n", cmd, engine->show_hud);
		menu.print(msg);
		return;
	}

	ret = strcmp(cmd, "shadowmaps");
	if (ret == 0)
	{
		engine->shadowmaps = !engine->shadowmaps;
		snprintf(msg, LINE_SIZE, "%s %d\n", cmd, engine->shadowmaps);
		menu.print(msg);
		return;
	}

	ret = strcmp(cmd, "all_lights");
	if (ret == 0)
	{
		engine->all_lights = !engine->all_lights;
		snprintf(msg, LINE_SIZE, "%s %d\n", cmd, engine->all_lights);
		menu.print(msg);
		return;
	}

	ret = sscanf(cmd, "animation %s", data);
	if (ret == 1)
	{
		snprintf(msg, LINE_SIZE, "%s\n", cmd);
		menu.print(msg);
		engine->zcc.select_animation(atoi(data));
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
			warmup = false;

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


void Quake3::setup_func(vector<Entity *> &entity_list, Bsp &q3map)
{
	for (unsigned int i = 0; i < entity_list.size(); i++)
	{
		if (entity_list[i]->model_ref != -1)
			entity_list[i]->position = q3map.model_origin(entity_list[i]->model_ref);


		if (strstr(entity_list[i]->type, "trigger_push"))
		{
			sprintf(entity_list[i]->trigger->action, "push %s", entity_list[i]->target);
		}

		if (strstr(entity_list[i]->type, "trigger_teleport"))
		{
			// Reset action because of ordering issues
			sprintf(entity_list[i]->trigger->action, "teleport %s %d", entity_list[i]->target, i);
		}

		if (strstr(entity_list[i]->type, "func_") || strstr(entity_list[i]->type, "info_player_intermission") ||
			strstr(entity_list[i]->type, "target_position") || strstr(entity_list[i]->type, "info_notnull") ||
			strstr(entity_list[i]->type, "trigger_push"))
		{
			entity_list[i]->rigid->gravity = false;
		}


		if (strstr(entity_list[i]->type, "func_"))
		{
			for (unsigned int j = 0; j < entity_list.size(); j++)
			{
				if (i == j)
					continue;

				if (entity_list[i]->target[0] == '\0')
					continue;

				if (strcmp(entity_list[i]->target, entity_list[j]->target_name) == 0)
				{
					printf("Entity %d type %s pursuing %d type %s\n", i, entity_list[i]->type,
						j, entity_list[j]->type);
					entity_list[i]->rigid->pursue_flag = true;
					entity_list[i]->rigid->target = entity_list[j];
					break;
				}
			}
		}
	}
}


/*
	Used to create entities for network clients
	Network clients are more interested in visual properties,
	they get the position and orientation over the network
	and let server handle damage etc
*/
void Quake3::make_dynamic_ent(nettype_t item, int ent_id)
{
	Entity *ent = engine->entity_list[ent_id];
	engine->clean_entity(ent_id);

	switch (item)
	{
	case NT_NONE:
		break;
	case NT_ROCKET:
		ent->nettype = NT_ROCKET;
		ent->trigger = new Trigger(ent, engine->audio);
		ent->trigger->projectile = true;
		ent->trigger->explode_index = SND_EXPLODE;
		ent->trigger->idle_index = SND_ROCKETFLY;

		ent->trigger->hide = false;
		ent->trigger->radius = 25.0f;
		ent->trigger->idle = true;
		ent->trigger->explode = true;
		ent->trigger->idle_timer = 0;
		ent->trigger->explode_timer = 10;
		ent->trigger->explode_color = vec3(1.0f, 0.0f, 0.0f);
		ent->trigger->explode_intensity = 500.0f;
		ent->trigger->splash_radius = 250.0f;
		ent->trigger->knockback = 250.0f;
		ent->trigger->splash_damage = 0;
		ent->num_particle = 5000;


		ent->light = new Light(ent, engine->gfx, 999, engine->res_scale);
		ent->light->color = vec3(1.0f, 1.0f, 1.0f);
		ent->light->intensity = 1000.0f;

		ent->rigid = new RigidBody(ent);
		ent->model = ent->rigid;
		ent->rigid->clone(*(model_table[MODEL_ROCKET]));
		ent->rigid->gravity = false;
		break;
	case NT_ROCKET_LAUNCHER:
		ent->rigid = new RigidBody(ent);
		ent->model = ent->rigid;
		ent->model->load(engine->gfx, "media/models/weapons2/rocketl/rocketl");
		ent->nettype = NT_ROCKET_LAUNCHER;
		ent->rigid->translational_friction_flag = true;
		ent->rigid->rotational_friction_flag = true;

		ent->trigger = new Trigger(ent, engine->audio);

		ent->trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
		ent->trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
		sprintf(ent->trigger->action, "weapon_rocketlauncer");
		break;
	case NT_GRENADE:
		ent->nettype = NT_GRENADE;

		ent->rigid = new RigidBody(ent);
		ent->model = ent->rigid;
		ent->rigid->clone(*(model_table[MODEL_GRENADE]));
		ent->rigid->gravity = true;
		ent->rigid->rotational_friction_flag = true;
		ent->rigid->translational_friction_flag = true;
		ent->rigid->translational_friction = 0.9f;
		ent->num_particle = 5000;


		ent->trigger = new Trigger(ent, engine->audio);
		ent->trigger->explode_index = SND_EXPLODE;
		ent->trigger->projectile = true;
		ent->trigger->splash_damage = 0;
		ent->trigger->hide = false;
		ent->trigger->radius = 25.0f;
		ent->trigger->idle = true;
		ent->trigger->idle_timer = 120;
		ent->trigger->explode = true;
		ent->trigger->explode_timer = 10;
		ent->trigger->explode_color = vec3(1.0f, 0.0f, 0.0f);
		ent->trigger->explode_intensity = 500.0f;
		ent->trigger->splash_radius = 250.0f;
		ent->trigger->knockback = 250.0f;
		break;
	case NT_GRENADE_LAUNCHER:
		ent->rigid = new RigidBody(ent);
		ent->model = ent->rigid;
		ent->model->load(engine->gfx, "media/models/weapons2/grenadel/grenade");
		ent->nettype = NT_ROCKET_LAUNCHER;
		ent->rigid->translational_friction_flag = true;
		ent->rigid->rotational_friction_flag = true;

		ent->trigger = new Trigger(ent, engine->audio);
		ent->trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
		ent->trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
		sprintf(ent->trigger->action, "weapon_grenadelauncer");
		break;
	case NT_LIGHTNING:
		ent->nettype = NT_LIGHTNING;
		ent->rigid = new RigidBody(ent);
		ent->rigid->clone(*(model_table[MODEL_BOX]));
		ent->rigid->velocity = vec3();
		ent->rigid->angular_velocity = vec3();
		ent->rigid->gravity = false;
		ent->rigid->lightning_trail = true;
		ent->rigid->bounce = 5;
		ent->model = ent->rigid;
		ent->rigid->noclip = true;


		ent->trigger = new Trigger(ent, engine->audio);
		sprintf(ent->trigger->action, " ");
		ent->trigger->projectile = true;
		ent->trigger->splash_damage = 0;
		ent->trigger->hide = false;
		ent->trigger->radius = 25.0f;
		ent->trigger->idle = true;
		ent->trigger->idle_timer = (int)(0.1 * TICK_RATE);
		ent->trigger->explode = true;
		ent->trigger->explode_timer = 20;
		break;
	case NT_LIGHTNINGGUN:
		ent->rigid = new RigidBody(ent);
		ent->model = ent->rigid;
		ent->model->load(engine->gfx, "media/models/weapons2/lightning/lightning");
		ent->nettype = NT_ROCKET_LAUNCHER;
		ent->rigid->translational_friction_flag = true;
		ent->rigid->rotational_friction_flag = true;

		ent->trigger = new Trigger(ent, engine->audio);
		ent->trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
		ent->trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
		sprintf(ent->trigger->action, "weapon_lightninggun");
		break;
	case NT_RAIL:
		ent->nettype = NT_RAIL;
		ent->rigid = new RigidBody(ent);
		ent->rigid->clone(*(model_table[MODEL_BALL]));
		ent->rigid->velocity = vec3();
		ent->rigid->angular_velocity = vec3();
		ent->rigid->gravity = false;
		ent->rigid->bounce = 5;
		ent->model = ent->rigid;
		ent->model->rail_trail = true;
		ent->rigid->noclip = true;

		ent->trigger = new Trigger(ent, engine->audio);
		sprintf(ent->trigger->action, " ");
		ent->trigger->projectile = true;
		ent->trigger->hide = false;
		ent->trigger->radius = 25.0f;
		ent->trigger->idle = true;
		ent->trigger->idle_timer = (int)(5.0 * TICK_RATE);
		ent->trigger->explode = true;
		ent->trigger->explode_timer = 10;
		break;
	case NT_RAILGUN:
		ent->rigid = new RigidBody(ent);
		ent->model = ent->rigid;
		ent->model->load(engine->gfx, "media/models/weapons2/railgun/railgun");
		ent->nettype = NT_ROCKET_LAUNCHER;
		ent->rigid->translational_friction_flag = true;
		ent->rigid->rotational_friction_flag = true;

		ent->trigger = new Trigger(ent, engine->audio);
		ent->trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
		ent->trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
		sprintf(ent->trigger->action, "weapon_railgun");
		break;
	case NT_PLASMA:
		ent->nettype = NT_PLASMA;
		ent->rigid = new RigidBody(ent);
		ent->model = ent->rigid;

		ent->rigid->clone(*(model_table[MODEL_BALL]));
		ent->rigid->gravity = false;
		ent->trigger = new Trigger(ent, engine->audio);
		ent->trigger->explode_index = engine->get_load_wave("sound/weapons/plasma/plasmx1a.wav");
		ent->trigger->idle_index = engine->get_load_wave("sound/weapons/plasma/lasfly.wav");

		ent->trigger->projectile = true;
		ent->trigger->splash_damage = 0;
		ent->trigger->hide = false;
		ent->trigger->radius = 25.0f;
		ent->trigger->idle = true;
		ent->trigger->explode = false;
		ent->trigger->explode_timer = 10;
		ent->trigger->explode_color = vec3(0.0f, 0.0f, 1.0f);
		ent->trigger->explode_intensity = 200.0f;
		ent->trigger->splash_radius = 75.0f;
		ent->trigger->knockback = 10.0f;


		ent->light = new Light(ent, engine->gfx, 999, engine->res_scale);
		ent->light->color = vec3(0.0f, 0.0f, 1.0f);
		ent->light->intensity = 1000.0f;

		break;
	case NT_PLASMAGUN:
		ent->rigid = new RigidBody(ent);
		ent->model = ent->rigid;
		ent->model->load(engine->gfx, "media/models/weapons2/plasmagun/plasmagun");
		ent->nettype = NT_ROCKET_LAUNCHER;
		ent->rigid->translational_friction_flag = true;
		ent->rigid->rotational_friction_flag = true;

		ent->trigger = new Trigger(ent, engine->audio);
		ent->trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
		ent->trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
		sprintf(ent->trigger->action, "weapon_plasma");
		break;
	case NT_SHOTGUN:
		ent->rigid = new RigidBody(ent);
		ent->model = ent->rigid;
		ent->model->load(engine->gfx, "media/models/weapons2/shotgun/shotgun");
		ent->nettype = NT_ROCKET_LAUNCHER;
		ent->rigid->translational_friction_flag = true;
		ent->rigid->rotational_friction_flag = true;

		ent->trigger = new Trigger(ent, engine->audio);
		ent->trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
		ent->trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
		sprintf(ent->trigger->action, "weapon_shotgun");
		break;
	case NT_MACHINEGUN:
		ent->rigid = new RigidBody(ent);
		ent->model = ent->rigid;
		ent->model->load(engine->gfx, "media/models/weapons2/machinegun/machinegun");
		ent->nettype = NT_ROCKET_LAUNCHER;
		ent->rigid->translational_friction_flag = true;
		ent->rigid->rotational_friction_flag = true;

		ent->trigger = new Trigger(ent, engine->audio);
		ent->trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
		ent->trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
		sprintf(ent->trigger->action, "weapon_machinegun");
		break;
	case NT_QUAD:
		ent->rigid = new RigidBody(ent);
		ent->model = ent->rigid;
		ent->model->load(engine->gfx, "media/models/powerups/instant/quad");
		ent->nettype = NT_ROCKET_LAUNCHER;
		ent->rigid->translational_friction_flag = true;
		ent->rigid->rotational_friction_flag = true;

		ent->trigger = new Trigger(ent, engine->audio);
		ent->trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
		ent->trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
		sprintf(ent->trigger->action, "quaddamage");
		break;
	case NT_REGEN:
		ent->rigid = new RigidBody(ent);
		ent->model = ent->rigid;
		ent->model->load(engine->gfx, "media/models/powerups/instant/regen");
		ent->nettype = NT_ROCKET_LAUNCHER;
		ent->rigid->translational_friction_flag = true;
		ent->rigid->rotational_friction_flag = true;

		ent->trigger = new Trigger(ent, engine->audio);
		ent->trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
		ent->trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
		sprintf(ent->trigger->action, "regeneration");
		break;
	case NT_INVIS:
		ent->rigid = new RigidBody(ent);
		ent->model = ent->rigid;
		ent->model->load(engine->gfx, "media/models/powerups/instant/invis");
		ent->nettype = NT_ROCKET_LAUNCHER;
		ent->rigid->translational_friction_flag = true;
		ent->rigid->rotational_friction_flag = true;

		ent->trigger = new Trigger(ent, engine->audio);
		ent->trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
		ent->trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
		sprintf(ent->trigger->action, "invisibility");
		break;
	case NT_FLIGHT:
		ent->rigid = new RigidBody(ent);
		ent->model = ent->rigid;
		ent->model->load(engine->gfx, "media/models/powerups/instant/flight");
		ent->nettype = NT_ROCKET_LAUNCHER;
		ent->rigid->translational_friction_flag = true;
		ent->rigid->rotational_friction_flag = true;

		ent->trigger = new Trigger(ent, engine->audio);
		ent->trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
		ent->trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
		sprintf(ent->trigger->action, "flight");
		break;
	case NT_HASTE:
		ent->rigid = new RigidBody(ent);
		ent->model = ent->rigid;
		ent->model->load(engine->gfx, "media/models/powerups/instant/haste");
		ent->nettype = NT_ROCKET_LAUNCHER;
		ent->rigid->translational_friction_flag = true;
		ent->rigid->rotational_friction_flag = true;

		ent->trigger = new Trigger(ent, engine->audio);
		ent->trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
		ent->trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
		sprintf(ent->trigger->action, "haste");
		break;
	case NT_BLUE_FLAG:
		ent->rigid = new RigidBody(ent);
		ent->model = ent->rigid;
		ent->model->load(engine->gfx, "media/models/flags/b_flag");
		ent->nettype = NT_ROCKET_LAUNCHER;
		ent->rigid->translational_friction_flag = true;
		ent->rigid->rotational_friction_flag = true;

		ent->trigger = new Trigger(ent, engine->audio);
		ent->trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
		ent->trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");
		sprintf(ent->trigger->action, "blue_flag");
		break;
	case NT_RED_FLAG:
		ent->rigid = new RigidBody(ent);
		ent->model = ent->rigid;
		ent->model->load(engine->gfx, "media/models/flags/r_flag");
		ent->nettype = NT_ROCKET_LAUNCHER;
		ent->rigid->translational_friction_flag = true;
		ent->rigid->rotational_friction_flag = true;

		ent->trigger = new Trigger(ent, engine->audio);
		ent->trigger->pickup_index = engine->get_load_wave("sound/misc/w_pkup.wav");
		ent->trigger->respawn_index = engine->get_load_wave("sound/items/s_health.wav");

		sprintf(ent->trigger->action, "red_flag");
		break;
	}
}


void Quake3::endgame(char *winner)
{
	engine->input.scores = true;

	win_timer = 3 * TICK_RATE;
	strcpy(win_msg, winner);

	warmup = true;
	console(-1, "reset 0", engine->menu, engine->entity_list);
	round_time = 0;
}



void Quake3::check_triggers(int self, vector<Entity *> &entity_list)
{
	engine->num_light = 0;
	for (unsigned int i = 0; i < entity_list.size(); i++)
	{
		bool inside = false;
		RigidBody *rigid = entity_list[i]->rigid;

		if (entity_list[i]->light)
			engine->num_light++;

		if (rigid && rigid->hard_impact && i >= engine->max_player)
		{
			if (rigid->impact_velocity <= -RIGID_IMPACT)
			{
				rigid->hard_impact = false;

				engine->play_wave(entity_list[i]->position, rigid->impact_index);

			}
		}


		if (entity_list[i]->ent_type > ENT_FUNC_START && entity_list[i]->ent_type < ENT_FUNC_END)
		{
			float period = 2200.0f; // manually setting for q3tourney6 plat
			float sin_wave = (float)fsin(MY_PI * engine->tick_num / period);
			float square_wave = (float)sign((float)fsin(2 * MY_PI * engine->tick_num / period));
			float amount = sin_wave * square_wave;

			if (entity_list[i]->ent_type == ENT_FUNC_STATIC)
				continue;

			if (entity_list[i]->ent_type == ENT_FUNC_DOOR)
			{
				amount = 25.0f * amount;
			}
			static int count;
			{
				count++;
				amount *= entity_list[i]->height / 800.0f;

				switch (entity_list[i]->angle)
				{
				case 0:
				case 360:
					entity_list[i]->position += vec3(amount, 0.0f, 0.0f);
					break;
				case 90:
					entity_list[i]->position += vec3(0.0f, 0.0f, -amount);
					break;
				case 180:
					entity_list[i]->position += vec3(-amount, 0.0f, 0.0f);
					break;
				case 270:
					entity_list[i]->position += vec3(0.0f, 0.0f, amount);
					break;
				case -1://up
					entity_list[i]->position += vec3(0.0f, amount, 0.0f);
					break;
				case -2://down
					entity_list[i]->position += vec3(0.0f, -amount, 0.0f);
					break;
				}
			}
		}


		// Not a trigger
		Trigger *trigger = entity_list[i]->trigger;

		if (trigger == NULL)
			continue;

		// Delete when not moving
		if (trigger->idle == true)
		{
			if (entity_list[i]->rigid)
			{
				if (entity_list[i]->rigid->bounce > trigger->num_bounce)
				{
					entity_list[i]->particle_on = false;
					if (trigger->explode == false)
					{
						if (trigger->explode_timer <= 0)
						{
							engine->clean_entity(i);
							entity_list[i]->~Entity();
							continue;
						}
						else
						{
							trigger->explode_timer--;
						}
					}
					else
					{
						// Explode after being idle for idle_timer time (usually zero)
						if (trigger->idle_timer <= 0)
						{
							trigger->radius = trigger->splash_radius;
							sprintf(trigger->action, "damage %d", trigger->splash_damage);
							if (entity_list[i]->light == NULL)
							{
								entity_list[i]->light = new Light(entity_list[i], engine->gfx, 999, engine->res_scale);
							}
							entity_list[i]->light->intensity = trigger->explode_intensity;
							entity_list[i]->light->color = trigger->explode_color;
							trigger->explode = false;

							engine->play_wave(entity_list[i]->position, trigger->explode_index);
							continue;
						}
						else
						{
							trigger->idle_timer--;
						}
					}
				}
			}
		}


		// Only other players can pick up
		if (trigger->owner == self && entity_list[i]->rigid->bounce == 0)
			continue;

		Player *player = entity_list[self]->player;

		if (trigger->owner >= 0 &&
			gametype != GAMETYPE_DEATHMATCH &&
			entity_list[trigger->owner]->player->team == player->team)
			continue;

		float distance = (entity_list[i]->position - entity_list[self]->position).magnitude();

		if (distance < trigger->radius)
			inside = true;

		if (inside && entity_list[i]->ent_type == ENT_TEAM_CTF_BLUEFLAG)
		{
			if (player->team == TEAM_BLUE)
			{
				if (player->holdable_flag)
				{
					player->holdable_flag = false;
					blue_flag_caps++;

					engine->play_wave(entity_list[i]->position, SND_FLAGCAP);

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
				continue;
			}
		}

		if (inside && entity_list[i]->ent_type == ENT_TEAM_CTF_REDFLAG)
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
						engine->play_wave(entity_list[i]->position, SND_FLAGTAKE);

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
				continue;
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

			if (player->teleport_timer > 0 && entity_list[i]->ent_type == ENT_TRIGGER_TELEPORT)
				pickup = false;



			if (pickup)
			{
				trigger->active = true;
				if (trigger->action[0] != '\0')
					console(self, trigger->action, engine->menu, entity_list);

				if (trigger->projectile)
				{
					if (player->health <= 0)
					{
						char word[32] = { 0 };
						char weapon[32];
						int owner = trigger->owner;

						player->stats.deaths++;
						if (owner != -1)
						{
							Player *powner = entity_list[owner]->player;
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


							if (player->health <= -50)
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

							debugf(msg);
							engine->menu.print_notif(msg);
							notif_timer = 3 * TICK_RATE;
							handle_frags_left(*(entity_list[owner]->player));
						}

					}
				}

				entity_list[i]->visible = false;
				trigger->timeout = entity_list[i]->trigger->timeout_value;

				if (trigger->explode_timer)
				{
					vec3 distance = entity_list[self]->position - entity_list[i]->position;
					float mag = MIN(distance.magnitude(), 50.0f);
					//add knockback to explosions
					entity_list[self]->rigid->velocity += (distance.normalize() * trigger->knockback) / mag;
				}

				if (player->local)
					engine->play_wave_global(trigger->pickup_index);
				else
					engine->play_wave(entity_list[i]->position, trigger->pickup_index);

			}
		}


		if (trigger->timeout > 0)
		{
			trigger->timeout -= 0.016f;
		}
		else
		{
			if (trigger->active)
			{
				engine->play_wave(entity_list[i]->position, trigger->respawn_index);
			}

			if (trigger->noise == false)
			{
				trigger->active = false;
				trigger->timeout = 0.0f;
			}
			else
			{
				trigger->timeout = trigger->timeout_value;
			}
		}
	}
}

void Quake3::get_state(serverdata_t *data)
{
	memset(data, 0, sizeof(serverdata_t));
	memcpy(data->header, "data", 4);
	if (warmup)
		data->warmup = 0xFF;

	data->warmup_time = warmup_time;
	data->red_flag_caps = red_flag_caps;
	data->blue_flag_caps = blue_flag_caps;
	data->fraglimit = fraglimit;
	data->timelimit = timelimit;
	data->round_time = round_time;
}

void Quake3::set_state(serverdata_t *data)
{
	if (memcmp(data->header, "data", 4) == 0)
	{
		if (data->warmup)
			warmup = true;
		else
			warmup = false;
		warmup_time = data->warmup_time;
		red_flag_caps = data->red_flag_caps;
		blue_flag_caps = data->blue_flag_caps;
		fraglimit = data->fraglimit;
		timelimit = data->timelimit;
		round_time = data->round_time;
	}
}

void Quake3::add_decal(vec3 &start, Frame &camera_frame, Model &decal_model, float offset, bool idle, int idle_timer)
{
	plane_t plane;
	float depth;
	float water_depth;
	bool water;
	vec3 clip;
	vec3 velocity;
	bool ret = false;
	bool hit = false;
	vec3 pos;
	vec3 step = start;
	bool lava;
	bool slime;


	vec3 end = step + camera_frame.forward;
	
//	pos = engine->q3map.trace(start, end, normal);
//	engine->q3map.collision = false;

	do
	{
		ret = engine->q3map.collision_detect(end, step, &plane, &depth, water, water_depth, engine->surface_list, false, clip, velocity, lava, slime);
		if (ret)
		{
			hit = true;
			pos = end + camera_frame.forward.normalize() * -depth;
		}
		else
		{
			step = end;
			end += camera_frame.forward;
		}
	} while (hit == false);





	if (hit)
	{
//		printf("decal added at x %4.3f y %4.3f z %4.3f\n", pos.x, pos.y, pos.z);
		Entity *decal = engine->entity_list[engine->get_entity()];
		decal->rigid = new RigidBody(decal);
		decal->position = pos + plane.normal * offset;
		decal->rigid->clone(decal_model);
		decal->rigid->gravity = false;
		decal->rigid->bounce = 2;
		decal->model = decal->rigid;
		decal->rigid->noclip = true;
		decal->visible = true; // accomodate for low spatial testing rate
		decal->bsp_leaf = true;
		decal->rigid->blend = true;

		decal->trigger = new Trigger(decal, engine->audio);
		decal->trigger->idle = idle;
		decal->trigger->explode_timer = idle_timer;
		decal->trigger->idle_timer = idle_timer;
		decal->trigger->projectile = true;
		decal->trigger->hide = false;




		// make point axial to plane (change forward vector and normalze a bunch)
		vec3 up = camera_frame.up;
		vec3 forward = plane.normal;

		vec3 right = vec3::crossproduct(forward, up);
		right.normalize();
		up = vec3::crossproduct(right, forward);
		up.normalize();
		right = vec3::crossproduct(forward, up);
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

void Quake3::load_model(Entity &ent)
{
	Graphics &gfx = engine->gfx;

	if (ent.ent_type == ENT_ITEM_ARMOR_SHARD)
	{
		debugf("Loading item_armor_shard\n");
		ent.model->load(gfx, "media/models/powerups/armor/shard");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (ent.ent_type == ENT_WEAPON_ROCKETLAUNCHER)
	{
		debugf("Loading weapon_rocketlauncher\n");
		ent.model->load(gfx, "media/models/weapons2/rocketl/rocketl");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (ent.ent_type == ENT_AMMO_SHELLS)
	{
		debugf("Loading ammo_shells\n");
		ent.model->load(gfx, "media/models/powerups/ammo/ammo_shells");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (ent.ent_type == ENT_AMMO_ROCKETS)
	{
		debugf("Loading ammo_rockets\n");
		ent.model->load(gfx, "media/models/powerups/ammo/ammo_rockets");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (ent.ent_type == ENT_AMMO_CELLS)
	{
		debugf("Loading ammo_cells\n");
		ent.model->load(gfx, "media/models/powerups/ammo/ammo_lightning");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (ent.ent_type == ENT_AMMO_BFG)
	{
		debugf("Loading ammo_bfg\n");
		ent.model->load(gfx, "media/models/powerups/ammo/ammo_lightning");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (ent.ent_type == ENT_AMMO_GRENADES)
	{
		debugf("Loading ammo_bfg\n");
		ent.model->load(gfx, "media/models/powerups/ammo/ammo_grenades");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (ent.ent_type == ENT_AMMO_LIGHTNING)
	{
		debugf("Loading ammo_lightning\n");
		ent.model->load(gfx, "media/models/powerups/ammo/ammo_lightning");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (ent.ent_type == ENT_AMMO_SLUGS)
	{
		debugf("Loading ammo_slugs\n");
		ent.model->load(gfx, "media/models/powerups/ammo/ammo_slugs");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (ent.ent_type == ENT_AMMO_BULLETS)
	{
		debugf("Loading ammo_bullets\n");
		ent.model->load(gfx, "media/models/powerups/ammo/ammo_bullets");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (ent.ent_type == ENT_ITEM_ARMOR_COMBAT)
	{
		debugf("Loading item_armor_combat\n");
		ent.model->load(gfx, "media/models/powerups/armor/item_armor_combat");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.position.y += 15.0f;
		ent.rigid->gravity = false;
	}
	else if (ent.ent_type == ENT_ITEM_ARMOR_BODY)
	{
		debugf("Loading item_armor_body\n");
		ent.model->load(gfx, "media/models/powerups/armor/item_armor_body");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.position.y += 15.0f;
		ent.rigid->gravity = false;
	}
	else if (ent.ent_type == ENT_TEAM_CTF_BLUEFLAG)
	{
		debugf("Loading team_CTF_blueflag\n");
		ent.model->load(gfx, "media/models/flags/b_flag");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.position += vec3(0.0f, 50.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (ent.ent_type == ENT_TEAM_CTF_REDFLAG)
	{
		debugf("Loading team_CTF_redflag\n");
		ent.model->load(gfx, "media/models/flags/r_flag");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.position += vec3(0.0f, 50.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (ent.ent_type == ENT_ITEM_QUAD)
	{
		debugf("Loading item_quad\n");
		ent.model->load(gfx, "media/models/powerups/instant/quad");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
		ent.position.y += 15.0f;
	}
	else if (ent.ent_type == ENT_ITEM_REGEN)
	{
		debugf("Loading item_regen\n");
		ent.model->load(gfx, "media/models/powerups/instant/regen");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
		ent.position.y += 15.0f;
	}
	else if (ent.ent_type == ENT_ITEM_INVIS)
	{
		debugf("Loading item_invis\n");
		ent.model->load(gfx, "media/models/powerups/instant/invis");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
		ent.position.y += 15.0f;
	}
	else if (ent.ent_type == ENT_ITEM_FLIGHT)
	{
		debugf("Loading item_flight\n");
		ent.model->load(gfx, "media/models/powerups/instant/flight");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
		ent.position.y += 15.0f;
	}
	else if (ent.ent_type == ENT_ITEM_HASTE)
	{
		debugf("Loading item_haste\n");
		ent.model->load(gfx, "media/models/powerups/instant/haste");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
		ent.position.y += 15.0f;
	}
	else if (ent.ent_type == ENT_HOLDABLE_MEDKIT)
	{
		debugf("Loading holdable_medkit\n");
		ent.model->load(gfx, "media/models/powerups/holdable/medkit");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
		ent.position.y += 15.0f;
	}
	else if (ent.ent_type == ENT_HOLDABLE_TELEPORTER)
	{
		debugf("Loading holdable_teleporter\n");
		ent.model->load(gfx, "media/models/powerups/holdable/teleporter");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
		ent.position.y += 15.0f;
	}
	else if (ent.ent_type == ENT_WEAPON_LIGHTNING)
	{
		debugf("Loading weapon_lightning\n");
		ent.model->load(gfx, "media/models/weapons2/lightning/lightning");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (ent.ent_type == ENT_WEAPON_BFG)
	{
		debugf("Loading weapon_bfg\n");
		ent.model->load(gfx, "media/models/weapons2/lightning/lightning");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (ent.ent_type == ENT_WEAPON_SHOTGUN)
	{
		debugf("Loading weapon_shotgun\n");
		ent.model->load(gfx, "media/models/weapons2/shotgun/shotgun");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (ent.ent_type == ENT_WEAPON_RAILGUN)
	{
		debugf("Loading weapon_railgun\n");
		ent.model->load(gfx, "media/models/weapons2/railgun/railgun");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (ent.ent_type == ENT_WEAPON_GRENADELAUNCHER)
	{
		debugf("Loading weapon_grenadelauncher\n");
		ent.model->load(gfx, "media/models/weapons2/grenadel/grenade");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (ent.ent_type == ENT_WEAPON_PLASMA)
	{
		debugf("Loading weapon_plasmagun\n");
		ent.model->load(gfx, "media/models/weapons2/plasma/plasma");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (ent.ent_type == ENT_ITEM_HEALTH)
	{
		debugf("Loading item_health\n");
		ent.model->load(gfx, "media/models/powerups/health/item_health");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (ent.ent_type == ENT_ITEM_HEALTH_LARGE)
	{
		debugf("Loading item_health_large\n");
		ent.model->load(gfx, "media/models/powerups/health/item_health_large");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (ent.ent_type == ENT_ITEM_HEALTH_SMALL)
	{
		debugf("Loading item_health_large\n");
		ent.model->load(gfx, "media/models/powerups/health/item_health_small");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (ent.ent_type == ENT_ITEM_HEALTH_MEGA)
	{
		debugf("Loading item_health_mega\n");
		ent.model->load(gfx, "media/models/powerups/health/item_health_mega");
		ent.rigid->angular_velocity = vec3(0.0f, 2.0f, 0.0);
		ent.rigid->gravity = false;
	}
	else if (ent.ent_type == ENT_INFO_PLAYER_DEATHMATCH)
	{
		debugf("Loading info_player_deathmatch\n");
		ent.rigid->load(gfx, "media/models/ball");
		ent.rigid->gravity = false;
		ent.nodraw = true;
	}
	else if (ent.ent_type == ENT_TEAM_CTF_BLUESPAWN)
	{
		debugf("Loading team_CTF_bluespawn\n");
		ent.rigid->load(gfx, "media/models/ball");
		ent.rigid->gravity = false;
		ent.nodraw = true;
	}
	else if (ent.ent_type == ENT_TEAM_CTF_REDSPAWN)
	{
		debugf("Loading team_CTF_redspawn\n");
		ent.rigid->load(gfx, "media/models/ball");
		ent.rigid->gravity = false;
		ent.nodraw = true;
	}
	else if (ent.ent_type == ENT_TEAM_CTF_BLUEPLAYER)
	{
		debugf("Loading team_CTF_blueplayer\n");
		ent.rigid->load(gfx, "media/models/ball");
		ent.rigid->gravity = false;
		ent.nodraw = true;
	}
	else if (ent.ent_type == ENT_TEAM_CTF_REDPLAYER)
	{
		debugf("Loading team_CTF_redplayer\n");
		ent.rigid->load(gfx, "media/models/ball");
		ent.rigid->gravity = false;
		ent.nodraw = true;
	}
}


Quake3::~Quake3()
{
	for(unsigned int i = 0; i < model_table.size(); i++)
	{
		delete model_table[i];
	}
}
