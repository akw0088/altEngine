//
//  EngineInterface.m
//  xcode
//
//  Created by Wright, Alexander (CONT) on 9/18/16.
//  Copyright © 2016 Wright, Alexander (CONT). All rights reserved.
//

#import <Foundation/Foundation.h>
#define XCODE
#import <OpenAL/al.h>
#include "../../include/include.h"

@interface EngineInterface : NSObject {
    Engine *altEngine;
}
@end

@implementation EngineInterface
- (id)init {
    if(self = [super init]) {
        altEngine = new Engine();
    }
    return self;
}
- (void) dealloc {
//    if(altEngine != NULL) delete altEngine;
//    [super dealloc];
}

- (void)step {
    altEngine->step();
}


- (void)render {
    altEngine->render(16.0);
}
@end