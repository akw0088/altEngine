#include "include.h"


void handle_report(dbh_t *header, char *buffer, char *client_ip, report_t *report)
{
	//unsigned int qport = 0;

	if (header->num_server < 32)
	{
		int found = 0;
		for (int i = 0; i < header->num_server; i++)
		{
			if (strcmp(client_ip, header->server[i].ip) == 0)
			{
				int index = i;

				printf("server already exists, updating info\n");
				printf("\t%s\n", report->sv_hostname);
				printf("\t%s\n", report->map);
				strncpy(header->server[index].ip, client_ip, 31);
				header->server[index].last_time = (int)time(0);
				sprintf(header->server[index].sv_hostname, "%s", report->sv_hostname);
				sprintf(header->server[index].map, "%s", report->map);
				header->server[index].capturelimit = report->capturelimit;
				header->server[index].fraglimit = report->fraglimit;
				header->server[index].gametype = report->gametype;
				header->server[index].gametype = report->timelimit;
				header->server[index].num_player = report->num_player;
				header->server[index].max_player = report->max_player;
				header->server[index].qport = report->qport;
				found = 1;
				break;
			}
		}

		if (found == 0)
		{
			int index = header->num_server;
			strncpy(header->server[index].ip, client_ip, 31);
			header->server[index].last_time = (int)time(0);
			sprintf(header->server[index].sv_hostname, "%s", report->sv_hostname);
			sprintf(header->server[index].map, "%s", report->map);
			header->server[index].capturelimit = report->capturelimit;
			header->server[index].fraglimit = report->fraglimit;
			header->server[index].gametype = report->gametype;
			header->server[index].gametype = report->timelimit;
			header->server[index].num_player = report->num_player;
			header->server[index].max_player = report->max_player;
			header->server[index].qport = report->qport;
			header->num_server++;
			
			printf("adding server %s to master\n", client_ip);
			printf("\t%s\n", report->sv_hostname);
			printf("\t%s\n", report->map);
		}
	}
	else
	{
		printf("master server is full\n");
	}
}

void handle_list(dbh_t *header, char *client_ip, sockaddr *client, int client_size)
{
	unsigned int client_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	report_t report[512];

	memset(report, 0, sizeof(report));
	printf("Got master list request from client %s\n", client_ip);
	//struct sockaddr_in *info = (sockaddr_in *)&client;
	for (int i = 0; i < header->num_server; i++)
	{
		if (time(0) - header->server[i].last_time > 15 * 60)
		{
			printf("Removing host %s at index %d (not updated in 15 minutes)\n", header->server[i].ip, i);
			// should probably just use a linked list
			memmove(&header->server[i], &header->server[i+1], (header->num_server - i + 1) * sizeof(report_t));
			header->num_server--;
			i--;
			continue;
		}

		sprintf(report[i].ip, "%s", header->server[i].ip);
		sprintf(report[i].sv_hostname, "%s", header->server[i].sv_hostname);
		sprintf(report[i].map, "%s", header->server[i].map);
		report[i].capturelimit = header->server[i].capturelimit;
		report[i].timelimit = header->server[i].timelimit;
		report[i].fraglimit = header->server[i].fraglimit;
		report[i].gametype = header->server[i].gametype;
		report[i].num_player = header->server[i].num_player;
		report[i].max_player = header->server[i].max_player;
		report[i].qport = header->server[i].qport;
		report[i].cmd = MASTER_RESPONSE;
	}
	printf("Sending serverlist (%d servers)\n", header->num_server);
	sendto(client_sock, (char *)report, header->num_server * sizeof(report_t), 0, client, client_size);
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
		char buffer[4096] = { 0 };
		sockaddr client;
		int client_size = sizeof(client);
		char client_ip[128];
		master_cmd_t *cmd;
		int num_read = 0;

		num_read = recvfrom(server_sock, buffer, 4096, 0, &client, (socklen_t *)&client_size);
		struct sockaddr_in *sa = (struct sockaddr_in*)&client;
		struct in_addr addr = sa->sin_addr;
		inet_ntop(AF_INET, &addr, client_ip, 127);

		printf("accepted connection from %s\n", client_ip);
		cmd = (master_cmd_t *)&buffer[0];

		switch (*cmd)
		{
		case MASTER_REPORT:
			if (num_read == sizeof(report_t))
			{
				handle_report(header, buffer, client_ip, (report_t *)buffer);
			}
			break;
		case MASTER_LIST:
			if (num_read == sizeof(int))
			{
				handle_list(header, client_ip, &client, client_size);
			}
			break;
		case MASTER_RESPONSE:
			break;
		}
		printf("connection complete\n");
	}
}
