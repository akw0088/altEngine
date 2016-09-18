#include "net.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void Net::bind(char *address, int port)
{
	int sndbuf;
	int rcvbuf;
	socklen_t arglen = sizeof(int);

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1)
	{
		throw("socket error");
	}

	getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char *)&sndbuf, &arglen);
	printf("SO_SNDBUF = %d\n", sndbuf);

	getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char *)&rcvbuf, &arglen);
	printf("SO_RCVBUF = %d\n", rcvbuf);

	sndbuf = 8192; //default 8192
	setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char *)&sndbuf, sizeof(sndbuf));
	printf("Setting SO_SNDBUF to %d\n", sndbuf);

	rcvbuf = 8192; //default 8192
	setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char *)&rcvbuf, sizeof(rcvbuf));
	printf("Setting SO_RCVBUF to %d\n", rcvbuf);

	getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char *)&sndbuf, &arglen);
	printf("SO_SNDBUF = %d\n", sndbuf);

	getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char *)&rcvbuf, &arglen);
	printf("SO_RCVBUF = %d\n", rcvbuf);

#ifdef _WINDOWS_
	unsigned long nonblock = 1;
	ioctlsocket(sockfd, FIONBIO, &nonblock);
#else
	fcntl(sockfd, F_SETFL, O_NONBLOCK);
#endif

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_port        = htons(port);
	if (address != NULL)
		servaddr.sin_addr.s_addr = inet_addr(address);
	else
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);


	if ( (::bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) == -1 )
	{
		throw("bind error");
	}
	printf("bound on: %s:%d\n", inet_ntoa(servaddr.sin_addr), htons(servaddr.sin_port));
}

int Net::send(char *buffer, int size)
{
	return ::send(sockfd, buffer, size, 0);
}

int Net::recv(char *buff, int size)
{
	return ::recv(sockfd, buff, size, 0);
}

int Net::recv(char *buff, int size, int delay)
{
	int ret;
	struct timeval	timeout = { 0, 0 };
	fd_set		readfds;

	timeout.tv_sec = delay;

	FD_ZERO(&readfds);
	FD_SET(sockfd, &readfds);
	ret = select(sockfd + 1, &readfds, NULL, NULL, &timeout);

	if (ret == -1 || ret == 0)
		return 0;

	if ( FD_ISSET(sockfd, &readfds) )
		return ::recv(sockfd, buff, size, 0);
	else
		return 0;
}

int Net::recvfrom(char *buff, int size, char *from, int length)
{
	sockaddr_in		addr;
	socklen_t		sock_length = sizeof(sockaddr_in);
	int ret;
	int n = 0;
	struct timeval	timeout = { 0, 0 };
	fd_set		readfds;

	FD_ZERO(&readfds);
	FD_SET(sockfd, &readfds);
	ret = select(sockfd + 1, &readfds, NULL, NULL, &timeout);
	
	if (ret == -1 || ret == 0)
		return 0;

	if ( FD_ISSET(sockfd, &readfds) )
	{
		n = ::recvfrom(sockfd, buff, size, 0, (sockaddr *)&addr, &sock_length);

		snprintf(from, length, "%s:%d", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
		return n;
	}
	return 0;
}

int Net::sendto(char *buff, int size, char *to)
{
	sockaddr_in		addr;
	socklen_t		sock_length = sizeof(sockaddr_in);

	strtoaddr(to, addr);
	return ::sendto(sockfd, buff, size, 0, (sockaddr *)&addr, sock_length); 
}

void Net::strtoaddr(char *str, sockaddr_in &addr)
{
	char ip[80];
	int a,b,c,d;
	int port;

	memset(&addr, 0, sizeof(sockaddr_in));
	addr.sin_family	= AF_INET;
	if (sscanf(str, "%d.%d.%d.%d:%d", &a, &b, &c, &d, &port) == 5)
	{
		sprintf(ip, "%d.%d.%d.%d", a,b,c,d);
		addr.sin_addr.s_addr = inet_addr(ip);
	}
	else if (sscanf(str, "%s:%d", ip, &port) == 2)
	{
		addr.sin_addr.s_addr = inet_addr(ip);
	}
	else
	{
		char err[LINE_SIZE];

		snprintf(err, LINE_SIZE, "strtoaddr() invalid address %s", str);
		throw err;
	}
	addr.sin_port = htons(port);
}

void Net::connect(char *server, int port)
{
	int sndbuf;
	int rcvbuf;
	int ret = 0;
	socklen_t arglen = sizeof(int);

	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		throw("socket error");
	}

	getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char *)&sndbuf, &arglen);
	printf("SO_SNDBUF = %d\n", sndbuf);

	getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char *)&rcvbuf, &arglen);
	printf("SO_RCVBUF = %d\n", rcvbuf);

	sndbuf = 8192; //default 8192
	setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char *)&sndbuf, sizeof(sndbuf));
	printf("Setting SO_SNDBUF to %d\n", sndbuf);

	rcvbuf = 8192; //default 8192
	setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char *)&rcvbuf, sizeof(rcvbuf));
	printf("Setting SO_RCVBUF to %d\n", rcvbuf);

	getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char *)&sndbuf, &arglen);
	printf("SO_SNDBUF = %d\n", sndbuf);

	getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char *)&rcvbuf, &arglen);
	printf("SO_RCVBUF = %d\n", rcvbuf);

#ifdef _WINDOWS_
	unsigned long nonblock = 1;
	ioctlsocket(sockfd, FIONBIO, &nonblock);
#else
	fcntl(sockfd, F_SETFL, O_NONBLOCK);
#endif

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port   = htons(port);
	servaddr.sin_addr.s_addr = inet_addr(server);
#ifdef WIN32
	ret = inet_pton(AF_INET, server, &servaddr.sin_addr); //linux giving me linker errors?
#endif
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


#ifdef _WIN32
int inet_pton(int af, const char *server, in_addr *addr)
{
	struct hostent *host = gethostbyname(server);
	if (host)
	*addr = *((struct in_addr *)*host->h_addr_list);
	return 1;
}
#endif
