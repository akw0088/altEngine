#include "entity.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif




Entity::Entity()
{
	position = vec3(0.0f, 0.0f, 0.0f);
	visible = false;
	frustum_visible = false;
	bsp_visible = false;
	bsp_leaf = 0;
	nodraw = false;
	rigid = NULL;
	model = NULL;
	light = NULL;
	speaker = NULL;
	trigger = NULL;
	projectile = NULL;
	player = NULL;
	construct = NULL;
	portal_camera = NULL;
	model_lerp = 0.0f;
	model_offset = vec3();
	opening = false;

	ent_type = ENT_UNKNOWN;
	nettype = NET_NONE;
	angle = -1;
	model_ref = -1;
	func_height = 0.0f;

	particle_on = false;
	num_particle = 0;

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

	position = vec3(0.0f, 0.0f, 0.0f);
	visible = false;
	particle_on = false;
	target[0] = '\0';
	target_name[0] = '\0';
	angle = 0;
	model_ref = -1;
	rigid = NULL;
	model = NULL;
	light = NULL;
	speaker = NULL;
	trigger = NULL;
	projectile = NULL;
	player = NULL;
	nettype = NET_NONE;
}

