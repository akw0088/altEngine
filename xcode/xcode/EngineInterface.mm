//
//  EngineInterface.m
//  xcode
//
//  Created by Wright, Alexander (CONT) on 9/18/16.
//  Copyright © 2016 Wright, Alexander (CONT). All rights reserved.
//

#import <Foundation/Foundation.h>
#import "EngineInterface.h"

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
    altEngine->init(NULL, NULL);
}

- (void)step
{
    altEngine->step();
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
    altEngine->keystroke(key);
}


@end