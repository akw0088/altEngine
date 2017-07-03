#include "include.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef WIN32
#include <io.h>
#include <fcntl.h>


double com_maxfps = 1000.0f / 250;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL setupPixelFormat(HDC);
void RedirectIOToConsole();
unsigned int getTimeStamp(void);
double GetCounter(double freq);
void GetFreq(double &freq);
double freq = 0.0;

//for dxerr
int (WINAPIV * __vsnprintf)(char *, size_t, const char*, va_list) = _vsnprintf;


#define TICK_TIMER 1
#define WMU_RENDER WM_USER + 1

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, INT iCmdShow)
{
	HWND		hwnd;
	HACCEL		hAccel;
	MSG			msg;
	WNDCLASS	wndclass;
	char		szAppName[] = TEXT("WinMain");

	wndclass.style		= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.lpfnWndProc	= WndProc;
	wndclass.cbClsExtra	= 0;
	wndclass.cbWndExtra	= 0;
	wndclass.hInstance	= hInstance;
	wndclass.hIcon		= LoadIcon(hInstance, "WinMain");
	wndclass.hCursor	= LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground	= NULL;
	wndclass.lpszMenuName	= NULL;
	wndclass.lpszClassName	= szAppName;

	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);



	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("Unable to register window class."), szAppName, MB_ICONERROR);
		return 0;
	}
	hwnd = CreateWindow(	szAppName,			// window class name
				TEXT("altEngine2"),		// window caption
				WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,		// window style
				CW_USEDEFAULT,			// initial x position
				CW_USEDEFAULT,			// initial y position
				CW_USEDEFAULT,			// initial x size
				CW_USEDEFAULT,			// initial y size
				NULL,				// parent window handle
				NULL,				// window menu handle
				hInstance,			// program instance handle
				NULL);				// creation parameters

	hAccel = LoadAccelerators(hInstance, szAppName);
#ifndef DEDICATED
	ShowWindow(hwnd, iCmdShow);
#endif
	UpdateWindow(hwnd);
     
	while (TRUE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if ( GetMessage(&msg, NULL, 0, 0) > 0 )
			{
				if (!TranslateAccelerator(hwnd, hAccel, &msg))
				{
					TranslateMessage (&msg) ;
					DispatchMessage (&msg) ;
				}
			}
			else
			{
				break;
			}
		}
		else
		{
			SendMessage(hwnd, WMU_RENDER, 0, 0);
#ifdef DEDICATED
//			Sleep(1);
#endif
		}
	}

	if (!UnregisterClass(szAppName, hInstance))
	{
		MessageBox(NULL, TEXT("Unable to unregister class."), szAppName, MB_ICONERROR);
		return 1;
	}
	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND		shwnd;
	static HDC		hdc;
	static HGLRC	hglrc, hglrc_legacy;
	static Engine	altEngine;
	static POINT	center;
	static WSADATA	wsadata;
	static unsigned int		tick_count;
	static unsigned int		last_tick;
	static unsigned int		last_resize = 0;
	static bool show_cursor = true;
	static double start = 0.0, end = 0.0, last_frametime = 0.0;
	static double min_frametime = 0.0;
	static double max_frametime = 0.0;
	
#ifdef VULKAN
	static bool resized = false;
	static bool initialized = false;
