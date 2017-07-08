#ifndef TYPES_H
#define TYPES_H
typedef unsigned int uint;


#ifndef WIN32
typedef char byte;
#endif 

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

typedef struct {
	vec3 a;
	vec3 b;
	vec3 c;
	vec3 lightdir1;
	vec3 lightdir2;
	vec3 lightdir3;
} shadowvol_t;


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
	char msg[LINE_SIZE];
} menu_t;

/*
	Holds state transition information for menu definition files
*/
typedef struct
{
	int start;
	char delta[LINE_SIZE];
	int end;
	char cmd[LINE_SIZE];
} state_t;

// Any dynamically created entity type that should be synced over network
// dropped items, muzzle flashes, shells, projectiles etc
typedef enum {
	NT_NONE,
	NT_ROCKET,
	NT_ROCKET_LAUNCHER,
	NT_GRENADE,
	NT_GRENADE_LAUNCHER,
	NT_LIGHTNING,
	NT_LIGHTNINGGUN,
	NT_RAIL,
	NT_RAILGUN,
	NT_PLASMA,
	NT_PLASMAGUN,
	NT_SHOTGUN,
	NT_MACHINEGUN,
	NT_QUAD,
	NT_HASTE,
	NT_REGEN,
	NT_INVIS,
	NT_FLIGHT,
	NT_BLUE_FLAG,
	NT_RED_FLAG
} nettype_t;

/*
	Holds data sent across network to represent entities
*/
typedef struct
{
	unsigned int	id;
	nettype_t		type;
	matrix3			morientation;
	vec3			velocity;
	vec3			angular_velocity;
	vec3			position;
	uint8_t			active; // whether trigger is active
	short int		owner;
	//player info
	char			name[64];
	short int		health;
	short int		armor;
	uint8_t			weapon_flags;
	short int		current_weapon;
	short int		ammo_bullets;
	short int		ammo_shells;
	short int		ammo_rockets;
	short int		ammo_lightning;
	short int		ammo_slugs;
	short int		ammo_plasma;
	uint8_t 		holdable_teleporter;
	uint8_t			holdable_medikit;
	uint8_t			holdable_flag;
	unsigned int flight_timer;
	unsigned int regen_timer;
	unsigned int haste_timer;
	unsigned int quad_timer;
} entity_t;

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
	float			pos[3];		// Sending position (not used, but interesting to calculate delta's)
	byte			num_cmds;
	unsigned char		data[16834];
} clientmsg_t;


#define SERVER_HEADER 10
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
	unsigned short int	num_ents;
	unsigned char		data[256000];
} servermsg_t;
#pragma pack(8)

typedef struct
{
	unsigned short int	sequence;
	unsigned short int	size; // size of entire header + data
	char			msg[LINE_SIZE];
}
reliablemsg_t;

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

	bool map;
	char map_tex[128];
	bool clampmap;
	char clampmap_tex[128];
	bool anim_map;
	char anim_map_tex[512];
	float anim_map_freq;
	bool lightmap;

	bool blendfunc_add;
	bool blendfunc_filter;
	bool blendfunc_blend;	//source * srccoef + dest * destcoef
	bool blend_zero_one;
	bool blend_one_zero;
	bool blend_one_one;
	bool blend_one_src_color;
	bool blend_dst_color_one;
	bool blend_dst_color_zero;
	bool blend_dst_color_one_minus_dst_alpha;
	bool one_minus_src_alpha_src_alpha;
	bool blend_dst_color_src_alpha;
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
	bool depth_write;
	bool depthfunc_equal;
	bool rgbgen_identity;
	bool tcmod_rotate;
	float tcmod_rotate_value; // deg/sec
	bool tcmod_scale;
	vec2 tcmod_scale_value;
	bool tcmod_stretch_sin;
	bool tcmod_stretch_sawtooth;
	bool tcmod_stretch_inverse_sawtooth;
	bool tcmod_stretch_square;
	bool tcmod_stretch_triangle;
	vec4 tcmod_stretch_value;
	bool tcmod_scroll;
	vec2 tcmod_scroll_value;
	bool tcmod_turb;
	vec4 tcmod_turb_value;
	bool tcgen_env;

	bool rgbgen_wave_sin;
	bool rgbgen_wave_sawtooth;
	bool rgbgen_wave_inverse_sawtooth;
	bool rgbgen_wave_square;
	bool rgbgen_wave_triangle;
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

	bool nomipmaps;
	bool nopicmip;
	bool polygon_offset;
	bool portal;
	//sort value 1:portal,2:sky,3:opaque,6:banner,8:underwater,9:additive,16:nearest
	bool surfaceparm_trans;
	bool surfaceparm_nonsolid;
	bool surfaceparm_noclip;
	bool surfaceparm_nodraw;
	bool surfaceparm_nodrop;
	bool surfaceparm_nodlight;
	bool surfaceparm_areaportal;
	bool surfaceparm_clusterportal;
	bool surfaceparm_donotenter;
	bool surfaceparm_origin;
	bool surfaceparm_detail;
	bool surfaceparm_playerclip;
	bool surfaceparm_water;
	bool surfaceparm_slime;
	bool surfaceparm_lava;
	bool surfaceparm_slick;
	bool surfaceparm_structural;
	bool surfaceparm_fog;
	bool surfaceparm_sky;
	bool surfaceparm_nolightmap;
	bool surfaceparm_nodamage;
	bool surfaceparm_noimpact;
	bool surfaceparm_nomarks;
	bool surfaceparm_metalsteps;
	bool surfaceparm_alphashadow;
	bool q3map_surfacelight;
	int q3map_surfacelight_value;
	bool q3map_sun;
	vec3 q3map_sun_value[2]; //rgb + intensity degrees elevation
	bool cull_disable;
	bool cull_none;
	bool cull_backside;
	bool cull_twosided;
	bool deformVertexes;
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

	bool rgbgen_wave_sin[MAX_TEXTURES];
	bool rgbgen_wave_square[MAX_TEXTURES];
	bool rgbgen_wave_triangle[MAX_TEXTURES];
	bool rgbgen_wave_sawtooth[MAX_TEXTURES];
	bool rgbgen_wave_inverse_sawtooth[MAX_TEXTURES];
	vec4 rgbgen_wave_value[MAX_TEXTURES];


	bool lightmap[MAX_TEXTURES];
	unsigned int stage;
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
} gpustat_t;


