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
- (void)mouseDown:(NSEvent *)theEvent;
- (void)mouseUp:(NSEvent *)theEvent;
- (void)keyDown:(NSEvent *)theEvent;
-(IBAction)moveUp:(id)sender;
-(IBAction)moveDown:(id)sender;
-(IBAction)moveLeft:(id)sender;
-(IBAction)moveRight:(id)sender;
@end

#endif /* nsopengl_h */
