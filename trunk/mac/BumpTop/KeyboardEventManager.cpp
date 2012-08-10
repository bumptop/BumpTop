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

#include "BumpTop/KeyboardEventManager.h"

#include "BumpTop/Authorization.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/BumpTopScene.h"
#include "BumpTop/FileManager.h"
#include "BumpTop/OSX/EventModifierFlags.h"
#include "BumpTop/PerformanceStatsHUD.h"
#include "BumpTop/Processes.h"
#include "BumpTop/Room.h"
#include "BumpTop/RoomSurface.h"
#include "BumpTop/SearchBox.h"
#include "BumpTop/QuickLookPreviewPanel.h"

KeyboardEvent::KeyboardEvent(QString characters, int modifier_flags)
: characters(characters),
  modifier_flags(modifier_flags) {
}

KeyboardEventManager::KeyboardEventManager(BumpTopApp* bumptop)
: bumptop_(bumptop),
  command_key_down_(false),
  option_key_down_(false) {
}

KeyboardEventManager::~KeyboardEventManager() {
}

void KeyboardEventManager::goToMenu(NSString* menu_item) {
  NSString* script_format = @"tell application \"Finder\"\n"
                              @"activate\n"
                            @"end tell\n"
                            @"tell application \"System Events\"\n"
                              @"tell process \"Finder\"\n"
                                @"click menu item %@ of menu \"Go\" of menu bar 1\n"
                              @"end tell\n"
                            @"end tell\n";
  NSString* script = [NSString stringWithFormat:script_format, menu_item];

  FileManager::runAppleScriptThroughNSTask(script);
}

