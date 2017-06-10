#include "edge.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ensures there are no duplicates, sorts least to greatest by x coordinate
void Edge::insert(vec3 *edge)
{
	int i;

	for(i = 0; i < num_edges; i++)
	{
		if (edge_list[2*i] == edge[0])
		{
			if (edge_list[2*i+1] == edge[1])
			{
				remove(i);
				return;
			}
		}
	}
	insert(i, edge);
};

void Edge::insert(int index, vec3 *edge)
{
	vec3 *old = edge_list;
	if (num_edges == size)
	{
		size++;
		size *= 4;
		edge_list = new vec3[2 * size];
	}
	num_edges++;


	for(int i = 0, j = 0; i < num_edges; i++, j++)
	{
		if (i == index)
		{
			edge_list[2*i] = edge[0];
			edge_list[2*i+1] = edge[1];
			j--;
		}
		else
		{
			edge_list[2*i] = old[2*j];
			edge_list[2*i+1] = old[2*j+1];
		}
	}
}

void Edge::remove(int index)
{
	for(int i = index; i < num_edges; i++)
	{
		edge_list[2*i] = edge_list[2*(i+1)];
		edge_list[2*i+1] = edge_list[2*(i+1)+1];
	}
	num_edges--;
}
