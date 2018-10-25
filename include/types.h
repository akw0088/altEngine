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

#ifndef TYPES_H
#define TYPES_H
typedef unsigned int uint;

#define MAX_TEXTURES 4 // multi texture limits (quake 3 shader stages)
#define LINE_SIZE 512

typedef enum
{
	PRIM_TRIANGLES,
	PRIM_TRIANGLE_STRIP,
	PRIM_LINE_STRIP,
	PRIM_LINES,
	PRIM_POINTS
} primitive_t;

typedef struct
{
	vec3	position;		// (x, y, z) position. 
	vec2	texCoord0;	// (u, v) texture coordinate
	vec2	texCoord1;	// (u, v) lightmap coordinate
	vec3	normal;		// (x, y, z) normal vector
	int		color;		// RGBA color for the vertex 
	vec4	tangent;
} vertex_t;

typedef struct
{
	vec3	normal;		// Plane normal. 
	float	d;			// The plane distance from origin 
} plane_t;


typedef struct
{
	short	format;
	short	channels;
	int	sampleRate;
	int	avgSampleRate;
	short	align;
	short	sampleSize;
} waveFormat_t;

typedef struct
{
	char			file[LINE_SIZE];
	waveFormat_t	*format;
	void			*pcmData;
	int				dataSize;
	int				duration;
	char			*data;
	int				buffer;
} wave_t;

/*
	Temporarily saves original entity position
	that may be restored if integration yields
	too much interpenetration.
*/
typedef struct
{
	matrix3 morientation;
	matrix3	world_tensor;
	vec3	velocity;
	vec3	angular_velocity;
	vec3	position;
} cfg_t;

/*
	Holds data read in from menu definition files
*/
typedef struct
{
	int state;
	float position[3];
	float scale;
	float color[3];
	int flag;
	int mouse_state;
	char msg[128];
} menu_t;

/*
	Holds state transition information for menu definition files
*/
typedef struct
{
	int start;
	char delta[128];
	int end;
	char cmd[128];
} state_t;





// size of fixed part of network packets
// length + sequence + ack + num_cmds
#define CLIENT_HEADER 45

#ifdef LINUX
typedef unsigned char byte;
#endif

/*
	Variable length client msg
	after num_cmds is a variable number of key / cmd states
	after keystate list is a null terminated reliably
	transmitted msg string that is retransmitted until acked.
*/
typedef struct
{
	unsigned short int	length;
	unsigned short int	qport;
	unsigned short int	sequence;
	unsigned short int	server_sequence;
	float			up[3];
	float			forward[3];
	float			pos[3];		// Sending position (not used, but interesting to calculate delta's on server)
	char			num_cmds;
	unsigned char		data[16834];
} clientmsg_t;


#define SERVER_HEADER 14
/*
	Variable length server msg
	after num_ents is a variable number of entity states
	after entity state list is a null terminated reliably
	transmitted msg string that is retransmitted until acked.
*/
#pragma pack(1)
typedef struct
{
	unsigned short int	length;
	unsigned short int	compressed_size;
	unsigned short int	sequence;
	unsigned short int	client_sequence;
	unsigned short int	num_ents;	// not really neeeded with size, but worth the two bytes info wise
	unsigned int		data_size;
	unsigned char		data[16384];
} servermsg_t;
#pragma pack(8)

typedef struct
{
	unsigned short int	sequence;
	unsigned short int	size; // size of entire header + data
	char			msg[8192];
} reliablemsg_t;

typedef struct
{
	unsigned short int sequence;
	unsigned short int qport;
	unsigned char data[8192];
} voicemsg_t;
#define VOICE_HEADER 4

typedef struct
{
	bool attack;
	bool use;
	bool zoom;
	bool jump;
	bool escape;
	bool duck;
	bool control;
	bool pickup;
	bool moveup;
	bool moveleft;
	bool movedown;
	bool moveright;
	bool walk;

	bool weapon_up;
	bool weapon_down;

	bool scores;
} input_t;

