#include "include.h"

#ifndef ENTITY_H
#define ENTITY_H

class Entity
{
public:
	Entity();
	~Entity();

	vec3			position;
	char			target[64];
	char			target_name[64];
	int				angle;
	bool			visible;
	bool			network;
	char			type[64];
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
