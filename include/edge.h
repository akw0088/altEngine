#include "include.h"

#ifndef EDGE_H
#define EDGE_H

class Edge
{
public:
	Edge()
	{
		num_edges = 0;
		size = 0;
		edge_list = NULL;
	}

	void insert(vec3 *edge);
	void insert(int index, vec3 *edge);
	void remove(int index);
	int num_edges;
	vec3 *edge_list;
private:
	int size;
};

#endif
