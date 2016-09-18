//
//  opengl.m
//  altEngine
//
//  Created by Wright, Alexander (CONT) on 9/18/16.
//  Copyright © 2016 Wright, Alexander (CONT). All rights reserved.
//

#import <Foundation/Foundation.h>

#import "opengl.h"
#include <OpenGL/gl.h>

@implementation OpenGLView

-(void) drawRect: (NSRect) bounds
{
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

@end