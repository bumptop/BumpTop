//
//  CMCrashReporter.h
//  CMCrashReporter-App
//
//  Created by Jelle De Laender on 20/01/08.
//  Copyright 2008 CodingMammoth. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "CMCrashReporterGlobal.h"
#import "CMCrashReporterWindow.h"

@interface CMCrashReporter : NSObject
{

}
+ (BOOL)check;
+ (NSMutableArray *)getReports;
+ (BOOL)isTaskCompleted;
+ (void)onTaskComplete;

BOOL is_task_completed_;
@end
