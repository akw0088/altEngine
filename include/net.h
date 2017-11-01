#include "include.h"

#ifndef NET_H
#define NET_H

#ifndef WIN32
#define SOCKET int
#endif

#ifdef WIN32
int inet_pton(int af, const char *server, void *vaddr);
#endif

/*
	UDP non blocking
*/
class Net
{
public:
	int connect(char *server, int port);
	int bind(char *address, int port);
	int send(char *buffer, int size);
	int sendto(char *buff, int size, char *to);
	int recv(char *buff, int size);
	int recv(char *buffer, int size, int delay);
	int recvfrom(char *buff, int size, char *from, int length);
	int strtoaddr(char *str, struct sockaddr_in &addr);
	void closesock();

	struct sockaddr_in	servaddr;
	int			sockfd;
private:


};

#endif
