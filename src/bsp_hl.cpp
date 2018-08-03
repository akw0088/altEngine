#include "include.h"
#include <math.h> //for ceil

using namespace hl;

HLBsp::HLBsp()
{
	loaded = false;
	map_selected = false;
}

int HLBsp::load(Graphics &gfx, char *map)
{
	int size;
	dheader_t *tBsp = (dheader_t *)get_file(map, &size);
	char *pBsp = (char *)tBsp;

	initialized = false;

	if (pBsp == NULL)
	{
		debugf("Failed to open %s", map);
		return -1;
	}

	if (tBsp->magic != HL_BSP_HEADER)
	{
		debugf("Not a valid source engine bsp\n");
		return -1;
	}

	// Version should be 20 assuming counter strike maps
	if (tBsp->version < 17 || tBsp->version >= 29)
	{
		debugf("Unexpected BSP Version %X\n", tBsp->version);
		return -1;
	}


	data.header = (dheader_t *)tBsp;
	data.Entity = (char *)&pBsp[tBsp->lumps[LMP_ENTITIES].offset];
	data.Plane = (dplane_t *)&pBsp[tBsp->lumps[LMP_PLANES].offset];
	data.TexData = (dtexdata_t *)&pBsp[tBsp->lumps[LMP_TEXDATA].offset];
	data.Vert = (vec3 *)&pBsp[tBsp->lumps[LMP_VERTICES].offset];
	data.Vis = (dvis_t *)&pBsp[tBsp->lumps[LMP_VISIBILITY].offset];
	data.Node = (dnode_t *)&pBsp[tBsp->lumps[LMP_NODES].offset];
	data.TexInfo = (dtexinfo_t *)&pBsp[tBsp->lumps[LMP_TEXINFO].offset];
	data.Face = (dface_t *)&pBsp[tBsp->lumps[LMP_FACES].offset];
	data.Lightmap = (ColorRGBExp32 *)&pBsp[tBsp->lumps[LMP_LIGHTING].offset];
	// lump 9 missing LMP_OCCLUSION
	data.Leaf = (dleaf_t *)&pBsp[tBsp->lumps[LMP_LEAFS].offset];
	//lump 11 missing LMP_FACEIDS
	data.Edge = (dedge_t *)&pBsp[tBsp->lumps[LMP_EDGES].offset];
	data.SurfEdge = (int *)&pBsp[tBsp->lumps[LMP_SURFEDGES].offset];
	data.Model = (dmodel_t *)&pBsp[tBsp->lumps[LMP_MODELS].offset];
	//lump 15 missing LMP_WORLDLIGHTS
	data.LeafFace = (unsigned short int *)&pBsp[tBsp->lumps[LMP_LEAFFACES].offset];
	data.LeafBrush = (unsigned short int *)&pBsp[tBsp->lumps[LMP_LEAFBRUSHES].offset];
	data.Brush = (dbrush_t *)&pBsp[tBsp->lumps[LMP_BRUSHES].offset];
	data.BrushSide = (dbrushside_t *)&pBsp[tBsp->lumps[LMP_BRUSHSIDES].offset];
	// huge gap
	data.VertNormal = (vec3 *)&pBsp[tBsp->lumps[LMP_VERTNORMALS].offset]; //LMP_VERTNORMALS Face plane normals 
	data.VertNormalIndex = (int *)&pBsp[tBsp->lumps[LMP_VERTNORMALINDICES].offset];//LMP_VERTNORMALINDICES Face plane normal index array 
	//LUMP_PAKFILE Embedded uncompressed Zip-format file 

	data.Game = (dgamelump_t *)&pBsp[tBsp->lumps[LMP_GAME_LUMP].offset];
	data.StringTable = (int *)&pBsp[tBsp->lumps[LMP_TEXDATA_STRING_TABLE].offset];
	data.StringData = (char *)&pBsp[tBsp->lumps[LMP_TEXDATA_STRING_DATA].offset];


	data.num_entity = tBsp->lumps[LMP_ENTITIES].length;;
	data.num_planes = tBsp->lumps[LMP_PLANES].length / sizeof(dplane_t);
	data.num_texdata = tBsp->lumps[LMP_TEXDATA].length / sizeof(dtexdata_t);
	data.num_verts = tBsp->lumps[LMP_VERTICES].length / sizeof(vec3);
	data.num_vis = tBsp->lumps[LMP_VERTICES].length / sizeof(dvis_t);
	data.num_nodes = tBsp->lumps[LMP_NODES].length / sizeof(dnode_t);
	data.num_texinfo = tBsp->lumps[LMP_TEXINFO].length / sizeof(dtexinfo_t);
	data.num_faces = tBsp->lumps[LMP_FACES].length / sizeof(dface_t);
	data.num_lightmap = tBsp->lumps[LMP_MODELS].length / sizeof(ColorRGBExp32);
	data.num_leafs = tBsp->lumps[LMP_LEAFS].offset / sizeof(dleaf_t);
	data.num_edges = tBsp->lumps[LMP_EDGES].length / sizeof(dedge_t);
	data.num_surfedges = tBsp->lumps[LMP_SURFEDGES].length / sizeof(int);
	data.num_model = tBsp->lumps[LMP_MODELS].length / sizeof(dmodel_t);
	data.num_LeafFaces = tBsp->lumps[LMP_LEAFFACES].length / sizeof(unsigned short int);
	data.num_LeafBrush = tBsp->lumps[LMP_LEAFBRUSHES].length / sizeof(unsigned short int);
	data.num_Brush = tBsp->lumps[LMP_BRUSHES].length / sizeof(dbrush_t);
	data.num_BrushSide = tBsp->lumps[LMP_BRUSHSIDES].length / sizeof(dbrushside_t);
	data.num_VertexNormal = tBsp->lumps[LMP_VERTNORMALS].length / sizeof(vec3); // For lightmaps?
	data.num_VertexNormalIndex = tBsp->lumps[LMP_VERTNORMALINDICES].length / sizeof(int);
	data.num_game = tBsp->lumps[LMP_GAME_LUMP].length / sizeof(dgamelump_t);
	data.num_StringTable = tBsp->lumps[LMP_TEXDATA_STRING_TABLE].length / sizeof(int);
	data.num_StringData = tBsp->lumps[LMP_TEXDATA_STRING_DATA].length;


	map_vertex = new vertex_t[data.num_verts];
	texdata_to_obj = new int[data.num_texdata];
	for (int i = 0; i < data.num_texdata; i++)
	{
		texdata_to_obj[i] = 0;
	}
	face_start_index = new int[data.num_faces];
	face_count = new int[data.num_faces];
	face_lightmap = new vec2[data.num_faces];
	face_lightmap_obj = new int[data.num_faces];

	for (int i = 0; i < data.num_faces; i++)
	{
		face_start_index[i] = 0;
		face_count[i] = 0;
		face_lightmap[i].x = 1.0f;
		face_lightmap[i].y = 1.0f;
		face_lightmap_obj[i] = 0;
	}




	lightmap_object = new int[8192];
	tex_object = new int[8192];

	// write entity string to file
	char name[80];
	sprintf(name, "%s", map);
	strcat(name, ".ent");
	write_file(name, data.Entity, data.num_entity);
	name[strlen(name) - 4] = '\0';
	char *pname = strstr(name, "maps/");

	if (pname)
	{
		sprintf(map_name, "%s", pname + 5);
	}

	change_axis();

	load_lightmap(gfx);
	load_textures(gfx);


	// Need to render once to generate buffers
	for (unsigned int i = 0; i < data.num_verts; i++)
	{
		map_vertex[i].position = data.Vert[i];
		map_vertex[i].tangent = vec4();
	}


	vec3 pos(0, -100000.0f, 0.0f);
	render(gfx, pos);
	initialized = true;


	// generate normals
	for (unsigned int i = 0; i < index_array.size();)
	{
		if (i == 0)
		{
			// Triangle fan, first set will have 3 points

			vec3 a = data.Vert[index_array[i + 1]] - data.Vert[index_array[i + 0]];
			vec3 b = data.Vert[index_array[i + 2]] - data.Vert[index_array[i + 0]];
			vec3 normal = vec3::crossproduct(a, b);

			map_vertex[index_array[i + 0]].normal = normal;
			map_vertex[index_array[i + 1]].normal = normal;
			map_vertex[index_array[i + 2]].normal = normal;

			i += 3;
		}
		else
		{
			// rest will be one new point and two previous
			vec3 a = data.Vert[index_array[i - 1]] - data.Vert[index_array[i - 2]];
			vec3 b = data.Vert[index_array[i + 0]] - data.Vert[index_array[i - 2]];
			vec3 normal = vec3::crossproduct(a, b);

			map_vertex[index_array[i + 0]].normal = normal;
			i++;
		}
	}

	map_vertex_vbo = gfx.CreateVertexBuffer(map_vertex, data.num_verts);
	map_index_vbo = gfx.CreateIndexBuffer(index_array.data(), index_array.size());

	loaded = true;

	return 0;
}

