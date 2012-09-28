#include "include.h"

//eliminate these globals
AGLContext	aglContext;
int		WindowVisible;
Engine		altEngine;

void			DisplayFunc(void);
static pascal OSStatus	EventHandler(EventHandlerCallRef nextHandler, EventRef event, void *userData);

int main(int  argc, char *argv[])
{
	AGLPixelFormat		format;		/* OpenGL pixel format */
	WindowPtr		window;		/* Window */
	int			winattrs;	/* Window attributes */
	Str255			title;		/* Title of window */
	Rect			rect;		/* Rectangle definition */
	EventHandlerUPP		handler;	/* Event handler */
	EventLoopTimerUPP	thandler;	/* Timer handler */
	EventLoopTimerRef	timer;		/* Timer for animating the window */
	ProcessSerialNumber	psn;		/* Process serial number */

	static EventTypeSpec	events[] =	/* Events we are interested in... */
			{
			  { kEventClassMouse, kEventMouseDown },
			  { kEventClassMouse, kEventMouseUp },
			  { kEventClassMouse, kEventMouseMoved },
			  { kEventClassMouse, kEventMouseDragged },
			  { kEventClassWindow, kEventWindowDrawContent },
			  { kEventClassWindow, kEventWindowShown },
			  { kEventClassWindow, kEventWindowHidden },
			  { kEventClassWindow, kEventWindowActivated },
			  { kEventClassWindow, kEventWindowDeactivated },
			  { kEventClassWindow, kEventWindowClose },
			  { kEventClassWindow, kEventWindowBoundsChanged },
			  { kCoreEventClass, kAEOpenApplication }
			};
	
	static GLint 		attributes[] =	/* OpenGL attributes */
			{
			  AGL_RGBA,
			  AGL_GREEN_SIZE, 1,
			  AGL_DOUBLEBUFFER,
			  AGL_DEPTH_SIZE, 16,
			  AGL_NONE
			};

	//Set initial values for window
	const int		        origWinHeight = 628;
	const int		        origWinWidth  = 850;
	const int		        origWinXOffset = 50;
	const int		        origWinYOffset = 50;


	// Create the window...

	aglContext = 0;
	WindowVisible = 0;

	SetRect(&rect, origWinXOffset, origWinYOffset, origWinWidth, origWinHeight);

	winattrs =	kWindowStandardHandlerAttribute | kWindowCloseBoxAttribute |
			kWindowCollapseBoxAttribute | kWindowFullZoomAttribute |
			kWindowResizableAttribute | kWindowLiveResizeAttribute;
	winattrs &= GetAvailableWindowAttributes(kDocumentWindowClass);

	strcpy(title + 1, "altEngine2");
	title[0] = strlen(title + 1);

	CreateNewWindow(kDocumentWindowClass, winattrs, &rect, &window);
	SetWTitle(window, title);

	handler = NewEventHandlerUPP(EventHandler);
	InstallWindowEventHandler(window, handler, sizeof(events) / sizeof(events[0]), events, NULL, 0L);
	thandler = NewEventLoopTimerUPP((void (*)(EventLoopTimerRef, void *))IdleFunc);
	InstallEventLoopTimer(GetMainEventLoop(), 0, 0, thandler, 0, &timer);

	GetCurrentProcess(&psn);
	SetFrontProcess(&psn);

	DrawGrowIcon(window);
	ShowWindow(window);

	// Create the OpenGL context and bind it to the window...
	format     = aglChoosePixelFormat(NULL, 0, attributes);
	aglContext = aglCreateContext(format, NULL);

	if (aglContext == NULL)
	{
		printf("Unable to create OpenGL context.\n");
		return 1;
	}

	aglDestroyPixelFormat(format);
	aglSetDrawable(aglContext, GetWindowPort(window));

	// Set the initial size of the cube
	altEngine.init((void *)window, NULL);
	altEngine.resize(origWinWidth - origWinXOffset, origWinHeight - origWinYOffset);


	for (;;)
	{
		if (WindowVisible)
			SetEventLoopTimerNextFireTime(timer, 0.05);
	
		RunApplicationEventLoop();
	
		if (WindowVisible)
		{
			//render frame, must pass a message to event handler
			altEngine.render();
		}
	}
}

static pascal OSStatus EventHandler(
	EventHandlerCallRef nextHandler,	/* I - Next handler to call */
	EventRef            event,		/* I - Event reference */
	void                *userData)	/* I - User data (not used) */
{
	UInt32			kind;			/* Kind of event */
	Rect			rect;			/* New window size */
	EventMouseButton	button;			/* Mouse button */
	Point			point;			/* Mouse position */


	kind = GetEventKind(event);

	switch (kind)
	{
	case kEventWindowDrawContent:
		break;

	case kEventWindowBoundsChanged:
		GetEventParameter(event, kEventParamCurrentBounds, typeQDRectangle, NULL, sizeof(Rect), NULL, &rect);

		if (aglContext)
			aglUpdateContext(aglContext);

		altEngine.resize(rect.right - rect.left, rect.bottom - rect.top);
		break;

	case kEventWindowShown:
		WindowVisible = 1;
		break;

	case kEventWindowHidden:
		WindowVisible = 0;
		break;

	case kEventMouseDown:
		GetEventParameter(event, kEventParamMouseButton, typeMouseButton, NULL, sizeof(EventMouseButton), NULL, &button);
		GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, NULL, sizeof(Point), NULL, &point);
		//MouseFunc(button, 0, point.h, point.v);
		break;

	case kEventMouseUp:
		GetEventParameter(event, kEventParamMouseButton, typeMouseButton, NULL, sizeof(EventMouseButton), NULL, &button);
		GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, NULL, sizeof(Point), NULL, &point);
		//MouseFunc(button, 1, point.h, point.v);
		break;

	case kEventMouseDragged:
		GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, NULL, sizeof(Point), NULL, &point);
		//MotionFunc(point.h, point.v);
		break;

	case kEventWindowClose:
		altEngine.destroy();
		ExitToShell();
		break;
	default:
		return CallNextEventHandler(nextHandler, event);
	}


	return noErr;
}

