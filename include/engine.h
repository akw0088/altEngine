//=============================================================================
// This file is part of the altEngine distribution
// (https://github.com/akw0088/altEngine/)
// Copyright (c) 2018 Alexander Wright All Rights Reserved.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON - INFRINGEMENT.IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//=============================================================================

#include "include.h"

#define VOICE_SAMPLE_RATE 48000
#define VOICE_FORMAT	AL_FORMAT_MONO8

#define MAX_DEPTH 6

/*
Frame

0-5 run axe						5 frames
6-11 run gun					5 frames

29-34 axe pain					6 frames 29 30 31 32 33 34
35-40 gun pain					5 frames



41-49 axe death					8 frames


50-60 choke death gun			10 frames
61-69 sit death gun				8 frames
70-84 fall death gun			14 frames
85-93 fall face down death gun	8 frames
94-102 shot belly death gun		8 frames
*/


//0-5 run axe						5 frames
#define RUN_AXE 0
#define LENGTH_RUN_AXE 5
//6 - 11 run gun					5 frames
#define RUN_GUN 6
#define LENGTH_RUN_GUN 5
//12 - 16 idle gun					5 frames
#define IDLE_GUN 12
#define LENGTH_IDLE_GUN 5
//17 - 28 idle axe					12 frames
#define IDLE_AXE 17
#define LENGTH_IDLE_AXE 12
//29 - 34 axe pain					6 frames
#define PAIN_AXE 29
#define LENGTH_PAIN_AXE 6
//35 - 40 gun paine					5 frames
#define PAIN_GUN 35
#define LENGTH_PAIN_GUN 5
//41 - 49 axe death					8 frames
#define DEATH_AXE 35
#define LENGTH_DEATH_AXE 5
//50 - 60 choke death gun			10 frames
#define DEATH0_GUN 50
#define LENGTH_DEATH0_GUN 10
//61 - 69 sit death gun				8 frames
#define DEATH1_GUN 61
#define LENGTH_DEATH1_GUN 8
//70 - 84 fall death gun			14 frames
#define DEATH2_GUN 70
#define LENGTH_DEATH2_GUN 14
//85 - 93 fall face down death gun	8 frames
#define DEATH3_GUN 85
#define LENGTH_DEATH3_GUN 8
//94 - 102 shot belly death gun		8 frames
#define DEATH4_GUN 94
#define LENGTH_DEATH4_GUN 8
//103 - 106 shoot gun - no kick		3 frames
#define FIRE_GUN_SM 103
#define LENGTH_FIRE_GUN_SM 3
//107 - 112 shoot gun big kickback	5 frames
#define FIRE_GUN_BIG 107
#define LENGTH_FIRE_GUN_BIG 5
//113 - 118 shoot gun medium kick	5 frames
#define FIRE_GUN_MED 113
#define LENGTH_FIRE_GUN_MED 5
//119 - 123 axe swing 1				4 frames
#define FIRE_AXE0 119
#define LENGTH_FIRE_AXE0 4
//124 - 128 axe swing 2				4 frames
#define FIRE_AXE1 124
#define LENGTH_FIRE_AXE1 4
//129 - 136 axe swing 3				7 frames
#define FIRE_AXE2 129
#define LENGTH_FIRE_AXE2 7
//137 - 144 axe swing 4				7 frames
#define FIRE_AXE3 137
#define LENGTH_FIRE_AXE3 6


typedef enum
{
	RANGER,
	GRUNT,
	DOG,
	OGRE,
	ZOMBIE,
	WIZARD,
	KNIGHT,
	DEMON,
	SHAMBLER,
	DEATH_KNIGHT,
	ENFORCER,
	FISH,
	SPAWN,
	VORE,
	MAX_MODEL
} quake_model_t;


// Height above desired position we allow a step to occur
// (Fairly large as forward velocity can put you fairly deep forward)
#define STAIR_HEIGHT 20.0f
// Stair position and velocity added when we step up a stair
// Too much causes bouncing when standing still
// Too little and we dont walk up steps :)
#define STAIR_POS	1.00f
// Too little and we have lots of friction on flat planes
// Too much causes jumping up stairs
#define STAIR_VEL	0.25f


typedef enum {
	PROP_POSITION,
	PROP_COLOR,
	PROP_INTENSITY,
	MAX_PROP
} property_t;

#ifndef ENGINE_H
#define ENGINE_H

class Engine
{
	friend class Quake3;
	friend class Commando;
	friend class Netcode;

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
	void render_weapon(const matrix4 &trans, bool lights, int i);
	void render_trails(matrix4 &trans);
	void render_players(matrix4 &trans, matrix4 &projection, bool lights, bool self);
	void render_shadowmaps(bool everything);
	void render_portalcamera();
	void render_texture(int texObj, bool depth_view);
	void render_bloom(bool debug);
	void render_skyray(bool debug);
	void render_wave(bool debug);
	void post_process(int num_passes, int type);
	void resize(int width, int height);
	void fullscreen();
	void check_pk3_md5sum();
	void load_q3_shaders();
	void test_triangle();

	void step(int tick_num);
	void dynamics();
	void handle_springs();
	void handle_cloth();
	void spatial_testing();
	void activate_light(float distance, EntLight *light);
	void update_audio();
	void set_reference_distance(float value);
	void set_max_distance(float value);
	void set_rolloff_factor(float value);

