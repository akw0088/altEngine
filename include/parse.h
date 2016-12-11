#include "include.h"

#ifndef PARSE_H
#define PARSE_H

bool parse_entity(const char *input, vector<Entity *> &entity_list, Graphics &gfx, Audio &audio);
bool parse_shader(const char *input, vector<Surface *> &surface_list);
char machine_entity(char state, char input, char *stack, int &sp);
char machine_shader(char state, char input, char *stack, int &sp);
void pop(char input, char *stack, int &sp);
void push(char input, char *stack, int &sp);

#endif
