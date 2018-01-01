#include "include.h"

#ifndef HLBSP_H
#define HLBSP_H

class HLBsp
{
public:
	int load(Graphics &gfx, char *file);
	void render(vec3 &pos);
	void temp_render(Graphics &gfx);

private:
	void render_leaf (int leaf);
	void render_face(int face);

	int find_leaf(vec3 pos, int node);
	void bsp_render_node(int node, int leaf, vec3 pos);
	bool point_AABB(vec3 pos, short min[3], short max[3]);


	std::vector <unsigned int> index;
	hl_bsp_t data;

	int map_vertex_vbo;
	int map_index_vbo;
};

#endif