typedef struct
{
	int sequence_delta;
	int ping;
	int ping_tick;
	int dropped;
	int num_ents;
	int size;
	int uncompressed_size;
	bool send_full;
	bool send_partial;
	bool recv_empty;
} netinfo_t;

typedef struct
{
	char			socketname[32];
	unsigned int	qport;
	int				ent_id;
	unsigned short int	client_sequence;
	unsigned short int	server_sequence;
	unsigned int	last_time;
	input_t			input;
	netinfo_t		netinfo;
	bool			needs_state;
	vec3			position_delta;
} client_t;


//8 units equal 1 foot
//sin -1,1
//triangle 0,1
//square -1,1
//sawtooth 0,1,0
//inverse 0,1,0
//sawtooth 0,1,0
//base amp phase frequency

typedef struct
{
	char wave[64];
	float div;
	float func;
	float base;
	float amplitude;
	float phase;
	float freq;
} deform_t;

typedef struct
{
	//	char *stage; //raw parser output

	char map_tex[128];
	char clampmap_tex[128];
	char anim_map_tex[512];
	float anim_map_freq;

	struct flag
	{
		uint64_t map : 1,
			clampmap : 1,
			anim_map : 1,
			lightmap : 1,
			alpha : 1,
			blendfunc_add : 1,
			blendfunc_filter : 1,
			blendfunc_blend : 1,	//source * srccoef + dest * destcoef
			blend_zero_one : 1,
			blend_one_zero : 1,
			blend_one_one : 1,
			blend_one_src_color : 1,
			blend_dst_color_one : 1,
			blend_dst_color_zero : 1,
			blend_dst_color_one_minus_dst_alpha : 1,
			one_minus_src_alpha_src_alpha : 1,
			blend_dst_color_src_alpha : 1,
			blend_one_minus_src_alpha_src_alpha : 1,
			blend_src_alpha_one_minus_src_alpha : 1,
			blend_one_src_alpha : 1,
			blend_one_minus_dst_color_zero : 1,
			blend_zero_src_color : 1,
			blend_dst_color_src_color : 1,
			blend_zero_src_alpha : 1,
			alpha_gt0 : 1,
			alpha_ge128 : 1,
			alpha_lt128 : 1,
			depth_write : 1,
			depthfunc_equal : 1,
			rgbgen_identity : 1,
			tcmod_rotate : 1,
			tcmod_scale : 1,
			tcmod_stretch_sin : 1,
			tcmod_stretch_sawtooth : 1,
			tcmod_stretch_inverse_sawtooth : 1,
			tcmod_stretch_square : 1,
			tcmod_stretch_triangle : 1,
			tcmod_scroll : 1,
			tcmod_turb : 1,
			tcgen_env : 1,
			rgbgen_wave_sin : 1,
			rgbgen_wave_sawtooth : 1,
			rgbgen_wave_inverse_sawtooth : 1,
			rgbgen_wave_square : 1,
			rgbgen_wave_triangle : 1;
	} flags;

	float tcmod_rotate_value; // deg/sec
	vec2 tcmod_scale_value;
	vec4 tcmod_stretch_value;
	vec2 tcmod_scroll_value;
	vec4 tcmod_turb_value;
	vec4 rgbgen_wave_value;

} stage_t;


// Keeping size small, because there are a lot of these
struct surface_t
{
	char file[128];
	char name[128];
	//	char *cmd[64];	//raw parser output
	stage_t stage[8]; //looks like it quake3 maxes out at 4, but doing 8
	unsigned int num_stage;


