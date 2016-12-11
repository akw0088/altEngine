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
	void render_hud(double last_frametime);
	void draw_name(vec4 &pos, Entity *entity, Menu &menu, vec3 &color);

	void step(int frame_step);
	void dynamics();
	void spatial_testing();
	void activate_light(float distance, Light *light);
	void check_triggers();
	void update_audio();

	bool mousepos(int x, int y, int deltax, int deltay);
	void keypress(char *key, bool pressed);
	void keystroke(char key);

	void init_camera();
	void load_entities();
	int get_entity();
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
	void chat(char *msg);
	void kick(unsigned int i);
	int GetKeyState(button_t &keyboard);
	button_t GetKeyState(int keystate);
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

//temp
	unsigned int xres, yres;
	unsigned int testObj;
	unsigned int num_light;
	bool show_names;
	bool show_debug;
	bool show_hud;

	Quake3 q3;

	int		frame_step;
	MD5Model	zcc;
	MD5Model	sentry;
	MD5Model	zsec_shotgun;

	// for cloning
	Entity *box;
	Entity *ball;
	Entity *thug22;
	Entity *rocket;
	Entity *pineapple;


protected:
	matrix4		projection;
	matrix4		identity;

	Graphics	gfx;
	Audio		audio;
	Bsp		map;
	button_t	input;
	Menu		menu;
	Frame		camera_frame;
	Frame		light_frame;

	unsigned int	fbo;
	unsigned int	global_vao;
	unsigned int	quad_tex;
	unsigned int	depth_tex;
	unsigned int	fb_width;
	unsigned int	fb_height;
	unsigned int	no_tex;
	unsigned int	num_dynamic;

	vector<Entity *>	entity_list;
	vector<Light *>		light_list;
	vector<wave_t>		snd_wave;


	//net stuff
	int		sequence;
	Net		net;
	reliablemsg_t	reliable;
	int		spawn;			// entity index of self
	int		server_spawn;	// entity index of hosting player



	//server
	bool	server_flag;
	vector <client_t *> client_list;
	
	//client
	bool	client_flag;
	unsigned int	last_server_sequence;

	//Shaders
	Post		post;
	mLight2		mlight2;
	mLight3		mlight3;
	Global		global;
	ShadowMap	shadowmap;
	matrix4		shadowmvp;

	bool	initialized;
	void	*param1;
	void	*param2;

};

#endif

