#ifndef INCLUDE_H
#define INCLUDE_H

#define _HAS_EXCEPTIONS 0
#define D3D_DEBUG_INFO

#define D3D11


#define LINE_SIZE 512

//#define NORMALMAP

#define TICK_MS		8		// 125hz
#define TICK_RATE	125
//#define TICK_MS 16		// 62.5hz
//#define TICK_RATE 60
// quite jerky movement, makes me sick
//#define TICK_MS 33
//#define TICK_RATE 30



#define GRAVITY 9.8f

#define GRAVITY_SCALE 1.25f
#define JUMPPAD_SCALE 0.015f

#define RIGID_IMPACT 0.5f
#define IMPACT_VELOCITY 4.0f
#define FALL_DAMAGE_VELOCITY 7.0f

#define MAX_TEXTURES 4 // multi texture limits (quake 3 shader stages)

#define PARTICLES
// Could probably increase, but we are now at a point where
//particle count drops FPS linearly to near zero without hitting the cap
//#define MAX_PARTICLES 175000


#ifdef _WIN32
	#define APP_NAME "altEngine.exe"
	#define APP_HASH "fac891919c0d02efdcf76b8c300ec63f"
#else
	#define APP_NAME "altEngine"
	#define APP_HASH "fac891919c0d02efdcf76b8c300ec63f"
#endif


#ifdef _WIN32
	#define _CRTDBG_MAP_ALLOC
	#define _USE_MATH_DEFINES
	#define WIN32_EXTRA_LEAN
	#define _CRT_SECURE_NO_DEPRECATE
	#define NOMINMAX
	#include <windows.h>
	#include <winsock.h>
	#include <omp.h>

	#pragma comment(lib, "wsock32.lib")

	typedef int socklen_t;


//#define SHADOWMAPS

#ifdef DIRECTX
#ifdef D3D11
	#include <d3d11.h>
	#include <d3dx11.h>
	#include <D3Dcompiler.h>
//	#include <d3dx10.h>
#else
	#include <d3d9.h>
	#include <d3dx9.h>
	#include <d3dx9math.h>
#endif
#else
	#define GLEW_STATIC
	#include <GL/glew.h>
	#include <GL/wglew.h>
#endif

#endif

#ifdef __OBJC__

//	#define glGenVertexArrays glGenVertexArraysAPPLE
//	#define glBindVertexArray glBindVertexArrayAPPLE
//	#define glDeleteVertexArrays glDeleteVertexArraysAPPLE
	#define glFramebufferTexture glFramebufferTextureEXT
	#define glFramebufferTexture glFramebufferTextureEXT
#endif

#ifndef WIN32
	#include <errno.h>
	#ifdef __linux__
		#include <GL/gl.h>
		#include <GL/glx.h>
	#else
	   	#define GLX_GLXEXT_PROTOTYPES
		#ifdef __OBJC__
            #define __gl_h_
            #include <OpenGL/gl3.h>
            #include <OpenGL/glext.h>
            //#include <OpenGL/gl.h>
			//#include <OpenGL/glu.h>
		#else
			#include <GL/gl.h>
			//#include <GL/glu.h>
			#include <GL/glx.h>
		#endif
#endif

#ifndef __OBJC__
	//mac has xquartz and can run xwindow apps
	#include <X11/Xlib.h>
	#include <X11/Xutil.h>
	#include <X11/Xos.h>

	#include <X11/Xatom.h>
	#include <X11/keysym.h>
#endif
	#include <unistd.h>
	#include <fcntl.h>
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

//#define SHADOWVOL

#define MAX(x,y) (x) > (y) ? (x) : (y)
#define MIN(x,y) (x) < (y) ? (x) : (y)

#ifndef __OBJC__
//audio
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/efx.h>
#else
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#endif

#ifdef WIN32
	#include <al.h>
	#include <alc.h>
	#include <efx.h>
	#include <EFX-Util.h>
	#include <efx-creative.h>
#endif

//std
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <vector>
#include <float.h>
#ifdef WIN32
	#include <crtdbg.h>
#endif

#ifdef DIRECTX
	#include <DxErr.h>

#endif

#define G_QUAKE3

using namespace std;

#include "huffman.h"
#include "net.h"
#include "vector.h"
#include "matrix.h"
#include "quaternion.h"
#include "plane.h"
#include "types.h"
#include "md5_types.h"


#include "hashtable.h"
#include "graph.h"
#include "md5.h"
#include "graphics.h"
#include "bspTypes.h"
#include "audio.h"
#include "frame.h"

#include "edge.h"
#include "bsp.h"
#include "trigger.h"
#include "light.h"
#include "speaker.h"
#include "model.h"
#include "md5model.h"
#include "rigidbody.h"
#include "vehicle.h"
#include "player.h"
#include "decal.h"

#include "sin_table.h"
#include "common.h"
#include "entity.h"
#include "shader.h"
#include "parse.h"
#include "menu.h"
#include "basegame.h"
#include "quake3.h"
#include "commando.h"
#include "engine.h"


bool RayBoxSlab(vec3 &origin, vec3 &dir, vec3 &min, vec3 &max, float &distance);
int load_texture(Graphics &gfx, char *file_name, bool clamp, bool bgr);
int load_texture_pk3(Graphics &gfx, char *file_name, char **pk3_list, int num_pk3, bool clamp, bool bgr);

#define DEBUG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)


byte *tga_24to32(int width, int height, byte *pBits, bool bgr);
float abs32(float val);
int abs32(int val);


double fsin(double rad);
double fcos(double rad);

int debugf(const char *format, ...);

#define MY_PI 3.14159265359f
#define MY_HALF_PI 1.5707963268f
#define MAXLINE 4096


//quake3 game units, 8 units = 1 foot, ~3.3ft per meter (each unit is 8 values large)
#define UNITS_TO_METERS (8.0f * 8.0f * 3.3f)



#endif
