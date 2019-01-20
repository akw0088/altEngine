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

#ifndef NETCODE_H
#define NETCODE_H

class Netcode
{
	friend class Engine;
	friend class Quake3;
public:
	Netcode(Engine *engine);

	int GetKeyState(input_t &keyboard);
	input_t GetKeyState(int keystate);
	int server_recv();
	void server_send();
	void server_send_state(int client);
	void set_player_string(char *msg, client_t *client);
	void parse_player_string(char *msg);
	void send_player_string(servermsg_t &servermsg);
	void client_rename();
	void server_rename(char *oldname, char *newname, int self);
	int client_recv();
	void client_send(input_t input, Frame &camera_frame);
	int handle_servermsg(servermsg_t &servermsg, unsigned char *data, reliablemsg_t *reliablemsg);
	void disconnect();
	void kick(unsigned int i);
	int bind(int port);
	void connect(char *server);
	void chat(char *name, char *msg);
	void query_master();
	void report_master();

	int net_sendto(char *data, int size, client_t *client);
	int net_send(char *data, int size);
	int net_recv(char *data, int size, int delay);
	int net_recvfrom(char *data, int size, char *client, int client_size);
	int net_rawrecvfrom(int socket, char *data, int size, sockaddr *addr, int *socksize);
	int net_rawsendto(int socket, char *data, int size, sockaddr *addr, unsigned int socksize);

	int serialize_ents(unsigned char *data, unsigned short int &num_ents, unsigned int &data_size);
	int deserialize_ents(unsigned char *data, unsigned short int num_ents, unsigned int data_size);
	int deserialize_net_player(net_player_t *player, int index, int etype);
	int deserialize_net_rigid(net_rigid_t *rigid, int index, int etype);
	int deserialize_net_trigger(net_trigger_t *net, int index, int etype);
	int deserialize_net_projectile(net_projectile_t *net, int index, int etype);

protected:
	Engine *engine;

	//server
	bool	server_flag;
	vector <client_t *> client_list;

	//client
	bool	client_flag;
	unsigned int	last_server_sequence;
	unsigned int	qport;
	bool active_clients[8];

	netinfo_t netinfo;
	char *master_list[32];
	unsigned int num_master;



	//net stuff
	int		sequence;
	Socket		sock;
	reliablemsg_t	reliable[8];
	reliablemsg_t	client_reliable;
	int cl_skip;
	char sv_hostname[512];
	char sv_motd[512];
	char password[512];
	unsigned int sv_maxclients;
	unsigned short int net_port;

	net_entity_t delta_list[512];

	bool recording_demo;
	bool playing_demo;
	FILE *demofile;

#ifdef SERIAL
	handle_t handle;
#endif

};

#endif
