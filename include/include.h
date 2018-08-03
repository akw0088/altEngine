#ifndef INCLUDE_H
#define INCLUDE_H

#define _HAS_EXCEPTIONS 0
#define D3D_DEBUG_INFO

#define SHADOWVOL


#define TICK_MS		8		// 125hz
#define TICK_RATE	125
//#define TICK_MS 16		// 62.5hz
//#define TICK_RATE 60
// quite jerky movement, makes me sick
//#define TICK_MS 33
//#define TICK_RATE 30


#define GRAVITY 9.8f

#define GRAVITY_SCALE 1.4f
#define JUMPPAD_SCALE 0.015f

#define RIGID_IMPACT 0.5f
#define IMPACT_VELOCITY 4.0f
#define FALL_DAMAGE_VELOCITY 7.0f

#define MAX_RES 32


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
	#define WIN32_LEAN_AND_MEAN
	#define WIN32_EXTRA_LEAN
	#define VC_EXTRALEAN
	#include <windows.h>
	#include <winsock.h>
	#include <omp.h>
	#include <mmsystem.h>


	#pragma comment(lib, "wsock32.lib")

	typedef int socklen_t;
	typedef unsigned char byte;

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
#endif
#ifdef OPENGL
	//#define GLEW_STATIC
	#include <GL/glew.h>
	#include <GL/wglew.h>
#endif

#ifdef VULKAN
	#define VK_USE_PLATFORM_WIN32_KHR
	#include <vulkan/vulkan.h>
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
		#include <opus/opus.h>

		void fullscreen(Display *display, Window w);
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
	#include <dlfcn.h>
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


#define MAX(x,y) (x) > (y) ? (x) : (y)
#define MIN(x,y) (x) < (y) ? (x) : (y)
#define SWAP(x, y, T) {T temp = x; x = y; y = temp;}

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
	#include <opus.h>
#endif

//std
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <vector>
#include <functional>
#include <thread>
#include <float.h>
#ifdef WIN32
	#include <crtdbg.h>
#endif

#ifdef DIRECTX
	#include <DxErr.h>
#endif

#define G_QUAKE3
//#define OPENMP

using namespace std;

//void * operator new(size_t n, char *filename, UINT line) throw(std::bad_alloc);


#ifdef VULKAN
// Garbage for vulkan that I'll delete eventually

struct MemoryTypeInfo
{
	bool deviceLocal = false;
	bool hostVisible = false;
	bool hostCoherent = false;
	bool hostCached = false;
	bool lazilyAllocated = false;

	struct Heap
	{
		uint64_t size = 0;
		bool deviceLocal = false;
	};

	Heap heap;
	int index;
};

struct SwapchainFormatColorSpace
{
	VkFormat format;
	VkColorSpaceKHR colorSpace;
};


enum MemoryProperties
{
	MT_DeviceLocal = 1,
	MT_HostVisible = 2
};

template <typename T>
T RoundToNextMultiple(const T a, const T multiple)
{
	return ((a + multiple - 1) / multiple) * multiple;
}

