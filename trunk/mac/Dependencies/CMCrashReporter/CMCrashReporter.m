//
//  CMCrashReporter.m
//  CMCrashReporter-App
//
//  Created by Jelle De Laender on 20/01/08.
//  Copyright 2008 CodingMammoth. All rights reserved.
//

#import "CMCrashReporter.h"

@implementation CMCrashReporter

+ (BOOL)check
{
	if ([CMCrashReporterGlobal checkOnCrashes]) {
    NSString* bundle_path = [[NSBundle mainBundle] bundlePath];
    NSString* resources_path = [[[bundle_path stringByDeletingLastPathComponent]
                                 stringByDeletingLastPathComponent]
                                stringByAppendingString: @"/Resources"];
    NSDirectoryEnumerator *dir_enumerator = [[NSFileManager defaultManager] enumeratorAtPath:resources_path];
    NSMutableArray *std_output_files = [[NSMutableArray alloc] init];
    NSString *file;
    while (file = [dir_enumerator nextObject]) {
      if ([file length] > 33 &&
          ([[file substringToIndex: 6] isEqualToString:@"stderr"] ||
           [[file substringToIndex: 6] isEqualToString:@"stdout"])) {
            NSString *date_string = [[file substringWithRange:NSMakeRange(7, 25)] stringByReplacingOccurrencesOfString:@"#"
                                                                                                            withString:@" "];
            NSDate *date = [[NSDate alloc] initWithString: date_string];
            if (abs([date timeIntervalSinceNow]) > 20) { // if file was created more than 20 sec ago
              NSString *file_path = [NSString stringWithFormat:@"%@/%@", resources_path, file];
              [std_output_files addObject:file_path];
            }
          }
    }
    

		NSMutableArray *reports = [self getReports];
		if ([reports count] > 0) {
      NSEnumerator * enumerator = [std_output_files objectEnumerator];
      while(file = [enumerator nextObject]) {
        [reports addObject:file];
      }    
      
      is_task_completed_ = FALSE;
			[CMCrashReporterWindow runCrashReporterWithPaths:reports];
      return TRUE;
		} else {

      NSEnumerator * enumerator = [std_output_files objectEnumerator];
      while(file = [enumerator nextObject]) {
        [[NSFileManager defaultManager] removeItemAtPath:file error:NULL];
      }
    }
  }
  return FALSE;
}

+ (NSMutableArray *)getReports
{
	NSFileManager *fileManager = [NSFileManager defaultManager];
	
	if ([CMCrashReporterGlobal isRunningLeopard]) {
		// Leopard format is AppName_Year_Month_Day
		NSString *file;
		NSString *path = [@"~/Library/Logs/CrashReporter/" stringByExpandingTildeInPath];
		NSDirectoryEnumerator *dirEnum = [[NSFileManager defaultManager] enumeratorAtPath:path];

		NSMutableArray *array = [NSMutableArray array];
		while (file = [dirEnum nextObject])
			if ([file hasPrefix:[CMCrashReporterGlobal appName]])
				[array addObject:[[NSString stringWithFormat:@"~/Library/Logs/CrashReporter/%@",file] stringByExpandingTildeInPath]];
		
		return array;
	} else {
		// Tiger Formet is AppName.crash.log
		NSString *path = [[NSString stringWithFormat:@"~/Library/Logs/CrashReporter/%@.crash.log",[CMCrashReporterGlobal appName]] stringByExpandingTildeInPath];
		if ([fileManager fileExistsAtPath:path]) return [NSArray arrayWithObject:path];
		else return nil;
	}
}

+ (void)onTaskComplete
{
  is_task_completed_ = TRUE;
}

+ (BOOL)isTaskCompleted
{
  return is_task_completed_;
}
@end
