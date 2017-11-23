#include "include.h"

#ifndef ISOCUBE_H
#define ISOCUBE_H

class IsoCube
{
public:
	IsoCube();
	void init_cube();
	int load(Graphics &gfx, char *texture_str, char *heightmap, int depth, float scale);
	void drawTriangle(const vertex_t &v1, const vertex_t &v2, const vertex_t &v3, float scale, unsigned char *image, int width, int height);
	void subdivide(const vertex_t &v1, const vertex_t &v2, const vertex_t &v3, int depth, float scale, unsigned char *image, int width, int height);
	void generate(int depth, float scale, unsigned char *image, int width, int height);
	void render(Graphics &gfx);
	void destroy(Graphics &gfx);

private:
	bool loaded;
	vertex_t *vertex_array;
	unsigned int *index_array;

	unsigned int num_vert;
	unsigned int num_index;

	int iso_tex;
	int ibo;
	int vbo;
};

#endif
