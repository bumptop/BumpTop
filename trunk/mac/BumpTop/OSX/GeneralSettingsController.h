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

#import <Sparkle/Sparkle.h>

@interface GeneralSettingsController : NSViewController {
  IBOutlet NSButton* bumptop_should_start_on_login_check_box_;
  IBOutlet NSButton* image_name_on_walls_hidden_check_box_;
  IBOutlet NSButton* use_lasso_check_box_;
  IBOutlet NSButton* use_new_items_pile_check_box_;
  IBOutlet NSSliderCell* default_icon_size_slider_;
  bool bumptop_should_start_on_login_;
  bool image_name_on_walls_hidden_;
  bool use_lasso_;
  bool use_new_items_pile_;
  float default_icon_size_;
  SUUpdater* updater_;
}

-(void) prepareForDisplay;
-(IBAction)changeImageNameOnWallsHidden:(id)sender;
-(IBAction)changeAutoStartStatus:(id)sender;
-(IBAction)changeUseLasso:(id)sender;
-(IBAction)changeUseNewItemsPile:(id)sender;
-(IBAction)changeDefaultIconSize:(id)sender;
+(GeneralSettingsController *)singleton;

@end
