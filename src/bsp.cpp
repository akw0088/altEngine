#include "include.h"

void Bsp::load(char *map)
{
	tBsp = (bsp_t *)getFile(map);
	byte *pBsp = (byte *)tBsp;

	if (tBsp == NULL)
	{
		char err[80];

		snprintf(err, 80, "Unable to load bsp %s.\n", map);
		throw err;
	}

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
	generateMeshes();
}

void Bsp::generateMeshes()
{
	int mesh_index = 0;

	num_meshes = 0;
	mesh_level = 8;

	for (int i = 0; i < data.numFaces; i++)
	{
		face_t *face = &data.Face[i];

		if (face->type == 2)
			num_meshes++;
	}

	mesh_index2face = new int [num_meshes];
	mesh_vertex_array = new vertex_t *[num_meshes];
	mesh_index_array = new int *[num_meshes];
	mesh_numVerts = new int [num_meshes];
	mesh_numIndexes = new int [num_meshes];

	for (int i = 0; i < data.numFaces; i++)
	{
		face_t *face = &data.Face[i];

		if (face->type == 2)
		{
			tessellate(mesh_level, &(data.Vert[face->vertexIndex]), &mesh_vertex_array[mesh_index], mesh_numVerts[mesh_index], &mesh_index_array[mesh_index], mesh_numIndexes[mesh_index]);
			mesh_index2face[mesh_index] = face->vertexIndex;
			mesh_index++;
		}
	}
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

const char *Bsp::getEnts()
{
	return (const char *)data.Ent;
}


void Bsp::unload()
{
	delete [] mesh_index2face;
	delete [] mesh_vertex_array;
	delete [] mesh_index_array;
	delete [] mesh_numVerts;
	delete [] mesh_numIndexes;
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

		distance = (plane->vNormal * position) - plane->d;

		if(distance >= 0)	
			i = data.Node[i].front;
		else
			i = data.Node[i].back;
	}
	return -(i + 1);
}


void Bsp::render(Entity &entity, Graphics &gfx, Keyboard &keyboard)
{
	int frameIndex = findLeaf(entity.position);
	int numTriangles = 0;
	char msg[80];


	leaf_t *frameLeaf = &data.Leaf[frameIndex];

	for (int i = data.numLeafs - 1; i >= 0; i--)		// loop through all leaves, checking if leaf visible from current leaf
	{
		leaf_t *leaf = &data.Leaf[i];

		if ( !isClusterVisible(frameLeaf->cluster, leaf->cluster) )
			continue;

		int numFaces = leaf->numOfLeafFaces;
		for (int j = numFaces - 1; j >= 0; j--) 	// draw faces within visible leaf's
		{
			int faceIndex = data.LeafFace[leaf->leafface + j];
			face_t *face = &data.Face[faceIndex];

			// bezier patch
			if ((face->type == 2))
			{
				int mesh_index = -1;

				for( int k = 0; k < num_meshes; k++)
				{
					if (mesh_index2face[k] == face->vertexIndex)
					{
						mesh_index = k;
						break;
					}
				}

				gfx.VertexArray(mesh_vertex_array[mesh_index], mesh_numVerts[mesh_index]);
				gfx.TextureArray( &(data.Vert[face->vertexIndex].vTextureCoord), mesh_numVerts[mesh_index]);
//				gfx.NormalArray(  &(data.Vert[face->vertexIndex].vNormal), data.numVerts);

				for( int row = 0; row < mesh_level; row++)
				{
					gfx.SelectTexture(face->textureID);
					glDrawElements(GL_TRIANGLE_STRIP,
						2 * (mesh_level + 1), GL_UNSIGNED_INT,
						&mesh_index_array[mesh_index][row * 2 * (mesh_level + 1)]);
					gfx.DeselectTexture();
				}
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
				numTriangles += face->numIndexes / 3;
			}
		}
	}
	
	snprintf(msg, 80, "%d Triangles rendered %f %f %f", numTriangles, entity.position.x, entity.position.y, entity.position.z);
	gfx.drawText(msg, 0.01f, 0.02f);
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

		snprintf(buffer, 120, "media/%s.tga", texture->name);
		bytes = gltLoadTGA(buffer, &width, &height, &components, &format);
		if (bytes == NULL)
			printf("Unable to load texture %s\n", buffer);

		gfx.LoadTexture(i, width, height, components, format, bytes);
		free((void *)bytes);
	}
}

