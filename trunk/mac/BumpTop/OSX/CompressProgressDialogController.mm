//
//  Copyright 2012 Google Inc. All Rights Reserved.
//  
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  
//      http://www.apache.org/licenses/LICENSE-2.0
//  
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

#import "BumpTop/OSX/CompressProgressDialogController.h"

#include <QtCore/QString>

#include "BumpTop/BumpTopCommands.h"
#include "BumpTop/FileItem.h"
#include "BumpTop/FileManager.h"
#include "BumpTop/QStringHelpers.h"

@implementation CompressProgressDialogController

-(id)init {
  if (![super initWithWindowNibName:@"CompressProgressDialog"]) {
    return nil;
  }
  number_of_items_ = 0;
  timer_ = NULL;
  standard_error_pipe_for_copy_ = NULL;
  return self;
}

-(void)windowDidLoad {
}

-(void)showCompressProgressDialogForFile:(NSString *)source_path withTargetPath:(NSString *)target_path {
  is_multiple_files_ = false;
  source_path_ = source_path;
  [source_path_ retain];
  target_path_ = target_path;
  [target_path_ retain];
  [self showWindow:self];
  [progress_bar_ startAnimation:self];
  [file_name_ setStringValue:[@"Compressing to " stringByAppendingFormat:
                              [[NSFileManager defaultManager] displayNameAtPath:target_path_]]];
  [self progressController:@"compress"];
}

-(void)showCompressProgressDialogForFiles:(NSArray *)file_paths withArchivePath:(NSString *)archive_path andTargetPath:(NSString *)target_path {
  number_of_items_ = 0;
  is_multiple_files_ = true;
  source_paths_ = [NSMutableArray arrayWithArray:file_paths];
  [source_paths_ retain];
  target_path_ = target_path;
  [target_path_ retain];
  archive_path_ = archive_path;
  [archive_path_ retain];
  source_path_ = [archive_path stringByAppendingFormat:@"/Archive"];
  [source_path_ retain];
  [self showWindow:self];
  [progress_bar_ startAnimation:self];
  [file_name_ setStringValue:[@"Preparing to compress to " stringByAppendingFormat:
                              [[NSFileManager defaultManager] displayNameAtPath:target_path_]]];
  [self progressController:@"deleteOldTemperoryArchive"];
}

-(void)checkTimeForDeleteOldTemperoryArchive:(NSTimer *)time {
  if(![remove_archive_folder_ isRunning]) {
    [remove_archive_folder_ release];
    remove_archive_folder_ = NULL;

    // we have to create the dir properly with qdir first, letting ditto create the dir will cause problems (dir can't be deleted
    // due to not having permission
    QDir dir(FileManager::getApplicationDataPath());
    QString archive_name = QFileInfo(QStringFromNSString(archive_path_)).fileName();
    dir.mkdir(archive_name);
    QDir archive(QStringFromNSString(archive_path_));
    archive.mkdir("Archive");
    [self progressController:@"copy"];
  }
}

-(void)checkTimeForCopy:(NSTimer *)time {
  if(![copy_file_to_archive_ isRunning]) {
    [copy_file_to_archive_ release];
    copy_file_to_archive_ = NULL;
    [standard_error_pipe_for_copy_ release];
    standard_error_pipe_for_copy_ = NULL;
    if ([source_paths_ count] == 0) {
      [file_name_ setStringValue:[self numberOfItemsToBeCompressed]];
      [self progressController:@"compress"];
    } else {
      [self copy:[source_paths_ objectAtIndex:0]];
      [source_paths_ removeObjectAtIndex:0];
    }
  }
}

-(void)checkTimeForCompress:(NSTimer *)time {
  if(![file_compression_task_ isRunning]) {
    [file_compression_task_ release];
    file_compression_task_ = NULL;
    [standard_error_pipe_for_compress_ release];
    standard_error_pipe_for_compress_ = NULL;
    [file_name_ setStringValue:@"Finishing"];

    if (is_multiple_files_) {
      files_to_remove_ = [[NSMutableArray alloc] initWithObjects:archive_path_, nil];
      [self progressController:@"cleanupArchiveAndZipFile"];
    } else {
      [self progressController:@"closeWindow"];
    }
  }
}

