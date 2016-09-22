#import <Foundation/Foundation.h>

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

- (void)mouseMoved:(NSEvent *)theEvent
{
    NSPoint pos;
    NSPoint delta;

    //Origin is lower left, we get mouse messages outside of NSView bounds
    pos = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    delta.x = [self bounds].size.width / 2 - pos.x;
    delta.y = [self bounds].size.height / 2 - pos.y;
    [altEngine mousepos: pos.x y: pos.y deltax: pos.x deltay: delta.y];
//  printf("%3.3lf %3.3lf %3.3lf %3.3lf\n", pos.x, pos.y, delta.x, delta.y);
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
    
    [altEngine engine_init];
    
    NSTimer *timer;
    
    //create 16ms timer for time step
    timer = [NSTimer scheduledTimerWithTimeInterval:0.016
                     target:self
                     selector:@selector(step)
                     userInfo:nil
                     repeats:YES];
    
    // still need proper render loop
}

-(BOOL) acceptsFirstResponder
{
    return YES;
}

-(void) render
{
    [altEngine render];
    [[self openGLContext] flushBuffer];
}

-(void) step
{
    [altEngine step];
    [altEngine render];
    [[self openGLContext] flushBuffer];
}


- (void) reshape
{
    float width = [self frame].size.width;
    float height = [self frame].size.height;
    
    //Why God
    [altEngine resize: width height: height];
}


- (void)mouseDown:(NSEvent *)theEvent
{
    //CGWarpMouseCursorPosition(center);
    [altEngine keypress: "mouse1" pressed:true];
}

- (void)mouseUp:(NSEvent *)theEvent
{
    [altEngine keypress: "mouse1" pressed:false];
}

- (void)keyDown:(NSEvent *)theEvent
{
    switch (theEvent.keyCode)
    {
        case kVK_Return:
            [altEngine keypress: "enter" pressed:true];
            [altEngine keystroke: '\r'];
            break;
        case kVK_Shift:
            [altEngine keypress: "shift" pressed:true];
            break;
        case kVK_UpArrow:
            [altEngine keypress: "up" pressed:true];
            break;
        case kVK_DownArrow:
            [altEngine keypress: "down" pressed:true];
            break;
        case kVK_LeftArrow:
            [altEngine keypress: "left" pressed:true];
            break;
        case kVK_RightArrow:
            [altEngine keypress: "right" pressed:true];
            break;
        case kVK_ANSI_Keypad0:
            [altEngine keypress: "num0" pressed:true];
            break;
        case kVK_ANSI_Keypad1:
            [altEngine keypress: "num1" pressed:true];
            break;
        case kVK_ANSI_Keypad2:
            [altEngine keypress: "num2" pressed:true];
            break;
        case kVK_ANSI_Keypad3:
            [altEngine keypress: "num3" pressed:true];
            break;
        case kVK_ANSI_Keypad4:
            [altEngine keypress: "num4" pressed:true];
            break;
        case kVK_ANSI_Keypad5:
            [altEngine keypress: "num5" pressed:true];
            break;
        case kVK_ANSI_Keypad6:
            [altEngine keypress: "num6" pressed:true];
            break;
        case kVK_ANSI_Keypad7:
            [altEngine keypress: "num7" pressed:true];
            break;
        case kVK_ANSI_Keypad8:
            [altEngine keypress: "num8" pressed:true];
            break;
        case kVK_ANSI_Keypad9:
            [altEngine keypress: "num9" pressed:true];
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
            [altEngine keypress: "enter" pressed:false];
            break;
        case kVK_Shift:
            [altEngine keypress: "shift" pressed:false];
            break;
        case kVK_UpArrow:
            [altEngine keypress: "up" pressed:false];
            break;
        case kVK_DownArrow:
            [altEngine keypress: "down" pressed:false];
            break;
        case kVK_LeftArrow:
            [altEngine keypress: "left" pressed:false];
            break;
        case kVK_RightArrow:
            [altEngine keypress: "right" pressed:false];
            break;
        case kVK_ANSI_Keypad0:
            [altEngine keypress: "num0" pressed:false];
            break;
        case kVK_ANSI_Keypad1:
            [altEngine keypress: "num1" pressed:false];
            break;
        case kVK_ANSI_Keypad2:
            [altEngine keypress: "num2" pressed:false];
            break;
        case kVK_ANSI_Keypad3:
            [altEngine keypress: "num3" pressed:false];
            break;
        case kVK_ANSI_Keypad4:
            [altEngine keypress: "num4" pressed:false];
            break;
        case kVK_ANSI_Keypad5:
            [altEngine keypress: "num5" pressed:false];
            break;
        case kVK_ANSI_Keypad6:
            [altEngine keypress: "num6" pressed:false];
            break;
        case kVK_ANSI_Keypad7:
            [altEngine keypress: "num7" pressed:false];
            break;
        case kVK_ANSI_Keypad8:
            [altEngine keypress: "num8" pressed:false];
            break;
        case kVK_ANSI_Keypad9:
            [altEngine keypress: "num9" pressed:false];
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
    [altEngine keypress: "up" pressed:true];
}

-(IBAction)moveDown:(id)sender
{
    [altEngine keypress: "down" pressed:true];
}

-(IBAction)moveLeft:(id)sender
{
    [altEngine keypress: "left" pressed:true];
}

-(IBAction)moveRight:(id)sender
{
    [altEngine keypress: "right" pressed:true];
}


@end
