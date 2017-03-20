#ifndef LIST_H
#define LIST_H

/*
	This is really just std::vector, I should delete this
*/

template <typename dtype>
class List
{
public:
	int add(dtype *item);	// does not make copy
	void remove();
	~List();
	dtype &operator[](int index);

	int		num;
private:
	dtype	**list;
	int		size;
};

template <typename dtype>
int List<dtype>::add(dtype *item)
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

	list[num - 1] = item;
	return num - 1;
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


#endif