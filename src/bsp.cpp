#include "include.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Bsp::Bsp()
{
	loaded = false;
}

void Bsp::load(char *map)
{
	tBsp = (bsp_t *)get_file(map);
	byte *pBsp = (byte *)tBsp;

	if (tBsp == NULL)
	{
		char err[LINE_SIZE];

		snprintf(err, LINE_SIZE, "Unable to load bsp %s.\n", map);
		throw err;
	}

	// init data
	data.Vert = (bspvertex_t *)	&pBsp[tBsp->directory[VertexArray].offset];
	data.Ent = (byte *)			&pBsp[tBsp->directory[Entities].offset];
	data.Material = (material_t *)&pBsp[tBsp->directory[Materials].offset];
	data.Plane = (plane_t *)	&pBsp[tBsp->directory[Planes].offset];
	data.Node = (node_t *)		&pBsp[tBsp->directory[Nodes].offset];
	data.Leaf = (leaf_t *)		&pBsp[tBsp->directory[Leafs].offset];
	data.LeafFace = (int *)		&pBsp[tBsp->directory[LeafFaces].offset];
	data.LeafBrush = (int *)	&pBsp[tBsp->directory[LeafBrushes].offset];
	data.Brushes = (brush_t *)	&pBsp[tBsp->directory[Brushes].offset];
	data.BrushSides = (brushSide_t *)	&pBsp[tBsp->directory[BrushSides].offset];
	data.Face = (face_t *)		&pBsp[tBsp->directory[Faces].offset];
	data.VisData = (visData_t *)&pBsp[tBsp->directory[VisData].offset];
	data.IndexArray = (int *)		&pBsp[tBsp->directory[IndexArray].offset];
	data.LightMaps = (lightmap_t *)	&pBsp[tBsp->directory[LightMaps].offset];
	data.Fog = (fog_t *) &pBsp[tBsp->directory[Fog].offset];

	data.num_verts = tBsp->directory[VertexArray].length / sizeof(bspvertex_t);
	data.num_ents = tBsp->directory[Entities].length;
	data.num_materials = tBsp->directory[Materials].length / sizeof(material_t);
	data.num_planes = tBsp->directory[Planes].length / sizeof(plane_t);
	data.num_nodes = tBsp->directory[Nodes].length / sizeof(node_t);
	data.num_leafs = tBsp->directory[Leafs].length / sizeof(leaf_t);
	data.num_LeafFaces = tBsp->directory[LeafFaces].length / sizeof(int);
	data.num_LeafBrushes = tBsp->directory[LeafBrushes].length / sizeof(int);
	data.num_brushes = tBsp->directory[Brushes].length / sizeof(brush_t);
	data.num_BrushSides = tBsp->directory[BrushSides].length / sizeof(brushSide_t);
	data.num_faces = tBsp->directory[Faces].length / sizeof(face_t);
	data.num_vis = tBsp->directory[VisData].length / sizeof(visData_t);
	data.num_index = tBsp->directory[IndexArray].length / sizeof(int);
	data.num_lightmaps = tBsp->directory[LightMaps].length / sizeof(lightmap_t);
	data.num_fog = tBsp->directory[Fog].length / sizeof(fog_t);

	change_axis();


	tangent = new vec4 [data.num_verts];
	memset(tangent, 0, sizeof(vec4) * data.num_verts);
	CalculateTangentArray(data.Vert, data.num_verts, data.IndexArray, data.num_index, tangent);

	tex_object = new int [data.num_materials];
	lightmap_object = new unsigned int [data.num_lightmaps];
	normal_object = new unsigned int [data.num_materials];

	for(int i = 0; i < data.num_materials; i++)
		tex_object[i] = (unsigned int)-1;
	for(int i = 0; i < data.num_lightmaps; i++)
		lightmap_object[i] = (unsigned int)-1;
	for(int i = 0; i < data.num_materials; i++)
		normal_object[i] = (unsigned int)-1;

	loaded = true;
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
		{
			if (face->patchWidth == 9)
			{
				num_meshes += 4;
			}
			else if (face->patchWidth == 5)
			{
				num_meshes += 2;
			}
			else
			{
				num_meshes++;
			}
		}
	}

	mesh_index2face = new int [num_meshes];
	mesh_vertex_array = new vertex_t *[num_meshes];
	mesh_index_array = new int *[num_meshes];
	mesh_num_verts = new int [num_meshes];
	mesh_num_indexes = new int [num_meshes];

	mesh_vao = new unsigned int [num_meshes];
	mesh_vertex_vbo = new unsigned int [num_meshes];
	mesh_index_vbo = new unsigned int [num_meshes];

	for (int i = 0; i < data.num_faces; i++)
	{
		face_t *face = &data.Face[i];

		if (face->type == 2)
		{
			if (face->patchHeight == 3 && face->patchWidth == 3)
			{
				tessellate(mesh_level, &(data.Vert[face->vertex]), &mesh_vertex_array[mesh_index], mesh_num_verts[mesh_index], &mesh_index_array[mesh_index], mesh_num_indexes[mesh_index]);
				mesh_index2face[mesh_index] = face->vertex;
//				mesh_vao[mesh_index] = gfx.CreateVertexArrayObject();
				mesh_vertex_vbo[mesh_index] = gfx.CreateVertexBuffer(mesh_vertex_array[mesh_index], mesh_num_verts[mesh_index]);
				mesh_index_vbo[mesh_index] = gfx.CreateIndexBuffer(mesh_index_array[mesh_index], mesh_num_indexes[mesh_index]);
				mesh_index++;
			}
			else if (face->patchWidth == 5)
			{
				bspvertex_t controlpoint[9];

				controlpoint[0] = data.Vert[face->vertex];
				controlpoint[1] = data.Vert[face->vertex + 1];
				controlpoint[2] = data.Vert[face->vertex + 2];

				controlpoint[3] = data.Vert[face->vertex + 5];
				controlpoint[4] = data.Vert[face->vertex + 6];
				controlpoint[5] = data.Vert[face->vertex + 7];

				controlpoint[6] = data.Vert[face->vertex + 10];
				controlpoint[7] = data.Vert[face->vertex + 11];
				controlpoint[8] = data.Vert[face->vertex + 12];

				tessellate(mesh_level, controlpoint, &mesh_vertex_array[mesh_index], mesh_num_verts[mesh_index], &mesh_index_array[mesh_index], mesh_num_indexes[mesh_index]);
				mesh_index2face[mesh_index] = face->vertex;
//				mesh_vao[mesh_index] = gfx.CreateVertexArrayObject();
				mesh_vertex_vbo[mesh_index] = gfx.CreateVertexBuffer(mesh_vertex_array[mesh_index], mesh_num_verts[mesh_index]);
				mesh_index_vbo[mesh_index] = gfx.CreateIndexBuffer(mesh_index_array[mesh_index], mesh_num_indexes[mesh_index]);
				mesh_index++;

				controlpoint[0] = data.Vert[face->vertex + 2];
				controlpoint[1] = data.Vert[face->vertex + 3];
				controlpoint[2] = data.Vert[face->vertex + 4];

				controlpoint[3] = data.Vert[face->vertex + 7];
				controlpoint[4] = data.Vert[face->vertex + 8];
				controlpoint[5] = data.Vert[face->vertex + 9];

				controlpoint[6] = data.Vert[face->vertex + 12];
				controlpoint[7] = data.Vert[face->vertex + 13];
				controlpoint[8] = data.Vert[face->vertex + 14];

				tessellate(mesh_level, controlpoint, &mesh_vertex_array[mesh_index], mesh_num_verts[mesh_index], &mesh_index_array[mesh_index], mesh_num_indexes[mesh_index]);
//				mesh_vao[mesh_index] = gfx.CreateVertexArrayObject();
				mesh_vertex_vbo[mesh_index] = gfx.CreateVertexBuffer(mesh_vertex_array[mesh_index], mesh_num_verts[mesh_index]);
				mesh_index_vbo[mesh_index] = gfx.CreateIndexBuffer(mesh_index_array[mesh_index], mesh_num_indexes[mesh_index]);
				mesh_index++;
			}
			else if (face->patchWidth == 9)
			{
				bspvertex_t controlpoint[9];

				controlpoint[0] = data.Vert[face->vertex];
				controlpoint[1] = data.Vert[face->vertex + 1];
				controlpoint[2] = data.Vert[face->vertex + 2];

				controlpoint[3] = data.Vert[face->vertex + 9];
				controlpoint[4] = data.Vert[face->vertex + 10];
				controlpoint[5] = data.Vert[face->vertex + 11];

				controlpoint[6] = data.Vert[face->vertex + 18];
				controlpoint[7] = data.Vert[face->vertex + 19];
				controlpoint[8] = data.Vert[face->vertex + 20];

				tessellate(mesh_level, controlpoint, &mesh_vertex_array[mesh_index], mesh_num_verts[mesh_index], &mesh_index_array[mesh_index], mesh_num_indexes[mesh_index]);
				mesh_index2face[mesh_index] = face->vertex;
//				mesh_vao[mesh_index] = gfx.CreateVertexArrayObject();
				mesh_vertex_vbo[mesh_index] = gfx.CreateVertexBuffer(mesh_vertex_array[mesh_index], mesh_num_verts[mesh_index]);
				mesh_index_vbo[mesh_index] = gfx.CreateIndexBuffer(mesh_index_array[mesh_index], mesh_num_indexes[mesh_index]);
				mesh_index++;

				controlpoint[0] = data.Vert[face->vertex + 2];
				controlpoint[1] = data.Vert[face->vertex + 3];
				controlpoint[2] = data.Vert[face->vertex + 4];

				controlpoint[3] = data.Vert[face->vertex + 11];
				controlpoint[4] = data.Vert[face->vertex + 12];
				controlpoint[5] = data.Vert[face->vertex + 13];

				controlpoint[6] = data.Vert[face->vertex + 20];
				controlpoint[7] = data.Vert[face->vertex + 21];
				controlpoint[8] = data.Vert[face->vertex + 22];

				tessellate(mesh_level, controlpoint, &mesh_vertex_array[mesh_index], mesh_num_verts[mesh_index], &mesh_index_array[mesh_index], mesh_num_indexes[mesh_index]);
//				mesh_vao[mesh_index] = gfx.CreateVertexArrayObject();
				mesh_vertex_vbo[mesh_index] = gfx.CreateVertexBuffer(mesh_vertex_array[mesh_index], mesh_num_verts[mesh_index]);
				mesh_index_vbo[mesh_index] = gfx.CreateIndexBuffer(mesh_index_array[mesh_index], mesh_num_indexes[mesh_index]);
				mesh_index++;


				controlpoint[0] = data.Vert[face->vertex + 4];
				controlpoint[1] = data.Vert[face->vertex + 5];
				controlpoint[2] = data.Vert[face->vertex + 6];

				controlpoint[3] = data.Vert[face->vertex + 13];
				controlpoint[4] = data.Vert[face->vertex + 14];
				controlpoint[5] = data.Vert[face->vertex + 15];

				controlpoint[6] = data.Vert[face->vertex + 22];
				controlpoint[7] = data.Vert[face->vertex + 23];
				controlpoint[8] = data.Vert[face->vertex + 24];

				tessellate(mesh_level, controlpoint, &mesh_vertex_array[mesh_index], mesh_num_verts[mesh_index], &mesh_index_array[mesh_index], mesh_num_indexes[mesh_index]);
//				mesh_vao[mesh_index] = gfx.CreateVertexArrayObject();
				mesh_vertex_vbo[mesh_index] = gfx.CreateVertexBuffer(mesh_vertex_array[mesh_index], mesh_num_verts[mesh_index]);
				mesh_index_vbo[mesh_index] = gfx.CreateIndexBuffer(mesh_index_array[mesh_index], mesh_num_indexes[mesh_index]);
				mesh_index++;


				controlpoint[0] = data.Vert[face->vertex + 6];
				controlpoint[1] = data.Vert[face->vertex + 7];
				controlpoint[2] = data.Vert[face->vertex + 8];

				controlpoint[3] = data.Vert[face->vertex + 15];
				controlpoint[4] = data.Vert[face->vertex + 16];
				controlpoint[5] = data.Vert[face->vertex + 17];

				controlpoint[6] = data.Vert[face->vertex + 24];
				controlpoint[7] = data.Vert[face->vertex + 25];
				controlpoint[8] = data.Vert[face->vertex + 26];

				tessellate(mesh_level, controlpoint, &mesh_vertex_array[mesh_index], mesh_num_verts[mesh_index], &mesh_index_array[mesh_index], mesh_num_indexes[mesh_index]);
//				mesh_vao[mesh_index] = gfx.CreateVertexArrayObject();
				mesh_vertex_vbo[mesh_index] = gfx.CreateVertexBuffer(mesh_vertex_array[mesh_index], mesh_num_verts[mesh_index]);
				mesh_index_vbo[mesh_index] = gfx.CreateIndexBuffer(mesh_index_array[mesh_index], mesh_num_indexes[mesh_index]);
				mesh_index++;
			}
		}
	}

	vertex = new vertex_t [data.num_verts];
	CreateTangentArray(vertex, data.Vert, data.num_verts, tangent);

	map_vertex_vbo = gfx.CreateVertexBuffer(vertex, data.num_verts);
	map_index_vbo = gfx.CreateIndexBuffer(data.IndexArray, data.num_index);
}

