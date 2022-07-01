//=============================================================================
// This file is part of the altEngine distribution
// (https://github.com/akw0088/altEngine/)
// Copyright (c) 2018 Alexander Wright All Rights Reserved.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON - INFRINGEMENT.IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//=============================================================================

// based on https://www.gamedev.net/articles/programming/graphics/an-example-of-shadow-rendering-in-direct3d-9-r2019/ tutorial

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
	for (unsigned int i = 0; i < num_edge; i++)
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

int ShadowVolume::CreateVolume(Graphics &gfx, vertex_t *pVertex, unsigned int *pIndex, unsigned int num_face, vec3 &vLight)
{
	unsigned int num_edge = 0;
	num_vert = 0;

	if (num_face * 9 > alloc_vert)
	{
		if (vert_array != NULL)
			delete[] vert_array;

		if (index_array != NULL)
			delete[] index_array;

		if (pEdges != NULL)
			delete[] pEdges;


		vert_array = new vertex_t[num_face * 9];
		index_array = new unsigned int [num_face * 9];
		pEdges = new int[num_face * 9];

		alloc_vert = num_face * 9;

		for (unsigned int i = 0; i < num_face * 9; i++)
		{
			index_array[i] = i;
		}

	}

	// For each face
	for (unsigned int i = 0; i < num_face; i++)
	{
		unsigned int wFace0 = pIndex[3 * i + 0];
		unsigned int wFace1 = pIndex[3 * i + 1];
		unsigned int wFace2 = pIndex[3 * i + 2];

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



	vec3 far_cap[16384];
	unsigned int num_far_cap = 0;


	vec3 near_cap[16384];
	unsigned int num_near_cap = 0;


	// For each face, keep the ones facing the light as the near cap
	for (unsigned int i = 0; i < num_face; i++)
	{
		int wFace0 = pIndex[3 * i + 0];
		int wFace1 = pIndex[3 * i + 1];
		int wFace2 = pIndex[3 * i + 2];

		vec3 v0 = pVertex[wFace0].position;
		vec3 v1 = pVertex[wFace1].position;
		vec3 v2 = pVertex[wFace2].position;

		// Transform vertices
		vec3 vCross1 = v2 - v1;
		vec3 vCross2 = v1 - v0;
		vec3 vNormal = vec3::crossproduct(vCross1, vCross2);

		if (vNormal * vLight > 0.0f)
		{
			// reverse order to reverse faces
			far_cap[num_far_cap++] = v2 - vLight * 2048;
			far_cap[num_far_cap++] = v1 - vLight * 2048;
			far_cap[num_far_cap++] = v0 - vLight * 2048;
		}
		else
		{
			// reverse order to reverse faces (using parts in shadow reversed to prevent z-fighting)
			near_cap[num_near_cap++] = v2;
			near_cap[num_near_cap++] = v1;
			near_cap[num_near_cap++] = v0;
		}




	}

	for (int i = 0; i < num_edge; i++)
	{
		vec3 v1 = pVertex[pEdges[2 * i + 0]].position;
		vec3 v2 = pVertex[pEdges[2 * i + 1]].position;
		vec3 v3 = v1 - vLight * 2048;
		vec3 v4 = v2 - vLight * 2048;

		// Add a quad (two triangles) to the vertex list
		vert_array[num_vert++].position = v1;
		vert_array[num_vert++].position = v2;
		vert_array[num_vert++].position = v3;

		vert_array[num_vert++].position = v2;
		vert_array[num_vert++].position = v4;
		vert_array[num_vert++].position = v3;
	}


	for (unsigned int i = 0; i < num_near_cap; i += 3)
	{
		vert_array[num_vert++].position = near_cap[i + 0];
		vert_array[num_vert++].position = near_cap[i + 1];
		vert_array[num_vert++].position = near_cap[i + 2];
	}

	for (unsigned int i = 0; i < num_far_cap; i += 3)
	{
		vert_array[num_vert++].position = far_cap[i + 0];
		vert_array[num_vert++].position = far_cap[i + 1];
		vert_array[num_vert++].position = far_cap[i + 2];
	}



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