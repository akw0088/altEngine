#include "include.h"

#ifndef MD5_H
#define MD5_H

#include <stack>

class MD5
{
public:
	MD5();
	~MD5();

	int load_md5(char *file);
	int load_md5_animation(char *file, anim_list_t *plist);

	void PrepareMesh(int mesh_index, md5_joint_t *skeleton, int &num_index, int *index_array, vertex_t *vertex_array, int &num_vertex);

	void generate_animation(md5_joint_t **&frame, md5_anim_t *anim);
	void destroy_animation(md5_joint_t **&frame, md5_anim_t *anim);
	void generate_tangent(int *index_array, int num_index, vertex_t *vertex_array, int num_vertex);

private:
	void InterpolateSkeletons(const md5_joint_t *skelA, const md5_joint_t *skelB, int num_joints, float interp, md5_joint_t *out);

	void calc_tangent(vertex_t &a, vertex_t &b, vertex_t &c);
	void build_frame(md5_joint_t *joint, float *frame, md5_anim_t *anim);


	int parse_joint(char *data, md5_joint_t *joint, int num_joint);
	int parse_mesh(char *data, md5_mesh_t *mesh);
	int parse_hierarchy(char *data, int num_joint, md5_hierarchy_t *hierarchy);
	int parse_bounds(char *data, int num_bound, md5_aabb_t *aabb);
	int parse_base(char *data, int num_base, md5_base_t *base);
	int parse_frame(char *data, int num_frame, int num_ani, float *frame);


	bool			loaded;
	stack<anim_list_t *> plist_stack;
public:
	//TODO: dynamically allocate these
	vertex_t	vertex_array[32][8196];
	int			index_array[32][8196];
	int			num_index[32];
	int			num_vertex[32];
	md5_model_t *model;
};

#endif