/*
	We need to stick tangent vector into old bsp datatype
*/
void Bsp::CreateTangentArray(vertex_t *vertex, bspvertex_t *bsp_vertex, int num_vert, vec4 *tangent)
{
	for(int i = 0; i < num_vert; i++)
	{
		vertex[i].color = bsp_vertex[i].color;
		vertex[i].normal = bsp_vertex[i].normal;
		vertex[i].position = bsp_vertex[i].position;
		vertex[i].texCoord0 = bsp_vertex[i].texCoord0;
		vertex[i].texCoord1 = bsp_vertex[i].texCoord1;
		vertex[i].tangent = tangent[i];
	}
}

/*
	Converts axis from quake3 to opengl format
*/
void Bsp::change_axis()
{
	for(int i = 0; i < data.num_verts; i++)
	{
		bspvertex_t *vert = &data.Vert[i];
		float temp;

		temp = vert->position.y;
		vert->position.y = vert->position.z;
		vert->position.z =  -temp;

		temp = vert->normal.y;
		vert->normal.y =  vert->normal.z;
		vert->normal.z =  -temp;
		vert->texCoord0.y = -vert->texCoord0.y;
		vert->texCoord1.y = -vert->texCoord1.y;


//		vert->vPosition *= (1.0f / UNITS_TO_METERS);
	}

	for(int i = 0; i < data.num_planes; i++)
	{
		plane_t *plane = &data.Plane[i];
		float	temp;

		temp = plane->normal.y;
		plane->normal.y = plane->normal.z;
		plane->normal.z =  -temp;

//		plane->d *= (1.0f / UNITS_TO_METERS);
	}

	for(int i = 0; i < data.num_leafs; i++)
	{
		int temp = data.Leaf[i].min[1];
		data.Leaf[i].min[1] = data.Leaf[i].min[2];
		data.Leaf[i].min[2] = -temp;
	}

	for(int i = 0; i < data.num_nodes; i++)
	{
		int temp = data.Node[i].min[1];
		data.Node[i].min[1] = data.Node[i].min[2];
		data.Node[i].min[2] = -temp;
	}
}

