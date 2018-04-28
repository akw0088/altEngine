#include "include.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef WIN32
#include <io.h>
#include <fcntl.h>
#include <xmmintrin.h>

//xbox controller
#include <xutility>
#include <Xinput.h>

// Raw mouse input
#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
#endif

bool debug = false;
double freq = 0.0;
double com_maxfps = 1000.0f / 250;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void clipboard_paste(HWND hwnd, char *value, int size); 
BOOL setupPixelFormat(HDC);
void RedirectIOToConsole(bool debug);
unsigned int getTimeStamp(void);
double GetCounter(double freq);
void GetFreq(double &freq);
void get_cpu_info(struct cpuinfo *);
void show_hw_info();
void xbox_controller(HWND hwnd, int index, Engine *engine);

//for dxerr
int (WINAPIV * __vsnprintf)(char *, size_t, const char*, va_list) = _vsnprintf;


#define TICK_TIMER 1
#define WMU_RENDER WM_USER + 1

#if 1
char *alloc_buffer = (char *)malloc(0x40000000); // 1GB of memory;

void * operator new(size_t n) throw(std::bad_alloc)
{
	static int index = 0;
	void *pointer = &alloc_buffer[index];
	index += n;
	//printf("Allocate %d bytes address %X [%X of %X used]\r\n", n, pointer, index, 0x40000000);
	return pointer;
}

void operator delete(void * p) throw()
{
//	printf("Delete %X\r\n", p);
}
#endif


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

	GetFreq(freq);

	if (strstr(szCmdLine, "debug"))
	{
		debug = true;
	}


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


	show_hw_info();


	if (!SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS))
	{
		DWORD dwError = GetLastError();
		printf("Failed to set process priority to high Error #%d)\n", dwError);
	}


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
	static HCURSOR hCursor;
	
#ifdef VULKAN
	static bool resized = false;
	static bool initialized = false;
