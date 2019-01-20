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

#include "netcode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Distance we ignore the server position before correcting (usually lags behind by ping)
#define DELTA_GRACE 200.0f

static unsigned char huffbuf[HUFFHEAP_SIZE];

Netcode::Netcode(Engine *engine)
{
	Netcode::engine = engine;

	cl_skip = 0;
	sv_maxclients = 16;
	recording_demo = false;
	playing_demo = false;

	demofile = NULL;
	last_server_sequence = 0;
	sequence = 0;
	client_flag = false;
	qport = 0;
	server_flag = false;

	net_port = 65535;

	sprintf(sv_hostname, "altEngine2 Server");
	sprintf(sv_motd, "Write engines, not games");
	sprintf(password, "iddqd");
	memset(&netinfo, 0, sizeof(netinfo));

	qport = rand();

	//net crap
	sequence = 0;
	server_flag = false;
	client_flag = false;
	memset(&reliable, 0, sizeof(reliablemsg_t));

	for (unsigned int i = 0; i < engine->max_player; i++)
	{
		reliable[i].sequence = -1;
	}
	last_server_sequence = 0;
}

// wrappers intended to allow for other communication protocols eg: serial port
// Note: Does not include master server report/query code
int Netcode::net_sendto(char *data, int size, client_t *client)
{
#ifdef SERIAL
	int num_sent = serial_write(handle, data, size);
#else
	int num_sent = sock.sendto(data, size, client->socketname);
#endif

	return num_sent;
}

int Netcode::net_send(char *data, int size)
{
#ifdef SERIAL
	return serial_write(handle, data, size);
#else
	return sock.send(data, size);
#endif
}

int Netcode::net_recv(char *data, int size, int delay)
{
#ifdef SERIAL
	if (recv_queue.size == SIZE_QUEUE)
		memset(&recv_queue, 0, sizeof(recv_queue));

	int num_read = serial_read(handle, data, size);
	enqueue(&recv_queue, (unsigned char *)data, num_read);
	dequeue_peek(&recv_queue, (unsigned char *)data, size);

	return num_read;
#else
	return sock.recv(data, size, delay);
#endif
}

int Netcode::net_recvfrom(char *data, int size, char *client, int client_size)
{
#ifdef SERIAL
	if (recv_queue.size == SIZE_QUEUE)
		memset(&recv_queue, 0, sizeof(recv_queue));

	snprintf(client, client_size - 1, "COMPORT");
	int num_read = serial_read(handle, data, size);
	enqueue(&recv_queue, (unsigned char *)data, num_read);
	dequeue_peek(&recv_queue, (unsigned char *)data, size);

	return num_read;
#else
	return sock.recvfrom(data, size, client, client_size);
#endif
}

int Netcode::net_rawrecvfrom(int socket, char *data, int size, sockaddr *addr, int *socksize)
{
#ifdef SERIAL
	if (recv_queue.size == SIZE_QUEUE)
		memset(&recv_queue, 0, sizeof(recv_queue));

	int num_read = serial_read(handle, data, size);
	enqueue(&recv_queue, (unsigned char *)data, num_read);
	dequeue_peek(&recv_queue, (unsigned char *)data, size);

	return num_read;
#else
#ifdef WIN32
	return ::recvfrom(socket, data, size, 0, addr, socksize);
#else
	return ::recvfrom(socket, data, size, 0, addr, (unsigned int *)socksize);
#endif
#endif
}


int Netcode::net_rawsendto(int socket, char *data, int size, sockaddr *addr, unsigned int socksize)
{
#ifdef SERIAL
	return serial_write(handle, data, size);
#else
	return ::sendto(socket, data, size, 0, addr, socksize);
#endif
}




void Netcode::server_send()
{
	static servermsg_t	servermsg;
	static unsigned char	data[16384];

	if (client_list.size() == 0)
		return;

	servermsg.sequence = sequence;
	servermsg.client_sequence = 0;
	servermsg.num_ents = 0;

	//	memset(&data[0], 0, sizeof(data));
	serialize_ents(&data[0], servermsg.num_ents, servermsg.data_size);
	netinfo.uncompressed_size = servermsg.data_size + SERVER_HEADER;
	servermsg.compressed_size = (unsigned short)huffman_compress((unsigned char *)&data[0], servermsg.data_size,
		servermsg.data, sizeof(servermsg.data), huffbuf);

	if (recording_demo)
	{
		fwrite(&servermsg, servermsg.compressed_size + SERVER_HEADER, 1, demofile);
	}


	for (unsigned int i = 0; i < client_list.size(); i++)
	{
		// idle client timeout

#ifndef SERIAL
		if (time(NULL) - client_list[i]->last_time > 90)
		{
			debugf("client %s timed out\n", client_list[i]->socketname);
			engine->clean_entity(client_list[i]->ent_id);
			//			delete entity_list[client_list[i]->ent_id]->rigid;
			//			entity_list[client_list[i]->ent_id]->rigid = NULL;
			//			entity_list[client_list[i]->ent_id]->model = NULL;
			client_list.erase(client_list.begin() + i);
			i--;
			continue;
		}
#endif

		if (reliable[i].size == 0)
			reliable[i].size = (unsigned short)(2 * sizeof(short) + strlen(reliable[i].msg) + 1);
		servermsg.client_sequence = client_list[i]->client_sequence;
		servermsg.length = SERVER_HEADER + servermsg.compressed_size + reliable[i].size;
		memcpy(&servermsg.data[servermsg.compressed_size], (void *)&reliable[i], reliable[i].size);

		client_list[i]->netinfo.num_ents = servermsg.num_ents;
		client_list[i]->netinfo.size = servermsg.length;
		netinfo.num_ents = servermsg.num_ents;
		netinfo.size = servermsg.length;

		int num_sent = net_sendto((char *)&servermsg, servermsg.length, client_list[i]);
		if (num_sent <= 0)
		{
			netinfo.send_full = true;

#ifdef WIN32
			int ret = WSAGetLastError();

			if (ret != WSAEWOULDBLOCK)
			{
				printf("Fatal socket error %d\n", ret);
				printf("size was %d bytes\n", servermsg.length);
			}
#else
			if (errno != EWOULDBLOCK)
			{
				printf("Fatal socket error %d\n", errno);
			}

#endif
		}
		else
		{
			netinfo.send_full = false;
		}

		if (num_sent != servermsg.length)
		{
			netinfo.send_partial = true;
		}
		else
		{
			netinfo.send_partial = false;
		}

		if (client_list[i]->needs_state)
		{
			server_send_state(i);
			client_list[i]->needs_state = false;
		}
	}
}

