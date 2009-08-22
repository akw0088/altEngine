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
	void drawBox(int *min, int *max);
	void render(vec3 &position, Graphics &graphics);
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

