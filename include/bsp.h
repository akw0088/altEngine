#include "include.h"

#ifndef BSP_H
#define BSP_H

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
	char *stage; //raw parser output

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
struct Surface
{
	char file[128];
	char name[128];
	char *cmd[64];	//raw parser output
	stage_t stage[64];
	int num_cmd;
	int num_stage;

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

class Bsp
{
	void change_axis();
	inline int find_leaf(const vec3 &position);
	void sort_leaf(vector<int> *leaf_list, int node_index, const vec3 &position);
	inline int cluster_visible(int visCluster, int testCluster);
	void tessellate(int level, bspvertex_t control[], vertex_t **vertex_array, int &numVerts, int **index_array, int &numIndexes);
	inline void render_face(face_t *face, Graphics &gfx);
	inline void render_patch(face_t *face, Graphics &gfx);
	inline void render_billboard(face_t *face, Graphics &gfx);
	void draw_line_box(int *min, int *max);
	void draw_box(int *min, int *max);
public:
	void find_edges(vec3 &position, Edge &edge_list);
	void hitscan(vec3 &origin, vec3 &dir, float &distance);
	Bsp();
	bool vis_test(vec3 &x, vec3 &y);
	bool leaf_test(vec3 &x, vec3 &y);
	void generate_meshes(Graphics &gfx);
	const char *get_entities();
	void render(vec3 &position, matrix4 &mvp, Graphics &gfx);
	void render_model(unsigned int index, Graphics &gfx);
	vec3 model_origin(unsigned int index);
//	bool load(char *map);
	bool load(char *map, char **pk3list, int num_pk3);
	void load_textures(Graphics &gfx, vector<Surface *> &surface_list);
	void unload(Graphics &gfx);
	void CalculateTangentArray(bspvertex_t *vertex, int num_vert, int *index, int num_index, vec4 *tangent);
	void CreateTangentArray(vertex_t *vertex, bspvertex_t *bsp_vertex, int num_vert, vec4 *tangent);
	~Bsp()
	{
	}


	bool collision_detect(vec3 &point, plane_t *plane, float *depth, bool &water, float &water_depth);

	bool loaded;
	bspData_t	data;
	char map_name[80];
private:
	bsp_t		*tBsp;

	vec4		*tangent;
	vector<int> face_list;
	vector<int> blend_list;

	vector<int> model_list;
	vector<int> model_blend_list;
	
	int last_leaf;

	// keeping this around because it seems like data.vertex is getting corrupted.
	vertex_t *vertex;

	//for bezier patches
	int				mesh_level;
	int				*mesh_index2face;
	vertex_t		**mesh_vertex_array;
	int				**mesh_index_array;
	int				*mesh_num_verts;
	int				*mesh_num_indexes;
	int				num_meshes;
	unsigned int	*mesh_index_vbo;
	unsigned int	*mesh_vertex_vbo;

	//objects
	unsigned int	map_index_vbo;
	unsigned int	map_vertex_vbo;

	int	*tex_object;
	unsigned int	*normal_object;
	unsigned int	*lightmap_object;
};

#endif