#endif



	switch (message)
	{
	case WM_CREATE:
	{
		WSAStartup(MAKEWORD(2, 2), &wsadata);
#ifndef NDEBUG
		debug = true;
#endif


		RedirectIOToConsole(debug);

		hCursor = LoadCursorFromFile("media/mouse.cur");
		if (hCursor)
			SetCursor(hCursor);

		SetProcessDPIAware();
		SetTimer(hwnd, TICK_TIMER, TICK_MS, NULL);
		hdc = GetDC(hwnd);
		GetFreq(freq);
		setupPixelFormat(hdc);
#ifdef OPENGL
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
#ifndef DEDICATED
			xbox_controller(hwnd, altEngine.controller, &altEngine);
#endif
			altEngine.step(tick_count++);
			break;
		}
		return 0;
	case WM_INPUT:
		{
			UINT dwSize = 40;
			static BYTE lpb[40];

			GetRawInputData((HRAWINPUT)lParam, RID_INPUT,
				lpb, &dwSize, sizeof(RAWINPUTHEADER));

			RAWINPUT* raw = (RAWINPUT*)lpb;

			if (raw->header.dwType == RIM_TYPEMOUSE)
			{
				int x = raw->data.mouse.lLastX;
				int y = raw->data.mouse.lLastY;


				if (altEngine.mousepos_raw(center.x, center.y, x, y))
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
		}
		break;
	case WM_MOUSEMOVE:
		{
			int	x, y;

			x = LOWORD(lParam);
			y = HIWORD(lParam);

			if ((x == center.x) && (y == center.y))
				return 0;

//			if (tick_count == last_tick)
//				return 0;

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
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		{
			bool pressed = (message == WM_LBUTTONDOWN) ? true : false;
			altEngine.keypress("leftbutton", pressed);
			return 0;
		}
	case WM_MBUTTONDOWN:
	{
		char data[512];
		clipboard_paste(hwnd, data, 512);
		altEngine.paste(data, strlen(data) + 1);
	}
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

	case WM_SETCURSOR:
		if (LOWORD(lParam) == HTCLIENT)
		{
			SetCursor(hCursor);
			return TRUE;
		}
		break;


	case WM_SYSKEYUP:
	case WM_SYSKEYDOWN:
	{
		bool pressed = (message == WM_SYSKEYDOWN) ? true : false;

		switch (wParam)
		{
		case VK_F10:
			altEngine.keypress("F10", pressed);
			return 0;
		case VK_RETURN:
			if ((HIWORD(lParam) & KF_ALTDOWN))
			{
				altEngine.console("fullscreen");
				break;
			}
		}
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
//	case WM_SYSCHAR:
//		return TRUE;


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
		altEngine.keystroke((char)wParam, NULL);
		return 0;
	case WM_SETFOCUS:
	{
		POINT screen = center;
		ClientToScreen(hwnd, &screen);
		SetCursorPos(screen.x, screen.y);
		if (show_cursor == true)
		{
			show_cursor = false;
			ShowCursor(FALSE);
		}
		return 0;
	}
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
#ifdef OPENGL
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

void RedirectIOToConsole(bool debug)
{
	if (debug)
	{
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

		fp = _fdopen(hConHandle, "w");
		*stdout = *fp;
		setvbuf(stdout, NULL, _IONBF, 0);

		// redirect unbuffered STDIN to the console
		lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
		hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

		fp = _fdopen(hConHandle, "r");
		*stdin = *fp;
		setvbuf(stdin, NULL, _IONBF, 0);

		// redirect unbuffered STDERR to the console
		lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
		hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
		fp = _fdopen(hConHandle, "w");
		*stderr = *fp;
		setvbuf(stderr, NULL, _IONBF, 0);

		// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog point to console as well
		//ios::sync_with_stdio();

		//Fix issue on windows 10
		FILE *fp2 = freopen("CONOUT$", "w", stdout);
	}
	else
	{
		freopen("altEngine.log", "a", stdout);
		freopen("altEngine.log", "a", stderr);
	}
}


void get_resolution(int &width, int &height, int &bpp, int &refresh_rate)
{
	DEVMODE dmScreenSettings;

	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dmScreenSettings);
	width = dmScreenSettings.dmPelsWidth;
	height = dmScreenSettings.dmPelsHeight;
	bpp = dmScreenSettings.dmBitsPerPel;
	refresh_rate = dmScreenSettings.dmDisplayFrequency;
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

	if (freq < 1000)
		return 0.0;

	return (double)li.QuadPart / freq;
}

int clipboard_copy(HWND hwnd, char *value, int size)
{
	HGLOBAL hglobal;
	char *str = NULL;
	BOOL ret = 0;

	ret = OpenClipboard(hwnd);
	hglobal = GlobalAlloc(GMEM_MOVEABLE, size);
	if (hglobal == NULL)
	{
		CloseClipboard();
		return -1;
	}
	EmptyClipboard();
	str = (char *)GlobalLock(hglobal);
	if (str)
	{
		sprintf(str, "%s", value);
		ret = GlobalUnlock(hglobal);
		if (SetClipboardData(CF_TEXT, hglobal) == NULL)
		{
			printf("error\n");
		}
	}
	ret = CloseClipboard();
	return 0;
}

void clipboard_paste(HWND hwnd, char *value, int size)
{
	HGLOBAL hglobal;
	char *str = NULL;

	memset(value, 0, size);
	OpenClipboard(hwnd);

	if (IsClipboardFormatAvailable(CF_TEXT))
	{
		hglobal = GetClipboardData(CF_TEXT);
		if (hglobal != NULL)
		{
			str = (char *)GlobalLock(hglobal);
			if (str != NULL)
			{
				strncpy(value, str, size - 1);
				GlobalUnlock(hglobal);
			}
		}
	}
	CloseClipboard();
}

void GetScreenShot(HWND hwnd, unsigned int &luminance, bool luminance_only)
{
	RECT rect;
	char filename[256];
	static int count = 0;

	GetClientRect(hwnd, &rect);

	POINT p1, p2;

	p1.x = rect.left;
	p1.y = rect.top;
	p2.x = rect.right;
	p2.y = rect.bottom;
	ClientToScreen(hwnd, &p1);
	ClientToScreen(hwnd, &p2);

	// copy screen to bitmap

	HDC     hScreen = GetDC(NULL);
	HDC     hDC = CreateCompatibleDC(hScreen);
//	HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, rect.right, rect.bottom);
	BITMAPINFO bmi;
	char *bytes = NULL;

	memset(&bmi, 0, sizeof(BITMAPINFO));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biHeight = rect.bottom;
	bmi.bmiHeader.biWidth = rect.right;
	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	HBITMAP hBitmap = CreateDIBSection(hScreen, &bmi, DIB_RGB_COLORS, (LPVOID*)&bytes, NULL, 0);

	BITMAPFILEHEADER bmfh;
	memset(&bmfh, 0, sizeof(BITMAPFILEHEADER));

	bmfh.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M"  
	bmfh.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) +
		bmi.bmiHeader.biSize + bmi.bmiHeader.biClrUsed
		* sizeof(RGBQUAD) + bmi.bmiHeader.biSizeImage);

	bmfh.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + bmi.bmiHeader.biSize + bmi.bmiHeader.biClrUsed * sizeof(RGBQUAD);


	

	HGDIOBJ old_obj = SelectObject(hDC, hBitmap);
	BOOL    bRet = BitBlt(hDC, 0, 0, rect.right, rect.bottom, hScreen, p1.x, p1.y, SRCCOPY);



	BITMAP bitmap;
	GetObject(hBitmap, sizeof(bitmap), &bitmap);
	int size = bitmap.bmWidth * bitmap.bmHeight * bitmap.bmBitsPixel;

	if (bitmap.bmBits && luminance_only == false)
	{
		sprintf(filename, "screenshot%d.bmp", count++);
		FILE *fp = fopen(filename, "wb");
		fwrite(&bmfh, sizeof(bmfh), 1, fp);
		fwrite(&bmi, sizeof(bmi), 1, fp);
		fwrite(bitmap.bmBits, bitmap.bmWidth * bitmap.bmHeight * 4, 1, fp);
		fclose(fp);
	}


	uint64_t average = 0;
	unsigned char *data = (unsigned char *)bitmap.bmBits;
	for (int i = 0; i < bitmap.bmWidth * bitmap.bmHeight * 4;)
	{
		average += data[i];
		average += data[i+1];
		average += data[i+2];
//		average += data[i+3];
		i += 4;
	}
	average =  (uint64_t)((float)average / (bitmap.bmWidth * bitmap.bmHeight * 4));
	luminance = (int)average;

	if (luminance_only == false)
	{
		// save bitmap to clipboard
		OpenClipboard(NULL);
		EmptyClipboard();
		SetClipboardData(CF_BITMAP, hBitmap);
		CloseClipboard();
	}

	// clean up
	SelectObject(hDC, old_obj);
	DeleteDC(hDC);
	ReleaseDC(NULL, hScreen);
	DeleteObject(hBitmap);
}


BOOL CALLBACK SettingsProc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND	hwndCombo = 0;
	DEVMODE		dmScreenSettings;
	TCHAR		resbuf[128];
	static TCHAR	currentRes[128];
	int i;

	switch (message)
	{
	case WM_INITDIALOG:
//		hwndCombo = GetDlgItem(hdlg, IDD_RESOLUTION);

		// populate available resolutions
		for (i = 1; i != 0; i++)
		{
			if (EnumDisplaySettings(NULL, i - 1, &dmScreenSettings))
			{
				wsprintf(resbuf, "%dx%d %dbpp", dmScreenSettings.dmPelsWidth,
					dmScreenSettings.dmPelsHeight,
					dmScreenSettings.dmBitsPerPel);
				// add to combo box
				SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)resbuf);
			}
			else
			{
				i = -1;
			}
		}

		// make default selection in dropdown box
		EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dmScreenSettings);
		wsprintf(currentRes, "%dx%d %dbpp", dmScreenSettings.dmPelsWidth,
			dmScreenSettings.dmPelsHeight,
			dmScreenSettings.dmBitsPerPel);
		SendMessage(hwndCombo, CB_SELECTSTRING, -1, (LPARAM)currentRes);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
