#include "include.h"

#ifndef VOICE_H
#define VOICE_H


#define SEGMENT_SIZE		480
#define BITRATE			64000
 
#define MAX_SEGMENT_SIZE 	(6 * 480)
#define MAX_PACKET_SIZE		(3 * 480)


class Voice
{
public:
	int init();
	void destroy();
	int encode(unsigned short *pcm, unsigned int size, unsigned char *data, unsigned int &num_bytes);
	int decode(unsigned char *data, unsigned short *pcm, unsigned int &size);

private:
	OpusEncoder *encoder; 
	OpusDecoder *decoder; 
};

#endif