	struct flag
	{
		uint64_t nomipmaps : 1,
			nopicmip : 1,
			polygon_offset : 1,
			portal : 1, //sort value 1:portal,2:sky,3:opaque,6:banner,8:underwater,9:additive,16:nearest
			surfaceparm_trans : 1,
			surfaceparm_nonsolid : 1,
			surfaceparm_noclip : 1,
			surfaceparm_nodraw : 1,
			surfaceparm_nodrop : 1,
			surfaceparm_nodlight : 1,
			surfaceparm_areaportal : 1,
			surfaceparm_clusterportal : 1,
			surfaceparm_donotenter : 1,
			surfaceparm_origin : 1,
			surfaceparm_detail : 1,
			surfaceparm_playerclip : 1,
			surfaceparm_water : 1,
			surfaceparm_slime : 1,
			surfaceparm_lava : 1,
			surfaceparm_slick : 1,
			surfaceparm_structural : 1,
			surfaceparm_fog : 1,
			surfaceparm_sky : 1,
			surfaceparm_nolightmap : 1,
			surfaceparm_nodamage : 1,
			surfaceparm_noimpact : 1,
			surfaceparm_nomarks : 1,
			surfaceparm_metalsteps : 1,
			surfaceparm_alphashadow : 1,
			q3map_surfacelight : 1,
			q3map_sun : 1,
			cull_disable : 1,
			cull_none : 1,
			cull_backside : 1,
			cull_twosided : 1,
			deformVertexes : 1,
			fog : 1;
	} flags;

	int q3map_surfacelight_value;
	vec3 q3map_sun_value[2]; //rgb + intensity degrees elevation
	vec3 fog_color;
	int fog_density;
	deform_t deform;
};


// Maps opengl texture object to q3shader index and stage (if available)
typedef struct
{
	char name[128];
	int texObj[MAX_TEXTURES];
	int texObjAnim[8];
	int anim_unit;
	int freq;
	int num_anim;
	int anim_count;
	int index; // index into surface list (if it exists)
	int num_tex;
	bool portal;
} texture_t;

typedef struct
{
	char *name;
	int face;
	bool shader;
	bool sky;
	bool tcmod_rotate[MAX_TEXTURES];
	bool tcmod_scroll[MAX_TEXTURES];
	bool tcmod_scale[MAX_TEXTURES];
	bool tcmod_stretch_sin[MAX_TEXTURES];
	bool tcmod_stretch_square[MAX_TEXTURES];
	bool tcmod_stretch_triangle[MAX_TEXTURES];
	bool tcmod_stretch_sawtooth[MAX_TEXTURES];
	bool tcmod_stretch_inverse_sawtooth[MAX_TEXTURES];
	vec4 stretch_value[MAX_TEXTURES];
	float deg[MAX_TEXTURES];
	vec2 scroll[MAX_TEXTURES];
	vec2 scale[MAX_TEXTURES];
	vec2 scroll_value[MAX_TEXTURES]; // integrated with time
	bool cull_none;
	bool blend;
	bool blend_filter;
	bool blend_one_one;
	bool blend_one_zero;
	bool blend_zero_one;
	bool blend_dstcolor_one;
	bool blend_dstcolor_zero;
	bool blend_default;
	bool blend_dst_color_one_minus_dst_alpha;
	bool blend_dst_color_src_alpha;
	bool one_minus_src_alpha_src_alpha;
	bool blend_one_minus_src_alpha_src_alpha;
	bool blend_src_alpha_one_minus_src_alpha;
	bool blend_one_src_alpha;
	bool blend_one_minus_dst_color_zero;
	bool blend_zero_src_color;
	bool blend_dst_color_src_color;
	bool blend_zero_src_alpha;
	bool alpha_gt0;
	bool alpha_ge128;
	bool alpha_lt128;
	bool envmap;
	bool turb;
	bool portal;
	bool alpha;

	bool rgbgen_identity;
	bool rgbgen_wave_sin[MAX_TEXTURES];
	bool rgbgen_wave_square[MAX_TEXTURES];
	bool rgbgen_wave_triangle[MAX_TEXTURES];
	bool rgbgen_wave_sawtooth[MAX_TEXTURES];
	bool rgbgen_wave_inverse_sawtooth[MAX_TEXTURES];
	vec4 rgbgen_wave_value[MAX_TEXTURES];


	bool lightmap[MAX_TEXTURES];
	unsigned int stage;

	bool fog;
	vec3 fog_color;
	int fog_density;
} faceinfo_t;


