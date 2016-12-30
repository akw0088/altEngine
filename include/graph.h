#include "include.h"
#include "heap.h"

#ifndef GRAPH_H
#define GRAPH_H

class Graph
{
public:
	Graph();
	void load(graph_node_t *node, int num_nodes);
//	int insert(graph_node_t *node);
//	int remove(int i);
	float *dijkstra(int start);
	int *dijkstra_path(int start, int end, int *path_length);
	int *astar_path(int *path, ref_t *ref, int start, int end, int *path_length);
	int modify_weight(int seq1, int seq2, float weight);
	int size();
	~Graph();
private:
	int	num_nodes;
	graph_node_t	*node;
};
#endif