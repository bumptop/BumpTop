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

#import "BumpTop/DesktopItems.h"

#include "BumpTop/DebugAssert.h"
#include "BumpTop/FileManager.h"
#include "BumpTop/QStringHelpers.h"

// This function iterates through the Desktop items and returns an array of their path, location and their icon.
std::vector<DesktopItem> getDesktopItems() {
  std::vector<DesktopItem> desktop_items;
  desktop_items = getDrives();

  NSString* script = @"tell application \"Finder\"\n"
                       @"{URL, desktop position} of items\n"
                     @"end tell";
  NSAppleScript *desktop_script = [[NSAppleScript alloc] initWithSource:script];
  NSAppleEventDescriptor *desktop_script_return = [desktop_script executeAndReturnError:nil];

  // This needs to be a debug assert since it appears we are getting some users failing this
  // Since it's a bad place to fail, we try again two more times (below) before giving up.
  DEBUG_ASSERT([desktop_script_return numberOfItems] == 2);

  // This little loop is designed to try getting a valid return from the apple script
  // by retrying up to two more times. If after trying a total of 3 times, nothing valid is
  // returned, we fail.
  bool try_again = [desktop_script_return numberOfItems] != 2;
  int number_of_tries = 1;
  while (try_again) {
    if (number_of_tries >= 2) {
      return desktop_items;
    }
    desktop_script_return = [desktop_script executeAndReturnError:nil];
    try_again =  [desktop_script_return numberOfItems] != 2;
    number_of_tries++;
  }

  NSAppleEventDescriptor *urls = [desktop_script_return descriptorAtIndex:1];
  NSAppleEventDescriptor *desktop_positions = [desktop_script_return descriptorAtIndex:2];

  for (int i = 1; i <= [urls numberOfItems]; i++) {
    NSAppleEventDescriptor *url = [urls descriptorAtIndex:i];
    NSAppleEventDescriptor *desktop_position = [desktop_positions descriptorAtIndex:i];

    if (FileManager::getFileKind(QStringFromNSString([url stringValue])) != VOLUME
        && ![[url stringValue] isEqualToString:@"file://localhost/"]) {
      desktop_items.push_back(createDesktopItem(url, desktop_position));
    }
  }

  [desktop_script release];
  return desktop_items;
}

std::vector<DesktopItem> getDrives() {
  std::vector<DesktopItem> drives;

  // One index-matched query for every disk. Disks are identified by their
  // mount-point URL, which stays unique even when two volumes share a display
  // name (the old per-disk name lookup gave same-named volumes the position
  // of whichever Finder resolved first).
  NSString* script_source = @"tell application \"Finder\"\n"
                            @"{URL, desktop position, ejectable, local volume} of disks\n"
                            @"end tell";
  NSAppleScript* desktop_script_for_disks = [[NSAppleScript alloc] initWithSource:script_source];
  NSAppleEventDescriptor* script_result = [desktop_script_for_disks executeAndReturnError:nil];
  [desktop_script_for_disks release];
  if ([script_result numberOfItems] != 4) {
    return drives;
  }

  NSAppleEventDescriptor* urls = [script_result descriptorAtIndex:1];
  NSAppleEventDescriptor* positions = [script_result descriptorAtIndex:2];
  NSAppleEventDescriptor* ejectables = [script_result descriptorAtIndex:3];
  NSAppleEventDescriptor* local_volumes = [script_result descriptorAtIndex:4];

  for (int i = 1; i <= [urls numberOfItems]; i++) {
    DesktopItem item = createDesktopItem([urls descriptorAtIndex:i],
                                         [positions descriptorAtIndex:i]);
    if (item.file_path == "")
      continue;
    if (item.file_path == "/") {
      // The startup disk mounts at "/"; its desktop item lives behind the
      // /Volumes symlink.
      QDir volumes_dir("/Volumes");
      for_each(QFileInfo drive_info, volumes_dir.entryInfoList()) {
        if (drive_info.symLinkTarget() == "/") {
          item.file_path = drive_info.absoluteFilePath();
          break;
        }
      }
      if (item.file_path == "/")
        continue;
    }
    // Only volumes mounted under /Volumes appear on the desktop; the disks
    // query also returns hidden system volumes (Preboot, iSCPreboot, ...).
    if (!item.file_path.startsWith("/Volumes/"))
      continue;
    if (!QFileInfo(item.file_path).exists())
      continue;

    drives.push_back(item);
    if ([[ejectables descriptorAtIndex:i] booleanValue]) {
      FileManager::addEjectableDrive(item.file_path);
    }
    if (![[local_volumes descriptorAtIndex:i] booleanValue]) {
      // Connected Servers not recognized as ejectable by applescript.
      FileManager::addEjectableDrive(item.file_path);
      FileManager::addConnectedServer(item.file_path);
    }
  }

  return drives;
}

DesktopItem createDesktopItem(NSAppleEventDescriptor *url,
                              NSAppleEventDescriptor *desktop_position) {
  NSString *item_path;
  NSURL *item_url;
  DesktopItem desktop_item;
  int x;
  int y;

  item_url = [NSURL URLWithString:[url stringValue]];

  x = [[desktop_position descriptorAtIndex:1] int32Value];
  y = [[desktop_position descriptorAtIndex:2] int32Value];
  item_path = [item_url path];

  // Did not fundamentally solve the problem of null [item_url path], which occurs occasionally.
  // May potentially be due to path implementation:
  // http://developer.apple.com/mac/library/documentation/Cocoa/Reference/Foundation/Classes/NSURL_Class/Reference/Reference.html
  if (item_path != NULL)
    desktop_item.file_path = QStringFromNSString(item_path);

  desktop_item.position_x = x;
  desktop_item.position_y = y;
  return desktop_item;
}