/*
	Returns entity string for parsing
*/
const char *Bsp::get_entities()
{
	return (const char *)data.Ent;
}

/*
	Should cleanly unload a map
*/
void Bsp::unload(Graphics &gfx)
{
	int mesh_index = 0;

	delete vertex;
	for(int i = 0; i < data.num_faces; i++)
	{
		face_t *face = &data.Face[i];

		if (face->type == 2)
		{
			if (face->patchHeight == 3 && face->patchWidth == 3)
			{
				delete [] mesh_vertex_array[mesh_index];
				delete [] mesh_index_array[mesh_index];
				mesh_index++;
			}
			else if (face->patchWidth == 5)
			{
				delete [] mesh_vertex_array[mesh_index];
				delete [] mesh_index_array[mesh_index];
				mesh_index++;
				delete [] mesh_vertex_array[mesh_index];
				delete [] mesh_index_array[mesh_index];
				mesh_index++;
			}
			else if (face->patchWidth == 9)
			{
				delete [] mesh_vertex_array[mesh_index];
				delete [] mesh_index_array[mesh_index];
				mesh_index++;
				delete [] mesh_vertex_array[mesh_index];
				delete [] mesh_index_array[mesh_index];
				mesh_index++;
				delete [] mesh_vertex_array[mesh_index];
				delete [] mesh_index_array[mesh_index];
				mesh_index++;
				delete [] mesh_vertex_array[mesh_index];
				delete [] mesh_index_array[mesh_index];
				mesh_index++;
			}
		}
	}

	delete [] mesh_index2face;
	delete [] mesh_vertex_array;
	delete [] mesh_index_array;
	delete [] mesh_num_verts;
	delete [] mesh_num_indexes;

	mesh_index2face = NULL;
	mesh_vertex_array = NULL;
	mesh_index_array = NULL;
	mesh_num_verts = NULL;
	mesh_num_indexes = NULL;
	gfx.DeleteIndexBuffer(map_index_vbo);
	map_index_vbo = 0;
	gfx.DeleteVertexBuffer(map_vertex_vbo);
	map_vertex_vbo = 0;
	mesh_level = 0;

	for(int i = 0; i < num_meshes; i++)
	{
#ifndef DIRECTX
		gfx.DeleteVertexArrayObject(mesh_vao[i]);
#endif
		gfx.DeleteIndexBuffer(mesh_index_vbo[i]);
		gfx.DeleteVertexBuffer(mesh_vertex_vbo[i]);
	}
	delete [] mesh_vao;
	delete [] mesh_index_vbo;
	delete [] mesh_vertex_vbo;
	num_meshes = 0;
	mesh_index_vbo = NULL;
	mesh_vertex_vbo = NULL;

	//Todo, try to find a way to keep loaded textures between map loads
	for(int i = 0; i < data.num_materials; i++)
	{
		gfx.DeleteTexture(tex_object[i]);
		gfx.DeleteTexture(normal_object[i]);
	}
	delete [] tex_object;
	delete [] normal_object;
	tex_object = NULL;
	normal_object = NULL;

	for(int i = 0; i < data.num_lightmaps; i++)
	{
		gfx.DeleteTexture(lightmap_object[i]);
	}
	delete [] lightmap_object;
	lightmap_object = NULL;

	// raw lump file structure
	delete [] tBsp;
	tBsp = NULL;

	// high level data pointers into lump
	memset((void *)&data, NULL, sizeof(bspData_t));

	delete [] tangent;

	loaded = false;
}

