//=============================================================================
// This file is part of the altEngine distribution
// (https://github.com/akw0088/altEngine/)
// Copyright (c) 2018 Alexander Wright All Rights Reserved.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON - INFRINGEMENT.IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//=============================================================================

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
	int init(Audio &audio, unsigned short qport);
	void destroy();
	void bind(char *ip, unsigned short port);
	int encode(unsigned short *pcm, unsigned int size, unsigned char *data, int &num_bytes);
	int decode(unsigned char *data, unsigned short *pcm, unsigned int &size);
	int voice_send(Audio &audio, vector<client_t *> &client_list, bool client_flag, bool server_flag);
	int voice_recv(Audio &audio);

	char server[128];
private:
#ifndef __OBJC__
	OpusEncoder *encoder;
	OpusDecoder *decoder;
#endif
	
	Socket		sock;
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
