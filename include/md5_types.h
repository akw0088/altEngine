struct md5_joint_t
{
	char		name[256];
	int		parent;

	vec3		pos;
	quaternion	orient;
};

struct md5_vertex_t
{
	float u;
	float v;

	int start;
	int count;
};

struct md5_triangle_t
{
	int a;
	int b;
	int c;
};

struct md5_weight_t
{
	int	joint;
	float	bias;

	vec3	pos;
};

typedef struct
{
	vec3 min;
	vec3 max;
} md5_aabb_t;


struct md5_mesh_t
{
	md5_vertex_t	*vertex;
	md5_triangle_t	*triangle;
	md5_weight_t	*weight;

	int num_vertex;
	int num_tri;
	int num_weight;

	char shader[256];
};

struct md5_model_t
{
	char name[128];
	struct md5_joint_t *joint;
	struct md5_mesh_t *mesh;

	int num_joint;
	int num_mesh;
};

typedef struct
{
	vec3 pos;
	quaternion orient;
} md5_base_t;


typedef struct
{
	char name[64];
	int parent;
	int flag;
	int start;
} md5_hierarchy_t;

struct md5_anim_t
{
	int num_frame;
	int num_joint;
	int frame_rate;
	int num_ani;

	md5_base_t *base;
	md5_aabb_t *aabb;
	md5_hierarchy_t *hierarchy;
	float *frame;
};
