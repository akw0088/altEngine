#import <Foundation/Foundation.h>

#import "nsopengl.h"
#include <OpenGL/gl.h>
#define XCODE
#include "../../include/include.h"
#import "EngineInterface.h"


@implementation opengl


EngineInterface *altEngine = [EngineInterface alloc];


-(void) drawRect: (NSRect) bounds
{
    float width = [self frame].size.width;
    float height = [self frame].size.height;
    
    glViewport(0, 0, width, height);
    
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    drawAnObject();
    glFlush();
}

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

- (void)awakeFromNib
{
    // 1. Create a context with opengl pixel format
    NSOpenGLPixelFormatAttribute pixelFormatAttributes[] =
    {
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
        NSOpenGLPFAColorSize    , 32,
        NSOpenGLPFAAlphaSize    , 8,
        NSOpenGLPFADoubleBuffer ,
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


- (void)reshape
{
    float width = [self frame].size.width;
    float height = [self frame].size.height;
    
    //Why God
    [altEngine resize: width height: height];
    
}



@end