/*
	Given a position return the bsp leaf node containing that point
*/
inline int Bsp::find_leaf(const vec3 &position)
{
	float	distance;
	int		i = 0;

	while (i >= 0)
	{
		node_t *node = &data.Node[i];
		plane_t *plane = &data.Plane[node->plane];

		distance = (plane->normal * position) - plane->d;

		if (distance >= 0)	
			i = data.Node[i].front;
		else
			i = data.Node[i].back;
	}
	return -(i + 1);
}

void Bsp::sort_leaf(vector<int> *leaf_list, int node_index, const vec3 &position)
{
	if (node_index < 0)
	{
		leaf_list->push_back( -(node_index + 1) );
		return;
	}

	node_t *node = &data.Node[node_index];
	plane_t *plane = &data.Plane[node->plane];

	float	distance = (plane->normal * position) - plane->d;
	if (distance >= 0)
	{
		sort_leaf(leaf_list, data.Node[node_index].front, position);
		sort_leaf(leaf_list, data.Node[node_index].back, position);
	}
	else
	{
		sort_leaf(leaf_list, data.Node[node_index].back, position);
		sort_leaf(leaf_list, data.Node[node_index].front, position);
	}
}

bool Bsp::collision_detect(vec3 &point, plane_t *plane, float *depth)
{
	int leaf_index = find_leaf(point);
	leaf_t *leaf = &data.Leaf[leaf_index];
	float max_depth = 2048.0f;

	for (int i = 0; i < leaf->num_brushes; i++)
	{
		int *index = &data.LeafBrush[leaf->leaf_brush + i];
		brush_t	*brush = &data.Brushes[*index];
		int brush_index = brush->first_side;
		int num_sides = brush->num_sides;

		if ((data.Material[brush->material].contents & CONTENTS_SOLID) == 0)
			continue;

		if (strcmp(data.Material[brush->material].name, "textures/common/weapclip") == 0)
			continue;


		for( int j = 0; j < num_sides; j++)
		{
			brushSide_t *brushSide = &data.BrushSides[brush_index + j];
			int plane_index = brushSide->plane;

			float d = point * data.Plane[plane_index].normal + data.Plane[plane_index].d ;

			if (d > 0.0f)
				continue;

			if (d <= max_depth)
			{
				plane->normal = data.Plane[plane_index].normal;
				plane->d = data.Plane[plane_index].d;
				max_depth = d;
			}
		}
	}
	*depth = max_depth;

	if (max_depth != 2048.0f)
		return true;
	else
		return false;
}