	bool mousepos(int x, int y, int deltax, int deltay);
	bool mousepos_raw(int x, int y, int deltax, int deltay);
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
	int console_general(char *cmd);
	int console_network(char *cmd);
	int console_render(char *cmd);
	int console_sound(char *cmd);


	void handle_game(char key);

	bool collision_detect(EntRigidBody &body);
	bool map_collision(EntRigidBody &body);

	bool body_collision(EntRigidBody &body);

	void get_shaderlist_pk3(char **shaderlist, int &num_shader);

	void bind_keys();



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

	void hitscan(vec3 &origin, vec3 &dir, int *index_list, int &num_index, int self, float range, bool light = false);
	void rayscan(vec3 &origin, vec3 &dir, int *index_list, int &num_index, int self, float range, bool light);
	void load_md5();
	void load_quake_md5();
	void find_path(int *&path, int &path_length, int start_path, int end_path);
	void zoom(float level);
	void reload_shaders();
	void enum_resolutions();
	void CreateObjects();
	void draw_plane(Graphics &gfx, plane_t &plane, vec3 &fwd, vec3 &origin);

	int get_source();
	int get_loop_source();
	int get_global_source();
	int get_global_loop_source();

	void savegame(char *file);
	void loadgame(char *file);

	int debug_triangle(vec3 *triangle);
	int debug_vector(vec3 &pos, vec3 &dir);
	int debug_point(vec3 &pos);
	void Pick(int x, int y, int width, int height, Frame &frame);
	void MousetoRay(int x, int y, matrix4 &model, matrix4 &proj, const Frame &frame, vec3 &start, vec3 &end, vec3 &dir, int width, int height);
	void edit_entity(unsigned int index);

	vector<vertex_t> debug_tri_list;
	vector<int> debug_tri_list_index;

	vector<vertex_t> debug_vec_list;
	vector<int> debug_vec_list_index;


	unsigned int xres, yres;
	unsigned int tick_num;

	vector<char *> serverlist;

	int quad_index;
	int quad_vertex;

	int picked_ent;
	int edit_ent;
	int edit_prop;

//temp
	vector<cloth::Cloth *> cloth;
	vec3 ball_pos;
	float ball_radius;
	float ball_time;


	unsigned int testObj;
	unsigned int num_light;
	int doom_sound;
	unsigned int fullscreen_timer;
	unsigned int ingame_menu_timer;
	Terrain terrain;
	IsoSphere isosphere[10];
	IsoCube isocube[10];

	unsigned int audio_source[32];
	unsigned int global_source[32];
	unsigned int audio_loop_source[32];
	unsigned int global_loop_source[32];
	int filter;
	int filter_index;
	int max_sources;
	bool raw_mouse;
	

	rendermode_t render_mode;
	bool dynamic_resolution;

	char *pk3_list[64];
	unsigned int num_pk3;
	char hash_result[64][32];

	unsigned int controller;

	frustum_t *pfrustum;
	frustum_t frustum;

	char server_comport[32];
	char client_comport[32];


#ifdef OCULUS
	Oculus ovr;
	matrix3 head, lf, rh, lh, le, re;
	vec3 hpos, lp, rp, rep, lep;
	ovrtouch_t touch;
#endif


	char *cmd_list[1024];
	unsigned int num_cmd;
	char *shader_list[256];
	int num_shader;
	HashTable key_bind;

	char *pk3list;
	char *cmdlist;
	char *hashlist;
	char *hacklist;
	char *masterlist;
	char *wad;

	Entity *thug22;
	vector<Entity *>	entity_list;




//console flags
	bool show_names;
	bool show_box;
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
	bool enable_terrain;
	bool enable_planet;
	bool enable_cloth;
	bool pick_mode;


	char resbuf[MAX_RES][80];
	int num_res;
	int current_res;

	int current_model;
	int num_model;
	int current_team;
	int num_team;


	int debug_tri_index;
	int debug_tri_vert;
	int debug_vec_index;
	int debug_vec_vert;

//Game logic
	BaseGame *game;
	Netcode netcode;


	MD5Model md5_model[16];
	vector<surface_t *> surface_list;

	unsigned int num_bot;
	unsigned int shadow_light;

	Bsp				q3map;
	HLBsp			hlmap;
	Q1Bsp			q1map;
	float			res_scale;

	//ssao temp
	ScreenSpace ssao;

	void render_ssao(bool debug);


	bool  show_shading;
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

#ifdef WIN32
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
	unsigned int	box_ibo;
	unsigned int	box_vbo;
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
	int				dissolve_tex;
	unsigned int	particle_tex;

	unsigned int	palette1;
	unsigned int	palette2;


	bool			voted;
	unsigned int	vote_yes;
	unsigned int	vote_no;

	
	unsigned int	max_dynamic;
	unsigned int	max_player;

	vector<EntLight *>		light_list;
	vector<wave_t>		snd_wave;


#ifdef VOICECHAT
	Voice voice;
#endif
	// pathfinding stuff
	Graph		graph;
	graph_node_t	*node;
	ref_t		*ref;

	int num_node;
	int num_ref;


	char **animation;



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