typedef struct
{
	vertex_t		*vertex_array;
	int				*index_array;
	int				num_verts;
	int				num_indexes;

	int facevert;
	unsigned int vbo;
	unsigned int ibo;
	int num_mesh;
} patch_t;



typedef struct
{
	int	a;
	int	b;
	float	weight;
} graph_arc_t;

typedef struct
{
	graph_arc_t	arc[8];
	int	num_arcs;
} graph_node_t;

typedef struct
{
	int x;
	int y;
	int z;
} ref_t;


typedef struct
{
	// must have at least number of nodes (assume we take the longest path possible)
	int *path;
	int length;
	int step;
} path_t;

typedef struct
{
	vec3 position;
	char targetname[64];
	char target[512];
} navpoint_t;

//Uniform data
typedef struct
{
	vec3 position;
	vec3 vel_min;
	vec3 vel_range;
	vec3 gravity;
	vec3 seed;
	float size;
	float life_min;
	float life_range;
	float delta_time;
	int color;
	unsigned int num;
	int bsp_leaf;
	bool visible;
	bool enabled;
} emitter_t;

typedef struct
{
	float x;
	float y;
	int tex;
	char filename[128];
} icon_t;

typedef enum
{
	GAMETYPE_DEATHMATCH,
	GAMETYPE_TDM,
	GAMETYPE_CTF,
	GAMETYPE_DUEL
} gametype_t;


typedef enum
{
	TEAM_NONE,
	TEAM_BLUE,
	TEAM_RED,
} team_t;

typedef struct
{
	int drawcall;
	int triangle;
	int buffer_size;
	int texture_size;
} gpustat_t;


typedef struct
{
        int length;
        int pos;
} rletable_t;


// I should really just use binary for everything
typedef struct
{
	char header[6]; // <data>
	uint8_t warmup;
	uint32_t warmup_time;
	uint32_t round_time;
	uint8_t fraglimit;
	uint8_t timelimit;
	uint8_t red_flag_caps;
	uint8_t blue_flag_caps;
	char footer[8]; // </data>
} serverdata_t;



typedef enum
{
	MODE_FORWARD,
	MODE_INDIRECT
} rendermode_t;

typedef struct
{
	char magic[5];
	char map[64];
} demo_fileheader_t;

typedef struct
{
	char magic[6];
	int num_ents;
	int tick_num;
} demo_frameheader_t;

#pragma pack(1)
typedef struct
{
	char type[2];
	int file_size;
	int reserved;
	int offset;
} bmpheader_t;
#pragma pack(8)

#pragma pack(1)
typedef struct
{
	int size;
	int width;
	int height;
	short planes;
	short bpp;
	int compression;
	int image_size;
	int xres;
	int yres;
	int clr_used;
	int clr_important;
} dib_t;
#pragma pack(8)

#pragma pack(1)
typedef struct
{
	bmpheader_t	header;
	dib_t		dib;
} bitmap_t;
#pragma pack(8)

