#include "include.h"

ShadowVolume::ShadowVolume()
{
	num_vert = 0;
}

void ShadowVolume::AddEdge(int *pEdge, int &num_edge, int v0, int v1)
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

int ShadowVolume::CreateVolume(Graphics &gfx, vec3 *pVertex, int *pIndex, int num_face, vec3 &vLight)
{
	// Allocate a temporary edge list
	int *pEdges = new int[num_face * 6];
	if (pEdges == NULL)
	{
		return -1;
	}
	int num_edge = 0;

	// For each face
	for (int i = 0; i < num_face; i++)
	{
		int wFace0 = pIndex[3 * i + 0];
		int wFace1 = pIndex[3 * i + 1];
		int wFace2 = pIndex[3 * i + 2];

		vec3 v0 = pVertex[wFace0];
		vec3 v1 = pVertex[wFace1];
		vec3 v2 = pVertex[wFace2];

		// Transform vertices
		vec3 vCross1 = v2 - v1;
		vec3 vCross2 = v1 - v0;
		vec3 vNormal;
		vNormal = vec3::crossproduct(vCross1, vCross2);

		if (vNormal * vLight >= 0.0f )
		{
			AddEdge(pEdges, num_edge, wFace0, wFace1);
			AddEdge(pEdges, num_edge, wFace1, wFace2);
			AddEdge(pEdges, num_edge, wFace2, wFace0);
		}
	}

	for (int i = 0; i < num_edge; i++)
	{

		vec3 v1 = pVertex[pEdges[2 * i + 0]];
		vec3 v2 = pVertex[pEdges[2 * i + 1]];
		vec3 v3 = v1 - vLight * 500;
		vec3 v4 = v2 - vLight * 500;

		// Add a quad (two triangles) to the vertex list
		vert_array[num_vert++] = v1;
		vert_array[num_vert++] = v2;
		vert_array[num_vert++] = v3;

		vert_array[num_vert++] = v2;
		vert_array[num_vert++] = v4;
		vert_array[num_vert++] = v3;
	}

	// Delete the temporary edge list
	delete[] pEdges;




	int *index_array = new int [num_vert];

	for( int i = 0; i < num_vert; i++)
	{	
		index_array[i] = i;
	}
	vbo = gfx.CreateVertexBuffer(vert_array, num_vert);
	ibo = gfx.CreateIndexBuffer(index_array, num_vert); 

	delete [] index_array;	
	return 0;
}



int ShadowVolume::Render(Graphics &gfx)
{
	gfx.SelectIndexBuffer(ibo);
	gfx.SelectVertexBuffer(vbo);
	gfx.DrawArrayTri(0, 0, num_vert, num_vert);
	/*
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < m_dwNumVertices; i++)
	{
		glVertex3f(m_pVertices[i].x, m_pVertices[i].y, m_pVertices[i].z);
	}
	glEnd();
	*/
	return 0;
}
