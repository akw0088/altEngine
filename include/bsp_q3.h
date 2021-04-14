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

#ifndef Q3BSP_H
#define Q3BSP_H



class mLight2;

class Bsp
{
	void change_axis();
	inline int find_leaf(const vec3 &position);
	void sort_leaf(vector<int> *leaf_list, int node_index, const vec3 &position, q3::leaf_t *frameLeaf, bool order, int depth);
	inline int cluster_visible(int visCluster, int testCluster);
	void tessellate(int level, q3::bspvertex_t control[], vertex_t **vertex_array, int &numVerts,
		int **index_array, int &numIndexes, vec2 &texcoord, vec2 &lightcoord, vec2 &size);
	inline void render_face(q3::face_t *face, Graphics &gfx, int stage, bool lightmap, bool shader);
	inline void render_flag(q3::face_t *face, Graphics &gfx, int stage, bool lightmap, bool is_shader);
	inline void render_patch(q3::face_t *face, Graphics &gfx, int stage, bool lightmap, bool shader);
	inline void render_billboard(q3::face_t *face, Graphics &gfx, int stage, bool lightmap, bool shader);
	void gen_renderlists(int leaf, vector<surface_t *> &surface_list, vec3 &position, frustum_t *frustum, int tick_num);
	void add_list(vector<surface_t *> &surface_list, bool blend_flag, int i);
	void set_blend_mode(Graphics &gfx, faceinfo_t &face);
	void set_tcmod(mLight2 &mlight2, faceinfo_t &face, int tick_num, float time);
	//void draw_line_box(int *min, int *max);
	//void draw_box(int *min, int *max);
public:
	Bsp();
	void CreateShadowVolumes(Graphics &gfx, vec3 &light_pos, int current_light, vertex_t *shadow_vertex, unsigned int *shadow_index, int &num_index);
	void hitscan(vec3 &origin, vec3 &dir, float &distance);
	bool is_point_in_brush(int brush_index, vec3 &point, vec3 &oldpoint, float *depth, plane_t *plane, content_flag_t &flag, float &water_depth, bool debug);
	bool vis_test(vec3 &x, vec3 &y, int &leaf_a, int &leaf_b);
	bool leaf_test(vec3 &x, vec3 &y);
	void generate_meshes(Graphics &gfx);
	const char *get_entities();

	void render(vec3 &position, Graphics &gfx, vector<surface_t *> &surface_list, mLight2 &mlight2, int tick_num, frustum_t *frustum);
	void render_brush_entity(unsigned int index, Graphics &gfx);
	void render_sky(Graphics &gfx, mLight2 &mlight2, int tick_num, vector<surface_t *> &surface_list);
	vec3 model_origin(unsigned int index);

	bool load(char *map, char **pk3list, int num_pk3);
	void load_textures(Graphics &gfx, vector<surface_t *> &surface_list, char **pk3_list, int num_pk3, int anisotropic);
	void load_from_shader(char *name, vector<surface_t *> &surface_list, texture_t *texObj, Graphics &gfx, char **pk3_list, int num_pk3, int anisotropic);
	void load_from_file(char *filename, texture_t &texObj, Graphics &gfx, char **pk3_list, int num_pk3, int anisotropic);


	void unload(Graphics &gfx);
	void CalculateTangentArray(q3::bspvertex_t *vertex, int num_vert, int *index, int num_index, vec4 *tangent);
	void CreateTangentArray(vertex_t *vertex, q3::bspvertex_t *bsp_vertex, int num_vert, vec4 *tangent);

	bool collision_detect(vec3 &point, vec3 &oldpoint, plane_t *plane, float *depth, float &water_depth,
		vector<surface_t *> &surface_list, bool debug, vec3 &clip, const vec3 &velocity, int &model_trigger, int &model_platform, content_flag_t &flag, vec3 *tri);


	vec3 trace(vec3 &start, vec3 &end, vec3 &normal);
	void check_node(int node_index, float start_amount, float end_amount, vec3 &start, vec3 &end);
	void check_brush(q3::brush_t *brush, vec3 &start, vec3 &end);

	bool on_ground;
	bool collision;
	vec3 collision_normal;
	float trace_result;
	int max_depth;

	bool update;


	bool loaded;
	bool quake1;
	q3::bspData_t	data;
	char map_name[64];
	vector<texture_t *> anim_list;
	texture_t	*tex_object;

	unsigned int skybox_index;
	unsigned int skybox_vertex;
	unsigned int quad_index;
	unsigned int quad_vertex;
	int skybox_ibo;
	int skybox_vbo;


	vector<faceinfo_t> face_list;	// sorted front to back
	vector<faceinfo_t> blend_list; // sorted back to front
	vector<int> leaf_list;
	vector<int> frustum_list;
	int *face_to_patch;
	int lastIndex;

	int model_type[128];
	vec3 model_offset[128];
	vec3 model_rot[128];
	vec3 model_vel[128];

	unsigned int portal_tex; // hack





	bool enable_fog;
	bool enable_textures;
	bool enable_shader;
	bool enable_blend;
	bool enable_patch;
	bool enable_sky;
	bool enable_normalmap;
	bool bezier_collision;
	unsigned int max_stage;
private:
	bool bezier_collision_detect(vec3 &point, plane_t *plane, float *depth, vec3 *tri);
	void make_aabb(vertex_t *model_vertex_array, int num_vertex, vec3 *aabb, vec3 &center);

	q3::bsp_t		*tBsp;
	vec4		*tangent;

	bool selected_map;

//	int last_leaf;

	// keeping this around because it seems like data.vertex is getting corrupted.
	vertex_t *map_vertex;

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
