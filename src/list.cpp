#include "list.h"

template <typename dtype>
void List<dtype>::add(dtype *item)
{
	num++;
	size++;

	if (size == num)
	{
		dtype	**old = list;

		list = new dtype *[size];

		for (int i = 0; i < num - 1; i++)
			list[i] = old[i];

		delete [] old;
	}

	list[size - 1] = item;
}

template <typename dtype>
void List<dtype>::remove()
{
	num--;
	delete list[num];
}

template <typename dtype>
dtype &List<dtype>::operator[](int index)
{
	return *list[index];
}

template <typename dtype>
List<dtype>::~List()
{
	while (num != 0)
	{
		remove();
	}
	delete [] list;
	size = 0;
}
