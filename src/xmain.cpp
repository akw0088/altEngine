#include "include.h"

#ifndef WIN32
#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <signal.h>

int EventProc(Display *display, Window window, GLXContext context);

int timer_tick = 0;
char cmdline[1024] = {0};
char paste_value[512];

double com_maxfps;

//int clipboard_copy(Display *display, char *value, int size);
int clipboard_paste(Display *display, Window w, char *value, int size);
void show_utf8_prop(Display *dpy, Window w, Atom p, char *clip);

void timer_handler(int sig, siginfo_t *si, void *uc)
{
//	timer_t *timer_d = (timer_t *)si->si_value.sival_ptr;

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
    timer_create(CLOCK_MONOTONIC, &te, timer_id);

    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = interval * 1000000;
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = interval * 1000000;
    timer_settime(*timer_id, 0, &its, NULL);

    return(0);
}


void show_utf8_prop(Display *dpy, Window w, Atom p, char *clip)
{
    Atom da, incr, type;
    int di;
    unsigned long size, dul;
    unsigned char *prop_ret = NULL;

    // Dummy call to get type and size
    XGetWindowProperty(dpy, w, p, 0, 0, False, AnyPropertyType,
                       &type, &di, &dul, &size, &prop_ret);
    XFree(prop_ret);

    incr = XInternAtom(dpy, "INCR", False);
    if (type == incr)
    {
        printf("Data too large and INCR mechanism not implemented\n");
        return;
    }

    // Read the data in one go.
    printf("Property size: %lu\n", size);

    XGetWindowProperty(dpy, w, p, 0, size, False, AnyPropertyType,
                       &da, &di, &dul, &dul, &prop_ret);
    sprintf(clip, "%s", prop_ret);
    XFree(prop_ret);

    // Signal the selection owner that we have successfully read the data
    XDeleteProperty(dpy, w, p);
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
					GLX_DEPTH_SIZE, 24,
                                    	GLX_STENCIL_SIZE, 8,
					None	};


	for(int i = 0; i < argc; i++)
	{
		strncat(cmdline, argv[i], 1023);
		strncat(cmdline, " ", 1023);
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

/*
	char input[] = "hello";
	char data[10];
	memset(data, 0, 10);
	clipboard_copy(display, input, 6);
	clipboard_paste(display, window, data, 10);
*/
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
	//XEvent 			respond;
	static Engine		altEngine;
	static bool		init = false;
	static bool		once = false;
	static int		xcenter, ycenter;
	static int		frame_step;
	static Cursor		invisibleCursor;
	static Cursor		cursor;
	static Pixmap		bitmapNoData;
	static char		clip[512];
	XSelectionEvent *sev;
	static Atom		target_property;

        //Atom a1;
	//Atom type;
	//Window Sown;
        //int format;
	//int result;
        //unsigned long len;
	//unsigned long bytes_left;
	//unsigned long dummy;
        //unsigned char *data;
	XSelectionRequestEvent *req;

        XSelectInput(display, window, StructureNotifyMask);
        XSelectInput(display, window, StructureNotifyMask + ExposureMask);
        XSetSelectionOwner(display, XA_PRIMARY, window, CurrentTime);
        XFlush(display);


	if (display == NULL)
	{
#ifndef DEDICATED
		altEngine.render(TICK_MS);
#else
		usleep(1);
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
		target_property = XInternAtom(display, "PENGUIN", False);
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
				altEngine.keypress("up", pressed);
				break;
			case XK_w:
				altEngine.keypress("w", pressed);
				break;
			case XK_Left:
				altEngine.keypress("left", pressed);
				break;
			case XK_a:
				altEngine.keypress("a", pressed);
				break;
			case XK_Down:
				altEngine.keypress("down", pressed);
				break;
			case XK_s:
				altEngine.keypress("s", pressed);
				break;
			case XK_Right:
				altEngine.keypress("right", pressed);
				break;
			case XK_d:
				altEngine.keypress("d", pressed);
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
	case SelectionRequest:
		// Some window did a paste from something we have "selected/hightlighted" and wants the data
		XEvent respond;
                req = &(event.xselectionrequest);
//                printf("Selection Request from Mr %i I am %i\n", (int)event.xselection.requestor, (int)window);
//                printf("prop:%i tar:%i sel:%i\n", req->property, req->target, req->selection);
                if (req->target == XA_STRING)
                {
                        XChangeProperty(display, req->requestor, req->property, XA_STRING, strlen(paste_value), PropModeReplace, (unsigned char*)paste_value, strlen(paste_value));
                        respond.xselection.property = req->property;
                }
                else
                {
                        printf("No String %i\n", (int)req->target);
                        respond.xselection.property = None;
                }
                respond.xselection.type = SelectionNotify;
                respond.xselection.display = req->display;
                respond.xselection.requestor = req->requestor;
                respond.xselection.selection = req->selection;
                respond.xselection.target = req->target;
                respond.xselection.time = req->time;
                XSendEvent(display, req->requestor, 0, 0, &respond);
                XFlush(display);
		break;
	case SelectionNotify:
                sev = (XSelectionEvent*)&event.xselection;
                if (sev->property == None)
                {
                    printf("Conversion could not be performed.\n");
                    return 1;
                }
                else    
                {
                    show_utf8_prop(display, window, target_property, &clip[0]);
                    return 0;
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


int clipboard_copy(char *value, int size)
{
	sprintf(paste_value, "%s", value);
	return 0;
}

int clipboard_paste(Display *display, Window window, char *value, int size)
{
	// Copy from application
	Atom type;
	XSelectInput(display, window, StructureNotifyMask + ExposureMask);
	int format, result;
	unsigned long len, bytes_left, dummy;
	unsigned char *data;
	Window owner; // selection owner

	owner = XGetSelectionOwner(display, XA_PRIMARY);
	printf("Selection owner%i\n", (int)owner);
	if (owner == None)
	{
		return -1;
	}
	XSelectInput(display, window, SelectionNotify);
	XConvertSelection(display, XA_PRIMARY, XA_STRING, None,	owner, CurrentTime);
	XFlush(display);

	// Do not get any data, see how much data is there
	XGetWindowProperty(display, owner, XA_STRING, 0, 0, 0, AnyPropertyType, &type, &format, &len, &bytes_left, &data);

	// DATA is There
	if (bytes_left > 0)
	{
		result = XGetWindowProperty(display, owner, XA_STRING, 0, bytes_left, 0, AnyPropertyType, &type, &format, &len, &dummy, &data);
		if (result == Success)
		{
			printf("Clipboard: %s", data);
			snprintf(value, size - 1, "%s", data);
			XFree(data);
			return 0;
		}
		else
		{
			printf("FAIL\n");
			XFree(data);
			return -1;
		}
		
	}
	return -1;
}

// Send request to X11 (convert function) we want clipboard data, will receive clipboard event
int request_clipboard(Display *dpy, Window target_window)
{
    Window owner, root;
    int screen;
    Atom sel, target_property, utf8;
    XEvent ev;
    XSelectionEvent *sev;

    screen = DefaultScreen(dpy);
    root = RootWindow(dpy, screen);

    sel = XInternAtom(dpy, "CLIPBOARD", False);
    utf8 = XInternAtom(dpy, "UTF8_STRING", False);

    owner = XGetSelectionOwner(dpy, sel);
    if (owner == None)
    {
        printf("'CLIPBOARD' has no owner\n");
        return 1;
    }
    printf("0x%lX\n", owner);

    XSelectInput(dpy, target_window, SelectionNotify);

    // That's the property used by the owner. Note that it's completely arbitrary
    target_property = XInternAtom(dpy, "PENGUIN", False);

    // Request conversion to UTF-8. Not all owners will be able to fulfill that request.
    XConvertSelection(dpy, sel, utf8, target_property, target_window, CurrentTime);
    return 0;
}

#endif
