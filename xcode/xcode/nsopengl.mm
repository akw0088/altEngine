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
 
    
    
    altEngine.init; // gets pointer to implementation
    altEngine.engine_init;
    altEngine.step;
    
    
    
    
    NSTimer *timer;
    timer = [NSTimer scheduledTimerWithTimeInterval:1
                                             target:self
                                           selector:@selector(render)
                                           userInfo:nil repeats:YES];
    
    
    
    glClearColor(0.5, 0.5, 0.5, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    [[self openGLContext] flushBuffer];
    
    
}

-(void) render
{
    float width = [self frame].size.width;
    float height = [self frame].size.height;
    
    glViewport(0, 0, width, height);
    
    altEngine.step;
    altEngine.render;
    
    [[self openGLContext] flushBuffer];
}


- (void)reshape
{
    float width = [self frame].size.width;
    float height = [self frame].size.height;
    
    glViewport(0, 0, width, height);
    
    //Why God
    [altEngine resize: width height: height];
    
}



@end