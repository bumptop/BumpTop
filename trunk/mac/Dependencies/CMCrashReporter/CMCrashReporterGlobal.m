//
//  CMCrashReporterGlobal.m
//  CMCrashReporter-App
//
//  Created by Jelle De Laender on 20/01/08.
//  Copyright 2008 CodingMammoth. All rights reserved.
//

#import "CMCrashReporterGlobal.h"


@implementation CMCrashReporterGlobal

+ (NSString *)appName
{
	return [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleName"];
}

+ (NSString *)version
{
	return [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleVersion"];
}

+ (int)numberOfMaximumReports
{
	if (! [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CMMaxReports"]) return 0;
	
	return [[[[NSBundle mainBundle] infoDictionary] objectForKey:@"CMMaxReports"] intValue];
}

+ (BOOL)isRunningLeopard
{
	SInt32 MacVersion;
	Gestalt(gestaltSystemVersion, &MacVersion);
	return MacVersion >= 4176;
}

+ (BOOL)checkOnCrashes
{
	// Integration for later
	return YES;
}

+ (NSString *)crashReportURL
{
	NSString *value = [[[NSBundle mainBundle] infoDictionary]
			objectForKey:@"CMSubmitURL"];
		if (!value) NSLog(@"Warning: No CMSubmitURL - key available for CMCrashReporter. Please add this key at your info.plist file.");
		return value;
}

+ (NSString *)osVersion
{
	return [[NSDictionary dictionaryWithContentsOfFile:@"/System/Library/CoreServices/SystemVersion.plist"]
			objectForKey:@"ProductVersion"];
}
@end
