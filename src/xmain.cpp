//=============================================================================
// This file is part of the altEngine distribution
// (https://github.com/akw0088/altEngine/)
// Copyright (c) 2018 Alexander Wright All Rights Reserved.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON - INFRINGEMENT.IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//=============================================================================

#include "include.h"

#ifdef __linux
#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <signal.h>
#include <linux/input.h>
#include <linux/joystick.h>

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
		strncat(cmdline, argv[i], sizeof(cmdline) - strlen(cmdline) - 1);
		strncat(cmdline, " ", sizeof(cmdline) - strlen(cmdline) - 1);
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

	XSetStandardProperties(display, window, "altEngine2", "altEngine2", None, argv, argc, NULL );

	if ( glXMakeCurrent(display, window, context) )
	{
		printf("GL Version: %s\n", glGetString(GL_VERSION));
	}

	Atom wmDelete = XInternAtom(display, "WM_DELETE_WINDOW", True);
	XSetWMProtocols(display, window, &wmDelete, 1);

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

        //XSelectInput(display, window, StructureNotifyMask);
        //XSelectInput(display, window, StructureNotifyMask + ExposureMask);
        //XSetSelectionOwner(display, XA_PRIMARY, window, CurrentTime);
        //XFlush(display);


	if (display == NULL)
	{
#ifndef DEDICATED
		struct timeval start;
		struct timeval end;

		float value = end.tv_usec - start.tv_usec;
		gettimeofday(&start, NULL);
		altEngine.render(value / 1000.0f);
		gettimeofday(&end, NULL);
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
		xcenter = event.xconfigure.width / 2;
		ycenter = event.xconfigure.height / 2;
		altEngine.resize(event.xconfigure.width, event.xconfigure.height);
		break;
	case MapNotify:
		printf("MapNotify\n");
		XMoveResizeWindow(display, window, 0, 0, 1920, 1080);
		altEngine.resize(1920, 1080);
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

			bool pressed = (event.type == KeyPress);
			KeySym keysym = XkbKeycodeToKeysym(display, event.xkey.keycode, 0, 0);

			count = XLookupString((XKeyEvent *)&event, buffer, 128, &keysym, &compose);
			for(int i = 0; i < count; i++)
			{
				char character[2] = {0};

				character[0] = buffer[i];
				if (pressed)
				{
					//altEngine.keypress(character, pressed);
					altEngine.keystroke(character[0], character);
				}
			}

			switch (keysym)
			{
			case XK_Pause:
				altEngine.keypress("pause", pressed);
				break;
			case XK_Tab:
				altEngine.keypress("tab", pressed);
				break;
			case XK_Return:
			case XK_KP_Enter:
				altEngine.keypress("enter", pressed);
				break;
			case XK_space:
				altEngine.keypress("space", pressed);
				break;
			case XK_Shift_L:
			case XK_Shift_R:
				altEngine.keypress("shift", pressed);
				break;
			case XK_Control_L:
			case XK_Control_R:
				altEngine.keypress("control", pressed);
				break;
			case XK_Alt_L:
			case XK_Alt_R:
				altEngine.keypress("alt", pressed);
				break;
			case XK_Escape:
				altEngine.keypress("escape", pressed);
				break;
			case XK_Up:
				altEngine.keypress("up", pressed);
				break;
			case XK_Left:
				altEngine.keypress("left", pressed);
				break;
			case XK_Down:
				altEngine.keypress("down", pressed);
				break;
			case XK_Right:
				altEngine.keypress("right", pressed);
				break;
			case XK_KP_0:
				altEngine.keypress("numpad0", pressed);
				break;
			case XK_KP_1:
				altEngine.keypress("numpad1", pressed);
				break;
			case XK_KP_2:
				altEngine.keypress("numpad2", pressed);
				break;
			case XK_KP_3:
				altEngine.keypress("numpad3", pressed);
				break;
			case XK_KP_4:
				altEngine.keypress("numpad4", pressed);
				break;
			case XK_KP_5:
				altEngine.keypress("numpad5", pressed);
				break;
			case XK_KP_6:
				altEngine.keypress("numpad6", pressed);
				break;
			case XK_KP_7:
				altEngine.keypress("numpad7", pressed);
				break;
			case XK_KP_8:
				altEngine.keypress("numpad8", pressed);
				break;
			case XK_KP_9:
				altEngine.keypress("numpad9", pressed);
				break;
			case XK_F1:
				altEngine.keypress("f1", pressed);
				break;
			case XK_F2:
				altEngine.keypress("f2", pressed);
				break;
			case XK_F3:
				altEngine.keypress("f3", pressed);
				break;
			case XK_F4:
				altEngine.keypress("f4", pressed);
				break;
			case XK_F5:
				altEngine.keypress("f5", pressed);
				break;
			case XK_F6:
				altEngine.keypress("f6", pressed);
				break;
			case XK_F7:
				altEngine.keypress("f7", pressed);
				break;
			case XK_F8:
				altEngine.keypress("f8", pressed);
				break;
			case XK_F9:
				altEngine.keypress("f9", pressed);
				break;
			case XK_F10:
				altEngine.keypress("f10", pressed);
				break;
			case XK_F11:
				altEngine.keypress("f11", pressed);
				break;
			case XK_F12:
				altEngine.keypress("f12", pressed);
				break;
			case XK_Insert:
				altEngine.keypress("insert", pressed);
				break;
			case XK_Delete:
				altEngine.keypress("delete", pressed);
				break;
			case XK_Home:
				altEngine.keypress("home", pressed);
				break;
			case XK_End:
				altEngine.keypress("end", pressed);
				break;
			case XK_KP_Multiply:
				altEngine.keypress("numpad*", pressed);
				break;
			case XK_KP_Add:
				altEngine.keypress("numpad+", pressed);
				break;
			case XK_KP_Subtract:
				altEngine.keypress("numpad-", pressed);
				break;
			case XK_KP_Divide:
				altEngine.keypress("numpad/", pressed);
				break;
			case XK_KP_Decimal:
				altEngine.keypress("numpad.", pressed);
				break;
			case XK_Caps_Lock:
				altEngine.keypress("capslock", pressed);
				break;
			case XK_Prior:
				altEngine.keypress("pgup", pressed);
				break;
			case XK_Next:
				altEngine.keypress("pgdown", pressed);
				break;
			}

                        if ((keysym >= 'A' && keysym <= 'Z') || (keysym>= 'a' && keysym <= 'z') || (keysym >= '0' && keysym <= '9'))
                        {
                                char c[2];

                                c[0] = keysym;
                                c[1] = '\0';
                                altEngine.keypress(c, pressed);
                        }


		}
		break;
	case UnmapNotify:
		printf("UnmapNotify\n");
		break;
	case Expose:
		printf("Expose %d\n", event.xexpose.count);
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
	case ClientMessage:
		printf("Window manager close event\r\n");
		altEngine.destroy();
		XFreeCursor(display, cursor);
		XFreeCursor(display, invisibleCursor);
		XFreePixmap(display, bitmapNoData);
		return 1;
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
    Window owner;
    Atom sel, target_property, utf8;
    //XEvent ev;
    //XSelectionEvent *sev;

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