int Netcode::client_recv()
{
	static unsigned char data[16384];
	static servermsg_t	servermsg;
	reliablemsg_t *rmsg = NULL;
	unsigned int socksize = sizeof(sockaddr_in);


	// get entity information

	int size = net_rawrecvfrom(sock.sockfd, (char *)&servermsg, sizeof(servermsg_t), (sockaddr *)&(sock.servaddr), (int *)&socksize);

	if (size > 0)
	{
		if (size < servermsg.length)
		{
			printf("Warning packet size mismatch: %d %d\n", size, servermsg.length);
			return -1;
		}

#ifdef SERIAL
		static unsigned char buff[4096];
		dequeue(&recv_queue, buff, servermsg.length);
#endif

		if (servermsg.sequence < last_server_sequence)
		{
			printf("Got old server packet\n");
			netinfo.dropped++;
			return 1;
		}

		double delay = ping_time_end(servermsg.client_sequence);
		if (delay > 0.0)
			netinfo.ping = (int)delay;

		//		printf("Recieved %d ents from server\n", servermsg.num_ents);
		if (servermsg.client_sequence > client_reliable.sequence)
		{
			memset(client_reliable.msg, 0, LINE_SIZE);
			client_reliable.size = 0;
			client_reliable.sequence = -1;
		}

		if ((unsigned int)servermsg.length > SERVER_HEADER + servermsg.compressed_size + sizeof(int) + 1)
		{
			rmsg = (reliablemsg_t *)&servermsg.data[servermsg.compressed_size];
		}

		if (servermsg.num_ents)
		{
			unsigned int dsize = 0;

			dsize = huffman_decompress(servermsg.data, servermsg.compressed_size, data, sizeof(data), huffbuf);
			if (dsize != servermsg.data_size)
			{
				printf("Decompressed size mismatch: %d %d\n", dsize, (int)(servermsg.data_size));
				return 1;
			}
			netinfo.uncompressed_size = dsize + SERVER_HEADER;
		}

		netinfo.num_ents = servermsg.num_ents;
		netinfo.size = servermsg.length;
		netinfo.sequence_delta = sequence - servermsg.client_sequence;
		netinfo.ping_tick = netinfo.sequence_delta * TICK_MS;

		handle_servermsg(servermsg, data, rmsg);
		last_server_sequence = servermsg.sequence;
	}
	else
	{
		return 0;
	}
	return 0;
}

void Netcode::client_send(input_t input, Frame &camera_frame)
{
	static clientmsg_t clientmsg;
	unsigned int socksize = sizeof(sockaddr_in);
	int keystate = GetKeyState(input);

	// Dont want to skip an attack command, so fiddle with send rate
	if (input.attack == false && cl_skip != 0)
	{
		if (engine->tick_num % cl_skip == 0)
			return;
	}

	// send keyboard state
	memset(&clientmsg, 0, sizeof(clientmsg_t));
	clientmsg.sequence = sequence;
	clientmsg.qport = qport;
	clientmsg.server_sequence = last_server_sequence;
	clientmsg.up[0] = camera_frame.up.x;
	clientmsg.up[1] = camera_frame.up.y;
	clientmsg.up[2] = camera_frame.up.z;
	clientmsg.forward[0] = camera_frame.forward.x;
	clientmsg.forward[1] = camera_frame.forward.y;
	clientmsg.forward[2] = camera_frame.forward.z;
	clientmsg.pos[0] = camera_frame.pos.x;
	clientmsg.pos[1] = camera_frame.pos.y;
	clientmsg.pos[2] = camera_frame.pos.z;

	clientmsg.num_cmds = 1;
	memcpy(clientmsg.data, &keystate, sizeof(int));
	memcpy(&clientmsg.data[clientmsg.num_cmds * sizeof(int)], (void *)&client_reliable, client_reliable.size);
	clientmsg.length = CLIENT_HEADER + clientmsg.num_cmds * sizeof(int) + client_reliable.size;

	ping_time_start(sequence);
	int num_sent = net_rawsendto(sock.sockfd, (char *)&clientmsg, clientmsg.length, (sockaddr *)&(sock.servaddr), socksize);

	if (server_flag == false)
	{
		if (num_sent <= 0)
			netinfo.send_full = true;
		else
			netinfo.send_full = false;

		if (num_sent != clientmsg.length)
		{
			netinfo.send_partial = true;
		}
		else
		{
			netinfo.send_partial = false;
		}
	}
}

int Netcode::handle_servermsg(servermsg_t &servermsg, unsigned char *data, reliablemsg_t *reliablemsg)
{
	//	int ent_id = -1;

	if (reliablemsg != NULL)
	{
		if (last_server_sequence <= reliablemsg->sequence)
		{
			int ret;

			debugf("server to client: %s\n", reliablemsg->msg);

			char *start = strstr(reliablemsg->msg, "<chat>");

			if (start)
			{
				static char msg[256];

				start += 6;
				char *end = strstr(start, "</chat>");

				// prevent duplicate chats
				if (end && strncmp(msg, start, end - start) != 0)
				{
					memcpy(msg, start, end - start);
					engine->menu.print_chat(msg);
					engine->game->set_chat_timer(3 * TICK_RATE);

					#define SND_TALK 268
					engine->play_wave_global(SND_TALK);
				}
			}

			char *pstate = strstr(reliablemsg->msg, "<data>");
			if (pstate)
			{
				serverdata_t *state = (serverdata_t *)pstate;

				debugf("client received server state\n");
				engine->game->set_state(state);
			}

			ret = strcmp(reliablemsg->msg, "<disconnect/>");
			if (ret == 0)
			{
				engine->unload();
			}

			parse_player_string(reliablemsg->msg);

		}
	}

	if (engine->entity_list.size() == 0)
	{
		debugf("Client failed load map correctly\n");
		engine->unload();
		return 0;
	}

	deserialize_ents(data, servermsg.num_ents, servermsg.data_size);
	return 0;
}

// packs keyboard input into an integer
int Netcode::GetKeyState(input_t &input)
{
	int keystate = 0;

	if (input.attack)
		keystate |= 1;
	if (input.use)
		keystate |= 2;
	if (input.jump)
		keystate |= 4;
	if (input.moveup)
		keystate |= 8;
	if (input.moveleft)
		keystate |= 16;
	if (input.movedown)
		keystate |= 32;
	if (input.moveright)
		keystate |= 64;
	if (input.weapon_down)
		keystate |= 128;
	if (input.weapon_up)
		keystate |= 256;

	return keystate;
}

