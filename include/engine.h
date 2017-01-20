#include "include.h"

#ifndef ENGINE_H
#define ENGINE_H

class Engine
{
	friend class Quake3;

public:
	Engine();
	void init(void *param1, void *param2);
	void load(char *level);
	void unload();
	void destroy();
	void quit();

	void render(double last_frametime);
	void render_scene(bool lights);
	void render_scene_using_shadowmap(bool lights);
	void render_entities(const matrix4 &trans, bool lights);
	void render_shadow_volumes(int current_light);


	void render_shadowmaps();
	void render_texture(int texObj);
	void post_process(int num_passes);
	void resize(int width, int height);

	void step(int tick_num);
	void dynamics();
	void spatial_testing();
	void activate_light(float distance, Light *light);
	void check_triggers(int player_index);
	void update_audio();

	bool mousepos(int x, int y, int deltax, int deltay);
	void keypress(char *key, bool pressed);
	void keystroke(char key);

	void load_entities();
	int get_entity();
	int get_player();
	int find_type(char *type, int skip);
	void clean_entity(int index);
	void load_sounds();
	void load_models();
	void load_model(Entity &ent);
	void create_sources();

	void console(char *cmd);
	void handle_game(char key);

	bool collision_detect(RigidBody &body);
	bool map_collision(RigidBody &body);
	bool body_collision(RigidBody &body);

	int bind(int port);
	void connect(char *server);
	void chat(char *name, char *msg);
	void kick(unsigned int i);
	int GetKeyState(input_t &keyboard);
	input_t GetKeyState(int keystate);
	void bind_keys();
	void server_step();
	void client_step();
	void send_entities();
	void render_client(int i, const matrix4 &trans, bool lights, bool hack);


	bool select_wave(int source, char *file);
	void destroy_buffers();
	void render_to_framebuffer();
	void handle_input();

	void hitscan(vec3 &origin, vec3 &dir, int *index_list, int &num_index, int self);
	void load_md5();
	void find_path(int *&path, int &path_length, int start_path, int end_path);
	void zoom(float level);
	void reload_shaders();

	unsigned int xres, yres;
	unsigned int tick_num;

//temp
	unsigned int testObj;
	unsigned int num_light;
	char *pk3_list[32];
	char *hash_list[32];
	int num_pk3;
	int num_hash;
	char *shader_list[32];
	int num_shader;
	HashTable key_bind;
//	map<string, string> key_bind;

	// for cloning
	Entity *box;
	Entity *ball;
	Entity *thug22;
	Entity *rocket;
	Entity *pineapple;
	Entity *shell;
	Entity *bullet;
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

//Game logic
	Quake3 q3;

	MD5Model	zcc;
	MD5Model	sentry;
	MD5Model	zsec_shotgun;
	vector<surface_t *> surface_list;

	unsigned int num_bot;

protected:
	matrix4		projection;
	matrix4		identity;
	float zNear;
	float zFar;
	bool inf;
	float fov; // y fov

	Graphics	gfx;
	Audio		audio;
	Bsp			q3map;
	input_t		input;
	Menu		menu;
	Frame		camera_frame;
	Frame		light_frame;


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
	unsigned int	no_tex;
	unsigned int	particle_tex;

	vector<Entity *>	entity_list;
	unsigned int	num_dynamic;
	unsigned int	num_player;

	vector<Light *>		light_list;
	vector<wave_t>		snd_wave;


	//net stuff
	int		sequence;
	Net		net;
	reliablemsg_t	reliable;
	
	
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

	//Shaders
	Global				global; // basic shader for menu's etc
	Post				post;	// post process shader
	mLight2				mlight2;	// main shader for lighting

	ParticleUpdate		particle_update;
	ParticleRender		particle_render;
	emitter_t			emitter;

	bool entities_enabled;

	bool	initialized;
	void	*param1;
	void	*param2;
};

#endif

