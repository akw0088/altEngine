#include "include.h"

#ifndef WIN32

int EventProc(Display *display, Window window, GLXContext context);

long long mstime()
{
    struct timeval te; 
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // caculate milliseconds
    return milliseconds;
}


int main(int argc, char *argv[])
{
	Display			*display;
	XVisualInfo		*vinfo;
	XSetWindowAttributes	winattrs;
	Window			window;
	GLXContext		context;
	long long		last_time = mstime();

	int visual[] = {	GLX_RGBA, GLX_DOUBLEBUFFER,
					GLX_RED_SIZE, 8,
					GLX_GREEN_SIZE, 8,
					GLX_BLUE_SIZE, 8,
					GLX_DEPTH_SIZE, 16,
					None	};

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

	XMapWindow(display, window);
	glXMakeCurrent(display, window, context);

	printf("GL Version: %s\n", glGetString(GL_VERSION));


	while (True)
	{
		if ( XPending(display) )
		{
			if ( EventProc(display, window, context) )
				break;
		}
		else
		{
			if (mstime() - last_time >= 16)
			{
				last_time = mstime();
				EventProc((Display *)1, window, context);
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
	static int		xcenter, ycenter;
	static int		frame_step;

	if (display == NULL)
	{
		altEngine.render(16);
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
		break;
	case ButtonPress:
		switch(event.xbutton.button)
		{
		case Button1:
			altEngine.keypress("leftbutton", true);
			break;
		case Button2:
			altEngine.keypress("rightbutton", true);
			break;
		case Button3:
			altEngine.keypress("middlebutton", true);
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
			altEngine.keypress("rightbutton", false);
			break;
		case Button3:
			altEngine.keypress("middlebutton", false);
			break;
		default:
			break;
        	}
		break;
	case MotionNotify:
		if ((event.xmotion.x == xcenter) && (event.xmotion.y == ycenter))
			break;

		if ( altEngine.mousepos(event.xmotion.x, event.xmotion.y, event.xmotion.x - xcenter, event.xmotion.y - ycenter) )
		{
			XWarpPointer(display, None, window, 0, 0, 0, 0, xcenter, ycenter);
		}
		break;
	case KeyPress:
	case KeyRelease:
		{
			XComposeStatus compose;
			int count;
			char buffer[128] = {0};

			bool pressed = false;
			KeySym keysym = XKeycodeToKeysym(display, event.xkey.keycode, 0);

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
			altEngine.init((void *)display, (void *)&window);
		init = true;
		break;
	case DestroyNotify:
		printf("DestroyNotify\n");
		altEngine.destroy();
		return 1;
	}
	return 0;
}

#endif
