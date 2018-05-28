#import <Foundation/Foundation.h>
#import "EngineInterface.h"

unsigned int com_maxfps;

@implementation EngineInterface
- (id)init
{
    if(self = [super init])
    {
        altEngine = new Engine();
    }
    return self;
}
- (void) dealloc
{
//    if(altEngine != NULL) delete altEngine;
//    [super dealloc];
}

- (void)engine_init
{
    altEngine->init(NULL, NULL, NULL);
}

- (void)step
{
    static int tick_num;
    altEngine->step(tick_num++);
}

- (void)resize:(int)x height:(int)y
{
    altEngine->resize(x,y);
}


- (void)render
{
    altEngine->render(16.0);
}

- (void)keypress:(char *)key pressed:(bool)press
{
    altEngine->keypress(key, press);
}

- (void)keystroke:(char)key
{
    altEngine->keystroke(key, NULL);
}

- (void)mousepos:(int)xpos y:(bool)ypos deltax:(int)xd deltay:(int)yd;
{
    altEngine->mousepos(xpos, ypos, xd, yd);
}


@end
