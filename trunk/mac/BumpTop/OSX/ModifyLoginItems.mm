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

// Source extracted sample project at:
// http://carpeaqua.com/2008/03/01/adding-an-application-to-login-items-in-mac-os-x-leopard/
// and modified slightly

#include "BumpTop/AppSettings.h"
#include "BumpTop/QStringHelpers.h"
#import "BumpTop/OSX/ModifyLoginItems.h"

void LoginItems::addOrRepairLoginItemIfRequired() {

  AppSettings::singleton()->loadSettingsFile();
  // if the setting doesn't exist, then we should add a login item with the current path
  if (!AppSettings::singleton()->has_bumptop_should_start_on_login()) {
    AppSettings::singleton()->set_bumptop_should_start_on_login(true);
    AppSettings::singleton()->set_bumptop_login_item_path(utf8(QStringFromNSString([[NSBundle mainBundle] bundlePath])));
    [ModifyLoginItems addApplicationAsLoginItem:[[NSBundle mainBundle] bundlePath]];
  }
  syncBumpTopShouldStartOnLoginWithSystemPreferences();
  if (AppSettings::singleton()->bumptop_should_start_on_login()) {
    if (QStringFromUtf8(AppSettings::singleton()->bumptop_login_item_path()) != QStringFromNSString([[NSBundle mainBundle] bundlePath])) {
      [ModifyLoginItems removeApplicationAsLoginItem:NSStringFromUtf8(AppSettings::singleton()->bumptop_login_item_path())];
      AppSettings::singleton()->set_bumptop_login_item_path(utf8(QStringFromNSString([[NSBundle mainBundle] bundlePath])));
      [ModifyLoginItems addApplicationAsLoginItem:[[NSBundle mainBundle] bundlePath]];
    }
  } else {
    if (QStringFromUtf8(AppSettings::singleton()->bumptop_login_item_path()) != QStringFromNSString([[NSBundle mainBundle] bundlePath]))
      AppSettings::singleton()->set_bumptop_login_item_path(utf8(QStringFromNSString([[NSBundle mainBundle] bundlePath])));
  }
  AppSettings::singleton()->saveSettingsFile();
}

void LoginItems::syncBumpTopShouldStartOnLoginWithSystemPreferences() {
  AppSettings::singleton()->loadSettingsFile();
  BOOL bumptop_login_item_exists = [ModifyLoginItems hasLoginItem:NSStringFromUtf8(AppSettings::singleton()->bumptop_login_item_path())];
  if (bumptop_login_item_exists != AppSettings::singleton()->bumptop_should_start_on_login())
    AppSettings::singleton()->set_bumptop_should_start_on_login(bumptop_login_item_exists);

  AppSettings::singleton()->saveSettingsFile();
}

void LoginItems::removeBumpTopFromLoginItems() {
  [ModifyLoginItems removeApplicationAsLoginItem:NSStringFromUtf8(AppSettings::singleton()->bumptop_login_item_path())];
}

void LoginItems::addBumpTopToLoginItems() {
  [ModifyLoginItems addApplicationAsLoginItem:NSStringFromUtf8(AppSettings::singleton()->bumptop_login_item_path())];
}

@implementation ModifyLoginItems

+ (void)enableLoginItemWithLoginItemsReference:(LSSharedFileListRef )theLoginItemsRefs ForPath:(CFURLRef)thePath {
  // We call LSSharedFileListInsertItemURL to insert the item at the bottom of Login Items list.
  LSSharedFileListItemRef item = LSSharedFileListInsertItemURL(theLoginItemsRefs, kLSSharedFileListItemLast,
                                                               NULL, NULL, thePath, NULL, NULL);
  if (item) {
    CFRelease(item);
  }
}

+ (id)loginItemWithLoginItemsReference:(LSSharedFileListRef )theLoginItemsRefs ForPath:(CFURLRef)thePath {
  UInt32 seedValue;
  id return_item = NULL;
  // We're going to grab the contents of the shared file list (LSSharedFileListItemRef objects)
  // and pop it in an array so we can iterate through it to find our item.
  NSArray  *loginItemsArray = (NSArray *)LSSharedFileListCopySnapshot(theLoginItemsRefs, &seedValue);
  for (id item in loginItemsArray) {
    CFURLRef itemPath;
    LSSharedFileListItemRef itemRef = (LSSharedFileListItemRef)item;
    if (LSSharedFileListItemResolve(itemRef, 0, (CFURLRef*) &itemPath, NULL) == noErr) {
      if ([[(NSURL *)itemPath path] isEqualToString:[(NSURL*)thePath path]]) {
        return_item = [item retain];
        break;
      }
    }
  }
  [loginItemsArray release];
  return return_item;
}

+ (void)addApplicationAsLoginItem:(NSString*) application_path {
  CFURLRef url = (CFURLRef)[NSURL fileURLWithPath:application_path];
  // Create a reference to the shared file list.
  LSSharedFileListRef loginItems = LSSharedFileListCreate(NULL, kLSSharedFileListSessionLoginItems, NULL);
  if (loginItems) {
    [self enableLoginItemWithLoginItemsReference:loginItems ForPath:url];
  }
  CFRelease(loginItems);
}

+ (void)removeApplicationAsLoginItem:(NSString*) application_path {
  CFURLRef url = (CFURLRef)[NSURL fileURLWithPath:application_path];

  // Create a reference to the shared file list.
  LSSharedFileListRef loginItems = LSSharedFileListCreate(NULL, kLSSharedFileListSessionLoginItems, NULL);
  if (loginItems) {
    id item = [self loginItemWithLoginItemsReference:loginItems ForPath:url];
    LSSharedFileListItemRef itemRef = (LSSharedFileListItemRef) item;
    if (itemRef != NULL) {
      LSSharedFileListItemRemove(loginItems, itemRef);
      [item release];
    }
  }
  CFRelease(loginItems);
}

+ (BOOL)hasLoginItem:(NSString*) application_path {
  CFURLRef url = (CFURLRef)[NSURL fileURLWithPath:application_path];
  BOOL has_item = NO;
  // Create a reference to the shared file list.
  LSSharedFileListRef loginItems = LSSharedFileListCreate(NULL, kLSSharedFileListSessionLoginItems, NULL);
  if (loginItems) {
    id itemRef = [self loginItemWithLoginItemsReference:loginItems ForPath:url];
    if (itemRef != NULL) {
      has_item = YES;
      [itemRef release];
    }
  }
  CFRelease(loginItems);
  return has_item;
}

@end
