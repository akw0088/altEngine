#include "include.h"

#ifndef SPEAKER_H
#define SPEAKER_H

class Speaker
{
public:
	Speaker(Entity *entity);
	~Speaker();
	void destroy(Sound &audio);
	Entity			*entity;

	char			file[LINE_SIZE];
	unsigned int	source;
	bool			playing;
	bool			loop;
};

#endif
