#include "voice.h"
 
int Voice::init() 
{ 
	int ret; 

	//OPUS_APPLICATION_AUDIO -- music
	//OPUS_APPLICATION_VOIP -- voice
	//OPUS_APPLICATION_RESTRICTED_LOWDELAY -- low delay voice
	encoder = opus_encoder_create(VOICE_SAMPLE_RATE, 1, OPUS_APPLICATION_VOIP, &ret);
	if (ret < 0) 
	{ 
		printf("failed to create an encoder: %s\n", opus_strerror(ret)); 
		return -1; 
	} 


	int complexity = 0;

	ret = opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(complexity));
	if (ret < 0)
	{
		printf("failed to set complexity: %s\n", opus_strerror(ret));
		return -1;
	}

	/*
	ret = opus_encoder_ctl(encoder, OPUS_SET_BITRATE(BITRATE)); 
	if (ret < 0) 
	{ 
		printf("failed to set bitrate: %s\n", opus_strerror(ret)); 
		return -1; 
	} 
	*/

	decoder = opus_decoder_create(VOICE_SAMPLE_RATE, 1, &ret);
	if (ret < 0) 
	{ 
		 printf("failed to create decoder: %s\n", opus_strerror(ret)); 
		 return -1; 
	}
	return 0;
}



int Voice::encode(unsigned short *pcm, unsigned int size, unsigned char *data, int &num_bytes)
{
	static short extend_buffer[SEGMENT_SIZE];
	if (size > SEGMENT_SIZE)
	{
		printf("warning dropping samples\n");
		size = SEGMENT_SIZE;
	}

	// opus only works with 16 bit samples, and I get garbled audio without using 8bit for some reason
	unsigned char *pdata = (unsigned char *)pcm;
	for (unsigned int i = 0; i < SEGMENT_SIZE; i++)
	{
		extend_buffer[i] = 0;
		if (i < size)
		{
			extend_buffer[i] = (unsigned short)(pdata[i] - 0x80) << 8;;
		}
	}

	// Encode the frame.
	num_bytes = opus_encode(encoder, (opus_int16 *)extend_buffer, SEGMENT_SIZE, data, MAX_PACKET_SIZE);
	if (num_bytes < 0)
	{ 
		printf("encode failed: %s\n", opus_strerror(num_bytes)); 
		return -1; 
	}
	return 0;
}


int Voice::decode(unsigned char *data, unsigned short *pcm, unsigned int &size)
{ 
	int frame_size;

	frame_size = opus_decode(decoder, data, size, (opus_int16 *)pcm, MAX_SEGMENT_SIZE, 0);
	if (frame_size < 0)
	{ 
		printf("decoder failed: %s\n", opus_strerror(frame_size)); 
		return -1; 
	} 

	size = frame_size;
	return 0;
}


void Voice::destroy()
{
	 opus_encoder_destroy(encoder); 
	 opus_decoder_destroy(decoder); 
} 

