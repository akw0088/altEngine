#include "include.h"

#ifndef VOICE_H
#define VOICE_H


#define SEGMENT_SIZE		960
#define BITRATE			64000
 
#define MAX_SEGMENT_SIZE 	(6 * 960)
#define MAX_PACKET_SIZE		(3 * 1276)


class Voice
{
public:
	int init();
	void destroy();
	int encode(unsigned short *pcm, unsigned int size);
	int decode(unsigned short *pcm, unsigned int &size);

private:
	OpusEncoder *encoder; 
	OpusDecoder *decoder; 
};

#endif