typedef enum
{
	ENT_UNKNOWN,
	ENT_VISIBLE_START,
	ENT_ITEM_ARMOR_COMBAT,
	ENT_ITEM_ARMOR_BODY,
	ENT_ITEM_ARMOR_SHARD,
	ENT_ITEM_HEALTH,
	ENT_ITEM_HEALTH_LARGE,
	ENT_ITEM_HEALTH_SMALL,
	ENT_ITEM_HEALTH_MEGA,
	ENT_ITEM_QUAD,
	ENT_ITEM_ENVIRO,
	ENT_ITEM_FLIGHT,
	ENT_ITEM_HASTE,
	ENT_ITEM_INVIS,
	ENT_ITEM_REGEN,
	ENT_TEAM_CTF_BLUEFLAG,
	ENT_TEAM_CTF_REDFLAG,
	ENT_START_AMMO,
	ENT_AMMO_BULLETS,
	ENT_AMMO_ROCKETS,
	ENT_AMMO_SLUGS,
	ENT_AMMO_SHELLS,
	ENT_AMMO_CELLS, //	ENT_AMMO_PLASMA
	ENT_AMMO_LIGHTNING,
	ENT_AMMO_BFG,
	ENT_AMMO_GRENADES,
	ENT_END_AMMO,
	ENT_START_WEAPON,
	ENT_WEAPON_ROCKETLAUNCHER,
	ENT_WEAPON_LIGHTNING,
	ENT_WEAPON_BFG,
	ENT_WEAPON_SHOTGUN,
	ENT_WEAPON_MACHINEGUN,
	ENT_WEAPON_RAILGUN,
	ENT_WEAPON_PLASMA,
	ENT_WEAPON_GRENADELAUNCHER,
	ENT_END_WEAPON,
	ENT_HOLDABLE_MEDKIT,
	ENT_HOLDABLE_TELEPORTER,
	ENT_FUNC_START,
	ENT_FUNC_BOBBING,
	ENT_FUNC_BUTTON,
	ENT_FUNC_DOOR,
	ENT_FUNC_PLAT,
	ENT_FUNC_PENDULUM,
	ENT_FUNC_ROTATING,
	ENT_FUNC_TRAIN,
	ENT_FUNC_STATIC,
	ENT_FUNC_END,
	ENT_MISC_MODEL,
	ENT_VISIBLE_END,
	ENT_SPECTATOR,
	ENT_PLAYER,
	ENT_NPC,
	ENT_SERVER,
	ENT_CLIENT,
	ENT_ITEM_BOT_ROAM,
	ENT_FUNC_TIMER, // not visible, keeping seperate
	ENT_INFO_START,
	ENT_INFO_PLAYER_INTERMISSION,
	ENT_INFO_PLAYER_START,
	ENT_INFO_PLAYER_DEATHMATCH,
	ENT_INFO_SPECTATOR_START,
	ENT_INFO_FIRSTPLACE,
	ENT_INFO_SECONDPLACE,
	ENT_INFO_THIRDPLACE,
	ENT_INFO_NOTNULL,
	ENT_INFO_NULL,
	ENT_INFO_CAMP,
	ENT_INFO_END,
	ENT_TEAM_CTF_BLUESPAWN,
	ENT_TEAM_CTF_REDSPAWN,
	ENT_TEAM_CTF_BLUEPLAYER,
	ENT_TEAM_CTF_REDPLAYER,
	ENT_PATH_CORNER,
	ENT_TARGET_TELEPORTER,
	ENT_TARGET_POSITION,
	ENT_TARGET_GIVE,
	ENT_TARGET_RELAY,
	ENT_TARGET_DELAY,
	ENT_TARGET_SPEAKER,
	ENT_TARGET_REMOVE_POWERUPS,
	ENT_TARGET_LOCATION,
	ENT_TRIGGER_HURT,
	ENT_TRIGGER_TELEPORT,
	ENT_TRIGGER_PUSH,
	ENT_TRIGGER_ALWAYS,
	ENT_TRIGGER_MULTIPLE,
	ENT_TRIGGER_CHANGELEVEL, // quake1
	ENT_MISC_PORTAL_SURFACE,
	ENT_MISC_PORTAL_CAMERA,
	ENT_MISC_TELEPORTER_DEST,
	ENT_LIGHT,
	ENT_WORLDSPAWN,
	ENT_SHOOTER_GRENADE,
	ENT_NAVPOINT,
	ENT_Q1_START,
	ENT_Q1_MONSTER_ARMY,
	ENT_Q1_MONSTER_DOG,
	ENT_Q1_MONSTER_OGRE,
	ENT_Q1_MONSTER_ZOMBIE,
	ENT_Q1_MONSTER_SHAMBLER,
	ENT_Q1_MONSTER_WIZARD,
	ENT_Q1_MONSTER_KNIGHT,
	ENT_Q1_MONSTER_DEMON1,
	ENT_Q1_MONSTER_ENFORCER,
	ENT_Q1_MONSTER_FISH,
	ENT_Q1_MONSTER_HELL_KNIGHT,
	ENT_Q1_MONSTER_SHALRATH,
	ENT_Q1_MONSTER_TARBABY,
	ENT_Q1_LIGHT_FLUORO,
	ENT_Q1_LIGHT_FLUOROSPARK,
	ENT_Q1_INFO_TELEPORT_DESTINATION,
	ENT_Q1_FUNC_DOOR_SECRET,
	ENT_Q1_INFO_INTERMISSION,
	ENT_Q1_INFO_PLAYER_COOP, 
	ENT_TRIGGER_ONCE,
	ENT_Q1_TRIGGER_COUNTER,
	ENT_Q1_TRIGGER_SECRET,
	ENT_Q1_ITEM_SPIKES,
	ENT_Q1_ITEM_ARTIFACT_SUPER_DAMAGE,
	ENT_Q1_ITEM_ARTIFACT_INVULNERABILITY,
	ENT_Q1_ITEM_ARTIFACT_ENVIROSUIT,
	ENT_Q1_WEAPON_NAILGUN,
	ENT_Q1_WEAPON_SUPERNAILGUN,
	ENT_Q1_WEAPON_SUPERSHOTGUN,
	ENT_Q1_MISC_EXPLOBOX,
	ENT_Q1_AMBIENT_COMP_HUM, 
	ENT_Q1_AMBIENT_DRONE,
	ENT_Q1_AMBIENT_DRIP,
	ENT_Q1_AMBIENT_SWAMP1,
	ENT_Q1_AMBIENT_SWAMP2,
	ENT_Q1_FUNC_WALL,
	ENT_Q1_LIGHT_TORCH_SMALL_WALLTORCH,
	ENT_Q1_LIGHT_FLAME_SMALL_YELLOW,
	ENT_Q1_LIGHT_FLAME_SMALL_WHITE,
	ENT_Q1_ITEM_KEY1,
	ENT_Q1_ITEM_KEY2,
	ENT_Q1_ITEM_ARMOR1,
	ENT_Q1_ITEM_ARMOR2,
	ENT_Q1_TRAP_SPIKESHOOTER,
	ENT_Q1_ITEM_SIGIL,
	ENT_Q1_END,
	ENT_CONSTRUCT,
	ENT_SENTRY,
	ENT_SENTRY_BASE,
	ENT_FUNC_TERRAIN // giant OBJ file with texture, need to figure out collision
} entity_type_t;

