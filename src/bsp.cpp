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
	data.Ent = (byte *)			&pBsp[tBsp->directory[Entities].offset];
	data.Texture = (texture_t *)&pBsp[tBsp->directory[Textures].offset];
	data.Plane = (plane_t *)	&pBsp[tBsp->directory[Planes].offset];
	data.Node = (node_t *)		&pBsp[tBsp->directory[Nodes].offset];
	data.Leaf = (leaf_t *)		&pBsp[tBsp->directory[Leafs].offset];
	data.LeafFace = (int *)		&pBsp[tBsp->directory[LeafFaces].offset];
	data.LeafBrush = (int *)	&pBsp[tBsp->directory[LeafBrushes].offset];
	data.Brushes = (brush_t *)	&pBsp[tBsp->directory[Brushes].offset];
	data.BrushSides = (brushSide_t *)	&pBsp[tBsp->directory[BrushSides].offset];
	data.Vert = (vertex_t *)	&pBsp[tBsp->directory[Vertices].offset];
	data.Face = (face_t *)		&pBsp[tBsp->directory[Faces].offset];
	data.VisData = (visData_t *)&pBsp[tBsp->directory[VisData].offset];
	data.Index = (int *)		&pBsp[tBsp->directory[Indexes].offset];

	data.num_verts = tBsp->directory[Vertices].length / sizeof(vertex_t);
	data.num_ents = tBsp->directory[Entities].length;
	data.num_textures = tBsp->directory[Textures].length / sizeof(texture_t);
	data.num_planes = tBsp->directory[Planes].length / sizeof(plane_t);
	data.num_nodes = tBsp->directory[Nodes].length / sizeof(node_t);
	data.num_leafs = tBsp->directory[Leafs].length / sizeof(leaf_t);
	data.num_LeafFaces = tBsp->directory[LeafFaces].length / sizeof(int);
	data.num_LeafBrushes = tBsp->directory[LeafBrushes].length / sizeof(int);
	data.num_brushes = tBsp->directory[Brushes].length / sizeof(brush_t);
	data.num_BrushSides = tBsp->directory[BrushSides].length / sizeof(brushSide_t);
	data.num_faces = tBsp->directory[Faces].length / sizeof(face_t);
	data.num_vis = tBsp->directory[VisData].length / sizeof(visData_t);
	data.num_index = tBsp->directory[Indexes].length / sizeof(int);

	changeAxis();
}

/*
	Loops through map data to find bezeir patches to tessellate into verticies
	And now also creates vbos for map and meshes
*/
void Bsp::generate_meshes(Graphics &gfx)
{
	int mesh_index = 0;

	num_meshes = 0;
	mesh_level = 8;

	for (int i = 0; i < data.num_faces; i++)
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

	mesh_vertex_vbo = new unsigned int [num_meshes];
	mesh_index_vbo = new unsigned int [num_meshes];

	for (int i = 0; i < data.num_faces; i++)
	{
		face_t *face = &data.Face[i];

		if (face->type == 2)
		{
			tessellate(mesh_level, &(data.Vert[face->vertexIndex]), &mesh_vertex_array[mesh_index], mesh_numVerts[mesh_index], &mesh_index_array[mesh_index], mesh_numIndexes[mesh_index]);
			mesh_index2face[mesh_index] = face->vertexIndex;
			mesh_vertex_vbo[mesh_index] = gfx.CreateVertexBuffer(mesh_vertex_array[mesh_index], mesh_numVerts[mesh_index]);
			mesh_index_vbo[mesh_index] = gfx.CreateIndexBuffer(mesh_index_array[mesh_index], mesh_numIndexes[mesh_index]);
			mesh_index++;
		}
	}


	map_vertex_vbo = gfx.CreateVertexBuffer(data.Vert, data.num_verts);
	map_index_vbo = gfx.CreateIndexBuffer(data.Index, data.num_index);
}

