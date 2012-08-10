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

#import "BumpTop/OSX/TextureSettingsController.h"

#include "BumpTop/AppSettings.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/BumpTopScene.h"
#include "BumpTop/FileManager.h"
#include "BumpTop/MaterialLoader.h"
#include "BumpTop/protoc/AllMessages.pb.h"
#include "BumpTop/QStringHelpers.h"
#include "BumpTop/Room.h"

static TextureSettingsController *singleton_ = NULL;

// This "singleton" implementation assumes this controller will only be initialized once, from the nib.
// From code, we can always assume it's been created, since it's in MainMenu.xib
@implementation TextureSettingsController
// opens the window if it has not been open yet and initializes default paths
-(id)init {
  if(![super initWithNibName:@"TextureSettingsBox" bundle:nil])
    return nil;
  AppSettings::singleton()->loadSettingsFile();
  assert(singleton_ == NULL);
  singleton_ = self;
  return self;
}

-(void)showInWindow:(NSWindow *)preference_window {
  preference_window_ = preference_window;
  room_ = BumpTopApp::singleton()->scene()->room();
  is_undo_button_enabled_ = false;
  [self initAllBackupSettings];
  is_undo_button_enabled_ = false;
  [undoChangesButton setEnabled:false];
  use_floor_texture_for_all_ = AppSettings::singleton()->apply_floor_to_all_surfaces();
  [checkBox setState:use_floor_texture_for_all_];
}

// action for when browse button for floor is hit which opens up a OpenPanel
-(IBAction)browseFloor:(id)sender {
  [self showOpenPanel:FLOOR];
}

// action for when browse button for front wall is hit which opens up a OpenPanel
-(IBAction)browseFront:(id)sender {
  [self showOpenPanel:FRONT_WALL];
}

// action for when browse button for back wall is hit which opens up a OpenPanel
-(IBAction)browseBack:(id)sender {
  [self showOpenPanel:BACK_WALL];
}

// action for when browse button for right wall is hit which opens up a OpenPanel
-(IBAction)browseRight:(id)sender {
  [self showOpenPanel:RIGHT_WALL];
}

// action for when browse button for left wall is hit which opens up a OpenPanel
-(IBAction)browseLeft:(id)sender {
  [self showOpenPanel:LEFT_WALL];
}

// reset to factory defaults
-(IBAction)resetToFactorySettings:(id)sender {
  room_->setTextureSettingForSurface(FLOOR, QStringFromUtf8(kDefaultFloorTexturePath));
  room_->setTextureSettingForSurface(LEFT_WALL, QStringFromUtf8(kDefaultLeftWallTexturePath));
  room_->setTextureSettingForSurface(RIGHT_WALL, QStringFromUtf8(kDefaultRightWallTexturePath));
  room_->setTextureSettingForSurface(BACK_WALL, QStringFromUtf8(kDefaultBackWallTexturePath));
  room_->setTextureSettingForSurface(FRONT_WALL, QStringFromUtf8(kDefaultFrontWallTexturePath));
  AppSettings::singleton()->set_apply_floor_to_all_surfaces(KDefaultUseFloorTextureForAll);
  AppSettings::singleton()->saveSettingsFile();
  [self updateAllTexturesFromSettings];
  if (!is_undo_button_enabled_) {
    [undoChangesButton setEnabled:true];
    is_undo_button_enabled_ = true;
  }
}

// action for when close button is hit which sets all back_up path to last applied and closes window
-(IBAction)undoChanges:(id)sender {
  room_->setTextureSettingForSurface(FLOOR, QStringFromUtf8(backup_floor_texture_path_));
  room_->setTextureSettingForSurface(LEFT_WALL, QStringFromUtf8(backup_left_wall_texture_path_));
  room_->setTextureSettingForSurface(RIGHT_WALL, QStringFromUtf8(backup_right_wall_texture_path_));
  room_->setTextureSettingForSurface(BACK_WALL, QStringFromUtf8(backup_back_wall_texture_path_));
  room_->setTextureSettingForSurface(FRONT_WALL, QStringFromUtf8(backup_front_wall_texture_path_));
  AppSettings::singleton()->saveSettingsFile();
  [self updateAllTexturesFromSettings];
  [undoChangesButton setEnabled:false];
  is_undo_button_enabled_ = false;
}

// this function registers path user selected and updates and protocol buffer
-(void)openPanelDidEnd:(NSOpenPanel *) openPanel
            returnCode:(int)returnCode
           contextInfo:(void *)contextInfo
{
  RoomSurfaceType wall_type = static_cast<RoomSurfaceType>((int)contextInfo);
  if (returnCode == NSOKButton) {
    NSString *path = [openPanel filename];
    QString image_path = QStringFromNSString(path);

    room_->setTextureSettingForSurface(wall_type, image_path);
    AppSettings::singleton()->saveSettingsFile();

    if (wall_type == FLOOR && use_floor_texture_for_all_) {
      [self updateAllTexturesFromSettings];
    } else {
      room_->setAndAdjustMaterialForSurface(wall_type, image_path);
    }
    if (!is_undo_button_enabled_) {
      [undoChangesButton setEnabled:true];
      is_undo_button_enabled_ = true;
    }
  }
}

// this function opens up an OpenPanel
-(void)showOpenPanel:(RoomSurfaceType) typeWall {
  NSOpenPanel *panel = [NSOpenPanel openPanel];
  NSArray *fileTypes = [NSArray arrayWithObjects:@"jpg", @"jpeg", @"gif", @"png",  @"dds", @"tga", @"raw", @"bmp", @"tif", @"tiff", nil];
  [panel beginSheetForDirectory:nil
                           file:nil
                          types:fileTypes
                 modalForWindow:preference_window_
                  modalDelegate:self
                 didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:)
                    contextInfo:(void*)typeWall];
}


-(void)changeUseFloorTextureForAll:(id)sender {
  use_floor_texture_for_all_ = [checkBox state];
  AppSettings::singleton()->set_apply_floor_to_all_surfaces(use_floor_texture_for_all_);
  AppSettings::singleton()->saveSettingsFile();
  [self updateAllTexturesFromSettings];
  if (!is_undo_button_enabled_) {
    [undoChangesButton setEnabled:true];
    is_undo_button_enabled_ = true;
  }
}

-(void)initAllBackupSettings {
  backup_floor_texture_path_ = AppSettings::singleton()->floor_image_path();
  backup_front_wall_texture_path_ = AppSettings::singleton()->front_wall_image_path();
  backup_back_wall_texture_path_ = AppSettings::singleton()->back_wall_image_path();
  backup_right_wall_texture_path_ = AppSettings::singleton()->right_wall_image_path();
  backup_left_wall_texture_path_ = AppSettings::singleton()->left_wall_image_path();
  backup_use_floor_texture_for_all_ = AppSettings::singleton()->apply_floor_to_all_surfaces();
}

-(void)updateAllTexturesFromSettings {
  room_->updateAllTexturesFromSettings();
  [checkBox setState:AppSettings::singleton()->apply_floor_to_all_surfaces()];
}

+(TextureSettingsController*)singleton {
  return singleton_;
}

-(void)awakeFromNib {
  [self initAllBackupSettings];
  use_floor_texture_for_all_ = AppSettings::singleton()->apply_floor_to_all_surfaces();
  [checkBox setState:use_floor_texture_for_all_];
}

@end
