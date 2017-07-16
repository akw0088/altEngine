#include "include.h"

#ifndef BSP_H
#define BSP_H

class mLight2;

class Bsp
{
	void change_axis();
	inline int find_leaf(const vec3 &position);
	void sort_leaf(vector<int> *leaf_list, int node_index, const vec3 &position, leaf_t *frameLeaf, bool order);
	inline int cluster_visible(int visCluster, int testCluster);
	void tessellate(int level, bspvertex_t control[], vertex_t **vertex_array, int &numVerts,
		int **index_array, int &numIndexes, vec2 &texcoord, vec2 &lightcoord, vec2 &size);
	inline void render_face(face_t *face, Graphics &gfx, int stage, bool lightmap);
	inline void render_patch(face_t *face, Graphics &gfx, int stage, bool lightmap);
	inline void render_billboard(face_t *face, Graphics &gfx, int stage, bool lightmap);
	void gen_renderlists(int leaf, vector<surface_t *> &surface_list, vec3 &position);
	void add_list(vector<surface_t *> &surface_list, bool blend_flag, int i);
	void set_blend_mode(Graphics &gfx, faceinfo_t &face);
	void set_tcmod(mLight2 &mlight2, faceinfo_t &face, int tick_num, float time);
	//void draw_line_box(int *min, int *max);
	//void draw_box(int *min, int *max);
public:
	void render_sky(Graphics &gfx, mLight2 &mlight2, int tick_num, vector<surface_t *> surface_list);
	void find_edges(vec3 &position, Edge &edge_list);
	void hitscan(vec3 &origin, vec3 &dir, float &distance);
	Bsp();
	bool vis_test(vec3 &x, vec3 &y, int &leaf_a, int &leaf_b);
	bool leaf_test(vec3 &x, vec3 &y);
	void generate_meshes(Graphics &gfx);
	const char *get_entities();
	void render(vec3 &position, matrix4 &mvp, Graphics &gfx, vector<surface_t *> &surface_list, mLight2 &mlight2, int tick_num);
	void render_model(unsigned int index, Graphics &gfx);
	vec3 model_origin(unsigned int index);

	bool load(char *map, char **pk3list, int num_pk3);
	void load_textures(Graphics &gfx, vector<surface_t *> &surface_list, char **pk3_list, int num_pk3);
	void load_from_shader(char *name, vector<surface_t *> &surface_list, texture_t *texObj, Graphics &gfx, char **pk3_list, int num_pk3);
	void load_from_file(char *filename, texture_t &texObj, Graphics &gfx, char **pk3_list, int num_pk3);


	void unload(Graphics &gfx);
	void CalculateTangentArray(bspvertex_t *vertex, int num_vert, int *index, int num_index, vec4 *tangent);
	void CreateTangentArray(vertex_t *vertex, bspvertex_t *bsp_vertex, int num_vert, vec4 *tangent);

	bool collision_detect(vec3 &point, vec3 &oldpoint, plane_t *plane, float *depth, bool &water, float &water_depth,
		vector<surface_t *> &surface_list, bool debug, vec3 &clip, const vec3 &velocity, bool &lava, bool &slime, int &model_trigger);


	vec3 trace(vec3 &start, vec3 &end, vec3 &normal);
	void check_node(int node_index, float start_amount, float end_amount, vec3 &start, vec3 &end);
	void check_brush(brush_t *brush, vec3 &start, vec3 &end);

	bool on_ground;
	bool collision;
	vec3 collision_normal;
	float trace_result;


	bool loaded;
	bspData_t	data;
	char map_name[64];
	vector<texture_t *> anim_list;
	texture_t	*tex_object;

	vector<faceinfo_t> face_list;	// sorted front to back
	vector<faceinfo_t> blend_list; // sorted back to front
	vector<int> leaf_list;
	int *face_to_patch;
	int lastIndex;


	vec3 model_offset[128];

	unsigned int portal_tex; // hack

	bool enable_textures;
	bool enable_shader;
	bool enable_blend;
	bool enable_patch;
	bool enable_sky;
private:
	bsp_t		*tBsp;
	vec4		*tangent;

	bool selected_map;

//	int last_leaf;

	// keeping this around because it seems like data.vertex is getting corrupted.
	vertex_t *vertex;

	//for bezier patches
	patch_t *patchdata;
	int				mesh_level;
	int				num_meshes;

	//gpu objects
	unsigned int	map_index_vbo;
	unsigned int	map_vertex_vbo;

	int	*normal_object;
	int	*lightmap_object;
	int	sky_face;
};

#endif
