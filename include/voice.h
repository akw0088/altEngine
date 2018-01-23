#include "include.h"

#ifndef VOICE_H
#define VOICE_H


#define SEGMENT_SIZE		1920
 
#define MAX_SEGMENT_SIZE 	(6 * SEGMENT_SIZE)
#define MAX_PACKET_SIZE		(3 * SEGMENT_SIZE)


class Voice
{
public:
	Voice();
	int init(unsigned short qport);
	void destroy();
	void bind(char *ip, unsigned short port);
	int encode(unsigned short *pcm, unsigned int size, unsigned char *data, int &num_bytes);
	int decode(unsigned char *data, unsigned short *pcm, unsigned int &size);
	int voice_send(Audio &audio, vector<client_t *> &client_list, bool client_flag, bool server_flag);
	int voice_recv(Audio &audio);

private:
	OpusEncoder *encoder;
	OpusDecoder *decoder;

	char voice_server[128];
	Net		net;
	unsigned short qport;
	unsigned short int		voice_send_sequence;
	unsigned short int		voice_recv_sequence;


#define NUM_PONG 2
	unsigned int mic_buffer[NUM_PONG];
	unsigned int mic_source;
	unsigned short mic_pcm[NUM_PONG][SEGMENT_SIZE];

	unsigned int decode_buffer[NUM_PONG];
	unsigned short decode_pcm[NUM_PONG][SEGMENT_SIZE];
	unsigned int decode_source;

};

#endif
