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
  // directory Volumes contains all mounted drives including the startup drive
  QDir dir = QDir("/Volumes");
  QFileInfoList drives_info = dir.entryInfoList();
  std::vector<DesktopItem> drives;

  // get properites of each drive in the directory Volumes
  for_each(QFileInfo drive_info, drives_info) {
    // We want to ignore the "/" and "/Volumes" returned by entryInfoList
    if (drive_info.absoluteFilePath().startsWith("/Volumes/")) {
      if (drive_info.readLink() == "" || drive_info.readLink() == "/") {
        // Apple stores all its mounted drives and a link to the startup disk in folder "/Volumes"
        // the original path of the startup disk is "/".
        // Sometimes other special items for example the iDisk can get into Volumes, these items are not drives but links
        // to drives so in this case we don't want to include them in room (Apple does not include them on desktop as well)

        NSString* script_format = @"tell application \"Finder\"\n"
                                      @"set macpath to POSIX file \"/%@\" as text\n"
                                      @"{URL, desktop position,ejectable,local volume} of item macpath\n"
                                  @"end tell";
        NSString* script = [NSString stringWithFormat:script_format, NSStringFromQString(drive_info.absoluteFilePath())];

        NSAppleScript *desktop_script_for_disks = [[NSAppleScript alloc] initWithSource:script];
        NSAppleEventDescriptor *desktop_script_for_disks_return = [desktop_script_for_disks executeAndReturnError:nil];

        //DEBUG_ASSERT([desktop_script_for_disks_return numberOfItems] == 4);
        // TODO: Why is it that sometimes we're not getting a proper return from this script?
        if ([desktop_script_for_disks_return numberOfItems] != 4) {
          continue;
        }

        NSAppleEventDescriptor *url = [desktop_script_for_disks_return descriptorAtIndex:1];
        NSAppleEventDescriptor *desktop_position = [desktop_script_for_disks_return descriptorAtIndex:2];
        bool ejectable = [[desktop_script_for_disks_return descriptorAtIndex:3] booleanValue];
        bool local_volume = [[desktop_script_for_disks_return descriptorAtIndex:4] booleanValue];

        // Drives will _always_ be desktop items, so the below line is ok, as it they will always have a position
        DesktopItem item = createDesktopItem(url, desktop_position);
        // the URL from applescript is not the same as its actual URL for some reason so we are just gonna use the
        // path of file using QFileInfo but we will still be using the positions received from applescript
        item.file_path = drive_info.absoluteFilePath();
        drives.push_back(item);
        if (ejectable) {
          FileManager::addEjectableDrive(item.file_path);
        }
        if (!local_volume) {
          // Connected Servers not recognized as ejectable by applescript.
          FileManager::addEjectableDrive(item.file_path);
          FileManager::addConnectedServer(item.file_path);
        }
        [desktop_script_for_disks release];
      }
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