// unpacks keyboard integer into keyboard state
input_t Netcode::GetKeyState(int keystate)
{
	input_t input;

	memset((void *)&input, 0, sizeof(input_t));
	if (keystate & 1)
		input.attack = true;
	else
		input.attack = false;

	if (keystate & 2)
		input.use = true;
	else
		input.use = false;

	if (keystate & 4)
		input.jump = true;
	else
		input.jump = false;

	if (keystate & 8)
		input.moveup = true;
	else
		input.moveup = false;

	if (keystate & 16)
		input.moveleft = true;
	else
		input.moveleft = false;

	if (keystate & 32)
		input.movedown = true;
	else
		input.movedown = false;

	if (keystate & 64)
		input.moveright = true;
	else
		input.moveright = false;

	if (keystate & 128)
		input.weapon_down = true;
	else
		input.weapon_down = false;

	if (keystate & 256)
		input.weapon_up = true;
	else
		input.weapon_up = false;

	return input;
}

void Netcode::server_send_state(int client)
{
	serverdata_t data;
	int offset = strlen(reliable[client].msg) + 1;
	engine->game->get_state(&data);
	reliable[client].msg[offset - 1] = ' ';
	memcpy(&reliable[client].msg[offset], &data, sizeof(serverdata_t));
	reliable[client].msg[offset + sizeof(serverdata_t) + 1] = '\0';
	reliable[client].size = 2 * sizeof(int) + sizeof(serverdata_t) + offset;
	reliable[client].sequence = sequence;
}


void Netcode::set_player_string(char *msg, client_t *client)
{
	int serverid = engine->find_type(ENT_PLAYER, 0);

	if (serverid != -1)
	{
		sprintf(msg, "<player> %d \"%s\", %d \"%s\", ", client->ent_id, engine->entity_list[client->ent_id]->player->name,
			serverid, engine->entity_list[serverid]->player->name);
	}
	else
	{
		sprintf(msg, "<player> %d \"%s\", ", client->ent_id, engine->entity_list[client->ent_id]->player->name);
	}
	for (unsigned int i = 0; i < client_list.size(); i++)
	{
		char client_index[128];
		if (client_list[i] == client)
			continue;

		sprintf(client_index, "%d \"%s\", ", client_list[i]->ent_id, engine->entity_list[client_list[i]->ent_id]->player->name);
		strcat(msg, client_index);
	}
	strcat(msg, "</player> ");
}

void Netcode::parse_player_string(char *msg)
{
	char *start = strstr(msg, "<player>");
	char *end = strstr(msg, "</player>");
	int count = 0;
	int index;

	if (start && end)
	{
		char temp[512];

		start += 8;
		int length = (int)(end - start);

		memset(temp, 0, sizeof(temp));
		memcpy(temp, start, length);
		char *line = strtok(temp, ",");
		while (line)
		{
			char name[128] = { 0 };
			int ret = -1;

			ret = sscanf(line, "%d \"%[^\"]s", &index, name);
			if (ret != 2)
			{
				printf("Failed to parse player [%s]\n", line);
				line = strtok(NULL, ",");
				continue;
			}

			if (count == 0 && active_clients[count] == false)
			{
				Entity *ent = engine->entity_list[index];

				active_clients[count] = true;
				engine->clean_entity(index);
				ent->rigid = new RigidBody(ent);
				ent->model = ent->rigid;
				ent->rigid->clone(*(engine->thug22->model));
				ent->rigid->flags.step_flag = true;
				ent->position += ent->rigid->center;
				ent->player = new Player(ent, engine->gfx, engine->audio, 21, TEAM_NONE, ENT_PLAYER, engine->game->get_model_table());
				ent->player->type = PLAYER;
				sprintf(ent->player->name, "%s", name);
				ent->player->local = true;
				engine->camera_frame.pos = ent->position;
				printf("Adding client player \"%s\" at index %d\n", name, index);
			}
			else
			{
				if (active_clients[count] == false)
				{
					active_clients[count] = true;
					Entity *ent = engine->entity_list[index];
					engine->clean_entity(index);
					ent->rigid = new RigidBody(ent);
					ent->model = ent->rigid;
					ent->rigid->clone(*(engine->thug22->model));
					ent->rigid->flags.step_flag = true;
					ent->position += ent->rigid->center;
					ent->player = new Player(ent, engine->gfx, engine->audio, 21, TEAM_NONE, ENT_SERVER, engine->game->get_model_table());
					ent->player->local = false;
					ent->player->type = SERVER;
					sprintf(ent->player->name, "%s", name);
					printf("Adding server player \"%s\" at index %d\n", name, index);
				}
			}
			line = strtok(NULL, ",");
			count++;
		}
	}
}


void Netcode::send_player_string(servermsg_t &servermsg)
{
	static char player_str[4096];

	// Notify other clients of new player
	for (unsigned int i = 0; i < client_list.size() - 1; i++)
	{

		set_player_string(player_str, client_list[i]);
		strcat(reliable[i].msg, player_str);

		reliable[i].size = (unsigned short)(2 * sizeof(unsigned short int) + strlen(reliable[i].msg) + 1);
		reliable[i].sequence = sequence;

		memset(&servermsg, 0, sizeof(servermsg));
		servermsg.client_sequence = client_list[i]->client_sequence;
		servermsg.sequence = sequence;
		servermsg.num_ents = 0;
		servermsg.compressed_size = 0;
		servermsg.length = SERVER_HEADER + reliable[i].size;
		memcpy(&servermsg.data[servermsg.data_size], &reliable[i], reliable[i].size);

		net_sendto((char *)&servermsg, servermsg.length, client_list[i]);
	}
}

