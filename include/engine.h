#include "include.h"

#ifndef ENGINE_H
#define ENGINE_H

class Engine
{
	friend class Quake3;
	friend class Commando;

public:
	Engine();
	void init(void *param1, void *param2, char *cmdline);
	void load(char *level);
	void unload();
	void destroy();
	void quit();

	void render(double last_frametime);
	void render_scene(bool lights);
	void render_scene_using_shadowmap(bool lights);
	void render_entities(const matrix4 &trans, matrix4 &proj, bool lights, bool blend, bool vis = true);
	void render_shadow_volumes();


	void render_shadowmaps(bool everything);
	void render_portalcamera();
	void render_texture(int texObj, bool depth_view);
	void post_process(int num_passes, int type);
	void render_bloom(bool debug);
	void render_skyray(bool debug);
	void render_wave(bool debug);
	void resize(int width, int height);
	void fullscreen();

	void step(int tick_num);
	void dynamics();
	void spatial_testing();
	void activate_light(float distance, Light *light);
	void update_audio();
	void set_reference_distance(float value);
	void set_max_distance(float value);
	void set_rolloff_factor(float value);

	bool mousepos(int x, int y, int deltax, int deltay);
	void keypress(char *key, bool pressed);
	void keystroke(char key, char *keystr);

	void load_entities();
	int get_entity();
	int get_player();
	int find_type(int ent_type, int skip);
	void clean_entity(int index);
	void load_models();
	void create_sources();

	void console(char *cmd);
	void handle_game(char key);

	bool collision_detect(RigidBody &body);
	bool map_collision(RigidBody &body);
	bool body_collision(RigidBody &body);

	void get_shaderlist_pk3(char **shaderlist, int &num_shader);

	int bind(int port);
	void connect(char *server);
	void chat(char *name, char *msg);
	void kick(unsigned int i);
	int GetKeyState(input_t &keyboard);
	input_t GetKeyState(int keystate);
	void bind_keys();
	void server_recv();
	void server_send();
	void server_send_state(int client);
	void set_spawn_string(char *msg, client_t *client);
	void parse_spawn_string(char *msg);
	void client_recv();
	void client_send();
	int handle_servermsg(servermsg_t &servermsg, unsigned char *data, reliablemsg_t *reliablemsg);
	void render_weapon(const matrix4 &trans, bool lights, int i);
	void render_trails(matrix4 &trans);
	void render_players(matrix4 &trans, matrix4 &projection, bool lights, bool self);


	int serialize_ents(unsigned char *data, unsigned short int &num_ents, unsigned int &data_size);
	int deserialize_ents(unsigned char *data, unsigned short int num_ents, unsigned int data_size);
	int deserialize_net_player(net_player_t *player, int index, int etype);
	int deserialize_net_rigid(net_rigid_t *rigid, int index, int etype);
	int deserialize_net_trigger(net_trigger_t *net, int index, int etype);

	bool select_wave(int source, char *file);

	// Hit max source limit, only moving entities really need them
	bool play_wave_source(int source, int index);
	// Rest will play from 32 sources round robin style
	int play_wave(vec3 &position, int index);
	int play_wave_loop(vec3 &position, int index);
	int play_wave_global(int index);
	int play_wave_global_loop(int index);
	int get_load_wave(const char *file);
	void screenshot(unsigned int &luminance, bool luminance_only);
	void paste(char *data, unsigned int size);
	void copy(char *data, unsigned int size);

	void destroy_buffers();
	void render_to_framebuffer(double last_frametime);
	void set_dynamic_resolution(double last_frametime);

	void hitscan(vec3 &origin, vec3 &dir, int *index_list, int &num_index, int self, float range);
	void load_md5();
	void find_path(int *&path, int &path_length, int start_path, int end_path);
	void zoom(float level);
	void reload_shaders();
	void enum_resolutions();

	int get_source();
	int get_loop_source();
	int get_global_source();
	int get_global_loop_source();


	unsigned int xres, yres;
	unsigned int tick_num;

	unsigned int lum_table[125];
	unsigned int lum_index;
	unsigned int lum_avg;




//temp
	unsigned int testObj;
	unsigned int num_light;
	int doom_sound;
	unsigned int fullscreen_timer;
	unsigned int ingame_menu_timer;


	unsigned int audio_source[32];
	unsigned int global_source[32];
	unsigned int audio_loop_source[32];
	unsigned int global_loop_source[32];
	int max_sources;

	rendermode_t render_mode;
	bool dynamic_resolution;