void HLBsp::render(Graphics &gfx, vec3 &pos)
{
	int current_leaf = find_leaf(pos, 0);
	map_selected = false;

	if (initialized == false)
		current_leaf = -1;

	bsp_render_node(gfx, 0, current_leaf, pos);
}

void HLBsp::bsp_render_node(Graphics &gfx, int node_index, int leaf, vec3 pos)
{
	if (node_index < 0)
	{
		if (node_index == -1)
			return;

		render_leaf(gfx, ~node_index);
		return;
	}

	float distance;

	if (data.Plane[data.Node[node_index].planenum].type == 0)
		distance = pos.x - data.Plane[data.Node[node_index].planenum].distance;
	else if (data.Plane[data.Node[node_index].planenum].type == 1)
		distance = pos.y - data.Plane[data.Node[node_index].planenum].distance;
	else if (data.Plane[data.Node[node_index].planenum].type == 2)
		distance = pos.z - data.Plane[data.Node[node_index].planenum].distance;
	else
		distance = data.Plane[data.Node[node_index].planenum].normal * pos - data.Plane[data.Node[node_index].planenum].distance;

	if (distance > 0)
	{
		bsp_render_node(gfx, data.Node[node_index].children[1], leaf, pos);
		bsp_render_node(gfx, data.Node[node_index].children[0], leaf, pos);
	}

	else
	{
		bsp_render_node(gfx, data.Node[node_index].children[0], leaf, pos);
		bsp_render_node(gfx, data.Node[node_index].children[1], leaf, pos);
	}

}