int Netcode::server_recv()
{
	static servermsg_t	servermsg;
	static clientmsg_t clientmsg;
	char socketname[LINE_SIZE];
	bool connected = false;
	int index = -1;
	//int ret = -1;
	char name[LINE_SIZE] = { 0 };

	// get client packet
	int size = net_recvfrom((char *)&clientmsg, 8192, socketname, LINE_SIZE);

	if (size <= 0)
	{
		netinfo.recv_empty = true;
		return 0;
	}
	netinfo.recv_empty = false;


	if (clientmsg.length < size)
	{
		printf("Warning: Packet size mismatch\n");
		return -1;
	}
	else
	{
#ifdef SERIAL
		static unsigned char buff[4096];
		dequeue(&recv_queue, buff, clientmsg.length);
#endif
	}

	// see if this ip/port combo already connected to server
	for (unsigned int i = 0; i < client_list.size(); i++)
	{
		if (strcmp(client_list[i]->socketname, socketname) == 0)
		{
			if (client_list[i]->qport == clientmsg.qport)
			{
				index = i;
				connected = true;
				break;
			}
		}
	}

	// update client input
	if (connected)
	{
		int keystate;
		memcpy(&keystate, clientmsg.data, 4);
		input_t clientkeys = GetKeyState(keystate);

		if (clientmsg.sequence <= client_list[index]->client_sequence)
		{
			printf("Got old client packet\n");
			client_list[index]->netinfo.dropped++;
			return 1;
		}

		client_list[index]->server_sequence = clientmsg.server_sequence; //  Client is ACK'ing our sequence

		if (clientmsg.server_sequence > reliable[index].sequence)
		{
			memset(reliable[index].msg, 0, LINE_SIZE);
			reliable[index].size = 0;
			reliable[index].sequence = -1;
		}

		if ((unsigned int)client_list[index]->ent_id > engine->entity_list.size())
		{
			printf("Invalid Entity\n");
			return 1;
		}

		client_list[index]->last_time = (unsigned int)time(NULL);


		Frame client_frame;
		vec3 client_pos; // dont use, client can cheat, only to see how bad delta is

		client_frame.up.x = clientmsg.up[0];
		client_frame.up.y = clientmsg.up[1];
		client_frame.up.z = clientmsg.up[2];
		client_frame.forward.x = clientmsg.forward[0];
		client_frame.forward.y = clientmsg.forward[1];
		client_frame.forward.z = clientmsg.forward[2];
		vec3 right = vec3::crossproduct(client_frame.up, client_frame.forward);
		right.normalize();

		client_pos.x = clientmsg.pos[0];
		client_pos.y = clientmsg.pos[1];
		client_pos.z = clientmsg.pos[2];

		Entity *client = engine->entity_list[client_list[index]->ent_id];

		client_frame.set(client->rigid->morientation);
		client_frame.pos = client->position + client->rigid->center;
		client_list[index]->input = clientkeys;
		client_list[index]->position_delta = client->position - client_pos;

		if ((unsigned int)clientmsg.length > CLIENT_HEADER + sizeof(int) + sizeof(int) + 1)
		{
			reliablemsg_t *reliablemsg = (reliablemsg_t *)&clientmsg.data[4];

			if (client_list[index]->client_sequence <= reliablemsg->sequence)
			{
				char msg[LINE_SIZE] = { 0 };

				debugf("client to server: %s\n", reliablemsg->msg);


				char *start = strstr(reliablemsg->msg, "<chat>");
				if (start)
				{

					sprintf(name, "%s", start + 6);
					char *end = strchr(name, ':');
					if (end != NULL)
					{
						end[0] = '\0';
						char *terminate = strstr(end + 2, "</chat>");
						*terminate = '\0';
						sprintf(msg, "say \"%s\"", end + 2);
						//  Echoes chat back to all clients
						chat(name, msg);
					}
				}

				start = strstr(reliablemsg->msg, "<getplayer/>");
				if (start)
				{
					send_player_string(servermsg);
				}
			}
		}
		client_list[index]->netinfo.sequence_delta = sequence - clientmsg.sequence;
		client_list[index]->netinfo.ping = netinfo.sequence_delta * TICK_MS;
		client_list[index]->client_sequence = clientmsg.sequence;
	}
	else
	{
		printf("Got input packet from unconnected client %s qport %d\n", socketname, clientmsg.qport);
	}

	// client not in list, check if it is a connect msg
	reliablemsg_t *reliablemsg = (reliablemsg_t *)&clientmsg.data[clientmsg.num_cmds * sizeof(int)];
	if (strcmp(reliablemsg->msg, "<connect/>") == 0)
	{
		client_t temp_client;

		strcpy(temp_client.socketname, socketname);
		debugf("client %s qport %d connected\n", socketname, clientmsg.qport);

		if (client_list.size() > sv_maxclients)
		{
			debugf("Client connection failed: server full");
			servermsg.sequence = sequence;
			servermsg.client_sequence = clientmsg.sequence;
			servermsg.num_ents = 0;
			sprintf(reliable[index].msg, "<serverfull/>");
			reliable[index].size = (unsigned short)(2 * sizeof(short) + strlen(reliable[index].msg) + 1);
			reliable[index].sequence = sequence;

			memcpy(&servermsg.data[0], &reliable, reliable[index].size);
			servermsg.length = SERVER_HEADER + reliable[index].size;
			net_sendto((char *)&servermsg, servermsg.length, &temp_client);
			return 1;
		}

		servermsg.sequence = sequence;
		servermsg.client_sequence = clientmsg.sequence;
		servermsg.num_ents = 0;
		sprintf(reliable[index].msg, "<map>%s</map>", engine->q3map.map_name);
		reliable[index].size = (unsigned short)(2 * sizeof(short) + strlen(reliable[index].msg) + 1);
		reliable[index].sequence = sequence;

		memcpy(&servermsg.data[0], &reliable[index], reliable[index].size);
		servermsg.length = SERVER_HEADER + reliable[index].size;

		net_sendto((char *)&servermsg, servermsg.length, &temp_client);
		debugf("sent client map data\n");
	}
	else if (1 == sscanf(reliablemsg->msg, "<player \"%[^\"]/>", name))
	{
		bool found = false;
		unsigned int i = 0;

		for (i = 0; i < client_list.size(); i++)
		{
			if (strcmp(client_list[i]->socketname, socketname) == 0)
			{
				if (client_list[i]->qport == clientmsg.qport)
				{
					found = true;
					Player *player = engine->entity_list[client_list[i]->ent_id]->player;

					if (strcmp(name, player->name) != 0)
					{
						char msg[256];

						sprintf(msg, "say \"%s renamed to %s\"", player->name, name);
						chat("Server", msg);
						sprintf(player->name, "%s", name);
					}
					client_list[i]->client_sequence = clientmsg.sequence;
					client_list[i]->server_sequence = sequence + 1;
					client_list[i]->last_time = (unsigned int)time(NULL);
					break;
				}
			}
		}

		if (found)
		{
			//printf("Client already spawned, ignoring\n");
			return 1;
		}
		client_t *client = new client_t;
		if (client == NULL)
		{
			debugf("malloc failed allocating client");
			return 1;
		}

		memset(client, 0, sizeof(client_t));
		client->last_time = (unsigned int)time(NULL);
		strcpy(client->socketname, socketname);
		client->qport = clientmsg.qport;
		client->needs_state = true;


		client_list.push_back(client);
		debugf("client %s qport %d spawned\n", client->socketname, client->qport);
		client->client_sequence = clientmsg.sequence;

		// assign entity to client
		//set to zero if we run out of info_player_deathmatches

		index = (int)client_list.size() - 1;
		engine->game->add_player(engine->entity_list, CLIENT, client->ent_id, name);
		printf("client %s \"%s\" qport %d got entity %d\n", socketname, name, client->qport, client->ent_id);


		servermsg.sequence = sequence;
		servermsg.client_sequence = clientmsg.sequence;
		servermsg.num_ents = 0;





		// let new client know how many players exist
		char player_str[1024];
		set_player_string(player_str, client);
		strcat(reliable[index].msg, player_str);

		char motd[256];
		sprintf(motd, "<chat>Welcome to %s: %s</chat>", sv_hostname, sv_motd);
		strcat(reliable[index].msg, motd);

		reliable[index].size = (unsigned short)(2 * sizeof(unsigned short int) + strlen(reliable[index].msg) + 1);
		reliable[index].sequence = sequence;


		memcpy(&servermsg.data[servermsg.data_size], &reliable[index], reliable[index].size);
		servermsg.length = SERVER_HEADER + reliable[index].size;

		net_sendto((char *)&servermsg, servermsg.length, client);
		debugf("Client is now entity %d\n", client->ent_id);


		send_player_string(servermsg);

	}
	else if (strcmp(reliablemsg->msg, "getstatus") == 0)
	{
		/*
		"getstatus" responds with all the info that qplug or qspy can see about the server and all connected players.
		Used for getting detailed information after the simple info query.  It is sent along with a challenge string.
		The server will respond with a "getstatusResponse" packet.
		*/
	}
	else if (strcmp(reliablemsg->msg, "getinfo") == 0)
	{
		/*
		"getinfo" responds with a short info message that should be enough to determine if a user is interested in a server
		to do a full status.  It is also sent with a challenge string.
		*/

		debugf("getinfo request from %s\n", socketname);
		servermsg.sequence = sequence;
		servermsg.client_sequence = clientmsg.sequence;
		servermsg.num_ents = 0;
		sprintf(reliable[index].msg, "/sv_hostname %s/map %s/players %d/maxplayers %d/gametype %d/fraglimit %d/timelimit %d/capturelimit %d/",
			sv_hostname, engine->q3map.map_name, (int)client_list.size(), engine->max_player, engine->game->get_gametype(),
			engine->game->get_fraglimit(), engine->game->get_timelimit(), engine->game->get_capturelimit());
		reliable[index].size = (unsigned short)(2 * sizeof(unsigned short int) + strlen(reliable[index].msg) + 1);
		reliable[index].sequence = sequence;

		memcpy(&servermsg.data[servermsg.data_size], &reliable[index], reliable[index].size);
		servermsg.length = SERVER_HEADER + servermsg.data_size + reliable[index].size;

		client_t temp_client;

		strcpy(temp_client.socketname, socketname);
		net_sendto((char *)&servermsg, servermsg.length, &temp_client);
	}
	else if (strcmp(reliablemsg->msg, "getchallenge") == 0)
	{
		/*
		"getchallenge" returns a challenge number that can be used in a subsequent connectResponse command.
		We do this to prevent denial of service attacks that flood the server with invalid connection IPs.
		With a challenge, they must give a valid IP address.
		The server will respond with a "challengeResponse" packet.
		*/
	}
	else if (strstr(reliablemsg->msg, "rcon") != 0)
	{
		char cmd[512] = "";
		char pass[512] = "";

		if (sscanf(reliablemsg->msg, "rcon %s %s", &pass[0], &cmd[0]) == 2)
		{
			if (strcmp(pass, password) == 0)
			{
				engine->console(cmd);
			}
		}
	}
	/*
	"connect" is the first step in a client connecting to a server.  You send the "connect" string followed by the infoString
	containing the protocol version of the client, the qport, the challenge string (obtained via getchallenge), and the userinfo.
	*/

	return 1;
}


