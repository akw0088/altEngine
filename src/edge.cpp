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

// This file should be deleted eventually, may still be used by some shadow volume generation in light class

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