inline void Bsp::render_face(face_t *face, Graphics &gfx)
{
	gfx.SelectVertexBuffer(map_vertex_vbo);
	gfx.SelectIndexBuffer(map_index_vbo);

/*
	if (data.Texture[face->textureID].contents == CONTENTS_FOG)
	{
		//select fog shader
	}
*/

	gfx.SelectTexture(0, tex_object[face->material]);
	// surfaces that arent lit with lightmaps eg: skies
	if (face->lightmap != -1)
		gfx.SelectTexture(1, lightmap_object[face->lightmap]);
	gfx.SelectTexture(2, normal_object[face->material]);
	gfx.DrawArray(PRIM_TRIANGLES, face->index, face->vertex, face->num_index, face->num_verts);
	gfx.DeselectTexture(2);
	gfx.DeselectTexture(1);
	gfx.DeselectTexture(0);
	gfx.SelectVertexBuffer(0);
	gfx.SelectIndexBuffer(0);
}

inline void Bsp::render_patch(face_t *face, Graphics &gfx)
{
	int mesh_index = -1;
	int index_per_row = 2 * (mesh_level + 1);

	// Find pre-generated vertex data for patch O(n)
	for( int i = 0; i < num_meshes; i++)
	{
		if (mesh_index2face[i] == face->vertex)
		{
			mesh_index = i;
			break;
		}
	}

	for(int i = 0; i < 4; i++)
	{
		gfx.SelectVertexBuffer(mesh_vertex_vbo[mesh_index + i]);
		gfx.SelectIndexBuffer(mesh_index_vbo[mesh_index + i]);

		// Render each row
		gfx.SelectTexture(0, tex_object[face->material]);
		gfx.SelectTexture(1, lightmap_object[face->lightmap]);
		gfx.SelectTexture(2, normal_object[face->material]);
		for( int row = 0; row < mesh_level; row++)
		{
			gfx.DrawArray(PRIM_TRIANGLE_STRIP,
				row * index_per_row, 0,
				index_per_row, mesh_num_verts[mesh_index + i]);
		}
		gfx.DeselectTexture(2);
		gfx.DeselectTexture(1);
		gfx.DeselectTexture(0);


		if (face->patchWidth == 5 && i == 1)
			break;

		if (face->patchWidth == 3)
			break;
	}
}

inline void Bsp::render_billboard(face_t *face, Graphics &gfx)
{
	gfx.SelectTexture(0, tex_object[face->material]);
	gfx.SelectTexture(1, normal_object[face->material]);
	gfx.SelectIndexBuffer(Model::quad_index);
	gfx.SelectVertexBuffer(Model::quad_vertex);
	gfx.DrawArray(PRIM_TRIANGLES, 0, 0, 6, 4);
	gfx.SelectVertexBuffer(0);
	gfx.SelectIndexBuffer(0);
	gfx.DeselectTexture(1);
	gfx.DeselectTexture(0);
}

void Bsp::render(vec3 &position, Plane *frustum, Graphics &gfx)
{
	int frameIndex = find_leaf(position);

	leaf_t *frameLeaf = &data.Leaf[frameIndex];

	blend_list.clear();
	face_list.clear();

	// walk bsp tree, sort leafs front to back
	vector<int> leaf_list;
	sort_leaf(&leaf_list, 0, position);

	// loop through all leaves, checking if leaf visible from current leaf
	for (unsigned int i = 0; i < leaf_list.size(); i++)
	{
		leaf_t *leaf = &data.Leaf[leaf_list[i]];

		if ( cluster_visible(frameLeaf->cluster, leaf->cluster) == false )
			continue;

		if ( leaf_visible(leaf, frustum) == false)
			continue;

		// generate face lists
		for (int j = 0; j < leaf->num_faces; j++)
		{
			int face_index = data.LeafFace[leaf->leaf_face + j];
			face_t *face = &data.Face[face_index];

			// need a way to tell if a face should be blended
			if (tex_object[face->material] < 0)
				blend_list.push_back(face_index);
			else
				face_list.push_back(face_index);
		}
	}
	leaf_list.clear();

	for (unsigned int i = 0; i < face_list.size(); i++)
	{
		face_t *face = &data.Face[face_list[i]];

		if (face->type == 1 || face->type == 3)
		{
			render_face(face, gfx);
		}
		else if (face->type == 2)
		{
			render_patch(face, gfx);
		}
		else// (face->type == 4)
		{
			render_billboard(face, gfx);
		}
	}

	gfx.Blend(true);
//	gfx.Depth(false);
	// leaves sorted front to back, render blends back to front
	for (int i = blend_list.size() - 1; i >= 0; i--)
	{
		face_t *face = &data.Face[blend_list[i]];

		if (face->type == 1 || face->type == 3)
		{
			render_face(face, gfx);
		}
		else if (face->type == 2)
		{
			render_patch(face, gfx);
		}
		else// (face->type == 4)
		{
			render_billboard(face, gfx);
		}
	}
//	gfx.Depth(true);
	gfx.Blend(false);
//	draw_box(frameLeaf->mins, frameLeaf->maxs);
}

/*
	Determines set of visible leafs from current leaf
*/
inline int Bsp::cluster_visible(int vis_cluster, int test_cluster)
{
	int byte_offset, bit_offset;
	char test_byte;

	// if no vis data or if camera leaf invalid (outside map) draw everything
	if (vis_cluster < 0 || data.num_vis == 0)
		return 1;

	// bit of pVecs we need to return
	bit_offset = vis_cluster + test_cluster * data.VisData->vector_size * 8; 
	byte_offset = bit_offset / 8;
	test_byte = 1 << (bit_offset - byte_offset * 8);
	return 	(&data.VisData->pVecs)[byte_offset] & test_byte;
}

