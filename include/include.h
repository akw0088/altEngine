#ifndef INCLUDE_H
#define INCLUDE_H

#ifdef _WIN32
	#define _USE_MATH_DEFINES
	#define WIN32_EXTRA_LEAN
	#define _CRT_SECURE_NO_DEPRECATE
	#define NOMINMAX
	#include <windows.h>
	#include <winsock.h>

	#pragma comment(lib, "wsock32.lib")

	#define snprintf sprintf_s

	typedef int socklen_t;
	int inet_pton(int af, const char *src, void *dst);

#ifdef DIRECTX
	#include <d3d9.h>
	#include <d3dx9.h>
	#include <d3dx9math.h>
#else
	#include <GL/glew.h>
#endif

#else
	#include <GL/gl.h>
	#include <GL/glu.h>
	#include <GL/glx.h>
	#include <X11/Xlib.h>
	#include <X11/Xatom.h>
	#include <X11/keysym.h>
	#include <unistd.h>
	#include <sys/select.h>
	#include <sys/types.h>
	#include <sys/time.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>

	#define closesocket close

	typedef	int SOCKET;
	#define SOCKET_ERROR	-1
	#define INVALID_SOCKET	-1

#endif

//audio
#include <AL/al.h>
#include <AL/alc.h>

//std
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

using namespace std;

#include "list.h"
#include "sin_table.h"
#include "vector.h"
#include "matrix.h"
#include "quaternion.h"
#include "plane.h"
#include "types.h"

#include "keyboard.h"
#include "graphics.h"
#include "bspTypes.h"
#include "entity.h"
#include "parse.h"
#include "frame.h"
#include "bsp.h"
#include "sound.h"
#include "engine.h"

byte *gltLoadTGA(const char *file, int *iWidth, int *iHeight, int *iComponents, int *eFormat);
char *getFile(char *fileName);

#define MAXLINE 4096

//quake3 game units, 8 units = 1 foot, ~3.3ft per meter (each unit is 8 values large)
#define UNITS_TO_METERS (8.0f * 8.0f * 3.3f)

#endif
