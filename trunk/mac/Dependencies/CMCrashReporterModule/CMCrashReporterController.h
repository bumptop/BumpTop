//
//  CMCrashReporterController.h
//  CMCrashReporterModule
//
//  Created by Anning on 12/9/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface CMCrashReporterController : NSObject {

}

- (void)startCMCrashReporter;
- (void)quitIfCMCrashReporterCompleted:(NSTimer *)time;
+(CMCrashReporterController*)singleton;

NSTimer* timer_;

@end
