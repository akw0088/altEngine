#include "include.h"

ShadowVolume::ShadowVolume()
{
	num_vert = 0;
	vbo = -1;
	ibo = -1;
	position = vec3(0.0f, 0.0f, 0.0f);
	alloc_vert = 0;
	alloc_edge = 0;

	vert_array = NULL;
	pEdges = NULL;
	index_array = NULL;

}

void ShadowVolume::AddEdge(int *pEdge, unsigned int &num_edge, int v0, int v1)
{
	// Remove interior edges (which appear in the list twice)
	for (int i = 0; i < num_edge; i++)
	{
		if ((pEdge[2 * i + 0] == v0 && pEdge[2 * i + 1] == v1) ||
			(pEdge[2 * i + 0] == v1 && pEdge[2 * i + 1] == v0))
		{
			if (num_edge > 1)
			{
				pEdge[2 * i + 0] = pEdge[2 * (num_edge - 1) + 0];
				pEdge[2 * i + 1] = pEdge[2 * (num_edge - 1) + 1];
			}
			num_edge--;
			return;
		}
	}

	pEdge[2 * num_edge + 0] = v0;
	pEdge[2 * num_edge + 1] = v1;
	num_edge++;
}

int ShadowVolume::CreateVolume(Graphics &gfx, vertex_t *pVertex, unsigned int *pIndex, unsigned int start_index, unsigned int num_face, vec3 &vLight)
{
	unsigned int num_edge = 0;
	num_vert = 0;

	if (num_face * 6 > alloc_vert)
	{
		if (vert_array != NULL)
			delete[] vert_array;

		if (index_array != NULL)
			delete[] index_array;

		if (pEdges != NULL)
			delete[] pEdges;


		vert_array = new vertex_t[num_face * 6];
		index_array = new unsigned int [num_face * 6];
		pEdges = new int[num_face * 6];

		alloc_vert = num_face * 6;

		for (unsigned int i = 0; i < num_face * 6; i++)
		{
			index_array[i] = i;
		}

	}

	// For each face
	for (unsigned int i = 0; i < num_face; i++)
	{
		unsigned int wFace0 = pIndex[3 * i + 0 + start_index];
		unsigned int wFace1 = pIndex[3 * i + 1 + start_index];
		unsigned int wFace2 = pIndex[3 * i + 2 + start_index];

		vec3 v0 = pVertex[wFace0].position;
		vec3 v1 = pVertex[wFace1].position;
		vec3 v2 = pVertex[wFace2].position;

		// Transform vertices
		vec3 vCross1 = v2 - v1;
		vec3 vCross2 = v1 - v0;
		vec3 vNormal = vec3::crossproduct(vCross1, vCross2);

		if (vNormal * vLight <= 0.0f )
		{
			AddEdge(pEdges, num_edge, wFace0, wFace1);
			AddEdge(pEdges, num_edge, wFace1, wFace2);
			AddEdge(pEdges, num_edge, wFace2, wFace0);
		}
	}

	for (unsigned int i = 0; i < num_edge; i++)
	{
		vec3 v1 = pVertex[pEdges[2 * i + 0]].position;
		vec3 v2 = pVertex[pEdges[2 * i + 1]].position;
		vec3 v3 = v1 - vLight * 1024;
		vec3 v4 = v2 - vLight * 1024;

		// Add a quad (two triangles) to the vertex list
		vert_array[num_vert++].position = v1;
		vert_array[num_vert++].position = v2;
		vert_array[num_vert++].position = v3;

		vert_array[num_vert++].position = v2;
		vert_array[num_vert++].position = v4;
		vert_array[num_vert++].position = v3;
	}

	/*
	// Attempted to use dynamic draw vs delete / recreate, no noticable improvement
	if (vbo == -1)
	{
		vbo = gfx.CreateVertexBuffer(vert_array, MAX_VERT, true);
	}
	else
	{
		gfx.SelectVertexBuffer(vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, num_vert * sizeof(vertex_t), (void *)vert_array);
	}

	if (ibo == -1)
	{
		ibo = gfx.CreateIndexBuffer(index_array, MAX_VERT);
	}
	*/

	if (vbo != -1)
	{
		gfx.DeleteVertexBuffer(vbo);
		vbo = -1;
	}

	if (ibo != -1)
	{
		gfx.DeleteIndexBuffer(ibo);
		ibo = -1;
	}

	vbo = gfx.CreateVertexBuffer(vert_array, num_vert);
	ibo = gfx.CreateIndexBuffer(index_array, num_vert);


	return 0;
}



int ShadowVolume::render(Graphics &gfx)
{
	gfx.SelectIndexBuffer(ibo);
	gfx.SelectVertexBuffer(vbo);
	gfx.DrawArrayTri(0, 0, num_vert, num_vert);
	return 0;
}


void ShadowVolume::destroy(Graphics &gfx)
{
	if (vbo != -1)
	{
		gfx.DeleteVertexBuffer(vbo);
		vbo = -1;
	}

	if (ibo != -1)
	{
		gfx.DeleteIndexBuffer(ibo);
		ibo = -1;
	}
}