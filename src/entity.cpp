#include "entity.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif




Entity::Entity()
{
	position = vec3(0.0f, 0.0f, 0.0f);
	rigid = NULL;
	model = NULL;
	light = NULL;
	speaker = NULL;
	trigger = NULL;
	projectile = NULL;
	player = NULL;
	construct = NULL;
	portal_camera = NULL;
	brushinfo = NULL;

	ent_type = ENT_UNKNOWN;
	nettype = NET_NONE;
	model_ref = -1;
	
	bsp_leaf = 0;
	num_particle = 0;
	flags.visible = false;
	flags.frustum_visible = false;
	flags.bsp_visible = false;
	flags.nodraw = false;
	flags.particle_on = false;

	sprintf(type, "free");
	target[0] = '\0';
	target_name[0] = '\0';
}

Entity::~Entity()
{
	sprintf(type, "free");

	if (light)
		delete light;
	if (rigid)
		delete rigid;
	else if (model)
		delete model;
	if (speaker)
		delete speaker;
	if (trigger)
		delete trigger;
	if (projectile)
		delete projectile;
	if (player)
		delete player;
	if (brushinfo)
	{
		delete brushinfo;
	}

	position = vec3(0.0f, 0.0f, 0.0f);
	rigid = NULL;
	model = NULL;
	light = NULL;
	speaker = NULL;
	trigger = NULL;
	projectile = NULL;
	player = NULL;
	brushinfo = NULL;
	nettype = NET_NONE;
	model_ref = -1;
	flags.visible = false;
	flags.particle_on = false;
	target[0] = '\0';
	target_name[0] = '\0';

}

