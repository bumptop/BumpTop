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
#include "BumpTop/Room.h"
#include <QtCore/QString>
#include <string>

void setTextureForSurface(RoomSurfaceType wall_type, QString path);

@interface TextureSettingsController : NSViewController {
  IBOutlet NSButton *checkBox;
  IBOutlet NSButton *undoChangesButton;
  std::string backup_floor_texture_path_;
  std::string backup_front_wall_texture_path_;
  std::string backup_back_wall_texture_path_;
  std::string backup_right_wall_texture_path_;
  std::string backup_left_wall_texture_path_;
  bool use_floor_texture_for_all_;
  bool backup_use_floor_texture_for_all_;
  bool is_undo_button_enabled_;
  NSWindow* preference_window_;
  Room* room_;
}

-(IBAction)browseFloor:(id)sender;
-(IBAction)browseFront:(id)sender;
-(IBAction)browseBack:(id)sender;
-(IBAction)browseRight:(id)sender;
-(IBAction)browseLeft:(id)sender;
-(IBAction)resetToFactorySettings:(id)sender;
-(IBAction)undoChanges:(id)sender;
-(IBAction)changeUseFloorTextureForAll:(id)sender;
-(void)showInWindow:(NSWindow *)preference_window;
-(void)showOpenPanel:(RoomSurfaceType)typeWall;
-(void)initAllBackupSettings;
-(void)updateAllTexturesFromSettings;
+(TextureSettingsController*)singleton;
@end
