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

	void handle_gauntlet(Player &player, int self, bool client);
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
	void add_decal(vec3 &start, Frame &camera_frame, Model &decal_model, float offset, bool explode, int explode_timer);

	void create_icon();
	void draw_icon(float scale, int index, float x = 0.0f, float y = 0.0f);
	void load_icon();
	void load_model(Entity &ent);
	void load_sounds(Audio &audio, vector<wave_t> &snd_wave);


	void make_dynamic_ent(nettype_t item, int ent_id);
	void handle_player(int index, input_t &input);
	void handle_frags_left(Player &player);
	void player_died(int index);
	void drop_weapon(int index);
	void drop_powerup(vec3 &position, char *model, char *action);
	team_t get_team();

	int bot_get_path(int item, int self, int *nav_array, path_t &path);
	int bot_follow(path_t &path, int *nav_array, Entity *entity, float speed_scale);

	void load(gametype_t gametype);
	void setup_func(vector<Entity *> &entity_list, Bsp &q3map);
	void unload();
	void add_player(vector<Entity *> &entity_list, playertype_t type, int &ent_id, char *player_name);
	void console(int self, char *cmd, Menu &menu, vector<Entity *> &entity_list);
	void endgame(char *winner);
	void get_state(serverdata_t *data);
	void set_state(serverdata_t *data);

	unsigned int weapon_switch_timer;

	~Quake3()
	{
	};

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
	bool warmup;
	bool faceicon;


//	unsigned int		timelimit;
	unsigned int		round_time;
	unsigned int		warmup_time;

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
};

#endif

