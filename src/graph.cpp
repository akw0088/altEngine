//=============================================================================
// This file is part of the altEngine distribution
// (https://github.com/akw0088/altEngine/)
// Copyright (c) 2018 Alexander Wright All Rights Reserved.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON - INFRINGEMENT.IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//=============================================================================

#include "graph.h"


Graph::Graph()
{
	Graph::node = NULL;
	Graph::num_nodes = 0;
}

void Graph::load(graph_node_t *node, int num_nodes)
{
	Graph::node = node;
	Graph::num_nodes = num_nodes;
}

Graph::~Graph()
{
	node = NULL;
	num_nodes = 0;
}

int Graph::modify_weight(int seq1, int seq2, float weight)
{
	for(int i = 0; i < node[seq1].num_arcs; i++)
	{
		if (node[seq1].arc[i].b == seq2)
		{
			node[seq1].arc[i].weight += weight;
			return 0;
		}
	}
	return 1;
}

/*
	dijkstra - dijkstra's algorithm (shortest path)
		graph_node_t	nodes[]	- nodes in graph
		int	num_nodes	- number of verts/nodes
		returns distance table for each node from source on sucess, null on failure.
*/
float *Graph::dijkstra(int start)
{
	Heap		heap;
	heapkey_t	key;
	float		*distance_table;
	int		i;

	distance_table = new float[num_nodes];
	if (distance_table == NULL)
	{
		perror("malloc() failed in dijkstra()");
		return NULL;
	}

	// 1. Assign to every node a distance value. Set it to zero for our initial node and to infinity for all other nodes.
	for (i = 0; i < num_nodes; i++)
	{
		if (i == start)
			distance_table[i] = 0.0f;
		else
			distance_table[i] = FLT_MAX;

		key.index = i;
		key.data = distance_table[i];

		// Heap used to quickly sort by minimum weight
		if ( heap.insert(&key) )
		{
			delete [] distance_table;
			return NULL;
		}
	}

	while ( heap.heap_length )
	{
		key = heap.extract(1);

		// For each node v adjacent to u
		for (i = 0; i < node[ key.index ].num_arcs; i++)
		{
			int u = key.index;
			int v = node[ key.index ].arc[i].b;

			// update distance if required
			if (distance_table[v] >= distance_table[u] + node[ key.index ].arc[i].weight)
			{
				heapkey_t	update;

				distance_table[v] = distance_table[u] + node[ key.index ].arc[i].weight;
				update.index = v;
				update.data = distance_table[v];
				heap.modify(&update);
				//predTable[v] = u;
			}
			
		}
	}
	return distance_table;
}


int *Graph::dijkstra_path(int start, int end, int *path_length)
{
	Heap		heap;
	heapkey_t	key;
	float		*distance_table;
	int		*predecessor_table;
	int		*path_table;
	int		i, j;

	distance_table = new float[num_nodes];
	if (distance_table == NULL)
	{
		perror("malloc() failed in dijkstra()");
		return NULL;
	}

	predecessor_table = new int[num_nodes];
	if (predecessor_table == NULL)
	{
		perror("calloc() failed in dijkstra()");
		delete[] distance_table;
		return NULL;
	}

	path_table = new int[num_nodes];
	if (path_table == NULL)
	{
		perror("calloc() failed in dijkstra()");
		delete[] distance_table;
		delete[] predecessor_table;
		return NULL;
	}

	// 1. Assign to every node a distance value. Set it to zero for our initial node and to infinity for all other nodes.
	for (i = 0; i < num_nodes; i++)
	{
		if (i == start)
			distance_table[i] = 0.0f;
		else
			distance_table[i] = FLT_MAX;

		key.index = i;
		key.data = distance_table[i];

		// Heap used to quickly sort by minimum weight
		if ( heap.insert(&key) )
		{
			delete[] distance_table;
			delete[] path_table;
			delete[] predecessor_table;
			return NULL;
		}
	}

	while ( heap.heap_length )
	{
		key = heap.extract(1);

		// For each node v adjacent to u
		for (i = 0; i < node[ key.index ].num_arcs; i++)
		{
			int u = key.index;
			int v = node[ key.index ].arc[i].b;

			// update distance if required
			if (distance_table[v] >= distance_table[u] + node[ key.index ].arc[i].weight)
			{
				heapkey_t	update;

				distance_table[v] = distance_table[u] + node[ key.index ].arc[i].weight;
				predecessor_table[v] = u;
				update.index = v;
				update.data = distance_table[v];
				heap.modify(&update);

				if (v == end)
					break;
			}
			
		}
	}

	for(i = end, j = 0; i != start; j++)
	{
		path_table[j] = i;
		i = predecessor_table[i];
	}
	*path_length = j;

	for(i = 0; i < *path_length / 2; i++)
	{
		int temp;
		temp = path_table[i];
		path_table[i] = path_table[*path_length - 1 - i];
		path_table[*path_length - 1 - i] = temp;
	}

	delete[] distance_table;
	delete[] predecessor_table;
	return path_table;
}

