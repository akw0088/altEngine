#include "include.h"


// Essentially from here:
// https://matthias-research.github.io/pages/tenMinutePhysics/11-hashing.pdf
// https://www.youtube.com/watch?v=D2M8jTtKi44
//
// Uses a hash table to make distance checks between objects faster
// comparing each pair of objects in two loops would be n^2
//
// This will hash objects that are close to each other into a table
// Then we just check all the objects in the same hash
// Collisions will just fail the distance check which you still have to do
//
// Collisions are stored in a seperate table that is sized by max objects,
// no link list chaining, but more like array chaining with pre-determined
// max size
//
// Does check neighboring hash positions to avoid issues where
// Two are near, but in different hash positions
//
// query radius and spacing set the size of each cell
// Which you want to exceed your bounding volumes max size
// so you dont have something span too many cells

class SpatialHash
{
public:
	// allocate memory
	void init(float spacing, int maxNumObjects);

	// create hash table given entities (done each step)
	void create(vector<Entity *> entity_list, int num_ent);

	// search hash table for given neigbor in radius
	void query(vec3 &pos, float radius, unsigned int *&query_table, int &query_size);

private:
	// float to integer floor
	int get_coord(float coord);

	// integer hash function
	unsigned int hash(int xi, int yi, int zi);

	// vector to hash helper
	unsigned int hash_vec(vec3 &pos);


	float spacing;
	int hash_table_size;
	int *cell_start;
	int *cell_entry;
};