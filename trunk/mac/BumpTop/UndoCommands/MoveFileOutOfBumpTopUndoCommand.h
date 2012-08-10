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

#ifndef BUMPTOP_UNDOCOMMANDS_MOVEFILEOUTOFBUMPTOPUNDOCOMMAND_H_
#define BUMPTOP_UNDOCOMMANDS_MOVEFILEOUTOFBUMPTOPUNDOCOMMAND_H_

#include <QtGui/QUndoCommand>

#include "BumpTop/VisualPhysicsActor.h"
#include "BumpTop/VisualPhysicsActorId.h"

class Room;

enum FinderCommandType {
  MOVE_TO_TRASH_COMMAND,
  MOVE_FILE_COMMAND
};

class MoveFileOutOfBumpTopUndoCommand : public QUndoCommand {
 public:
  explicit MoveFileOutOfBumpTopUndoCommand(VisualPhysicsActorList actors, bool are_actors_dragged,
                                           FinderCommandType command_type, Room* room,
                                           QString target_path);
  virtual ~MoveFileOutOfBumpTopUndoCommand();
  virtual void undo();
  virtual void redo();

 protected:
  void updateHashes(QStringList invalid_paths);

  bool first_run_;
  Room* room_;
  FinderCommandType command_type_;
  QString target_path_;

  QHash<QString, ino_t> file_paths_to_inodes_;
  QHash<QString, VisualPhysicsActorId> files_paths_to_actors_ids_;
  QHash<QString, Ogre::Vector2> paths_to_screen_positions_;
  QHash<VisualPhysicsActorId, VisualPhysicsActorId> actors_ids_to_parents_ids_;
  QHash<VisualPhysicsActorId, VisualPhysicsActorType> parents_ids_to_parents_types_;
  QHash<VisualPhysicsActorId, QList<VisualPhysicsActorId> > parents_ids_to_children_ids_;
};
#endif  // BUMPTOP_UNDOCOMMANDS_MOVEFILEOUTOFBUMPTOPUNDOCOMMAND_H_
