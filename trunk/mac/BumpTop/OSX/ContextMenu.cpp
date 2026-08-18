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

#include "BumpTop/OSX/ContextMenu.h"

#include "BumpTop/Authorization.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/BumpTopScene.h"
#include "BumpTop/DebugAssert.h"
#include "BumpTop/OSX/EventModifierFlags.h"
#include "BumpTop/FileItem.h"
#include "BumpTop/FileManager.h"
#include "BumpTop/QStringHelpers.h"
#include "BumpTop/VisualPhysicsActor.h"

typedef void (*ContextMenuItemFunction)(const QString& file_path);

// Captures the menu item picked during the synchronous NSMenu tracking loop.
@interface BTContextMenuTarget : NSObject {
 @public
  NSMenuItem* picked_item;
}
- (void)menuItemPicked:(id)sender;
@end

@implementation BTContextMenuTarget
- (void)menuItemPicked:(id)sender {
  picked_item = (NSMenuItem*)sender;
}
@end

void launchContextMenu(const BumpEnvironment& env,
                       const VisualPhysicsActorList& actors, Ogre::Vector2 mouse_in_window_space) {
  assert(actors.size() > 0);
  BumpTopCommandSet supported_context_menu_items;
  bool first_actor = true;
  for_each(VisualPhysicsActor* actor, actors) {
    if (first_actor) {
      supported_context_menu_items = BumpTopCommandSet(*actor->supported_context_menu_items());  // make a copy
      first_actor = false;
    } else {
      supported_context_menu_items = supported_context_menu_items.intersect(*actor->supported_context_menu_items());
    }
  }

  BumpTopCommandSet context_menu_items_to_remove;
  for_each(BumpTopCommand* option, supported_context_menu_items) {
    if (!option->canBeAppliedToActors(env, actors) || option->is_toolbar_command()) {
      context_menu_items_to_remove.insert(option);
    }
  }
  supported_context_menu_items.subtract(context_menu_items_to_remove);
  launchContextMenu(env, actors, &supported_context_menu_items, mouse_in_window_space);
}

bool contextMenuItemLessThan(BumpTopCommand* item1, BumpTopCommand* item2)
{
  if (item1->number_of_separators_above_me() == item2->number_of_separators_above_me()) {
    return item1->position_within_my_category() < item2->position_within_my_category();
  }
  return item1->number_of_separators_above_me() < item2->number_of_separators_above_me();
}

