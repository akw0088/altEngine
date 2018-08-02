#include "include.h"



class QBsp
{
public:
	QBsp();
	int load(Graphics &gfx, char *filename);
	void bsp_render_world(Graphics &gfx, vec3 &cam_loc, q1dplane_t *pl);
	void bsp_render_node(Graphics &gfx, int node, vec3 &cam_loc);
	int point_plane_test(vec3 &loc, q1dplane_t *plane);
	void render_node_faces(Graphics &gfx, int node, int side);
	void draw_face(Graphics &gfx, int face);
	void build_face(Graphics &gfx, int face);
	void change_axis();
	void render(Graphics &gfx, vec3 &cam_loc);
	int visit_visible_leaves(vec3 &cam_loc);
	int find_leaf(vec3 &loc);
	void bsp_visit_visible_leaves(vec3 &cam_loc, q1dplane_t *pl);
	int bsp_find_visible_nodes(int node);
	void bsp_explore_node(int node);
	int leaf_in_frustrum(q1dleaf_t *node, q1dplane_t *planes);
	void mark_leaf_faces(int leaf);
	int compute_mip_level(int face, vec3 &loc);
	void get_tmap(bitmap *bm, int face, int tex, int ml, float *u, float *v);
	void get_face_extent(int face, int *u0, int *v0, int *u1, int *v1);
	void get_raw_tmap(bitmap *bm, int tex, int ml);
	void compute_texture_gradients(int face, int tex, int mip, float u, float v);

	bool loaded;

private:

	bool initialized;
	std::vector<unsigned int> index_array;
	int map_index_vbo;
	int map_vertex_vbo;
	int *face_start_index;
	int *face_count;


	q1dnode_t			*dnodes;
	q1texinfo_t		*texinfo;
	q1dface_t			*dfaces;
	q1dclipnode_t		*dclipnodes;
	q1dedge_t			*dedges;
	unsigned short	*dmarksurfaces;
	int				*dsurfedges;
	q1dplane_t		*dplanes;
	q1dmodel_t		*dmodels;

	byte        *dvisdata;
	byte        *dlightdata;
	byte        *dtexdata; // (dmiptexlump_t)
	char        *dentdata;
	q1dleaf_t      *dleafs;
	vec3    *dvertexes;





	int         numnodes;
	int         numtexinfo;
	int         numfaces;
	int         numclipnodes;
	int         numedges;
	int         nummarksurfaces;
	int         numsurfedges;
	int         numplanes;
	int         nummodels;

	int			visdatasize;
	int			lightdatasize;
	int			texdatasize;
	int			entdatasize;
	int			numleafs;
	int			numvertexes;
	int			numleaf;

	
	#define   MAX_MAP_LEAFS      10001
	char *vis_node;
	char *vis_face;
	char vis_leaf[MAX_MAP_LEAFS / 8 + 1];
	

	vertex_t *map_vertex;

	q1dplane_t *planes;

};
