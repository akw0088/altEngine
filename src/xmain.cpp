#include "include.h"

#ifndef WIN32
#include <X11/XKBlib.h>
#include <X11/cursorfont.h>
#include <signal.h>

int EventProc(Display *display, Window window, GLXContext context);

int timer_tick = 0;
char cmdline[1024] = {0};

void timer_handler(int sig, siginfo_t *si, void *uc)
{
	timer_t *timer_d = (timer_t *)si->si_value.sival_ptr;

	timer_tick = 1;
}

int make_timer(char *name, timer_t *timer_id, int interval )
{
    struct sigevent         te;
    struct itimerspec       its;
    struct sigaction        sa;
    int                     sigNo = SIGRTMIN;

    // Set up signal handler
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = timer_handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(sigNo, &sa, NULL) == -1)
    {
        fprintf(stderr, "Failed to setup signal handling for %s.\n", name);
        return(-1);
    }

    // Set and enable alarm
    te.sigev_notify = SIGEV_SIGNAL;
    te.sigev_signo = sigNo;
    te.sigev_value.sival_ptr = timer_id;
    timer_create(CLOCK_REALTIME, &te, timer_id);

    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = interval * 1000000;
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = interval * 1000000;
    timer_settime(*timer_id, 0, &its, NULL);

    return(0);
}


int main(int argc, char *argv[])
{
	Display			*display;
	XVisualInfo		*vinfo;
	XSetWindowAttributes	winattrs;
	Window			window;
	GLXContext		context;
	timer_t time_id;

	int visual[] = {	GLX_RGBA, GLX_DOUBLEBUFFER,
					GLX_RED_SIZE, 8,
					GLX_GREEN_SIZE, 8,
					GLX_BLUE_SIZE, 8,
					GLX_DEPTH_SIZE, 16,
					None	};


	for(int i = 0; i < argc; i++)
	{
		strcat(cmdline, argv[i]);
		strcat(cmdline, " ");
	}
	printf("commandline: %d %s\n", argc, cmdline);

	display	= XOpenDisplay(NULL);
	if (!display)
	{
		printf("Cannot connect to X server %s\n", XDisplayName(NULL));
		return 0;
	}
	vinfo = glXChooseVisual(display, DefaultScreen(display), visual);
	if (!vinfo)
	{
		printf("Unable to create suitable opengl visual.\n");
		XCloseDisplay(display);
		return 0;
	}
	context	= glXCreateContext(display, vinfo, 0, True);
	if (!context)
	{
		printf("Unable to create opengl context.\n");
		XCloseDisplay(display);
		return 0;
	}

//	memset(winattrs, 0, sizeof(XSetWindowAttributes));
	winattrs.colormap	= XCreateColormap(display, DefaultRootWindow(display), vinfo->visual, AllocNone);
	winattrs.event_mask	= ExposureMask | StructureNotifyMask | KeyPressMask | KeyReleaseMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask;
	winattrs.border_pixel	= 0;

	window = XCreateWindow(display, DefaultRootWindow(display), 0, 0, 640, 480, 0, vinfo->depth, InputOutput, vinfo->visual,
		CWBorderPixel | CWColormap | CWEventMask, &winattrs);

	XSetStandardProperties(display, window, "altEngine", "opengl", None, argv, argc, NULL );

	if ( glXMakeCurrent(display, window, context) )
	{
		printf("GL Version: %s\n", glGetString(GL_VERSION));
	}

	XMapWindow(display, window);
	make_timer("Timer", &time_id, 8);


	while (True)
	{
		if ( XPending(display) )
		{
			if ( EventProc(display, window, context) )
				break;
		}
		else
		{
			if (timer_tick)
			{
				EventProc((Display *)1, window, context);
				timer_tick = 0;
			}

			EventProc(NULL, window, context);
		}
	}
	printf("Closing...\n");
	glXDestroyContext(display, context);
	XCloseDisplay(display);
	return 0;
}

