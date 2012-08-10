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

#include <gtest/gtest.h>

#include "BumpTop/DesktopItems.h"
#include "BumpTop/QStringHelpers.h"

namespace {
  class DesktopItemsTest : public ::testing::Test {
  };

  TEST_F(DesktopItemsTest, Item_gets_added_to_desktop_SLOW_TEST) {
    NSArray* paths;
    paths = NSSearchPathForDirectoriesInDomains(NSDesktopDirectory, NSUserDomainMask, NO);
    NSString* fullPathToDesktop = [[paths objectAtIndex:0] stringByExpandingTildeInPath];
    NSString* test_file_path = [fullPathToDesktop stringByAppendingPathComponent:@"test_file.txt"];
    [[NSFileManager defaultManager] createFileAtPath:test_file_path contents:nil attributes:nil];

    // Seems to take a second or so before the item will be registered by the AppleScript on the desktop.
    sleep(1.0);
    std::vector<DesktopItem> desktop_items = getDesktopItems();

    NSString *iter_string;
    BOOL found_flag = NO;
    for (int i = 0; i < desktop_items.size(); i++) {
      iter_string = NSStringFromQString(desktop_items[i].file_path);
      NSLog(iter_string);
      if ([test_file_path isEqualToString:iter_string]) {
        found_flag = YES;
      }
    }
    [[NSFileManager defaultManager] removeFileAtPath:test_file_path handler:nil];
    ASSERT_EQ(found_flag, YES);
  }

}  // namespace