/*
	Converts axis from quake3 to opengl format
*/
void Bsp::changeAxis()
{
	for(int i = 0; i < data.num_verts; i++)
	{
		vertex_t *vert = &data.Vert[i];
		float temp;

		temp = vert->vPosition.y;
		vert->vPosition.y = vert->vPosition.z;
		vert->vPosition.z =  -temp;
	}

	for(int i = 0; i < data.num_planes; i++)
	{
		plane_t *plane = &data.Plane[i];
		float	temp;

		temp = plane->vNormal.y;
		plane->vNormal.y = plane->vNormal.z;
		plane->vNormal.z =  -temp;
	}
}

/*
	Returns entity string for parsing
*/
const char *Bsp::getEnts()
{
	return (const char *)data.Ent;
}

/*
	Should cleanly unload a map
*/
void Bsp::unload()
{
	delete [] mesh_index2face;
	delete [] mesh_vertex_array;
	delete [] mesh_index_array;
	delete [] mesh_numVerts;
	delete [] mesh_numIndexes;
	delete [] tex_object; // need to unload from gfx card first
	free((void *)tBsp);
}

/*
	Given a position return the bsp leaf node containing that point
*/
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

/*
	Should return a set collision planes within the entities leaf node
*/
void Bsp::get_visible_planes(Entity &entity, Plane *plane, int &num_planes)
{
	int frameIndex = findLeaf(entity.position);
	leaf_t *frameLeaf = &data.Leaf[frameIndex];

	num_planes = 0;
	for (int i = 0; i < data.num_leafs; i++)
	{
		leaf_t *leaf = &data.Leaf[i];

		if ( !isClusterVisible(frameLeaf->cluster, leaf->cluster) )
			continue;

		for (int j = 0; j < leaf->numOfLeafBrushes; j++)
		{
			brush_t	*brush = &data.Brushes[leaf->leafBrush + j];
			int brush_index = brush->brushSide;
			int num_brushes = brush->numOfBrushSides;

			for( int k = 0; k < num_brushes; k++)
			{
				brushSide_t *brushSide = &data.BrushSides[brush_index + k];
				int plane_index = brushSide->plane;
				plane[num_planes++] = Plane::Plane(data.Plane[plane_index].vNormal, data.Plane[plane_index].d);
			}
		}
	}
}

/*
	Gets all collision planes
*/
void Bsp::get_collision_planes(Plane **rplane, int &num_planes)
{
	Plane *plane = new Plane [data.num_BrushSides];
	num_planes = 0;

	for (int i = 0; i < data.num_brushes; i++)
	{
		int brush_index = data.Brushes[i].brushSide;
		int num_brushes = data.Brushes[i].numOfBrushSides;

		for( int j = 0; j < num_brushes; j++)
		{
			int plane_index = data.BrushSides[brush_index + j].plane;
			plane[num_planes++] = Plane::Plane(data.Plane[plane_index].vNormal, data.Plane[plane_index].d);
		}
	}
	*rplane = plane;
}

void Bsp::render_face(face_t *face, Graphics &gfx)
{
	int vertex_offset = (int)((char *)&(data.Vert[face->vertexIndex].vPosition) - (char *)data.Vert);
	int texture_offset = (int)((char *)&(data.Vert[face->vertexIndex].vTextureCoord) - (char *)data.Vert);
	int normal_offset = (int)((char *)&(data.Vert[face->vertexIndex].vNormal) - (char *)data.Vert);
	int index_offset = (int)((char *)&data.Index[face->index] - (char *)data.Index);

	gfx.SelectVertexBuffer(map_vertex_vbo);
	gfx.SelectIndexBuffer(map_index_vbo);

	gfx.VertexArray( (void *)vertex_offset);
	gfx.TextureArray( (void *)texture_offset);
	gfx.NormalArray( (void *)normal_offset);
//	glColorPointer(sizeof(int), GL_BYTE, sizeof(vertex_t),  (void *)((char *)&(data.Vert[face->vertexIndex].color) - (char *)data.Vert));

	gfx.SelectTexture(tex_object[face->textureID]);
	gfx.DrawArray("triangle", (void *)index_offset, face->num_index, face->num_verts);
	gfx.DeselectTexture();
	gfx.SelectVertexBuffer(0);
	gfx.SelectIndexBuffer(0);
}

