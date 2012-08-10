//
//  CMCrashReporterWindow.m
//  CMCrashReporter-App
//
//  Created by Jelle De Laender on 20/01/08.
//  Copyright 2008 CodingMammoth. All rights reserved.
//

#import "CMCrashReporterWindow.h"
#import "CMCrashReporter.h"

@implementation CMCrashReporterWindow

+ (void)runCrashReporterWithPaths:(NSArray *)ar
{
	CMCrashReporterWindow *windowController = [[self alloc] init];
	[windowController setPaths:ar];
	[[windowController window] makeKeyAndOrderFront:nil];
}

#pragma mark Default methods

- (id)init
{
	self = [super initWithWindowNibName:NSStringFromClass([self class])];
	if (self) {
  
	}
	return self;
}

- (NSString *)translationForKey:(NSString *)key
{
 return [[NSBundle bundleForClass:[self class]] localizedStringForKey:key value:key table:nil];
}

- (NSArray *)paths
{
	return paths;
}

- (void)setPaths:(NSArray *)ar
{
	[paths release];
	[ar retain];
	paths = ar;
}

- (void)windowDidLoad
{
  NSString *appname = [CMCrashReporterGlobal appName];
  NSString *desc    = [NSString stringWithFormat:[self translationForKey:@"Explanation"],appname, appname];
  if (desc)
    [description setStringValue:desc];
	[[self window] setTitle:[NSString stringWithFormat:@"%@ - %@ (%@) ",
                           [self translationForKey:@"CrashReport"], appname, [CMCrashReporterGlobal version]]];
	
	ABMultiValue *emails = [[[ABAddressBook sharedAddressBook] me] valueForProperty: kABEmailProperty];
	NSString *email = (NSString *) [emails valueAtIndex: [emails indexForIdentifier: [emails primaryIdentifier]]];	
  
  if (email)
    [mailaddress setStringValue:email];
}

- (void)removeCrashLog:(NSString *)path
{
	NSFileManager *fileManager = [NSFileManager defaultManager];
	fileManager = fileManager;
	[fileManager removeFileAtPath:path handler:nil];
}

- (void)updateIgnoreCrashes
{
	NSUserDefaults *defaults = [[NSUserDefaultsController sharedUserDefaultsController] defaults];
	[defaults setBool:[dontShowThis state] forKey:@"CMCrashReporterIgnoreCrashes"];
}

- (IBAction)submitData:(id)sender
{
	int i = 0;
	
	BOOL failures = NO;
	
	int max = [CMCrashReporterGlobal numberOfMaximumReports];
	
	if (max == 0) max = [paths count];
	
	for (i = 0; i < max; i++) {
		if ([self submitFile:[paths objectAtIndex:i]]) {
			// report succeed
			// File will be removed on close
		} else
			failures = YES;
	}
	
	NSAlert *alert = [[[NSAlert alloc] init] autorelease];
	[alert addButtonWithTitle:[self translationForKey:@"OK"]];
	[alert setMessageText:[CMCrashReporterGlobal appName]];
	[alert setAlertStyle:NSInformationalAlertStyle];
	
	if (!failures)
		[alert setInformativeText:[NSString stringWithFormat:[self translationForKey:@"Thanks for helping us improve %@!"],[CMCrashReporterGlobal appName]]];
	else
		[alert setInformativeText:[NSString stringWithFormat:[self translationForKey:@"%@ was unable to deliver the crash log."],[CMCrashReporterGlobal appName]]];
		
	[alert runModal];
  [self updateIgnoreCrashes];
	[self close];
}

- (IBAction)dontReport:(id)sender
{
  [self updateIgnoreCrashes];
	[self close];
}

- (void)windowWillClose:(NSNotification *)notification
{
  for (NSString *path in paths)
		if ([[NSFileManager defaultManager] fileExistsAtPath:path])
			[self removeCrashLog:path];
  [CMCrashReporter onTaskComplete];
}

#pragma mark send