/*
	Needs frustum in world space
*/
bool Bsp::leaf_visible(leaf_t *leaf, Plane *frustum)
{
	vec3 max((float)leaf->max[0], (float)leaf->max[1], (float)leaf->max[2]);
	vec3 min((float)leaf->min[0], (float)leaf->min[1], (float)leaf->min[2]);

	if (frustum == NULL)
		return true;

	for(int i = 0; i < 5; i++)
	{
		if ( frustum[i].normal * max + frustum[i].d < 0 )
		{
			if ( frustum[i].normal * min + frustum[i].d < 0 )
			{
				return false;
			}
		}
	}
	return true;
}

void Bsp::load_textures(Graphics &gfx)
{
	for (int i = 0; i < data.num_lightmaps; i++)
	{
#ifndef DIRECTX
		lightmap_object[i] = gfx.LoadTexture(128, 128, 3, GL_RGB, (void *)data.LightMaps[i].image);
#else
		byte *pBits = tga_24to32(128, 128, (byte *)data.LightMaps[i].image);
		lightmap_object[i] = gfx.LoadTexture(128, 128, 4, 4, (void *)data.LightMaps[i].image);
		delete [] pBits;
#endif
	}

	for (int i = 0; i < data.num_materials; i++)
	{
		material_t	*material = &data.Material[i];
		char		buffer[LINE_SIZE];

		snprintf(buffer, LINE_SIZE, "media/%s.tga", material->name);
		tex_object[i] = load_texture(gfx, buffer);
		snprintf(buffer, LINE_SIZE, "media/%s_normal.tga", material->name);
		normal_object[i] = load_texture(gfx, buffer);
	}
}

/*
	cylindrical patches are a 3x9 set of control points
	U patches are a 3x5 set of control points
	This function assumes it's given 3x3 set of control points
	hacky fix for cylindrical patches and U patches in calling function
*/
void Bsp::tessellate(int level, bspvertex_t control[], vertex_t **vertex_array, int &num_verts, int **index_array, int &num_indexes)
{
	vec3 a, b;
	int i, j;

	num_verts = level + 1;

	*vertex_array = new vertex_t[num_verts * num_verts];

	// calculate first set of verts
	for (i = 0; i <= level; i++)
	{
		float a = (float) i / level;
		float b = 1.0f - a;
		(*vertex_array)[i].position =
			control[0].position * (b * b) +
			control[3].position * (2 * b * a) +
			control[6].position * (a * a);
	}

	// calculate rest of verts
	for ( i = 1; i <= level; i++)
	{
		float a = (float)i / level;
		float b = 1.0f - a;

		vertex_t temp[3];

		temp[0].position = 
			control[0].position * (b * b) + 
			control[1].position * (2 * b * a) + 
			control[2].position * (a * a);
		temp[1].position = 
			control[3].position * (b * b) + 
			control[4].position * (2 * b * a) + 
			control[5].position * (a * a);
		temp[2].position = 
			control[6].position * (b * b) + 
			control[7].position * (2 * b * a) + 
			control[8].position * (a * a);

		for(j = 0; j <= level; j++)
		{
			float a = (float) j / level;
			float b = 1.0f - a;

			(*vertex_array)[i * num_verts + j].position =
				temp[0].position * (b * b) +
				temp[1].position * (2 * b * a) +
				temp[2].position * (a * a);
		}
	}
	
	//Create index array
	num_indexes = level * num_verts * 2;
	*index_array = new int[num_indexes];
	for (i = 0; i < level; i++)
	{
		for(j = 0; j <= level; j++)
		{
			(*index_array)[(i * num_verts + j) * 2 + 1] = i * num_verts + j;
			(*index_array)[(i * num_verts + j) * 2] = (i + 1) * num_verts + j;
		}
	}

	// Generate normals and tangent
	for(i = 0; i <= level; i++)
	{
		for(j = 0; j <= level; j++)
		{
			if (j != level)
				a = (*vertex_array)[i * num_verts + j].position - (*vertex_array)[i * num_verts + (j + 1)].position;
			else
				a = (*vertex_array)[i * num_verts + j].position - (*vertex_array)[i * num_verts + (j - 1)].position;
				
			if ( i != level)
				b = (*vertex_array)[i * num_verts + j].position - (*vertex_array)[(i + 1) * num_verts + j].position;
			else
				b = (*vertex_array)[i * num_verts + j].position - (*vertex_array)[(i - 1) * num_verts + j].position;

			(*vertex_array)[i * num_verts + j].color = -1;
			(*vertex_array)[i * num_verts + j].texCoord0 = vec2((float)(i % 2), (float)(j % 2));
			(*vertex_array)[i * num_verts + j].texCoord0 *= (1.0f/8.0f);
			(*vertex_array)[i * num_verts + j].tangent.x = a.x;
			(*vertex_array)[i * num_verts + j].tangent.y = a.y;
			(*vertex_array)[i * num_verts + j].tangent.z = a.z;
			(*vertex_array)[i * num_verts + j].tangent.w = 0.0f;

			if (j == level || i == level)
			{
				(*vertex_array)[i * num_verts + j].normal = vec3::crossproduct(a,b).normalize();
			}
			else
			{
				(*vertex_array)[i * num_verts + j].normal = vec3::crossproduct(b,a).normalize();
				(*vertex_array)[i * num_verts + j].tangent.x = b.x;
				(*vertex_array)[i * num_verts + j].tangent.y = b.y;
				(*vertex_array)[i * num_verts + j].tangent.z = b.z;
				(*vertex_array)[i * num_verts + j].tangent.w = 0.0f;
			}

			if (j == level && i == level)
			{
				(*vertex_array)[i * num_verts + j].normal = vec3::crossproduct(b,a).normalize();
				(*vertex_array)[i * num_verts + j].tangent.x = b.x;
				(*vertex_array)[i * num_verts + j].tangent.y = b.y;
				(*vertex_array)[i * num_verts + j].tangent.z = b.z;
				(*vertex_array)[i * num_verts + j].tangent.w = 0.0f;
			}
		}
	}

	// correct numVerts size
	num_verts = num_verts * num_verts;
}