// Any dynamically created entity type that should be synced over network
// dropped items, muzzle flashes, shells, projectiles etc
typedef enum {
	NET_NONE,
	NET_BULLET_HIT,
	NET_PLASMA_HIT,
	NET_SHELL,
	NET_BULLET,
	NET_BOOM,
	NET_ROCKET,
	NET_ROCKET_LAUNCHER,
	NET_GRENADE,
	NET_GRENADE_LAUNCHER,
	NET_LIGHTNING,
	NET_LIGHTNINGGUN,
	NET_RAIL,
	NET_RAILGUN,
	NET_PLASMA,
	NET_PLASMAGUN,
	NET_SHOTGUN,
	NET_MACHINEGUN,
	NET_QUAD,
	NET_HASTE,
	NET_REGEN,
	NET_INVIS,
	NET_FLIGHT,
	NET_BLUE_FLAG,
	NET_RED_FLAG
} net_ent_t;

typedef enum
{
	NET_UNKNOWN,
	NET_RIGID,
	NET_PLAYER,
	NET_TRIGGER,
	NET_PROJECTILE
} net_type_t;

typedef struct
{
	unsigned short int ctype;		// class type [net_type_t]
	unsigned short int etype;		// net entity type [net_ent_t] need to combine with entity_type_t really
	unsigned short int index;		// index into entity_list
	unsigned short int data_size;	// size of data packet below not including header
	char data[1024];	// data size depends on ctype
} net_entity_t;
#define SIZE_NET_ENTITY_HEADER 8

