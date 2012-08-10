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

#ifndef BUMPTOP_UNDOCOMMANDS_PILEBREAKUNDOCOMMAND_H_
#define BUMPTOP_UNDOCOMMANDS_PILEBREAKUNDOCOMMAND_H_

class Room;
class BreakSinglePileUndoCommand;
class BumpPile;
class Physics;
class VisualPhysicsActor;

#include <QtGui/QUndoCommand>

#include "BumpTop/BumpTopCommands.h"
#include "BumpTop/Singleton.h"
#include "BumpTop/VisualPhysicsActorId.h"

enum BreakType {
  CONSTRAIN_FINAL_MEMBER_POSITIONS,
  DONT_CONSTRAIN_FINAL_MEMBER_POSITIONS
};

class VisualPhysicsActorDeletionManager : public QObject {
  Q_OBJECT
  SINGLETON_HEADER(VisualPhysicsActorDeletionManager)
 public:
  VisualPhysicsActorDeletionManager();
 public slots:  // NOLINT
  void deleteActorOnNextRenderTick(VisualPhysicsActor* actor);
  void renderTick();
 protected:
  VisualPhysicsActorList actors_to_delete_;
  bool has_connected_to_render_tick_;
};

class PileBreakUndoCommand : public QUndoCommand {
 public:
  explicit PileBreakUndoCommand(VisualPhysicsActorList piles_to_break, Room* room,
                                Ogre::SceneManager* scene_manager, Physics* physics,
                                BreakType break_type = CONSTRAIN_FINAL_MEMBER_POSITIONS);
  virtual ~PileBreakUndoCommand();
  virtual void undo();
  virtual void redo();

  static bool willHaveAnEffect(VisualPhysicsActorList piles_to_break, BumpEnvironment env);

 protected:
  BreakType break_type_;
  bool first_run_;
  Room* room_;
  Physics* physics_;
  Ogre::SceneManager* scene_manager_;

  QList<VisualPhysicsActorId> piles_to_break_ids_;
  QList<BreakSinglePileUndoCommand*> pile_break_undo_commands_;
};
#endif  // BUMPTOP_UNDOCOMMANDS_PILEBREAKUNDOCOMMAND_H_
