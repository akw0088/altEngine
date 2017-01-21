#include "include.h"

#ifndef QUAKE3_H
#define QUAKE3_H

class Quake3
{
public:
	Quake3();

	void init(Engine *engine);
	void step(int frame_step);
	void destroy();

	void render_hud(double last_frametime);
	void draw_name(Entity *entity, Menu &menu, matrix4 &real_projection);
	void draw_line(Entity *a, Entity *b, Menu &menu, vec3 &color);
	void transform_3d_2d(vec3 &position, vec3 &pos2d, matrix4 &projection);

	void handle_weapons(Player &player, input_t &input, int self);

	void handle_plasma(Player &player, int self);
	void handle_rocketlauncher(Player &player, int self);
	void handle_grenade(Player &player, int self);
	void handle_lightning(Player &player, int self);
	void handle_railgun(Player &player, int self);
	void handle_machinegun(Player &player, int self);
	void handle_shotgun(Player &player, int self);
	void handle_gibs(Player &player);
	void create_crosshair();
	void draw_crosshair();
	void draw_flash(Player &player);


	void create_icon();
	void draw_icon(float scale, int index, float x = 0.0f, float y = 0.0f);
	void load_icon();


	void handle_player(int index);
	void player_died(int index);
	void drop_weapon(int index);
	void drop_quaddamage(vec3 &position);
	void add_bot(int &index);

	int bot_get_path(int item, int self, int *nav_array, path_t &path);
	int bot_follow(path_t &path, int *nav_array, Entity *entity, float speed_scale);

	void load();
	void setup_func(vector<Entity *> &entity_list, Bsp &q3map);
	void unload();
	void init_camera(vector<Entity *> &entity_list);
	void console(int self, char *cmd, Menu &menu, vector<Entity *> &entity_list);
	void endgame();

	gametype_t gametype;
	int		fraglimit;
	int		timelimit;
	int		capturelimit;
	unsigned int chat_timer;
	unsigned int notif_timer;

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

