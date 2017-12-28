#include "include.h"

#ifndef ENTITY_H
#define ENTITY_H


class Entity
{
public:
	Entity();
	~Entity();

	vec3			position;
	RigidBody		*rigid;
	Model			*model;
	Light			*light;
	Speaker			*speaker;
	Trigger			*trigger;
	Projectile		*projectile;
	Player			*player;
	Constructable	*construct;
	PortalCamera	*portal_camera;
	brushinfo_t		*brushinfo; // doors, platforms, etc
	ent_string_t	*entstring;



	int				ent_type; // will replace string type eventually
	net_ent_t		nettype;
	int				model_ref;		// bsp model index
	int				bsp_leaf;
	int				num_particle;

	struct flag
	{
		unsigned char visible : 1,
		frustum_visible : 1,
		bsp_visible : 1,
		nodraw : 1,
		particle_on : 1,
		pad : 3;
	} flags;

};
#endif