const unsigned char BasicVertexShader[] = {
	0x3 , 0x2 , 0x23, 0x7 , 0x0 , 0x0 , 0x1 , 0x0 , 0x1 , 0x0 , 0x8 , 0x0 , 0x21,
	0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x11, 0x0 , 0x2 , 0x0 , 0x1 , 0x0 ,
	0x0 , 0x0 , 0x11, 0x0 , 0x2 , 0x0 , 0x20, 0x0 , 0x0 , 0x0 , 0xb , 0x0 , 0x6 ,
	0x0 , 0x1 , 0x0 , 0x0 , 0x0 , 0x47, 0x4c, 0x53, 0x4c, 0x2e, 0x73, 0x74, 0x64,
	0x2e, 0x34, 0x35, 0x30, 0x0 , 0x0 , 0x0 , 0x0 , 0xe , 0x0 , 0x3 , 0x0 , 0x0 ,
	0x0 , 0x0 , 0x0 , 0x1 , 0x0 , 0x0 , 0x0 , 0xf , 0x0 , 0x9 , 0x0 , 0x0 , 0x0 ,
	0x0 , 0x0 , 0x4 , 0x0 , 0x0 , 0x0 , 0x6d, 0x61, 0x69, 0x6e, 0x0 , 0x0 , 0x0 ,
	0x0 , 0xd , 0x0 , 0x0 , 0x0 , 0x12, 0x0 , 0x0 , 0x0 , 0x1d, 0x0 , 0x0 , 0x0 ,
	0x1f, 0x0 , 0x0 , 0x0 , 0x3 , 0x0 , 0x3 , 0x0 , 0x2 , 0x0 , 0x0 , 0x0 , 0x90,
	0x1 , 0x0 , 0x0 , 0x4 , 0x0 , 0x9 , 0x0 , 0x47, 0x4c, 0x5f, 0x41, 0x52, 0x42,
	0x5f, 0x73, 0x65, 0x70, 0x61, 0x72, 0x61, 0x74, 0x65, 0x5f, 0x73, 0x68, 0x61,
	0x64, 0x65, 0x72, 0x5f, 0x6f, 0x62, 0x6a, 0x65, 0x63, 0x74, 0x73, 0x0 , 0x0 ,
	0x4 , 0x0 , 0x9 , 0x0 , 0x47, 0x4c, 0x5f, 0x41, 0x52, 0x42, 0x5f, 0x73, 0x68,
	0x61, 0x64, 0x69, 0x6e, 0x67, 0x5f, 0x6c, 0x61, 0x6e, 0x67, 0x75, 0x61, 0x67,
	0x65, 0x5f, 0x34, 0x32, 0x30, 0x70, 0x61, 0x63, 0x6b, 0x0 , 0x5 , 0x0 , 0x4 ,
	0x0 , 0x4 , 0x0 , 0x0 , 0x0 , 0x6d, 0x61, 0x69, 0x6e, 0x0 , 0x0 , 0x0 , 0x0 ,
	0x5 , 0x0 , 0x6 , 0x0 , 0xb , 0x0 , 0x0 , 0x0 , 0x67, 0x6c, 0x5f, 0x50, 0x65,
	0x72, 0x56, 0x65, 0x72, 0x74, 0x65, 0x78, 0x0 , 0x0 , 0x0 , 0x0 , 0x6 , 0x0 ,
	0x6 , 0x0 , 0xb , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x67, 0x6c, 0x5f,
	0x50, 0x6f, 0x73, 0x69, 0x74, 0x69, 0x6f, 0x6e, 0x0 , 0x6 , 0x0 , 0x7 , 0x0 ,
	0xb , 0x0 , 0x0 , 0x0 , 0x1 , 0x0 , 0x0 , 0x0 , 0x67, 0x6c, 0x5f, 0x50, 0x6f,
	0x69, 0x6e, 0x74, 0x53, 0x69, 0x7a, 0x65, 0x0 , 0x0 , 0x0 , 0x0 , 0x6 , 0x0 ,
	0x7 , 0x0 , 0xb , 0x0 , 0x0 , 0x0 , 0x2 , 0x0 , 0x0 , 0x0 , 0x67, 0x6c, 0x5f,
	0x43, 0x6c, 0x69, 0x70, 0x44, 0x69, 0x73, 0x74, 0x61, 0x6e, 0x63, 0x65, 0x0 ,
	0x5 , 0x0 , 0x3 , 0x0 , 0xd , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x5 ,
	0x0 , 0x3 , 0x0 , 0x12, 0x0 , 0x0 , 0x0 , 0x70, 0x6f, 0x73, 0x0 , 0x5 , 0x0 ,
	0x4 , 0x0 , 0x1d, 0x0 , 0x0 , 0x0 , 0x6f, 0x75, 0x74, 0x55, 0x76, 0x0 , 0x0 ,
	0x0 , 0x5 , 0x0 , 0x4 , 0x0 , 0x1f, 0x0 , 0x0 , 0x0 , 0x69, 0x6e, 0x55, 0x76,
	0x0 , 0x0 , 0x0 , 0x0 , 0x48, 0x0 , 0x5 , 0x0 , 0xb , 0x0 , 0x0 , 0x0 , 0x0 ,
	0x0 , 0x0 , 0x0 , 0xb , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x48, 0x0 ,
	0x5 , 0x0 , 0xb , 0x0 , 0x0 , 0x0 , 0x1 , 0x0 , 0x0 , 0x0 , 0xb , 0x0 , 0x0 ,
	0x0 , 0x1 , 0x0 , 0x0 , 0x0 , 0x48, 0x0 , 0x5 , 0x0 , 0xb , 0x0 , 0x0 , 0x0 ,
	0x2 , 0x0 , 0x0 , 0x0 , 0xb , 0x0 , 0x0 , 0x0 , 0x3 , 0x0 , 0x0 , 0x0 , 0x47,
	0x0 , 0x3 , 0x0 , 0xb , 0x0 , 0x0 , 0x0 , 0x2 , 0x0 , 0x0 , 0x0 , 0x47, 0x0 ,
	0x4 , 0x0 , 0x12, 0x0 , 0x0 , 0x0 , 0x1e, 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
	0x0 , 0x47, 0x0 , 0x4 , 0x0 , 0x1d, 0x0 , 0x0 , 0x0 , 0x1e, 0x0 , 0x0 , 0x0 ,
	0x0 , 0x0 , 0x0 , 0x0 , 0x47, 0x0 , 0x4 , 0x0 , 0x1f, 0x0 , 0x0 , 0x0 , 0x1e,
	0x0 , 0x0 , 0x0 , 0x1 , 0x0 , 0x0 , 0x0 , 0x13, 0x0 , 0x2 , 0x0 , 0x2 , 0x0 ,
	0x0 , 0x0 , 0x21, 0x0 , 0x3 , 0x0 , 0x3 , 0x0 , 0x0 , 0x0 , 0x2 , 0x0 , 0x0 ,
	0x0 , 0x16, 0x0 , 0x3 , 0x0 , 0x6 , 0x0 , 0x0 , 0x0 , 0x20, 0x0 , 0x0 , 0x0 ,
	0x17, 0x0 , 0x4 , 0x0 , 0x7 , 0x0 , 0x0 , 0x0 , 0x6 , 0x0 , 0x0 , 0x0 , 0x4 ,
	0x0 , 0x0 , 0x0 , 0x15, 0x0 , 0x4 , 0x0 , 0x8 , 0x0 , 0x0 , 0x0 , 0x20, 0x0 ,
	0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x2b, 0x0 , 0x4 , 0x0 , 0x8 , 0x0 , 0x0 ,
	0x0 , 0x9 , 0x0 , 0x0 , 0x0 , 0x1 , 0x0 , 0x0 , 0x0 , 0x1c, 0x0 , 0x4 , 0x0 ,
	0xa , 0x0 , 0x0 , 0x0 , 0x6 , 0x0 , 0x0 , 0x0 , 0x9 , 0x0 , 0x0 , 0x0 , 0x1e,
	0x0 , 0x5 , 0x0 , 0xb , 0x0 , 0x0 , 0x0 , 0x7 , 0x0 , 0x0 , 0x0 , 0x6 , 0x0 ,
	0x0 , 0x0 , 0xa , 0x0 , 0x0 , 0x0 , 0x20, 0x0 , 0x4 , 0x0 , 0xc , 0x0 , 0x0 ,
	0x0 , 0x3 , 0x0 , 0x0 , 0x0 , 0xb , 0x0 , 0x0 , 0x0 , 0x3b, 0x0 , 0x4 , 0x0 ,
	0xc , 0x0 , 0x0 , 0x0 , 0xd , 0x0 , 0x0 , 0x0 , 0x3 , 0x0 , 0x0 , 0x0 , 0x15,
	0x0 , 0x4 , 0x0 , 0xe , 0x0 , 0x0 , 0x0 , 0x20, 0x0 , 0x0 , 0x0 , 0x1 , 0x0 ,
	0x0 , 0x0 , 0x2b, 0x0 , 0x4 , 0x0 , 0xe , 0x0 , 0x0 , 0x0 , 0xf , 0x0 , 0x0 ,
	0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x17, 0x0 , 0x4 , 0x0 , 0x10, 0x0 , 0x0 , 0x0 ,
	0x6 , 0x0 , 0x0 , 0x0 , 0x3 , 0x0 , 0x0 , 0x0 , 0x20, 0x0 , 0x4 , 0x0 , 0x11,
	0x0 , 0x0 , 0x0 , 0x1 , 0x0 , 0x0 , 0x0 , 0x10, 0x0 , 0x0 , 0x0 , 0x3b, 0x0 ,
	0x4 , 0x0 , 0x11, 0x0 , 0x0 , 0x0 , 0x12, 0x0 , 0x0 , 0x0 , 0x1 , 0x0 , 0x0 ,
	0x0 , 0x2b, 0x0 , 0x4 , 0x0 , 0x6 , 0x0 , 0x0 , 0x0 , 0x14, 0x0 , 0x0 , 0x0 ,
	0x0 , 0x0 , 0x80, 0x3f, 0x20, 0x0 , 0x4 , 0x0 , 0x19, 0x0 , 0x0 , 0x0 , 0x3 ,
	0x0 , 0x0 , 0x0 , 0x7 , 0x0 , 0x0 , 0x0 , 0x17, 0x0 , 0x4 , 0x0 , 0x1b, 0x0 ,
	0x0 , 0x0 , 0x6 , 0x0 , 0x0 , 0x0 , 0x2 , 0x0 , 0x0 , 0x0 , 0x20, 0x0 , 0x4 ,
	0x0 , 0x1c, 0x0 , 0x0 , 0x0 , 0x3 , 0x0 , 0x0 , 0x0 , 0x1b, 0x0 , 0x0 , 0x0 ,
	0x3b, 0x0 , 0x4 , 0x0 , 0x1c, 0x0 , 0x0 , 0x0 , 0x1d, 0x0 , 0x0 , 0x0 , 0x3 ,
	0x0 , 0x0 , 0x0 , 0x20, 0x0 , 0x4 , 0x0 , 0x1e, 0x0 , 0x0 , 0x0 , 0x1 , 0x0 ,
	0x0 , 0x0 , 0x1b, 0x0 , 0x0 , 0x0 , 0x3b, 0x0 , 0x4 , 0x0 , 0x1e, 0x0 , 0x0 ,
	0x0 , 0x1f, 0x0 , 0x0 , 0x0 , 0x1 , 0x0 , 0x0 , 0x0 , 0x36, 0x0 , 0x5 , 0x0 ,
	0x2 , 0x0 , 0x0 , 0x0 , 0x4 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x3 ,
	0x0 , 0x0 , 0x0 , 0xf8, 0x0 , 0x2 , 0x0 , 0x5 , 0x0 , 0x0 , 0x0 , 0x3d, 0x0 ,
	0x4 , 0x0 , 0x10, 0x0 , 0x0 , 0x0 , 0x13, 0x0 , 0x0 , 0x0 , 0x12, 0x0 , 0x0 ,
	0x0 , 0x51, 0x0 , 0x5 , 0x0 , 0x6 , 0x0 , 0x0 , 0x0 , 0x15, 0x0 , 0x0 , 0x0 ,
	0x13, 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x51, 0x0 , 0x5 , 0x0 , 0x6 ,
	0x0 , 0x0 , 0x0 , 0x16, 0x0 , 0x0 , 0x0 , 0x13, 0x0 , 0x0 , 0x0 , 0x1 , 0x0 ,
	0x0 , 0x0 , 0x51, 0x0 , 0x5 , 0x0 , 0x6 , 0x0 , 0x0 , 0x0 , 0x17, 0x0 , 0x0 ,
	0x0 , 0x13, 0x0 , 0x0 , 0x0 , 0x2 , 0x0 , 0x0 , 0x0 , 0x50, 0x0 , 0x7 , 0x0 ,
	0x7 , 0x0 , 0x0 , 0x0 , 0x18, 0x0 , 0x0 , 0x0 , 0x15, 0x0 , 0x0 , 0x0 , 0x16,
	0x0 , 0x0 , 0x0 , 0x17, 0x0 , 0x0 , 0x0 , 0x14, 0x0 , 0x0 , 0x0 , 0x41, 0x0 ,
	0x5 , 0x0 , 0x19, 0x0 , 0x0 , 0x0 , 0x1a, 0x0 , 0x0 , 0x0 , 0xd , 0x0 , 0x0 ,
	0x0 , 0xf , 0x0 , 0x0 , 0x0 , 0x3e, 0x0 , 0x3 , 0x0 , 0x1a, 0x0 , 0x0 , 0x0 ,
	0x18, 0x0 , 0x0 , 0x0 , 0x3d, 0x0 , 0x4 , 0x0 , 0x1b, 0x0 , 0x0 , 0x0 , 0x20,
	0x0 , 0x0 , 0x0 , 0x1f, 0x0 , 0x0 , 0x0 , 0x3e, 0x0 , 0x3 , 0x0 , 0x1d, 0x0 ,
	0x0 , 0x0 , 0x20, 0x0 , 0x0 , 0x0 , 0xfd, 0x0 , 0x1 , 0x0 , 0x38, 0x0 , 0x1 ,
	0x0 ,
};
const unsigned char TexturedFragmentShader[] = {
	0x3 , 0x2 , 0x23, 0x7 , 0x0 , 0x0 , 0x1 , 0x0 , 0x1 , 0x0 , 0x8 , 0x0 , 0x19,
	0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x11, 0x0 , 0x2 , 0x0 , 0x1 , 0x0 ,
	0x0 , 0x0 , 0xb , 0x0 , 0x6 , 0x0 , 0x1 , 0x0 , 0x0 , 0x0 , 0x47, 0x4c, 0x53,
	0x4c, 0x2e, 0x73, 0x74, 0x64, 0x2e, 0x34, 0x35, 0x30, 0x0 , 0x0 , 0x0 , 0x0 ,
	0xe , 0x0 , 0x3 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x1 , 0x0 , 0x0 , 0x0 , 0xf ,
	0x0 , 0x7 , 0x0 , 0x4 , 0x0 , 0x0 , 0x0 , 0x4 , 0x0 , 0x0 , 0x0 , 0x6d, 0x61,
	0x69, 0x6e, 0x0 , 0x0 , 0x0 , 0x0 , 0x9 , 0x0 , 0x0 , 0x0 , 0x16, 0x0 , 0x0 ,
	0x0 , 0x10, 0x0 , 0x3 , 0x0 , 0x4 , 0x0 , 0x0 , 0x0 , 0x7 , 0x0 , 0x0 , 0x0 ,
	0x3 , 0x0 , 0x3 , 0x0 , 0x2 , 0x0 , 0x0 , 0x0 , 0x90, 0x1 , 0x0 , 0x0 , 0x4 ,
	0x0 , 0x9 , 0x0 , 0x47, 0x4c, 0x5f, 0x41, 0x52, 0x42, 0x5f, 0x73, 0x65, 0x70,
	0x61, 0x72, 0x61, 0x74, 0x65, 0x5f, 0x73, 0x68, 0x61, 0x64, 0x65, 0x72, 0x5f,
	0x6f, 0x62, 0x6a, 0x65, 0x63, 0x74, 0x73, 0x0 , 0x0 , 0x4 , 0x0 , 0x9 , 0x0 ,
	0x47, 0x4c, 0x5f, 0x41, 0x52, 0x42, 0x5f, 0x73, 0x68, 0x61, 0x64, 0x69, 0x6e,
	0x67, 0x5f, 0x6c, 0x61, 0x6e, 0x67, 0x75, 0x61, 0x67, 0x65, 0x5f, 0x34, 0x32,
	0x30, 0x70, 0x61, 0x63, 0x6b, 0x0 , 0x5 , 0x0 , 0x4 , 0x0 , 0x4 , 0x0 , 0x0 ,
	0x0 , 0x6d, 0x61, 0x69, 0x6e, 0x0 , 0x0 , 0x0 , 0x0 , 0x5 , 0x0 , 0x5 , 0x0 ,
	0x9 , 0x0 , 0x0 , 0x0 , 0x6f, 0x75, 0x74, 0x70, 0x75, 0x74, 0x43, 0x6f, 0x6c,
	0x6f, 0x72, 0x0 , 0x5 , 0x0 , 0x6 , 0x0 , 0xc , 0x0 , 0x0 , 0x0 , 0x63, 0x6f,
	0x6c, 0x6f, 0x72, 0x54, 0x65, 0x78, 0x74, 0x75, 0x72, 0x65, 0x0 , 0x0 , 0x0 ,
	0x0 , 0x5 , 0x0 , 0x6 , 0x0 , 0x10, 0x0 , 0x0 , 0x0 , 0x63, 0x6f, 0x6c, 0x6f,
	0x72, 0x53, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x72, 0x0 , 0x0 , 0x0 , 0x0 , 0x5 ,
	0x0 , 0x3 , 0x0 , 0x16, 0x0 , 0x0 , 0x0 , 0x75, 0x76, 0x0 , 0x0 , 0x47, 0x0 ,
	0x4 , 0x0 , 0x9 , 0x0 , 0x0 , 0x0 , 0x1e, 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
	0x0 , 0x47, 0x0 , 0x4 , 0x0 , 0xc , 0x0 , 0x0 , 0x0 , 0x22, 0x0 , 0x0 , 0x0 ,
	0x0 , 0x0 , 0x0 , 0x0 , 0x47, 0x0 , 0x4 , 0x0 , 0xc , 0x0 , 0x0 , 0x0 , 0x21,
	0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x47, 0x0 , 0x4 , 0x0 , 0x10, 0x0 ,
	0x0 , 0x0 , 0x22, 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x47, 0x0 , 0x4 ,
	0x0 , 0x10, 0x0 , 0x0 , 0x0 , 0x21, 0x0 , 0x0 , 0x0 , 0x1 , 0x0 , 0x0 , 0x0 ,
	0x47, 0x0 , 0x4 , 0x0 , 0x16, 0x0 , 0x0 , 0x0 , 0x1e, 0x0 , 0x0 , 0x0 , 0x0 ,
	0x0 , 0x0 , 0x0 , 0x13, 0x0 , 0x2 , 0x0 , 0x2 , 0x0 , 0x0 , 0x0 , 0x21, 0x0 ,
	0x3 , 0x0 , 0x3 , 0x0 , 0x0 , 0x0 , 0x2 , 0x0 , 0x0 , 0x0 , 0x16, 0x0 , 0x3 ,
	0x0 , 0x6 , 0x0 , 0x0 , 0x0 , 0x20, 0x0 , 0x0 , 0x0 , 0x17, 0x0 , 0x4 , 0x0 ,
	0x7 , 0x0 , 0x0 , 0x0 , 0x6 , 0x0 , 0x0 , 0x0 , 0x4 , 0x0 , 0x0 , 0x0 , 0x20,
	0x0 , 0x4 , 0x0 , 0x8 , 0x0 , 0x0 , 0x0 , 0x3 , 0x0 , 0x0 , 0x0 , 0x7 , 0x0 ,
	0x0 , 0x0 , 0x3b, 0x0 , 0x4 , 0x0 , 0x8 , 0x0 , 0x0 , 0x0 , 0x9 , 0x0 , 0x0 ,
	0x0 , 0x3 , 0x0 , 0x0 , 0x0 , 0x19, 0x0 , 0x9 , 0x0 , 0xa , 0x0 , 0x0 , 0x0 ,
	0x6 , 0x0 , 0x0 , 0x0 , 0x1 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
	0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x1 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
	0x0 , 0x0 , 0x20, 0x0 , 0x4 , 0x0 , 0xb , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
	0x0 , 0xa , 0x0 , 0x0 , 0x0 , 0x3b, 0x0 , 0x4 , 0x0 , 0xb , 0x0 , 0x0 , 0x0 ,
	0xc , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x1a, 0x0 , 0x2 , 0x0 , 0xe ,
	0x0 , 0x0 , 0x0 , 0x20, 0x0 , 0x4 , 0x0 , 0xf , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 ,
	0x0 , 0x0 , 0xe , 0x0 , 0x0 , 0x0 , 0x3b, 0x0 , 0x4 , 0x0 , 0xf , 0x0 , 0x0 ,
	0x0 , 0x10, 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x1b, 0x0 , 0x3 , 0x0 ,
	0x12, 0x0 , 0x0 , 0x0 , 0xa , 0x0 , 0x0 , 0x0 , 0x17, 0x0 , 0x4 , 0x0 , 0x14,
	0x0 , 0x0 , 0x0 , 0x6 , 0x0 , 0x0 , 0x0 , 0x2 , 0x0 , 0x0 , 0x0 , 0x20, 0x0 ,
	0x4 , 0x0 , 0x15, 0x0 , 0x0 , 0x0 , 0x1 , 0x0 , 0x0 , 0x0 , 0x14, 0x0 , 0x0 ,
	0x0 , 0x3b, 0x0 , 0x4 , 0x0 , 0x15, 0x0 , 0x0 , 0x0 , 0x16, 0x0 , 0x0 , 0x0 ,
	0x1 , 0x0 , 0x0 , 0x0 , 0x36, 0x0 , 0x5 , 0x0 , 0x2 , 0x0 , 0x0 , 0x0 , 0x4 ,
	0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x0 , 0x3 , 0x0 , 0x0 , 0x0 , 0xf8, 0x0 ,
	0x2 , 0x0 , 0x5 , 0x0 , 0x0 , 0x0 , 0x3d, 0x0 , 0x4 , 0x0 , 0xa , 0x0 , 0x0 ,
	0x0 , 0xd , 0x0 , 0x0 , 0x0 , 0xc , 0x0 , 0x0 , 0x0 , 0x3d, 0x0 , 0x4 , 0x0 ,
	0xe , 0x0 , 0x0 , 0x0 , 0x11, 0x0 , 0x0 , 0x0 , 0x10, 0x0 , 0x0 , 0x0 , 0x56,
	0x0 , 0x5 , 0x0 , 0x12, 0x0 , 0x0 , 0x0 , 0x13, 0x0 , 0x0 , 0x0 , 0xd , 0x0 ,
	0x0 , 0x0 , 0x11, 0x0 , 0x0 , 0x0 , 0x3d, 0x0 , 0x4 , 0x0 , 0x14, 0x0 , 0x0 ,
	0x0 , 0x17, 0x0 , 0x0 , 0x0 , 0x16, 0x0 , 0x0 , 0x0 , 0x57, 0x0 , 0x5 , 0x0 ,
	0x7 , 0x0 , 0x0 , 0x0 , 0x18, 0x0 , 0x0 , 0x0 , 0x13, 0x0 , 0x0 , 0x0 , 0x17,
	0x0 , 0x0 , 0x0 , 0x3e, 0x0 , 0x3 , 0x0 , 0x9 , 0x0 , 0x0 , 0x0 , 0x18, 0x0 ,
	0x0 , 0x0 , 0xfd, 0x0 , 0x1 , 0x0 , 0x38, 0x0 , 0x1 , 0x0 ,
};
#endif

