#include "include.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Bsp::Bsp()
{
	loaded = false;
	on_ground = false;
	collision = false;
	trace_result = 0.0f;
	enable_textures = false;
	enable_patch = true;
	enable_sky = true;
	enable_shader = true;
	enable_blend = true;
	portal_tex = 0;
	memset(map_name, 0, 64);
	sky_face = -1;
	lastIndex = -2;
	normal_object = NULL;
	lightmap_object = NULL;
	num_meshes = 0;
	normal_object = NULL;
	mesh_level = 8;
	map_vertex_vbo = 0;
	map_index_vbo = 0;
	lightmap_object = NULL;
	face_to_patch = NULL;
	vertex = NULL;
	tBsp = NULL;
	tex_object = NULL;
	tangent = NULL;
	patchdata = NULL;
	selected_map = false;
}


bool Bsp::load(char *map, char **pk3list, int num_pk3)
{
	max_stage = MAX_TEXTURES;
	selected_map = false;
	for (int i = 0; i < num_pk3; i++)
	{
		get_zipfile(pk3list[i], map, (unsigned char **)&tBsp, NULL);
		if (tBsp != NULL)
			break;
	}

	if (tBsp == NULL)
		tBsp = (bsp_t *)get_file(map, NULL);


	if (tBsp == NULL)
	{
		debugf("Unable to open map %s\n", map);
		return false;
	}

	byte *pBsp = (byte *)tBsp;

	// init data
	data.header = (bsp_t *)tBsp;
	data.Vert = (bspvertex_t *)	&pBsp[tBsp->directory[VertexArray].offset];
	data.Ent = (byte *)			&pBsp[tBsp->directory[Entities].offset];
	data.Material = (material_t *)&pBsp[tBsp->directory[Materials].offset];
	data.Plane = (plane_t *)	&pBsp[tBsp->directory[Planes].offset];
	data.Node = (node_t *)		&pBsp[tBsp->directory[Nodes].offset];
	data.Leaf = (leaf_t *)		&pBsp[tBsp->directory[Leafs].offset];
	data.LeafFace = (int *)		&pBsp[tBsp->directory[LeafFaces].offset];
	data.LeafBrush = (int *)	&pBsp[tBsp->directory[LeafBrushes].offset];
	data.Model = (model_t *)&pBsp[tBsp->directory[Models].offset];
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
	data.num_model = tBsp->directory[Models].length / sizeof(model_t);
	data.num_brushes = tBsp->directory[Brushes].length / sizeof(brush_t);
	data.num_BrushSides = tBsp->directory[BrushSides].length / sizeof(brushSide_t);
	data.num_faces = tBsp->directory[Faces].length / sizeof(face_t);
	data.num_vis = tBsp->directory[VisData].length / sizeof(visData_t);
	data.num_index = tBsp->directory[IndexArray].length / sizeof(int);
	data.num_lightmaps = tBsp->directory[LightMaps].length / sizeof(lightmap_t);
	data.num_fog = tBsp->directory[Fog].length / sizeof(fog_t);

	change_axis();

	face_to_patch = new int[data.num_faces];
	for (unsigned int i = 0; i < data.num_faces; i++)
	{
		face_to_patch[i] = -1;
	}

	tangent = new vec4 [data.num_verts];
	memset(tangent, 0, sizeof(vec4) * data.num_verts);
	//CalculateTangentArray(data.Vert, data.num_verts, data.IndexArray, data.num_index, tangent);

	tex_object = new texture_t [data.num_materials];

	memset(tex_object, 0, sizeof(texture_t) * data.num_materials);
	normal_object = new int [data.num_materials];

	if (data.num_lightmaps == 0 && data.header->version == 0x2F)
	{
		// assuming 32 is enough for wolfenstein maps
		lightmap_object = new int[32];
	}
	else
	{
		lightmap_object = new int[data.num_lightmaps];
	}


	for (unsigned int i = 0; i < data.num_materials; i++)
	{
		for (int j = 0; j < MAX_TEXTURES; j++)
		{
			tex_object[i].texObj[j] = (unsigned int)0; // zero deselects a texture
			tex_object[i].texObjAnim[j] = 0;
		}
		tex_object[i].num_anim = 0;
		tex_object[i].num_tex = 0;
		tex_object[i].index = (unsigned int)-1;
//		tex_object[i].stage = (unsigned int)-1;
	}
	for(unsigned int i = 0; i < data.num_lightmaps; i++)
		lightmap_object[i] = (unsigned int)-1;
	for(unsigned int i = 0; i < data.num_materials; i++)
		normal_object[i] = (unsigned int)-1;

	loaded = true;
	memcpy(map_name, map, strlen(map) + 1);
	return true;
}