void HLBsp::render_leaf(Graphics &gfx, int leaf)
{
	for (int i = 0; i < data.Leaf[leaf].numleaffaces; i++)
	{
		if (initialized)
		{
			render_face(gfx, data.LeafFace[data.Leaf[leaf].firstleafface + i]);
		}
		else
		{
			build_face(data.LeafFace[data.Leaf[leaf].firstleafface + i]);
		}
	}
}

void HLBsp::render_face(Graphics &gfx, int face)
{
	int tex_index = data.Face[face].texinfo;
	int tex_data = data.TexInfo[tex_index].texdata;

	gfx.SelectTexture(0, texdata_to_obj[tex_data]);
	//gfx.SelectTexture(8, face_lightmap_obj[tex_data]);

	if (map_selected == false)
	{
		gfx.SelectIndexBuffer(map_index_vbo);
		gfx.SelectVertexBuffer(map_vertex_vbo);
		map_selected = true;
	}
	gfx.DrawArrayTri(face_start_index[face], 0, face_count[face], face_count[face]);
}

void HLBsp::calculate_texcoords(int face, int edge0, int edge1, int edge2)
{
	int tex_index = data.Face[face].texinfo;
	int tex_data = data.TexInfo[tex_index].texdata;
	int width = data.TexData[tex_data].width;
	int height = data.TexData[tex_data].height;
	int tex_name = data.TexData[tex_data].nameStringTableID;

	vec3 tu(data.TexInfo[tex_index].textureVecs[0].x, data.TexInfo[tex_index].textureVecs[0].y, data.TexInfo[tex_index].textureVecs[0].z);
	vec3 tv(data.TexInfo[tex_index].textureVecs[1].x, data.TexInfo[tex_index].textureVecs[1].y, data.TexInfo[tex_index].textureVecs[1].z);
	vec3 ltu(data.TexInfo[tex_index].lightmapVecs[0].x, data.TexInfo[tex_index].lightmapVecs[0].y, data.TexInfo[tex_index].lightmapVecs[0].z);
	vec3 ltv(data.TexInfo[tex_index].lightmapVecs[1].x, data.TexInfo[tex_index].lightmapVecs[1].y, data.TexInfo[tex_index].lightmapVecs[1].z);


	// Only calculate vertex 1 once
	if (face_start_index[face] == 0)
	{
		float u1 = tu * data.Vert[edge0] + data.TexInfo[tex_index].textureVecs[0].w;
		float v1 = tv * data.Vert[edge0] + data.TexInfo[tex_index].textureVecs[1].w;

		map_vertex[edge0].texCoord0.x = u1 / width;
		map_vertex[edge0].texCoord0.y = v1 / height;

		float lu1 = ltu * data.Vert[edge0] + data.TexInfo[tex_index].lightmapVecs[0].w;;
		float lv1 = ltu * data.Vert[edge0] + data.TexInfo[tex_index].lightmapVecs[1].w;;

		map_vertex[edge0].texCoord1.x = lu1 / face_lightmap[face].x;
		map_vertex[edge0].texCoord1.y = lv1 / face_lightmap[face].y;
	}


	//vertex 2
	float u2 = tu * data.Vert[edge1] + data.TexInfo[tex_index].textureVecs[0].w;
	float v2 = tv * data.Vert[edge1] + data.TexInfo[tex_index].textureVecs[1].w;

	map_vertex[edge1].texCoord0.x = u2 / width;
	map_vertex[edge1].texCoord0.y = v2 / height;

	float lu2 = ltu * data.Vert[edge1] + data.TexInfo[tex_index].lightmapVecs[0].w;;
	float lv2 = ltu * data.Vert[edge1] + data.TexInfo[tex_index].lightmapVecs[1].w;;

	map_vertex[edge1].texCoord1.x = lu2 / face_lightmap[face].x;
	map_vertex[edge1].texCoord1.y = lv2 / face_lightmap[face].y;


	// vertex 3
	float u3 = tu * data.Vert[edge2] + data.TexInfo[tex_index].textureVecs[0].w;
	float v3 = tv * data.Vert[edge2] + data.TexInfo[tex_index].textureVecs[1].w;

	map_vertex[edge2].texCoord0.x = u3 / width;
	map_vertex[edge2].texCoord0.y = v3 / height;

	float lu3 = ltu * data.Vert[edge2] + data.TexInfo[tex_index].lightmapVecs[0].w;;
	float lv3 = ltu * data.Vert[edge2] + data.TexInfo[tex_index].lightmapVecs[1].w;;

	map_vertex[edge2].texCoord1.x = lu3 / face_lightmap[face].x;
	map_vertex[edge2].texCoord1.y = lv3 / face_lightmap[face].y;

}

