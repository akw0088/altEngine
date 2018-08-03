#ifndef BSP_TYPES_Q2
#define BSP_TYPES_Q2
namespace q2 {
	typedef enum
	{
		Entity,
		Plane,
		Vertex,
		Visibility,
		Node,
		TexInfo,
		Face,
		Lightmap,
		Leaf,
		LeafFace,
		LeafBrush,
		Edge,
		FaceEdge,
		Model,
		Brush,
		BrushSide,
		Pop,
		Area,
		AreaPortal
	} q2lump_t;

	typedef struct
	{
		unsigned int offset;
		unsigned int length;
	} qdir_t;

	typedef struct
	{
		int magic;
		int version;
		qdir_t lump[19];
	} qheader_t;

	typedef struct
	{
		short int v[2];
	} qedge_t;

	typedef struct
	{
		short int plane;
		short int plane_size;
		int first_edge;
		short int num_edge;
		short int texinfo;
		char lm_style[4];
		int lm_offset;

	} qface_t;

	typedef struct
	{
		vec3 normal;
		float dist;
		int type;
	} qplane_t;

	typedef struct
	{
		int plane;
		int front;
		int back;
		int mins[3];
		int max[3];
		short int first_face;
		short int num_face;
	} qnode_t;

	typedef struct
	{
		int brush_or;
		short cluster;
		short area;
		int min[3];
		int max[3];
		short first_face;
		short num_face;
		short first_brush;
		short num_brush;
	} qleaf_t;

	typedef struct
	{
		vec4 vec_u;
		vec4 vec_v;
		int flag;
		int value;
		char name[32];
		int next;
	} qtexinfo_t;


	typedef struct
	{
		int pvs;
		int phs;
	} qvis_t;

	typedef struct
	{
		int temp;
	} qarea_t;

	typedef struct
	{
		int temp;
	} qareaportal_t;

	typedef struct
	{
		int temp;
	} qpop_t;

	typedef struct
	{
		int temp;
	} qbrush_t;

	typedef struct
	{
		int temp;
	} qbrushside_t;

	typedef struct
	{
		unsigned char r;
		unsigned char g;
		unsigned char b;
	} qlightmap_t;

	typedef struct
	{
		int temp;
	} qleafbrush_t;

	typedef struct
	{
		int temp;
	} qmodel_t;

	typedef struct
	{
		char name[32];
		int width;
		int height;
		int offset[4];
		char next[32];
		int flags;
		int contents;
		int value;
	} wal_header_t;


	typedef struct
	{
		char *Ent;
		qplane_t *plane;
		vec3 *vertex;
		qvis_t *vis;
		qnode_t *node;
		qtexinfo_t *texinfo;
		qface_t *face;
		qlightmap_t *lightmap;
		qleaf_t *leaf;
		unsigned short *leafface;
		qleafbrush_t *leafbrush;
		qedge_t *edge;
		unsigned int *face_edge;
		qmodel_t *model;
		qbrush_t *brush;
		qbrushside_t *brushside;
		qpop_t *pop;
		qarea_t *area;
		qareaportal_t *area_portal;

		unsigned int num_ent;
		unsigned int num_plane;
		unsigned int num_vertex;
		unsigned int num_vis;
		unsigned int num_node;
		unsigned int num_texinfo;
		unsigned int num_face;
		unsigned int num_lightmap;
		unsigned int num_leaf;
		unsigned int num_leafface;
		unsigned int num_leafbrush;
		unsigned int num_edge;
		unsigned int num_face_edge;
		unsigned int num_model;
		unsigned int num_brush;
		unsigned int num_brushside;
		unsigned int num_pop;
		unsigned int num_area;
		unsigned int num_area_portal;
	} qdata_t;

};
#endif