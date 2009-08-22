#include "include.h"

void Bsp::load(char *map)
{
	tBsp = (bsp_t *)getFile(map);
	byte *pBsp = (byte *)tBsp;

	// init data
	data.Vert = (vertex_t *)	&pBsp[tBsp->directory[Vertices].offset];
	data.Ent = (byte *)		&pBsp[tBsp->directory[Entities].offset];
	data.Texture = (texture_t *)	&pBsp[tBsp->directory[Textures].offset];
	data.Plane = (plane_t *)	&pBsp[tBsp->directory[Planes].offset];
	data.Node = (node_t *)		&pBsp[tBsp->directory[Nodes].offset];
	data.Leaf = (leaf_t *)		&pBsp[tBsp->directory[Leafs].offset];
	data.LeafFace = (int *)		&pBsp[tBsp->directory[LeafFaces].offset];
	data.LeafBrush = (int *)	&pBsp[tBsp->directory[LeafBrushes].offset];
	data.Vert = (vertex_t *)	&pBsp[tBsp->directory[Vertices].offset];
	data.Face = (face_t *)		&pBsp[tBsp->directory[Faces].offset];
	data.VisData = (visData_t *)	&pBsp[tBsp->directory[VisData].offset];
	data.Indexes = (int *)		&pBsp[tBsp->directory[Indexes].offset];

	data.numVerts = tBsp->directory[Vertices].length / sizeof(vertex_t);
	data.numEnts = tBsp->directory[Entities].length;
	data.numTextures = tBsp->directory[Textures].length / sizeof(texture_t);
	data.numPlanes = tBsp->directory[Planes].length / sizeof(plane_t);
	data.numNodes = tBsp->directory[Nodes].length / sizeof(node_t);
	data.numLeafs = tBsp->directory[Leafs].length / sizeof(leaf_t);
	data.numLeafFaces = tBsp->directory[LeafFaces].length / sizeof(int);
	data.numLeafBrushes = tBsp->directory[LeafBrushes].length / sizeof(int);
	data.numFaces = tBsp->directory[Faces].length / sizeof(face_t);
	data.numVis = tBsp->directory[VisData].length / sizeof(visData_t);

	changeAxis();
}

void Bsp::changeAxis()
{
	for(int i = 0; i < data.numVerts; i++)
	{
		vertex_t *vert = &data.Vert[i];
		float temp;

		temp = vert->vPosition.y;
		vert->vPosition.y = vert->vPosition.z;
		vert->vPosition.z =  -temp;
	}

	for(int i = 0; i < data.numPlanes; i++)
	{
		plane_t *plane = &data.Plane[i];
		float	temp;

		temp = plane->vNormal.y;
		plane->vNormal.y = plane->vNormal.z;
		plane->vNormal.z =  -temp;
	}
}


void Bsp::unload()
{
	free((void *)tBsp);
}

int Bsp::findLeaf(const vec3 &position)
{
	float		distance;
	int		i = 0;

	while(i >= 0)
	{
		node_t *node = &data.Node[i];
		plane_t *plane = &data.Plane[node->plane];

		distance = plane->vNormal * position;

		if(distance >= 0)	
			i = data.Node[i].front;
		else
			i = data.Node[i].back;
	}
	return -(i + 1);
}


void Bsp::render(vec3 &position, Graphics &gfx)
{
	int frameIndex = findLeaf(position);

	leaf_t *frameLeaf = &data.Leaf[frameIndex];

	for (int i = data.numLeafs - 1; i >= 0; i--)		// loop through all leaves, checking if leaf visible from current leaf
	{
		leaf_t *leaf = &data.Leaf[i];

		if ( !isClusterVisible(frameLeaf->cluster, leaf->cluster) )
			continue;

//		if ( !isFrustumVisible() )
//			continue;

		int numFaces = leaf->numOfLeafFaces;
		for (int j = numFaces - 1; j >= 0; j--) 	// draw faces within visible leaf's
		{
			int faceIndex = data.LeafFace[leaf->leafface + j];
			face_t *face = &data.Face[faceIndex];

			if (face->type == 2)
			{
				// bezier patch
			}
			else if (face->type == 4)
			{
				// billboard
			}
			else
			{
				gfx.VertexArray(  &(data.Vert[face->vertexIndex].vPosition), data.numVerts);
				gfx.TextureArray( &(data.Vert[face->vertexIndex].vTextureCoord), data.numVerts);
				gfx.NormalArray(  &(data.Vert[face->vertexIndex].vNormal), data.numVerts);

				if (face->type == 1)
					glColor3f(1.0f, 1.0f, 1.0f);
				else
					glColor3f(0.0f, 0.0f, 0.0f);

				gfx.SelectTexture(face->textureID);
				gfx.DrawArray(&data.Indexes[face->indexes], face->numIndexes);
				gfx.DeselectTexture();
			}
		}
	}
}


int Bsp::isClusterVisible(int visCluster, int testCluster)
{
	int byteOffset, bitOffset;
	char testByte;

	// if no vis data or if camera leaf invalid (outside map) draw regardless
	if (visCluster < 0)
		return 1;

	// bit of pVecs we need to return
	bitOffset = visCluster + testCluster * data.VisData->vectorSize * 8; 
	byteOffset = bitOffset / 8;
	testByte = 1 << (bitOffset - byteOffset * 8);
	return 	(&data.VisData->pVecs)[byteOffset] & testByte;
}

void Bsp::loadTextures(Graphics &gfx)
{
	gfx.InitTextures(data.numTextures);
	for (int i = 0; i < data.numTextures; i++)
	{
		texture_t	*texture = &data.Texture[i];
		byte		*bytes;
		int			width, height, components, format;
		char	buffer[120];

		sprintf(buffer, "Media/%s.tga", texture->name);
		bytes = gltLoadTGA(buffer, &width, &height, &components, &format);
		gfx.LoadTexture(i, width, height, components, format, bytes);
		free((void *)bytes);
	}
}

