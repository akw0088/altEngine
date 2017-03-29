#include "include.h"

#ifndef COMMANDO_H
#define COMMANDO_H

/*
Remember Commandos: Behind Enemy Lines?
	Think that game in 3d using wolfenstein maps from rtcw and enemy territory

enemy:
	halt: ALTO.WAV
	stab: CUCHI.WAV
	find dead body: MUERTO.WAV
	whose there: QUIENVA.WAV
	distract: SOLDAT.WAV
	alarm: VOZALARM.WAV


GB cant move:
	ESA/GBIM01.WAV
	ESA/GBIM02.WAV
	ESA/GBIM03.WAV

GB can move:
	ESA/GBMV01.WAV - move 1
	ESA/GBMV02.WAV - move 2
	ESA/GBMV03.WAV - move 3
	ESA/GBMV04.WAV - move 4

GB okay:
	GBNM01.WAV - okay1
	GBNM02.WAV - okay1
	GBNM03.WAV - okay1

GB damage:
	GBSD01.WAV - damage1
	GBSD02.WAV - damage1
	GBSD03.WAV - damage1


Spy:
	SPN
Driver:
	DR
Sapper?
	MA
Sniper:
	SN

Green Beret:
	pistol
	knife
	climb
	decoy
	carry barrel
	carry body
	shovel
	noise maker: SEGNUELO.WAV


Sniper:
	pistol
	rifle scope (low ammo)
	first aid kit

Marine:
	pistol
	knife
	boat
	diving gear
	harpoon (close range instant kill)
Sapper:
	pistol
	bear trap
	time bomb
	remote detonate bomb
	grenade
	pliers
Driver:
	Pistol
	submachine gun
	first aid kit
	driving (duh)
	enemy fixed gun control
Spy:
	Pistol
	lethal injection
	get enemy uniform (not from bodies)
	disguise
	distract
	first aid kit
	move bodies
*/

class Commando : public BaseGame
{
public:
	Commando();

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
	void handle_knife(Player &player, int self);
	void handle_rifle(Player &player, int self);
	void handle_machinegun(Player &player, int self);
	void handle_pistol(Player &player, int self);
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

	void load(gametype_t gametype);
	void setup_func(vector<Entity *> &entity_list, Bsp &q3map);
	void unload();
	void init_camera(vector<Entity *> &entity_list);
	void console(int self, char *cmd, Menu &menu, vector<Entity *> &entity_list);
	void endgame(char *winner);

	gametype_t gametype;
	int		fraglimit;
	int		timelimit;
	int		capturelimit;
	unsigned int chat_timer;
	unsigned int notif_timer;
	unsigned int weapon_switch_timer;

private:

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

