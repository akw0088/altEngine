#ifndef nsopengl_h
#define nsopengl_h

#import <Cocoa/Cocoa.h>

@interface opengl : NSOpenGLView
{
}
- (void) drawRect: (NSRect) bounds;
- (void) reshape;
- (void) render;
- (void) step;
- (void) viewDidMoveToWindow;
- (void) awakeFromNib;
@end

#endif /* nsopengl_h */
