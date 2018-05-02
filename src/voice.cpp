#include "voice.h"
 
Voice::Voice()
{
	voice_send_sequence = 0;
	voice_recv_sequence = 0;
}

int Voice::init(Audio &audio, unsigned short qport)
{ 
	int ret; 

	Voice::qport = qport;

	if (audio.microphone == NULL)
	{
		return -1;
	}

#ifndef DEDICATED
	alGenSources(1, &mic_source);
	alGenBuffers(NUM_PONG, (unsigned int *)&mic_buffer[0]);
	alSourcei(mic_source, AL_SOURCE_RELATIVE, AL_TRUE);

	alGenSources(1, &decode_source);
	alGenBuffers(NUM_PONG, (unsigned int *)&decode_buffer[0]);
	alSourcei(decode_source, AL_SOURCE_RELATIVE, AL_TRUE);
#endif

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

void Voice::bind(char *ip, unsigned short port)
{
	sock.bind(ip, port);
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

int Voice::voice_send(Audio &audio, vector<client_t *> &client_list, bool client_flag, bool server_flag)
{
	unsigned int size;
	int isize;
	static int pong = 0;
	static bool looped = false;
	unsigned int uiBuffer;
	int buffersProcessed = 0;
	bool local_echo = false;
	static voicemsg_t msg;

	if (audio.microphone == NULL)
	{
		return 0;
	}

#ifdef DEDICATED
	return 0;
#endif

	if (looped)
	{
#ifndef DEDICATED
		if (local_echo)
		{
			alGetSourcei(mic_source, AL_BUFFERS_PROCESSED, &buffersProcessed);
			if (buffersProcessed == 0)
			{
				ALenum state;

				alGetSourcei(mic_source, AL_SOURCE_STATE, &state);

				if (state == AL_STOPPED)
				{
					audio.play(mic_source);
				}
				return 0;
			}

			alSourceUnqueueBuffers(mic_source, 1, &uiBuffer);
			int al_err = alGetError();
			if (al_err != AL_NO_ERROR)
			{
				return 0;
			}
		}
#endif

		audio.capture_sample(mic_pcm[pong], isize);
		size = isize;
		if (local_echo)
		{
#ifndef DEDICATED
			alBufferData(uiBuffer, VOICE_FORMAT, mic_pcm[pong], size, VOICE_SAMPLE_RATE);
			alSourceQueueBuffers(mic_source, 1, &uiBuffer);
#endif
		}

	}
	else
	{
		audio.capture_sample(mic_pcm[pong], isize);
		size = isize;
		if (local_echo)
		{
#ifndef DEDICATED
			alBufferData(mic_buffer[pong], VOICE_FORMAT, mic_pcm[pong], size, VOICE_SAMPLE_RATE);
			int al_err = alGetError();
			if (al_err != AL_NO_ERROR)
			{
				debugf("Error alBufferData\n");
			}
			alSourceQueueBuffers(mic_source, 1, &mic_buffer[pong]);
#endif
		}
	}

	if (size == 0)
	{
		return 0;
	}


	if (server_flag)
	{
		for (unsigned int i = 0; i < client_list.size(); i++)
		{
			sprintf(server, "%s", client_list[i]->socketname);

			char *colon = strchr(server, ':');
			if (colon)
			{
				*colon = '\0';
			}
			sprintf(colon, ":65530");

			int num_bytes = 0;
			encode(mic_pcm[pong], size, msg.data, num_bytes);


			msg.sequence = voice_send_sequence++;
			msg.qport = qport;
			int ret = sock.sendto((char *)&msg, VOICE_HEADER + num_bytes, server);
			if (ret < 0)
			{
#ifdef WIN32
				int ret = WSAGetLastError();
#else
				int ret = errno;
#endif

				printf("Failed to send voice data %d\n", ret);
			}
		}
	}
	else if (client_flag)
	{
		int num_bytes = 0;

		msg.sequence = voice_send_sequence++;
		msg.qport = qport;
		encode(mic_pcm[pong], size, msg.data, num_bytes);
		int ret = sock.sendto((char *)&msg, VOICE_HEADER + num_bytes, server);
		if (ret < 0)
		{
#ifdef WIN32
			int ret = WSAGetLastError();
#else
			int ret = errno;
#endif

			printf("Failed to send voice data %d\n", ret);
		}
	}


	pong++;
	if (pong >= NUM_PONG)
	{
		pong = 0;
		if (local_echo)
		{
			if (looped == false)
			{
				looped = true;
				audio.play(mic_source);
			}
		}
	}

	if (size)
		return 1;
	else
		return 0;
}

int Voice::voice_recv(Audio &audio)
{
	unsigned int size;
	int ret;
	static int pong = 0;
	static bool looped = false;
	int buffersProcessed = 0;
	unsigned int uiBuffer;
	bool remote_echo = true;

	static voicemsg_t msg;

#ifdef DEDICATED
	return 0;
#endif

	if (remote_echo)
	{

#ifndef DEDICATED
		if (looped)
		{
			alGetSourcei(decode_source, AL_BUFFERS_PROCESSED, &buffersProcessed);
			if (buffersProcessed == 0)
			{
				ALenum state;

				alGetSourcei(decode_source, AL_SOURCE_STATE, &state);

				if (state == AL_STOPPED)
				{
					audio.play(decode_source);
				}
				return 0;
			}

		}
#endif
	}

	char client[128] = "";
	ret = sock.recvfrom((char *)&msg, SEGMENT_SIZE, client, 128);
	if (ret > 0)
	{
		size = ret;

		if (voice_recv_sequence > msg.sequence)
		{
			// old packet
			printf("voice chat got old packet %d older than %d\n", msg.sequence, voice_recv_sequence);
			voice_recv_sequence = msg.sequence + 1;
			return 0;
		}
		else
		{
			voice_recv_sequence = msg.sequence + 1;
		}


		if (remote_echo)
		{
			decode(msg.data, decode_pcm[pong], size);

			if (looped)
			{
#ifndef DEDICATED
				alSourceUnqueueBuffers(decode_source, 1, &uiBuffer);
				alBufferData(uiBuffer, AL_FORMAT_MONO16, decode_pcm[pong], size, VOICE_SAMPLE_RATE);
				alSourceQueueBuffers(decode_source, 1, &uiBuffer);
#endif
			}
			else
			{
#ifndef DEDICATED
				alBufferData(decode_buffer[pong], AL_FORMAT_MONO16, decode_pcm[pong], size, VOICE_SAMPLE_RATE);
				alSourceQueueBuffers(decode_source, 1, &decode_buffer[pong]);
#endif
			}


			pong++;
			if (pong >= NUM_PONG)
			{
				pong = 0;

				if (looped == false)
				{
					looped = true;
					audio.play(decode_source);
				}
			}
		}


	}

	if (ret > 0)
		return 1;
	else
		return 0;
}



void Voice::destroy()
{
	 opus_encoder_destroy(encoder); 
	 opus_decoder_destroy(decoder); 
} 

