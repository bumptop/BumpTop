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

#import "BumpTop/OSX/ProSettingsController.h"

#include "BumpTop/Authorization.h"
#include "BumpTop/QStringHelpers.h"

static ProSettingsController *singleton_ = NULL;

@implementation ProSettingsController
-(id)init {
  if(![super initWithNibName:@"ProSettingsWindow" bundle:nil]) {
    return nil;
  }
  assert(singleton_ == NULL);
  singleton_ = self;
  return self;
}

-(IBAction)bumpTopWebsiteLink:(id)sender {
  NSURL* url = [[NSURL alloc] initWithString: @"http://bumptop.com/mac/bumptoppro.html"];
  [[NSWorkspace sharedWorkspace] openURL:url];
}

-(IBAction)authorize:(id)sender {
  if (ProAuthorization::singleton()->authorized()) {
    if (ProAuthorization::singleton()->deleteLicenseFile()) {
      ProAuthorization::singleton()->set_authorized(false);
      [authorized_key_ setStringValue:@""];
    }
  } else {
    QString invite_code = QStringFromNSString([authorized_key_ stringValue]).trimmed();
    BoolAndQString success_and_error_message = ProAuthorization::singleton()->registerOverInternet(invite_code);

    if (success_and_error_message.first) {
      success_and_error_message = ProAuthorization::singleton()->decipherAndLoadConfigFile();
      if (success_and_error_message.first) {
        if (!ProAuthorization::singleton()->saveLicenseFile()) {
          success_and_error_message = BoolAndQString(false, "Failed to write license file to hard disk");
        }
      }
    } else if (success_and_error_message.second == "Couldn't find that invite code.") {
      QDateTime now = QDateTime::currentDateTime();
    }

    if (!success_and_error_message.first) {
      NSAlert *alert = [[NSAlert alloc] init];
      [alert addButtonWithTitle:@"OK"];
      [alert setMessageText:@"Authorization"];
      [alert setInformativeText:NSStringFromQString(success_and_error_message.second)];
      [alert setAlertStyle:NSWarningAlertStyle];

      [alert beginSheetModalForWindow:[[self view] window]
                        modalDelegate:nil
                       didEndSelector:nil
                          contextInfo:nil];
    }
  }
  [self updateDisplay];
}

-(void) prepareForDisplay {
  [self updateDisplay];
}

-(void) updateDisplay {
  if (ProAuthorization::singleton()->authorized()) {
    [authorized_text_ setStringValue:@"Thank you! BumpTop Pro features have been unlocked."];
    [authorized_key_ setEnabled:false];
    [authorized_button_ setTitle:@"Deauthorize"];
  } else {
    [authorized_text_  setStringValue:@"If you bought BumpTop Pro already, enter your key here."];
    [authorized_key_ setEnabled:true];
    [authorized_button_ setTitle:@"Unlock"];
  }
}

+(ProSettingsController *)singleton {
  return singleton_;
}

@end