typedef struct
{
//	matrix3			morientation;
	vec3			forward;
	vec3			right;
	vec3			velocity;
	vec3			angular_velocity;
	vec3			position;
} net_rigid_t;

typedef struct
{
	uint8_t			active; // whether trigger is active
} net_trigger_t;

typedef struct
{
	uint8_t			active;
	short int		owner;
	vec3			forward;
	vec3			right;
//	vec3			quat;
	vec3			velocity;
	vec3			angular_velocity;
	vec3			position;
} net_projectile_t;



typedef struct
{
	vec3			forward;
	vec3			right;
//	vec3			quat;
	vec3			velocity;
	vec3			angular_velocity;
	vec3			position;
	vec3			center;

	short int		health;
	unsigned short int		armor;
	unsigned short int		current_weapon;
	unsigned short int		ammo_bullets;
	unsigned short int		ammo_shells;
	unsigned short int		ammo_rockets;
	unsigned short int		ammo_lightning;
	unsigned short int		ammo_slugs;
	unsigned short int		ammo_plasma;
	unsigned short int flight_timer;
	unsigned short int regen_timer;
	unsigned short int haste_timer;
	unsigned short int quad_timer;
	uint8_t			weapon_flags;
	uint8_t 		holdable_flags;
	char			name[32];
} net_player_t;


typedef struct
{
	int				index;
	int				count;
	vec3			*target;
	vec3			*next;
	int				loop;

	vec3 path_list[8];
	int num_path;
} func_path_t;

typedef struct
{
	vec3 dState;		// Last position input
	vec3 iState;		// Integrator state
	float iMin, iMax;	// min max integrator state

	float iGain;
	float pGain;
	float dGain;
} pid_state_t;

typedef struct
{
	bool fullscreen;
	int vsync;
	char resolution[64];
	char window[64];
	char monitor[128];
	float rscale;
	float fov;
	float sensitivity;
	int antialias;
	int anisotropic;
	int crosshair;
	int mousemode;

	char apply[64];

	char attack[64];
	char jump[64];
	char duck[64];
	char zoom[64];
	char use[64];
	char weapnext[64];
	char weapprev[64];
	char moveforward[64];
	char moveback[64];
	char moveleft[64];
	char moveright[64];
	char walk[64];
	char spectate[64];
	char console[64];
	char menu[64];
	char scores[64];

	char name[128];
	char model[128];
	char skin[128];
	char team[128];

	float volume;
	float musicvol;
	float sfxvol;
	float brightness;
	float contrast;

	bool ssao;
	bool bloom;
	bool shadowmaps;
	bool shadowvol;
	bool skyray;
	bool portal;
	bool invert;
} menudata_t;

typedef struct
{
	bool lava;
	bool slime;
	bool water;
	bool fog;
	int contents;
	int surf_flags;
} content_flag_t;

typedef struct
{
	int rigid0;
	int rigid1;
	unsigned int vertex0;
	unsigned int vertex1;
} body_spring_t;

typedef struct
{
	int rigid0;
	unsigned int vertex0;
	vec3 anchor;
} spring_t;

typedef struct
{
	float size;
	vec3 offset;
	int num_col;
	int num_row;
	float trilength;
} terrain_t;


