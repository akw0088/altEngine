#import <Foundation/Foundation.h>

#import "nsopengl.h"
#include <OpenGL/gl.h>

#include "include.h"
#import "EngineInterface.h"


@implementation opengl


EngineInterface *altEngine = [EngineInterface alloc];

- (void) drawRect: (NSRect) bounds;
{
    [[self openGLContext] makeCurrentContext];
    //Perform drawing here
    [[self openGLContext] flushBuffer];
}

-(void) viewDidMoveToWindow
{
    [super viewDidMoveToWindow];
    if ([self window] == nil)
        [[self openGLContext] clearDrawable];
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

-(void) render
{
    [altEngine render];
    [[self openGLContext] flushBuffer];
}

-(void) step
{
    [altEngine step];
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
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
    [altEngine keypress: "mouse1" pressed:true];
}

- (void)mouseUp:(NSEvent *)theEvent
{
    [altEngine keypress: "mouse1" pressed:false];
}

- (void)keyDown:(NSEvent *)theEvent
{
    [self interpretKeyEvents:[NSArray arrayWithObject:theEvent]];
}


- (void)keyUp:(NSEvent *)theEvent
{

    [self interpretKeyEvents:[NSArray arrayWithObject:theEvent]];
}

- (void)insertText:(char *) str
{
    [altEngine keystroke: str[0]];
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