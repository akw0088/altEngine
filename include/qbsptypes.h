#define BSPVERSION   29

#define LUMP_ENTITIES		0
#define LUMP_PLANES			1
#define LUMP_TEXTURES		2
#define LUMP_VERTEXES		3
#define LUMP_VISIBILITY		4
#define LUMP_NODES			5
#define LUMP_TEXINFO		6
#define LUMP_FACES			7
#define LUMP_LIGHTING		8
#define LUMP_CLIPNODES		9
#define LUMP_LEAFS			10
#define LUMP_MARKSURFACES	11
#define LUMP_EDGES			12
#define LUMP_SURFEDGES		13
#define LUMP_MODELS			14
#define HEADER_LUMPS		15

#define CONTENTS_EMPTY		-1
#define CONTENTS_SOLID		-2
#define CONTENTS_WATER		-3
#define CONTENTS_SLIME		-4
#define CONTENTS_LAVA		-5
#define CONTENTS_SKY		-6

#define AMBIENT_WATER		0
#define AMBIENT_SKY			1
#define AMBIENT_SLIME		2
#define AMBIENT_LAVA		3
#define NUM_AMBIENTS		4      // automatic ambient sounds

#define MAX_MAP_HULLS		4

// 0-2 are axial planes
#define PLANE_X				0
#define PLANE_Y				1
#define PLANE_Z				2

// 3-5 are non-axial planes snapped to the nearest
#define PLANE_ANYX			3
#define PLANE_ANYY			4
#define PLANE_ANYZ			5

#define is_marked(x)     (vis_face[(x) >> 3] &   (1 << ((x) & 7)))
#define mark_face(x)     (vis_face[(x) >> 3] |=  (1 << ((x) & 7)))
#define unmark_face(x)   (vis_face[(x) >> 3] &= ~(1 << ((x) & 7)))

typedef unsigned char byte;
typedef int fix;

typedef struct {
	char *bits;
	int wid;
	int ht;
} bitmap;




typedef struct
{
	int offset;
	int size;
} lump_t;

typedef struct
{
	int version;
	lump_t entity;
	lump_t plane;
	lump_t tex;
	lump_t vert;
	lump_t vis;
	lump_t node;
	lump_t texinfo;
	lump_t face;
	lump_t lightmap;
	lump_t clipnode;
	lump_t leaf;
	lump_t marksurf;
	lump_t edge;
	lump_t surface_edge;
	lump_t model;
} qbsp_t;

typedef struct
{
	float      mins[3], maxs[3];
	float      origin[3];
	int         headnode[MAX_MAP_HULLS];
	int         visleafs;      // not including the solid leaf 0
	int         firstface, numfaces;
} dmodel_t;

typedef struct
{
	int         version;
	lump_t      lumps[HEADER_LUMPS];
} dheader_t;

typedef struct
{
	int         nummiptex;
	int         dataofs[4];      // [nummiptex]
} dmiptexlump_t;

#define   MIPLEVELS   4
typedef struct miptex_s
{
	char      name[16];
	unsigned   width, height;
	unsigned   offsets[MIPLEVELS];      // four mip maps stored
} miptex_t;


typedef struct
{
	float   point[3];
} dvertex_t;



typedef struct
{
	float   normal[3];
	float   dist;
	int      type;      // PLANE_X - PLANE_ANYZ ?remove? trivial to regenerate
} dplane_t;

// !!! if this is changed, it must be changed in asm_i386.h too !!!
typedef struct
{
	int         planenum;
	short      children[2];   // negative numbers are -(leafs+1), not nodes
	short      mins[3];      // for sphere culling
	short      maxs[3];
	unsigned short   firstface;
	unsigned short   numfaces;   // counting both sides
} dnode_t;

typedef struct
{
	int         planenum;
	short      children[2];   // negative numbers are contents
} dclipnode_t;


typedef struct texinfo_s
{
	float      vecs[2][4];      // [s/t][xyz offset]
	int         miptex;
	int         flags;
} texinfo_t;
#define TEX_SPECIAL   1      // sky or slime, no lightmap or 256 subdivision

// note that edge 0 is never used, because negative edge nums are used for
// counterclockwise use of the edge in a face
typedef struct
{
	unsigned short   v[2];      // vertex numbers
} dedge_t;


typedef unsigned char byte;

#define   MAXLIGHTMAPS   4
typedef struct
{
	short      planenum;
	short      side;

	int         firstedge;      // we must support > 64k edges
	short      numedges;
	short      texinfo;

	// lighting info
	byte      styles[MAXLIGHTMAPS];
	int         lightofs;      // start of [numstyles*surfsize] samples
} dface_t;

// leaf 0 is the generic CONTENTS_SOLID leaf, used for all solid areas
// all other leafs need visibility info
typedef struct
{
	int         contents;
	int         visofs;            // -1 = no visibility info

	short      mins[3];         // for frustum culling
	short      maxs[3];

	unsigned short      firstmarksurface;
	unsigned short      nummarksurfaces;

	byte      ambient_level[NUM_AMBIENTS];
} dleaf_t;

typedef struct
{
	float x, y, z;
} vector;

typedef struct
{
	vector p;
	unsigned char  ccodes;
	unsigned char  pad0, pad1, pad2;
	fix    sx, sy;
	float  u, v;
} point_3d;   // 32 bytes
