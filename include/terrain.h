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

#ifndef TERRAIN_H
#define TERRAIN_H

class Terrain
{
public:
	Terrain();
	int load(Graphics &gfx, float scale, vec3 &offset, char *heightmap, char *texture_str, int anisotropic);
	int CreateMesh(char *heightmap, float scale_width, float scale_height, vec3 &offset, vertex_t *&vertex, unsigned int *&index, unsigned int &num_vertex, unsigned int &num_index);
	void render(Graphics &gfx);
	bool collision_detect(RigidBody &body);
	float GetHeight(const vec3 &position, vec3 &normal);
	void Smooth(vertex_t *image, int width, int height, vec3 &avg);
	int CreateWater(float scale_width, float scale_height, vertex_t *&vertex, unsigned int *&index, unsigned int &num_vertex, unsigned int &num_index);

	void CreateSphere(char *heightmap, float radius, vec3 &offset, vertex_t *&vertex, unsigned int *&index, unsigned int &num_vertex, unsigned int &num_index);

	float height;
	int x_index;
	int y_index;
	int top;
private:

	float size;
	vec3 offset;
	int num_col;
	int num_row;
	float trilength;
	bool loaded;


	vec3 avg;
	vertex_t *water_array;
	unsigned int *water_index;
	unsigned int water_nvertex;
	unsigned int water_nindex;
	int water_vbo;
	int water_ibo;
	int water_tex;




	vertex_t *vertex_array;
	unsigned int *index_array;
	unsigned int num_vertex;
	unsigned int num_index;
	int vbo;
	int ibo;
	int terrain_tex;
};

#endif
