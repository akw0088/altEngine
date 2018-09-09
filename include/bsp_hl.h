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

#include "include.h"

#ifndef HLBSP_H
#define HLBSP_H

class HLBsp
{
public:
	HLBsp();
	int load(Graphics &gfx, char *file);
	void render(Graphics &gfx, vec3 &pos);

	bool loaded;
	char map_name[128];

private:
	void render_leaf (Graphics &gfx, int leaf);
	void build_face(int face);
	void render_face(Graphics &gfx, int face);

	int find_leaf(vec3 pos, int node);
	void bsp_render_node(Graphics &gfx, int node_index, int leaf, vec3 pos);
	bool point_AABB(vec3 pos, short min[3], short max[3]);
	void change_axis();
	void load_lightmap(Graphics &gfx);
	void load_textures(Graphics &gfx);
	void calculate_texcoords(int face, int edge0, int edge1, int edge2);

	std::vector <unsigned int> index_array;
	hl::hl_bsp_t data;

	int *face_start_index;
	int *face_count;
	vec2 *face_lightmap;
	int *face_lightmap_obj;

	bool map_selected;
	int map_vertex_vbo;
	int map_index_vbo;
	int *lightmap_object;
	int *tex_object;
	vertex_t *map_vertex;
	int *texdata_to_obj;
	bool initialized;
};

#endif