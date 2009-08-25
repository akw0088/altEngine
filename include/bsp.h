#include "include.h"

#ifndef BSP_H
#define BSP_H

class Bsp
{
	void changeAxis();
	int findLeaf(const vec3 &position);
	int isClusterVisible(int visCluster, int testCluster);
public:
	Bsp()
	{
	}
	void tessellate(int level, vertex_t control[], vertex_t **vertex_array, int &numVerts, int **index_array, int &numIndexes);
	void generateMeshes();
	const char *getEnts();
	void drawBox(int *min, int *max);
	void render(Entity &entity, Graphics &gfx, Keyboard &keyboard);
	void load(char *map);
	void loadTextures(Graphics &graphics);
	void unload();
	~Bsp()
	{
	}

private:
	bsp_t		*tBsp;
	bspData_t	data;

	//for bezier patches
	int			mesh_level;
	int			*mesh_index2face;
	vertex_t	**mesh_vertex_array;
	int			**mesh_index_array;
	int			*mesh_numVerts;
	int			*mesh_numIndexes;
	int			num_meshes;
};

#endif
