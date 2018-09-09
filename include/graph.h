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