-(void)checkTimeForCleanupArchiveAndZipFile:(NSTimer *)time {
  if(![remove_archive_folder_ isRunning]) {
    if ([files_to_remove_ count] == 0) {
      [files_to_remove_ release];
      [remove_archive_folder_ release];
      remove_archive_folder_ = NULL;
      [self progressController:@"closeWindow"];
    } else {
      [remove_archive_folder_ release];
      remove_archive_folder_ = NULL;
      [self removeFile:[files_to_remove_ objectAtIndex:0]];
      [files_to_remove_ removeObjectAtIndex:0];
    }
  }
}

// http://www.cocoadev.com/index.pl?UsingZipFilesExamples
-(void)compress {
  file_compression_task_ = [[NSTask alloc] init];
  standard_error_pipe_for_compress_ = [[NSPipe alloc] init];

  [file_compression_task_ setLaunchPath:@"/usr/bin/ditto"];
  [file_compression_task_ setArguments:
                          [NSArray arrayWithObjects:@"-v", @"-V", @"-c", @"-k", @"--rsrc", source_path_, target_path_, nil]];
  [file_compression_task_ setStandardError:standard_error_pipe_for_compress_];
  NSFileHandle *standard_error_file_handle_for_compress = [standard_error_pipe_for_compress_ fileHandleForReading];

  // sends data to updateDialogWithCompressStatus whenever ditto starts to compress a file
  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(updateDialogWithCompressStatus:)
                                               name:NSFileHandleReadCompletionNotification
                                             object:standard_error_file_handle_for_compress];

  [file_compression_task_ launch];
  [standard_error_file_handle_for_compress readInBackgroundAndNotify];
}

-(void)updateDialogWithCompressStatus:(NSNotification *)notification {
  NSData *standard_error_data = [[notification userInfo] valueForKey:NSFileHandleNotificationDataItem];
  NSString *standard_error_str = [[NSString alloc] initWithData:standard_error_data
                                                       encoding:NSASCIIStringEncoding];
  if ([standard_error_str isEqualToString:@""]) {
    [current_status_ setStringValue:@""];
  } else {
    [current_status_ setStringValue:[self convertStandardErrorToCurrentStatusForCompress:standard_error_str]];
  }
  [standard_error_str release];
  if (file_compression_task_ != NULL) {
    [[standard_error_pipe_for_compress_ fileHandleForReading] readInBackgroundAndNotify];
  }
}

-(void)copy:(NSString *)file_name {
  copy_file_to_archive_ = [[NSTask alloc] init];
  standard_error_pipe_for_copy_ = [[NSPipe alloc] init];
  [copy_file_to_archive_ setLaunchPath:@"/usr/bin/ditto"];
  [copy_file_to_archive_ setArguments:
   [NSArray arrayWithObjects:@"-v", @"-V", @"--rsrc", file_name, [source_path_ stringByAppendingFormat:[@"/" stringByAppendingFormat:[[NSFileManager defaultManager] displayNameAtPath:file_name]]], nil]];
  [copy_file_to_archive_ setStandardError:standard_error_pipe_for_copy_];
  NSFileHandle *standard_error_file_handle_for_copy = [standard_error_pipe_for_copy_ fileHandleForReading];

  // sends data to updateDialogWithCompressStatus whenever ditto starts to copy a file to temporary archive
  [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(updateDialogWithCopyStatus:)
                                               name:NSFileHandleReadCompletionNotification
                                             object:standard_error_file_handle_for_copy];

  [copy_file_to_archive_ launch];
  [standard_error_file_handle_for_copy readInBackgroundAndNotify];
}

-(void)updateDialogWithCopyStatus:(NSNotification *)notification {
  NSData *standard_error_data = [[notification userInfo] valueForKey:NSFileHandleNotificationDataItem];
  NSString *standard_error_str = [[NSString alloc] initWithData:standard_error_data
                                                       encoding:NSASCIIStringEncoding];
  if (![standard_error_str isEqualToString:@""])
    [current_status_ setStringValue:[self convertStandardErrorToCurrentStatusForCopy:standard_error_str]];

  [standard_error_str release];
  if (copy_file_to_archive_ != NULL) {
    [[standard_error_pipe_for_copy_ fileHandleForReading] readInBackgroundAndNotify];
  }
}

-(void)removeFile:(NSString *)archive_path {
  remove_archive_folder_ = [[NSTask alloc] init];
  [remove_archive_folder_ setLaunchPath:@"/bin/rm"];
  [remove_archive_folder_ setArguments:
   [NSArray arrayWithObjects:@"-f", @"-R", archive_path, nil]];
  [remove_archive_folder_ launch];
}

