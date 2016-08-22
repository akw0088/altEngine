#include "include.h"

#ifndef MD5MODEL_H
#define MD5MODEL_H

class MD5Model
{
public:
	void load(char *md5file, char *animationfile, Graphics &gfx);
	void generate_buffers(Graphics &gfx);
	void destroy_buffers(Graphics &gfx);
	void render(Graphics &gfx, int framstep);

private:
	MD5			md5;
	md5_joint_t		**frame;
	int			index_array[8192];
	vertex_t		vertex_array[8192];
	int			**frame_index;
	int			**count_index;
	int			**frame_vertex;
	int			**count_vertex;
	int			*tex_object;
	int			*normal_object;
};

#endif
