#include "entity.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Entity::Entity()
{
	position = vec3(0.0f, 0.0f, 0.0f);
	visible = false;
	frustum_visible = false;
	network = false;
	target[0] = '\0';
	target_name[0] = '\0';
	angle = 0;
	model_ref = -1;
	sprintf(type, "free");
	rigid = NULL;
	vehicle = NULL;
	model = NULL;
	light = NULL;
	speaker = NULL;
	trigger = NULL;
	player = NULL;
	decal = NULL;
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
	if (player)
		delete player;

	position = vec3(0.0f, 0.0f, 0.0f);
	visible = false;
	network = false;
	target[0] = '\0';
	target_name[0] = '\0';
	angle = 0;
	model_ref = -1;
	rigid = NULL;
	vehicle = NULL;
	model = NULL;
	light = NULL;
	speaker = NULL;
	trigger = NULL;
	player = NULL;
	decal = NULL;
}