void get_control_points(bspvertex_t *cp, const bspvertex_t *data, int set, int width, int height)
{
	int x = 0;
	int y = 0;

	int x_shift = (1 + (width - 3) / 2);
	int y_shift = (1 + (height - 3) / 2);


	for (y = 0; y < y_shift; y++)
	{
		for (x = 0; x < x_shift; x += 1)
		{
			const bspvertex_t *box = &data[x + y * width];

			cp[0] = box[x + 0 + (y + 0) * width];
			cp[1] = box[x + 1 + (y + 0) * width];
			cp[2] = box[x + 2 + (y + 0) * width];
			cp[3] = box[x + 0 + (y + 1) * width];
			cp[4] = box[x + 1 + (y + 1) * width];
			cp[5] = box[x + 2 + (y + 1) * width];
			cp[6] = box[x + 0 + (y + 2) * width];
			cp[7] = box[x + 1 + (y + 2) * width];
			cp[8] = box[x + 2 + (y + 2) * width];
			set--;

			if (set <= 0)
				return;
		}
		x = 0;
	}
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

	// Find number  of 3x3 patches
	printf("quadratic_bezier_surface dimensions for %s: ", map_name);
	for (unsigned int i = 0; i < data.num_faces; i++)
	{
		face_t *face = &data.Face[i];

		if (face->type == 2)
		{
			int num_patch = (1 + (face->patchWidth - 3) / 2) * (1 + (face->patchHeight - 3) / 2);
			num_meshes += num_patch; // number of 3x3 meshes
			printf("%dx%d, ", face->patchWidth, face->patchHeight);
		}
	}
	printf("\n");

	patchdata = new patch_t[num_meshes];


	// Generate buffer objects for 3x3 patches given NxN control points
	for (unsigned int i = 0; i < data.num_faces; i++)
	{
		face_t *face = &data.Face[i];
		bspvertex_t controlpoint[9];

		if (face->type == 2)
		{
			int num_patch = (1 + (face->patchWidth - 3) / 2) * (1 + (face->patchHeight - 3) / 2);

			for (int j = 0; j < num_patch; j++)
			{
				get_control_points(controlpoint, &data.Vert[face->vertex], j + 1, face->patchWidth, face->patchHeight);

				patchdata[mesh_index].num_mesh = num_patch;
				patchdata[mesh_index].facevert = face->vertex;

				vec2 lightmap(face->lightmapX / 128.0f, face->lightmapY / 128.0f);
				vec2 size(face->lightmapWidth / 128.0f, face->lightmapHeight / 128.0f);

				//tessellate_quadratic_bezier_surface(control, patchdata[mesh_index].vertex_array, patchdata[mesh_index].index_array, patchdata[mesh_index].num_verts, patchdata[mesh_index].num_indexes, mesh_level);
				tessellate(mesh_level, controlpoint, &patchdata[mesh_index].vertex_array, patchdata[mesh_index].num_verts,
					&patchdata[mesh_index].index_array, patchdata[mesh_index].num_indexes,
					data.Vert[face->vertex].texCoord0,
					lightmap,
					size);
				patchdata[mesh_index].vbo = gfx.CreateVertexBuffer(patchdata[mesh_index].vertex_array, patchdata[mesh_index].num_verts);
				patchdata[mesh_index].ibo = gfx.CreateIndexBuffer(patchdata[mesh_index].index_array, patchdata[mesh_index].num_indexes);
				delete[] patchdata[mesh_index].vertex_array;
				delete[] patchdata[mesh_index].index_array;
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
void Bsp::CreateTangentArray(vertex_t *vertex_out, bspvertex_t *bsp_vertex, int num_vert, vec4 *tangent_in)
{
	for(int i = 0; i < num_vert; i++)
	{

		vertex_out[i].position = bsp_vertex[i].position;
		vertex_out[i].texCoord0 = bsp_vertex[i].texCoord0;
		vertex_out[i].texCoord1 = bsp_vertex[i].texCoord1;
		vertex_out[i].normal = bsp_vertex[i].normal;
		vertex_out[i].color = bsp_vertex[i].color;
		vertex_out[i].tangent = tangent_in[i];
	}
}

/*
	Converts axis from quake3 to opengl format
*/
void Bsp::change_axis()
{
	quake1 = false;
	if (strstr((char *)data.Ent, "\"wad\""))
	{
		quake1 = true;
	}


	for(unsigned int i = 0; i < data.num_verts; i++)
	{
		bspvertex_t *vert = &data.Vert[i];
		float temp;

		temp = vert->position.y;
		vert->position.y = vert->position.z;
		vert->position.z =  -temp;

		temp = vert->normal.y;
		vert->normal.y =  vert->normal.z;
		vert->normal.z =  -temp;

		if (quake1)
		{
			// some quake1 textures misscaled, need to find out why
//				vert->texCoord0.x *= 0.5f;
//				vert->texCoord0.y *= 0.5f;
		}

		vert->texCoord0.y = -vert->texCoord0.y;
//		vert->texCoord1.y = vert->texCoord1.y;


//		vert->vPosition *= (1.0f / UNITS_TO_METERS);
	}


//	data.Plane[plane_index].normal
	for(unsigned int i = 0; i < data.num_planes; i++)
	{
		plane_t *plane = &data.Plane[i];
		float	temp;

		temp = plane->normal.y;
		plane->normal.y = plane->normal.z;
		plane->normal.z =  -temp;

//		plane->d *= (1.0f / UNITS_TO_METERS);
	}

	for(unsigned int i = 0; i < data.num_leafs; i++)
	{
		int temp = data.Leaf[i].min[1];
		data.Leaf[i].min[1] = data.Leaf[i].min[2];
		data.Leaf[i].min[2] = -temp;

		temp = data.Leaf[i].max[1];
		data.Leaf[i].max[1] = data.Leaf[i].max[2];
		data.Leaf[i].max[2] = -temp;
	}

	for(unsigned int i = 0; i < data.num_nodes; i++)
	{
		int temp = data.Node[i].min[1];
		data.Node[i].min[1] = data.Node[i].min[2];
		data.Node[i].min[2] = -temp;

		temp = data.Node[i].max[1];
		data.Node[i].max[1] = data.Node[i].max[2];
		data.Node[i].max[2] = -temp;
	}

	for (unsigned int i = 0; i < data.num_model; i++)
	{
		float temp = data.Model[i].min[1];
		data.Model[i].min[1] = data.Model[i].min[2];
		data.Model[i].min[2] = -temp;

		temp = data.Model[i].max[1];
		data.Model[i].max[1] = data.Model[i].max[2];
		data.Model[i].max[2] = -temp;
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
	//int mesh_index = 0;

	loaded = false;
	enable_textures = false;

	anim_list.resize(0);

	delete[] face_to_patch;

	if (vertex != NULL)
	{
		delete [] vertex;
		vertex = NULL;
	}


	delete[] patchdata;


	gfx.DeleteIndexBuffer(map_index_vbo);
	map_index_vbo = 0;
	gfx.DeleteVertexBuffer(map_vertex_vbo);
	map_vertex_vbo = 0;
	mesh_level = 0;
	num_meshes = 0;

	//Todo, try to find a way to keep loaded textures between map loads
	for(unsigned int i = 0; i < data.num_materials; i++)
	{
		for(int j = 0; j < tex_object->num_tex; j++)
		{
			if ( tex_object[i].texObj[j] )
			{
				gfx.DeleteTexture(tex_object[i].texObj[j]);
				tex_object[i].texObj[j] = 0;
			}
		}
		tex_object[i].index = -1;
		

		if (normal_object[i] != -1)
		{
			gfx.DeleteTexture(normal_object[i]);
			normal_object[i] = -1;
		}
	}
	delete [] tex_object;
	delete [] normal_object;
	tex_object = NULL;
	normal_object = NULL;

	for(unsigned int i = 0; i < data.num_lightmaps; i++)
	{
		if (lightmap_object[i] != -1)
		{
			gfx.DeleteTexture(lightmap_object[i]);
			lightmap_object[i] = -1;
		}
	}
	delete [] lightmap_object;
	lightmap_object = NULL;

	// raw lump file structure
	delete [] tBsp;
	tBsp = NULL;

	// high level data pointers into lump
	memset((void *)&data, 0, sizeof(bspData_t));

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

// in order tree walk, keeping only visible nodes, front to back order
void Bsp::sort_leaf(vector<int> *leaf_list, int node_index, const vec3 &position, leaf_t *frameLeaf, bool order)
{
	if (node_index < 0)
	{
		leaf_t *leaf = &data.Leaf[-(node_index + 1)];

		if (cluster_visible(frameLeaf->cluster, leaf->cluster) == false)
				return;
		leaf_list->push_back( -(node_index + 1) );
		return;
	}

	node_t *node = &data.Node[node_index];
	plane_t *plane = &data.Plane[node->plane];

	float	distance = (plane->normal * position) - plane->d;
	if (distance >= 0)
	{
		if (order)
		{
			sort_leaf(leaf_list, node->back, position, frameLeaf, order);
			sort_leaf(leaf_list, node->front, position, frameLeaf, order);
		}
		else
		{
			sort_leaf(leaf_list, node->front, position, frameLeaf, order);
			sort_leaf(leaf_list, node->back, position, frameLeaf, order);
		}
	}
	else
	{
		if (order)
		{
			sort_leaf(leaf_list, node->front, position, frameLeaf, order);
			sort_leaf(leaf_list, node->back, position, frameLeaf, order);
		}
		else
		{
			sort_leaf(leaf_list, node->back, position, frameLeaf, order);
			sort_leaf(leaf_list, node->front, position, frameLeaf, order);
		}
	}
}

bool Bsp::collision_detect(vec3 &point, vec3 &oldpoint, plane_t *plane, float *depth, bool &water, float &water_depth,
	vector<surface_t *> &surface_list, bool debug, vec3 &clip, const vec3 &velocity, bool &lava, bool &slime, int &model_trigger)
{
	int leaf_index = find_leaf(point);
	leaf_t *leaf = &data.Leaf[leaf_index];
	model_trigger = 0;


	water = false;
	water_depth = 2048.0f;

	if (leaf_index == -1)
	{
		// Outside of map
		return true;
	}
	//vec3 midpoint = point - oldpoint;


	// A leaf is a convex volume of open space divided from the other leafs by brushes and bsp planes

	// A brush is usually a solid block usually defined by 6 planes, but can have more / less planes
	// (can be any shape really, but think floors walls etc, must be convex)
	// Can be non solid if majority of planes are non solid (eg: water/fog are nonsolid brushes
	// Each face of the brush has a texture, non visible faces (common/caulk etc) are removed for rendering
	// Normals point *inward*, you wont see collision planes until you are already inside the brush,
	// so to see which plane you hit you must test with the old position

	for (int i = 0; i < leaf->num_brushes; i++)
	{
		int *index = &data.LeafBrush[leaf->leaf_brush + i];
		brush_t	*brush = &data.Brushes[*index];
		int brush_index = brush->first_side;
		int num_sides = brush->num_sides;
		int count = 0;
		int nonsolid = 0;


		for( int j = 0; j < num_sides; j++)
		{
			brushSide_t *brushSide = &data.BrushSides[brush_index + j];
			int plane_index = brushSide->plane;

			float d = point * data.Plane[plane_index].normal - data.Plane[plane_index].d;

			// outside of brush plane
			if (d > 0.0f)
				continue;

			// Ignore non solid brushes
			if ((data.Material[brush->material].contents & CONTENTS_SOLID) == 0)
			{
				nonsolid++;

				if (data.Material[brush->material].contents & CONTENTS_LAVA && count + nonsolid == num_sides)
				{
					lava = true;
					continue;
				}
				else if (data.Material[brush->material].contents & CONTENTS_SLIME && count + nonsolid == num_sides)
				{
					slime = true;
					continue;
				}
				else if (data.Material[brush->material].contents & CONTENTS_WATER && count + nonsolid == num_sides)
				{
					// Set underwater flag + depth
					water = true;
					water_depth = -d;
					//printf("underwater depth = %f\n", d);
					continue;
				}
				else if (data.Material[brush->material].contents & CONTENTS_FOG && count + nonsolid == num_sides)
				{
					continue;
				}



				continue;
			}

			// Ignore individual non solid surfaces (had to move lower to allow water)
			if (data.Material[brush->material].surface & SURF_NONSOLID)
			{
				nonsolid++;
				continue;
			}


			// Check old position against planes, if we werent colliding before
			//then it is the collision plane we want to return
			d = oldpoint * data.Plane[plane_index].normal - data.Plane[plane_index].d;
			if (d > 0.0)
			{
				plane->normal = data.Plane[plane_index].normal;
				plane->d = data.Plane[plane_index].d;
				*depth = d;
			}
			count++;
		}

		if (count == num_sides)
		{
			if (debug)
			{
				printf("Inside brush %d with texture %s and contents 0x%X surf 0x%X\nDepth is %3.3f count is %d\nnormal is %3.3f %3.3f %3.3f\n", i,
					data.Material[brush->material].name,
					data.Material[brush->material].contents,
					data.Material[brush->material].surface,
					*depth, count,
					plane->normal.x, plane->normal.y, plane->normal.z);
			}
			return true;
		}
	}



	// do same thing for bsp doors platforms etc
	for (unsigned int i = 1; i < data.num_model; i++)
	{

		if (model_offset[i].magnitude() > 0.001f)
			continue;

		int index = data.Model[i].brush_index;
		if (index > data.num_brushes)
			break;
		brush_t	*brush = &data.Brushes[index];
		int brush_index = brush->first_side;
		int num_sides = brush->num_sides;
		int count = 0;


		//Inside brush 2 with texture textures/common/trigger and contents 0x1 surf 0x80


		// Let water pass through for water flag
		if ((data.Material[brush->material].contents & CONTENTS_WATER) == 0)
		{
			// Ignore non solid brushes
			if ((data.Material[brush->material].contents & CONTENTS_SOLID) == 0)
				continue;
		}

		for (int j = 0; j < num_sides; j++)
		{
			brushSide_t *brushSide = &data.BrushSides[brush_index + j];
			int plane_index = brushSide->plane;

			float d = point * data.Plane[plane_index].normal - data.Plane[plane_index].d;

			// outside of brush plane
			if (d > 0.0f)
				continue;

			// Inside a brush
			if (data.Material[brush->material].contents & CONTENTS_WATER)
			{
				// Set underwater flag + depth
				water = true;
				water_depth = -d;
				//				printf("underwater depth = %f\n", d);
				continue;
			}

			// Ignore individual non solid surfaces (had to move lower to allow water)
			if (data.Material[brush->material].surface & SURF_NONSOLID)
				continue;


			// Check old position against planes, if we werent colliding before
			//then it is the collision plane we want to return
			d = oldpoint * data.Plane[plane_index].normal - data.Plane[plane_index].d;
			if (d > 0.0)
			{
				plane->normal = data.Plane[plane_index].normal;
				plane->d = data.Plane[plane_index].d;
				*depth = d;
			}
			count++;
		}

		if (count == num_sides)
		{
			if (debug)
			{
				printf("Inside brush %d with texture %s and contents 0x%X surf 0x%X\nDepth is %3.3f count is %d\nnormal is %3.3f %3.3f %3.3f\n", i,
					data.Material[brush->material].name,
					data.Material[brush->material].contents,
					data.Material[brush->material].surface,
					*depth, count,
					plane->normal.x, plane->normal.y, plane->normal.z);
			}


			// inside a common/trigger etc
			if (data.Material[brush->material].surface & SURF_NODRAW)
			{

				model_trigger = i;
				return false;
			}


			return true;
		}
	}

	return false;
}

void Bsp::render_sky(Graphics &gfx, mLight2 &mlight2, int tick_num, vector<surface_t *> surface_list)
{
	float time = (float)tick_num / TICK_RATE;

	if (sky_face == -1 || enable_sky == false)
		return;

	gfx.SelectVertexBuffer(Model::skybox_vertex);
	gfx.SelectIndexBuffer(Model::skybox_index);


	if (enable_textures)
	{
		static vec2 scroll(0.0f, 0.0f);
		int surface_index = tex_object[data.Face[sky_face].material].index;
		if (surface_index != -1)
		{
			for (unsigned int i = 0; i < surface_list[surface_index]->num_stage; i++)
			{

				stage_t *stage = &surface_list[surface_index]->stage[i];

				if (stage->tcmod_rotate)
				{
					mlight2.tcmod_rotate(stage->tcmod_rotate_value * time, i);
				}
				if (stage->tcmod_scroll)
				{
					scroll.x += stage->tcmod_scroll_value.x * time * 0.01f;
					scroll.y += stage->tcmod_scroll_value.y * time * 0.01f;
					mlight2.tcmod_scroll(scroll, i);
				}
				if (stage->tcmod_scale)
				{
					mlight2.tcmod_scale(stage->tcmod_scale_value, i);
				}

				gfx.SelectTexture(i, tex_object[data.Face[sky_face].material].texObj[i]);
			}
		}
		else
		{
			gfx.SelectTexture(0, tex_object[data.Face[sky_face].material].texObj[0]);
		}
	}

	gfx.DrawArrayTri(0, 0, 36, 36);

	for (int i = 0; i < MAX_TEXTURES; i++)
	{
		gfx.SelectTexture(i, 0);
	}

}


inline void Bsp::render_face(face_t *face, Graphics &gfx, int stage, bool lightmap)
{
	bool lightmap_selected = false;
	bool shader = false;


	if (selected_map == false)
	{
		selected_map = true;
		gfx.SelectVertexBuffer(map_vertex_vbo);
		gfx.SelectIndexBuffer(map_index_vbo);
	}

	if (enable_textures)
	{
		if (lightmap && face->lightmap != -1)
		{
			// Pretty much shader stage with lightmap
			// normal faces without shaders get set below
//			gfx.Blend(true);
			//glBlendFunc(GL_SRC_COLOR, GL_DST_ALPHA);
			gfx.SelectTexture(stage, lightmap_object[face->lightmap]);
			shader = true;
		}
		else
		{
			gfx.SelectTexture(stage, tex_object[face->material].texObj[stage]);
		}
		// surfaces that arent lit with lightmaps eg: skies
		if (face->lightmap != -1 && shader == false)
		{
			gfx.SelectTexture(8, lightmap_object[face->lightmap]);
			lightmap_selected = true;
		}

	}
	if (enable_normalmap)
	{
		gfx.SelectTexture(9, normal_object[face->material]);
	}
	gfx.DrawArrayTri(face->index, face->vertex, face->num_index, face->num_verts);

	if (enable_textures)
	{
		if (lightmap_selected)
			gfx.SelectTexture(8, 0);
		gfx.SelectTexture(stage, 0);
	}
}

inline void Bsp::render_patch(face_t *face, Graphics &gfx, int stage, bool lightmap)
{
	int mesh_index = -1;
	int index_per_row = 2 * (mesh_level + 1);
	bool lightmap_selected = false;

	// Find pre-generated vertex data for patch O(n)

//	mesh_index = face_to_patch[face->vertex];
	if (mesh_index == -1)
	{
		for (int i = 0; i < num_meshes; i++)
		{
			if (patchdata[i].facevert == face->vertex)
			{
				mesh_index = i;
//				face_to_patch[face->vertex] = i;
				break;
			}
		}
	}

	if (mesh_index < 0 || mesh_index > 5000)
		return;

	// will be same texture for all 3x3 patches making up this mesh
	if (enable_textures)
	{
		bool shader = false;

		// surfaces that arent lit with lightmaps eg: skies
		if (lightmap && face->lightmap != -1)
		{
			//gfx.SelectTexture(stage, lightmap_object[face->lightmap]);
			shader = true;
			return;
		}
		else
		{
			gfx.SelectTexture(stage, tex_object[face->material].texObj[stage]);
		}

		if (face->lightmap != -1 && shader == false)
		{
			gfx.SelectTexture(8, lightmap_object[face->lightmap]);
			lightmap_selected = true;
		}
	}



	for (int i = 0; i < patchdata[mesh_index].num_mesh; i++)
	{
		selected_map = false;
		gfx.SelectVertexBuffer(patchdata[mesh_index + i].vbo);
		gfx.SelectIndexBuffer(patchdata[mesh_index + i].ibo);

		if (enable_normalmap)
		{
			gfx.SelectTexture(9, normal_object[face->material]);
		}
		// Rendered row by row because tessellate leaves a degenerate triangles at row ends
		for( int row = 0; row < mesh_level; row++)
		{
			gfx.DrawArrayTriStrip(row * index_per_row, 0,
				index_per_row, patchdata[mesh_index + i].num_verts);
		}
		
		/*
		tessellate_quadratic_bezier_surface could do 3x3 rendering in one pass
		Then we could probably put all the 3x3 patches for a mesh into a single ibo/vbo
		*/
		//gfx.DrawArrayTri(0, 0,
		//patchdata[mesh_index + i].num_indexes,
		//patchdata[mesh_index + i].num_verts);
		
	}
	if (enable_textures)
	{

		if (lightmap_selected)
		{
			gfx.SelectTexture(8, 0);
		}
		gfx.SelectTexture(stage, 0);
	}
}

inline void Bsp::render_billboard(face_t *face, Graphics &gfx, int stage, bool lightmap)
{
	bool lightmap_selected = false;

	if (enable_textures)
	{
		bool shader = false;

		if (lightmap && face->lightmap != -1)
		{
			shader = true;
			//gfx.SelectTexture(stage, lightmap_object[face->lightmap]);
			return;
		}
		else
		{
			gfx.SelectTexture(stage, tex_object[face->material].texObj[stage]);
		}

		// surfaces that arent lit with lightmaps eg: skies
		if (face->lightmap != -1 && shader == false)
		{
			gfx.SelectTexture(8, lightmap_object[face->lightmap]);
			lightmap_selected = true;
		}
	}

	if (enable_normalmap)
	{
		gfx.SelectTexture(9, normal_object[face->material]);
	}
	selected_map = false;
	gfx.SelectIndexBuffer(Model::quad_index);
	gfx.SelectVertexBuffer(Model::quad_vertex);
	gfx.DrawArrayTri(0, 0, 6, 4);
	if (lightmap_selected)
	{
		gfx.SelectTexture(8, 0);
	}
	gfx.SelectTexture(stage, 0);
}

void Bsp::gen_renderlists(int leaf, vector<surface_t *> &surface_list, vec3 &position)
{
	leaf_t *frameLeaf = &data.Leaf[leaf];

	face_list.resize(0);
	blend_list.resize(0);
	leaf_list.resize(0);

	// sort leafs front to back
	sort_leaf(&leaf_list, 0, position, frameLeaf, false);	
	// loop through visible sorted leaves, checking if leaf visible from current leaf
	for (unsigned int i = 0; i < leaf_list.size(); i++)
		add_list(surface_list, false, i);
	for (unsigned int i = leaf_list.size() - 1; i < leaf_list.size(); i--)
		add_list(surface_list, true, i);

}


void Bsp::add_list(vector<surface_t *> &surface_list, bool blend_flag, int i)
{
	leaf_t *leaf = &data.Leaf[leaf_list[i]];

	// generate face lists
	for (int j = 0; j < leaf->num_faces; j++)
	{
		int face_index = data.LeafFace[leaf->leaf_face + j];
		face_t *face = &data.Face[face_index];

		if (tex_object[face->material].index != -1 && enable_shader)
		{
			// Texture with a shader
			surface_t *surface = surface_list[tex_object[face->material].index];
			faceinfo_t render;

			memset(&render, 0, sizeof(faceinfo_t));
			render.face = face_index;
			render.shader = true;
			render.stage = 0;
			render.envmap = false;
			render.turb = false;

			if (surface->surfaceparm_sky)
			{
				render.sky = true;
				sky_face = face_index;
			}

			if (surface->surfaceparm_lava || surface->surfaceparm_slime || surface->surfaceparm_water)
			{
				render.turb = true;
			}

			if (surface->portal)			{				render.portal = true;			}


			for (unsigned int k = 0; k < surface->num_stage && k < max_stage; k++)
			{
				render.tcmod_rotate[k] = surface->stage[k].tcmod_rotate;
				render.deg[k] = surface->stage[k].tcmod_rotate_value;
				render.tcmod_scroll[k] = surface->stage[k].tcmod_scroll;
				render.scroll[k] = surface->stage[k].tcmod_scroll_value;
				render.tcmod_scale[k] = surface->stage[k].tcmod_scale;
				render.scale[k] = surface->stage[k].tcmod_scale_value;
				render.tcmod_stretch_sin[k] = surface->stage[k].tcmod_stretch_sin;
				render.tcmod_stretch_square[k] = surface->stage[k].tcmod_stretch_square;
				render.tcmod_stretch_triangle[k] = surface->stage[k].tcmod_stretch_triangle;
				render.tcmod_stretch_sawtooth[k] = surface->stage[k].tcmod_stretch_sawtooth;
				render.tcmod_stretch_inverse_sawtooth[k] = surface->stage[k].tcmod_stretch_inverse_sawtooth;
				render.stretch_value[k] = surface->stage[k].tcmod_stretch_value;
				render.stage = k;
				render.name = surface->stage[k].map_tex;
				render.lightmap[k] = surface->stage[k].lightmap;
				render.alpha_ge128 = surface->stage[k].alpha_ge128;
				render.alpha_lt128 = surface->stage[k].alpha_lt128;
				render.alpha_gt0 = surface->stage[k].alpha_gt0;
				render.envmap = surface->stage[k].tcgen_env;

				render.rgbgen_identity = surface->stage[k].rgbgen_identity;
				render.rgbgen_wave_sin[k] = surface->stage[k].rgbgen_wave_sin;
				render.rgbgen_wave_square[k] = surface->stage[k].rgbgen_wave_square;
				render.rgbgen_wave_triangle[k] = surface->stage[k].rgbgen_wave_triangle;
				render.rgbgen_wave_sawtooth[k] = surface->stage[k].rgbgen_wave_sawtooth;
				render.rgbgen_wave_inverse_sawtooth[k] = surface->stage[k].rgbgen_wave_inverse_sawtooth;
				render.rgbgen_wave_value[k] = surface->stage[k].rgbgen_wave_value;



				render.blend = false;

				if (surface->stage[k].alpha /*|| (surface->stage[0].lightmap && k > 0)*/)
				{
					render.alpha = true;
				}

				if (surface->stage[k].alpha_gt0)
				{
					render.alpha_gt0 = true;
				}
				else if (surface->stage[k].alpha_ge128)
				{
					render.alpha_ge128 = true;
				}
				else if (surface->stage[k].alpha_lt128)
				{
					render.alpha_lt128 = true;
				}
				else if (surface->stage[k].blendfunc_add ||
					surface->stage[k].blend_one_one)
				{
					// Doing multiple passes to get the quake3 blending right
					render.blend = true;
					render.blend_one_one = true;
				}
				else if (surface->stage[k].blend_one_zero)
				{
					render.blend = true;
					render.blend_one_zero = true;
				}
				else if (surface->stage[k].blendfunc_blend)
				{
					render.blend = true;
					render.blend_default = true;
				}
				else if (surface->stage[k].blendfunc_filter)
				{
					render.blend = true;
					render.blend_filter = true;
				}
				else if (surface->stage[k].blend_dst_color_one)
				{
					render.blend = true;
					render.blend_dstcolor_one = true;
				}
				else if (surface->stage[k].blend_dst_color_zero)
				{
					render.blend = true;
					render.blend_dstcolor_zero = true;
				}
				else if (surface->stage[k].blend_dst_color_src_alpha)
				{
					render.blend = true;
					render.blend_dst_color_src_alpha = true;
				}
				else if (surface->stage[k].blend_dst_color_one_minus_dst_alpha)
				{
					render.blend = true;
					render.blend_dst_color_one_minus_dst_alpha = true;
				}
				else if (surface->stage[k].one_minus_src_alpha_src_alpha)
				{
					render.blend = true;
					render.one_minus_src_alpha_src_alpha = true;
				}
				else if (surface->stage[k].blend_one_minus_src_alpha_src_alpha)
				{
					render.blend = true;
					render.blend_one_minus_src_alpha_src_alpha = true;
				}
				else if (surface->stage[k].blend_src_alpha_one_minus_src_alpha)
				{
					render.blend = true;
					render.blend_src_alpha_one_minus_src_alpha = true;
				}
				else if (surface->stage[k].blend_one_minus_dst_color_zero)
				{
					render.blend = true;
					render.blend_one_minus_dst_color_zero = true;
				}
				else if (surface->stage[k].blend_one_src_alpha)
				{
					render.blend = true;
					render.blend_one_src_alpha = true;
				}
				else if (surface->stage[k].blend_zero_src_color)
				{
					render.blend = true;
					render.blend_zero_src_color = true;
				}
				else if (surface->stage[k].blend_dst_color_src_color)
				{
					render.blend = true;
					render.blend_dst_color_src_color = true;
				}
				else if (surface->stage[k].blend_zero_src_alpha)
				{
					render.blend = true;
					render.blend_zero_src_alpha = true;
				}

				if (render.blend == false)
				{
					if (blend_flag == false)
						face_list.push_back(render);
				}
				else
				{
					if (blend_flag)
						blend_list.push_back(render);
				}
			}
		}
		else
		{
			// Texture without a shader
			faceinfo_t render;

			memset(&render, 0, sizeof(faceinfo_t));
			render.name = data.Material[face->material].name;
			render.face = face_index;
			render.stage = 0;
			render.envmap = false;
			render.turb = false;

			if (tex_object[face->material].texObj[0] < 0)
			{
				// texture has alpha channel, use it as a mask
				render.blend = true;
				render.blend_default = true;
			}

			if (render.blend == false)
			{
				if (blend_flag == false)
					face_list.push_back(render);
			}
			else
			{
				if (blend_flag)
					blend_list.push_back(render);
			}
		}
	}
}



void Bsp::set_blend_mode(Graphics &gfx, faceinfo_t &face)
{
	static int last_mode;

	if (face.blend_one_one)
	{
		if (last_mode != 1)
			gfx.BlendFuncOneOne();
		last_mode = 1;
	}
	else if (face.blend_default)
	{
		if (last_mode != 2)
			gfx.BlendFuncSrcAlphaOneMinusSrcAlpha();
		last_mode = 2;
	}
	else if (face.blend_filter)
	{
		// gl_dst_color gl_zero or blendfunc gl_zero gl_src_color
		if (last_mode != 99 && face.stage == 0)
		{
			gfx.BlendFuncZeroSrcColor();
			last_mode = 99;		}
		else if (last_mode != 98 && face.stage > 0)
		{
			gfx.BlendFuncDstColorZero();
			last_mode = 98;		}

		/*
		if (last_mode != 1)
			gfx.BlendFuncOneOne();
		last_mode = 1;
		*/
	}
	else if (face.blend_dstcolor_one)
	{
		if (last_mode != 3)
			gfx.BlendFuncDstColorOne();
		last_mode = 3;
	}
	else if (face.blend_one_zero)
	{
		if (last_mode != 4)
			gfx.BlendFuncOneZero();
		last_mode = 4;
	}
	else if (face.blend_zero_one)
	{
		if (last_mode != 5)
			gfx.BlendFuncZeroOne();
		last_mode = 5;
	}
	else if (face.blend_dst_color_one_minus_dst_alpha)
	{
		if (last_mode != 6)
			gfx.BlendFuncDstColorOneMinusDstAlpha();
//		gfx.BlendFuncOneZero();
		last_mode = 6;
	}
	else if (face.blend_dst_color_src_alpha)
	{
		if (last_mode != 7)
			gfx.BlendFuncDstColorSrcAlpha();
		last_mode = 7;
	}
	else if (face.blend_one_minus_src_alpha_src_alpha)
	{
		if (last_mode != 8)
			gfx.BlendFuncOneMinusSrcAlphaSrcAlpha();
		last_mode = 8;
	}
	else if (face.blend_src_alpha_one_minus_src_alpha)
	{
		if (last_mode != 9)
			gfx.BlendFuncSrcAlphaOneMinusSrcAlpha();
		last_mode = 9;
	}
	else if (face.blend_one_src_alpha)
	{
		if (last_mode != 10)
			gfx.BlendFuncOneSrcAlpha();
		last_mode = 10;
	}
	else if (face.blend_one_minus_dst_color_zero)
	{
		if (last_mode != 11)
			gfx.BlendFuncOneMinusDstColorZero();
		last_mode = 11;
	}
	else if (face.blend_zero_src_color)
	{
		if (last_mode != 12)
			gfx.BlendFuncZeroSrcColor();
		last_mode = 12;
	}
	else if (face.blend_dst_color_src_color)
	{
		if (last_mode != 13)
			gfx.BlendFuncDstColorSrcColor();
		last_mode = 13;
	}
	else if (face.blend_zero_src_alpha)
	{
		if (last_mode != 14)
			gfx.BlendFuncZeroSrcAlpha();
		last_mode = 14;
	}
	else if (face.blend_dstcolor_zero)
	{
		if (last_mode != 15)
			gfx.BlendFuncDstColorZero();
		last_mode = 15;
	}
	else
	{
		if (last_mode != 2)
			gfx.BlendFuncSrcAlphaOneMinusSrcAlpha();
		last_mode = 2;
	}
}


void Bsp::set_tcmod(mLight2 &mlight2, faceinfo_t &face, int tick_num, float time)
{
	int j = face.stage;

	if (face.tcmod_rotate[j])
	{
		mlight2.tcmod_rotate(face.deg[j] * time, j);
	}
	if (face.tcmod_scroll[j])
	{
		face.scroll_value[j].x += face.scroll[j].x * time * 0.01f;
		face.scroll_value[j].y += face.scroll[j].y * time * 0.01f;
		mlight2.tcmod_scroll(face.scroll_value[j], j);
	}
	if (face.tcmod_scale[j])
	{
		mlight2.tcmod_scale(face.scale[j], j);
	}
	if (face.tcmod_stretch_sin[j])
	{
		mlight2.tcmod_stretch_sin(face.stretch_value[j].x,
			face.stretch_value[j].y,
			face.stretch_value[j].z,
			tick_num, j);
	}
	if (face.tcmod_stretch_square[j])
	{
		mlight2.tcmod_stretch_square(face.stretch_value[j].y,
			face.stretch_value[j].z,
			face.stretch_value[j].w,
			tick_num, j);
	}
	if (face.tcmod_stretch_triangle[j])
	{
		mlight2.tcmod_stretch_square(face.stretch_value[j].x,
			face.stretch_value[j].y,
			face.stretch_value[j].z,
			tick_num, j);
	}
	if (face.tcmod_stretch_sawtooth[j])
	{
		mlight2.tcmod_stretch_square(face.stretch_value[j].x,
			face.stretch_value[j].y,
			face.stretch_value[j].z,
			tick_num, j);
	}
	if (face.tcmod_stretch_inverse_sawtooth[j])
	{
		mlight2.tcmod_stretch_square(face.stretch_value[j].x,
			face.stretch_value[j].y,
			face.stretch_value[j].z,
			tick_num, j);
	}
	//rgbgen
	if (face.rgbgen_wave_sin[j])
	{
		mlight2.rgbgen_wave_sin(face.rgbgen_wave_value[j].x,
			face.rgbgen_wave_value[j].y,
			face.rgbgen_wave_value[j].z,
			tick_num, j);
	}
	if (face.rgbgen_wave_square[j])
	{
		mlight2.rgbgen_wave_square(face.rgbgen_wave_value[j].y,
			face.rgbgen_wave_value[j].z,
			face.rgbgen_wave_value[j].w,
			tick_num, j);
	}
	if (face.rgbgen_wave_triangle[j])
	{
		mlight2.rgbgen_wave_square(face.rgbgen_wave_value[j].x,
			face.rgbgen_wave_value[j].y,
			face.rgbgen_wave_value[j].z,
			tick_num, j);
	}
	if (face.rgbgen_wave_sawtooth[j])
	{
		mlight2.rgbgen_wave_square(face.rgbgen_wave_value[j].x,
			face.rgbgen_wave_value[j].y,
			face.rgbgen_wave_value[j].z,
			tick_num, j);
	}
	if (face.rgbgen_wave_inverse_sawtooth[j])
	{
		mlight2.rgbgen_wave_square(face.rgbgen_wave_value[j].x,
			face.rgbgen_wave_value[j].y,
			face.rgbgen_wave_value[j].z,
			tick_num, j);
	}

}

void Bsp::render(vec3 &position, matrix4 &mvp, Graphics &gfx, vector<surface_t *> &surface_list, mLight2 &mlight2, int tick_num)
{
	int frameIndex = find_leaf(position);
	vec2 zero(0.0f, 0.0f);
	vec2 one(1.0f, 1.0f);
	float time = ((float)tick_num / TICK_RATE);
	selected_map = false;
	float alpha_value = 1.0f;

	if (frameIndex != lastIndex)
	{
		gen_renderlists(frameIndex, surface_list, position);
		lastIndex = frameIndex;
	}

	gfx.Blend(false);
	for (unsigned int i = 0; i < face_list.size(); i++)
	{
		face_t *face = &data.Face[face_list[i].face];

		if (face_list[i].sky)
		{
			continue;
		}

		if (face_list[i].shader && enable_textures)
		{
			if (face_list[i].alpha_gt0)
			{
				mlight2.alphatest(face_list[i].stage, 1);
			}
			else if (face_list[i].alpha_lt128)
			{
				mlight2.alphatest(face_list[i].stage, 2);
			}
			else if (face_list[i].alpha_ge128)
			{
				mlight2.alphatest(face_list[i].stage, 3);
			}
			else
			{
				mlight2.alphatest(face_list[i].stage, 0);
			}


			set_tcmod(mlight2, face_list[i], tick_num, time);

			if (face_list[i].envmap)
				mlight2.envmap(face_list[i].stage, 255);

			if (face_list[i].turb)
				mlight2.turb(face_list[i].stage, 255);


			if (face_list[i].alpha)
				mlight2.set_alpha(alpha_value);
			else
				mlight2.set_alpha(-1.0f);

			if (face_list[i].portal)
			{
//				glStencilMask(0xFF);
				mlight2.portal(255);
					tex_object[face->material].texObj[0] = portal_tex;
					tex_object[face->material].texObj[1] = portal_tex;
					tex_object[face->material].texObj[2] = portal_tex;
					tex_object[face->material].texObj[3] = portal_tex;
			}

			if (face_list[i].lightmap[face_list[i].stage] && face->lightmap != -1)
				mlight2.set_lightmap_stage(1);
		}

		if (face->type == 1 || face->type == 3)
		{
			render_face(face, gfx, face_list[i].stage, face_list[i].lightmap[face_list[i].stage]);
			mlight2.portal(0);
		}
		else if (face->type == 2 && enable_patch)
		{
			render_patch(face, gfx, face_list[i].stage, face_list[i].lightmap[face_list[i].stage]);
		}
		else// (face->type == 4)
		{
			render_billboard(face, gfx, face_list[i].stage, face_list[i].lightmap[face_list[i].stage]);
		}

		mlight2.alphatest(0, 0);
		mlight2.alphatest(1, 0);
		mlight2.alphatest(2, 0);
		mlight2.alphatest(3, 0);

		if (face_list[i].shader  && enable_textures)
		{
			int j = face_list[i].stage;
			mlight2.tcmod_rotate(0, j);
			mlight2.tcmod_scroll(zero, j);
			mlight2.tcmod_scale(one, j);
			mlight2.rgbgen_scale(j, one.x);

			if (face_list[i].lightmap[face_list[i].stage] && face->lightmap != -1)
				mlight2.set_lightmap_stage(0);
			else if (face_list[i].envmap)
				mlight2.envmap(face_list[i].stage, 0);
			else if (face_list[i].turb)
				mlight2.turb(face_list[i].stage, 0);

		}
	}


	if (blend_list.size() > 0 && enable_textures)
	{
		mlight2.set_light(1.0f, 1.0f, 0);
		if (enable_blend)
		{
			gfx.DepthFunc("<=");
			gfx.Blend(true);
		}

		for (unsigned int i = 0; i < blend_list.size(); i++)
		{
			face_t *face = &data.Face[blend_list[i].face];

			if (blend_list[i].sky)
			{
				//skip sky texture, already drawn
				continue;
			}

			if (blend_list[i].shader && enable_textures)
			{
				set_tcmod(mlight2, blend_list[i], tick_num, time);

				if (blend_list[i].lightmap[blend_list[i].stage] && face->lightmap != -1)
					mlight2.set_lightmap_stage(1);
				else if (blend_list[i].envmap)
					mlight2.envmap(blend_list[i].stage, 255);
				else if (blend_list[i].turb)
					mlight2.turb(blend_list[i].stage, 255);

			}

			if (blend_list[i].alpha)
				mlight2.set_alpha(alpha_value);
			else
				mlight2.set_alpha(-1.0f);

			if (blend_list[i].blend)
			{
				set_blend_mode(gfx, blend_list[i]);
			}

			if (blend_list[i].portal)
			{
//				glStencilMask(0xFF);
				mlight2.portal(255);
				tex_object[face->material].texObj[0] = portal_tex;
				tex_object[face->material].texObj[1] = portal_tex;
				tex_object[face->material].texObj[2] = portal_tex;
				tex_object[face->material].texObj[3] = portal_tex;
			}

			if (face->type == 1 || face->type == 3)
			{
				render_face(face, gfx, blend_list[i].stage, blend_list[i].lightmap[blend_list[i].stage]);
				mlight2.portal(0);
			}
			else if (face->type == 2 && enable_patch)
			{
				render_patch(face, gfx, blend_list[i].stage, blend_list[i].lightmap[blend_list[i].stage]);
			}
			else// (face->type == 4)
			{
				render_billboard(face, gfx, blend_list[i].stage, blend_list[i].lightmap[blend_list[i].stage]);
			}

			if (blend_list[i].shader && enable_textures)
			{
				int j = blend_list[i].stage;
				//technically shouldnt need to reset
				mlight2.tcmod_rotate(0, j);
				mlight2.tcmod_scroll(zero, j);
				mlight2.tcmod_scale(one, j);
				mlight2.rgbgen_scale(j, one.x);


				if (blend_list[i].lightmap[blend_list[i].stage] && face->lightmap != -1)
					mlight2.set_lightmap_stage(0);
				else if (blend_list[i].envmap)
					mlight2.envmap(blend_list[i].stage, 0);
				else if (blend_list[i].turb)
					mlight2.turb(blend_list[i].stage, 0);

			}
		}
	}
	if (enable_blend)
	{
		gfx.DepthFunc("<");
		gfx.Blend(false);
	}


	for (unsigned int i = 1; i < data.num_model; i++)
	{
//		mlight2.set_matrix(mvp);
		if (abs32(model_offset[i].x) + abs32(model_offset[i].y) + abs32(model_offset[i].z) < 0.001f)
			render_model(i, gfx);
	}
	
#ifndef DIRECTX
	render_sky(gfx, mlight2, tick_num, surface_list);
#endif
//	draw_box(frameLeaf->mins, frameLeaf->maxs);


}


/*
	This fixes origin for func_door's, func_bobbing, trigger's etc
*/
vec3 Bsp::model_origin(unsigned int index)
{
	if (index >= data.num_model)
	{
		debugf("Invalid model index\n");
		return vec3();
	}
	model_t *model = &data.Model[index];

	vec3 min((float)model->min[0], (float)model->min[1], (float)model->min[2]);
	vec3 max((float)model->max[0], (float)model->max[1], (float)model->max[2]);
	vec3 origin(
		(max.x - min.x) * 0.5f + min.x,
		(max.y - min.y) * 0.5f + min.y,
		(max.z - min.z) * 0.5f + min.z);

	return origin;
}

void Bsp::render_model(unsigned int index, Graphics &gfx)
{
	if (index >= data.num_model)
	{
		debugf("Invalid model index\n");
		return;
	}
	model_t *model = &data.Model[index];

	vec3 min((float)model->min[0], (float)model->min[1], (float)model->min[2]);
	vec3 max((float)model->max[0], (float)model->max[1], (float)model->max[2]);

	for (int i = 0; i < model->num_faces; i++)
	{
		int face_index = model->face_index + i;

		if (face_index > data.num_faces)
			break;

		face_t *face = &data.Face[face_index];

		if (face->type == 1 || face->type == 3)
		{
			render_face(face, gfx, 0, false);
		}
		else if (face->type == 2)
		{
			render_patch(face, gfx, 0, false);
		}
		else// (face->type == 4)
		{
			render_billboard(face, gfx, 0, false);
		}
	}
}

/*
	Determines set of visible leafs from current leaf
*/
inline int Bsp::cluster_visible(int vis_cluster, int test_cluster)
{
	int byte_offset, bit_offset;
	char test_byte;
	int shift = 0;

	// if no vis data or if camera leaf invalid (outside map) draw everything
	if (vis_cluster < 0 || data.num_vis == 0)
		return 1;

	// bit of pVecs we need to return
	bit_offset = vis_cluster + test_cluster * data.VisData->vector_size * 8; 
	byte_offset = bit_offset / 8;

	shift = (bit_offset - byte_offset * 8);
	if (shift < 0)
		return 1;

	test_byte = 1 << shift;
	return 	(&data.VisData->pVecs)[byte_offset] & test_byte;
}

void Bsp::load_from_file(char *filename, texture_t &texObj, Graphics &gfx, char **pk3_list, int num_pk3)
{
	char	texture_name[LINE_SIZE] = { 0 };
	int		tex_object = 0;

	//printf("Attempting to load %s, trying .tga\n", filename);
	snprintf(texture_name, LINE_SIZE, "media/%s.tga", filename);
	tex_object = load_texture_pk3(gfx, texture_name, pk3_list, num_pk3, false, true);

	if (tex_object == 0)
	{
		//printf("Attempting to load %s, trying .jpg\n", filename);
		snprintf(texture_name, LINE_SIZE, "media/%s.jpg", filename);
		tex_object = load_texture_pk3(gfx, texture_name, pk3_list, num_pk3, false, true);
	}
	if (tex_object != 0)
	{
		//printf("loaded %s into unit %d\n", filename, texObj.num_tex);
		texObj.texObj[texObj.num_tex] = tex_object;
	}

}

void Bsp::load_from_shader(char *name, vector<surface_t *> &surface_list, texture_t *texObj, Graphics &gfx, char **pk3_list, int num_pk3)
{
	char			texture_name[LINE_SIZE + 1];
	int				tex_object = 0;
	unsigned int	j = 0;

	memset(texture_name, 0, LINE_SIZE + 1);

	//printf("Attempting to load %s, trying surface_list\n", name);
	for (j = 0; j < surface_list.size(); j++)
	{
		if (strcmp(name, surface_list[j]->name) == 0)
		{
			//printf("Found shader [%s], trying stages\n", surface_list[j]->name);
			texObj->index = j;
			break;
		}
	}

	if (j == surface_list.size())
	{
		return;
	}

	if (surface_list[j]->portal)
		texObj->portal = true;
	else
		texObj->portal = false;


	for (unsigned int k = 0; k < surface_list[j]->num_stage && k < 4; k++)
	{
		stage_t *stage = &surface_list[j]->stage[k];
		//printf("Raw stage %d is [%s]\n", j, surface_list[i]->stage.stage[j]);

		if (stage->lightmap)
		{
			continue;
		}

		if (stage->map /*&& surface_list[j]->stage[k].tcgen_env == false*/)
		{
			snprintf(texture_name, LINE_SIZE, "media/%s", stage->map_tex);

			//printf("Trying texture [%s]\n", texture_name);
			tex_object = load_texture_pk3(gfx, texture_name, pk3_list, num_pk3, false, true);
		}
		else if (stage->clampmap)
		{
			snprintf(texture_name, LINE_SIZE, "media/%s", stage->clampmap_tex);
			tex_object = load_texture_pk3(gfx, texture_name, pk3_list, num_pk3, true, true);
		}
		else if (stage->anim_map)
		{
			char *tex = strtok(stage->anim_map_tex, " ");
			anim_list.push_back(texObj);
			int n = 0;

			int freq = atoi(tex);
			texObj->freq = freq;

//			printf("animmap freq %d\n", freq);
			tex = strtok(NULL, " ");
			while (tex != NULL)
			{
//				printf("animmap tex %s\n", tex);
				snprintf(texture_name, LINE_SIZE, "media/%s", tex);
				texObj->texObjAnim[n++] = load_texture_pk3(gfx, texture_name, pk3_list, num_pk3, false, true);
				tex = strtok(NULL, " ");
			}

			texObj->texObj[k] = texObj->texObjAnim[0];
			texObj->anim_unit = k;
			texObj->num_anim = n;
			continue;
		}


		if (tex_object != 0 && tex_object != -1 && stage->anim_map == false)
		{
			//printf("Loaded texture stage %d into unit %d for shader with texture %s\n", k, texObj->num_tex, texture_name);
			//texObj->stage = k;
			texObj->texObj[k] = tex_object;
			continue;
		}

		if (strlen(texture_name) > 4)
		{
			texture_name[strlen(texture_name) - 4] = '\0';
			strcat(texture_name, ".jpg");
			//printf("Trying jpeg texture [%s]\n", texture_name);
		}
		else
		{
			continue;
		}

		tex_object = load_texture_pk3(gfx, texture_name, pk3_list, num_pk3, false, true);
		if (tex_object != 0)
		{
			//printf("Loaded texture stage %d for shader with texture %s\n", k, texture_name);
			//texObj->stage = k;
			texObj->texObj[k] = tex_object;
			continue;
		}
	}

	return;
}



void Bsp::load_textures(Graphics &gfx, vector<surface_t *> &surface_list, char **pk3_list, int num_pk3)
{
	enable_textures = true;
	for (unsigned int i = 0; i < data.num_lightmaps; i++)
	{
#ifdef OPENGL
		if (data.header->version == 0x2E)
		{
			lightmap_object[i] = gfx.LoadTexture(128, 128, GL_RGB, GL_RGB, (void *)&(data.LightMaps[i].image), false);
		}
		else
		{
			printf("Unknown BSP Version %X, assuming Quake3 0x2E\n", data.header->version);
			lightmap_object[i] = gfx.LoadTexture(128, 128, GL_RGB, GL_RGB, (void *)&(data.LightMaps[i].image), false);
		}
#endif
#ifdef DIRECTX
		byte *pBits = tga_24to32(128, 128, (byte *)data.LightMaps[i].image, false);
		lightmap_object[i] = gfx.LoadTexture(128, 128, 4, 4, (void *)data.LightMaps[i].image, false);
		delete [] pBits;
#endif
	}

	// Wolfenstein Enemy territory has lightmap tgas outside of bsp
	if (data.header->version == 0x2F)
	{
		char filename[128];
		char name[128];

		//remove .bsp
		strcpy(name, map_name);
		name[strlen(name) - 4] = '\0';

		//mapname/lm_0000.tga
		for (int i = 0;; i++)
		{
			sprintf(filename, "media/%s/lm_%04d.tga", name, i);
			lightmap_object[i] = load_texture_pk3(gfx, filename, pk3_list, num_pk3, false, false);
			if (lightmap_object[i] == 0)
				break;
		}
	}

	for (unsigned int i = 0; i < data.num_materials; i++)
	{
		material_t	*material = &data.Material[i];

		// hack for quake1 maps
		if (i > 75 && quake1)
			break;

		int length = strlen(material->name);
		for (int i = 0; i < length; i++)
		{
			if (material->name[i] == '*')
				material->name[i] = '#';
		}
		
		strcpy(tex_object[i].name, data.Material[i].name);
		load_from_shader(material->name, surface_list, &tex_object[i], gfx, pk3_list, num_pk3);
		if (tex_object[i].texObj[tex_object[i].num_tex] == 0)
		{
			load_from_file(material->name, tex_object[i], gfx, pk3_list, num_pk3);
		}
		
		if (tex_object[i].texObj[tex_object[i].num_tex] == 0)
		{
			//printf("******* Failed to find texture for shader %s texunit %d\n", material->name, tex_object[i].num_tex);
//			tex_object[i].texObj[tex_object[i].num_tex] = 1; // no_tex image
//			tex_object->num_tex++;
			continue;
		}
		tex_object[i].num_tex++;

		if (enable_normalmap)
		{
			char texture_name[128];
			snprintf(texture_name, LINE_SIZE, "media/%s_normal.tga", material->name);
			normal_object[i] = load_texture(gfx, texture_name, false, false);
		}

	}
}

/*
	cylindrical patches are a 3x9 set of control points
	U patches are a 3x5 set of control points
	This function assumes it's given 3x3 set of control points
	hacky fix for cylindrical patches and U patches in calling function
*/
void Bsp::tessellate(int level, bspvertex_t control[], vertex_t **vertex_array, int &num_verts,
	int **index_array, int &num_indexes, vec2 &texcoord, vec2 &lightcoord, vec2 &size)
{
	vec3 a, b;
	int i, j;

	num_verts = level + 1;

	*vertex_array = new vertex_t[num_verts * num_verts];

	// calculate first set of verts
	for (i = 0; i <= level; i++)
	{
		float a2 = (float) i / level;
		float b2 = 1.0f - a2;
		(*vertex_array)[i].position =
			control[0].position * (b2 * b2) +
			control[3].position * (2 * b2 * a2) +
			control[6].position * (a2 * a2);
	}

	// calculate rest of verts
	for ( i = 1; i <= level; i++)
	{
		float a2 = (float)i / level;
		float b2 = 1.0f - a2;

		vec3 temp[3];

		temp[0] = 
			control[0].position * (b2 * b2) + 
			control[1].position * (2 * b2 * a2) + 
			control[2].position * (a2 * a2);
		temp[1] = 
			control[3].position * (b2 * b2) + 
			control[4].position * (2 * b2 * a2) + 
			control[5].position * (a2 * a2);
		temp[2] = 
			control[6].position * (b2 * b2) + 
			control[7].position * (2 * b2 * a2) + 
			control[8].position * (a2 * a2);

		for(j = 0; j <= level; j++)
		{
			float a3 = (float) j / level;
			float b3 = 1.0f - a3;

			(*vertex_array)[i * num_verts + j].position =
				temp[0] * (b3 * b3) +
				temp[1] * (2 * b3 * a3) +
				temp[2] * (a3 * a3);
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
			(*vertex_array)[i * num_verts + j].texCoord0.x = i * (1.0f / level) + texcoord.x;
			(*vertex_array)[i * num_verts + j].texCoord0.y = -(j * (1.0f / level) + texcoord.y);

			if (texcoord.x < 0.0f)
				(*vertex_array)[i * num_verts + j].texCoord0.x *= -1;
			if (texcoord.y < 0.0f)
				(*vertex_array)[i * num_verts + j].texCoord0.y *= -1;


			(*vertex_array)[i * num_verts + j].texCoord1.x = size.x * i * (1.0f / level) + lightcoord.x;
			(*vertex_array)[i * num_verts + j].texCoord1.y = size.y * j * (1.0f / level) + lightcoord.y;

			if (lightcoord.x < 0.0f)
				(*vertex_array)[i * num_verts + j].texCoord1.x *= -1;
			if (lightcoord.y < 0.0f)
				(*vertex_array)[i * num_verts + j].texCoord1.y *= -1;


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

bool Bsp::vis_test(vec3 &x, vec3 &y, int &leaf_a, int &leaf_b)
{
		int a = find_leaf(x);
		int b = find_leaf(y);

		leaf_a = a;
		leaf_b = b;


		if (a == b)
		{
			return true;
		}

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
	vec3 *temp_tan = new vec3 [num_index];
	vec3 *temp_btan = new vec3 [num_index];

	memset(temp_tan, 0, num_vert * sizeof(vec3));
	memset(temp_btan, 0, num_vert * sizeof(vec3));
	for (int i = 0; i < num_index - 2; i += 3)
	{
		int ai = index[i];
		int bi = index[i + 1];
		int ci = index[i + 2];
        
		const vec3 p0 = vertex[ai].position;
		const vec3 p1 = vertex[bi].position;
		const vec3 p2 = vertex[ci].position;

		const vec2 tex0 = vertex[ai].texCoord0;
		const vec2 tex1 = vertex[bi].texCoord0;
		const vec2 tex2 = vertex[ci].texCoord0;
		
		
		vec3 p, q, n;
		vec3 t, b;
		float s1, s2, t1, t2;
		float denom;


		// triangle span vectors
		p = p1 - p0;
		q = p2 - p0;

		//cross product for normal
		n = vec3::crossproduct(p,q);

		// texture coordinate vectors
		s1 = tex1.x - tex0.x;
		t1 = tex1.y - tex0.y;

		s2 = tex2.x - tex0.x;
		t2 = tex2.y - tex0.y;

		// tangent and bitangent
		denom = (s1 * t2 - s2 * t1);
		if (abs32(denom) >= 0.00001f)
		{
			t = (p * t2 - q * t1) / denom;
			b = (q * s1 - p * s2) / denom;
		}
		else
		{
			t.x = 1.0f;
			t.y = 0.0f;
			t.z = 0.0f;
			b.x = 0.0f;
			b.y = 0.0f;
			b.z = 1.0f;
		}
		t.normalize();
		b.normalize();

		temp_tan[i]    = t;
		temp_tan[i+1]  = t;
		temp_tan[i+2]  = t;
		temp_btan[i]   = b;
		temp_btan[i+1] = b;
		temp_btan[i+2] = b;
	}

	for (int i = 0; i < num_vert; i++)
	{
		tangent[i].x = temp_tan[i].x;
		tangent[i].y = temp_tan[i].y;
		tangent[i].z = temp_tan[i].z;
		tangent[i].w = 1.0f;
	}

	delete[] temp_tan;
	delete[] temp_btan;
}
/*
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
*/

/*
Loop through all the model's triangles
If triangle faces the light source (dot product > 0)
Insert the three edges (pair of vertices), into an edge stack
Check for previous occurrence of each edges or it's reverse in the stack
If an edge or its reverse is found in the stack, remove both edges
Start with new triangle
*/
void Bsp::find_edges(vec3 &position, Edge &edge_list)
{
	int leaf_index = find_leaf(position);

	leaf_t *light_Leaf = &data.Leaf[leaf_index];

	// loop through all leaves, checking if leaf visible from current leaf
	for (unsigned int i = 0; i < data.num_leafs; i++)
	{
		leaf_t *leaf = &data.Leaf[i];

		if (!cluster_visible(light_Leaf->cluster, leaf->cluster))
			continue;

		for (int j = 0; j < leaf->num_faces; j++)
		{
			vector<vec3> vertex_list;
			int face_index = data.LeafFace[leaf->leaf_face + j];
			face_t *face = &data.Face[face_index];

			for (int k = 0; k < face->num_index; k++)
			{
				int index = data.IndexArray[face->index + k];
				vec3 x = data.Vert[face->vertex + index].position;
				vertex_list.push_back(x);
			}

			for(unsigned int k = 0; k < vertex_list.size(); k += 3)
			{
				vec3 x = vertex_list[k];
				vec3 y = vertex_list[k + 1];
				vec3 z = vertex_list[k + 2];

				
				vec3 a = y - x;
				vec3 b = z - x;
				vec3 normal = vec3::crossproduct(a, b);

				vec3 lightdir1 = x - position;
				vec3 lightdir2 = y - position;
				vec3 lightdir3 = z - position;
				vec3 lightdir;

				if (lightdir1.magnitude() < lightdir2.magnitude() && lightdir1.magnitude() < lightdir3.magnitude())
					lightdir = lightdir1;
				else if (lightdir2.magnitude() < lightdir1.magnitude() && lightdir2.magnitude() < lightdir3.magnitude())
					lightdir = lightdir2;
				else
					lightdir = lightdir3;

				normal.normalize();
//				if (lightdir.magnitude() > 400.0f)
//					continue;


				if (lightdir * normal > 0)
				{
					vec3 triple[3][2];

					if (x.x < y.x)
					{
						triple[0][0] = x;
						triple[0][1] = y;
					}
					else
					{
						triple[0][1] = x;
						triple[0][0] = y;
					}

					if (x.x < z.x)
					{
						triple[1][0] = x;
						triple[1][1] = z;
					}
					else
					{
						triple[1][1] = x;
						triple[1][0] = z;
					}

					if (y.x < z.x)
					{
						triple[2][0] = y;
						triple[2][1] = z;
					}
					else
					{
						triple[2][1] = y;
						triple[2][0] = z;
					}
					edge_list.insert(&triple[0][0]);
					edge_list.insert(&triple[1][0]);
					edge_list.insert(&triple[2][0]);
				}
			}
		}
	}
}

void Bsp::hitscan(vec3 &origin, vec3 &dir, float &distance)
{
	// Really need to test brush planes not leaf aabbs
	for (unsigned int i = 0; i < data.num_leafs; i++)
	{
		vec3 min((float)data.Leaf[i].min[0], (float)data.Leaf[i].min[1], (float)data.Leaf[i].min[2]);
		vec3 max((float)data.Leaf[i].max[0], (float)data.Leaf[i].max[1], (float)data.Leaf[i].max[2]);

		RayBoxSlab(origin, dir, min, max, distance);
	}	
}




//=====================================================================================
// Magic "trace" function for collision detection
//=====================================================================================
// Idea is you take these two points and walk them along the line towards each other
// if they pass each other, no collision, if they dont you collided with something
// For each collision plane you can move one point, (the one not inside the plane)
// eg: dist = plane.normal dot position - plane.d
// if dist > 0, then move point dist on the line
// http://lineofsight.awright2009.com/BrushCollision.pdf
//======================================================================================
//
// Inputs, start and desired end position
// Outputs, either end position, or as far as you can go 
// If a collision occurs, we set collision flag and collision normal
// we also set an on_ground flag to indicate we are on the ground plane
//======================================================================================
vec3 Bsp::trace(vec3 &start, vec3 &end, vec3 &normal)
{
	float trace_amount = 1.0f;

	// Check trace starting from BSP tree root
	check_node(0, 0.0f, 1.0f, start, end);

	if (abs32(trace_amount - 1.0f) < 0.001)
	{
		return end;
	}
	else
	{
		// collision occurred, collision normal expected in class
		vec3 new_pos = start + ((end - start) * trace_amount);
		//vec3 move = end - new_pos;

		//float distance = move * collision_normal;

		//vec3 end_pos = end - collision_normal * distance;

		new_pos = trace(new_pos, end, normal);

		normal = collision_normal;

		if (collision_normal.y > 0.2f || on_ground)
			on_ground = true;
		else
			on_ground = false;


		return new_pos;
	}
}

// Walks BSP Tree and checks brush planes for collisions
void Bsp::check_node(int node_index, float start_amount, float end_amount, vec3 &start, vec3 &end)
{
	// if leaf node, finish recursion
	if (node_index < 0)
	{
		leaf_t *leaf = &data.Leaf[-(node_index + 1)];

		for (int i = 0; i < leaf->num_brushes; i++)
		{
			int index = data.LeafBrush[leaf->leaf_brush + i];
			brush_t *brush = &data.Brushes[index];

			if ((brush->num_sides > 0) && (data.Material[brush->material].contents & CONTENTS_SOLID))
			{
				check_brush(brush, start, end);
			}
		}

		return;
	}

	node_t *node = &data.Node[node_index];
	plane_t *plane = &data.Plane[node->plane];

	// Find distance to BSP splitter plane
	float start_distance = start * plane->normal - plane->d;
	float end_distance = end * plane->normal - plane->d;


	if (start_distance >= 0.0f && end_distance >= 0.0f)
	{
		// start and end in front of splitter plane
		check_node(node->front, start_distance, end_distance, start, end);
	}
	else if (start_distance < 0.0f && end_distance < 0.0f)
	{
		// start and end behind splitter plane
		check_node(node->back, start_distance, end_distance, start, end);
	}
	else
	{
		// start and end span splitter plane, divide line in half
		float ratio1 = 1.0f, ratio2 = 0.0f, half_amount = 0.0f;
		vec3 middle;
		float epsilon = 0.01f; // leave small gap

		int side = node->front;

		if (start_distance < end_distance)
		{
			side = node->back;

			float inverse = 1.0f / (start_distance - end_distance);
			ratio1 = (start_distance - epsilon) * inverse;
			ratio2 = (start_distance + epsilon) * inverse;
		}
		else if (start_distance > end_distance)
		{
			float inverse = 1.0f / (start_distance - end_distance);
			ratio1 = (start_distance + epsilon) * inverse;
			ratio2 = (start_distance - epsilon) * inverse;
		}

		if (ratio1 < 0.0f)
			ratio1 = 0.0f;
		else if (ratio1 > 1.0f)
			ratio1 = 1.0f;

		if (ratio2 < 0.0f)
			ratio2 = 0.0f;
		else if (ratio2 > 1.0f)
			ratio2 = 1.0f;

		half_amount = start_amount + ((end_amount - start_amount) * ratio1);
		middle = start + ((end - start) * ratio1);

		check_node(side, start_amount, half_amount, start, middle);

		half_amount = start_amount + ((end_amount - start_amount) * ratio2);
		middle = start + ((end - start) * ratio2);

		if (side == node->back)
			check_node(node->front, half_amount, end_distance, middle, end);
		else
			check_node(node->back, half_amount, end_distance, middle, end);
	}
}


void Bsp::check_brush(brush_t *brush, vec3 &start, vec3 &end)
{
	float start_amount = -1.0f;
	float end_amount = 1.0f;
	bool starts_out = false;
	float epsilon = 0.01f;

	for (int i = 0; i < brush->num_sides; i++)
	{
		brushSide_t *brushSide = &data.BrushSides[brush->first_side + i];
		plane_t *plane = &data.Plane[brushSide->plane];

		float start_distance = start * plane->normal - plane->d;
		float end_distance = end * plane->normal - plane->d;

		if (start_distance > 0)
			starts_out = true;

		if (start_distance > 0 && end_distance > 0)
			return;

		if (start_distance <= 0 && end_distance <= 0)
			continue;

		if (start_distance > end_distance)
		{
			float ratio1 = (start_distance - epsilon) / (start_distance - end_distance);

			if (ratio1 > start_amount)
			{
				start_amount = ratio1;
				collision = true;

				collision_normal = plane->normal;


				if ((abs32(start.x - end.x) >= 0.001f || abs32(start.z - end.z) >= 0.001f) && abs32(plane->normal.y - 1.0f) >= 0.001f)
				{
					//attempt stair step
					//	step_flag = true;
				}

				if (collision_normal.y >= 0.2f)
					on_ground = true;

			}
		}
		else
		{
			float ratio = (start_distance + epsilon) / (start_distance - end_distance);

			if (ratio < end_amount)
				end_amount = ratio;
		}
	}

	if (starts_out == false)
	{
		return;
	}

	if (start_amount < end_amount)
	{
		if (start_amount > -1 && start_amount < trace_result)
		{
			if (start_amount < 0)
				start_amount = 0;
			trace_result = start_amount;
		}
	}
}