int Netcode::serialize_ents(unsigned char *data, unsigned short int &num_ents, unsigned int &data_size)
{
	data_size = 0;
	for (unsigned int i = 0; i < engine->entity_list.size(); i++)
	{
		net_entity_t ent;
		net_rigid_t *net_rigid = (net_rigid_t *)&(ent.data[0]);
		net_trigger_t *net_trigger = (net_trigger_t *)&(ent.data[0]);
		net_player_t *net_player = (net_player_t *)&ent.data[0];
		net_projectile_t *net_projectile = (net_projectile_t *)&(ent.data[0]);
		int size = 0;

		RigidBody *rigid = engine->entity_list[i]->rigid;


		memset(&ent, 0, sizeof(net_entity_t));
		ent.index = i;
		ent.etype = (net_ent_t)engine->entity_list[i]->nettype;
		ent.ctype = NET_UNKNOWN;


		Projectile *projectile = engine->entity_list[i]->projectile;
		if (projectile)
		{
			net_projectile->active = 0;

			if (engine->entity_list[i]->projectile->active)
				net_projectile->active = 1;

			net_projectile->owner = engine->entity_list[i]->projectile->owner;

			ent.ctype = NET_PROJECTILE;
			size = SIZE_NET_ENTITY_HEADER + sizeof(net_projectile_t);
			/*
			quaternion q;

			q.to_quat(rigid->morientation);

			if (q.s < 0)
			{
			net_projectile->quat.x = -q.x;
			net_projectile->quat.y = -q.y;
			net_projectile->quat.z = -q.z;
			}
			else
			{
			net_projectile->quat.x = q.x;
			net_projectile->quat.y = q.y;
			net_projectile->quat.z = q.z;
			}
			*/

			net_projectile->forward.x = rigid->morientation.m[6];
			net_projectile->forward.y = rigid->morientation.m[7];
			net_projectile->forward.z = rigid->morientation.m[8];
			net_projectile->right.x = rigid->morientation.m[0];
			net_projectile->right.y = rigid->morientation.m[1];
			net_projectile->right.z = rigid->morientation.m[2];


			net_projectile->angular_velocity = rigid->angular_velocity;
			net_projectile->velocity = rigid->velocity;
			net_projectile->position = engine->entity_list[i]->position;


			if (memcmp(&delta_list[i], &ent, size) != 0)
			{
				memcpy(&delta_list[i], &ent, size);
				memcpy(&data[data_size], &ent, size);
				data_size += size;
				num_ents++;
			}
			//			printf("serialized trigger index %d\n", i);
			continue;
		}

		Trigger *trigger = engine->entity_list[i]->trigger;
		if (trigger)
		{
			net_trigger->active = 0;

			if (engine->entity_list[i]->trigger->active)
				net_trigger->active = 1;

			ent.ctype = NET_TRIGGER;
			size = SIZE_NET_ENTITY_HEADER + sizeof(net_trigger_t);


			if (memcmp(&delta_list[i], &ent, size) != 0)
			{
				memcpy(&delta_list[i], &ent, size);
				memcpy(&data[data_size], &ent, size);
				data_size += size;
				num_ents++;
			}
			//			printf("serialized trigger index %d\n", i);
			continue;
		}

		Player *player = engine->entity_list[i]->player;
		if (player != NULL && rigid != NULL)
		{
			net_player->health = player->health;
			net_player->armor = player->armor;
			net_player->weapon_flags = player->weapon_flags;
			//ent.current_weapon = player->current_weapon;
			net_player->ammo_bullets = player->ammo_bullets;
			net_player->ammo_shells = player->ammo_shells;
			net_player->ammo_rockets = player->ammo_rockets;
			net_player->ammo_lightning = player->ammo_lightning;
			net_player->ammo_slugs = player->ammo_slugs;
			net_player->ammo_plasma = player->ammo_plasma;

			/*
			quaternion q;

			q.to_quat(rigid->morientation);
			if (q.s < 0)
			{
			net_projectile->quat.x = -q.x;
			net_projectile->quat.y = -q.y;
			net_projectile->quat.z = -q.z;
			}
			else
			{
			net_projectile->quat.x = q.x;
			net_projectile->quat.y = q.y;
			net_projectile->quat.z = q.z;
			}
			*/

			net_player->forward.x = rigid->morientation.m[6];
			net_player->forward.y = rigid->morientation.m[7];
			net_player->forward.z = rigid->morientation.m[8];
			net_player->right.x = rigid->morientation.m[0];
			net_player->right.y = rigid->morientation.m[1];
			net_player->right.z = rigid->morientation.m[2];

			net_player->angular_velocity = player->entity->rigid->angular_velocity;
			net_player->velocity = player->entity->rigid->velocity;
			net_player->position = player->entity->position;
			net_player->center = player->entity->rigid->center;

			ent.ctype = NET_PLAYER;
			size = SIZE_NET_ENTITY_HEADER + sizeof(net_player_t);

			//			if (memcmp(&delta_list[i], &ent, size) != 0)
			{
				memcpy(&delta_list[i], &ent, size);
				memcpy(&data[data_size], &ent, size);
				data_size += size;
				num_ents++;
			}
			//			printf("Sent player data index %d pos %3.3f %3.3f %3.3f\n", i, net_player->position.x, net_player->position.y, net_player->position.z);
			continue;
		}

		if (rigid)
		{
			//			net_rigid->morientation = rigid->morientation;
			net_rigid->forward.x = rigid->morientation.m[6];
			net_rigid->forward.y = rigid->morientation.m[7];
			net_rigid->forward.z = rigid->morientation.m[8];
			net_rigid->right.x = rigid->morientation.m[0];
			net_rigid->right.y = rigid->morientation.m[1];
			net_rigid->right.z = rigid->morientation.m[2];
			net_rigid->angular_velocity = rigid->angular_velocity;
			net_rigid->velocity = rigid->velocity;
			net_rigid->position = engine->entity_list[i]->position;

			ent.ctype = NET_RIGID;
			size = SIZE_NET_ENTITY_HEADER + sizeof(net_rigid_t);

			if (memcmp(&delta_list[i], &ent, size) != 0)
			{
				memcpy(&delta_list[i], &ent, size);
				memcpy(&data[data_size], &ent, size);
				data_size += size;
				num_ents++;
			}
			//			printf("serialized rigid index %d\n", i);
			continue;
		}


	}

	//	printf("serialize_ents: size is %d\n", num_ents * sizeof(entity_t));
	return 0;
}