void Bsp::render_patch(face_t *face, Graphics &gfx, Keyboard &keyboard)
{
	int mesh_index = -1;
	int index_per_row = 2 * (mesh_level + 1);

	// Find pre-generated vertex data for patch O(n)
	for( int i = 0; i < num_meshes; i++)
	{
		if (mesh_index2face[i] == face->vertexIndex)
		{
			mesh_index = i;
			break;
		}
	}

	if (keyboard.control)
	{
		gfx.SelectVertexBuffer(mesh_vertex_vbo[mesh_index]);
		gfx.SelectIndexBuffer(mesh_index_vbo[mesh_index]);

		gfx.VertexArray( (void *)((char *)&(mesh_vertex_array[mesh_index]->vPosition) - (char *)mesh_vertex_array[mesh_index]));
		gfx.TextureArray( (void *)((char *)&(mesh_vertex_array[mesh_index]->vTextureCoord) - (char *)mesh_vertex_array[mesh_index]));
		gfx.NormalArray(  (void *)((char *)&(mesh_vertex_array[mesh_index]->vNormal) - (char *)mesh_vertex_array[mesh_index]));
//		glColorPointer(sizeof(int), GL_BYTE, sizeof(vertex_t),  (void *)((char *)&(mesh_vertex_array[mesh_index]->color) - (char *)mesh_vertex_array[mesh_index]));
	}
	else
	{
		gfx.SelectVertexBuffer(0);
		gfx.SelectIndexBuffer(0);

		gfx.VertexArray(&(mesh_vertex_array[mesh_index]->vPosition));
		gfx.TextureArray( &(mesh_vertex_array[mesh_index]->vTextureCoord));
		gfx.NormalArray(  &(mesh_vertex_array[mesh_index]->vNormal));
//		glColorPointer(sizeof(int), GL_BYTE, sizeof(vertex_t), &(mesh_vertex_array[mesh_index]->color) );
	}
	// Render each row
	gfx.SelectTexture(tex_object[face->textureID]);
	for( int row = 0; row < mesh_level; row++)
	{
		//VBOs arent rendering entire patch, I see no problem code wise, sticking with indexed arrays.
		if (keyboard.control)
		{
			gfx.DrawArray("triangle_strip",
				(void *)(row * index_per_row),
				index_per_row, mesh_numVerts[mesh_index]);
		}
		else
		{
#ifndef DIRECTX
			gfx.DrawArray("triangle_strip",
				&mesh_index_array[mesh_index][row * index_per_row],
				index_per_row, mesh_numVerts[mesh_index]);
#endif
		}

	}
	gfx.DeselectTexture();
}

void Bsp::render(Entity &entity, Graphics &gfx, Keyboard &keyboard)
{
	int frameIndex = findLeaf(entity.position);
	int numTriangles = 0;
	char msg[80];

	leaf_t *frameLeaf = &data.Leaf[frameIndex];

	// loop through all leaves, checking if leaf visible from current leaf
	for (int i = 0; i < data.num_leafs; i++)
	{
		leaf_t *leaf = &data.Leaf[i];

		if ( !isClusterVisible(frameLeaf->cluster, leaf->cluster) )
			continue;

		// draw faces within visible leaf's
		for (int j = 0; j < leaf->numOfLeafFaces; j++)
		{
			int faceIndex = data.LeafFace[leaf->leafface + j];
			face_t *face = &data.Face[faceIndex];

			if (face->type == 1 || face->type == 3)
			{
				render_face(face, gfx);
				numTriangles += face->num_index / 3;
			}
			else if (face->type == 2)
			{
				render_patch(face, gfx, keyboard);
				numTriangles += face->num_index / 3;
			}
		}
	}
	
	snprintf(msg, 80, "%d Triangles rendered", numTriangles);
	gfx.DrawText(msg, 0.01f, 0.02f);
}

