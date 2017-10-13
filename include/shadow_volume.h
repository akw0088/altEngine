#ifndef SHADOW_VOLUME_H
#define SHADOW_VOLUME_H

#include "include.h"


typedef struct
{
	vec3 v1;
	vec3 v2;
	vec3 vNorm;
} edge_t;


class ShadowVolume
{
public:
	ShadowVolume();
	int CreateVolume(vec3 *pVertices, int *pIndices, int dwNumFaces, vec3 &vLight);
	void AddEdge(int *pEdges, int &dwNumEdges, int v0, int v1);
	int Render();

private:
	vec3	vert_array[4096]; // Vertex data for rendering shadow volume
	int     num_vert;
};

#endif