-(IBAction)closeCompressProgressDialog:(id)sender {
  if (copy_file_to_archive_ != NULL) {
    [copy_file_to_archive_ terminate];
    [copy_file_to_archive_ release];
    copy_file_to_archive_ = NULL;
  }
  if (file_compression_task_ != NULL) {
    [file_compression_task_ terminate];
    [file_compression_task_ release];
    file_compression_task_ = NULL;
  }
  if (standard_error_pipe_for_compress_ != NULL) {
    [standard_error_pipe_for_compress_ release];
    standard_error_pipe_for_compress_ = NULL;
  }
  if (standard_error_pipe_for_copy_ != NULL) {
    [standard_error_pipe_for_copy_ release];
    standard_error_pipe_for_copy_ = NULL;
  }
  [file_name_ setStringValue:@"Closing"];
  if (is_multiple_files_) {
    files_to_remove_ = [[NSMutableArray alloc] initWithObjects:archive_path_, target_path_, nil];
  } else {
    files_to_remove_ = [[NSMutableArray alloc] initWithObjects:target_path_, nil];
  }
  [self progressController:@"cleanupArchiveAndZipFile"];
}

-(NSString *)convertStandardErrorToCurrentStatusForCompress:(NSString *)standard_error_str {
  QString modifier = QStringFromNSString(standard_error_str);
  modifier.remove(0, modifier.lastIndexOf("/") + 1);
  if (modifier.lastIndexOf("...") != -1)
    modifier.remove(modifier.lastIndexOf(" ..."), 7);
  else {
    modifier.remove(modifier.length() - 1, 1);
    modifier += " ";
  }
  modifier = "Current item: \"" + modifier + "\"";
  return NSStringFromQString(modifier);
}

-(NSString *)numberOfItemsToBeCompressed {
  QString file_name;
  file_name.setNum(number_of_items_);
  file_name = "Compressing " + file_name + " items to " + "\"" + QStringFromNSString([[NSFileManager defaultManager] displayNameAtPath:target_path_]) + "\"";
  return NSStringFromQString(file_name);
}

-(NSString *)convertStandardErrorToCurrentStatusForCopy:(NSString *)standard_error_str {
  QString modifier = QStringFromNSString(standard_error_str);
  number_of_items_ += modifier.count("...");
  modifier.setNum((int)number_of_items_);
  modifier = "Preparing to compress " + modifier + " items.";
  return NSStringFromQString(modifier);
}

-(void)progressController:(NSString *)progress_identifier {
  if (timer_ != NULL) {
    [timer_ invalidate];
    [timer_ release];
    timer_ = NULL;
  }
  if([progress_identifier isEqualToString:@"deleteOldTemperoryArchive"]) {
    [self removeFile:archive_path_];
    timer_ = [[NSTimer scheduledTimerWithTimeInterval:0.02
                                               target:self
                                             selector:@selector(checkTimeForDeleteOldTemperoryArchive:)
                                             userInfo:nil
                                              repeats:YES] retain];
  } else if([progress_identifier isEqualToString:@"copy"]) {
    [self copy:[source_paths_ objectAtIndex:0]];
    [source_paths_ removeObjectAtIndex:0];
    timer_ = [[NSTimer scheduledTimerWithTimeInterval:0.02
                                               target:self
                                             selector:@selector(checkTimeForCopy:)
                                             userInfo:nil
                                              repeats:YES] retain];

  } else if([progress_identifier isEqualToString:@"compress"]) {
    [self compress];
    timer_ = [[NSTimer scheduledTimerWithTimeInterval:0.02
                                               target:self
                                             selector:@selector(checkTimeForCompress:)
                                             userInfo:nil
                                              repeats:YES] retain];
  } else if([progress_identifier isEqualToString:@"cleanupArchiveAndZipFile"]) {
    [self removeFile:[files_to_remove_ objectAtIndex:0]];
    [files_to_remove_ removeObjectAtIndex:0];
    timer_ = [[NSTimer scheduledTimerWithTimeInterval:0.02
                                               target:self
                                             selector:@selector(checkTimeForCleanupArchiveAndZipFile:)
                                             userInfo:nil
                                              repeats:YES] retain];
  } else {
    BumpTopCommand::pending_archive_zip_paths.removeAll(QStringFromNSString(target_path_));
    [source_path_ release];
    [target_path_ release];
    if (is_multiple_files_) {
      [source_paths_ release];
      [archive_path_ release];
    }
    [progress_bar_ stopAnimation:self];
    [[self window] setReleasedWhenClosed:true];
    [[self window] performClose:nil];
  }
}

@end
