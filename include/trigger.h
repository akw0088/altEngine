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

#ifndef TRIGGER_H
#define TRIGGER_H

class Entity;

class Trigger
{
public:
	Trigger(Entity *entity, Audio &audio);
	~Trigger();
	void create_sources(Audio &audio);
	void destroy(Audio &audio);
	Entity	*entity;

	bool noise;

	char noise_str[128];
	unsigned int pickup_index;
	unsigned int respawn_index;
	unsigned int explode_index;
	unsigned int idle_index;


	char	action[LINE_SIZE];
	char	message[LINE_SIZE];
	float	radius;
	int	source;
	int	loop_source;
	unsigned int	pickup_buf;
	unsigned int	respawn_buf;

	bool	hide;		// whether it should disappear when active
	bool	active;		// active when picked up
	bool	played;
	bool	client_active; // client side pickup registration (to prevent repeatedly picking something up client side)
	bool	health;		// For health items, prevents giving over 100
	bool	armor;		// For armor items, prevents giving over 200
	float	timeout;	// Timeout between pickups respawning
	float	timeout_value;


	// I should probably split item pickups from projectiles as it's starting to get complex
#if 0
	bool			projectile;
	int				explode_type; // type of explosion sprite
	vec3			explode_color;
	float			explode_intensity;
	unsigned int	explode_timer;	// How long an explosion lasts (light effect, damage trigger time)
	unsigned int	idle_timer;		// How long after stopping something sits before exploding (should probaably be from when fired)
	unsigned int	splash_damage;	// Damage given when exploding
	float			splash_radius;	// Trigger pickup radius when exploding
	float			knockback;		// Amount to knock people back when exploding (scaled by radius)
	bool	idle;		// Delete when it stops moving
	bool	explode;	// Does it explode
	int		owner;
	unsigned int	num_bounce;
#endif
};

#endif
