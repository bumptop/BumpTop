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

#import "AboutWindowController.h"


#include "BumpTop/QStringHelpers.h"
#include "BumpTop/FileManager.h"

@implementation AboutWindowController

- (void) setBumpTopVersion:(NSString*)version {
  [version_text_ setTitleWithMnemonic:version];
}

- (IBAction) openLicence:(id)sender {
  NSString* licence_path = NSStringFromQString(FileManager::pathForResource("EULA.pdf"));
  [[NSWorkspace sharedWorkspace] openFile:licence_path];
}

- (IBAction) openAcknowledgements:(id)sender {
  NSString* licence_path = NSStringFromQString(FileManager::pathForResource("Acknowledgements.pdf"));
  [[NSWorkspace sharedWorkspace] openFile:licence_path];
}

- (void)dealloc {
    [super dealloc];
}


@end