typedef struct
{
	//left
	bool a;
	bool b;
	bool rthumb;
	bool rthumb_rest;
	bool rindex_trigger;
	bool rindex_pointing;
	bool rthumb_up;
	float rtrigger;
	vec2 rthumbstick;

	//right
	bool x;
	bool y;
	bool lthumb;
	bool lthumb_rest;
	bool lindex_trigger;
	bool lindex_pointing;
	bool lthumb_up;
	float ltrigger;
	vec2 lthumbstick;
} ovrtouch_t;


typedef enum {
	MASTER_REPORT,
	MASTER_LIST,
	MASTER_RESPONSE
} master_cmd_t;

typedef struct
{
	int cmd;
	char ip[32];
	int qport;
	char sv_hostname[32];
	char map[32];
	int num_player;
	int max_player;
	int gametype;
	int fraglimit;
	int timelimit;
	int capturelimit;
} report_t;

typedef struct
{
	int				angle;
	vec3			model_offset;	// bsp model offset
	float			model_lerp;		// bsp lerp between 0.0-1.0
	bool			opening;		// flag for door opening
	float			func_height;			// How far a func_ item will move in map units, defaults to half width
	vec3			origin; // used on movers to store original position when spawned
} brushinfo_t;


typedef struct
{
	char			type[64];
	char			target[64];
	char			target_name[64];
} ent_string_t;

#define WEAPON_NONE				0
#define WEAPON_GAUNTLET			1
#define WEAPON_MACHINEGUN		2
#define WEAPON_SHOTGUN			4
#define WEAPON_GRENADE			8
#define WEAPON_ROCKET			16
#define WEAPON_RAILGUN			32
#define WEAPON_LIGHTNING		64
#define WEAPON_PLASMA			128

//multiple types exist because I forgot above code existed D:
typedef enum {
	wp_none = WEAPON_NONE,
	wp_gauntlet = WEAPON_GAUNTLET,
	wp_machinegun = WEAPON_MACHINEGUN,
	wp_shotgun = WEAPON_SHOTGUN,
	wp_grenade = WEAPON_GRENADE,
	wp_rocket = WEAPON_ROCKET,
	wp_railgun = WEAPON_RAILGUN,
	wp_lightning = WEAPON_LIGHTNING,
	wp_plasma = WEAPON_PLASMA
} weapon_t;

typedef enum {
	BOT_IDLE,
	BOT_ALERT,		// 1
	BOT_ATTACK,		// 2
	BOT_GET_ITEM,	// 3
	BOT_EXPLORE,	// 4
	BOT_DEAD		// 5
} bot_state_t;




// Seems like a good idea to handle animations
typedef enum {
	PLAYER_IDLE,
	PLAYER_MOVED,
	PLAYER_JUMPED,
	PLAYER_DUCKED,
	PLAYER_ATTACK,
	PLAYER_DEAD
} player_state_t;


typedef struct
{
	int kills;
	int deaths;
	int hits;
	int shots;
	int medal_impressive;
	int medal_excellent;
	int medal_frags;
	int medal_humiliation;
} stats_t;


typedef enum
{
	UNKNOWN,
	PLAYER,
	BOT,
	CLIENT,
	SERVER,
	SPECTATOR
} playertype_t;

typedef struct
{
	int *data[32];
	int width[32];
	int height[32];
	int components;
	int num_mip;
} texinfo_t;


typedef struct
{
	unsigned char r, g, b;
} rgb_t;

#pragma pack(1)
typedef struct
{
	unsigned char r, g, b, a;
} rgba_t;
#pragma pack(8)

typedef struct
{
	plane_t left;
	plane_t right;
	plane_t bottom;
	plane_t top;
	plane_t zNear;
	plane_t zFar;

	vec3 pos;
	vec3 forward;
}  frustum_t;


typedef enum
{
	CT_AUTOSENTRY = 1,
	CT_STRUCTURE
} construct_type_t;

#endif
