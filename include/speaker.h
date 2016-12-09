#include "include.h"

#ifndef SPEAKER_H
#define SPEAKER_H

class Speaker
{
public:
	Speaker(Entity *entity, Audio &audio);
	~Speaker();
	void gain(float value);
	void loop_gain(float value);
	void destroy(Audio &audio);
	Entity			*entity;

	char			file[LINE_SIZE];
	unsigned int	source;
	unsigned int	loop_source;
};

#endif
