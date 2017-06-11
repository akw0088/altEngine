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
	void render_entities(const matrix4 &trans, bool lights, bool blend);
	void render_shadow_volumes(int current_light);


	void render_shadowmaps();
	void render_texture(int texObj);
	void post_process(int num_passes);
	void resize(int width, int height);

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
	void keystroke(char key);

	void load_entities();
	int get_entity();
	int get_player();
	int find_type(char *type, int skip);
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
	void client_recv();
	void client_send();
	int handle_servermsg(servermsg_t &servermsg, reliablemsg_t *reliablemsg);
	void render_weapon(const matrix4 &trans, bool lights, int i);
	void render_trails(matrix4 &trans);
	void render_players(matrix4 &trans, bool lights, bool self);
	int serialize_ents(char *data, unsigned short int &num_ents);
	int deserialize_ents(char *data, unsigned short int num_ents);

	bool select_wave(int source, char *file);

	// Hit max source limit, only moving entities really need them
	bool play_wave_source(int source, int index);
	// Rest will play from 32 sources round robin style
	int play_wave(vec3 &position, int index);
	int play_wave_loop(vec3 &position, int index);
	int play_wave_global(int index);
	int play_wave_global_loop(int index);
	int get_load_wave(const char *file);

	void destroy_buffers();
	void render_to_framebuffer(double last_frametime);
	void set_dynamic_resolution(double last_frametime);
	void handle_input();

	void hitscan(vec3 &origin, vec3 &dir, int *index_list, int &num_index, int self, float range);
	void load_md5();
	void find_path(int *&path, int &path_length, int start_path, int end_path);
	void zoom(float level);
	void reload_shaders();

	int get_source();
	int get_loop_source();
	int get_global_source();
	int get_global_loop_source();

	unsigned int xres, yres;
	unsigned int tick_num;


//temp
	unsigned int testObj;
	unsigned int num_light;
	int doom_sound;


	unsigned int audio_source[32];
	unsigned int global_source[32];
	unsigned int audio_loop_source[32];
	unsigned int global_loop_source[32];
	int max_sources;

	rendermode_t render_mode;
	bool dynamic_resolution;


	char *pk3_list[32];
	char *hash_list[32];
	int num_pk3;
	int num_hash;
	char *shader_list[256];
	int num_shader;
	HashTable key_bind;

	// for cloning
	Entity *box;
	Entity *ball;
	Entity *thug22;
	Entity *rocket;
	Entity *pineapple;
	Entity *shell;
	Entity *bullet;
	Entity *bullet_hit;
	Entity *plasma_hit;
	Entity *mark;
	Entity *gib0;
	Entity *gib1;
	Entity *gib2;
	Entity *gib3;
	Entity *gib4;
	Entity *gib5;
	Entity *gib6;
	Entity *gib7;
	Entity *gib8;
	Entity *gib9;




//console flags
	bool show_names;
	bool show_lines;
	bool show_debug;
	bool show_hud;
	bool collision_detect_enable;
	bool demo;
	bool shadowmaps;
	bool all_lights;

//Game logic
	BaseGame *game;

	MD5Model	zcc;
	MD5Model	sentry;
	MD5Model	zsec_shotgun;
	vector<surface_t *> surface_list;

	unsigned int num_bot;
	Bsp			q3map;
	float			res_scale;


protected:
	matrix4		projection;
	matrix4		identity;
	float zNear;
	float zFar;
	bool inf;
	float fov; // y fov
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
	unsigned int	fbo;
	unsigned int	global_vao;
	unsigned int	quad_tex;
	unsigned int	depth_tex;
	unsigned int	fb_width;
	unsigned int	fb_height;
	unsigned int	multisample;
	unsigned int	no_tex;
	unsigned int	particle_tex;

	vector<Entity *>	entity_list;
	unsigned int	max_dynamic;
	unsigned int	max_player;

	vector<Light *>		light_list;
	vector<wave_t>		snd_wave;


	//net stuff
	int		sequence;
	Net		net;
	reliablemsg_t	reliable;
	int cl_skip;
	char servername[512];
	char password[512];

	bool recording_demo;
	bool playing_demo;
	FILE *demofile;
	
	
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

	netinfo_t netinfo;

	//Shaders
	Global				global; // basic shader for menu's etc
	Post				post;	// post process shader
	mLight2				mlight2;	// main shader for lighting

	ParticleUpdate		particle_update;
	ParticleRender		particle_render;
	emitter_t			emitter;
	unsigned int		emitter_count;

	bool entities_enabled;

	bool	initialized;
	void	*param1;
	void	*param2;
};

#endif

