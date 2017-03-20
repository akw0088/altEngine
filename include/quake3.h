#include "include.h"

#ifndef QUAKE3_H
#define QUAKE3_H

class Quake3 : public BaseGame
{
public:
	Quake3();

	void init(Engine *engine);
	void step(int frame_step);
	void destroy();


	void check_triggers(int self, vector<Entity *> &entity_list);
	void render_hud(double last_frametime);
	void draw_name(Entity *entity, Menu &menu, matrix4 &real_projection);
	void draw_line(Entity *a, Entity *b, Menu &menu, vec3 &color);
	void transform_3d_2d(vec3 &position, vec3 &pos2d, matrix4 &projection);

	void handle_weapons(Player &player, input_t &input, int self, bool client);

	void handle_plasma(Player &player, int self, bool client);
	void handle_rocketlauncher(Player &player, int self, bool client);
	void handle_grenade(Player &player, int self, bool client);
	void handle_lightning(Player &player, int self, bool client);
	void handle_railgun(Player &player, int self, bool client);
	void handle_machinegun(Player &player, int self, bool client);
	void handle_shotgun(Player &player, int self, bool client);
	void handle_gibs(Player &player);
	void create_crosshair();
	void draw_crosshair();
	void draw_flash(Player &player);


	void create_icon();
	void draw_icon(float scale, int index, float x = 0.0f, float y = 0.0f);
	void load_icon();


	void make_dynamic_ent(nettype_t item, int ent_id);
	void handle_player(int index, input_t &input);
	void player_died(int index);
	void drop_weapon(int index);
	void drop_quaddamage(vec3 &position);
	void add_bot(int &index);

	int bot_get_path(int item, int self, int *nav_array, path_t &path);
	int bot_follow(path_t &path, int *nav_array, Entity *entity, float speed_scale);

	void load();
	void setup_func(vector<Entity *> &entity_list, Bsp &q3map);
	void unload();
	void add_player(vector<Entity *> &entity_list, char *type, int &ent_id);
	void console(int self, char *cmd, Menu &menu, vector<Entity *> &entity_list);
	void endgame();

	int		timelimit;
	unsigned int weapon_switch_timer;

private:
	Engine *engine;
	bool blink;
	vector<navpoint_t> navmesh;
	int last_spawn;

	int crosshair_tex[10];
	int crosshair_vbo;
	int current_crosshair;
	float crosshair_scale;

	vector <icon_t> icon_list;
	unsigned int icon_vbo;
	bool spectator;

	unsigned int spectator_timer;
};

#endif

