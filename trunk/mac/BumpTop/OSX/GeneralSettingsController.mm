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

#import "BumpTop/OSX/GeneralSettingsController.h"

#include "BumpTop/AppSettings.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/BumpTopScene.h"
#include "BumpTop/OSX/ModifyLoginItems.h"

static GeneralSettingsController *singleton_ = NULL;

@implementation GeneralSettingsController
-(id)init {
  if(![super initWithNibName:@"GeneralSettingsWindow"
                      bundle:nil])
    return nil;
  assert(singleton_ == NULL);
    singleton_ = self;
  updater_ = [SUUpdater sharedUpdater];
  return self;
}

-(void) prepareForDisplay {
  LoginItems::syncBumpTopShouldStartOnLoginWithSystemPreferences();
  AppSettings::singleton()->loadSettingsFile();
  bumptop_should_start_on_login_ = AppSettings::singleton()->bumptop_should_start_on_login();
  [bumptop_should_start_on_login_check_box_ setState:bumptop_should_start_on_login_];
  image_name_on_walls_hidden_ = AppSettings::singleton()->image_name_on_walls_hidden();
  [image_name_on_walls_hidden_check_box_ setState:image_name_on_walls_hidden_];
  use_lasso_ = AppSettings::singleton()->use_lasso_setting();
  [use_lasso_check_box_ setState:use_lasso_];
  use_new_items_pile_ = AppSettings::singleton()->use_new_items_pile_setting();
  [use_new_items_pile_check_box_ setState:use_new_items_pile_];
  default_icon_size_ = AppSettings::singleton()->default_icon_size();
  [default_icon_size_slider_ setMaxValue:5];
  [default_icon_size_slider_ setMinValue:-4];
  [default_icon_size_slider_ setDoubleValue:default_icon_size_];
}

-(IBAction)changeAutoStartStatus:(id)sender {
  bumptop_should_start_on_login_ = [bumptop_should_start_on_login_check_box_ state];
  if (bumptop_should_start_on_login_ != AppSettings::singleton()->bumptop_should_start_on_login()) {
    AppSettings::singleton()->set_bumptop_should_start_on_login(bumptop_should_start_on_login_);
    if (bumptop_should_start_on_login_) {
      LoginItems::addBumpTopToLoginItems();
    } else {
      LoginItems::removeBumpTopFromLoginItems();
    }
  }
  AppSettings::singleton()->saveSettingsFile();
}

-(IBAction)changeImageNameOnWallsHidden:(id)sender {
  image_name_on_walls_hidden_ = [image_name_on_walls_hidden_check_box_ state];
  if (image_name_on_walls_hidden_ != AppSettings::singleton()->image_name_on_walls_hidden()) {
    AppSettings::singleton()->set_image_name_on_walls_hidden(image_name_on_walls_hidden_);
  }
  AppSettings::singleton()->saveSettingsFile();
  for_each(VisualPhysicsActor* actor, BumpTopApp::singleton()->scene()->room()->room_actors().values()) {
    if (actor->is_an_image_on_wall()) {
      if (actor->name_hidden() || image_name_on_walls_hidden_) {
        actor->set_label_visible(false);
      } else {
        actor->set_label_visible(true);
      }
    }
  }
}

-(IBAction)changeUseLasso:(id)sender {
  use_lasso_ = [use_lasso_check_box_ state];

  if (use_lasso_ != AppSettings::singleton()->use_lasso_setting()) {
    AppSettings::singleton()->set_use_lasso_setting(use_lasso_);
  }
  AppSettings::singleton()->saveSettingsFile();
}

-(IBAction)changeUseNewItemsPile:(id)sender {
  use_new_items_pile_ = [use_new_items_pile_check_box_ state];
  
  if (use_new_items_pile_ != AppSettings::singleton()->use_new_items_pile_setting()) {
    AppSettings::singleton()->set_use_new_items_pile_setting(use_new_items_pile_);
    if (use_new_items_pile_
        && BumpTopApp::singleton()->scene()->room()->new_items_pile() == 0) {
      BumpTopApp::singleton()->scene()->room()->addNewItemsPileInDefaultLocation();
    } else if (!use_new_items_pile_
               && BumpTopApp::singleton()->scene()->room()->new_items_pile() != 0) {
      BumpTopApp::singleton()->scene()->room()->removeNewItemsPile();
    }
  }
  AppSettings::singleton()->saveSettingsFile();
}

-(IBAction)changeDefaultIconSize:(id)sender {
  default_icon_size_ = [default_icon_size_slider_ doubleValue];

  if (default_icon_size_ != AppSettings::singleton()->default_icon_size()) {
    AppSettings::singleton()->set_default_icon_size(default_icon_size_);
  }
  AppSettings::singleton()->saveSettingsFile();
}

-(void)awakeFromNib {
  [self prepareForDisplay];
}


+(GeneralSettingsController *)singleton {
  return singleton_;
}

@end
