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
	char			*socketname;
	unsigned int	entity;
	unsigned int	client_sequence;
	unsigned int	server_sequence;
} client_t;

// size of fixed part of network packets
// length + sequence + ack + num_cmds
#define HEADER_SIZE 13 

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
	byte			num_cmds;
	char			data[16834];
} clientmsg_t;

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
