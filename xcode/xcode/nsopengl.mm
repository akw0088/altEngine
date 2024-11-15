#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <CoreGraphics/CoreGraphics.h>

#import "nsopengl.h"
#include <OpenGL/gl.h>
#include <Carbon/Carbon.h>



#include "include.h"
#import "EngineInterface.h"


@implementation opengl


EngineInterface *altEngine = [EngineInterface alloc];

- (void) drawRect: (NSRect) bounds;
{
    [[self openGLContext] makeCurrentContext];
    //Perform drawing here
    [[self openGLContext] flushBuffer];
/*
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    drawAnObject();
    glFlush();
 */
}

//Basic triangle test
static void drawAnObject ()
{
    glColor3f(1.0f, 0.85f, 0.35f);
    glBegin(GL_TRIANGLES);
    {
        glVertex3f(  0.0,  0.6, 0.0);
        glVertex3f( -0.2, -0.3, 0.0);
        glVertex3f(  0.2, -0.3 ,0.0);
    }
    glEnd();
}



-(void) viewDidMoveToWindow
{
    [super viewDidMoveToWindow];
    if ([self window] == nil)
        [[self openGLContext] clearDrawable];
}



// NSWindow - entire window area, with title bar
// NSScreen - This is the entire desktop area (monitor resolution)
// NSView - This is the content area of a window
// Frame -- a frame is just the rectangle defining the object
- (void)mouseMoved:(NSEvent *)theEvent
{
    NSPoint pos;
   
    //Origin is lower left, we get mouse messages outside of NSView bounds
    pos = [theEvent locationInWindow];

    
    // clamp pos to view bounds
    if (pos.x >= self.window.contentView.frame.size.width)
    {
        pos.x = self.window.contentView.frame.size.width - 1;
    }

    if (pos.y >= self.window.contentView.frame.size.height)
    {
        pos.y = self.window.contentView.frame.size.height - 1;
    }
    
    if (pos.x < 0)
    {
        pos.x = 0;
    }

    if (pos.y < 0)
    {
        pos.y = 0;
    }
    
    
    // flip so zero is upper left corner instead of lower left
    pos.y = self.window.contentView.frame.size.height - pos.y - 1;

    // get center of window
    NSPoint window_center_screen = NSMakePoint(NSMidX(self.window.frame), NSMidY(self.window.frame));
    NSPoint window_center = [self.window convertPointFromScreen:window_center_screen];
    
    
    NSPoint delta;
    
    // calculate delta from window center
    delta.x = pos.x - window_center.x;
    delta.y = pos.y - window_center.y;
    
//    NSLog(@"Pos: %@", NSStringFromPoint(pos));
//    NSLog(@"Delta: %@", NSStringFromPoint(delta));

    int flag = [altEngine mousepos: pos.x y: pos.y deltax: delta.x deltay: delta.y];
    
    if (flag)
    {
        NSPoint center;
        
        center.x = self.window.frame.origin.x + self.window.frame.size.width  / 2;
        center.y = (self.window.screen.frame.size.height - 1) - (self.window.frame.origin.y + self.window.frame.size.height  / 2);
        
        
        //Cocoa uses a coordinate space where the origin (0, 0) is the bottom-left of the primary display and increasing y goes up.
        
        //Quartz uses a coordinate space where the origin (0, 0) is at the top-left of the primary display. Increasing y goes down.
        
        // CG uses Quartz
        CGWarpMouseCursorPosition(center);
        CGDisplayHideCursor(kCGDirectMainDisplay);
    }
    else
    {
        CGDisplayShowCursor(kCGDirectMainDisplay);
    }

}

- (void)awakeFromNib
{
    // 1. Create a context with opengl pixel format
    NSOpenGLPixelFormatAttribute pixelFormatAttributes[] =
    {
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
        NSOpenGLPFAColorSize    , 24,
        NSOpenGLPFADepthSize    , 24,
        NSOpenGLPFAStencilSize  ,  8,
        NSOpenGLPFAAlphaSize    ,  8,
        NSOpenGLPFADoubleBuffer ,
        NSOpenGLPFASupersample  ,
        NSOpenGLPFAAccelerated  ,
        0
    };
    NSOpenGLPixelFormat *pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:pixelFormatAttributes];
    super.pixelFormat = pixelFormat;
    
    // 2. Make the context current
    [[self openGLContext] makeCurrentContext];
    [[self window] setAcceptsMouseMovedEvents:YES];
    
    id ret = [altEngine init]; // gets pointer to implementation (pimpl)
    
    
//    NSOpenGLContext    *currentContext = [self openGLContext];
  
    
        
    [altEngine engine_init];
    
    NSTimer *timer_step;
    NSTimer *timer_render;
    
    //create 16ms timer for time step
    timer_step = [NSTimer scheduledTimerWithTimeInterval:0.008
                     target:self
                     selector:@selector(step)
                     userInfo:nil
                     repeats:YES];
    
    // still need proper render loop
    timer_render = [NSTimer scheduledTimerWithTimeInterval:0.001
                     target:self
                     selector:@selector(render)
                     userInfo:nil
                     repeats:YES];
}

-(BOOL) acceptsFirstResponder
{
    return YES;
}

-(void) render
{
    [altEngine render];
//    [[self openGLContext] flushBuffer];
}

-(void) step
{
    [altEngine step];
}


- (void) reshape
{
//    float width = [self frame].size.width;
//    float height = [self frame].size.height;
    NSRect pixelsize = [self convertRect:[self bounds] toView:nil];
    
    
    //Why God
    [altEngine resize: pixelsize.size.width height: pixelsize.size.height];
}


