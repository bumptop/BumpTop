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

#ifndef BUMPTOP_UNDOCOMMANDS_INTERNALDRAGANDDROPUNDOCOMMAND_H_
#define BUMPTOP_UNDOCOMMANDS_INTERNALDRAGANDDROPUNDOCOMMAND_H_

#include <QtGui/QUndoCommand>

#include "BumpTop/VisualPhysicsActor.h"
#include "BumpTop/VisualPhysicsActorId.h"

class Room;
class RoomSurface;
class VisualPhysicsActorAnimation;

class InternalDragAndDropUndoCommand : public QObject, public QUndoCommand {
 public:
  explicit InternalDragAndDropUndoCommand(VisualPhysicsActorList actors,
                                          VisualPhysicsActorId receiver_id,
                                          Room* room);
  virtual ~InternalDragAndDropUndoCommand();
  virtual void undo();
  virtual void redo();

 protected:
  bool first_run_;
  Room* room_;

  VisualPhysicsActorId receiver_id_;
  QList<VisualPhysicsActorId> dragged_actors_ids_;
  QHash<VisualPhysicsActorId, BumpPose> actors_ids_to_undo_state_poses_;
  QHash<VisualPhysicsActorId, VisualPhysicsActorId> actors_ids_to_undo_state_parents_ids_;
  QHash<VisualPhysicsActorId, RoomSurface*> actors_ids_to_undo_state_room_surfaces_;
  QHash<VisualPhysicsActorId, QList<VisualPhysicsActorId> > undo_state_piles_ids_to_children_ids_;
  QHash<VisualPhysicsActorId, BumpPose> piles_ids_to_undo_state_children_offset_poses_;
  QHash<VisualPhysicsActorId, VisualPhysicsActorType> actors_ids_to_undo_state_actors_types_;
  QHash<VisualPhysicsActorId, QHash<VisualPhysicsActorId, BumpPose> > actors_ids_to_undo_state_siblings_offset_poses_;
  QHash<VisualPhysicsActorId, VisualPhysicsActorType> undo_state_parents_ids_to_parents_types_;
  QHash<VisualPhysicsActorId, Ogre::Vector3> undo_state_parents_ids_to_parents_positions_;

  QHash<VisualPhysicsActorId, BumpPose> actors_ids_to_redo_state_poses_;
  QHash<VisualPhysicsActorId, RoomSurface*> actors_ids_to_redo_state_room_surfaces_;
};
#endif  // BUMPTOP_UNDOCOMMANDS_INTERNALDRAGANDDROPUNDOCOMMAND_H_
