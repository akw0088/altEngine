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
	bool			frustum_visible;
	bool			bsp_visible;
	int				bsp_leaf;
	bool			nodraw;
	bool			network;
	RigidBody		*rigid;
	Vehicle			*vehicle;
	Model			*model;
	Light			*light;
	Speaker			*speaker;
	Trigger			*trigger;
	Player			*player;
	Decal			*decal;

	//Really bsp related items
	char			type[64];
	int			nettype;
	char			target[64];
	char			target_name[64];
	int				angle;
	int				model_ref;
	float			height; // for movers will make own class later

	//temp hack
	int num_particle;

};
#endif
