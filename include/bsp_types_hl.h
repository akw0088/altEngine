#include "include.h"

#define HL_HEADER_LUMPS 64
#define HL_BSP_HEADER (('P'<<24) + ('S' << 16) + ('B' << 8) + 'V')

// Reference
// https://developer.valvesoftware.com/wiki/Source_BSP_File_Format

enum dlumps
{
	LMP_ENTITIES = 0,
	LMP_PLANES,
	LMP_TEXDATA,
	LMP_VERTICES,
	LMP_VISIBILITY,
	LMP_NODES,
	LMP_TEXINFO,
	LMP_FACES,
	LMP_LIGHTING,
	LMP_OCCLUSION,
	LMP_LEAFS,
	LMP_FACEIDS,
	LMP_EDGES,
	LMP_SURFEDGES,
	LMP_MODELS,
	LMP_WORLDLIGHTS,
	LMP_LEAFFACES,
	LMP_LEAFBRUSHES,
	LMP_BRUSHES,
	LMP_BRUSHSIDES,
	LMP_AREAS,
	LMP_AREAPORTALS,
	LMP_PORTALS,
	LMP_UNUSED0,
	LMP_PROPCOLLISION,
	LMP_CLUSTERS,
	LMP_UNUSED1,
	LMP_PROPHULLS,
	LMP_PORTALVERTS,
	LMP_UNUSED2,
	LMP_PROPTRIS,
	LMP_DISPINFO,
	LMP_ORIGINALFACES,
	LMP_PHYSDISP,
	LMP_PHYSCOLLIDE,
	LMP_VERTNORMALS,
	LMP_VERTNORMALINDICES,
	LMP_DISP_LIGHTMAP_ALPHAS,
	LMP_DISP_VERTS,
	LMP_DISP_LIGHTMAP_SAMPLE_POSITIONS,
	LMP_GAME_LUMP,
	LMP_LEAFWATERDATA,
	LMP_PRIMITIVES,
	LMP_PRIMVERTS,
	LMP_PRIMINDICES,
	LMP_PAKFILE,
	LMP_CLIPPORTALVERTS,
	LMP_CUBEMAPS,
	LMP_TEXDATA_STRING_DATA,
	LMP_TEXDATA_STRING_TABLE,
	LMP_OVERLAYS,
	LMP_LEAFMINDISTTOWATER,
	LMP_FACE_MACRO_TEXTURE_INFO,
	LMP_DISP_TRIS,
	LMP_PHYSCOLLIDESURFACE,
	LMP_PROP_BLOB,
	LMP_WATEROVERLAYS,
	LMP_LIGHTMAPPAGES,
	LMP_LEAF_AMBIENT_INDEX_HDR,
	LMP_LIGHTMAPPAGEINFOS,
	LMP_LEAF_AMBIENT_INDEX,
	LMP_LIGHTING_HDR,
	LMP_WORLDLIGHTS_HDR,
	LMP_LEAF_AMBIENT_LIGHTING_HDR,
	LMP_LEAF_AMBIENT_LIGHTING,
	LMP_XZIPPAKFILE,
	LMP_FACES_HDR,
	LMP_MAP_FLAGS,
	LMP_OVERLAY_FADES,
	LMP_OVERLAY_SYSTEM_LEVELS,
	LMP_PHYSLEVEL,
	LMP_DISP_MULTIBLEND
};

typedef struct
{
	int offset;
	int length;
	int version;
	char ident[4];
} dlump_t;


typedef struct
{
	int magic;
	int version;
	dlump_t lumps[HL_HEADER_LUMPS];
	int revision;
} dheader_t;


typedef struct
{
	vec3 normal;
	float distance;
	int type;
} dplane_t;

typedef struct
{
	int		planenum;	// index into plane array
	int		children[2];	// negative numbers are -(leafs + 1), not nodes
	short		mins[3];	// for frustum culling
	short		maxs[3];
	unsigned short	firstface;	// index into face array
	unsigned short	numfaces;	// counting both sides
	short		area;		// If all leaves below this node are in the same area, then
							// this is the area index. If not, this is -1.
	short		paddding;	// pad to 32 bytes length
} dnode_t;

