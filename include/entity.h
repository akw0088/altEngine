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
