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

#ifndef SOCK_H
#define SOCK_H

#ifndef WIN32
#define SOCKET int
#endif

#ifdef WIN32
int inet_pton(int af, const char *server, void *vaddr);
#endif

/*
	UDP non blocking
*/
class Socket
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
