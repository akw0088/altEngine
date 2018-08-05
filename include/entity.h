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
	Projectile		*projectile;
	Player			*player;
	ent_string_t	*entstring;
	Trigger			*trigger;
	Light			*light;
	Speaker			*speaker;
	brushinfo_t		*brushinfo; // doors, platforms, etc
	Constructable	*construct;
	PortalCamera	*portal_camera;
	Vehicle			*vehicle;


	int				ent_type; // will replace string type eventually
	int				brush_ref;		// bsp brush associated with entity
	int				bsp_leaf;
	int				num_particle;
	net_ent_t		nettype;

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
