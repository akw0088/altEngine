#include "net.h"

void Net::listen(char *address, int port)
{
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd == -1)
	{
		throw("socket error");
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_port        = htons(port);
	if (address == NULL)
		servaddr.sin_addr.s_addr = inet_addr(address);
	else
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);


	if ( (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) == -1 )
	{
		throw("bind error");
	}
	printf("listening on: %s:%d\n", inet_ntoa(servaddr.sin_addr), htons(servaddr.sin_port));

	if ( ::listen(listenfd, 3) == -1 )
	{
		throw("listen error");
	}
}

void Net::accept()
{
	socklen_t size = sizeof(struct sockaddr_in);

	sockfd = ::accept(listenfd, (struct sockaddr *)&client, &size);
	if (sockfd == INVALID_SOCKET)
		return;

	printf("Accept: %s\n", inet_ntoa(client.sin_addr));
}

int Net::send(char *buffer, int size)
{
	int n = 0;
	int nSent = 0;

	do
	{
		n = ::send(sockfd, buffer + nSent, size - nSent, 0);
		nSent += n;
		printf("Sent %d of %d bytes\r", nSent, size);
	} while ( n > 0);

	if (n < 0)
	{
		throw("send error");
	}

	return nSent;
}

int Net::recv(char *buff, int size)
{
	int n = 0;
	int nRecv = 0;

	do
	{
		n = ::recv(sockfd, buff + nRecv, size - nRecv, 0);
		nRecv += n;
		printf("Received %d of %d bytes\t", nRecv, size);
	} while ( n > 0);

	if (n < 0)
	{
		throw("read error");
	}

	return nRecv;
}

void Net::drop()
{
	shutdown(sockfd, 1);
	closesocket(sockfd);
}

void Net::unbind()
{
	time_t timeout = time(NULL);

	printf("\nShutting down...\n");
	shutdown(listenfd, 1);
	while ( ::recv(listenfd, buff, MAXLINE, 0) != SOCKET_ERROR || time(NULL) > timeout + 5 );
	closesocket(listenfd);
}

void Net::connect(char *server, int port)
{
	int sndbuf;
	int rcvbuf;
	int ret;
	socklen_t arglen = sizeof(int);

	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		throw("socket error");
	}


	getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char *)&sndbuf, &arglen);
	printf("SO_SNDBUF = %d\n", sndbuf);

	getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char *)&rcvbuf, &arglen);
	printf("SO_RCVBUF = %d\n", sndbuf);

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port   = htons(port);
	servaddr.sin_addr.s_addr = inet_addr(server);
	ret = inet_pton(AF_INET, server, &servaddr.sin_addr);
	if ( ret == 0)
	{
		throw("inet_pton invalid server.");
	}
	else if (ret == -1)
	{
		throw("inet_pton error.");
	}

	if (::connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1)
	{
		throw("connect error");
	}

}

void Net::disconnect()
{
	int n;

	shutdown(sockfd, 1);
	while ( (n = ::recv(sockfd, buff, MAXLINE, 0) ) > 0);

	if (n < 0)
	{
		throw("read error");
	}

	closesocket(sockfd);
}

#ifdef _WIN32
int inet_pton(int af, const char *src, void *dst)
{
	return 1;
}
#endif
