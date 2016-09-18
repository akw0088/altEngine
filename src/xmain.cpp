#include "include.h"

#ifndef WIN32
#include <stdarg.h>

int EventProc(Display *display, Window window, GLXContext context);

int main(int argc, char *argv[])
{
	Display			*display;
	XVisualInfo		*vinfo;
	XSetWindowAttributes	winattrs;
	Window			window;
	GLXContext		context;
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

	if (display == NULL)
	{
		altEngine.render(16);
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
		printf("ButtonPress\n");
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
		printf("ButtonRelease\n");
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
		printf("MotionNotify\n");

		if ((event.xmotion.x == xcenter) && (event.xmotion.y == ycenter))
			break;

		if ( altEngine.mousepos(event.xmotion.x, event.xmotion.y, event.xmotion.x - xcenter, event.xmotion.y - ycenter) )
		{
			XWarpPointer(display, None, window, 0, 0, 0, 0, xcenter, ycenter);
		}
		break;
	case KeyPress:
	case KeyRelease:
		printf("KeyPress\n");
		{
			bool pressed = (event.type == KeyPress) ? true : false;
			KeySym keysym = XKeycodeToKeysym(display, event.xkey.keycode, 0);

			switch (keysym)
			{
			case XK_Return:
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
			case XK_Left:
				altEngine.keypress("left", pressed);
				break;
			case XK_Down:
				altEngine.keypress("down", pressed);
				break;
			case XK_Right:
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

char *get_file(char *filename)
{
        FILE    *file;
        char    *buffer;
        int     file_size, bytes_read;

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

int debugf(const char *format, ...)
{
        va_list args;
        char str[512] = { 0 };


        va_start(args, format);
        vsprintf(str, format, args);
        va_end(args);
        printf("%s", str);


        unsigned int width = 60;

        char *pstr = str;
        while (1)
        {
                if (strlen(pstr) < width)
                {
                        int size = strlen(pstr) + 1;
                        char *line = new char[size];
                        memcpy(line, pstr, size);
                        Menu::console_buffer.push_back(line);
                        break;
                }
                else
                {
                        int size = width + 1;
                        char *line = new char[size];
                        memcpy(line, pstr, size);
                        line[width] = '\0';
                        Menu::console_buffer.push_back(line);
                        pstr += width;
                }
        }

        return 0;
}


#endif
