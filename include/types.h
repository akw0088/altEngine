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
	waveFormat_t	*format;
	void			*pcmData;
	int				dataSize;
	int				duration;
	char			*data;
	char			file[LINE_SIZE];
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
	NT_ROCKET_FLASH,
	NT_ROCKET_LAUNCHER,
	NT_GRENADE,
	NT_GRENADE_FLASH,
	NT_GRENADE_LAUNCHER,
	NT_LIGHTNING,
	NT_LIGHTNING_FLASH,
	NT_LIGHTNINGGUN,
	NT_RAIL,
	NT_RAIL_FLASH,
	NT_RAILGUN,
	NT_PLASMA,
	NT_PLASMA_FLASH,
	NT_PLASMAGUN,
	NT_SHOTGUN,
	NT_SHOTGUN_FLASH,
	NT_MACHINEGUN,
	NT_MACHINEGUN_FLASH,
	NT_GIB0,
	NT_GIB1,
	NT_GIB2,
	NT_GIB3,
	NT_GIB4,
	NT_GIB5,
	NT_GIB6,
	NT_GIB7,
	NT_GIB8,
	NT_GIB9,
	NT_BULLET,
	NT_SHELL,
	NT_QUAD,
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
	uint8_t			health;
	uint8_t			armor;
	uint8_t			weapon_flags;
	uint8_t			current_weapon;
	uint8_t			ammo_bullets;
	uint8_t			ammo_shells;
	uint8_t			ammo_rockets;
	uint8_t			ammo_lightning;
	uint8_t			ammo_slugs;
	uint8_t			ammo_plasma;

} entity_t;

// size of fixed part of network packets
// length + sequence + ack + num_cmds
#define CLIENT_HEADER 37

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
	int	length;
	unsigned int	sequence;
	unsigned int	server_sequence;
	float			up[3];
	float			forward[3];
	byte			num_cmds;
	char			data[16834];
} clientmsg_t;


#define SERVER_HEADER 13
/*
	Variable length server msg
	after num_ents is a variable number of entity states
	after entity state list is a null terminated reliably
	transmitted msg string that is retransmitted until acked.
*/
typedef struct
{
	int		length;
	unsigned int	sequence;
	unsigned int	client_sequence;
	byte			num_ents;
	char			data[10 * 16834];
} servermsg_t;

typedef struct
{
	unsigned int	sequence;
	char			msg[10 * 16834];
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

	bool numpad0;
	bool numpad1;
	bool numpad2;
	bool numpad3;
	bool numpad4;
	bool numpad5;
	bool numpad6;
	bool numpad7;
	bool numpad8;
	bool numpad9;

	bool weapon_up;
	bool weapon_down;

	bool scores;
} input_t;

typedef struct
{
	char		socketname[80];
	unsigned int	entity;
	unsigned int	client_sequence;
	unsigned int	server_sequence;
	unsigned int	last_time;
	input_t		input;
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
