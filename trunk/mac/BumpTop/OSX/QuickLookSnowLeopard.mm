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

#  if (MAC_OS_X_VERSION_MAX_ALLOWED == MAC_OS_X_VERSION_10_6)

#import "BumpTop/OSX/QuickLookSnowLeopard.h"

// unfortunately Quartz must be here and not in precompiled headers, since including it there would
// cause conflicts in QuickLookLeopard (compiler would complain about two symbols being defined)
#import <Quartz/Quartz.h>

#include "BumpTop/BumpTopScene.h"
#include "BumpTop/OSX/OSXCocoaBumpTopApplication.h"
#include "BumpTop/QStringHelpers.h"
#include "BumpTop/Room.h"

@interface QuickLookPublic : NSResponder <QLPreviewPanelDataSource, QLPreviewPanelDelegate> {
  QLPreviewPanel* preview_panel_;
  NSArray* preview_items_;
}
+ (QuickLookPublic*) singleton;
- (void) setPreviewItemPaths:(QHash<QString, VisualPhysicsActorId>)paths;

@end

// PreviewItem will be used directlty as the items in preview panel
// The class just need to implement the QLPreviewItem protocol
@interface PreviewItem : NSObject {
  NSURL* item_url;
  VisualPhysicsActorId actor_id;
}
@property(retain) NSURL* item_url;
@property VisualPhysicsActorId actor_id;

@end

void SnowLeopardQuickLookInterface::previewPathsInSharedPreviewPanel(QHash<QString, VisualPhysicsActorId> paths) {
  setSharedPreviewPanelPaths(paths);
  [[QLPreviewPanel sharedPreviewPanel] makeKeyAndOrderFront:nil];
}

bool SnowLeopardQuickLookInterface::sharedPreviewPanelIsOpen() {
  return ([QLPreviewPanel sharedPreviewPanelExists] && [[QLPreviewPanel sharedPreviewPanel] isVisible]);
}

void SnowLeopardQuickLookInterface::closeSharedPreviewPanel() {
  if ([QLPreviewPanel sharedPreviewPanelExists]) {
    [[QLPreviewPanel sharedPreviewPanel] orderOut:nil];
  }
}

void SnowLeopardQuickLookInterface::setSharedPreviewPanelPaths(QHash<QString, VisualPhysicsActorId> paths) {
  [[QuickLookPublic singleton] setPreviewItemPaths:paths];
}

// PreviewItem
// *****************************

// Defines the interface which lets PreviewItem
// follow the QLPreviewItem protocol
@interface PreviewItem (QLPreviewItem) <QLPreviewItem>
@end

@implementation PreviewItem (QLPreviewItem)
- (NSURL *)previewItemURL {
  return self.item_url;
}
@end

@implementation PreviewItem
@synthesize item_url;
@synthesize actor_id;
@end
// *****************************


// QuickLookPublic
// *****************************
@implementation QuickLookPublic

- (void) setPreviewItemPaths:(QHash<QString, VisualPhysicsActorId>)paths {
  NSMutableArray* item_array = [NSMutableArray arrayWithCapacity:paths.count()];
  for_each(QString path, paths.keys()) {
    PreviewItem* item = [[PreviewItem alloc] init];
    item.item_url = [NSURL fileURLWithPath:NSStringFromQString(path)];
    item.actor_id = paths[path];
    [item_array addObject:item];
    [item release];
  }

  if (preview_items_ != nil) {
    [preview_items_ release];
  }

  preview_items_ = [item_array retain];

  if (preview_panel_ != nil) {
    [preview_panel_ reloadData];
  }
}

static QuickLookPublic *singleton_instance = NULL;

+ (QuickLookPublic *)singleton {
  @synchronized(self)
  {
    if (singleton_instance == NULL) {
      singleton_instance = [[self alloc] init];
      OgreView* ogre_view = OSXCocoaBumpTopApplication::singleton()->ogre_view();
      [singleton_instance setNextResponder: [[ogre_view nextResponder] nextResponder]];
      [[ogre_view nextResponder] setNextResponder: singleton_instance ];
    }
  }
  return(singleton_instance);
}


// Quick Look panel support
- (BOOL)acceptsPreviewPanelControl:(QLPreviewPanel *)panel  {
  return YES;
}

- (void)beginPreviewPanelControl:(QLPreviewPanel *)panel {
  // This document is now responsible of the preview panel
  // It is allowed to set the delegate, data source and refresh panel.
  preview_panel_ = [panel retain];
  panel.delegate = self;
  panel.dataSource = self;
}

- (void)endPreviewPanelControl:(QLPreviewPanel *)panel {
  // This document loses its responsisibility on the preview panel
  // Until the next call to -beginPreviewPanelControl: it must not
  // change the panel's delegate, data source or refresh it.
  [preview_panel_ release];
  preview_panel_ = nil;
}

// Quick Look panel data source
- (NSInteger)numberOfPreviewItemsInPreviewPanel:(QLPreviewPanel *)panel {
  return [preview_items_ count];
}

- (id <QLPreviewItem>)previewPanel:(QLPreviewPanel *)panel previewItemAtIndex:(NSInteger)index {
  return [preview_items_ objectAtIndex:index];
}

// Quick Look panel delegate
/*
- (BOOL)previewPanel:(QLPreviewPanel *)panel handleEvent:(NSEvent *)event {
  return NO;
}
*/

// This delegate method provides the rect on screen from which the panel will zoom.
- (NSRect)previewPanel:(QLPreviewPanel *)panel sourceFrameOnScreenForPreviewItem:(id <QLPreviewItem>)item {

  NSInteger index = [preview_items_ indexOfObject:item];
  if (index == NSNotFound) {
    return NSZeroRect;
  }

  PreviewItem* preview_item = (PreviewItem*) [preview_items_ objectAtIndex:index];

  VisualPhysicsActor* actor = OSXCocoaBumpTopApplication::singleton()->scene()->room()->actor_with_unique_id(preview_item.actor_id);
  if (actor == NULL) {
    return NSZeroRect;
  } else {
    Ogre::Vector2 screen_position = OSXCocoaBumpTopApplication::singleton()->cocoaCoordinatesToOgreCoordinates(actor->getScreenPosition());

    return NSMakeRect(screen_position.x-50, screen_position.y-50, 100, 100);
  }
}

// This delegate method provides a transition image between the table view and the preview panel
/*
- (id)previewPanel:(QLPreviewPanel *)panel transitionImageForPreviewItem:(id <QLPreviewItem>)item contentRect:(NSRect *)contentRect {
  return nil;
}
*/

@end
#endif
// *****************************

