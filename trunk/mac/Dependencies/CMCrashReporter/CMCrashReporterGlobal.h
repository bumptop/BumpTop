//
//  CMCrashReporterGlobal.h
//  CMCrashReporter-App
//
//  Created by Jelle De Laender on 20/01/08.
//  Copyright 2008 CodingMammoth. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface CMCrashReporterGlobal : NSObject
{

}

+ (NSString *)appName;
+ (NSString *)version;
+ (BOOL)isRunningLeopard;
+ (BOOL)checkOnCrashes;
+ (NSString *)crashReportURL;
+ (NSString *)osVersion;
+ (int)numberOfMaximumReports;
@end
