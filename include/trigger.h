#include "include.h"

#ifndef TRIGGER_H
#define TRIGGER_H

class Trigger
{
public:
	Trigger(Entity *entity);
	~Trigger();
	void destroy(Audio &audio);
	Entity	*entity;


	char	pickup_snd[LINE_SIZE];
	char	respawn_snd[LINE_SIZE];
	char	action[LINE_SIZE];
	float	radius;
	unsigned int	source;
	unsigned int	pickup_buf;
	unsigned int	respawn_buf;
	unsigned int	explode_timer;
	bool	hide;	// whether it should disappear when active
	bool	active; // active when picked up
	bool	self;	// whether current player can pick it up
	bool	idle;
	bool	explode;
	float	timeout;
};

#endif