void HLBsp::build_face(int face)
{
	int edge0;
	int edge1;
	int edge2;

	// usual case is four edges making a quad, can render as triangle fan
	for (int i = 0; i < data.Face[face].numedges; i++)
	{
		int edge_index = data.SurfEdge[data.Face[face].firstedge + i];
		dedge_t edge = data.Edge[abs32(edge_index)];
		
		// if edge is negative, edges are reversed (first edge is index 1, like quake2)
		bool reverse = (edge_index < 0);

		if (i == 0)
		{
			edge0 = edge.v[reverse ? 1 : 0];	// first edge, used in all triangles
		}
		else
		{
			// if first edge equals edge0, skip
			if (edge.v[reverse ? 1 : 0] == edge0)
				continue;

			// if second edge equals edge0 skip
			if (edge.v[reverse ? 0 : 1] == edge0)
				continue;
			
			edge1 = edge.v[reverse ? 1 : 0]; // first edge
			edge2 = edge.v[reverse ? 0 : 1]; // second edge

			calculate_texcoords(face, edge0, edge1, edge2);
			if (face_start_index[face] == 0)
			{
				face_start_index[face] = index_array.size();
			}

			index_array.push_back(edge0);
			index_array.push_back(edge1);
			index_array.push_back(edge2);
			face_count[face] += 3;
		}
	}
}

int HLBsp::find_leaf(vec3 pos, int node)
{
	for (int x = 0; x < 2; x++)
	{
		if (data.Node[node].children[x] >= 0)
		{
			if (point_AABB(pos, data.Node[data.Node[node].children[x]].mins, data.Node[data.Node[node].children[x]].maxs))
				return find_leaf(pos, data.Node[node].children[x]);
		}

		else if (~data.Node[node].children[x] != 0)
		{
			if (point_AABB(pos, data.Leaf[~(data.Node[node].children[x])].mins, data.Leaf[~(data.Node[node].children[x])].maxs))
				return ~data.Node[node].children[x];
		}
	}

	return -1;
}

