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