int *Graph::astar_path(int *path_table, ref_t *ref, int start, int end, int *path_length)
{
	Heap		heap;
	heapkey_t	key;
	float		*distance_table = NULL;
	int		*predecessor_table = NULL;
	int		i, j;

	distance_table = new float[num_nodes];
	if (distance_table == NULL)
	{
		perror("malloc() failed in dijkstra()");
		return NULL;
	}

	predecessor_table = new int[num_nodes];
	if (predecessor_table == NULL)
	{
		perror("malloc() failed in dijkstra()");
		delete[] distance_table;
		return NULL;
	}
	memset(predecessor_table, 0, sizeof(int) * num_nodes);

	// 1. Assign to every node a distance value. Set it to zero for our initial node and to infinity for all other nodes.
	for (i = 0; i < num_nodes; i++)
	{
		if (i == start)
			distance_table[i] = 0.0f;
		else
			distance_table[i] = FLT_MAX;

		key.index = i;
		key.data = distance_table[i];

		// Heap used to quickly sort by minimum weight
		if ( heap.insert(&key) )
		{
			delete[] distance_table;
			delete[] predecessor_table;
			return NULL;
		}
	}

	while ( heap.heap_length )
	{
		key = heap.extract(1);

		// For each node v adjacent to u
		for (i = 0; i < node[ key.index ].num_arcs; i++)
		{
			int u = key.index;
			int v = node[ key.index ].arc[i].b;
			float a = abs32((float)ref[u].x - ref[end].x);
			float b = abs32((float)ref[u].y - ref[end].y);
			float c = abs32((float)ref[u].z - ref[end].z);
			float astar_weight = a * a + b * b + c * c;


			// update distance if required
			if (distance_table[v] >= distance_table[u] + node[ key.index ].arc[i].weight + astar_weight)
			{
				heapkey_t	update;

				distance_table[v] = distance_table[u] + node[ key.index ].arc[i].weight;
				predecessor_table[v] = u;
				update.index = v;
				update.data = distance_table[v];
				heap.modify(&update);

				if (v == end)
					break;
			}
			
		}
	}

	for(i = end, j = 0; i != start; j++)
	{
		if (j >= num_nodes)
		{
//			printf("Fatal error: Path longer than available nodes\n");
			*path_length = -1;
			delete[] distance_table;
			delete[] predecessor_table;
			return NULL;
		}

		path_table[j] = i;
		i = predecessor_table[i];
	}
	path_table[j] = start; // add start node into list
	*path_length = j + 1;

	for(i = 0; i < *path_length / 2; i++)
	{
		int temp;
		temp = path_table[i];
		path_table[i] = path_table[*path_length - 1 - i];
		path_table[*path_length - 1 - i] = temp;
	}

	delete[] distance_table;
	delete[] predecessor_table;
	return path_table;
}

int Graph::size()
{
	return num_nodes;
}
