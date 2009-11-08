#ifndef WIN32
	typedef char byte;
#endif

typedef enum
{
	Entities = 0,		// Stores player/object positions, etc...
	Textures,		// Stores texture information
	Planes,			// Stores the splitting planes
	Nodes,			// Stores the BSP nodes
	Leafs,			// Stores the leafs of the nodes
	LeafFaces,		// Stores the leaf's indices into the faces
	LeafBrushes,		// Stores the leaf's indices into the brushes
	Models,			// Stores the info of world models
	Brushes,		// Stores the brushes info (for collision)
	BrushSides,		// Stores the brush surfaces info
	Vertices,		// Stores the level vertices
	Indexes,		// Stores the model vertices offsets - this is just wrong, these are the indicies for the face verticies. Try rendering q3map2 bsps.
	Shaders,		// Stores the shader files (blending, anims..)
	Faces,			// Stores the faces for the level
	Lightmaps,		// Stores the lightmaps for the level
	LightVolumes,		// Stores extra world lighting information
	VisData,		// Stores PVS and cluster info (visibility)
	MaxLumps		// A constant to store the number of lumps
} lumps;

typedef struct
{
	vec3	vPosition;		// (x, y, z) position. 
	vec2	vTextureCoord;	// (u, v) texture coordinate
	vec2	vLightmapCoord;	// (u, v) lightmap coordinate
	vec3	vNormal;		// (x, y, z) normal vector
	byte	color[4];		// RGBA color for the vertex 
} vertex_t;

typedef struct
{
	int	textureID;		// The index into the texture array 
	int	effect;			// The index for the effects (or -1 = n/a) 
	int	type;			// 1=polygon, 2=patch, 3=mesh, 4=billboard 
	int	vertexIndex;		// The index into this face's first vertex 
	int	numOfVerts;		// The number of vertices for this face 
	int	indexes;		// The index into the first meshvertex 
	int	numIndexes;		// The number of mesh vertices -- Indexes
	int	lightmapID;		// The texture index for the lightmap 
	int	lMapCorner[2];		// The face's lightmap corner in the image 
	int	lMapSize[2];		// The size of the lightmap section 
	float	lMapPos[3];		// The 3D origin of lightmap. 
	float	lMapVecs[2][3]; 	// The 3D space for s and t unit vectors. 
	float	vNormal[3];		// The face normal. 
	int	size[2];		// The bezier patch dimensions. 
} face_t;

typedef struct
{
	char	name[64];	// The name of the texture w/o the extension 
	int	flags;		// The surface flags (unknown) 
	int	contents;	// The content flags (unknown)
} texture_t;

typedef struct
{
	byte	imageBits[128][128][3];	// The RGB data in a 128x128 image
} lightmap_t;

typedef struct
{
	int	plane;		// The index into the planes array 
	int	front;		// The child index for the front node 
	int	back;		// The child index for the back node 
	int	mins[3];	// The bounding box min position. 
	int	maxs[3];	// The bounding box max position. 
} node_t;

typedef struct
{
	int	cluster;		// The visibility cluster 
	int	area;			// The area portal 
	int	mins[3];		// The bounding box min position 
	int	maxs[3];		// The bounding box max position 
	int	leafface;		// The first index into the face array 
	int	numOfLeafFaces;		// The number of faces for this leaf 
	int	leafBrush;		// The first index for into the brushes 
	int	numOfLeafBrushes;	// The number of brushes for this leaf 
} leaf_t;

typedef struct
{
	vec3	vNormal;	// Plane normal. 
	float	d;		// The plane distance from origin 
} plane_t;

typedef struct
{
	int	numOfVectors;	// This stores the number of bit-vectors
	int	vectorSize;	// The size of bit-vectors in bytes
	byte	pVecs;		// This holds all of the cluster bits
} visData_t;

typedef struct 
{
	int	brushSide;		// The starting brush side for the brush 
	int	numOfBrushSides;	// Number of brush sides for the brush
	int	textureID;		// The texture index for the brush
} brush_t;

typedef struct  
{
	int	plane;			// The plane index
	int	textureID;		// The texture index
} brushSide_t;

typedef struct  
{
	float	min[3];		// The min position for the bounding box
	float	max[3];		// The max position for the bounding box. 
	int	faceIndex;	// The first face index in the model 
	int	numOfFaces;	// The number of faces in the model 
	int	brushIndex;	// The first brush index in the model 
	int	numOfBrushes;	// The number brushes for the model
} model_t;

typedef struct
{
	char	strName[64];	// The name of the shader file 
	int	brushIndex;	// The brush index for this shader 
	int	unknown;	// This is 99% of the time 5
} shader_t;

typedef struct
{
	byte	ambient[3];	// This is the ambient color in RGB
	byte	directional[3];	// This is the directional color in RGB
	byte	direction[2];	// The direction of the light: [phi,theta] 
} light_t;

typedef struct
{
	int	offset;		// Offset to start of lump, relative to beginning of file. 
	int	length;		// Length of lump. Always a multiple of 4. 
} lump_t;

typedef struct
{
	byte	strId[4];		// IBSP
	int	version;		// 0x2e
	lump_t	directory[17];
} bsp_t;

typedef struct
{
	byte		*Ent;
	texture_t	*Texture;
	plane_t		*Plane;
	node_t		*Node;
	leaf_t		*Leaf;
	int		*LeafFace;
	int		*LeafBrush;
	vertex_t	*Vert;
	face_t		*Face;
	visData_t	*VisData;
	int		*Indexes;
	brush_t *Brushes;
	brushSide_t *BrushSides;

	int	numEnts;
	int	numTextures;
	int	numPlanes;
	int	numNodes;
	int	numLeafs;
	int	numLeafFaces;
	int	numLeafBrushes;
	int	numBrushes;
	int	numBrushSides;
	int	numVerts;
	int	numFaces;
	int	numVis;
	int	numIndexes;
} bspData_t;