typedef struct
{
	int			contents;		// OR of all brushes (not needed?)
	short			cluster;		// cluster this leaf is in
	short			area : 9;			// area this leaf is in
	short			flags : 7;		// flags
	short			mins[3];		// for frustum culling
	short			maxs[3];
	unsigned short		firstleafface;		// index into leaffaces
	unsigned short		numleaffaces;
	unsigned short		firstleafbrush;		// index into leafbrushes
	unsigned short		numleafbrushes;
	short			leafWaterDataID;	// -1 for not in water

										//!!! NOTE: for maps of version 19 or lower uncomment this block
										/*
										CompressedLightCube	ambientLighting;	// Precaculated light info for entities.
										short			padding;		// padding to 4-byte boundary
										*/
} dleaf_t;


typedef struct
{
	unsigned short	planenum;	// facing out of the leaf
	short			texinfo;	// texture info
	short			dispinfo;	// displacement info
	short			bevel;		// is the side a bevel plane?
} dbrushside_t;

typedef struct
{
	int	firstside;	// first brushside
	int	numsides;	// number of brushsides
	int	contents;	// contents flags
} dbrush_t;

typedef struct
{
	unsigned short	planenum;						// the plane number
	byte			side;							// faces opposite to the node's plane direction
	byte			onNode;							// 1 of on node, 0 if in leaf
	int				firstedge;						// index into surfedges
	short			numedges;						// number of surfedges
	short			texinfo;						// texture info
	short			dispinfo;						// displacement info
	short			surfaceFogVolumeID;				// ?
	byte			styles[4];						// switchable lighting info
	int				lightofs;						// offset into lightmap lump
	float			area;							// face area in units^2
	int				LightmapTextureMinsInLuxels[2];	// texture lighting info
	int				LightmapTextureSizeInLuxels[2];	// texture lighting info
	int				origFace;						// original face this was split from
	unsigned short	numPrims;						// primitives
	unsigned short	firstPrimID;
	unsigned int	smoothingGroups;				// lightmap smoothing group
} dface_t;

typedef struct
{
	unsigned short	v[2];	// vertex indices
} dedge_t;


typedef struct
{
	vec3 reflectivity;		// RGB reflectivity
	int	nameStringTableID;	// index into TexdataStringTable
	int	width, height;		// source image
	int	view_width, view_height;
} dtexdata_t;

typedef struct
{
	float	textureVecs[2][4];	// [s/t][xyz offset] 
	float	lightmapVecs[2][4];	// [s/t][xyz offset] - length is in units of texels/area
	int     flags;              // miptex flags	overrides
	int     texdata;            // Pointer to texture name, size, etc.
} dtexinfo_t;

typedef struct
{
	int	numclusters;
	//	int	byteofs[numclusters][2]
} dvis_t;

typedef struct
{
	int		id;		// gamelump ID
	unsigned short	flags;		// flags
	unsigned short	version;	// gamelump version
	int		fileofs;	// offset to this gamelump
	int		filelen;	// length
} dgamelump_t;


typedef struct {
	int lumpCount;
	dgamelump_t gamelump[10];
} dgamelumpheader_t;


typedef struct
{
	int	dictEntries;
	char	name[128];	// model name
}StaticPropDictLump_t;

struct StaticPropLeafLump_t
{
	int leafEntries;
	unsigned short	leaf[10];
};

typedef struct
{
	// v4
	vec3		Origin;		 // origin
	vec3		Angles;		 // orientation (pitch roll yaw)
	unsigned short	PropType;	 // index into model name dictionary
	unsigned short	FirstLeaf;	 // index into leaf array
	unsigned short	LeafCount;
	unsigned char	Solid;		 // solidity type
	unsigned char	Flags;
	int		Skin;		 // model skin numbers
	float		FadeMinDist;
	float		FadeMaxDist;
	vec3		LightingOrigin;  // for lighting
								 // since v5
	float		ForcedFadeScale; // fade distance scale
								 // v6 and v7 only
	unsigned short  MinDXLevel;      // minimum DirectX version to be visible
	unsigned short  MaxDXLevel;      // maximum DirectX version to be visible
									 // since v8
	unsigned char   MinCPULevel;
	unsigned char   MaxCPULevel;
	unsigned char   MinGPULevel;
	unsigned char   MaxGPULevel;
	// since v7
	int		        DiffuseModulation; // per instance color and alpha modulation
									   // since v10
	float           unknown;
	// since v9
	bool            DisableX360;     // if true, don't show on XBox 360
} StaticPropLump_t;