#endif



	switch (message)
	{
	case WM_CREATE:
	{
		WSAStartup(MAKEWORD(2, 2), &wsadata);
		RedirectIOToConsole();

		SetTimer(hwnd, TICK_TIMER, TICK_MS, NULL);
		hdc = GetDC(hwnd);
		GetFreq(freq);
		setupPixelFormat(hdc);
#ifdef OPENGL32
		hglrc_legacy = wglCreateContext(hdc);
		wglMakeCurrent(hdc, hglrc_legacy);
		glewInit();

		if (wglewIsSupported("WGL_ARB_create_context") == TRUE)
		{
			const int context[] =
			{
				WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
				WGL_CONTEXT_MINOR_VERSION_ARB, 4,
//				WGL_CONTEXT_FLAGS_ARB,
//				WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | WGL_CONTEXT_DEBUG_BIT_ARB,
				WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
				0
			};

			const int pixelformat[] =
			{
				WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
				WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
				WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
				WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
				WGL_COLOR_BITS_ARB, 32,
				WGL_DEPTH_BITS_ARB, 24,
				WGL_STENCIL_BITS_ARB, 8,
				0,
			};

			int format;
			unsigned int num_formats;

			wglChoosePixelFormatARB(hdc, (int *)pixelformat, NULL, 1, &format, &num_formats);
			hglrc = wglCreateContextAttribsARB(hdc, 0, context);
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(hglrc_legacy);
			wglMakeCurrent(hdc, hglrc);
		}
		else
		{
			//opengl 2.0
			hglrc = hglrc_legacy;
		}

#endif
		shwnd = hwnd;
#ifndef VULKAN
		altEngine.init(&shwnd, &hdc, (char *)GetCommandLine());
#endif
	}
	return 0;

	case WMU_RENDER:
	{
		if (end - start < com_maxfps)
		{
			end = GetCounter(freq);
			return 0;
		}

		start = GetCounter(freq);
		altEngine.render(last_frametime);
		end = GetCounter(freq);
		last_frametime = end - start;
		min_frametime = MIN(min_frametime, last_frametime);
		max_frametime = MIN(max_frametime, last_frametime);


		return 0;
	}
	case WM_TIMER:
		switch(wParam)
		{
		case TICK_TIMER:
			altEngine.step(tick_count++);
			break;
		}
		return 0;

	case WM_MOUSEWHEEL:
	{
		short int zDelta = (short)HIWORD(wParam);

		if (zDelta > 0)
		{
			altEngine.keypress("mousewheelup", true);
		}
		else
		{
			altEngine.keypress("mousewheeldown", true);
		}
		return 0;
	}
	case WM_MOUSEMOVE:
		{
			int	x, y;

			x = LOWORD(lParam);
			y = HIWORD(lParam);

			if ((x == center.x) && (y == center.y))
				return 0;


			if (tick_count == last_tick)
				return 0;

			last_tick = tick_count;

			if ( altEngine.mousepos(x, y, x - center.x, y - center.y) )
			{
				POINT screen = center;
				ClientToScreen(hwnd, &screen);
				SetCursorPos(screen.x, screen.y);
				if (show_cursor == true)
				{
					show_cursor = false;
					ShowCursor(FALSE);
				}
			}
			else
			{
				if (show_cursor == false)
				{
					show_cursor = true;
					ShowCursor(TRUE);
				}
			}
		}
		return 0;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		{
			bool pressed = (message == WM_LBUTTONDOWN) ? true : false;
			altEngine.keypress("leftbutton", pressed);
			return 0;
		}
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
		{
			bool pressed = (message == WM_MBUTTONDOWN) ? true : false;
			altEngine.keypress("middlebutton", pressed);
			return 0;
		}
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		{
			bool pressed = (message == WM_RBUTTONDOWN) ? true : false;
			altEngine.keypress("rightbutton", pressed);
			return 0;
		}

	case WM_KEYDOWN:
	case WM_KEYUP:
		{
			bool pressed = (message == WM_KEYDOWN) ? true : false;

			switch (wParam)
			{
			case VK_PAUSE:
				altEngine.keypress("pause", pressed);
				break;
			case VK_TAB:
				altEngine.keypress("tab", pressed);
				break;
			case VK_RETURN:
				altEngine.keypress("enter", pressed);
				break;
			case VK_SPACE:
				altEngine.keypress("space", pressed);
				break;
			case VK_SHIFT:
				altEngine.keypress("shift", pressed);
				break;
			case VK_CONTROL:
				altEngine.keypress("control", pressed);
				break;
			case VK_MENU:
				altEngine.keypress("alt", pressed);
				break;
			case VK_ESCAPE:
				altEngine.keypress("escape", pressed);
				break;
			case VK_UP:
				altEngine.keypress("up", pressed);
				break;
			case VK_LEFT:
				altEngine.keypress("left", pressed);
				break;
			case VK_DOWN:
				altEngine.keypress("down", pressed);
				break;
			case VK_RIGHT:
				altEngine.keypress("right", pressed);
				break;
			case VK_NUMPAD0:
				altEngine.keypress("numpad0", pressed);
				break;
			case VK_NUMPAD1:
				altEngine.keypress("numpad1", pressed);
				break;
			case VK_NUMPAD2:
				altEngine.keypress("numpad2", pressed);
				break;
			case VK_NUMPAD3:
				altEngine.keypress("numpad3", pressed);
				break;
			case VK_NUMPAD4:
				altEngine.keypress("numpad4", pressed);
				break;
			case VK_NUMPAD5:
				altEngine.keypress("numpad5", pressed);
				break;
			case VK_NUMPAD6:
				altEngine.keypress("numpad6", pressed);
				break;
			case VK_NUMPAD7:
				altEngine.keypress("numpad7", pressed);
				break;
			case VK_NUMPAD8:
				altEngine.keypress("numpad8", pressed);
				break;
			case VK_NUMPAD9:
				altEngine.keypress("numpad9", pressed);
				break;
			case VK_F1:
				altEngine.keypress("F1", pressed);
				break;
			case VK_F2:
				altEngine.keypress("F2", pressed);
				break;
			case VK_F3:
				altEngine.keypress("F3", pressed);
				break;
			case VK_F4:
				altEngine.keypress("F4", pressed);
				break;
			case VK_F5:
				altEngine.keypress("F5", pressed);
				break;
			case VK_F6:
				altEngine.keypress("F6", pressed);
				break;
			case VK_F7:
				altEngine.keypress("F7", pressed);
				break;
			case VK_F8:
				altEngine.keypress("F8", pressed);
				break;
			case VK_F9:
				altEngine.keypress("F9", pressed);
				break;
			case VK_F10:
				altEngine.keypress("F10", pressed);
				break;
			case VK_F11:
				altEngine.keypress("F11", pressed);
				break;
			case VK_F12:
				altEngine.keypress("F12", pressed);
				break;
			case VK_INSERT:
				altEngine.keypress("insert", pressed);
				break;
			case VK_DELETE:
				altEngine.keypress("delete", pressed);
				break;
			case VK_HOME:
				altEngine.keypress("home", pressed);
				break;
			case VK_END:
				altEngine.keypress("end", pressed);
				break;
			case VK_MULTIPLY:
				altEngine.keypress("numpad*", pressed);
				break;
			case VK_ADD:
				altEngine.keypress("numpad+", pressed);
				break;
			case VK_SUBTRACT:
				altEngine.keypress("numpad-", pressed);
				break;
			case VK_DIVIDE:
				altEngine.keypress("numpad/", pressed);
				break;
			case VK_DECIMAL:
				altEngine.keypress("numpad.", pressed);
				break;
			case VK_CAPITAL:
				altEngine.keypress("capslock", pressed);
				break;
			case VK_PRIOR:
				altEngine.keypress("pgup", pressed);
				break;
			case VK_NEXT:
				altEngine.keypress("pgdown", pressed);
				break;
			}


			if ((wParam >= 'A' && wParam <= 'Z') || (wParam >= 'a' && wParam <= 'z') || (wParam >= '0' && wParam <= '9'))
			{
				char c[2];

				c[0] = wParam;
				c[1] = '\0';
				altEngine.keypress(c, pressed);
				return DefWindowProc(hwnd, message, wParam, lParam);;
			}

			return 0;
		}
	case WM_CHAR:
		altEngine.keystroke((char)wParam);
		return 0;
	case WM_SIZE:
		{
			int	width, height;

			width = LOWORD(lParam);
			height = HIWORD(lParam);
#ifdef VULKAN
			if (resized == false)
			{
				RECT rect;
				int image_width = 1024;
				int image_height = 768;

				rect.top = 160;
				rect.left = 120;
				rect.right = 120 + image_width;
				rect.bottom = 160 + image_height;
				resized = true;
				// Vulkan is picky about client area matching the texture dimensions, SetWindowPos includes the window border
				AdjustWindowRectEx(&rect, GetWindowLong(hwnd, GWL_STYLE), FALSE, GetWindowLong(hwnd, GWL_EXSTYLE));
				SetWindowPos(hwnd, HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, 0);
			}

			if (width == 1024 && initialized == false)
			{
				initialized = true;
				altEngine.init(&shwnd, &hdc, (char *)GetCommandLine());
			}
#endif

			center.x = width / 2;
			center.y = height / 2;
			altEngine.resize(width, height);
		}
		return 0;

	case WM_SYSCOMMAND:
		switch(wParam)
		{
		case SC_SCREENSAVE:
		case SC_MONITORPOWER:
			return 0;
		}
		break;

	case WM_DESTROY:
#ifdef OPENGL32
		if (hglrc)
		{
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(hglrc);
			ReleaseDC(hwnd, hdc);
		}
#endif
		WSACleanup();
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

BOOL setupPixelFormat(HDC hdc)
{
	PIXELFORMATDESCRIPTOR pfd;
	INT pixelformat;

	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize		= sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion		= 1;
	pfd.dwFlags		= PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_GENERIC_ACCELERATED | PFD_DOUBLEBUFFER;
	pfd.dwLayerMask		= PFD_MAIN_PLANE;
	pfd.iPixelType		= PFD_TYPE_RGBA;
	pfd.cColorBits		= 32;
	pfd.cDepthBits		= 24;
	//pfd.cAccumBits	= 32;
	pfd.cStencilBits	= 8;

	if ( (pixelformat = ChoosePixelFormat(hdc, &pfd)) == 0 )
	{
		MessageBox(NULL, "ChoosePixelFormat failed", "Error", MB_OK);
		return FALSE;
	}

	if (SetPixelFormat(hdc, pixelformat, &pfd) == FALSE)
	{
		MessageBox(NULL, "SetPixelFormat failed", "Error", MB_OK);
		return FALSE;
	}
	return TRUE; 
}

unsigned int getTimeStamp(void)
{
	unsigned int timestamp = 0;
#ifndef _WIN64
	_asm
	{
		// rdtsc returns 64bit "timestamp" in edx:eax, timestamp is really a count of clock cycles
		rdtsc
		// we are only interested in small time intervals, so highword is worthless,
		mov	DWORD PTR timestamp, eax
	}
#endif
	return timestamp;
}

void RedirectIOToConsole()
{
#ifndef _WIN64
	int	hConHandle;
	long	lStdHandle;
	FILE	*fp;
	CONSOLE_SCREEN_BUFFER_INFO	coninfo;

	AllocConsole();
	HWND hwndConsole = GetConsoleWindow();

	ShowWindow(hwndConsole, SW_MAXIMIZE);
	// set the screen buffer to be big enough to let us scroll text
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);

	coninfo.dwSize.Y = 512;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

	// redirect unbuffered STDOUT to the console
	lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

	fp = _fdopen( hConHandle, "w" );
	*stdout = *fp;
	setvbuf( stdout, NULL, _IONBF, 0 );

	// redirect unbuffered STDIN to the console
	lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

	fp = _fdopen( hConHandle, "r" );
	*stdin = *fp;
	setvbuf( stdin, NULL, _IONBF, 0 );

	// redirect unbuffered STDERR to the console
	lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stderr = *fp;
	setvbuf( stderr, NULL, _IONBF, 0 );

	// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog point to console as well
	//ios::sync_with_stdio();

	//Fix issue on windows 10
	FILE *fp2 = freopen("CONOUT$", "w", stdout);
#endif
}


void set_resolution(int width, int height, int bpp)
{
	DEVMODE dm;

	memset(&dm, 0, sizeof(dm));
	dm.dmSize = sizeof(dm);
	dm.dmPelsWidth = width;
	dm.dmPelsHeight = height;
	dm.dmBitsPerPel = bpp;
	dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

	debugf("Attempting to set Video mode to %dx%d:%dbpp\n", dm.dmPelsWidth, dm.dmPelsHeight, dm.dmBitsPerPel);

	int ret = ChangeDisplaySettings(&dm, CDS_FULLSCREEN);
	if (ret == DISP_CHANGE_SUCCESSFUL)
	{
		debugf("video mode set\n");
	}
	else
	{
		debugf("*** Failed to set fullscreen mode: Error %d\n", ret);
	}
}

void GetFreq(double &freq)
{
	LARGE_INTEGER li;
	if (!QueryPerformanceFrequency(&li))
	{
		printf("QueryPerformanceFrequency() failed\n");
		return;
	}

	freq = double(li.QuadPart) / 1000.0;
}
double GetCounter(double freq)
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return (double)li.QuadPart / freq;
}

#endif
