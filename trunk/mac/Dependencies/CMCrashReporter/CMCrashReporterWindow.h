//
//  CMCrashReporterWindow.h
//  CMCrashReporter-App
//
//  Created by Jelle De Laender on 20/01/08.
//  Copyright 2008 CodingMammoth. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "CMCrashReporterGlobal.h"
#import <AddressBook/AddressBook.h>

@interface CMCrashReporterWindow : NSWindowController {
	NSArray *paths;
	
	IBOutlet id description;
	IBOutlet id mailaddress;
	IBOutlet id commentField;
	IBOutlet id dontShowThis;
	IBOutlet id includeRapport;
	IBOutlet id application;
	IBOutlet id version;
}

+ (void)runCrashReporterWithPaths:(NSArray *)ar;

- (id)init;
- (NSArray *)paths;
- (void)setPaths:(NSArray *)ar;
- (void)windowDidLoad;
- (IBAction)submitData:(id)sender;
- (IBAction)dontReport:(id)sender;
- (void)removeCrashLog:(NSString *)path;
- (BOOL)submitFile:(NSString *)file;
- (NSString *)translationForKey:(NSString *)key;
- (NSData*)generateFormData:(NSDictionary*)dict;
@end
