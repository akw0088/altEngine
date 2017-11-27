#include "include.h"


void handle_master_request(dbh_t *header, char *buffer, char *client_ip)
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
				header->server[header->num_server].last_time = (int)time(0);
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

void handle_list_request(dbh_t *header, char *client_ip, sockaddr *client, int client_size)
{
	unsigned int client_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	char msg[2048];

	memset(msg, 0, sizeof(msg));
	printf("Got master list request from client %s\n", client_ip);
	//struct sockaddr_in *info = (sockaddr_in *)&client;
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
	sendto(client_sock, msg, strlen(msg) + 1, 0, client, client_size);
}

int main(int argc, char *argv[])
{
	sockaddr_in	servaddr;
	int	server_sock;
	dbh_t *header;

	if (argc < 2)
	{
		printf("Usage: master_listen <port>\n");
		return 0;
	}

#ifdef WIN32
	WSADATA	wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);
#endif


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
		sockaddr client;
		int client_size = sizeof(client);
		char client_ip[128];

		recvfrom(server_sock, buffer, 80, 0, &client, (socklen_t *)&client_size);
		struct sockaddr_in *sa = (struct sockaddr_in*)&client;
		struct in_addr addr = sa->sin_addr;
		inet_ntop(AF_INET, &addr, client_ip, 127);

		printf("accepted connection from %s\n", client_ip);
		printf("<- [%s]\n", buffer);

		if (strstr(buffer, "master"))
		{
			handle_master_request(header, buffer, client_ip);
		}
		else if (strstr(buffer, "list"))
		{
			handle_list_request(header, client_ip, &client, client_size);
		}
		printf("connection complete\n");
	}
}
