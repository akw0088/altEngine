#include "include.h"

#ifndef ENTITY_H
#define ENTITY_H

class Entity
{
public:
	Entity();
	~Entity();

	vec3			position;
	bool			visible;
	bool			network;
	char			*type;
	RigidBody		*rigid;
	Vehicle			*vehicle;
	Model			*model;
	Light			*light;
	Speaker			*speaker;
	Trigger			*trigger;
	Player			*player;
	Decal			*decal;
};
#endif
