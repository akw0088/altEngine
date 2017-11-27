#include "include.h"


char *get_file(char *filename, int *size)
{
	FILE	*file;
	char	*buffer;
	int	file_size, bytes_read;

	file = fopen(filename, "rb");
	if (file == NULL)
		return 0;
	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	fseek(file, 0, SEEK_SET);
	buffer = new char[file_size + 1];
	bytes_read = (int)fread(buffer, sizeof(char), file_size, file);
	if (bytes_read != file_size)
	{
		delete[] buffer;
		fclose(file);
		return 0;
	}
	fclose(file);
	buffer[file_size] = '\0';

	if (size != NULL)
	{
		*size = file_size;
	}

	return buffer;
}

void query(char *serverip, unsigned int net_port)
{
	Net net;
	static clientmsg_t	clientmsg;
	static servermsg_t servermsg;
	reliablemsg_t	client_reliable;



	memset(&clientmsg, 0, sizeof(clientmsg_t));

	clientmsg.sequence = 0;
	clientmsg.server_sequence = 0;
	clientmsg.num_cmds = 0;
	clientmsg.qport = 1234;
	strcpy(client_reliable.msg, "<connect/>");
	client_reliable.size = (unsigned short)(2 * sizeof(unsigned short int) + strlen(client_reliable.msg) + 1);
	client_reliable.sequence = 0;
	memcpy(&clientmsg.data[clientmsg.num_cmds * sizeof(int)], &client_reliable, client_reliable.size);
	clientmsg.length = CLIENT_HEADER + clientmsg.num_cmds * sizeof(int) + client_reliable.size;

	net.connect(serverip, net_port);
	printf("Sending map request\n");
	net.send((char *)&clientmsg, clientmsg.length);
	printf("Waiting for server info\n");

	if (net.recv((char *)&servermsg, 8192, 5))
	{
		char level[128];

		printf("Connected\n");
		reliablemsg_t *reliablemsg = (reliablemsg_t *)&servermsg.data[0];
		if (sscanf(reliablemsg->msg, "<map>%s</map>", level) == 1)
		{
			char *end = strstr(level, "</map>");
			*end = '\0';
			printf("Loading %s\n", level);
			//			load((char *)level);
			strcpy(client_reliable.msg, "<spawn/>");
			client_reliable.size = (unsigned short)(2 * sizeof(unsigned short int) + strlen(client_reliable.msg) + 1);
			//			client_reliable.sequence = sequence;
			//			last_server_sequence = servermsg.sequence;
		}
		else
		{
			printf("Invalid response\n");
		}
	}
	else
	{
		printf("Connection timed out\n");
	}
}

int main(int argc, char *argv[])
{
	struct sockaddr_in	servaddr;
	int	server_sock;
	int	csock;	//client sock
	char *data = NULL;
	dbh_t *header;
	int size_db = 0;

	if (argc < 2)
	{
		printf("Usage: master_listen <port>\n");
		return 0;
	}

	WSADATA	wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);


	printf("Port %d\n", atoi(argv[1]));

	header = new dbh_t;
	memset(header, 0, sizeof(dbh_t));

	server_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	memset(&servaddr, 0, sizeof(struct sockaddr_in));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(atoi(argv[1]));

	bind(server_sock, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in));

	while (1)
	{
		char buffer[81] = { 0 };
		struct sockaddr client;
		int client_size = sizeof(client);
		char client_ip[128];




		while (1)
		{
			recvfrom(server_sock, buffer, 80, 0, &client, &client_size);
			struct sockaddr_in *sa = (struct sockaddr_in*)&client;
			struct in_addr addr = sa->sin_addr;
			inet_ntop(AF_INET, &addr, client_ip, 127);

			printf("accepted connection from %s\n", client_ip);
			printf("<- [%s]\n", buffer);

			if (strstr(buffer, "master"))
			{
				unsigned int qport = 0;
				printf("<- master\n");
				if (1 == sscanf(buffer, "master %d", &qport))
				{
					if (header->num_server < 32)
					{
						int found = 0;
						for (int i = 0; i < header->num_server; i++)
						{
							if (strcmp(client_ip, header->server[i].ip) == 0)
							{
								printf("server already exists\n");
								found = 1;
								break;
							}
						}

						if (found == 0)
						{
							strncpy(header->server[header->num_server].ip, client_ip, 31);
							header->server[header->num_server].last_time = time(0);
							header->num_server++;
							printf("adding server %s to master\n", client_ip);
						}
					}
					else
					{
						printf("master server is full\n");
					}
				}
			}

			if (strstr(buffer, "list"))
			{
				unsigned int client_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
				char msg[2048];

				memset(msg, 0, sizeof(msg));
				printf("Got master list request from client %s\n", client_ip);
				struct sockaddr_in *info = (sockaddr_in *)&client;
				for (int i = 0; i < header->num_server; i++)
				{
					char ip[80];


					if (header->server[i].last_time < time(0) + 3600)
					{
						sprintf(ip, "%s:%d ", header->server[i].ip, header->server[i].port);
						strcat(msg, ip);
					}
				}
				printf("Sending serverlist: %s\n", msg);
				sendto(client_sock, msg, strlen(msg) + 1, 0, &client, client_size);
			}


/*
			if (strcmp(buffer, "done") == 0)
			{
				sprintf(buffer, "ok");
				printf("-> %s\n", buffer);
				send(csock, buffer, strlen(buffer) + 1, 0);
				break;
			}
			usleep(1000000);
			*/
		}
		printf("connection complete\n");
	}

}