-(BOOL)submitFile:(NSString *)file
{
	NSMutableDictionary* post_dict = [[NSMutableDictionary alloc] init];

	[post_dict setValue:[NSString stringWithString:@"CMCrashReporter"] forKey:@"type"];
	[post_dict setValue:[CMCrashReporterGlobal appName] forKey:@"application"];
	[post_dict setValue:[CMCrashReporterGlobal version] forKey:@"appVersion"];
	[post_dict setValue:[CMCrashReporterGlobal osVersion] forKey:@"osVersion"];
	[post_dict setValue:[NSString stringWithString:[mailaddress stringValue]] forKey:@"mailaddress"];
	[post_dict setValue:[NSString stringWithString:[[commentField textStorage] string]] forKey:@"comments"];
	[post_dict setValue: [[[[NSDateFormatter alloc] initWithDateFormat:@"%H:%M:%S" allowNaturalLanguage:NO] autorelease] stringFromDate:[NSDate date]] forKey:@"time"];
	[post_dict setValue: [[[[NSDateFormatter alloc] initWithDateFormat:@"%m/%d/%Y" allowNaturalLanguage:NO] autorelease] stringFromDate:[NSDate date]] forKey:@"date"];
	
	if ([includeRapport state])
		[post_dict setValue:[NSString stringWithContentsOfFile:file encoding:NSUTF8StringEncoding error:nil] forKey:@"rapport"];
	else
		[post_dict setValue:@"not included" forKey:@"rapport"];
	
	NSData* regData = [self generateFormData:post_dict];
	[post_dict release];

	NSURL *url = [NSURL URLWithString:[CMCrashReporterGlobal crashReportURL]];
	NSMutableURLRequest* post = [NSMutableURLRequest requestWithURL:url];
	[post addValue: @"multipart/form-data; boundary=_insert_some_boundary_here_" forHTTPHeaderField: @"Content-Type"];
	[post setHTTPMethod: @"POST"];
	[post setHTTPBody:regData];
	
	NSURLResponse* response;
	NSError* error;
	NSData* result = [NSURLConnection sendSynchronousRequest:post returningResponse:&response error:&error];
	NSString *res = [[[NSString alloc] initWithData:result encoding:NSASCIIStringEncoding] autorelease];

	return ([res isEqualTo:@"ok"]);
}

#pragma mark Generate form data

- (NSData*)generateFormData:(NSDictionary*)dict
{
	NSString* boundary    = [NSString stringWithString:@"_insert_some_boundary_here_"];
	NSMutableData* result = [[NSMutableData alloc] initWithCapacity:100];

  for (NSString *key in dict) {
    id value = [dict valueForKey:key];
		[result appendData:[[NSString stringWithFormat:@"--%@\n", boundary] dataUsingEncoding:NSASCIIStringEncoding]];
		if ( ([value class] == [NSString class]) || ([@"tmp" isKindOfClass:[value class]])) {
			[result appendData:[[NSString stringWithFormat:@"Content-Disposition: form-data; name=\"%@\"\n\n", key] dataUsingEncoding:NSASCIIStringEncoding]];
			[result appendData:[[NSString stringWithFormat:@"%@",value] dataUsingEncoding:NSASCIIStringEncoding]];
		} else if ([value class] == [NSURL class] && [value isFileURL]) {
			[result appendData:[[NSString stringWithFormat:@"Content-Disposition: form-data; name=\"%@\"; filename=\"%@\"\n", key, [[value path] lastPathComponent]] dataUsingEncoding:NSASCIIStringEncoding]];
			[result appendData:[[NSString stringWithString:@"Content-Type: application/octet-stream\n\n"] dataUsingEncoding:NSASCIIStringEncoding]];
			[result appendData:[NSData dataWithContentsOfFile:[value path]]];
		} else {
			NSLog(@"No string or NSURL!");
		}
		
		[result appendData:[[NSString stringWithString:@"\n"] dataUsingEncoding:NSASCIIStringEncoding]];
	}
	[result appendData:[[NSString stringWithFormat:@"--%@--\n", boundary] dataUsingEncoding:NSASCIIStringEncoding]];
	
	return [result autorelease];
}
@end
