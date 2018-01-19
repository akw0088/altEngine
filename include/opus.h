#ifndef __OPUS_H
#define __OPUS_H

#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <errno.h> 
#include <math.h>

#include <opus/opus.h> 
 
#define SEGMENT_SIZE		960
#define BITRATE			64000
 
#define MAX_SEGMENT_SIZE 	(6 * 960)
#define MAX_PACKET_SIZE		(3 * 1276)


class Opus
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
