#include "include.h"

#ifndef BASEGAME_H
#define BASEGAME_H

/*
	Virtual class to keep game specific items out of Engine
*/
class BaseGame
{
public:
	BaseGame();
	virtual void init(Engine *altEngine) = 0;
	virtual void step(int frame_step) = 0;
	virtual void load(gametype_t gametype) = 0;
	virtual void map_model(Entity *ent) = 0;
	virtual void unload() = 0;
	virtual void destroy() = 0;
	virtual void add_player(vector<Entity *> &entity_list, playertype_t type, int &ent_id, char *player_name) = 0;
	virtual void console(int self, char *cmd, Menu &menu, vector<Entity *> &entity_list) = 0;
	virtual void setup_func(vector<Entity *> &entity_list, Bsp *q3map) = 0;
	virtual void render_hud(double last_frametime) = 0;
	virtual void draw_flash(Player *player) = 0;
	virtual void draw_icon(float scale, int index, float x, float y, float z) = 0;
	virtual void make_dynamic_ent(net_ent_t item, int ent_id) = 0;
	virtual void endgame(char *winner) = 0;
	virtual void get_state(serverdata_t *data) = 0;
	virtual void set_state(serverdata_t *data) = 0;
	virtual ~BaseGame()
	{
	};

public:

	virtual gametype_t get_gametype() = 0;
	virtual int get_fraglimit() = 0;
	virtual int get_timelimit() = 0;
	virtual int get_capturelimit() = 0;
	virtual int get_blue_flag_caps() = 0;
	virtual int get_red_flag_caps() = 0;
	virtual int get_notif_timer() = 0;
	virtual bool get_spectator() = 0;
	virtual Model *get_model_table() = 0;
	virtual int get_num_model() = 0;
	virtual vector<icon_t> get_icon_list() = 0;


	virtual void set_gametype(gametype_t &gametype) = 0;
	virtual void set_fraglimit(unsigned int limit) = 0;
	virtual void set_timelimit(unsigned int limit) = 0;
	virtual void set_capturelimit(unsigned int limit) = 0;
	virtual void set_chat_timer(unsigned int limit) = 0;
};

#endif

