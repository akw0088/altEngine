typedef unsigned int uint;

#ifdef __linux__
typedef char byte;
#endif 

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
} triangle_t;


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
	int		id;
	matrix3 morientation;
	vec3	velocity;
	vec3	angular_velocity;
	vec3	position;
} entity_t;

typedef struct
{
	char	*socketname;
	int		entity;
	int		client_sequence;
	int		server_sequence;
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
	char			data[16834];
} servermsg_t;

typedef struct
{
	unsigned int	sequence;
	char			msg[16834];
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
}  button_t;
