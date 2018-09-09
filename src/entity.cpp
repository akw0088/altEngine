//=============================================================================
// This file is part of the altEngine distribution
// (https://github.com/akw0088/altEngine/)
// Copyright (c) 2018 Alexander Wright All Rights Reserved.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON - INFRINGEMENT.IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//=============================================================================

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
	entstring = NULL;
	vehicle = NULL;

	ent_type = ENT_UNKNOWN;
	nettype = NET_NONE;
	brush_ref = -1;
	
	bsp_leaf = 0;
	num_particle = 0;
	flags.visible = false;
	flags.frustum_visible = false;
	flags.bsp_visible = false;
	flags.nodraw = false;
	flags.particle_on = false;
}

Entity::~Entity()
{
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
		delete brushinfo;
	if (entstring)
		delete entstring;
	if (portal_camera)
		delete portal_camera;

	position = vec3(0.0f, 0.0f, 0.0f);
	rigid = NULL;
	model = NULL;
	light = NULL;
	speaker = NULL;
	trigger = NULL;
	projectile = NULL;
	player = NULL;
	brushinfo = NULL;
	entstring = NULL;
	vehicle = NULL;
	nettype = NET_NONE;
	brush_ref = -1;
	flags.visible = false;
	flags.particle_on = false;
}

