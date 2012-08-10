//
//  main.m
//  CMCrashReporterModule
//
//  Created by Anning on 12/9/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "CMCrashReporterController.h"

int main(int argc, char *argv[])
{
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  [[CMCrashReporterController singleton] startCMCrashReporter];
  [pool release];
    printf("I'm starting!!");
  return NSApplicationMain(argc,  (const char **) argv);
}
