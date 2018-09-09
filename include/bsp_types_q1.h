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

#ifndef BSP_TYPES_Q1
#define BSP_TYPES_Q1

namespace q1 {

#define BSPVERSION   29

	typedef enum
	{
		LUMP_ENTITIES,
		LUMP_PLANES,
		LUMP_TEXTURES,
		LUMP_VERTEXES,
		LUMP_VISIBILITY,
		LUMP_NODES,
		LUMP_TEXINFO,
		LUMP_FACES,
		LUMP_LIGHTING,
		LUMP_CLIPNODES,
		LUMP_LEAFS,
		LUMP_MARKSURFACES,
		LUMP_EDGES,
		LUMP_SURFEDGES,
		LUMP_MODELS,
		HEADER_LUMPS
	} q1lump_t;

#define Q1CONTENTS_EMPTY		-1
#define Q1CONTENTS_SOLID		-2
#define Q1CONTENTS_WATER		-3
#define Q1CONTENTS_SLIME		-4
#define Q1CONTENTS_LAVA		-5
#define Q1CONTENTS_SKY		-6

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

	typedef struct
	{
		char *bits;
		int width;
		int height;
	} bitmap;


	typedef unsigned short fixang;

	typedef struct
	{
		fixang tx, ty, tz;
	} angvec;

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
		vec3 normal;
		float   dist;
		int      type;      // PLANE_X - PLANE_ANYZ ?remove? trivial to regenerate
	} dplane_t;

	// !!! if this is changed, it must be changed in asm_i386.h too !!!
	typedef struct
	{
		int			planenum;
		short		front;   // negative numbers are -(leafs+1), not nodes
		short		back;
		short		mins[3];      // for sphere culling
		short		maxs[3];
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
		vec4      vecs[2];      // [s/t][xyz offset]
		int         miptex;
		int         flags;
	} dtexinfo_t;
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
		vec3 p;
		unsigned char  ccodes;
		unsigned char  pad0, pad1, pad2;
		int    sx, sy;
		float  u, v;
	} q1vertex_t;   // 32 bytes


	typedef struct
	{
		dnode_t			*dnodes;
		dtexinfo_t		*texinfo;
		dface_t			*dfaces;
		dclipnode_t		*dclipnodes;
		dedge_t			*dedges;
		unsigned short	*dmarksurfaces;
		int				*dsurfedges;
		dplane_t		*dplanes;
		dmodel_t		*dmodels;

		char        *dvisdata;
		unsigned char        *dlightdata;
		char        *dtexdata; // (dmiptexlump_t)
		char        *dentdata;
		dleaf_t      *dleafs;
		vec3    *dvertexes;

		int         num_nodes;
		int         num_texinfo;
		int         num_faces;
		int         num_clipnodes;
		int         num_edges;
		int         num_marksurfaces;
		int         num_surfedges;
		int         num_planes;
		int         num_models;
		int			num_leafs;
		int			num_vertexes;
		int			num_leaf;
	} q1bsp_t;

	typedef struct
	{
		int face;
		int mip_level;
		int valid;
		bitmap *bm;
		float u, v;
	} surf_t;

};
#endif
