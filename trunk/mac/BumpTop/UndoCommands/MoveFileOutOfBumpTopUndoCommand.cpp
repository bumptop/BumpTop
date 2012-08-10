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

#include "BumpTop/UndoCommands/MoveFileOutOfBumpTopUndoCommand.h"
#include "BumpTop/FileManager.h"
#include "BumpTop/QStringHelpers.h"
#include "BumpTop/Room.h"
#include "BumpTop/UndoRedoStack.h"
#include "BumpTop/VisualPhysicsActor.h"
#include "BumpTop/VisualPhysicsActorId.h"

MoveFileOutOfBumpTopUndoCommand::MoveFileOutOfBumpTopUndoCommand(VisualPhysicsActorList actors, bool are_actors_dragged,
                                                                 FinderCommandType command_type, Room* room,
                                                                 QString target_path)
: first_run_(true),
  room_(room),
  command_type_(command_type),
  target_path_(target_path) {
  for_each(VisualPhysicsActor* actor, actors) {
    VisualPhysicsActorId parent_id;
    if (!are_actors_dragged) {
      if (actor->parent() == NULL) {
        parent_id = 0;
      } else {
        parent_id = actor->parent()->unique_id();
      }
    } else {
      parent_id = actor->actor_parent_id_before_drag();
      if (room->actor_with_unique_id(parent_id) == NULL) {
        parent_id = 0;
      }
    }

    if (parent_id == 0) {
      if (actor->path() != "") {
        files_paths_to_actors_ids_[actor->path()] = actor->unique_id();
        actors_ids_to_parents_ids_[actor->unique_id()] = 0;

        if (!are_actors_dragged) {
          paths_to_screen_positions_[actor->path()] = actor->getScreenPosition();
        } else {
          paths_to_screen_positions_[actor->path()] = actor->actor_screen_position_before_drag();
        }
      }

      if (!actor->children().empty()) {
        parents_ids_to_parents_types_[actor->unique_id()] = actor->actor_type();
        parents_ids_to_children_ids_[actor->unique_id()] = actor->children_ids();
      }

      for_each(VisualPhysicsActor* child, actor->children()) {
        files_paths_to_actors_ids_[child->path()] = child->unique_id();
        actors_ids_to_parents_ids_[child->unique_id()] = actor->unique_id();

        if (!are_actors_dragged) {
          paths_to_screen_positions_[child->path()] = actor->getScreenPosition();
        } else {
          paths_to_screen_positions_[child->path()] = actor->actor_screen_position_before_drag();
        }
      }
    } else {
      VisualPhysicsActor* parent = room->actor_with_unique_id(parent_id);

      files_paths_to_actors_ids_[actor->path()] = actor->unique_id();
      paths_to_screen_positions_[actor->path()] = parent->getScreenPosition();
      actors_ids_to_parents_ids_[actor->unique_id()] = parent_id;
      parents_ids_to_parents_types_[parent_id] = parent->actor_type();

      parents_ids_to_children_ids_[parent_id] = parent->flattenedChildrenIds();
    }
  }

  for_each(QString path, files_paths_to_actors_ids_.keys()) {
    struct stat fstat_info;
    stat(utf8(path).c_str(), &fstat_info);
    file_paths_to_inodes_[path] = fstat_info.st_ino;  // inode
  }
}

MoveFileOutOfBumpTopUndoCommand::~MoveFileOutOfBumpTopUndoCommand() {
}

void MoveFileOutOfBumpTopUndoCommand::undo() {
  if (UndoRedoStack::has_current_item_always_been_top_most_item) {
    QDir dir(target_path_);
    QStringList file_paths_that_will_put_back_to_room;

    // getting a qhash of file_path and inodes of all files in trash
    for_each(QFileInfo file_info, dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot)) {
      QString file_path = file_info.filePath();
      struct stat fstat_info;
      stat(utf8(file_path).c_str(), &fstat_info);
      if (file_paths_to_inodes_.values().contains(fstat_info.st_ino)) {
        QString parent_directory_of_original_item = QFileInfo(file_paths_to_inodes_.key(fstat_info.st_ino)).absolutePath();  // NOLINT
        FileManager::moveFile(file_path, parent_directory_of_original_item, kPromptUserOnConflict);  // NOLINT
        room_->expectFileToBeAdded(parent_directory_of_original_item + "/" + QFileInfo(file_path).fileName(),  // NOLINT
                                   paths_to_screen_positions_[file_paths_to_inodes_.key(fstat_info.st_ino)]);  // NOLINT
        file_paths_that_will_put_back_to_room.append(file_paths_to_inodes_.key(fstat_info.st_ino));
      }
    }

    QStringList invalid_paths;
    for_each(QString path, file_paths_to_inodes_.keys()) {
      if (!file_paths_that_will_put_back_to_room.contains(path)) {
        invalid_paths.append(path);
      }
    }

    updateHashes(invalid_paths);

    room_->expectFilesToBeRestoredToOriginalGrouping(files_paths_to_actors_ids_,
                                                     actors_ids_to_parents_ids_,
                                                     parents_ids_to_parents_types_,
                                                     parents_ids_to_children_ids_);
  } else {
    UndoRedoStack::last_command_changed_something = false;
  }
}

void MoveFileOutOfBumpTopUndoCommand::redo() {
  if (first_run_ || UndoRedoStack::has_current_item_always_been_top_most_item) {
    first_run_ = false;
    QStringList actors_paths_copy_ = file_paths_to_inodes_.keys();
    QStringList valid_path_to_move;
    for_each(QString actor_path, actors_paths_copy_) {
      if (QFileInfo(actor_path).exists()) {
        if (command_type_ == MOVE_TO_TRASH_COMMAND) {
          valid_path_to_move.append(actor_path);
        } else {
          FileManager::moveFile(actor_path, target_path_, kPromptUserOnConflict);
        }
      } else {
        file_paths_to_inodes_.remove(actor_path);
      }
    }
    if (command_type_ == MOVE_TO_TRASH_COMMAND) {
      FileManager::moveFileToTrash(valid_path_to_move);
    }
  } else {
    UndoRedoStack::last_command_changed_something = false;
  }
}

void MoveFileOutOfBumpTopUndoCommand::updateHashes(QStringList invalid_paths) {
  for_each(QString invalid_path, invalid_paths) {
    VisualPhysicsActorId invalid_actor_id = files_paths_to_actors_ids_[invalid_path];
    VisualPhysicsActorId parent_id = actors_ids_to_parents_ids_[invalid_actor_id];

    file_paths_to_inodes_.remove(invalid_path);
    files_paths_to_actors_ids_.remove(invalid_path);
    actors_ids_to_parents_ids_.remove(invalid_actor_id);
    parents_ids_to_children_ids_[parent_id].removeAll(invalid_actor_id);

    if (parents_ids_to_children_ids_[parent_id].count() <= 1) {
      parents_ids_to_parents_types_.remove(parent_id);
      parents_ids_to_children_ids_.remove(parent_id);
    }
  }
}