bool Bsp::vis_test(vec3 &x, vec3 &y)
{
		int a = find_leaf(x);
		int b = find_leaf(y);

		if (a == b)
			return true;

		leaf_t *a_leaf = &data.Leaf[a];
		leaf_t *b_leaf = &data.Leaf[b];

		if ( !cluster_visible(a_leaf->cluster, b_leaf->cluster) )
			return false;
		else
			return true;
}

bool Bsp::leaf_test(vec3 &x, vec3 &y)
{
		int a = find_leaf(x);
		int b = find_leaf(y);

		if (a == b)
			return true;
		else
			return false;
}

/*
	Can be done in a geometry shader
*/
void Bsp::CalculateTangentArray(bspvertex_t *vertex, int num_vert, int *index, int num_index, vec4 *tangent)
{
	vec3 *temp_tan = new vec3 [num_vert];
	vec3 *temp_btan = new vec3 [num_vert];

	memset(temp_tan, 0, num_vert * sizeof(vec3));
	memset(temp_btan, 0, num_vert * sizeof(vec3));
	for (int i = 0; i < num_index - 2;)
	{
		int a = index[i];
		int b = index[i + 1];
		int c = index[i + 2];
        
		const vec3 p0 = vertex[a].position;
		const vec3 p1 = vertex[b].position;
		const vec3 p2 = vertex[c].position;

		const vec2 tex0 = vertex[a].texCoord0;
		const vec2 tex1 = vertex[b].texCoord0;
		const vec2 tex2 = vertex[c].texCoord0;

		vec3 q1 = p1 - p0;
		vec3 q2 = p2 - p0;

		float s1 = tex1.x - tex0.x;
		float t1 = tex1.y - tex0.y;
		float s2 = tex2.x - tex0.x;
		float t2 = tex2.y - tex0.y;
        
		float denom = s1 * t2 - s2 * t1;

		// singular triangle
		if (denom == 0)
		{
			vec3 f(1.0f, 0.0f, 0.0f);
			vec3 r = vec3::crossproduct(f, vertex[a].normal);
			f = vec3::crossproduct(r, vertex[a].normal);

			temp_tan[a] += f;
			temp_tan[b] += f;
			temp_tan[c] += f;
        
			temp_btan[a] += r;
			temp_btan[b] += r;
			temp_btan[c] += r;
			i += 3;
			continue;
		}

		float r = 1.0f / denom;

		vec3 sdir((t2 * q1.x - t1 * q2.x) * r, (t2 * q1.y - t1 * q2.y) * r, (t2 * q1.z - t1 * q2.z) * r);
		vec3 tdir((s1 * q2.x - s2 * q1.x) * r, (s1 * q2.y - s2 * q1.y) * r, (s1 * q2.z - s2 * q1.z) * r);
		sdir.normalize();
		tdir.normalize();
        
		// adding so vectors are averaged for shared verticies
		temp_tan[a] += sdir;
		temp_tan[b] += sdir;
		temp_tan[c] += sdir;
        
		temp_btan[a] += tdir;
		temp_btan[b] += tdir;
		temp_btan[c] += tdir;
        
		i += 3;
//		vec3 n = vec3::crossproduct(q2,q1).normalize();
//		printf("n = %f %f %f   = %f %f %f\n", n.x, n.y, n.z, vertex[a].normal.x, vertex[a].normal.y, vertex[a].normal.z);
//		printf("t = %f %f %f b = %f %f %f\n", sdir.x, sdir.y, sdir.z, tdir.x, tdir.y, tdir.z);
	}
    
	for (int i = 0; i < num_vert; i++)
	{
		vec3 n = vertex[i].normal;
		vec3 t = temp_tan[i];
		vec3 b = temp_btan[i];
		vec3 vtan;
		vec3 vbtan;
        
		// Gram-Schmidt orthogonalize
		vtan = (t - n * (n * t)).normalize();
		vbtan = (b - n * (n * b)) - vtan * (vtan * b);
        
		// Calculate handedness
		tangent[i].x = vtan.x;
		tangent[i].y = vtan.y;
		tangent[i].z = vtan.z;
		tangent[i].w = (vec3::crossproduct(n, t) * temp_btan[i] < 0.0f) ? -1.0f : 1.0f;
	}

	delete[] temp_tan;
	delete[] temp_btan;
}

