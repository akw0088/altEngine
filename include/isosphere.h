#include "include.h"

#ifndef ISOSPHERE_H
#define ISOSPHERE_H

// Icosphere? IsoSphere? oh wells
class IsoSphere
{
public:
	IsoSphere();
	int load(Graphics &gfx, char *texture_str, char *heightmap, int depth, float scale, const vec3 &offset);
	void drawTriangle(const vec3 &v1, const vec3 &v2, const vec3 &v3, float scale, unsigned char *image, int width, int height, const vec3 &offset);
	void subdivide(const vec3 &v1, const vec3 &v2, const vec3 &v3, int depth, float scale, unsigned char *image, int width, int height, const vec3 &offset);
	void generate(int depth, float scale, unsigned char *image, int width, int height, const vec3 &offset);
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
