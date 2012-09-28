#include "include.h"

#ifndef TRIGGER_H
#define TRIGGER_H

class Trigger
{
public:
	Trigger(Entity *entity);
	~Trigger();
	void destroy(Sound &audio);
	Entity	*entity;


	char	pickup_snd[LINE_SIZE];
	char	respawn_snd[LINE_SIZE];
	char	action[LINE_SIZE];
	unsigned int	source;
	unsigned int	pickup_buf;
	unsigned int	respawn_buf;
	bool	active;
	float	timeout;
};

#endif
