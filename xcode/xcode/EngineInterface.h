//
//  EngineInterface.h
//  xcode
//
//  Created by Wright, Alexander (CONT) on 9/18/16.
//  Copyright © 2016 Wright, Alexander (CONT). All rights reserved.
//

#ifndef EngineInterface_h
#define EngineInterface_h

#define XCODE
#import <OpenAL/al.h>
#include "../../include/include.h"


@interface EngineInterface : NSObject {
    Engine *altEngine;
}
- (id) init;
- (void) dealloc;
- (void)engine_init;
- (void)step;
- (void)render;
- (void)resize:(int)x height:(int)y;
- (void)keypress:(char *)key pressed:(bool)press;
@end


#endif /* EngineInterface_h */