//		case IDD_RESOLUTION:
//			return TRUE;
		case IDOK:
			//if (SendMessage(GetDlgItem(hdlg, IDD_FULLSCREEN), BM_GETCHECK, 0, 0))
			{
				// check res and go fullscreen
				memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
				dmScreenSettings.dmSize = sizeof(dmScreenSettings);
				dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
				SendMessage(hwndCombo, LB_GETTEXT, SendMessage(hwndCombo, CB_GETCURSEL, 0, 0), (LPARAM)currentRes);
				/*				sscanf(currentRes, "%dx%x %dbpp",	&(dmScreenSettings.dmPelsWidth),
				&(dmScreenSettings.dmPelsHeight),
				&(dmScreenSettings.dmBitsPerPel));*/
				ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
			}
			EndDialog(hdlg, 0);
			return TRUE;
		case IDCANCEL:
			EndDialog(hdlg, 0);
			return TRUE;
		}
		break;
	}
	return FALSE;
}



void register_raw_mouse(HWND hwnd)
{
	RAWINPUTDEVICE Rid[1];
	Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
	Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
	Rid[0].dwFlags = RIDEV_INPUTSINK;
	Rid[0].hwndTarget = hwnd;
	if (RegisterRawInputDevices(Rid, 1, sizeof(Rid[0])) == FALSE)
	{
		debugf("RegisterRawInputDevices() failed\n");
	}
}

