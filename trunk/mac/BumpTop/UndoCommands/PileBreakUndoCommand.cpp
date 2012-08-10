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

#include "BumpTop/UndoCommands/PileBreakUndoCommand.h"

#include "BumpTop/AnimationManager.h"
#include "BumpTop/UndoCommands/BreakSinglePileUndoCommand.h"
#include "BumpTop/BumpPile.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/Room.h"
#include "BumpTop/UndoRedoStack.h"
#include "BumpTop/VisualPhysicsActorAnimation.h"

SINGLETON_IMPLEMENTATION(VisualPhysicsActorDeletionManager)

VisualPhysicsActorDeletionManager::VisualPhysicsActorDeletionManager()
: has_connected_to_render_tick_(false) {
}

void VisualPhysicsActorDeletionManager::deleteActorOnNextRenderTick(VisualPhysicsActor* actor) {
  if (!has_connected_to_render_tick_) {
    assert(QObject::connect(BumpTopApp::singleton(), SIGNAL(onRender()),  // NOLINT
                            this, SLOT(renderTick())));  // NOLINT
    has_connected_to_render_tick_ = true;
  }
  actors_to_delete_.push_back(actor);
}

void VisualPhysicsActorDeletionManager::renderTick() {
  for_each(VisualPhysicsActor* actor, actors_to_delete_) {
    delete actor;
  }
  actors_to_delete_.clear();
  BumpTopApp::singleton()->disconnect(this);
  has_connected_to_render_tick_ = false;
}

PileBreakUndoCommand::PileBreakUndoCommand(VisualPhysicsActorList piles_to_break, Room* room,
                                           Ogre::SceneManager* scene_manager, Physics* physics,
                                           BreakType break_type)
: room_(room),
scene_manager_(scene_manager),
physics_(physics),
first_run_(true),
break_type_(break_type) {
  piles_to_break_ids_ = QList<VisualPhysicsActorId>();
  for_each(VisualPhysicsActor* pile, piles_to_break) {
    if (pile->breakable()) {
      piles_to_break_ids_.push_back(pile->unique_id());
    }
  }
}

PileBreakUndoCommand::~PileBreakUndoCommand() {
}

bool PileBreakUndoCommand::willHaveAnEffect(VisualPhysicsActorList piles_to_break, BumpEnvironment env) {
  bool will_have_an_effect = false;
  for_each(VisualPhysicsActor* pile, piles_to_break) {
    if (pile->breakable()) {
      will_have_an_effect = true;
    }
  }
  return will_have_an_effect;
}

void PileBreakUndoCommand::undo() {
  bool command_changed_something = false;
  for_each(BreakSinglePileUndoCommand* break_undo_command, pile_break_undo_commands_) {
    break_undo_command->undo();
    command_changed_something = command_changed_something || UndoRedoStack::last_command_changed_something;
  }
  UndoRedoStack::last_command_changed_something = command_changed_something;
}

void PileBreakUndoCommand::redo() {
  bool command_changed_something = false;
  if (first_run_) {
    for_each(VisualPhysicsActorId pile_id, piles_to_break_ids_) {
      BreakSinglePileUndoCommand* break_undo_command = new BreakSinglePileUndoCommand(pile_id, room_, scene_manager_,
                                                                                      physics_, break_type_);
      pile_break_undo_commands_.push_back(break_undo_command);
    }
  }
  for_each(BreakSinglePileUndoCommand* break_undo_command, pile_break_undo_commands_) {
    break_undo_command->redo();
    command_changed_something = command_changed_something || UndoRedoStack::last_command_changed_something;
  }
  first_run_ = false;
  UndoRedoStack::last_command_changed_something = command_changed_something;
}

#include "BumpTop/moc/moc_PileBreakUndoCommand.cpp"