void Bsp::tessellate(int level, vertex_t control[], vertex_t **vertex_array, int &numVerts, int **index_array, int &numIndexes)
{
	int i, j;

	numVerts = level + 1;

	*vertex_array = new vertex_t[numVerts * numVerts];

	for (i = 0; i <= level; i++)
	{
		float a = (float) i / level;
		float b = 1.0f - a;
		(*vertex_array)[i].vPosition = control[0].vPosition * (b * b) +
			control[3].vPosition * (2 * b * a) +
			control[6].vPosition * (a * a);
	}

	for ( i = 1; i <= level; i++)
	{
		float a = (float)i / level;
		float b = 1.0f - a;

		vertex_t temp[3];

		temp[0].vPosition = control[0].vPosition * (b * b) + control[1].vPosition * (2 * b * a) + control[2].vPosition * (a * a);
		temp[1].vPosition = control[3].vPosition * (b * b) + control[4].vPosition * (2 * b * a) + control[5].vPosition * (a * a);
		temp[2].vPosition = control[6].vPosition * (b * b) + control[7].vPosition * (2 * b * a) + control[8].vPosition * (a * a);

		for(j = 0; j <= level; j++)
		{
			float a = (float) j / level;
			float b = 1.0f - a;

			(*vertex_array)[i * numVerts + j].vPosition =
				temp[0].vPosition * (b * b) +
				temp[1].vPosition * (2 * b * a) +
				temp[2].vPosition * (a * a);
		}
	}
	
	numIndexes = level * numVerts * 2;
	*index_array = new int[numIndexes];
	for (i = 0; i < level; i++)
	{
		for(j = 0; j <= level; j++)
		{
			(*index_array)[(i * numVerts + j) * 2 + 1] = i * numVerts + j;
			(*index_array)[(i * numVerts + j) * 2] = (i + 1) * numVerts + j;
		}
	}
}

void Bsp::drawBox(int *min, int *max)
{
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_LINES);
	//bottom square
	glVertex3i(min[0], min[2], -min[1]);
	glVertex3i(max[0], min[2], -min[1]);
	glVertex3i(min[0], min[2], -min[1]);
	glVertex3i(min[0], max[2], -min[1]);
	glVertex3i(max[0], max[2], -min[1]);
	glVertex3i(max[0], min[2], -min[1]);
	glVertex3i(max[0], max[2], -min[1]);
	glVertex3i(min[0], max[2], -min[1]);
	//top square                     
	glVertex3i(min[0], min[2], -max[1]);
	glVertex3i(max[0], min[2], -max[1]);
	glVertex3i(min[0], min[2], -max[1]);
	glVertex3i(min[0], max[2], -max[1]);
	glVertex3i(max[0], max[2], -max[1]);
	glVertex3i(max[0], min[2], -max[1]);
	glVertex3i(max[0], max[2], -max[1]);
	glVertex3i(min[0], max[2], -max[1]);
	//remaining legs                 
	glVertex3i(min[0], min[2], -min[1]);
	glVertex3i(min[0], min[2], -max[1]);
	glVertex3i(min[0], max[2], -max[1]);
	glVertex3i(min[0], max[2], -min[1]);
                                         
	glVertex3i(max[0], min[2], -min[1]);
	glVertex3i(max[0], min[2], -max[1]);
	glVertex3i(max[0], max[2], -max[1]);
	glVertex3i(max[0], max[2], -min[1]);
	glEnd();
	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
}
