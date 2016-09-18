#import <Foundation/Foundation.h>

#import "nsopengl.h"
#include <OpenGL/gl.h>
#define XCODE
#include "../../include/include.h"



@implementation opengl

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
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersionLegacy,
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
    
    NSTimer *timer;
    timer = [NSTimer scheduledTimerWithTimeInterval:1
                                             target:self
                                           selector:@selector(render)
                                           userInfo:nil repeats:YES];
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    glClearColor(0.5, 0.5, 0.5, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    [[self openGLContext] flushBuffer];
    
    
}

-(void) render
{
    float width = [self frame].size.width;
    float height = [self frame].size.height;
    
    glViewport(0, 0, width, height);
    
    glClear(GL_COLOR_BUFFER_BIT);
    drawAnObject();
    glFlush();
    
    [[self openGLContext] flushBuffer];
}


- (void)reshape
{
    float width = [self frame].size.width;
    float height = [self frame].size.height;
    
    glViewport(0, 0, width, height);
}



@end