- (void)mouseDown:(NSEvent *)theEvent
{
    [altEngine keypress: (char *)"leftbutton" pressed:true];
}

- (void)mouseUp:(NSEvent *)theEvent
{
    [altEngine keypress: (char *)"leftbutton" pressed:false];
}

- (void)rightMouseDown:(NSEvent *)theEvent
{
    [altEngine keypress: (char *)"rightButton" pressed:true];
}

- (void)rightMouseUp:(NSEvent *)theEvent
{
    [altEngine keypress: (char *)"rightButton" pressed:false];
}

- (void)otherMouseDown:(NSEvent *)theEvent
{
    [altEngine keypress: (char *)"middleButton" pressed:true];
}

- (void)otherMouseUp:(NSEvent *)theEvent
{
    [altEngine keypress: (char *)"middleButton" pressed:false];
}


- (void)keyDown:(NSEvent *)theEvent
{
    switch (theEvent.keyCode)
    {
        case kVK_Return:
            [altEngine keypress: (char *)"enter" pressed:true];
            [altEngine keystroke: '\r'];
            break;
        case kVK_Shift:
            [altEngine keypress: (char *)"shift" pressed:true];
            break;
        case kVK_UpArrow:
            [altEngine keypress: (char *)"up" pressed:true];
            break;
        case kVK_DownArrow:
            [altEngine keypress: (char *)"down" pressed:true];
            break;
        case kVK_LeftArrow:
            [altEngine keypress: (char *)"left" pressed:true];
            break;
        case kVK_RightArrow:
            [altEngine keypress: (char *)"right" pressed:true];
            break;
        case kVK_ANSI_Keypad0:
            [altEngine keypress: (char *)"num0" pressed:true];
            break;
        case kVK_ANSI_Keypad1:
            [altEngine keypress: (char *)"num1" pressed:true];
            break;
        case kVK_ANSI_Keypad2:
            [altEngine keypress: (char *)"num2" pressed:true];
            break;
        case kVK_ANSI_Keypad3:
            [altEngine keypress: (char *)"num3" pressed:true];
            break;
        case kVK_ANSI_Keypad4:
            [altEngine keypress: (char *)"num4" pressed:true];
            break;
        case kVK_ANSI_Keypad5:
            [altEngine keypress: (char *)"num5" pressed:true];
            break;
        case kVK_ANSI_Keypad6:
            [altEngine keypress: (char *)"num6" pressed:true];
            break;
        case kVK_ANSI_Keypad7:
            [altEngine keypress: (char *)"num7" pressed:true];
            break;
        case kVK_ANSI_Keypad8:
            [altEngine keypress: (char *)"num8" pressed:true];
            break;
        case kVK_ANSI_Keypad9:
            [altEngine keypress: (char *)"num9" pressed:true];
            break;
        case kVK_ANSI_Grave:
            [altEngine keypress: (char *)"~" pressed:true];
            break;
        default:
            // ignore unsupported keys
            break;
    }
    
    [self interpretKeyEvents:[NSArray arrayWithObject:theEvent]];
}


- (void)keyUp:(NSEvent *)theEvent
{
    switch (theEvent.keyCode)
    {
        case kVK_Return:
            [altEngine keypress: (char *)"enter" pressed:false];
            break;
        case kVK_Shift:
            [altEngine keypress: (char *)"shift" pressed:false];
            break;
        case kVK_UpArrow:
            [altEngine keypress: (char *)"up" pressed:false];
            break;
        case kVK_DownArrow:
            [altEngine keypress: (char *)"down" pressed:false];
            break;
        case kVK_LeftArrow:
            [altEngine keypress: (char *)"left" pressed:false];
            break;
        case kVK_RightArrow:
            [altEngine keypress: (char *)"right" pressed:false];
            break;
        case kVK_ANSI_Keypad0:
            [altEngine keypress: (char *)"num0" pressed:false];
            break;
        case kVK_ANSI_Keypad1:
            [altEngine keypress: (char *)"num1" pressed:false];
            break;
        case kVK_ANSI_Keypad2:
            [altEngine keypress: (char *)"num2" pressed:false];
            break;
        case kVK_ANSI_Keypad3:
            [altEngine keypress: (char *)"num3" pressed:false];
            break;
        case kVK_ANSI_Keypad4:
            [altEngine keypress: (char *)"num4" pressed:false];
            break;
        case kVK_ANSI_Keypad5:
            [altEngine keypress: (char *)"num5" pressed:false];
            break;
        case kVK_ANSI_Keypad6:
            [altEngine keypress: (char *)"num6" pressed:false];
            break;
        case kVK_ANSI_Keypad7:
            [altEngine keypress: (char *)"num7" pressed:false];
            break;
        case kVK_ANSI_Keypad8:
            [altEngine keypress: (char *)"num8" pressed:false];
            break;
        case kVK_ANSI_Keypad9:
            [altEngine keypress: (char *)"num9" pressed:false];
            break;
        case kVK_ANSI_Grave:
            [altEngine keypress: (char *)"~" pressed:false];
            break;
        default:
            // ignore unsupported keys
            break;
    }


    [self interpretKeyEvents:[NSArray arrayWithObject:theEvent]];
}

- (void)insertText:(char *) str
{
    [altEngine keystroke: *str];
}



-(IBAction)moveUp:(id)sender
{
    [altEngine keypress: (char *)"up" pressed:true];
}

-(IBAction)moveDown:(id)sender
{
    [altEngine keypress: (char *)"down" pressed:true];
}

-(IBAction)moveLeft:(id)sender
{
    [altEngine keypress: (char *)"left" pressed:true];
}

-(IBAction)moveRight:(id)sender
{
    [altEngine keypress: (char *)"right" pressed:true];
}


@end
