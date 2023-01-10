//
//  MGLModule.h
//  RCTMGL
//
//  Created by Nick Italiano on 8/23/17.
//  Copyright © 2017 Mapbox Inc. All rights reserved.
//
#ifdef __cplusplus
#import "consider-it-rn-core.h"
#endif

#import <Foundation/Foundation.h>
#import <React/RCTBridgeModule.h>
#import <React/RCTEventEmitter.h>

@interface MGLModule : RCTEventEmitter<RCTBridgeModule>
@end
