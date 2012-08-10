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

#ifndef BUMPTOP_UNDOCOMMANDS_PILEIZEUNDOCOMMAND_H_
#define BUMPTOP_UNDOCOMMANDS_PILEIZEUNDOCOMMAND_H_

class Room;
class BumpPile;
class Physics;
class PileBreakUndoCommand;
class VisualPhysicsActor;

#include <QtGui/QUndoCommand>

#include "BumpTop/VisualPhysicsActorId.h"
#include "BumpTop/VisualPhysicsActorList.h"

class PileizeUndoCommand : public QUndoCommand {
 public:
  explicit PileizeUndoCommand(VisualPhysicsActorList pile_members, Room* room,
                              Ogre::SceneManager* scene_manager, Physics* physics,
                              Ogre::Vector3 initial_position = Ogre::Vector3::ZERO,
                              QString pile_name = "", VisualPhysicsActorId pile_id = 0);
  virtual ~PileizeUndoCommand();

  virtual void undo();
  virtual void redo();

 protected:
  bool first_run_;
  VisualPhysicsActorId pile_id_;
  Room* room_;
  Physics* physics_;
  Ogre::SceneManager* scene_manager_;
  QList<VisualPhysicsActorId> pile_member_ids_;
  QList<Ogre::Vector3> pile_member_offsets_;
  QList<Ogre::Quaternion> pile_member_original_orientations_;
  PileBreakUndoCommand* pile_break_undo_command_;
  bool initial_position_provided_;
  Ogre::Vector3 initial_position_;
  QString pile_name_;
};

#endif  // BUMPTOP_UNDOCOMMANDS_PILEIZEUNDOCOMMAND_H_
