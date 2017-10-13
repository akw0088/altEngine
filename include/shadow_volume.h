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
	int CreateVolume(vec3 *pVertex, int *pIndex, int num_vert, vec3 &light_pos);
	void AddEdge(int *pEdge, int &num_edges, int v0, int v1);
	int Render();

private:
	vec3	vert_array[4096]; // Vertex data for rendering shadow volume
	int     num_vert;



	int vbo;
	int ibo;
};


#endif