int joystick_open()
{
	int fd = open ("/dev/input/js0", O_RDONLY | O_NONBLOCK);
	if (fd < 0)
	{
		printf("Failed to open /dev/input/js0\n");
		return -1;
	}

	char num;
	int ver;
	char name[128];

	ioctl (fd, JSIOCGAXES, &num);
	printf("Number of axis: %d\n", num);
	ioctl (fd, JSIOCGBUTTONS, &num);
	printf("Number of buttons: %d\n", num);
	ioctl (fd, JSIOCGBUTTONS, &ver);
	printf("version: %X\n", ver);

	if (ioctl(fd, JSIOCGNAME(sizeof(name)), name) < 0)
	{
		strncpy(name, "Unknown", sizeof(name));
	}
	printf("Name: %s\n", name);

	return fd;
}


int joystick_read(int fd)
{
	int ret = -1;

	struct js_event e;
	ret = read (fd, &e, sizeof(e));
	if (ret == -1)
	{
		if (errno == EAGAIN)
		{
			return 0;
		}
		else
		{
			perror("read failed");
			return -1;
		}
	}
	switch(e.type)
	{
	case JS_EVENT_BUTTON:
		printf("Axis %d\n", e.number);
		printf("value %d\n", e.value);
		printf("time %d\n", e.time);
		break;
	case JS_EVENT_AXIS:
		printf("Axis %d\n", e.number);
		printf("value %d\n", e.value);
		printf("time %d\n", e.time);
		break;
	case JS_EVENT_INIT:
		printf("Axis %d\n", e.number);
		printf("value %d\n", e.value);
		printf("time %d\n", e.time);
		break;
	}
	return 0;
}	

void fullscreen(Display* display, Window window)
{
	XEvent e;
	e.xclient.type         = ClientMessage;
	e.xclient.window       = window;
	e.xclient.message_type = XInternAtom(display,"_NET_WM_STATE",False);
	e.xclient.format = 32;
	e.xclient.data.l[0] = 2;    // _NET_WM_STATE_TOGGLE
	e.xclient.data.l[1] = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", True);
	e.xclient.data.l[2] = 0;    // no second property to toggle
	e.xclient.data.l[3] = 1;
	e.xclient.data.l[4] = 0;

	XSendEvent(display, DefaultRootWindow(display), False, SubstructureRedirectMask | SubstructureNotifyMask, &e);
	XMoveResizeWindow(display, window, 0, 0, 1920, 1080);
}

#endif
