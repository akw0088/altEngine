#include "include.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef WIN32
#include <io.h>
#include <fcntl.h>


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL setupPixelFormat(HDC);
void RedirectIOToConsole();
unsigned int getTimeStamp(void);

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
	wndclass.hIcon		= LoadIcon(hInstance, "altEngine");
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
	ShowWindow(hwnd, iCmdShow);
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
	static int	old_style = 0;
	static int	new_style = WS_CHILD | WS_VISIBLE;
	static int	xres, yres;
	static bool show_cursor = true;
	static int start = 0, end = 0, last_frametime = 0;


	switch (message)
	{
	case WM_CREATE:
	{
		HMONITOR hmon;
		MONITORINFO mi = { sizeof(MONITORINFO) };

		WSAStartup(MAKEWORD(2, 2), &wsadata);
		AllocConsole();
		RedirectIOToConsole();
		SetTimer(hwnd, TICK_TIMER, 16, NULL);
		hdc = GetDC(hwnd);

		hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
		GetMonitorInfo(hmon, &mi);

		xres = abs(mi.rcMonitor.right - mi.rcMonitor.left);
		yres = abs(mi.rcMonitor.bottom - mi.rcMonitor.top);

		setupPixelFormat(hdc);
#ifndef DIRECTX
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
//				WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
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
		altEngine.init(&shwnd, &hdc);
	}
	return 0;

	case WMU_RENDER:
	{
		start = GetTickCount();
		altEngine.render(last_frametime);
		end = GetTickCount();
		last_frametime = end - start;

		return 0;
	}
	case WM_TIMER:
		switch(wParam)
		{
		case TICK_TIMER:
			altEngine.step();
			tick_count++;
			break;
		}
		return 0;

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
				break;
			case VK_TAB:
				break;
			case VK_RETURN:
				altEngine.keypress("enter", pressed);
				break;
			case VK_SHIFT:
				altEngine.keypress("shift", pressed);
				break;
			case VK_CONTROL:
				altEngine.keypress("control", pressed);
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
				if (tick_count > 60 + last_resize)
				{
					last_resize = tick_count;
					old_style = SetWindowLong(hwnd, GWL_STYLE, new_style);
					new_style = old_style;
					SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, xres, yres, 0);
					break;
				}
			}
			return 0;
		}
	case WM_CHAR:
		altEngine.keystroke((char)wParam);
		return 0;
	case WM_SIZE:
		{
			int	width, height;

			width	= LOWORD(lParam);
			height	= HIWORD(lParam);
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
#ifndef DIRECTX
		if (hglrc)
		{
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(hglrc);
			ReleaseDC(hwnd, hdc);
		}
#endif
		altEngine.destroy();
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

	pfd.nSize			= sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion		= 1;
	pfd.dwFlags			= PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_GENERIC_ACCELERATED | PFD_DOUBLEBUFFER;
	pfd.dwLayerMask		= PFD_MAIN_PLANE;
	pfd.iPixelType		= PFD_TYPE_RGBA;
	pfd.cColorBits		= 32;
	pfd.cDepthBits		= 24;
	pfd.cAccumBits		= 32;
	pfd.cStencilBits	= 32;

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

char *get_file(char *filename)
{
	FILE	*file;
	char	*buffer;
	int	file_size, bytes_read;

	file = fopen(filename, "rb");
	if (file == NULL)
		return 0;
	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	fseek(file, 0, SEEK_SET);
	buffer = new char [file_size + 1];
	bytes_read = (int)fread(buffer, sizeof(char), file_size, file);
	if (bytes_read != file_size)
		return 0;

	fclose(file);
	buffer[file_size] = '\0';
	return buffer;
}

int write_file(char *filename, char *bytes, int size)
{
	FILE *fp = fopen(filename, "wb");
	int ret;

	if (fp == NULL)
	{
		perror("Unable to open file for writing");
		return -1;
	}

	ret = fwrite(bytes, sizeof(char), size, fp);

	if (ret != size)
	{
		printf("fwrite didnt write all data\n");
		return -1;
	}
	fclose(fp);
	return 0;
}

unsigned int getTimeStamp(void)
{
	unsigned int timestamp = 0;

	_asm
	{
		// rdtsc returns 64bit "timestamp" in edx:eax, timestamp is really a count of clock cycles
		rdtsc
		// we are only interested in small time intervals, so highword is worthless,
		mov	DWORD PTR timestamp, eax
	}
	return timestamp;
}

void RedirectIOToConsole()
{
	int	hConHandle;
	long	lStdHandle;
	FILE	*fp;
	CONSOLE_SCREEN_BUFFER_INFO	coninfo;

	AllocConsole();

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
}
#endif