void unregister_raw_mouse(HWND hwnd)
{
	RAWINPUTDEVICE Rid[1];
	Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
	Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
	Rid[0].dwFlags = RIDEV_REMOVE;
	Rid[0].hwndTarget = hwnd;
	RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));
}

#ifndef DEDICATED
void xbox_vibration(int index, bool left, bool right)
{
	_XINPUT_VIBRATION vib;

	memset(&vib, 0, sizeof(_XINPUT_VIBRATION));
	
	if (left)
		vib.wLeftMotorSpeed = 65535 >> 1;

	if (right)
		vib.wRightMotorSpeed = 65535 >> 1;

	XInputSetState(index, &vib);
}

// One came with my oculus, why not
void xbox_controller(HWND hwnd, int index, Engine *engine)
{
	static POINT pos = { 0 };
	XINPUT_STATE state;
	DWORD dwResult = 0;
	bool analog = false;
	bool trigger = false;

	memset(&state, 0, sizeof(state));
	dwResult = XInputGetState(index, &state);

	if (dwResult != ERROR_SUCCESS)
		return;


	vec2 left_analog(state.Gamepad.sThumbLX, state.Gamepad.sThumbLY);
	float left_mag = left_analog.magnitude();
	if (left_mag > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
	{
		left_mag = (left_mag - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) / (float)(UINT16_MAX - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
		left_analog = left_analog.normalize() * left_mag;

//		printf("left analog %3.3f %3.3f\n", left_analog.x, left_analog.y);

		if (left_analog.x > 0.1f)
		{
			analog = true;
			engine->keypress("right", true);
		}
		else if (left_analog.x < -0.1f)
		{
			analog = true;
			engine->keypress("left", true);
		}
		else
		{
			engine->keypress("right", false);
			engine->keypress("left", false);
		}


		if (left_analog.y > 0.1f)
		{
			analog = true;
			engine->keypress("up", true);
		}
		else if (left_analog.y < -0.1f)
		{
			analog = true;
			engine->keypress("down", true);
		}
		else
		{
			engine->keypress("up", false);
			engine->keypress("down", false);
		}
	}

	vec2 right_analog(state.Gamepad.sThumbRX, state.Gamepad.sThumbRY);
	float right_mag = right_analog.magnitude();
	if (right_mag > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
	{
		right_mag = (right_mag - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) / (float)(UINT16_MAX - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
		right_analog = right_analog.normalize() * right_mag;

		RECT rect;
		GetWindowRect(hwnd, &rect);

	//		printf("right analog %3.3f %3.3f\n", right_analog.x, right_analog.y);
		pos.x += (LONG)(right_analog.x * 20.0f);
		pos.y += (LONG)(right_analog.y * -20.0f);
		if (pos.x > rect.right)
			pos.x = rect.right;
		if (pos.x < rect.left)
			pos.x = rect.left;

		if (pos.y < rect.top)
			pos.y = rect.top;
		if (pos.y > rect.bottom)
			pos.y = rect.bottom;

		if (engine->q3map.loaded)
		{
			engine->mousepos(0, 0, (int)(10.0f * right_analog.x), (int)(-10.0f * right_analog.y));
		}
		else
		{
			SetCursorPos(pos.x, pos.y);
			engine->mousepos(pos.x, pos.y, (int)(10.0f * right_analog.x), (int)(-10.0f * right_analog.y));
		}
	}


	unsigned char right_trigger = state.Gamepad.bRightTrigger;
	if (right_trigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
	{
		float value = (right_trigger - XINPUT_GAMEPAD_TRIGGER_THRESHOLD) / (float)(UINT8_MAX - XINPUT_GAMEPAD_TRIGGER_THRESHOLD);

//		printf("right trigger %3.3f\n", value);

		if (value > 0.1f)
		{
			trigger = true;
			engine->keypress("space", true);
		}
		else
		{
			engine->keypress("space", false);
		}
	}

	unsigned char left_trigger = state.Gamepad.bLeftTrigger;
	if (left_trigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
	{
		float value = (left_trigger - XINPUT_GAMEPAD_TRIGGER_THRESHOLD) / (float)(UINT8_MAX - XINPUT_GAMEPAD_TRIGGER_THRESHOLD);

//		printf("left trigger %3.3f\n", value);

		if (value > 0.1f)
		{
			trigger = true;
			engine->keypress("space", true);
		}
		else
		{
			engine->keypress("space", false);
		}
	}



	if (analog == false)
	{
		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP)
		{
			engine->keypress("up", true);
		}
		else
		{
			engine->keypress("up", false);
		}
		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
		{
			engine->keypress("down", true);
		}
		else
		{
			engine->keypress("down", false);
		}
		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
		{
			engine->keypress("left", true);
		}
		else
		{
			engine->keypress("left", false);
		}
		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
		{
			engine->keypress("right", true);
		}
		else
		{
			engine->keypress("right", false);
		}

		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_START)
		{
			engine->keypress("escape", true);
		}
		else
		{
			engine->keypress("escape", false);
		}
		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK)
		{
			engine->keypress("tab", true);
		}
		else
		{
			engine->keypress("tab", false);
		}

	
		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB)
		{
			engine->keypress("[", true);
		}
		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)
		{
			engine->keypress("]", true);
		}
		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)
		{
			engine->keypress("[", true);
		}
		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
		{
			engine->keypress("]", true);
		}


		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_A)
		{
			engine->keypress("leftbutton", true);

			if (engine->q3map.loaded)
				xbox_vibration(0, false, true);
		}
		else
		{
			engine->keypress("leftbutton", false);
			if (engine->q3map.loaded)
				xbox_vibration(0, false, false);
		}
		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_B)
		{
			engine->keypress("rightbutton", true);
		}
		else
		{
			engine->keypress("rightbutton", false);
		}

		if (trigger == false)
		{
			if (state.Gamepad.wButtons & XINPUT_GAMEPAD_X)
			{
				engine->keypress("space", true);
			}
			else
			{
				engine->keypress("space", false);
			}
		}
		if (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y)
		{
			engine->keypress("shift", true);
		}
		else
		{
			engine->keypress("shift", false);
		}
	}
}
#endif

#endif
