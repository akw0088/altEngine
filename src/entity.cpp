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

///============================================================================
/// File: entity.cpp
///============================================================================
/// Class for entity definition
///
/// So the entities are stored as a fixed size array
/// first 0-16 are reserved for players
/// next section is map entities
/// last section is reserved for dynamic entities (bullet casing, rockets,
/// grenades etc)
///
/// Instead of using inheritance (which becomes messy) this base class
/// simply stores pointers to every subclass possible
/// This also allows for a single entity to have multiple properties
/// Eg: a sound source that emits light
/// a Model that emits sound, light, and triggers an event when near etc
///
/// The entity itself is just composed of a position in space
/// (although now it looks like some basic flags have crept in)
/// But the idea is to store most everything in the sub classes
///
/// The sub classes access this data through an Entity pointer passed to them
/// at construction
///
/// The entity list is a fixed size, but the pointers are currently dynamically
/// allocated, we could use fixed size arrays for each type, but memory leaking
/// isnt too big of a concern as we have pretty awesome debug tools nowadays
/// between visual studio and clang sanitizers. Hitching during allocation
/// doesn't seem to be an issue in my experience
///============================================================================

#include "entity.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


///=============================================================================
/// Function: Entity
///=============================================================================
/// Description: Constructor for Entity class
///
///
/// Parameters:
///		None
///
/// Returns:
///		None
///=============================================================================
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

