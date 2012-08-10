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

  Point point;
  point.h = mouse_in_window_space.x;
  point.v = mouse_in_window_space.y;

  AEDescList file_list;
  OSStatus err;


  // Create a AEDesc for the file
  // example that this is based off of:
  // http://devworld.apple.com/technotes/tn/tn1002.html

  AEDesc file_list_element;
  AECreateList(NULL, 0, false, &file_list);
  bool use_ae_desc_list = false;
  for_each(VisualPhysicsActor* actor, actors) {
    if (QFileInfo(actor->path()).exists()) {
      use_ae_desc_list = true;

      AliasHandle file_alias;
      Boolean is_directory = false;

      err = FSNewAliasFromPath(NULL, utf8(actor->path()).c_str(), 0,  &file_alias, &is_directory);

      HLock((Handle) file_alias);

      AECreateDesc(typeAlias, (Ptr) (*file_alias),
                   GetHandleSize((Handle) file_alias), &file_list_element);
      HUnlock((Handle) file_alias);

      AEPutDesc(&file_list, 0, &file_list_element);
    }

  }

  UInt32 user_selection_type;
  MenuID menu_id;
  MenuItemIndex menu_item_index;

  IBNibRef nib_ref;
  err = CreateNibReference(CFSTR("ContextMenu"), &nib_ref);
  MenuRef carbon_context_menu;
  err = CreateMenuFromNib(nib_ref, CFSTR("ItemContextMenu"), &carbon_context_menu);
  // change double separators to just one separator:
  err = ChangeMenuAttributes(carbon_context_menu, kMenuAttrCondenseSeparators, NULL);

  DisposeNibReference(nib_ref);

  int num_submenu_items = 0;
  int menu_size = 0;
  for_each(BumpTopCommand* option, ordered_context_menu_items) {
    err = InsertMenuItemTextWithCFString(carbon_context_menu,
                                         CFStringFromQString(option->name()),
                                         menu_size,   // MenuItemIndex inAfterItem
                                         option->isSeparator() ? kMenuItemAttrSeparator : 0,  // MenuItemAttributes inAttributes
                                         0);  // MenuCommand inCommandID
    if (option->has_subcommands()) {
      QStringList subcommand_names = option->subcommand_names(actors);
      err = CreateNibReference(CFSTR("ContextMenu"), &nib_ref);
      MenuRef carbon_context_sub_menu;
      err = CreateMenuFromNib(nib_ref, CFSTR("ItemContextMenu"), &carbon_context_sub_menu);
      err = ChangeMenuAttributes(carbon_context_sub_menu, kMenuAttrCondenseSeparators, NULL);
      DisposeNibReference(nib_ref);


      int sub_menu_size = 0;
      for_each(QString subcommand, subcommand_names) {
        err = InsertMenuItemTextWithCFString(carbon_context_sub_menu,
                                             CFStringFromQString(subcommand),
                                             sub_menu_size,   // MenuItemIndex inAfterItem
                                             subcommand == "" ? kMenuItemAttrSeparator : 0,  // MenuItemAttributes inAttributes
                                             0); // MenuCommand inCommandID

        CGImageRef icon = option->iconForSubcommand(actors, sub_menu_size);
        if (icon) {
          err = SetMenuItemIconHandle (carbon_context_sub_menu,
                                       sub_menu_size+1,
                                       kMenuCGImageRefType,
                                       (Handle) icon);
          CGImageRelease(icon);
        }
        sub_menu_size++;

      }
      num_submenu_items = sub_menu_size;

      SetMenuID(carbon_context_sub_menu, menu_size);
      SetMenuItemHierarchicalMenu(carbon_context_menu, menu_size+1, carbon_context_sub_menu);
    }

    menu_size += 1;
  }
  SetMenuID(carbon_context_menu, -1);

  // This is a bit hacky, but before we launch the context menu we want to call a mouse up so as to ensure
  // that any drag operations are ended properly -- this is required since the context menu will receive
  // the next mosue event, and hence BumpTop may get stuck in a dragging state
  Ogre::Vector2 mouse_location = BumpTopApp::singleton()->mouse_location();
  BumpTopApp::singleton()->mouseUp(mouse_location.x, mouse_location.y, 1, NO_KEY_MODIFIERS_MASK);

  BumpTopApp::singleton()->set_context_menu_open(true);
  // http://developer.apple.com/documentation/Carbon/Reference/Menu_Manager/Reference/reference.html#//apple_ref/c/func/ContextualMenuSelect
  err = ContextualMenuSelect(carbon_context_menu,
                             point,
                             false,
                             kCMHelpItemNoHelp,
                             NULL,
                             use_ae_desc_list ? NULL : &file_list,
                             &user_selection_type,
                             &menu_id,
                             &menu_item_index);
  BumpTopApp::singleton()->set_context_menu_open(false);

  if ((    (menu_id == -1 && menu_item_index <= ordered_context_menu_items.size())
        || (menu_id != -1 && menu_item_index <= num_submenu_items))
      && user_selection_type == kCMMenuItemSelected) {
    // If menu_id is -1 the user selected an option from the root context menu
    if (menu_id == -1) {
      DEBUG_ASSERT(ordered_context_menu_items.size() >= menu_item_index &&
                   ordered_context_menu_items.value(menu_item_index - 1) != NULL);
      if (ordered_context_menu_items.size() >= menu_item_index &&
          ordered_context_menu_items.value(menu_item_index - 1) != NULL) {

        ordered_context_menu_items.value(menu_item_index - 1)->applyToActors(env, actors);
      }
    // If menu_id is not -1, the user selected an option from a submenu
    } else {
#ifdef DEBUG
      assert(ordered_context_menu_items.size() > menu_id &&
             ordered_context_menu_items.value(menu_id) != NULL);
#endif
      if (ordered_context_menu_items.size() > menu_id &&
          ordered_context_menu_items.value(menu_id) != NULL) {
        ordered_context_menu_items.value(menu_id)->applyToActors(env, actors, menu_item_index - 1);
      }
    }
  }
}

// Show package contents: use this: tell application "System Events" to get package folder of alias POSIX file "/Users/web/Desktop/BumpTop.app"
//

