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

#import <Cocoa/Cocoa.h>

@interface CompressProgressDialogController : NSWindowController {
  IBOutlet NSProgressIndicator *progress_bar_;
  IBOutlet NSTextField *file_name_;
  IBOutlet NSTextField *current_status_;

  NSTask* file_compression_task_;
  NSTask* copy_file_to_archive_;
  NSTask* remove_archive_folder_;
  NSPipe* standard_error_pipe_for_compress_;
  NSPipe* standard_error_pipe_for_copy_;
  NSTimer* timer_;
  NSString* source_path_;
  NSString* target_path_;
  NSString* archive_path_;
  bool is_multiple_files_;
  NSMutableArray* source_paths_;
  NSMutableArray* files_to_remove_;
  int number_of_items_;
}

-(IBAction)closeCompressProgressDialog:(id)sender;
-(void)showCompressProgressDialogForFiles:(NSArray *)file_paths withArchivePath:(NSString *)archive_path andTargetPath:(NSString *) target_path;
-(void)showCompressProgressDialogForFile:(NSString *)source_path withTargetPath:(NSString *)target_path;
-(void)checkTimeForCopy:(NSTimer *)time;
-(void)checkTimeForCompress:(NSTimer *)time;
-(void)checkTimeForDeleteOldTemperoryArchive:(NSTimer *)time;
-(void)checkTimeForCleanupArchiveAndZipFile:(NSTimer *)time;
-(void)compress;
-(void)copy:(NSString *)file_name;
-(void)updateDialogWithCompressStatus:(NSNotification *)notification;
-(void)updateDialogWithCopyStatus:(NSNotification *)notification;
-(NSString *)convertStandardErrorToCurrentStatusForCompress:(NSString *)standard_error_str;
-(NSString *)convertStandardErrorToCurrentStatusForCopy:(NSString *)standard_error_str;
-(NSString *)numberOfItemsToBeCompressed;
-(void)removeFile:(NSString *)archive_path;
-(void)progressController:(NSString *)progress_identifier;

@end