#include "huffman.h"
#include "sock.h"
#include "vector.h"
#include "matrix.h"
#include "quaternion.h"
#include "plane.h"
#include "types.h"
#include "md5_types.h"
#include "shadow_volume.h"
#include "raster.h"


#include "hashtable.h"
#include "graph.h"
#include "md5.h"
#include "graphics.h"
#include "bsp_types_q3.h"
#include "bsp_types_hl.h"
#include "bsp_types_q1.h"
#include "bsp_types_q2.h"

#include "audio.h"
#include "frame.h"

#include "edge.h"
#include "bsp_q3.h"
#include "bsp_hl.h"
#include "bsp_q1.h"
#include "bsp_q2.h"
#include "trigger.h"
#include "projectile.h"
#include "light.h"
#include "portal_camera.h"
#include "speaker.h"
#include "spline.h"
#include "model.h"
#include "md5model.h"
#include "rigidbody.h"
#include "player.h"
#include "constructable.h"

#include "sin_table.h"
#include "common.h"
#include "entity.h"
#include "shader.h"
#include "parse.h"
#include "menu.h"
#include "netcode.h"
#include "basegame.h"
#include "quake3.h"
#include "commando.h"
#include "terrain.h"
#include "isocube.h"
#include "isosphere.h"
#include "oculus.h"
#include "voice.h"
#include "engine.h"


