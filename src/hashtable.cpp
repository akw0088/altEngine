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

#include "hashtable.h"

HashTable::HashTable()
{
	memset(table, 0, sizeof(table));
}

void HashTable::destroy()
{
	for (int i = 0; i < TABLE_SIZE; i++)
	{
		if (table[i])
		{
			delete[] table[i]->key;
			delete[] (char *)table[i]->value;
			delete table[i];
		}
	}
}

/*
	If hashtable is full nothing is inserted.
*/
void HashTable::insert(char *key, char *value)
{
//	printf("HashTable::insert(%s, %p)\n", key, value);
	int index;


	// Normally this doesnt copy values or assume string values,
	// but more convenient with this use case
	int length = strlen(key);
	int size = strlen((char *)value);
	char *nkey = new char [length + 1];
	char *nval = new char [size + 1];
	memcpy(nkey, key, length + 1);
	memcpy(nval, value, size + 1);

	for(int i = 0; i < TABLE_SIZE; i++)
	{
		index = hash(nkey, i);

		if (table[index] == NULL)
		{
			hashnode_t *node = new hashnode_t;
			node->key = nkey;
			node->value = nval;
			table[index] = node;
			break;
		}
	}
}

bool HashTable::update(const char *key, char *value)
{
//	printf("HashTable::update(%s, %s)\n", key, value);
	int index;

	int size = strlen(value) + 1;
	for(int i = 0; i < TABLE_SIZE; i++)
	{
		index = hash(key, i);

		if (table[index] == NULL)
			continue;

		if (strcmp(table[index]->key, key) == 0)
		{
			char *nval = new char[size];
			memcpy(nval, value, size);

			table[index]->value = nval;
			return true;
		}
	}
	return false;
}


void *HashTable::find(const char *key) const
{
//	printf("HashTable::find(%s)\n", key);
	int index;

	for(int i = 0; i < TABLE_SIZE; i++)
	{
		index = hash(key, i);

		// tried to find a key that doesnt exist
		if (table[index] == NULL)
			return NULL;

		if (strcmp(key, table[index]->key) == 0)
			return table[index]->value;
	}

	return (void *)NULL;
}

/*
		CLRS: h2 must be 'relatively prime' for entire table to be searched
	To ensure this let m be a power of two and h2 odd or let m be prime and
	let h2 be less than m. (m = table size)
*/
int HashTable::hash(const char *key, int i) const
{
	unsigned int h1 = hash_djb2(key);
	unsigned int h2 = hash_sdbm(key);

	// make h2 odd
	if (h2 % 2 == 0)
		h2 += 1;

	return ( h1 + (i * h2) ) % TABLE_SIZE;
}


int HashTable::hash_djb2(const char *key) const
{
	unsigned int hashval = 5381;
	int c;

	while ((c = *key++))
		hashval = ((hashval << 5) + hashval) + c;

	return hashval;
}


int HashTable::hash_sdbm(const char *key) const
{
	unsigned int hashval = 0;
	int c;

	while ((c = *key++))
		hashval = c + (hashval << 6) + (hashval << 16) - hashval;

	return hashval;
}
