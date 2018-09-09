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

#ifndef HASHTABLE_H
#define HASHTABLE_H


typedef struct
{
	char *key;
	void *value;
} hashnode_t;

#define TABLE_SIZE 256

class HashTable
{
public:
	HashTable();
	void insert(char *key, char *value);
	void *find(const char *key) const;
	bool update(const char *key, char *value);
	void destroy();
private:
	int hash(const char *key, int i) const;
	int hash_djb2(const char *key) const;
	int hash_sdbm(const char *key) const;
	hashnode_t *table[TABLE_SIZE];
};

#endif
