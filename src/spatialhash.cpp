#include "SpatialHash.h"



///=============================================================================
/// Function: hash
///=============================================================================
///
/// Magic hash function that gives good results, essentially just mixes the three
/// components into a single quantity with hopefully minimal collisions
///
/// Parameters:
///   spacer - spacing factor used to size the spatial hash ideally such that one object
/// can only span at most two spaces in each direction x,y,z
///
///   max_object - maximum number of objects to be entered into spatial hash
/// (smaller tables will yield more collisions, rule of thumb is double the number of
/// objects inserted into the table)
///
/// Returns:
///		None
///=============================================================================
void SpatialHash::init(float spacer, int max_object)
{
	spacing = spacer;
	hash_table_size = 2 * max_object;
	cell_start = new int  [hash_table_size + 1];
	cell_entry = new int [max_object];
}


///=============================================================================
/// Function: hash
///=============================================================================
///
/// Magic hash function that gives good results, essentially just mixes the three
/// components into a single quantity with hopefully minimal collisions
///
/// Parameters:
///   xi - integer x component
///   yi - integer y component
///   zi - integer z component
///
/// Returns:
///		None
///=============================================================================
unsigned int SpatialHash::hash(int xi, int yi, int zi)
{
	unsigned int h = (xi * 92837111) ^ (yi * 689287499) ^ (zi * 283923481);

	return h % hash_table_size;
}



///=============================================================================
/// Function: hash_vec
///=============================================================================
///
/// Hashes a float by essentially casting to int after dividing by spacing
///
/// Parameters:
///   coord - position to convert to integer
///
/// Returns:
///		None
///=============================================================================
int SpatialHash::get_coord(float coord)
{
	return (int)(coord / spacing);
}



///=============================================================================
/// Function: hash_vec
///=============================================================================
///
/// Hashes a vector by essentially casting to int after dividing by spacing
///
/// Parameters:
///   vec3 - position to convert to integer xyz
///
/// Returns:
///		None
///=============================================================================
unsigned int SpatialHash::hash_vec(vec3 &pos)
{
	return hash( get_coord(pos.x), get_coord(pos.y), get_coord(pos.z));
}



///=============================================================================
/// Function: create
///=============================================================================
///
/// creates / updates the hash table with a new set of objects
///
/// Parameters:
///   entity_list - set of objects to add to spatial hash
///   int num_ent - number of objects to add to spatial hash
///
/// Returns:
///		None
///=============================================================================
void SpatialHash::create(vector<Entity *> entity_list, int num_ent)
{
	// determine cell sizes
	for (int i = 0; i < hash_table_size; i++)
	{
		cell_start[i];
	}

	for (int i = 0; i < hash_table_size; i++)
	{
		cell_entry[i];
	}

	for (int i = 0; i < num_ent; i++)
	{
		unsigned int h = hash_vec(entity_list[i]->position);

		cell_start[h]++;
	}

	// determine cells starts - running sum
	int start = 0;
	for (int i = 0; i < hash_table_size; i++)
	{
		start += cell_start[i];
		cell_start[i] = start;
	}
	cell_start[hash_table_size] = start;	// guard

	// fill in objects ids
	for (int i = 0; i < num_ent; i++)
	{
		unsigned int h = hash_vec(entity_list[i]->position);

		// decrement to get correct index to cell_entry
		cell_start[h]--;

		// table that holds each object
		cell_entry[cell_start[h]] = i;
	}
}


///=============================================================================
/// Function: query
///=============================================================================
///
/// Queries the spatial hash table for objects that has to position with radius
///
///
/// Parameters:
///   pos - position xyz to hash
///   radius - radius from position xyz to also check (half size of maximum bounding volume)
///   query_table - array of integers that map to objects that are near pos +/- radius
///   query_size - size of array, max size is equal to number of objects in the table
///
/// Returns:
///		None
///=============================================================================
void SpatialHash::query(vec3 &pos, float radius, unsigned int *&query_table, int &query_size)
{
	unsigned int x0 = get_coord(pos.x - radius);
	unsigned int y0 = get_coord(pos.y - radius);
	unsigned int z0 = get_coord(pos.z - radius);

	unsigned int x1 = get_coord(pos.x + radius);
	unsigned int y1 = get_coord(pos.y + radius);
	unsigned int z1 = get_coord(pos.z + radius);

	query_size = 0;

	for (unsigned int xi = x0; xi <= x1; xi++)
	{
		for (unsigned int yi = y0; yi <= y1; yi++)
		{
			for (unsigned int zi = z0; zi <= z1; zi++)
			{
				unsigned int h = hash(xi, yi, zi);
				unsigned int start = cell_start[h];
				unsigned int end = cell_start[h + 1];

				for (unsigned int i = start; i < end; i++)
				{
					query_table[query_size] = cell_entry[i];
					query_size++;
				}
			}
		}
	}
}
