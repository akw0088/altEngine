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

/*
	Holds data sent across network to represent entities
*/
typedef struct
{
	unsigned int		id;
	matrix3 morientation;
	vec3	velocity;
	vec3	angular_velocity;
	vec3	position;
} entity_t;

typedef struct
{
	char			socketname[80];
	unsigned int	entity;
	unsigned int	client_sequence;
	unsigned int	server_sequence;
	unsigned int	last_time;
} client_t;

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
	unsigned int	length;
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
	unsigned int	length;
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

// formally keyboard class
typedef struct
{
	bool leftbutton;
	bool middlebutton;
	bool rightbutton;
	bool enter;
	bool escape;
	bool shift;
	bool control;
	bool up;
	bool left;
	bool down;
	bool right;

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

	bool mousewheelup;
	bool mousewheeldown;
}  button_t;


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
	char anim_map_tex[512]; // frequency, tex1-tex8
	float anim_map_freq;

	bool blendfunc_add;
	bool blendfunc_filter;
	bool blendfunc_blend;//source * srccoef + dest * destcoef
						 /*
						 bool blend_one;
						 bool blend_zero;
						 bool blend_dst_color;
						 bool blend_one_minus_dst_color;
						 bool blend_one_minus_src_alpha;
						 */
	bool blend_zero_one;
	bool blend_one_zero;
	bool blend_one_one;
	bool blend_one_srccolor;
	bool blend_dstcolor_one;
	bool blend_dstcolor_zero;
	bool alpha_func_gt0;
	bool depth_write;
	bool depthfunc_equal;
	bool rgbgen_identity;
	bool tcmod_rotate;
	float tcmod_rotate_value; // deg/sec
	bool tcmod_scale;
	vec2 tcmod_scale_value;
	bool tcmod_stretch;
	vec4 tcmod_stretch_value;
	bool tcmod_scroll;
	vec4 tcmod_scroll_value;
	bool tcmod_turb;
	vec2 tcmod_turb_value;
	bool tcgen_env;

} stage_t;


// Keeping size small, because there are a lot of these
struct surface_t
{
	char file[128];
	char name[128];
	//	char *cmd[64];	//raw parser output
	stage_t stage[64];
	unsigned int num_cmd;
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
	int texObj;
	int index; // index into surface list (if it exists)
	int stage; // stage texture was loaded from
} texture_t;