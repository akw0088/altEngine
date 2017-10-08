#include "include.h"

#ifndef ENTITY_H
#define ENTITY_H

class Entity
{
public:
	Entity();
	~Entity();

	vec3			position;
	vec3			origin;
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
	PortalCamera	*portal_camera;



	//Really bsp related items
	int				ent_type; // will replace string type eventually
	nettype_t		nettype;
	int				angle;
	int				model_ref;		// bsp model index
	vec3			model_offset;	// bsp model offset
	float			model_lerp;		// bsp lerp between 0.0-1.0
	bool			opening;		// flag for door opening
	float			func_height;			// How far a func_ item will move in map units, defaults to half width

	bool particle_on;
	int num_particle;

	char			type[64];
	char			target[64];
	char			target_name[64];


	int once;
	vec3 path_list[8];
	int num_path;
};
#endif
