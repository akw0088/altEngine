#include "include.h"

#ifndef QUAKE3_H
#define QUAKE3_H


#define MODEL_WEAPON_GAUNTLET 19
#define MODEL_WEAPON_MACHINEGUN 20
#define MODEL_WEAPON_SHOTGUN 21
#define MODEL_WEAPON_GRENADE 22
#define MODEL_WEAPON_ROCKET 23
#define MODEL_WEAPON_LIGHTNING 24
#define MODEL_WEAPON_RAILGUN 25
#define MODEL_WEAPON_PLASMA 26

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

#define GIB_HEALTH -40


class Quake3 : public BaseGame
{
public:
	Quake3();

	void init(Engine *engine);
	void step(int frame_step);
	void destroy();


	void check_triggers(Player *player, Entity *ent, int self, vector<Entity *> &entity_list);
	void check_projectiles(Player *player, Entity *ent, Entity *owner, int self, int proj_id, vector<Entity *> &entity_list);
	void check_func(Player *player, Entity *ent, int self, vector<Entity *> &entity_list);

	void render_hud(double last_frametime);
	void draw_name(Entity *entity, Menu &menu, matrix4 &real_projection, int ent_num);
	void draw_line(Entity *a, Entity *b, Menu &menu, vec3 &color);
	void transform_3d_2d(vec3 &position, vec3 &pos2d, matrix4 &projection);

	void handle_weapons(Player &player, input_t &input, int self, bool client);

	void handle_gauntlet(Player &player, int self, bool client);
	void handle_plasma(Player &player, int self, bool client);
	void handle_rocketlauncher(Player &player, Constructable *sentry, int self, bool client);
	void handle_grenade(Player &player, int self, bool client);
	void handle_lightning(Player &player, int self, bool client);
	void handle_railgun(Player &player, int self, bool client);
	void handle_machinegun(Player &player, Constructable *sentry, int self, bool client);
	void handle_shotgun(Player &player, int self, bool client);
	void handle_gibs(Player &player);
	void create_crosshair();
	void draw_crosshair();
	void draw_flash(Player *player);
	void add_decal(vec3 &start, Frame &camera_frame, net_ent_t nettype, Model &decal_model, float offset, bool explode, int explode_timer);

	void create_icon();
	void draw_icon(float scale, int index, float x = 0.0f, float y = 0.0f, float z = 0.0f);
	void load_icon();
	void map_model(Entity *ent);
	void load_sounds(Audio &audio, vector<wave_t> &snd_wave);
	void handle_model_trigger(vector<Entity *> &entity_list, Entity *ent, int self);
	void check_target(vector<Entity *> &entity_list, Entity *ent, Entity *target, int self);

	void make_dynamic_ent(net_ent_t item, int ent_id);
	void handle_player(int index, input_t &input);
	void handle_frags_left(Player &player);
	void player_died(int index);
	void drop_weapon(int index);
	void drop_powerup(vec3 &position, char *model, char *action);
	team_t get_team();

	int bot_get_path(int item, int self, int *nav_array, path_t &path);
	int bot_follow(path_t &path, int *nav_array, Entity *entity, float speed_scale);

	void load(gametype_t gametype);
	void load_models(Graphics &gfx);
	void load_q1_models(Graphics &gfx);
	void setup_func(vector<Entity *> &entity_list, Bsp *q3map);
	void unload();
	void add_player(vector<Entity *> &entity_list, playertype_t type, int &ent_id, char *player_name);
	void console(int self, char *cmd, Menu &menu, vector<Entity *> &entity_list);
	void endgame(char *winner);
	void get_state(serverdata_t *data);
	void set_state(serverdata_t *data);

	void handle_func_platform(Entity *ent);
	void handle_func_bobbing(Entity *ent);
	void handle_func_train(Entity *ent);
	int add_train_path(Entity *original, Entity *ref, Entity *target);

	void build_sentry();

	//base game items
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

	unsigned int weapon_switch_timer;

	~Quake3();

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


	// previous in basegame
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

#endif