int EventProc(Display *display, Window window, GLXContext context)
{
	XEvent			event;
	static Engine		altEngine;
	static bool		init = false;
	static bool		once = false;
	static int		xcenter, ycenter;
	static int		frame_step;
	static Cursor invisibleCursor;
	static Cursor cursor;
	static Pixmap bitmapNoData;

	if (display == NULL)
	{
#ifndef DEDICATED
		altEngine.render(TICK_MS);
#endif
		return 0;
	}
	else if (display == (void *)1)
	{
		altEngine.step(frame_step++);
		return 0;
	}


	XNextEvent(display, &event);
	switch(event.type)
	{
	case CreateNotify:
		printf("CreateNotify\n");
		break;
	case ConfigureNotify:
		printf("ConfigureNotify\n");
		xcenter = event.xconfigure.width / 2;
		ycenter = event.xconfigure.height / 2;
		altEngine.resize(event.xconfigure.width, event.xconfigure.height);
		break;
	case MapNotify:
		printf("MapNotify\n");
#ifdef DEDICATED
		XUnmapWindow(display, window);
#endif
		break;
	case ButtonPress:
		switch(event.xbutton.button)
		{
		case Button1:
			altEngine.keypress("leftbutton", true);
			break;
		case Button2:
			altEngine.keypress("middlebutton", true);
			break;
		case Button3:
			altEngine.keypress("rightbutton", true);
			break;
		case Button4:
			altEngine.keypress("mousewheelup", true);
			break;
		case Button5:
			altEngine.keypress("mousewheeldown", true);
			break;
		default:
			break;
        	}
        break;
	case ButtonRelease:
		switch(event.xbutton.button)
		{
		case Button1:
			altEngine.keypress("leftbutton", false);
			break;
		case Button2:
			altEngine.keypress("middlebutton", false);
			break;
		case Button3:
			altEngine.keypress("rightbutton", false);
			break;
		case Button4:
			//altEngine.keypress("mousewheelup", false);
			break;
		case Button5:
			//altEngine.keypress("mousewheeldown", false);
			break;
		default:
			break;
        	}
		break;
	case MotionNotify:
		if ((event.xmotion.x == xcenter) && (event.xmotion.y == ycenter))
			break;

		if (once == false)
		{
			once = true;
			//resize fixes opengl context rendering for some reason
#ifndef DEDICATED
			XMoveResizeWindow(display, window, 0, 0, 1920, 1080);
#endif
		}

		if ( altEngine.mousepos(event.xmotion.x, event.xmotion.y, event.xmotion.x - xcenter, event.xmotion.y - ycenter) )
		{
			XWarpPointer(display, None, window, 0, 0, 0, 0, xcenter, ycenter);
			XDefineCursor(display, window, invisibleCursor);
		}
		else
		{
			XDefineCursor(display, window, cursor);
		}
		break;
	case KeyPress:
	case KeyRelease:
		{
			XComposeStatus compose;
			int count;
			char buffer[128] = {0};

			bool pressed = false;
			KeySym keysym = XkbKeycodeToKeysym(display, event.xkey.keycode, 0, 0);

			if (event.type == KeyPress)
			{
				pressed = true;
				count = XLookupString((XKeyEvent *)&event, buffer, 128, &keysym, &compose);
				for(int i = 0; i < count; i++)
				{
					altEngine.keystroke(buffer[i]);
				}
			}

			switch (keysym)
			{
			case XK_Return:
			case XK_KP_Enter:
			case XK_space:
				altEngine.keypress("enter", pressed);
				break;
			case XK_Shift_L:
			case XK_Shift_R:
				altEngine.keypress("shift", pressed);
				break;
			case XK_Control_L:
			case XK_Control_R:
				altEngine.keypress("control", pressed);
				break;
			case XK_Escape:
				altEngine.keypress("escape", pressed);
				break;
			case XK_Up:
			case XK_w:
				altEngine.keypress("up", pressed);
				break;
			case XK_Left:
			case XK_a:
				altEngine.keypress("left", pressed);
				break;
			case XK_Down:
			case XK_s:
				altEngine.keypress("down", pressed);
				break;
			case XK_Right:
			case XK_d:
				altEngine.keypress("right", pressed);
				break;
			}
		}
		break;
	case UnmapNotify:
		printf("UnmapNotify\n");
		break;
	case Expose:
		printf("Expose %d\n", event.xexpose.count);

		if (!init)
		{
			altEngine.init((void *)display, (void *)&window, cmdline);
			// Hide the cursor
			XColor black;
			static char noData[] = { 0,0,0,0,0,0,0,0 };
			black.red = black.green = black.blue = 0;

			bitmapNoData = XCreateBitmapFromData(display, window, noData, 8, 8);
			invisibleCursor = XCreatePixmapCursor(display, bitmapNoData, bitmapNoData, 
                                     &black, &black, 0, 0);
			cursor = XCreateFontCursor(display,XC_left_ptr);
			init = true;
		}
		break;
	case DestroyNotify:
		printf("DestroyNotify\n");
		altEngine.destroy();
		XFreeCursor(display, cursor);
		XFreeCursor(display, invisibleCursor);
		XFreePixmap(display, bitmapNoData);
		return 1;
	}
	return 0;
}

#endif
