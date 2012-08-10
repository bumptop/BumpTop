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

#include "BumpTop/OSX/OSXCocoaDragAndDrop.h"

#include "BumpTop/BumpTopApp.h"
#include "BumpTop/BumpTopScene.h"
#include "BumpTop/FileManager.h"
#include "BumpTop/QStringHelpers.h"
#include "BumpTop/Room.h"
#include "BumpTop/UndoCommands/MoveFileOutOfBumpTopUndoCommand.h"
#include "BumpTop/UndoRedoStack.h"
#include "BumpTop/VisualPhysicsActor.h"

OSXCocoaDragAndDrop::OSXCocoaDragAndDrop(NSView* view)
: last_mouse_down_event_(NULL),
  view_(view) {
}

OSXCocoaDragAndDrop::~OSXCocoaDragAndDrop() {
  if (last_mouse_down_event_ != NULL) {
    [last_mouse_down_event_ release];
  }
}

// This function returns a BitmapImage corresponding to the icon for a given path
NSImage *OSXCocoaDragAndDrop::iconImageForPath(QString path, int icon_size, MaterialLoader::IconLoadMethod icon_load_method) {
  // check whether Quick Look is available on our system
  // you need to save it to an intermediate variable, and you need to cast it to uint32-- or else it fails
  // why? i have no fucking clue
  uint32 quick_look_fn_pointer = (uint32)QLThumbnailImageCreate;
  bool quick_look_supported = (quick_look_fn_pointer != 0);

  CGImageRef quick_look_image_ref;

  bool attempt_quick_lock = false && quick_look_supported && (icon_load_method == MaterialLoader::kQuickLook ||
                                                              icon_load_method == MaterialLoader::kQuickLookWithStandardIconFallback);
  if (attempt_quick_lock) {
    NSString *ns_path = NSStringFromQString(path);
    NSURL *url_path = [NSURL fileURLWithPath:ns_path];

    // Try to get a QuickLook Icon
    NSDictionary* quick_arguments_dict = [NSDictionary dictionaryWithObject:[NSNumber numberWithBool:YES]
                                                                     forKey:(NSString *)kQLThumbnailOptionIconModeKey];

    quick_look_image_ref = QLThumbnailImageCreate(kCFAllocatorDefault,
                                                  (CFURLRef)url_path,
                                                  CGSizeMake(icon_size, icon_size),
                                                  (CFDictionaryRef)quick_arguments_dict);
  }

  // If QuickLook fails, get the standard icon
  NSImage *icon_image = NULL;
  if (attempt_quick_lock && quick_look_image_ref) {
    NSBitmapImageRep *icon_bitmap = [[NSBitmapImageRep alloc] initWithCGImage: quick_look_image_ref];
    icon_image = [[NSImage alloc] initWithSize:[icon_bitmap size]];
    [icon_image addRepresentation:icon_bitmap];
    [icon_bitmap release];
  } else if (icon_load_method == MaterialLoader::kStandardIcon ||
             icon_load_method == MaterialLoader::kQuickLookWithStandardIconFallback) {
    icon_image = [[NSWorkspace sharedWorkspace] iconForFile:NSStringFromQString(path)];
    [icon_image setSize:NSMakeSize(icon_size, icon_size)];
  }

  return icon_image;
}

// **** important ***
// initiateDrag is blocking unless called from a mouse event
void OSXCocoaDragAndDrop::initiateDrag(QStringList paths) {
  assert(paths.size() != 0);
  dragged_file_paths_ = paths;
  NSImage *file_icon = iconImageForPath(paths.value(0), 100, MaterialLoader::kQuickLookWithStandardIconFallback);

  NSPoint mouse_down_point = [last_mouse_down_event_ locationInWindow];
  mouse_down_point = [view_ convertPoint:mouse_down_point fromView:nil];

  NSPasteboard *paste_board = [NSPasteboard pasteboardWithName:NSDragPboard];
  writeToPasteboard(paste_board, paths);

  [view_ dragImage:file_icon
                at:mouse_down_point
            offset:NSMakeSize(0,0)
             event:last_mouse_down_event_
        pasteboard:paste_board
            source:view_
         slideBack:NO];

  return;
}

void OSXCocoaDragAndDrop::dragFinishedInTrash() {
  QStringList paths_of_files_to_move_to_trash;
  for_each(QString dragged_file_path, dragged_file_paths_) {
    if (FileManager::getFileKind(dragged_file_path) == VOLUME) {
      if (!FileManager::isStartupDrive(dragged_file_path))
        FileManager::ejectDrive(dragged_file_path);
    } else {
      paths_of_files_to_move_to_trash.append(dragged_file_path);
    }
  }

  Room* room = BumpTopApp::singleton()->scene()->room();
  QString target_path = FileManager::getUserPath() + "/.trash";

  MoveFileOutOfBumpTopUndoCommand* move_to_trash_undo_command = new MoveFileOutOfBumpTopUndoCommand(room->selected_actors(),
                                                                                                    true,
                                                                                                    MOVE_TO_TRASH_COMMAND,
                                                                                                    room,
                                                                                                    target_path);
  room->updateCurrentState();
  room->undo_redo_stack()->push(move_to_trash_undo_command, room->current_state(), true);
}

void OSXCocoaDragAndDrop::writeToPasteboard(NSPasteboard *pb, QStringList paths)
{
  NSMutableArray* path_array = [NSMutableArray arrayWithCapacity:paths.count()];
  for_each(QString path, paths)
    [path_array addObject:NSStringFromQString(path)];
  [pb declareTypes:[NSArray arrayWithObject:NSFilenamesPboardType]
             owner:view_];
  [pb setPropertyList:path_array forType:NSFilenamesPboardType];
}

void OSXCocoaDragAndDrop::set_last_mouse_down_event(NSEvent* mouse_down_event) {
  [last_mouse_down_event_ release];
  [mouse_down_event retain];
  last_mouse_down_event_ = mouse_down_event;
}
