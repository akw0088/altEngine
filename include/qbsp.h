#include "qbsptypes.h"

class QBsp
{
public:
	int load(char *filename);
	void bsp_render_world(vector *cam_loc, dplane_t *pl);
	void bsp_render_node(int node);
	int point_plane_test(vector *loc, dplane_t *plane);
	void render_node_faces(int node, int side);
	void draw_face(int face);
	void SwapBSPFile(bool todisk);
	void render();
	int visit_visible_leaves(vector *cam_loc);
	int find_leaf(vector *loc);
	void bsp_visit_visible_leaves(vector *cam_loc, dplane_t *pl);
	int bsp_find_visible_nodes(int node);
	void bsp_explore_node(int node);
	int leaf_in_frustrum(dleaf_t *node, dplane_t *planes);
	void mark_leaf_faces(int leaf);
	int compute_mip_level(int face);
	void get_tmap(bitmap *bm, int face, int tex, int ml, float *u, float *v);
	void get_face_extent(int face, int *u0, int *v0, int *u1, int *v1);
	void get_raw_tmap(bitmap *bm, int tex, int ml);
	void compute_texture_gradients(int face, int tex, int mip, float u, float v);

private:

	dnode_t			*dnodes;
	texinfo_t		*texinfo;
	dface_t			*dfaces;
	dclipnode_t		*dclipnodes;
	dedge_t			*dedges;
	unsigned short	*dmarksurfaces;
	int				*dsurfedges;
	dplane_t		*dplanes;
	dmodel_t		*dmodels;

	byte        *dvisdata;
	byte        *dlightdata;
	byte        *dtexdata; // (dmiptexlump_t)
	char        *dentdata;
	dleaf_t      *dleafs;
	dvertex_t    *dvertexes;


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
	

	dplane_t *planes;
	vector *loc;
	vector cam_loc;
};
