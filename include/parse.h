#include "include.h"

#ifndef PARSE_H
#define PARSE_H

class Parse
{
public:
	Parse();

	bool parse_entity(const char *input, Entity_list &entity_list);

private:
	char machine(char state, char input);
	void pop(char input);
	void push(char input);

	char stack[80];
	int	sp;
};

#endif