typedef struct
{
        int length;
        int pos;
} rletable_t;

typedef struct
{
	char header[4]; // data
	uint8_t warmup;
	uint32_t warmup_time;
	uint32_t round_time;
	uint8_t fraglimit;
	uint8_t timelimit;
	uint8_t red_flag_caps;
	uint8_t blue_flag_caps;
} serverdata_t;



typedef enum
{
	MODE_FORWARD,
	MODE_INDIRECT,
	MODE_SHADOWVOL
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
	ENT_ITEM_BOT_ROAM,
	ENT_TEAM_CTF_BLUEFLAG,
	ENT_TEAM_CTF_REDFLAG,
	ENT_START_AMMO,
	ENT_AMMO_BULLETS,
	ENT_AMMO_ROCKETS,
	ENT_AMMO_SLUGS,
	ENT_AMMO_SHELLS,
	ENT_AMMO_CELLS,
	ENT_AMMO_LIGHTNING,
	ENT_AMMO_BFG,
	ENT_AMMO_GRENADES,
	ENT_AMMO_PLASMA,
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
	ENT_INFO_PLAYER_INTERMISSION,
	ENT_INFO_PLAYER_START,
	ENT_INFO_PLAYER_DEATHMATCH,
	ENT_TEAM_CTF_BLUESPAWN,
	ENT_TEAM_CTF_REDSPAWN,
	ENT_TEAM_CTF_BLUEPLAYER,
	ENT_TEAM_CTF_REDPLAYER,
	ENT_INFO_SPECTATOR_START,
	ENT_INFO_FIRSTPLACE,
	ENT_INFO_SECONDPLACE,
	ENT_INFO_THIRDPLACE,
	ENT_INFO_NOTNULL,
	ENT_INFO_NULL,
	ENT_INFO_CAMP,
	ENT_FUNC_START,
	ENT_FUNC_BOBBING,
	ENT_FUNC_BUTTON,
	ENT_FUNC_DOOR,
	ENT_FUNC_PLAT,
	ENT_FUNC_PENDULUM,
	ENT_FUNC_ROTATING,
	ENT_FUNC_TRAIN,
	ENT_FUNC_STATIC,
	ENT_FUNC_TIMER,
	ENT_FUNC_END,
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
	ENT_MISC_MODEL,
	ENT_MISC_PORTAL_SURFACE,
	ENT_MISC_PORTAL_CAMERA,
	ENT_MISC_TELEPORTER_DEST,
	ENT_LIGHT,
	ENT_WORLDSPAWN,
	ENT_SHOOTER_GRENADE,
	ENT_NAVPOINT,
	ENT_PLAYER,
	ENT_SPECTATOR,
	ENT_NPC,
	ENT_SERVER,
	ENT_CLIENT
}entity_type;

#endif