typedef struct
{
	vec3			startPosition;		// start position used for orientation
	int			DispVertStart;		// Index into LUMP_DISP_VERTS.
	int			DispTriStart;		// Index into LUMP_DISP_TRIS.
	int			power;			// power - indicates size of surface (2^power	1)
	int			minTess;		// minimum tesselation allowed
	float			smoothingAngle;		// lighting smoothing angle
	int			contents;		// surface contents
	unsigned short		MapFace;		// Which map face this displacement comes from.
	int			LightmapAlphaStart;	// Index into ddisplightmapalpha.
	int			LightmapSamplePositionStart;	// Index into LUMP_DISP_LIGHTMAP_SAMPLE_POSITIONS.
	//CDispNeighbor		EdgeNeighbors[4];	// Indexed by NEIGHBOREDGE_ defines.
	//CDispCornerNeighbors	CornerNeighbors[4];	// Indexed by CORNER_ defines.
	unsigned int		AllowedVerts[10];	// active verticies
} ddispinfo_t;

struct dDispVert
{
	vec3	vec;	// Vector field defining displacement volume.
	float	dist;	// Displacement distances.
	float	alpha;	// "per vertex" alpha values.
};

struct dDispTri
{
	unsigned short Tags;	// Displacement triangle tags.
};

struct dcubemapsample_t
{
	int		origin[3];	// position of light snapped to the nearest integer
	int	        size;		// resolution of cubemap, 0 - default
};

struct doverlay_t
{
	int		Id;
	short		TexInfo;
	unsigned short	FaceCountAndRenderOrder;
	int		Ofaces[10];
	float		U[2];
	float		V[2];
	vec3		UVPoints[4];
	vec3		Origin;
	vec3		BasisNormal;
};

struct ColorRGBExp32
{
	byte r, g, b;
	signed char exponent;
};

struct dleafambientlighting_t
{
//	CompressedLightCube	cube;
	byte x;		// fixed point fraction of leaf bounds
	byte y;		// fixed point fraction of leaf bounds
	byte z;		// fixed point fraction of leaf bounds
	byte pad;	// unused
};

struct CompressedLightCube
{
	ColorRGBExp32 m_Color[6];
};

struct dleafambientindex_t
{
	unsigned short ambientSampleCount;
	unsigned short firstAmbientSample;
};

typedef struct 
{
	int			count;
//	doccluderdata_t		data[10];
	int			polyDataCount;
//	doccluderpolydata_t	polyData[10];
	int			vertexIndexCount;
	int			vertexIndices[10];
}doccluder_t;

typedef struct
{
	int	flags;
	int	firstpoly;	// index into doccluderpolys
	int	polycount;	// amount of polygons
	vec3	mins;	        // minima of all vertices
	vec3	maxs;	        // maxima of all vertices
							// since v1
	int	area;
} doccluderdata_t;

typedef struct
{
	int	firstvertexindex;	// index into doccludervertindices
	int	vertexcount;		// amount of vertex indices
	int	planenum;
} doccluderpolydata_t;

typedef struct 
{
	int modelIndex;  // Perhaps the index of the model to which this physics model applies?
	int dataSize;    // Total size of the collision data sections
	int keydataSize; // Size of the text section
	int solidCount;  // Number of collision data sections
} dphysmodel_t;

typedef struct
{
	dheader_t			*header;
	dnode_t				*Node;
	dleaf_t				*Leaf;
	dface_t				*Face;
	dedge_t				*Edge;
	int					*SurfEdge;
	unsigned short int	*LeafFace;
	vec3				*Vert;
	dplane_t			*Plane;

	unsigned int	num_nodes;
	unsigned int	num_leafs;
	unsigned int	num_faces;
	unsigned int	num_edges;
	unsigned int	num_surfedges;
	unsigned int	num_LeafFaces;
	unsigned int	num_verts;
	unsigned int	num_planes;
} hl_bsp_t;