int Netcode::deserialize_ents(unsigned char *data, unsigned short int num_ents, unsigned int data_size)
{
	for (int i = 0; i < num_ents; i++)
	{
		net_entity_t *ent = (net_entity_t *)data;

		if (ent->index < 0 || ent->index >= engine->entity_list.size())
		{
//			disconnect();
//			engine->unload();
			return -1;
		}

		switch (ent->ctype)
		{
		case NET_PLAYER:
			deserialize_net_player((net_player_t *)ent->data, ent->index, ent->etype);
			data += SIZE_NET_ENTITY_HEADER + sizeof(net_player_t);
			break;
		case NET_RIGID:
			deserialize_net_rigid((net_rigid_t *)ent->data, ent->index, ent->etype);
			data += SIZE_NET_ENTITY_HEADER + sizeof(net_rigid_t);
			break;
		case NET_TRIGGER:
			deserialize_net_trigger((net_trigger_t *)ent->data, ent->index, ent->etype);
			data += SIZE_NET_ENTITY_HEADER + sizeof(net_trigger_t);
			break;
		case NET_PROJECTILE:
			deserialize_net_projectile((net_projectile_t *)ent->data, ent->index, ent->etype);
			data += SIZE_NET_ENTITY_HEADER + sizeof(net_projectile_t);
			break;
		default:
			printf("Unknown net_entity %d\n", ent->ctype);
			break;
		}
	}
	return 0;
}

int Netcode::deserialize_net_player(net_player_t *net, int index, int etype)
{
	Player *player = engine->entity_list[index]->player;
	RigidBody *rigid = engine->entity_list[index]->rigid;

	if (player != NULL)
	{
		player->health = net->health;
		player->armor = net->armor;
		player->weapon_flags = net->weapon_flags;
		// will force server to sync to our current weapon
		//			entity_list[ent[i].id]->player->current_weapon = ent[i].current_weapon;

		if (net->ammo_bullets - player->ammo_bullets > 1)
			player->ammo_bullets = net->ammo_bullets;
		if (net->ammo_shells - player->ammo_shells > 1)
			player->ammo_shells = net->ammo_shells;
		if (net->ammo_rockets - player->ammo_rockets > 1)
			player->ammo_rockets = net->ammo_rockets;
		if (net->ammo_lightning - player->ammo_lightning > 1)
			player->ammo_lightning = net->ammo_lightning;
		if (net->ammo_slugs - player->ammo_slugs > 1)
			player->ammo_slugs = net->ammo_slugs;
		if (net->ammo_plasma - player->ammo_plasma > 1)
			player->ammo_plasma = net->ammo_plasma;

		// current entity has the clients predicted position
		// the net->position has the server (lagged) position
		// Need to lerp between the two, but then we have time sync issues
		rigid->center = net->center;

		/*
		quaternion q;

		q.x = net->quat.x;
		q.y = net->quat.y;
		q.z = net->quat.z;
		q.compute_w();
		q.s = -q.s;


		rigid->morientation = q.to_matrix();
		*/
		vec3 up = vec3::crossproduct(net->right, net->forward);
		rigid->morientation.m[0] = net->right.x;
		rigid->morientation.m[1] = net->right.y;
		rigid->morientation.m[2] = net->right.z;
		rigid->morientation.m[3] = up.x;
		rigid->morientation.m[4] = up.y;
		rigid->morientation.m[5] = up.z;
		rigid->morientation.m[6] = net->forward.x;
		rigid->morientation.m[7] = net->forward.y;
		rigid->morientation.m[8] = net->forward.z;

		rigid->velocity = net->velocity;

		vec3 position_delta;

		position_delta = engine->camera_frame.pos - net->position;

		if (position_delta.magnitude() >= DELTA_GRACE && engine->entity_list[index]->player->local)
		{
			// Server will use catch up to our position (eventually)
			// So only force a correction for large deltas
			engine->camera_frame.pos = net->position;
			engine->entity_list[index]->position = net->position;
		}
		else
		{
			engine->camera_frame.pos = net->position;
			engine->entity_list[index]->position = net->position;
		}



		//printf("Got player data index %d pos %3.3f %3.3f %3.3f\n", index, net->position.x, net->position.y, net->position.z );
	}
	else
	{
		//		debugf("deserialize_net_player() failed to find player at index %d\n", index);
		if (client_reliable.size == 0)
		{
			sprintf(client_reliable.msg, "<getplayer/>");
			client_reliable.size = (unsigned short)(4 + strlen(client_reliable.msg) + 1);
			client_reliable.sequence = sequence;
		}
		return -1;
	}

	return 0;
}

