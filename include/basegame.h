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
	virtual void unload() = 0;
	virtual void destroy() = 0;
	virtual void add_player(vector<Entity *> &entity_list, playertype_t type, int &ent_id, char *player_name) = 0;
	virtual void console(int self, char *cmd, Menu &menu, vector<Entity *> &entity_list) = 0;
	virtual void setup_func(vector<Entity *> &entity_list, Bsp &q3map) = 0;
	virtual void render_hud(double last_frametime) = 0;
	virtual void draw_flash(Player &player) = 0;
	virtual void make_dynamic_ent(nettype_t item, int ent_id) = 0;
	virtual void endgame(char *winner) = 0;
	virtual ~BaseGame()
	{
	};

public:

	// Need to move these references out of Engine
	gametype_t gametype;
	int fraglimit;
	int capturelimit;
	int blue_flag_caps;
	int red_flag_caps;
	int notif_timer;
	int chat_timer;
	Engine *engine;
};

#endif

