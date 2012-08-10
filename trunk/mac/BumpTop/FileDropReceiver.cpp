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

#include "BumpTop/FileDropReceiver.h"

#include "BumpTop/BumpTopApp.h"
#include "BumpTop/BumpTopScene.h"
#include "BumpTop/FileManager.h"
#include "BumpTop/Room.h"
#include "BumpTop/UndoCommands/MoveFileIntoBumpTopUndoCommand.h"
#include "BumpTop/UndoCommands/MoveFileOutOfBumpTopUndoCommand.h"
#include "BumpTop/UndoRedoStack.h"
#include "BumpTop/VisualPhysicsActor.h"

#include "BumpTop/QStringHelpers.h"

void DropTarget::expectFileToBeAdded(QString path, Ogre::Vector2 mouse_in_window_space) {
}

void DropTarget::draggingExited() {
}

FileDropReceiver::FileDropReceiver(DropTarget* target)
: target_(target) {
}

QString FileDropReceiver::target_path() {
  if (FileManager::getFileKind(target_->path()) == ALIAS) {
    return QFileInfo(target_->path()).readLink();
  } else {
    return target_->path();
  }
}

bool FileDropReceiver::prepareForDragOperation(Ogre::Vector2 mouse_in_window_space,
                                               QStringList list_of_files,
                                               NSDragOperation drag_operation,
                                               VisualPhysicsActorList list_of_actors) {
  return drag_operation != NSDragOperationNone;
}

bool FileDropReceiver::performDragOperation(Ogre::Vector2 mouse_in_window_space,
                                            QStringList list_of_files,
                                            NSDragOperation drag_operation,
                                            VisualPhysicsActorList list_of_actors) {
  if (drag_operation & NSDragOperationGeneric || drag_operation & NSDragOperationMove) {
    bool moving_files_failed = false;
    Room* room = BumpTopApp::singleton()->scene()->room();

    emit onPerformDragOperation();

    if (target_path() == room->path()) {
      for_each(QString file, list_of_files) {
        if (FileManager::getParentPath(file) == target_path()) {
          moving_files_failed = true;
        }
      }
      if (!moving_files_failed) {
        MoveFileIntoBumpTopUndoCommand* move_file_undo_command = new MoveFileIntoBumpTopUndoCommand(list_of_files,
                                                                                                    mouse_in_window_space,  // NOLINT
                                                                                                    room,
                                                                                                    target_path());
        room->updateCurrentState();
        room->undo_redo_stack()->push(move_file_undo_command, room->current_state(), true);
      }
    } else {
      MoveFileOutOfBumpTopUndoCommand* move_file_undo_command = new MoveFileOutOfBumpTopUndoCommand(list_of_actors,
                                                                                                    true,
                                                                                                    MOVE_FILE_COMMAND,
                                                                                                    room,
                                                                                                    target_path());
      room->updateCurrentState();
      room->undo_redo_stack()->push(move_file_undo_command, room->current_state(), true);
    }

    return moving_files_failed;
  } else if (drag_operation & NSDragOperationCopy) {
    copyFilesToRoom(list_of_files, mouse_in_window_space);
    return true;
  } else if (drag_operation & NSDragOperationLink) {
    bool linking_files_failed = false;
    for_each(QString file, list_of_files) {
      target_->expectFileToBeAdded(target_path() + "/" + QFileInfo(file).fileName(),
                                 mouse_in_window_space);
      if (!FileManager::linkFile(file, target_path())) {
        linking_files_failed = true;
      }
    }
    return !linking_files_failed;
  } else {
    return false;
  }
}

void FileDropReceiver::copyFilesToRoom(QStringList list_of_files, Ogre::Vector2 mouse_in_window_space) {
  for_each(QString file, list_of_files) {
    BehaviorOnConflict behavior_on_conflict;
    if (FileManager::getParentPath(file) == target_path()) {
      behavior_on_conflict = kAutomaticallyRenameOnConflict;
    } else {
      behavior_on_conflict = kPromptUserOnConflict;
    }
    target_->expectFileToBeAdded(target_path() + "/" + QFileInfo(file).fileName(),
                               mouse_in_window_space);
    FileManager::copyFile(file, target_path(), behavior_on_conflict);
  }
}

void FileDropReceiver::concludeDragOperation() {
}

void FileDropReceiver::draggingExited() {
  target_->draggingExited();
}

#include "BumpTop/moc/moc_FileDropReceiver.cpp"