void KeyboardEventManager::keyDown(KeyboardEvent keyboard_event) {
  bool shift = keyboard_event.modifier_flags & NSShiftKeyMask;
  bool command = keyboard_event.modifier_flags & NSCommandKeyMask;
  bool option = keyboard_event.modifier_flags & NSAlternateKeyMask;

  BumpEnvironment bump_environment = BumpEnvironment(bumptop_->physics(),
                                                     bumptop_->scene()->room(),
                                                     bumptop_->ogre_scene_manager());
  VisualPhysicsActorList selected_actors = bump_environment.room->selected_actors();

  if (shift && command) {
    // Redo
    if (keyboard_event.characters.startsWith("Z")) {
      Redo::singleton()->execute(bump_environment);
    }
    if (keyboard_event.characters.startsWith("P")) {
      PerformanceStatsHUD::singleton()->toggleHUD();
    }
    // New Folder
    if (keyboard_event.characters.startsWith("N")) {
      VisualPhysicsActorList list_with_just_roomsurface_floor;
      list_with_just_roomsurface_floor.append(bumptop_->scene()->room()->getSurface(FLOOR));
      NewFolder::singleton()->applyToActors(bump_environment, list_with_just_roomsurface_floor);
    }

    // Open Applications in Finder
    if (keyboard_event.characters.startsWith("A")) {
      goToMenu(@"\"Applications\"");
    }

    // Open Computer in Finder
    if (keyboard_event.characters.startsWith("C")) {
      goToMenu(@"\"Computer\"");
    }

    // Open Desktop in Finder
    if (keyboard_event.characters.startsWith("D")) {
      goToMenu(@"\"Desktop\"");
    }

    // Open Home in Finder
    if (keyboard_event.characters.startsWith("H")) {
      goToMenu(@"\"Home\"");
    }

    // Open Network in Finder
    if (keyboard_event.characters.startsWith("K")) {
      goToMenu(@"\"Network\"");
    }

    // Open iDisk in Finder
    if (keyboard_event.characters.startsWith("I")) {
      goToMenu(@"\"My iDisk\" of menu of menu item \"iDisk\"");
    }

    // Open Documents in Finder
    if (keyboard_event.characters.startsWith("O")) {
      goToMenu(@"\"Documents\"");
    }

    // Open Utilities in Finder
    if (keyboard_event.characters.startsWith("U")) {
      goToMenu(@"\"Utilities\"");
    }

    // Empty Trash
    if (keyboard_event.characters.startsWith("\177")) {  // 177 "Delete" Key
      FileManager::emptyTrash();
    }
  } else if (option && command) {
    // Deselect All
    if (keyboard_event.characters.startsWith("a")) {
      bump_environment.room->deselectActors();
    }
  } else if (command) {
    // Move to trash
    if (keyboard_event.characters.startsWith("\177"))  // 177 "Delete" Key
      if (MoveToTrash::singleton()->canBeAppliedToActors(bump_environment, selected_actors))
        MoveToTrash::singleton()->applyToActors(bump_environment, selected_actors);

    // Select all
    if (keyboard_event.characters.startsWith("a"))
      SelectAllActors::singleton()->execute(bump_environment);

    // Copy
    if (keyboard_event.characters.startsWith("c"))
      if (Copy::singleton()->canBeAppliedToActors(bump_environment, selected_actors))
        Copy::singleton()->applyToActors(bump_environment, selected_actors);

    // Paste
    if (keyboard_event.characters.startsWith("v"))
      Paste::singleton()->execute(bump_environment);

    // PileBreak
    if (keyboard_event.characters.startsWith("b"))
      if (BreakPile::singleton()->canBeAppliedToActors(bump_environment, selected_actors))
          BreakPile::singleton()->applyToActors(bump_environment, selected_actors);

    // Duplicate
    if (keyboard_event.characters.startsWith("d"))
      if (Duplicate::singleton()->canBeAppliedToActors(bump_environment, selected_actors))
        Duplicate::singleton()->applyToActors(bump_environment, selected_actors);

    // Get Info
    if (keyboard_event.characters.startsWith("i"))
      if (GetInfo::singleton()->canBeAppliedToActors(bump_environment, selected_actors))
        GetInfo::singleton()->applyToActors(bump_environment, selected_actors);

    // Make Alias
    if (keyboard_event.characters.startsWith("l"))
      if (MakeAlias::singleton()->canBeAppliedToActors(bump_environment, selected_actors))
        MakeAlias::singleton()->applyToActors(bump_environment, selected_actors);

    // Pileize
    if (keyboard_event.characters.startsWith("p"))
      if (CreatePile::singleton()->canBeAppliedToActors(bump_environment, selected_actors))
        CreatePile::singleton()->applyToActors(bump_environment, selected_actors);

    // Shrink
    if (keyboard_event.characters.startsWith("-"))
      if (Shrink::singleton()->canBeAppliedToActors(bump_environment, selected_actors))
        Shrink::singleton()->applyToActors(bump_environment, selected_actors);

    // Grow
    if (keyboard_event.characters.startsWith("=") || keyboard_event.characters.startsWith("+"))
      if (Grow::singleton()->canBeAppliedToActors(bump_environment, selected_actors))
        Grow::singleton()->applyToActors(bump_environment, selected_actors);

    // Undo
    if (keyboard_event.characters.startsWith("z"))
      Undo::singleton()->execute(bump_environment);

    // Rename
    if (keyboard_event.characters.startsWith(QChar(13)))  // 13 "Enter" key, so rename
      if (Rename::singleton()->canBeAppliedToActors(bump_environment, selected_actors))
        Rename::singleton()->applyToActors(bump_environment, selected_actors);

    // Open
    if (keyboard_event.characters.startsWith("o"))
      if (Open::singleton()->canBeAppliedToActors(bump_environment, selected_actors))
        Open::singleton()->applyToActors(bump_environment, selected_actors);

    // Eject
    if (keyboard_event.characters.startsWith("e"))
      if (Eject::singleton()->canBeAppliedToActors(bump_environment, selected_actors))
        Eject::singleton()->applyToActors(bump_environment, selected_actors);

    // New Finder Window
    if (keyboard_event.characters.startsWith("n")) {
      NSString* script = @"tell application \"Finder\" to make new Finder window to home";

      NSAppleScript *desktop_script = [[NSAppleScript alloc] initWithSource:script];
      [desktop_script executeAndReturnError:nil];
      setFinderToFront();
    }

    // Arrow Keys
    if (keyboard_event.characters.startsWith(QChar(ARROW_UP))) {
      bumptop_->scene()->room()->process_arrow_key(ARROW_UP, keyboard_event.modifier_flags);
    }
    if (keyboard_event.characters.startsWith(QChar(ARROW_DOWN))) {
      bumptop_->scene()->room()->process_arrow_key(ARROW_DOWN, keyboard_event.modifier_flags);
    }
  } else {
    // Arrow Keys
    if (keyboard_event.characters.startsWith(QChar(ARROW_UP)))
      bumptop_->scene()->room()->process_arrow_key(ARROW_UP, keyboard_event.modifier_flags);
    if (keyboard_event.characters.startsWith(QChar(ARROW_DOWN)))
      bumptop_->scene()->room()->process_arrow_key(ARROW_DOWN, keyboard_event.modifier_flags);
    if (keyboard_event.characters.startsWith(QChar(ARROW_LEFT)))
      bumptop_->scene()->room()->process_arrow_key(ARROW_LEFT, keyboard_event.modifier_flags);
    if (keyboard_event.characters.startsWith(QChar(ARROW_RIGHT)))
      bumptop_->scene()->room()->process_arrow_key(ARROW_RIGHT, keyboard_event.modifier_flags);

    // Close View
    if (keyboard_event.characters.startsWith(QChar(27))) { // Escape Key
      bumptop_->scene()->room()->closeViewOfLastSelectedActor();
    }

    if (bumptop_->scene()->room()->search_box()->is_open()) {
      if (ProAuthorization::singleton()->authorized()) {
        bumptop_->scene()->room()->search_box()->startSearch(keyboard_event.characters);
      }
    } else if (keyboard_event.characters.startsWith("\33")) {
      if (bumptop_->scene()->surface_that_camera_is_zoomed_to() != NONE) {
        bumptop_->scene()->room()->setCameraForRoom();
      }
    } else {
      if (keyboard_event.characters.startsWith(" ")) {  // Spacebar is for QuickLook
        bumptop_->scene()->QuickLook_preview_panel()->toggle();
      } else if (keyboard_event.characters.startsWith(QChar(13))) {  // 13 "Enter" key, so rename
        if (Rename::singleton()->canBeAppliedToActors(bump_environment, selected_actors))
          Rename::singleton()->applyToActors(bump_environment, selected_actors);
      } else {
        if (ProAuthorization::singleton()->authorized()) {
          bumptop_->scene()->room()->search_box()->startSearch(keyboard_event.characters);
        }
      }
    }
  }
}

void KeyboardEventManager::flagsChanged(KeyboardEvent keyboard_event) {
  bool command_key_down = keyboard_event.modifier_flags & NSCommandKeyMask ||
                           keyboard_event.modifier_flags & NSShiftKeyMask;

  if (command_key_down != command_key_down_) {
    command_key_down_ = command_key_down;
    bumptop_->scene()->room()->setToolbarNeedsUpdating();
  }

  option_key_down_ = keyboard_event.modifier_flags & NSAlternateKeyMask;
}

bool KeyboardEventManager::command_key_down() {
  return command_key_down_;
}

bool KeyboardEventManager::option_key_down() {
  return option_key_down_;
}

#include "BumpTop/moc/moc_KeyboardEventManager.cpp"