bool HLBsp::point_AABB(vec3 pos, short min[3], short max[3])
{
	if ((min[0] <= pos.x && pos.x <= max[0] && min[1] <= pos.y && pos.y <= max[1] && min[2] <= pos.z && pos.z <= max[2]) ||
		(min[0] >= pos.x && pos.x >= max[0] && min[1] >= pos.y && pos.y >= max[1] && min[2] >= pos.z && pos.z >= max[2]))
	{
		return true;
	}
	else
	{
		return false;
	}
}


/*
Converts axis from quake3 to opengl format
*/
void HLBsp::change_axis()
{
	for (unsigned int i = 0; i < data.num_verts; i++)
	{
		vec3 *vert = &data.Vert[i];
		float temp;

		temp = vert->y;
		vert->y = vert->z;
		vert->z = -temp;
	}

	for (unsigned int i = 0; i < data.num_VertexNormal; i++)
	{
		vec3 *vert = &data.VertNormal[i];
		float temp;

		temp = vert->y;
		vert->y = vert->z;
		vert->z = -temp;
	}


	//	data.Plane[plane_index].normal
	for (unsigned int i = 0; i < data.num_planes; i++)
	{
		dplane_t *plane = &data.Plane[i];
		float	temp;

		temp = plane->normal.y;
		plane->normal.y = plane->normal.z;
		plane->normal.z = -temp;

		//		plane->d *= (1.0f / UNITS_TO_METERS);
	}

	for (unsigned int i = 0; i < data.num_leafs; i++)
	{
		int temp = data.Leaf[i].mins[1];
		data.Leaf[i].mins[1] = data.Leaf[i].mins[2];
		data.Leaf[i].mins[2] = -temp;

		temp = data.Leaf[i].maxs[1];
		data.Leaf[i].maxs[1] = data.Leaf[i].maxs[2];
		data.Leaf[i].maxs[2] = -temp;
	}

	for (unsigned int i = 0; i < data.num_nodes; i++)
	{
		int temp = data.Node[i].mins[1];
		data.Node[i].mins[1] = data.Node[i].mins[2];
		data.Node[i].mins[2] = -temp;

		temp = data.Node[i].maxs[1];
		data.Node[i].maxs[1] = data.Node[i].maxs[2];
		data.Node[i].maxs[2] = -temp;
	}


	for (unsigned int i = 0; i < data.num_texinfo; i++)
	{
		float temp = data.TexInfo[i].textureVecs[0].y;
		data.TexInfo[i].textureVecs[0].y = data.TexInfo[i].textureVecs[0].z;
		data.TexInfo[i].textureVecs[0].z = -temp;

		temp = data.TexInfo[i].textureVecs[1].y;
		data.TexInfo[i].textureVecs[1].y = data.TexInfo[i].textureVecs[1].z;
		data.TexInfo[i].textureVecs[1].z = -temp;
	}

}



void HLBsp::load_textures(Graphics &gfx)
{
	int loaded = 0;
	int attempted = 0;
	vector<char *> texture_list;


	for (unsigned int i = 0; i < data.num_faces; i++)
	{
		bool duplicate = false;
		char filename[128];
		dtexinfo_t *texinfo = &data.TexInfo[data.Face[i].texinfo];
		dtexdata_t *texdata = &data.TexData[texinfo->texdata];
		int string_offset = data.StringTable[texdata->nameStringTableID];
		char *texture = &data.StringData[string_offset];

		sprintf(filename, "media/materials/%s.tga", texture);

		for (unsigned int i = 0; i < texture_list.size(); i++)
		{
			if (strcmp(filename, texture_list[i]) == 0)
			{
				duplicate = true;
				break;
			}
		}

		if (duplicate)
		{
			continue;
		}

		tex_object[i] = load_texture(gfx, filename, false, false, 0);
		if (tex_object[i] != 0)
		{
			texdata_to_obj[texinfo->texdata] = tex_object[i];
			printf("loaded %s\n", filename);
			loaded++;
			attempted++;
		}
		else
		{
			printf("failed to load %s\n", filename);
			attempted++;
		}
		char *file = new char[128];
		sprintf(file, "%s", filename);
		texture_list.push_back(file);



		//texinfo->textureVecs[0] -- need to set texcoords too
	}
	printf("loaded %d of %d textures\n", loaded, attempted);

	for (unsigned int i = 0; i < texture_list.size(); i++)
	{
		delete[] texture_list[i];
	}

	// Could just loop through string table to avoid duplicates, but this gives height/width info

	// So a GCF file is packet and a VTF texture is inside along with text data vmt data
	// Could use devil 
}

