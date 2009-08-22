#include "include.h"

#ifndef VOXEL_H
#define VOXEL_H


class voxel
{
public:
	voxel();
	void render(vec3 &position, Graphics &graphics);
private:
	char *data;

	int x,y,z;

	//box temp crap
	vertex_t		*vertex_array;
	int				num_vertex;
	int				*index_array;
	int				num_index;
};

#endif

