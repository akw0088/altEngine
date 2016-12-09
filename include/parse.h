#include "include.h"

#ifndef PARSE_H
#define PARSE_H

bool parse_entity(const char *input, vector<Entity *> &entity_list, Graphics &gfx, Audio &audio);
char machine(char state, char input, char *stack, int &sp);
void pop(char input, char *stack, int &sp);
void push(char input, char *stack, int &sp);

#endif
