#include "entity.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Entity::Entity()
{
	position = vec3(0.0f, 0.0f, 0.0f);
	visible = false;
	network = false;
	target[0] = '\0';
	target_name[0] = '\0';
	angle = 0;
	type = NULL;
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
	if (type)
		delete [] type;

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

}