void HLBsp::load_lightmap(Graphics &gfx)
{
	for (unsigned int i = 0; i < data.num_faces; i++)
	{
		vec2 min(8192, 8192);
		vec2 max(-8192, -8192);

		if (data.Face[i].styles[0] != 0 || (signed)data.Face[i].lightofs == -1)
		{
			continue;
		}

		dtexinfo_t *info = &data.TexInfo[data.Face[i].texinfo];

		//int w = data.Face[i].LightmapTextureSizeInLuxels[0] + 1;
		//int h = data.Face[i].LightmapTextureSizeInLuxels[1] + 1;



		// calculate texture coordinates: https://www.gamedev.net/forums/topic/538713-bspv38-quake-2-bsp-loader-lightmap-problem/
#if 1
		//go through every vertex of the face, and calculate their UV co-ordinates 
		for (int j = 0; j < data.Face[i].numedges; j++)
		{
			vec3 v;
			int edge_index = data.SurfEdge[data.Face[i].firstedge + j];

			if (edge_index >= 0)
				v = data.Vert[data.Edge[edge_index].v[0]];
			else
				v = data.Vert[data.Edge[-edge_index].v[1]];

			float x = info->lightmapVecs[0].x * v.x + info->lightmapVecs[0].y * v.y + info->lightmapVecs[0].z * v.z + info->lightmapVecs[0].w;
			float y = info->lightmapVecs[1].x * v.x + info->lightmapVecs[1].y * v.y + info->lightmapVecs[1].z * v.z + info->lightmapVecs[1].w;

			if (x < min.x)
				min.x = x;
			if (x > max.x)
				max.x = x;

			if (y < min.y)
				min.y = y;
			if (y > max.y)
				max.y = y;
		}

		int width = (int)(ceil(max.x / 16.0f) - floor(min.x / 16.0f)) + 1;
		int height = (int)(ceil(max.y / 16.0f) - floor(min.y / 16.0f)) + 1;
		face_lightmap[i].x = width;
		face_lightmap[i].y = height;



#ifndef DEDICATED
#ifndef VULKAN //GL_RGBA not defined
#ifndef SOFTWARE
		lightmap_object[i] = gfx.LoadTexture(width, height, GL_RGBA, GL_RGBA, (void *)&data.Lightmap[data.Face[i].lightofs / sizeof(ColorRGBExp32)], false, 0);
		face_lightmap_obj[i] = lightmap_object[i];
#endif
#endif
#endif


		//generate a texture coordinate that's in the range 0.0 to 1.0
		vec2 mid_poly((min.x + max.x) * 0.5f,
			(min.y + max.y) * 0.5f);

		vec2 mid_tex(width * 0.5f,
			height * 0.5f);

		for (int j = 0; j < data.Face[i].numedges; ++j)
		{
			vec3 v;
			int edge_index = data.SurfEdge[data.Face[i].firstedge + j];

			if (edge_index >= 0)
				v = data.Vert[data.Edge[edge_index].v[0]];
			else
				v = data.Vert[data.Edge[-edge_index].v[1]];

			float x = info->lightmapVecs[0].x * v.x +
				info->lightmapVecs[0].y * v.y +
				info->lightmapVecs[0].z * v.z +
				info->lightmapVecs[0].w;

			float y = info->lightmapVecs[1].x * v.x +
				info->lightmapVecs[1].y * v.y +
				info->lightmapVecs[1].z * v.z +
				info->lightmapVecs[1].w;

			vec2 lightmap(mid_tex.x + (x - mid_poly.x) / 16.0f,
				mid_tex.y + (y - mid_poly.y) / 16.0f);

			vec2 coord(lightmap.x / (float)width, lightmap.y / (float)height);

			if (edge_index >= 0)
			{
				map_vertex[data.Edge[edge_index].v[0]].texCoord0 = coord;
				map_vertex[data.Edge[edge_index].v[0]].texCoord1 = coord;
			}
			else
			{
				map_vertex[data.Edge[-edge_index].v[1]].texCoord0 = coord;
				map_vertex[data.Edge[-edge_index].v[1]].texCoord1 = coord;
			}
		}
#endif

	}
}
