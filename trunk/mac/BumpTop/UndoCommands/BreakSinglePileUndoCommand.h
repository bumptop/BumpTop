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

#ifndef BUMPTOP_UNDOCOMMANDS_BREAKSINGLEPILEUNDOCOMMAND_H_
#define BUMPTOP_UNDOCOMMANDS_BREAKSINGLEPILEUNDOCOMMAND_H_

class Room;
class BumpPile;
class Physics;
class VisualPhysicsActor;

#include <QtGui/QUndoCommand>

#include "BumpTop/BumpTopCommands.h"
#include "BumpTop/UndoCommands/PileBreakUndoCommand.h"
#include "BumpTop/Singleton.h"
#include "BumpTop/VisualPhysicsActor.h"
#include "BumpTop/VisualPhysicsActorId.h"

class BreakSinglePileUndoCommand : public QUndoCommand {
 public:
  explicit BreakSinglePileUndoCommand(VisualPhysicsActorId pile_id, Room* room,
                                      Ogre::SceneManager* scene_manager, Physics* physics,
                                      BreakType break_type = CONSTRAIN_FINAL_MEMBER_POSITIONS);
  virtual ~BreakSinglePileUndoCommand();
  virtual void undo();
  virtual void redo();

  // static bool willHaveAnEffect(VisualPhysicsActorList piles_to_break, BumpEnvironment env);

 protected:
  BreakType break_type_;
  bool first_run_;
  Room* room_;
  Physics* physics_;
  Ogre::SceneManager* scene_manager_;
  QList<VisualPhysicsActorId> piles_members_ids_;
  Ogre::Vector3 piles_original_position_;
  Ogre::Quaternion piles_original_orientation_;

  bool pile_is_new_items_pile_;
  VisualPhysicsActorId pile_id_;
  QString piles_original_display_name_;
  QList<Ogre::Vector3> piles_members_offsets_;
  QList<Ogre::Quaternion> piles_members_original_orientations_;

  BumpBoxLabelColour label_colour_;
};
#endif  // BUMPTOP_UNDOCOMMANDS_BREAKSINGLEPILEUNDOCOMMAND_H_
