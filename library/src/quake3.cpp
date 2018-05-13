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

};

void Quake3::init(Engine *altEngine)
{
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
