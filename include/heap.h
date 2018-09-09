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

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>


#ifndef HEAP_H
#define HEAP_H
typedef struct
{
	int		index;
	float	data;
} heapkey_t;

typedef enum
{
	MIN_HEAP,
	MAX_HEAP
} type_t;

class Heap
{
public:
	Heap();
	Heap(type_t type);
	int insert(const heapkey_t *);
	heapkey_t extract(const int);
	void modify(const heapkey_t *);
	void fix();
	~Heap();

	int			heap_length;
private:
	heapkey_t	*heap;
	int			data_length;
	type_t		type;
};
#endif