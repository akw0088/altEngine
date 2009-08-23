#include "include.h"

class Bsp
{
	void changeAxis();
	int findLeaf(const vec3 &position);
	int isClusterVisible(int visCluster, int testCluster);
public:
	Bsp()
	{
	}
	void tessellate(int level, vertex_t control[], int nControls, vertex_t **vertex_array, int &numVerts, int **index_array, int &numIndexes);
	void drawBox(int *min, int *max);
	void render(vec3 &position, Graphics &gfx, Keyboard &keyboard);
	void load(char *map);
	void loadTextures(Graphics &graphics);
	void unload();
	~Bsp()
	{
	}

private:
	bsp_t		*tBsp;
	bspData_t	data;
};

