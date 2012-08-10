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

#include "BumpTop/UndoCommands/MoveFileIntoBumpTopUndoCommand.h"

#include "BumpTop/FileManager.h"
#include "BumpTop/QStringHelpers.h"
#include "BumpTop/Room.h"
#include "BumpTop/UndoRedoStack.h"

MoveFileIntoBumpTopUndoCommand::MoveFileIntoBumpTopUndoCommand(QStringList paths,
                                                               Ogre::Vector2 expected_position,
                                                               Room* room,
                                                               QString target_path)
: first_run_(true),
  paths_(paths),
  expected_position_(expected_position),
  room_(room),
  target_path_(target_path) {
  for_each(QString path, paths) {
    struct stat fstat_info;
    stat(utf8(path).c_str(), &fstat_info);
    file_paths_to_inodes_[path] = fstat_info.st_ino;  // inode
  }
}

MoveFileIntoBumpTopUndoCommand::~MoveFileIntoBumpTopUndoCommand() {
}

void MoveFileIntoBumpTopUndoCommand::undo() {
  if (UndoRedoStack::has_current_item_always_been_top_most_item) {
    QDir dir(target_path_);
    QStringList file_paths_to_put_back;

    // getting a qhash of file_path and inodes of all files in target_path
    for_each(QFileInfo file_info, dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot)) {
      QString file_path = file_info.filePath();
      struct stat fstat_info;
      stat(utf8(file_path).c_str(), &fstat_info);

      if (file_paths_to_inodes_.values().contains(fstat_info.st_ino)) {
        QString parent_directory_of_original_item = QFileInfo(file_paths_to_inodes_.key(fstat_info.st_ino)).absolutePath();  // NOLINT
        FileManager::moveFile(file_path, parent_directory_of_original_item, kPromptUserOnConflict);  // NOLINT

        file_paths_to_put_back.append(file_paths_to_inodes_.key(fstat_info.st_ino));
      }
    }

    QStringList invalid_paths;
    for_each(QString path, file_paths_to_inodes_.keys()) {
      if (!file_paths_to_put_back.contains(path)) {
        invalid_paths.append(path);
      }
    }

    updatePaths(invalid_paths);
  } else {
    UndoRedoStack::last_command_changed_something = false;
  }
}

void MoveFileIntoBumpTopUndoCommand::redo() {
  if (first_run_ || UndoRedoStack::has_current_item_always_been_top_most_item) {
    first_run_ = false;
    QStringList actors_paths_copy_ = paths_;
    QStringList valid_path_to_move;

    for_each(QString actor_path, actors_paths_copy_) {
      if (QFileInfo(actor_path).exists()) {
        FileManager::moveFile(actor_path, target_path_, kPromptUserOnConflict);
        room_->expectFileToBeAdded(target_path_ + "/" + QFileInfo(actor_path).fileName(), expected_position_);
      } else {
        paths_.removeAll(actor_path);
        file_paths_to_inodes_.remove(actor_path);
      }
    }
  } else {
    UndoRedoStack::last_command_changed_something = false;
  }
}

void MoveFileIntoBumpTopUndoCommand::updatePaths(QStringList invalid_paths) {
  for_each(QString invalid_path, invalid_paths) {
    paths_.removeAll(invalid_path);
    file_paths_to_inodes_.remove(invalid_path);
  }
}