	char *pk3_list[32];
	unsigned int num_pk3;
	char *hash_list[32];
	unsigned int num_hash;
	char *cmd_list[512];
	unsigned int num_cmd;
	char *shader_list[256];
	int num_shader;
	HashTable key_bind;

	Entity *thug22;

//console flags
	bool show_names;
	bool show_lines;
	bool show_debug;
	bool show_hud;
	bool collision_detect_enable;
	bool demo;
	bool shadowmaps;
	bool all_lights;
	bool enable_portal;
	bool enable_postprocess;
	bool enable_blur;
	bool enable_emboss;
	bool enable_wave;
	bool enable_bloom;
	bool debug_bloom;
	bool enable_ssao;
	bool enable_stencil;
	bool enable_map_shadows;
	bool enable_entities;
	bool enable_map;


	char resbuf[32][80];
	int num_res;
	int current_res;

	int current_model;
	int num_model;
	int current_team;
	int num_team;


//Game logic
	BaseGame *game;

	MD5Model	zcc;
	MD5Model	sentry;
	MD5Model	zsec_shotgun;
	vector<surface_t *> surface_list;

	unsigned int num_bot;
	unsigned int shadow_light;

	Bsp			q3map;
	float			res_scale;

	//ssao temp
	ScreenSpace ssao;

	void render_ssao(bool debug);


	bool  show_shading;
	bool  show_ao;
	float ssao_level;
	float object_level;
	float ssao_radius;
	bool  weight_by_angle;
	bool randomize_points;
	unsigned int point_count;

	float bloom_threshold;
	float bloom_strength;
	float bloom_amount;

	float dof_near;
	float dof_far;

#ifndef __linux
	HWAVEOUT hWaveOut;
#endif

	float zNear;
	float zFar;
	bool inf;
	float fov; // y fov

protected:
	matrix4		projection;
	matrix4		identity;
	float sensitivity;

	Graphics	gfx;
	Audio		audio;
	input_t		input;
	Menu		menu;
	Frame		camera_frame;


	unsigned int	spiral_vbo;
	unsigned int	spiral_ibo;
	unsigned int	lightning_ibo;
	unsigned int	lightning_vbo;
	unsigned int	global_vao;

	unsigned int	render_fbo;
	unsigned int	mask_fbo;
	unsigned int	blur1_fbo;
	unsigned int	blur2_fbo;
	unsigned int	ssao_fbo;


	unsigned int	render_quad;
	unsigned int	render_depth;
	unsigned int	render_ndepth;

	unsigned int	mask_quad;
	unsigned int	mask_depth;

	unsigned int	blur1_quad;
	unsigned int	blur1_depth;

	unsigned int	blur2_quad;
	unsigned int	blur2_depth;

	unsigned int	ssao_quad;
	unsigned int	ssao_depth;




	unsigned int	fb_width;
	unsigned int	fb_height;
	unsigned int	multisample;
	unsigned int	no_tex;
	unsigned int	particle_tex;

	unsigned int	palette1;
	unsigned int	palette2;


	bool			voted;
	unsigned int	vote_yes;
	unsigned int	vote_no;

	vector<Entity *>	entity_list;
	unsigned int	max_dynamic;
	unsigned int	max_player;

	vector<Light *>		light_list;
	vector<wave_t>		snd_wave;


	//net stuff
	int		sequence;
	Net		net;
	reliablemsg_t	reliable[8];
	reliablemsg_t	client_reliable;
	int cl_skip;
	char sv_hostname[512];
	char sv_motd[512];
	char password[512];
	unsigned int sv_maxclients;
	unsigned short int net_port;

	bool recording_demo;
	bool playing_demo;
	FILE *demofile;


	net_entity_t delta_list[512];

	
	int		server_spawn;	// entity index of hosting player

	// pathfinding stuff
	Graph		graph;
	graph_node_t	*node;
	ref_t		*ref;


	//server
	bool	server_flag;
	vector <client_t *> client_list;
	
	//client
	bool	client_flag;
	unsigned int	last_server_sequence;
	unsigned int	qport;
	bool active_clients[8];

	netinfo_t netinfo;

	//Shaders
	Global				global; // basic shader for menu's etc
	Post				post;	// post process shader
	mLight2				mlight2;	// main shader for lighting

	ParticleUpdate		particle_update;
	ParticleRender		particle_render;
	emitter_t			emitter;
	unsigned int		emitter_count;


	bool	initialized;
	void	*param1;
	void	*param2;
};

#endif

