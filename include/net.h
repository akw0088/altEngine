#include "include.h"

class Net
{
public:
	void connect(char *server, int port);
	void listen(char *address, int port);
	void accept();
	int send(char *buffer, int size);
	int recv(char *buffer, int size);
	void drop();
	void unbind();
	void disconnect();

private:
	SOCKET			listenfd, sockfd;
	struct sockaddr_in	servaddr, client;
	char			buff[MAXLINE + 1];
};