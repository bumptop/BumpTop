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

#ifndef BUMPTOP_UNDOCOMMANDS_PILETOGRIDUNDOCOMMAND_H_
#define BUMPTOP_UNDOCOMMANDS_PILETOGRIDUNDOCOMMAND_H_

class Room;
class BumpPile;
class Physics;
class VisualPhysicsActor;

#include <QtGui/QUndoCommand>

#include "BumpTop/BumpTopCommands.h"
#include "BumpTop/VisualPhysicsActorId.h"

class PileToGridUndoCommand : public QUndoCommand {
 public:
  explicit PileToGridUndoCommand(VisualPhysicsActorId pile_to_convert_id, Room* room,
                                Ogre::SceneManager* scene_manager, Physics* physics);
  virtual ~PileToGridUndoCommand();
  virtual void undo();
  virtual void redo();

 protected:
  bool first_run_;
  Room* room_;

  VisualPhysicsActorId pile_id_;

  Physics* physics_;
  Ogre::SceneManager* scene_manager_;
};

class GridToPileUndoCommand : public PileToGridUndoCommand {
 public:
  explicit GridToPileUndoCommand(VisualPhysicsActorId pile_to_convert_id, Room* room,
                                 Ogre::SceneManager* scene_manager, Physics* physics);
  virtual void undo();
  virtual void redo();
};

#endif  // BUMPTOP_UNDOCOMMANDS_PILETOGRIDUNDOCOMMAND_H_