void Bsp::draw_box(int *min, int *max)
{
#ifdef OPENGL_OLD
	glEnable(GL_BLEND);
	glColor3f(1.0f, 0.0f, 0.0f);
	glLineWidth(5.0f);
	glBegin(GL_TRIANGLES);
	glVertex3i(min[0], min[1], min[2]); // 0
	glVertex3i(min[0], min[1], max[2]); // 1
	glVertex3i(min[0], max[1], min[2]); // 2

	glVertex3i(min[0], min[1], max[2]); // 1
	glVertex3i(min[0], max[1], max[2]); // 3
	glVertex3i(min[0], max[1], min[2]); // 2

	glVertex3i(max[0], max[1], max[2]); // 7
	glVertex3i(min[0], min[1], max[2]); // 1
	glVertex3i(max[0], min[1], max[2]); // 5

	glVertex3i(max[0], max[1], max[2]); // 7
	glVertex3i(min[0], max[1], max[2]); // 3
	glVertex3i(min[0], min[1], max[2]); // 1

	glVertex3i(max[0], min[1], max[2]); // 5
	glVertex3i(max[0], max[1], min[2]); // 6
	glVertex3i(max[0], max[1], max[2]); // 7

	glVertex3i(max[0], min[1], max[2]); // 5
	glVertex3i(max[0], min[1], min[2]); // 4
	glVertex3i(max[0], max[1], min[2]); // 6

	glVertex3i(min[0], min[1], min[2]); // 0
	glVertex3i(min[0], max[1], min[2]); // 2
	glVertex3i(max[0], max[1], min[2]); // 6

	glVertex3i(min[0], min[1], min[2]); // 0
	glVertex3i(max[0], max[1], min[2]); // 6
	glVertex3i(max[0], min[1], min[2]); // 4

	glVertex3i(min[0], min[1], min[2]); // 0
	glVertex3i(max[0], min[1], min[2]); // 4
	glVertex3i(min[0], min[1], max[2]); // 1

	glVertex3i(min[0], min[1], max[2]); // 1
	glVertex3i(max[0], min[1], min[2]); // 4
	glVertex3i(max[0], min[1], max[2]); // 5

	glVertex3i(min[0], max[1], min[2]); // 2
	glVertex3i(min[0], max[1], max[2]); // 3
	glVertex3i(max[0], max[1], min[2]); // 6

	glVertex3i(min[0], max[1], max[2]); // 3
	glVertex3i(max[0], max[1], max[2]); // 7
	glVertex3i(max[0], max[1], min[2]); // 6
	glEnd();
	glDisable(GL_BLEND);
#endif
}

void Bsp::draw_line_box(int *min, int *max)
{
#ifdef OPENGL_OLD
	glColor3f(1.0f, 0.0f, 0.0f);
	glLineWidth(5.0f);
	glBegin(GL_LINES);
	//bottom square
	glVertex3i(min[0], min[1], min[2]);
	glVertex3i(max[0], min[1], min[2]);
	glVertex3i(min[0], min[1], min[2]);
	glVertex3i(min[0], min[1], max[2]);
	glVertex3i(max[0], min[1], max[2]);
	glVertex3i(max[0], min[1], min[2]);
	glVertex3i(max[0], min[1], max[2]);
	glVertex3i(min[0], min[1], max[2]);
	//top square                     
	glVertex3i(min[0], max[1], min[2]);
	glVertex3i(max[0], max[1], min[2]);
	glVertex3i(min[0], max[1], min[2]);
	glVertex3i(min[0], max[1], max[2]);
	glVertex3i(max[0], max[1], max[2]);
	glVertex3i(max[0], max[1], min[2]);
	glVertex3i(max[0], max[1], max[2]);
	glVertex3i(min[0], max[1], max[2]);
	//remaining legs                 
	glVertex3i(min[0], min[1], min[2]);
	glVertex3i(min[0], max[1], min[2]);
	glVertex3i(min[0], max[1], max[2]);
	glVertex3i(min[0], min[1], max[2]);
                                         
	glVertex3i(max[0], min[1], min[2]);
	glVertex3i(max[0], max[1], min[2]);
	glVertex3i(max[0], max[1], max[2]);
	glVertex3i(max[0], min[1], max[2]);
	glEnd();
#endif
}


/*
Intersect ray with aabb planes
get tmin and tmax values for each pair
can compare interval with other pairs and determine
if hit or miss occurs
*/
bool Bsp::RayBoxSlab(vec3 &origin, vec3 &dir, vec3 &min, vec3 &max, float &distance)
{
	float tmin = -10000, tmax = 10000;

	// check for a miss first
	if (origin.x <= min.x || origin.x >= max.x)
	{
		return false;
	}
	else if (origin.y <= min.y || origin.y >= max.y)
	{
		return false;
	}
	else if (origin.z <= min.z || origin.z >= max.z)
	{
		return false;
	}

	// X coordinate
	if (dir.x != 0.0)
	{
		float t1 = (min.x - origin.x) / dir.x;
		float t2 = (max.x - origin.x) / dir.x;

		tmin = MAX(tmin, MIN(t1, t2));
		tmax = MIN(tmax, MAX(t1, t2));
	}

	// Y coordinate
	if (dir.y != 0.0)
	{
		float t1 = (min.y - origin.y) / dir.y;
		float t2 = (max.y - origin.y) / dir.y;

		tmin = MAX(tmin, MIN(t1, t2));
		tmax = MIN(tmax, MAX(t1, t2));
	}

	// Z coordinate
	if (dir.z != 0.0)
	{
		float t1 = (min.z - origin.z) / dir.z;
		float t2 = (max.z - origin.z) / dir.z;

		tmin = MAX(tmin, MIN(t1, t2));
		tmax = MIN(tmax, MAX(t1, t2));
	}


	if (tmax > tmin && tmax > 0.0)
	{
		distance = tmax;
		return true;
	}
	else
	{
		return false;
	}
}


void Bsp::hitscan(vec3 &origin, vec3 &dir, float &distance)
{
	for (int i = 0; i < data.num_leafs; i++)
	{
		vec3 min((float)data.Leaf[i].min[0], (float)data.Leaf[i].min[1], (float)data.Leaf[i].min[2]);
		vec3 max((float)data.Leaf[i].max[0], (float)data.Leaf[i].max[1], (float)data.Leaf[i].max[2]);

		RayBoxSlab(origin, dir, min, max, distance);
	}
}
