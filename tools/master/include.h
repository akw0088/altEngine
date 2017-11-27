#ifndef INCLUDE_H
#define INCLUDE_H
#define _CRT_SECURE_NO_DEPRECATE
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <errno.h>

#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#endif

#ifdef _WIN32
#define M_PI 3.141592653589793238462643
#define _CRTDBG_MAP_ALLOC
#define _USE_MATH_DEFINES
#define WIN32_EXTRA_LEAN
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#define VC_EXTRALEAN
#include <windows.h>
#include <winsock2.h>
//#include <Ws2tcpip.h>
#include <omp.h>
#include <mmsystem.h>


#pragma comment(lib, "wsock32.lib")

#include "net.h"

typedef int socklen_t;
typedef unsigned char byte;

#endif

#define MAX_CELLS 16
#define MAX_ARC 32768
#define MASTER_PORT 6534

#define MAX(x,y) (x) > (y) ? (x) : (y)
#define MIN(x,y) (x) < (y) ? (x) : (y)


typedef struct
{
	char ip[32];
	unsigned short port;
	int last_time;
} ref_t;


typedef struct
{
	int num_server;
	ref_t server[32];
} dbh_t;

#pragma pack(1)
typedef struct
{
	unsigned short int	length;
	unsigned short int	compressed_size;
	unsigned short int	sequence;
	unsigned short int	client_sequence;
	unsigned short int	num_ents;	// not really neeeded with size, but worth the two bytes info wise
	unsigned int		data_size;
	unsigned char		data[16384];
} servermsg_t;
#pragma pack(8)

typedef struct
{
	unsigned short int	length;
	unsigned short int	qport;
	unsigned short int	sequence;
	unsigned short int	server_sequence;
	float			up[3];
	float			forward[3];
	float			pos[3];		// Sending position (not used, but interesting to calculate delta's on server)
	char			num_cmds;
	unsigned char		data[16834];
} clientmsg_t;

typedef struct
{
	unsigned short int	sequence;
	unsigned short int	size; // size of entire header + data
	char			msg[2048];
} reliablemsg_t;

#define CLIENT_HEADER 45
#define SERVER_HEADER 14


#endif