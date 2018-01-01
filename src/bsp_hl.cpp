#include "include.h"

HLBsp::HLBsp()
{
	loaded = false;
}

int HLBsp::load(Graphics &gfx, char *file)
{
	int size;
	dheader_t *tBsp = (dheader_t *)get_file(file, &size);
	char *pBsp = (char *)tBsp;

	if (pBsp == NULL)
	{
		debugf("Failed to open %s", file);
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
	data.Node = (dnode_t *)&pBsp[tBsp->lumps[LMP_NODES].offset];
	data.Leaf = (dleaf_t *)&pBsp[tBsp->lumps[LMP_LEAFS].offset];
	data.Face = (dface_t *)&pBsp[tBsp->lumps[LMP_FACES].offset];
	data.Edge = (dedge_t *)&pBsp[tBsp->lumps[LMP_EDGES].offset];
	data.SurfEdge = (int *)&pBsp[tBsp->lumps[LMP_SURFEDGES].offset];
	data.LeafFace = (unsigned short int*)&pBsp[tBsp->lumps[LMP_LEAFFACES].offset];
	data.Vert = (vec3 *)&pBsp[tBsp->lumps[LMP_VERTICES].offset];
	data.Plane = (dplane_t *)&pBsp[tBsp->lumps[LMP_PLANES].offset];

	data.num_nodes = tBsp->lumps[LMP_NODES].length / sizeof(dnode_t);
	data.num_leafs = tBsp->lumps[LMP_LEAFS].offset / sizeof(dleaf_t);
	data.num_faces = tBsp->lumps[LMP_FACES].length / sizeof(dface_t);
	data.num_edges = tBsp->lumps[LMP_EDGES].length / sizeof(dedge_t);
	data.num_surfedges = tBsp->lumps[LMP_SURFEDGES].length / sizeof(int);
	data.num_LeafFaces = tBsp->lumps[LMP_LEAFFACES].length / sizeof(unsigned short int);
	data.num_verts = tBsp->lumps[LMP_VERTICES].length / sizeof(vec3);
	data.num_planes = tBsp->lumps[LMP_PLANES].length / sizeof(dplane_t);

	vertex_t *map_vertex = new vertex_t[data.num_verts];


	change_axis();

	// generate index array, isnt using PVS
	render(vec3());


	for (int i = 0; i < data.num_verts; i++)
	{
		map_vertex[i].position = data.Vert[i];// +vec3(-11584, -5088, 2050); //offset for dust2
		map_vertex[i].texCoord0 = vec2(data.Vert[i].x, data.Vert[i].y); //just to have something
		map_vertex[i].texCoord1 = vec2(data.Vert[i].x, data.Vert[i].y); //just to have something
		map_vertex[i].tangent = vec4();
	}

	// generate normals
	for (int i = 0; i < index.size();)
	{
		if (i == 0)
		{
			// Triangle fan, first set will have 3 points

			vec3 a = data.Vert[index[i + 1]] - data.Vert[index[i + 0]];
			vec3 b = data.Vert[index[i + 2]] - data.Vert[index[i + 0]];
			vec3 normal = vec3::crossproduct(a, b);

			map_vertex[index[i + 0]].normal = normal;
			map_vertex[index[i + 1]].normal = normal;
			map_vertex[index[i + 2]].normal = normal;

			i += 3;
		}
		else
		{
			// rest will be one new point and two previous
			vec3 a = data.Vert[index[i - 1]] - data.Vert[index[i - 2]];
			vec3 b = data.Vert[index[i + 0]] - data.Vert[index[i - 2]];
			vec3 normal = vec3::crossproduct(a, b);

			map_vertex[index[i + 0]].normal = normal;
			i++;
		}
	}

	map_vertex_vbo = gfx.CreateVertexBuffer(map_vertex, data.num_verts);
	map_index_vbo = gfx.CreateIndexBuffer(index.data(), index.size());

	loaded = true;

	return 0;
}

void HLBsp::temp_render(Graphics &gfx)
{
	gfx.SelectVertexBuffer(map_vertex_vbo);
	gfx.SelectIndexBuffer(map_index_vbo);
	gfx.DrawArrayTri(0, 0, index.size(), data.num_verts);

}

void HLBsp::render(vec3 &pos)
{
	int curLeaf = find_leaf(pos, 0);
	bsp_render_node(0, curLeaf, pos);
}

void HLBsp::bsp_render_node(int node_index, int leaf, vec3 pos)
{
	if (node_index < 0)
	{
		if (node_index == -1)
			return;

		render_leaf (~node_index);
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
		bsp_render_node(data.Node[node_index].children[1], leaf, pos);
		bsp_render_node(data.Node[node_index].children[0], leaf, pos);
	}

	else
	{
		bsp_render_node(data.Node[node_index].children[0], leaf, pos);
		bsp_render_node(data.Node[node_index].children[1], leaf, pos);
	}

}

void HLBsp::render_leaf(int leaf)
{
	for (int i = 0; i < data.Leaf[leaf].numleaffaces; i++)
	{
		render_face(data.LeafFace[data.Leaf[leaf].firstleafface + i]);
	}
}

void HLBsp::render_face(int face)
{
	int shared;
	int edge0;
	int edge1;
	int edge2;
	int edge3;

	// usual case is four edges making a quad
	for (int i = 0; i < data.Face[face].numedges; i++)
	{
		int edge_index = data.SurfEdge[data.Face[face].firstedge + i];
		dedge_t edge = data.Edge[abs32(edge_index)];
		bool reverse = (edge_index >= 0);

		if (i == 0)
		{
			edge0 = edge.v[reverse ? 0 : 1];
			shared = edge.v[reverse ? 1 : 0];
		}
		else
		{
			shared = edge.v[reverse ? 0 : 1];
			if (shared == edge0)
				continue;
			else
				edge1 = shared;

			shared = edge.v[reverse ? 1 : 0];
			if (shared == edge0)
				continue;
			else
				edge2 = shared;

			index.push_back(edge0);
			index.push_back(edge1);
			index.push_back(edge2);
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
	if ((min[0] <= pos.x && pos.x <= max[0] && 	min[1] <= pos.y && pos.y <= max[1] && min[2] <= pos.z && pos.z <= max[2]) ||
		(min[0] >= pos.x && pos.x >= max[0] && 	min[1] >= pos.y && pos.y >= max[1] && min[2] >= pos.z && pos.z >= max[2]))
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
}