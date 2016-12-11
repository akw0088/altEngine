#include "include.h"

#ifndef BSP_H
#define BSP_H

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
	void load_textures(Graphics &gfx);
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
