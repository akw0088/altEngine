#include <iostream>

#include "base.h"

using namespace std;

class Quake3 : public BaseGame
{
public:
	Quake3()
	{
	}

	void init(Engine *altEngine);
	void step(int frame_step);
	void load(gametype_t gametype);
	void map_model(Entity *ent);
	void unload();
	void destroy();
	void add_player(vector<Entity *> &entity_list, playertype_t type, int &ent_id, char *player_name);
	void console(int self, char *cmd, Menu &menu, vector<Entity *> &entity_list);
	void setup_func(vector<Entity *> &entity_list, Bsp &q3map);
	void render_hud(double last_frametime);
	void draw_flash(Player &player);
	void draw_icon(float scale, int index, float x, float y, float z);
	void make_dynamic_ent(net_ent_t item, int ent_id);
	void endgame(char *winner);
	void get_state(serverdata_t *data);
	void set_state(serverdata_t *data);

	gametype_t get_gametype();
	int get_fraglimit();
	int get_timelimit();
	int get_capturelimit();
	int get_blue_flag_caps();
	int get_red_flag_caps();
	int get_notif_timer();
	bool get_spectator();
	Model *get_model_table();
	int get_num_model();
	vector<icon_t> get_icon_list();


	void set_gametype(gametype_t &gametype);
	void set_fraglimit(unsigned int limit);
	void set_timelimit(unsigned int limit);
	void set_capturelimit(unsigned int limit);
	void set_chat_timer(unsigned int limit);

private:
	Engine *engine;
	bool blink;
	vector<navpoint_t> navmesh;
	int last_spawn;

	int crosshair_tex[10];
	int crosshair_vbo;
	int current_crosshair;
	float crosshair_scale;

	unsigned int icon_vbo;
	bool warmup;
	bool hold_fire;
	bool faceicon;


	//	unsigned int		timelimit;
	unsigned int		round_time;
	unsigned int		warmup_time;
	float old_contrast;

	unsigned int snd_table[512];


	unsigned int win_timer;
	char win_msg[128];

	unsigned int num_player;
	unsigned int num_player_blue;
	unsigned int num_player_red;
	unsigned int spectator_timer;


	bool played_one_frag;
	bool played_two_frag;
	bool played_three_frag;
	bool played_prepare;
	bool played_sudden;


	gametype_t gametype;
	int fraglimit;
	int timelimit;
	int capturelimit;
	int blue_flag_caps;
	int red_flag_caps;
	int notif_timer;
	int chat_timer;
	bool spectator;
	//vector<Model *> model_table;
	Model *model_table;
	int num_model;
	vector <icon_t> icon_list;


private:
	void Quake3::create_crosshair();
	void Quake3::create_icon();
	void Quake3::load_icon();

};



void Quake3::init(Engine *altEngine)
{
	engine = altEngine;
	create_crosshair();
	crosshair_scale = 1.0f;


	load_icon();
	create_icon();
	//(load_sounds(engine->audio, engine->snd_wave);

	//*load_models(engine->gfx);

	//	load_q1_models(engine->gfx);
}

void Quake3::step(int frame_step)
{
}

void Quake3::load(gametype_t gametype)
{
}

void Quake3::map_model(Entity *ent)
{
}

void Quake3::unload()
{
}

void Quake3::destroy()
{
}

void Quake3::add_player(vector<Entity *> &entity_list, playertype_t type, int &ent_id, char *player_name)
{
}

void Quake3::console(int self, char *cmd, Menu &menu, vector<Entity *> &entity_list)
{
}

void Quake3::setup_func(vector<Entity *> &entity_list, Bsp &q3map)
{
}

void Quake3::render_hud(double last_frametime)
{
}

void Quake3::draw_flash(Player &player)
{
}

void Quake3::draw_icon(float scale, int index, float x, float y, float z)
{
}

void Quake3::make_dynamic_ent(net_ent_t item, int ent_id)
{
}

void Quake3::endgame(char *winner)
{
}

void Quake3::get_state(serverdata_t *data)
{
}

void Quake3::set_state(serverdata_t *data)
{
}



gametype_t Quake3::get_gametype()
{
	return gametype;
}

int Quake3::get_fraglimit()
{
	return fraglimit;
}

int Quake3::get_timelimit()
{
	return timelimit;
}

int Quake3::get_capturelimit()
{
	return capturelimit;
}

int Quake3::get_blue_flag_caps()
{
	return blue_flag_caps;
}

int Quake3::get_red_flag_caps()
{
	return red_flag_caps;
}

int Quake3::get_notif_timer()
{
	return notif_timer;
}

bool Quake3::get_spectator()
{
	return spectator;
}

Model *Quake3::get_model_table()
{
	return model_table;
}

int Quake3::get_num_model()
{
	return num_model;
}

vector<icon_t> Quake3::get_icon_list()
{
	return icon_list;
}

void Quake3::set_gametype(gametype_t &gt)
{
	gametype = gt;
}

void Quake3::set_fraglimit(unsigned int limit)
{
	chat_timer = limit;
}

void Quake3::set_timelimit(unsigned int limit)
{
	chat_timer = limit;
}

void Quake3::set_capturelimit(unsigned int limit)
{
	chat_timer = limit;
}

void Quake3::set_chat_timer(unsigned int limit)
{
	chat_timer = limit;
}

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


//private

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
		//*icon_list[i].tex = load_texture_pk3(engine->gfx, icon_list[i].filename, engine->pk3_list, engine->num_pk3, true, false, 0);

		if (icon_list[i].tex == 0)
		{
			//*icon_list[i].tex = load_texture(engine->gfx, icon_list[i].filename, false, false, 0);
		}

		if (icon_list[i].tex == 0)
		{
			printf("Failed to load %s\n", icon_list[i].filename);
		}
	}

	//*icon_vbo = engine->gfx.CreateVertexBuffer(&vert, icon_list.size());
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

	//*crosshair_vbo = engine->gfx.CreateVertexBuffer(&vert, 1);

	for (int i = 0; i < num_crosshair; i++)
	{
		char filename[128];

		sprintf(filename, "media/gfx/2d/crosshair%c.tga", 'a' + i);
		//*crosshair_tex[i] = load_texture_pk3(engine->gfx, filename, engine->pk3_list, engine->num_pk3, true, false, 0);
	}
	current_crosshair = 0;
}