bool RayBoxSlab(vec3 &origin, vec3 &dir, vec3 &min, vec3 &max, float &distance);
int load_texture(Graphics &gfx, char *file_name, bool clamp, bool bgr, int anisotropic);
int load_texture_pk3(Graphics &gfx, char *file_name, char **pk3_list, int num_pk3, bool clamp, bool bgr, int anisotropic);

#ifdef _WIN32
void GetScreenShot(HWND hwnd, unsigned int &luminance, bool luminance_only);
void get_resolution(int &width, int &height, int &bpp, int &refresh_rate);
void set_resolution(int width, int height, int bpp);
void register_raw_mouse(HWND hwnd);
void unregister_raw_mouse(HWND hwnd);
#endif


//void * operator new(size_t n, char *filename, UINT line) throw(std::bad_alloc);
//void operator delete(void * p) throw();

#define DEBUG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
//#define new new(__FILE__, __LINE__)



byte *tga_24to32(int width, int height, byte *pBits, bool bgr);
float abs32(float val);
int abs32(int val);


double fsin(double rad);
double fcos(double rad);

int debugf(const char *format, ...);

#define MY_PI 3.14159265359f
#define MY_HALF_PI 1.5707963268f
#define MAXLINE 4096
#define FBO_RESOLUTION 1024
#define SHADOWMAP_DEFAULT_RES FBO_RESOLUTION
#define FILE_OFFSET 34


//quake3 game units, 8 units = 1 foot, ~3.3ft per meter (each unit is 8 values large)
#define UNITS_TO_METERS (8.0f * 8.0f * 3.3f)


#endif
