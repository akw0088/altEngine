#include "voice.h"
 
int Voice::init() 
{ 
	int ret; 

	encoder = opus_encoder_create(48000, 1, OPUS_APPLICATION_AUDIO, &ret); 
	if (ret < 0) 
	{ 
		printf("failed to create an encoder: %s\n", opus_strerror(ret)); 
		return -1; 
	} 

	ret = opus_encoder_ctl(encoder, OPUS_SET_BITRATE(BITRATE)); 
	if (ret < 0) 
	{ 
		printf("failed to set bitrate: %s\n", opus_strerror(ret)); 
		return -1; 
	} 

	decoder = opus_decoder_create(48000, 1, &ret); 
	if (ret < 0) 
	{ 
		 printf("failed to create decoder: %s\n", opus_strerror(ret)); 
		 return -1; 
	}
	return 0;
}



int Voice::encode(unsigned short *pcm, unsigned int size, unsigned char *data)
{
	opus_int16 in[SEGMENT_SIZE]; 
	int num_bytes; 

	if (size > SEGMENT_SIZE)
	{
		printf("Must encode %d bytes samples at a time\n", SEGMENT_SIZE);
		return -1;
	}

	// byte swap to big endian
	for (int i = 0; i < SEGMENT_SIZE; i++)
	{ 
		 in[i] = pcm[2 * i + 1] << 8 | pcm[2 * i]; 
	}

	// Encode the frame.
	num_bytes = opus_encode(encoder, in, SEGMENT_SIZE, data, MAX_PACKET_SIZE); 
	if (num_bytes < 0)
	{ 
		printf("encode failed: %s\n", opus_strerror(num_bytes)); 
		return -1; 
	}
	return 0;
}


int Voice::decode(unsigned char *data, unsigned short *pcm, unsigned int &size)
{ 
	opus_int16 out[MAX_SEGMENT_SIZE]; 
	int frame_size;

	frame_size = opus_decode(decoder, data, size, out, MAX_SEGMENT_SIZE, 0); 
	if (frame_size < 0)
	{ 
		printf("decoder failed: %s\n", opus_strerror(frame_size)); 
		return -1; 
	} 

	// byte swap to littler endian
	for(int i = 0; i < frame_size; i++) 
	{ 
		pcm[2 * i] = out[i] & 0xFF; 
		pcm[2 * i + 1] = (out[i] >> 8) & 0xFF; 
	}

	size = frame_size;
	return 0;
}


void Voice::destroy()
{
	 opus_encoder_destroy(encoder); 
	 opus_decoder_destroy(decoder); 
} 

