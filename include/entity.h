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
	RigidBody		*rigid;
	Model			*model;
	Light			*light;
	Speaker			*speaker;
	Trigger			*trigger;
	Player			*player;

	//Really bsp related items
	int				ent_type; // will replace string type eventually
	nettype_t		nettype;
	int				angle;
	int				model_ref;
	float			height; // for movers will make own class later

	bool particle_on;
	int num_particle;

	char			type[64];
	char			target[64];
	char			target_name[64];
};
#endif
