#include "include.h"

#ifndef VOICE_H
#define VOICE_H


#define SEGMENT_SIZE		1920
 
#define MAX_SEGMENT_SIZE 	(6 * SEGMENT_SIZE)
#define MAX_PACKET_SIZE		(3 * SEGMENT_SIZE)


class Voice
{
public:
	int init();
	void destroy();
	int encode(unsigned short *pcm, unsigned int size, unsigned char *data, int &num_bytes);
	int decode(unsigned char *data, unsigned short *pcm, unsigned int &size);

private:
	OpusEncoder *encoder; 
	OpusDecoder *decoder; 
};

#endif
