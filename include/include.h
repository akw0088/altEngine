#ifndef INCLUDE_H
#define INCLUDE_H

#ifdef _WIN32
	#define _USE_MATH_DEFINES
	#define WIN32_EXTRA_LEAN
	#define _CRT_SECURE_NO_DEPRECATE
	#define NOMINMAX
	#include <windows.h>
	#define snprintf sprintf_s

#ifdef DIRECTX
	#include <d3d9.h>
	#include <d3dx9.h>
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
	#include <sys/select.h>
	#include <sys/types.h>
	#include <sys/time.h>
#endif

//audio
#include <AL/al.h>
#include <AL/alc.h>

//std
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

using namespace std;

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
#endif
