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

#ifndef SHADOW_VOLUME_H
#define SHADOW_VOLUME_H

#include "include.h"

typedef struct
{
	vec3 v1;
	vec3 v2;
	vec3 vNorm;
} edge_t;

class Graphics;

class ShadowVolume
{
public:
	ShadowVolume();
	int CreateVolume(Graphics &gfx, vertex_t *pVertex, unsigned int *pIndex, unsigned int start_index, unsigned int num_face, vec3 &vLight);
	void AddEdge(int *pEdge, unsigned int &num_edges, int v0, int v1);
	int render(Graphics &gfx);
	void destroy(Graphics &gfx);

	matrix3 morientation;
	vec3 position;
	int	vbo;
	int	ibo;

private:
	vertex_t	*vert_array; // Vertex data for rendering shadow volume
	unsigned int	*index_array;
	int			*pEdges;	// temporary edge list
	unsigned int			num_vert;

	unsigned int alloc_vert;
	unsigned int alloc_edge;

};


#endif
