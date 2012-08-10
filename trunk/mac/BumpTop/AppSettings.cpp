/*
 *  Copyright 2012 Google Inc. All Rights Reserved.
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
#include <string>
#include <utility>

#include "BumpTop/AppSettings.h"
#include "BumpTop/BumpTopInstanceLock.h"
#include "BumpTop/FileManager.h"
#include "BumpTop/ProtocolBufferHelpers.h"
#include "BumpTop/QStringHelpers.h"

#define SETTINGS_FILE_PATH FileManager::getApplicationDataPath() + \
                           QString(BumpTopInstanceLock::is_running_in_sandbox() ? "bump.sandbox.settings" : "bump.settings")  // NOLINT
SINGLETON_IMPLEMENTATION(AppSettings)

AppSettings::AppSettings()
: last_run_bumptop_version_(QString("")),
  desktop_shows_hard_disks_(true),
  desktop_shows_external_hard_disks_(true),
  desktop_shows_removable_media_(true),
  desktop_shows_connected_servers_(true) {
  image_extensions_ << "jpg"
                    << "jpeg"
                    << "png"
                    << "bmp"
                    << "psd"
                    << "pdf"
                    << "tif"
                    << "tiff"
                    << "gif";
}

bool AppSettings::loadSettingsFile() {
  return loadBufferFromFile(singleton(), SETTINGS_FILE_PATH);
}

bool AppSettings::saveSettingsFile() {
  return saveBufferToFile(singleton(), SETTINGS_FILE_PATH);
}

QString AppSettings::last_run_bumptop_version() {
  return last_run_bumptop_version_;
}

void AppSettings::set_last_run_bumptop_version(QString version) {
  last_run_bumptop_version_ = version;
}

void AppSettings::set_has_auto_hide_dock(bool has_auto_hide_dock) {
  has_auto_hide_dock_ = has_auto_hide_dock;
}

bool AppSettings::has_auto_hide_dock() {
  return has_auto_hide_dock_;
}

void AppSettings::set_dock_position(OSXDockPosition dock_position) {
  dock_position_ = dock_position;
}

OSXDockPosition AppSettings::dock_position() {
  return dock_position_;
}

void AppSettings::set_global_material_name(GlobalMaterials material, QString material_name) {
  global_material_names_[material] = material_name;
}

QString AppSettings::global_material_name(GlobalMaterials material) {
  return global_material_names_[material];
}

QStringList AppSettings::image_extensions() {
  return image_extensions_;
}

void AppSettings::updateFinderPreferences() {
  NSString* script;
  NSAppleEventDescriptor *script_return;
  NSDictionary* error;

  // Get Drive Shown Settings
  script = @"tell application \"Finder\" to {"
             @"desktop shows hard disks of Finder preferences,"
             @"desktop shows external hard disks of Finder preferences,"
             @"desktop shows removable media of Finder preferences,"
             @"desktop shows connected servers of Finder preferences"
           @"}";

  NSAppleScript *drive_shown_script = [[NSAppleScript alloc] initWithSource:script];
  script_return = [drive_shown_script executeAndReturnError:&error];
  [drive_shown_script release];

  NSLog(@"Loading Finder Preferences:\n");

  // If script ran properly
  if (script_return != nil) {
    NSLog(@"Success!\n");
    desktop_shows_hard_disks_ = [[script_return descriptorAtIndex:1] booleanValue];
    desktop_shows_external_hard_disks_ = [[script_return descriptorAtIndex:2] booleanValue];
    desktop_shows_removable_media_ = [[script_return descriptorAtIndex:3] booleanValue];
    desktop_shows_connected_servers_ = [[script_return descriptorAtIndex:4] booleanValue];
  } else {
    NSLog(@"Error: %@\n", [error valueForKey:NSAppleScriptErrorMessage]);
    desktop_shows_hard_disks_ = true;
    desktop_shows_external_hard_disks_ = true;
    desktop_shows_removable_media_ = true;
    desktop_shows_connected_servers_ = true;
  }

  // Get Dock Settings
  script = @"tell application \"System Events\" to {screen edge,autohide} of dock preferences";
  NSAppleScript* dock_script = [[NSAppleScript alloc] initWithSource:script];
  script_return = [dock_script executeAndReturnError:nil];
  [dock_script release];

  NSString* position = [[script_return descriptorAtIndex:1] stringValue];
  if ([position isEqualToString:@"bott"]) {
    set_dock_position(BOTTOM_DOCK);
  } else if ([position isEqualToString:@"left"]) {
    set_dock_position(LEFT_DOCK);
  } else if ([position isEqualToString:@"righ"]) {
    set_dock_position(RIGHT_DOCK);
  }

  bool has_auto_hide_dock = [[script_return descriptorAtIndex:2] booleanValue];
  set_has_auto_hide_dock(has_auto_hide_dock);
}

bool AppSettings::desktop_shows_hard_disks() {
  return desktop_shows_hard_disks_;
}

bool AppSettings::desktop_shows_external_hard_disks() {
  return desktop_shows_external_hard_disks_;
}

bool AppSettings::desktop_shows_removable_media() {
  return desktop_shows_removable_media_;
}

bool AppSettings::desktop_shows_connected_servers() {
  return desktop_shows_connected_servers_;
}