int Netcode::deserialize_net_rigid(net_rigid_t *net, int index, int etype)
{
	Entity *ent = engine->entity_list[index];

	// Check if an entity is a projectile that needs to be loaded
	if (etype != ent->nettype)
	{
		engine->game->make_dynamic_ent((net_ent_t)etype, index);
	}

	if (ent->rigid)
	{
		RigidBody *rigid = ent->rigid;
		ent->position = net->position;
		rigid->velocity = net->velocity;
		rigid->angular_velocity = net->angular_velocity;
		//		rigid->morientation = net->morientation;
		vec3 up = vec3::crossproduct(net->right, net->forward);
		rigid->morientation.m[0] = net->right.x;
		rigid->morientation.m[1] = net->right.y;
		rigid->morientation.m[2] = net->right.z;
		rigid->morientation.m[3] = up.x;
		rigid->morientation.m[4] = up.y;
		rigid->morientation.m[5] = up.z;
		rigid->morientation.m[6] = net->forward.x;
		rigid->morientation.m[7] = net->forward.y;
		rigid->morientation.m[8] = net->forward.z;

		//		printf("Got rigid data index %d\n", index);
	}
	else
	{
		printf("deserialize_net_rigid() failed to find rigid at index %d\n", index);
		return -1;
	}

	return 0;
}

int Netcode::deserialize_net_trigger(net_trigger_t *net, int index, int etype)
{
	Entity *ent = engine->entity_list[index];

	// Check if an entity is a projectile that needs to be loaded
	if (etype != ent->nettype)
	{
		engine->game->make_dynamic_ent((net_ent_t)etype, index);
	}

	if (ent->trigger)
	{
		if (net->active)
			ent->trigger->active = true;
		else
			ent->trigger->active = false;

		//		printf("Got trigger data index %d\n", index);
	}
	else
	{
		printf("deserialize_net_trigger() failed to find trigger at index %d\n", index);
		return -1;
	}

	return 0;
}

int Netcode::deserialize_net_projectile(net_projectile_t *net, int index, int etype)
{
	Entity *ent = engine->entity_list[index];
	// Check if an entity is a projectile that needs to be loaded
	if (etype != ent->nettype)
	{
		engine->game->make_dynamic_ent((net_ent_t)etype, index);
	}

	RigidBody *rigid = ent->rigid;

	if (rigid)
	{
		ent->position = net->position;
		rigid->velocity = net->velocity;
		rigid->angular_velocity = net->angular_velocity;

		vec3 up = vec3::crossproduct(net->right, net->forward);
		rigid->morientation.m[0] = net->right.x;
		rigid->morientation.m[1] = net->right.y;
		rigid->morientation.m[2] = net->right.z;
		rigid->morientation.m[3] = up.x;
		rigid->morientation.m[4] = up.y;
		rigid->morientation.m[5] = up.z;
		rigid->morientation.m[6] = net->forward.x;
		rigid->morientation.m[7] = net->forward.y;
		rigid->morientation.m[8] = net->forward.z;

		/*
		quaternion q;

		q.x = net->quat.x;
		q.y = net->quat.y;
		q.z = net->quat.z;
		q.compute_w();
		q.s = -q.s;

		rigid->morientation = q.to_matrix();
		*/
	}

	if (ent->projectile)
	{
		if (net->active)
			ent->projectile->active = true;
		else
			ent->projectile->active = false;
		ent->projectile->owner = net->owner;
		//		printf("Got trigger data index %d\n", index);
	}
	else
	{
		printf("deserialize_net_projectile() failed to find projectile at index %d\n", index);
		return -1;
	}

	return 0;
}

void Netcode::client_rename()
{
	sprintf(client_reliable.msg, "<player \"%s\"/>", engine->menu.data.name);
	client_reliable.size = (unsigned short)(2 * sizeof(unsigned short int) + strlen(client_reliable.msg) + 1);
	client_reliable.sequence = sequence;
}

void Netcode::server_rename(char *oldname, char *newname, int self)
{
	char msg[128];

	sprintf(msg, "say \"%s renamed to %s\"", oldname, newname);
	chat("Server", msg);
	snprintf(engine->entity_list[self]->player->name, 127, "%s", newname);
}

void Netcode::disconnect()
{
	static clientmsg_t	clientmsg;

	client_flag = false;

	memset(&clientmsg, 0, sizeof(clientmsg_t));

	clientmsg.sequence = sequence;
	clientmsg.server_sequence = 0;
	clientmsg.num_cmds = 0;
	clientmsg.qport = qport;
	strcpy(client_reliable.msg, "<disconnect/>");
	client_reliable.size = (unsigned short)(2 * sizeof(unsigned short int) + strlen(client_reliable.msg) + 1);
	client_reliable.sequence = sequence;
	memcpy(&clientmsg.data[clientmsg.num_cmds * sizeof(int)], &client_reliable, client_reliable.size);
	clientmsg.length = CLIENT_HEADER + clientmsg.num_cmds * sizeof(int) + client_reliable.size;
	debugf("disconnecting\n");

	net_send((char *)&clientmsg, clientmsg.length);
}

void Netcode::kick(unsigned int i)
{
	if (server_flag == false)
		return;

	if (i >= client_list.size())
	{
		debugf("kick() invalid index %d", i);
		return;
	}

	static servermsg_t servermsg = { 0 };

	servermsg.sequence = sequence + 1;
	servermsg.client_sequence = reliable[i].sequence;
	servermsg.num_ents = 0;

	sprintf(reliable[i].msg, "<disconnect/>");
	reliable[i].size = (unsigned short)(2 * sizeof(unsigned short int) + strlen(reliable[i].msg) + 1);
	reliable[i].sequence = sequence;
	servermsg.length = SERVER_HEADER + reliable[i].size;
	memcpy(servermsg.data, &reliable, reliable[i].size);

	net_sendto((char *)&servermsg, servermsg.length, client_list[i]);
	debugf("sent disconnect to client %d [%s]\n", i, client_list[i]->socketname);
	delete client_list[i];
	client_list.erase(client_list.begin() + i);
}

