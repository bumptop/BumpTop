//
//  CMCrashReporterController.mm
//  CMCrashReporterModule
//
//  Created by Anning on 12/9/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "CMCrashReporterController.h"
#import <CMCrashReporter/CMCrashReporter.h>

static CMCrashReporterController *singleton_ = NULL;

@implementation CMCrashReporterController

- (void)startCMCrashReporter {
  if ([CMCrashReporter check]) {
    ProcessSerialNumber bumptop_process_serial_number;
    GetCurrentProcess(&bumptop_process_serial_number);
    SetFrontProcess(&bumptop_process_serial_number);
    timer_ = [[NSTimer scheduledTimerWithTimeInterval:0.1
                                               target:self
                                             selector:@selector(quitIfCMCrashReporterCompleted:)
                                             userInfo:NULL
                                              repeats:YES] retain];
  } else {
    [[NSApplication sharedApplication] terminate:NSApp];
  }
}

- (void)quitIfCMCrashReporterCompleted:(NSTimer *)time {
  if([CMCrashReporter isTaskCompleted]) {
    [timer_ invalidate];
    [timer_ release];
    timer_ = NULL;
    [[NSApplication sharedApplication] terminate:NSApp];
  }
}

+(CMCrashReporterController*)singleton {
  if (singleton_ == NULL) {
    singleton_ = [[CMCrashReporterController alloc] init];
  }
  return singleton_;
}

@end
