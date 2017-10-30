#ifndef SHADOW_VOLUME_H
#define SHADOW_VOLUME_H

#include "include.h"

#define MAX_VERT 10 * 4096

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
	int CreateVolume(Graphics &gfx, vertex_t *pVertex, int *pIndex, int start_index, int num_face, vec3 &vLight);
	void AddEdge(int *pEdge, int &num_edges, int v0, int v1);
	int render(Graphics &gfx);
	void destroy(Graphics &gfx);

	matrix3 morientation;
	vec3 position;
private:
	vertex_t	vert_array[MAX_VERT]; // Vertex data for rendering shadow volume
	int			index_array[MAX_VERT];
	int			pEdges[MAX_VERT * 2];	// temporary edge list
	int			num_vert;


	int	vbo;
	int	ibo;
};


#endif