int Netcode::bind(int port)
{
	if (server_flag)
	{
		debugf("Server already bound to port\n");
		return -1;
	}

#ifdef SERIAL
	if (init == false)
	{
		serial_init(engine->server_comport, &handle);
		init = true;
	}
#endif

#ifdef WIN32
	if (!SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS))
	{
		DWORD dwError = GetLastError();
		printf("Failed to set process priority to high Error #%d)\n", dwError);
	}
#endif

#ifdef VOICE
	engine->voice.bind(NULL, 65530);
#endif
	if (sock.bind(NULL, port) == 0)
	{
		client_flag = false;
		server_flag = true;
		return 0;
	}
	else
	{
		engine->q3map.unload(engine->gfx);
		return -1;
	}
}

void Netcode::connect(char *serverip)
{
	static clientmsg_t	clientmsg;
	static servermsg_t servermsg;

	client_flag = false;

#ifdef SERIAL
	if (init == false)
	{
		serial_init(engine->client_comport, &handle);
		init = true;
	}
#endif

	memset(&clientmsg, 0, sizeof(clientmsg_t));

	clientmsg.sequence = sequence;
	clientmsg.server_sequence = 0;
	clientmsg.num_cmds = 0;
	clientmsg.qport = qport;
	strcpy(client_reliable.msg, "<connect/>");
	client_reliable.size = (unsigned short)(2 * sizeof(unsigned short int) + strlen(client_reliable.msg) + 1);
	client_reliable.sequence = sequence;
	memcpy(&clientmsg.data[clientmsg.num_cmds * sizeof(int)], &client_reliable, client_reliable.size);
	clientmsg.length = CLIENT_HEADER + clientmsg.num_cmds * sizeof(int) + client_reliable.size;


#ifdef VOICE
	engine->voice.bind(NULL, 65530);
#endif
	sock.connect(serverip, net_port);
	debugf("Sending connect request\n");
	net_send((char *)&clientmsg, clientmsg.length);
	debugf("Waiting for server info\n");


	int num_read = net_recv((char *)&servermsg, 8192, 5);
	if (num_read > 0)
	{
		char level[LINE_SIZE];

#ifdef SERIAL
		static unsigned char buff[4096];
		dequeue(&recv_queue, buff, num_read);
#endif

		client_flag = true;
		server_flag = false;
		debugf("Connected\n");
#ifdef VOICE
		sprintf(engine->voice.server, "%s:65530", serverip);
#endif
        reliablemsg_t *reliablemsg = (reliablemsg_t *)&servermsg.data[0];
		if (sscanf(reliablemsg->msg, "<map>%s</map>", level) == 1)
		{
			char *end = strstr(level, "</map>");
			*end = '\0';
			debugf("Loading %s\n", level);
			engine->load((char *)level);
			client_rename();
			last_server_sequence = servermsg.sequence;
		}
		else if (strstr(reliablemsg->msg, "<serverfull/>") != 0)
		{
			debugf("Server is full\n");
		}
		else
		{
			debugf("Invalid response\n");
		}
	}
	else
	{
		debugf("Connection timed out\n");
	}
}

void Netcode::chat(char *name, char *msg)
{
	char data[1024];

	if (name == NULL)
	{
		int index = engine->find_type(ENT_PLAYER, 0);
		if (index != -1)
		{
			//chatmode chat
			sprintf(data, "%s: %s", engine->entity_list[index]->player->name, msg);
		}
	}
	else
	{
		//console chat command
		// skip past 'say "'
		char *pmsg = msg + 5;
		// remove ending "
		pmsg[strlen(pmsg) - 1] = '\0';
		sprintf(data, "%s: %s", name, pmsg);
	}

	if (client_flag == false)
	{
		for (unsigned int i = 0; i < engine->max_player; i++)
		{
			sprintf(msg, "<chat>%s</chat>", data);
			strcat(reliable[i].msg, msg);
			reliable[i].size = (unsigned short)(2 * sizeof(unsigned short int) + strlen(reliable[i].msg) + 1);
			reliable[i].sequence = sequence;
		}
	}
	else
	{
		sprintf(msg, "<chat>%s</chat>", data);
		strcat(client_reliable.msg, msg);
		client_reliable.size = (unsigned short)(2 * sizeof(unsigned short int) + strlen(client_reliable.msg) + 1);
		client_reliable.sequence = sequence;
	}

	// Client will get message back from server
	if (client_flag == false)
	{
		engine->menu.print_chat(data);
		engine->game->set_chat_timer(3 * TICK_RATE);

		int self = engine->find_type(ENT_PLAYER, 0);
		if (self != -1)
		{
			engine->play_wave_global(SND_TALK);
		}
	}
}

void Netcode::report_master()
{
	report_t report;

	if (server_flag == false)
		return;

	report.cmd = MASTER_REPORT;
	report.qport = qport;
	sprintf(report.sv_hostname, "%s", sv_hostname);
	sprintf(report.map, "%s", engine->q3map.map_name);
	report.num_player = 1;
	report.max_player = 16;
	report.fraglimit = engine->game->get_fraglimit();
	report.timelimit = engine->game->get_timelimit();
	report.capturelimit = engine->game->get_capturelimit();
	for (unsigned int i = 0; i < num_master; i++)
	{
		int ret = sock.sendto((char *)&report, sizeof(report_t), master_list[i]);
		if (ret != sizeof(report_t))
		{
			debugf("Failed to send master report to %s\n", master_list[i]);
		}
		else
		{
			debugf("sent master report to %s\n", master_list[i]);
		}
	}
}

void Netcode::query_master()
{
	unsigned int msg = MASTER_LIST;
	char response[2048];
	char from[1024];
	report_t *report;

	sprintf(from, "127.0.0.1:65535");
	for (unsigned int i = 0; i < num_master; i++)
	{
		sock.sendto((char *)&msg, sizeof(int), master_list[i]);
		debugf("Sending request to master server %s\n", master_list[i]);
	}
#ifdef WIN32
	Sleep(500);
#endif
	int num_read = net_recvfrom(response, 512 * sizeof(report_t), from, 1023);

	report = (report_t *)response;
	int num_report = num_read / sizeof(report_t);
	if (num_read % sizeof(report_t) == 0)
	{
		for (int i = 0; i < num_report; i++)
		{
			if (report[i].cmd != MASTER_RESPONSE)
			{
				debugf("Invalid response\n");
				break;
			}

			debugf("IP: %s Hostname: %s Map: %s Players: %d/%d Gametype: %d Fraglimit: %d Timelimit: %d Capturelimit: %d\n\n",
				report[i].ip,
				report[i].sv_hostname,
				report[i].map,
				report[i].num_player,
				report[i].max_player,
				report[i].gametype,
				report[i].fraglimit,
				report[i].timelimit,
				report[i].capturelimit
			);
		}
	}


}
