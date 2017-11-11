#include "include.h"

#ifndef TERRAIN_H
#define TERRAIN_H

class Terrain
{
public:
	Terrain();
	int load(Graphics &gfx, char *heightmap, char *texture_str, bool sphere, int anisotropic);
	int CreateMesh(char *heightmap, float scale_width, float scale_height, vertex_t *&vertex, unsigned int *&index, unsigned int &num_vertex, unsigned int &num_index);
	void render(Graphics &gfx);
	bool collision_detect(RigidBody &body);


	void CreateSphere(char *heightmap, float radius, vertex_t *&vertex, unsigned int *&index, unsigned int &num_vertex, unsigned int &num_index);

private:

	float size;
	vec3 offset;
	int num_col;
	int num_row;
	float trilength;


	vertex_t *vertex_array;
	unsigned int *index_array;
	unsigned int num_vertex;
	unsigned int num_index;
	int vbo;
	int ibo;
	int terrain_tex;
};

#endif