void launchContextMenu(const BumpEnvironment& env, const VisualPhysicsActorList& actors,
                       BumpTopCommandSet* context_menu_items, Ogre::Vector2 mouse_in_window_space) {

  QList<BumpTopCommand*> ordered_context_menu_items = context_menu_items->values();

  qSort(ordered_context_menu_items.begin(),
        ordered_context_menu_items.end(),
        contextMenuItemLessThan);

  QList<SeparatorBumpTopCommand*> separators_to_add;

  if (ordered_context_menu_items.size() > 0) {
    int num_separators_above_previous_item = ordered_context_menu_items.value(0)->number_of_separators_above_me();

    int menu_index = 0;
    for_each(BumpTopCommand* menu_item, ordered_context_menu_items) {
      if (menu_item->number_of_separators_above_me() != num_separators_above_previous_item) {
        separators_to_add.append(new SeparatorBumpTopCommand(menu_index + separators_to_add.size()));
      }
      num_separators_above_previous_item = menu_item->number_of_separators_above_me();
      menu_index += 1;
    }
    for_each(SeparatorBumpTopCommand* separator, separators_to_add)
      ordered_context_menu_items.insert(separator->menu_index(), separator);
  }

  // Modern port: the Carbon Menu Manager (ContextualMenuSelect et al.) was
  // removed from macOS, so the menu is built and tracked with NSMenu instead.
  // popUpMenuPositioningItem: runs a synchronous tracking loop, and the picked
  // item is captured by BTContextMenuTarget before it returns.
  BTContextMenuTarget* menu_target = [[BTContextMenuTarget alloc] init];
  NSMenu* context_menu = [[NSMenu alloc] initWithTitle:@"ItemContextMenu"];
  [context_menu setAutoenablesItems:NO];

  int menu_size = 0;
  for_each(BumpTopCommand* option, ordered_context_menu_items) {
    NSMenuItem* menu_item;
    if (option->isSeparator()) {
      menu_item = (NSMenuItem*)[NSMenuItem separatorItem];
      [context_menu addItem:menu_item];
    } else {
      menu_item = [[NSMenuItem alloc] initWithTitle:NSStringFromQString(option->name())
                                             action:@selector(menuItemPicked:)
                                      keyEquivalent:@""];
      [menu_item setTarget:menu_target];
      [context_menu addItem:menu_item];
      [menu_item release];
    }
    [menu_item setTag:menu_size];

    if (option->has_subcommands()) {
      QStringList subcommand_names = option->subcommand_names(actors);
      NSMenu* sub_menu = [[NSMenu alloc] initWithTitle:NSStringFromQString(option->name())];
      [sub_menu setAutoenablesItems:NO];

      int sub_menu_size = 0;
      for_each(QString subcommand, subcommand_names) {
        NSMenuItem* sub_menu_item;
        if (subcommand == "") {
          sub_menu_item = (NSMenuItem*)[NSMenuItem separatorItem];
          [sub_menu addItem:sub_menu_item];
        } else {
          sub_menu_item = [[NSMenuItem alloc] initWithTitle:NSStringFromQString(subcommand)
                                                     action:@selector(menuItemPicked:)
                                              keyEquivalent:@""];
          [sub_menu_item setTarget:menu_target];
          [sub_menu addItem:sub_menu_item];
          [sub_menu_item release];
        }
        [sub_menu_item setTag:sub_menu_size];

        CGImageRef icon = option->iconForSubcommand(actors, sub_menu_size);
        if (icon) {
          NSImage* icon_image = [[NSImage alloc] initWithCGImage:icon size:NSMakeSize(16, 16)];
          [sub_menu_item setImage:icon_image];
          [icon_image release];
          CGImageRelease(icon);
        }
        sub_menu_size++;
      }
      [menu_item setSubmenu:sub_menu];
      [sub_menu release];
    }

    menu_size += 1;
  }

  // This is a bit hacky, but before we launch the context menu we want to call a mouse up so as to ensure
  // that any drag operations are ended properly -- this is required since the context menu will receive
  // the next mosue event, and hence BumpTop may get stuck in a dragging state
  Ogre::Vector2 mouse_location = BumpTopApp::singleton()->mouse_location();
  BumpTopApp::singleton()->mouseUp(mouse_location.x, mouse_location.y, 1, NO_KEY_MODIFIERS_MASK);

  // mouse_in_window_space uses a top-left origin; NSMenu wants bottom-left
  // screen coordinates.
  CGFloat screen_height = [[[NSScreen screens] objectAtIndex:0] frame].size.height;
  NSPoint popup_point = NSMakePoint(mouse_in_window_space.x, screen_height - mouse_in_window_space.y);

  BumpTopApp::singleton()->set_context_menu_open(true);
  [context_menu popUpMenuPositioningItem:nil atLocation:popup_point inView:nil];
  BumpTopApp::singleton()->set_context_menu_open(false);

  NSMenuItem* picked = menu_target->picked_item;
  if (picked != nil) {
    NSMenuItem* parent_item = [picked parentItem];
    if (parent_item == nil) {
      // Picked from the root context menu
      int index = (int)[picked tag];
      if (index < ordered_context_menu_items.size() &&
          ordered_context_menu_items.value(index) != NULL) {
        ordered_context_menu_items.value(index)->applyToActors(env, actors);
      }
    } else {
      // Picked from a submenu
      int parent_index = (int)[parent_item tag];
      int sub_index = (int)[picked tag];
      if (parent_index < ordered_context_menu_items.size() &&
          ordered_context_menu_items.value(parent_index) != NULL) {
        ordered_context_menu_items.value(parent_index)->applyToActors(env, actors, sub_index);
      }
    }
  }

  [context_menu release];
  [menu_target release];
}

// Show package contents: use this: tell application "System Events" to get package folder of alias POSIX file "/Users/web/Desktop/BumpTop.app"
//

