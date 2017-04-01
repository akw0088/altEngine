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
	void load_sounds(Audio &audio, std::vector<wave_t> &snd_wave);


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
	void add_player(vector<Entity *> &entity_list, char *type, int &ent_id, char *player_name);
	void console(int self, char *cmd, Menu &menu, vector<Entity *> &entity_list);
	void endgame(char *winner);

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


	unsigned int		timelimit;
	unsigned int		round_time;
	unsigned int		warmup_time;

	unsigned int win_timer;
	char win_msg[128];

	unsigned int num_player;
	unsigned int num_player_blue;
	unsigned int num_player_red;
	unsigned int spectator_timer;
	unsigned int global_source;


	bool played_one_frag;
	bool played_two_frag;
	bool played_three_frag;
	bool played_prepare;
	bool played_sudden;



	char empty_sound[128];
	char pad_sound[128];
	char land_sound[128];
	char weapon_swap_sound[128];
	char medikit_sound[128];
	char noitem_sound[128];
	char regen_bump_sound[128];

	char step1_sound[128];
	char step2_sound[128];
	char step3_sound[128];
	char step4_sound[128];

	char waterin_sound[128];
	char waterout_sound[128];

	char telein_sound[128];
	char teleout_sound[128];

	char gurp1_sound[128];
	char gurp2_sound[128];

	char gibbed_sound[128];
	char gibimpact1_sound[128];
	char gibimpact2_sound[128];
	char gibimpact3_sound[128];

	char chat_sound[128];
	char capture_sound[128];

	char one_min_sound[128];
	char five_min_sound[128];

	char one_frag_sound[128];
	char two_frag_sound[128];
	char three_frag_sound[128];

	char blue_lead_sound[128];
	char red_lead_sound[128];
	char teams_tied_sound[128];

	char tied_lead_sound[128];
	char lost_lead_sound[128];
	char taken_lead_sound[128];

	char excellent_sound[128];
	char gauntlet_sound[128];
	char humiliation_sound[128];
	char impressive_a_sound[128];
	char accuracy_sound[128];
	char excellent_a_sound[128];
	char frag_sound[128];
	char impressive_sound[128];

	char prepare_sound[128];
	char fight_sound[128];
	char denied_sound[128];
	char hit_teammate_sound[128];

	char one_sound[128];
	char two_sound[128];
	char three_sound[128];

	char hit_sound[128];
	char perfect_sound[128];
	char sudden_death_sound[128];
};

#endif

