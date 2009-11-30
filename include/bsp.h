#include "include.h"

#ifndef BSP_H
#define BSP_H

class Bsp
{
	void changeAxis();
	inline int findLeaf(const vec3 &position);
	inline int isClusterVisible(int visCluster, int testCluster);
public:
	Bsp()
	{
	}
	void tessellate(int level, vertex_t control[], vertex_t **vertex_array, int &numVerts, int **index_array, int &numIndexes);
	void generate_meshes(Graphics &gfx);
	const char *getEnts();
	void get_visible_planes(Entity &entity, Plane *plane, int &num_planes);
	void get_collision_planes(Plane **plane, int &num_planes);
	void drawBox(int *min, int *max);
	void render_face(face_t *face, Graphics &gfx);
	void render_patch(face_t *face, Graphics &gfx, Keyboard &keyboard);
	void render(Entity &entity, Graphics &gfx, Keyboard &keyboard);
	void load(char *map);
	void load_textures(Graphics &gfx);
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
	unsigned int	map_index_vbo;
	unsigned int	map_vertex_vbo;
	unsigned int	*mesh_index_vbo;
	unsigned int	*mesh_vertex_vbo;

	unsigned int	*tex_object;
};

#endif