/*
	Determines set of visible leafs from current leaf
*/
inline int Bsp::isClusterVisible(int visCluster, int testCluster)
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

void Bsp::load_textures(Graphics &gfx)
{
	tex_object = new unsigned int [data.num_textures];
	for (int i = 0; i < data.num_textures; i++)
	{
		texture_t	*texture = &data.Texture[i];
		byte		*bytes;
		int			width, height, components, format;
		char	buffer[120];

		snprintf(buffer, 120, "media/%s.tga", texture->name);
		bytes = gltLoadTGA(buffer, &width, &height, &components, &format);
		if (bytes == NULL)
			printf("Unable to load texture %s\n", buffer);

		tex_object[i] = gfx.LoadTexture(width, height, components, format, bytes);
		free((void *)bytes);
	}
}

/*
	This function does not work for cylindrical patches and 'U' patches

		I need to move away from bsp to my own map format so I dont
	feel like sitting down and writing a specification for something
	that will be replaced.
*/
void Bsp::tessellate(int level, vertex_t control[], vertex_t **vertex_array, int &numVerts, int **index_array, int &numIndexes)
{
	vec3 a, b;
	int i, j;

	numVerts = level + 1;

	*vertex_array = new vertex_t[numVerts * numVerts];

	// calculate first set of verts
	for (i = 0; i <= level; i++)
	{
		float a = (float) i / level;
		float b = 1.0f - a;
		(*vertex_array)[i].vPosition =
			control[0].vPosition * (b * b) +
			control[3].vPosition * (2 * b * a) +
			control[6].vPosition * (a * a);
	}

	// calculate rest of verts
	for ( i = 1; i <= level; i++)
	{
		float a = (float)i / level;
		float b = 1.0f - a;

		vertex_t temp[3];

		temp[0].vPosition = 
			control[0].vPosition * (b * b) + 
			control[1].vPosition * (2 * b * a) + 
			control[2].vPosition * (a * a);
		temp[1].vPosition = 
			control[3].vPosition * (b * b) + 
			control[4].vPosition * (2 * b * a) + 
			control[5].vPosition * (a * a);
		temp[2].vPosition = 
			control[6].vPosition * (b * b) + 
			control[7].vPosition * (2 * b * a) + 
			control[8].vPosition * (a * a);

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
	
	//Create index array
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

	// Generate normals
	for(i = 0; i <= level; i++)
	{
		for(j = 0; j <= level; j++)
		{
			if (j != level)
				a = (*vertex_array)[i * numVerts + j].vPosition - (*vertex_array)[i * numVerts + (j + 1)].vPosition;
			else
				b = (*vertex_array)[i * numVerts + j].vPosition - (*vertex_array)[i * numVerts + (j - 1)].vPosition;
				
			if ( i != level)
				b = (*vertex_array)[i * numVerts + j].vPosition - (*vertex_array)[(i + 1) * numVerts + j].vPosition;
			else
				b = (*vertex_array)[i * numVerts + j].vPosition - (*vertex_array)[(i - 1) * numVerts + j].vPosition;

			(*vertex_array)[i * numVerts + j].color = ~0;
			(*vertex_array)[i * numVerts + j].vTextureCoord = vec2((float)(i % 2), (float)(j % 2));
			(*vertex_array)[i * numVerts + j].vNormal = vec3::crossproduct(a,b).normalize();
		}
	}

	// correct numVerts size
	numVerts = numVerts * numVerts;
}

/*
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
*/