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

#include "BumpTop/UndoCommands/PileToGridUndoCommand.h"

#include <utility>

#include "BumpTop/AppSettings.h"
#include "BumpTop/BumpDummy.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/BumpPile.h"
#include "BumpTop/NewItemsPile.h"
#include "BumpTop/GriddedPile.h"
#include "BumpTop/Room.h"
#include "BumpTop/RoomItemPoseConstraints.h"
#include "BumpTop/ToolTipManager.h"
#include "BumpTop/UndoRedoStack.h"
#include "BumpTop/VisualPhysicsActorAnimation.h"

PileToGridUndoCommand::PileToGridUndoCommand(VisualPhysicsActorId pile_to_convert_id, Room* room,
                                           Ogre::SceneManager* scene_manager, Physics* physics)
: room_(room),
  scene_manager_(scene_manager),
  physics_(physics),
  pile_id_(pile_to_convert_id),
  first_run_(true) {
}

PileToGridUndoCommand::~PileToGridUndoCommand() {
}

void PileToGridUndoCommand::undo() {
  bool did_anything_happen = false;

  GriddedPile* gridded_pile = static_cast<GriddedPile*>(room_->actor_with_unique_id(pile_id_));
  if (gridded_pile != NULL) {
    Ogre::Vector3 position_of_new_pile;
    if (gridded_pile->position().distance(gridded_pile->initial_position()) < 20.0) {
      position_of_new_pile = gridded_pile->position_of_pile_before_gridded();
    } else {
      position_of_new_pile = gridded_pile->position();
    }
    gridded_pile->breakPile();

    BumpPile* pile;

    if (gridded_pile->is_new_items_pile()) {
      pile = new NewItemsPile(scene_manager_, physics_, room_, pile_id_);
    } else {
      pile = new BumpPile(scene_manager_, physics_, room_, pile_id_);
    }

    pile->initWithActors(gridded_pile->children(),
                         position_of_new_pile,
                         gridded_pile->children_offsets(),
                         gridded_pile->children_orientations());


    pile->stackViewOnInit();
    pile->set_selected(false);
    pile->set_selected(true);

    pile->set_display_name(gridded_pile->display_name());
    ToolTipManager::singleton()->showPileFlipTooltip(pile);

    room_->removeActor(pile_id_);
    room_->addActor(pile);
    delete gridded_pile;
    did_anything_happen = true;
  }

  // this command didn't do anything, skip to the next command on redo stack
  if (did_anything_happen == false) {
    UndoRedoStack::last_command_changed_something = false;
  }
}

void PileToGridUndoCommand::redo() {
  bool did_anything_happen = false;

  BumpPile* pile = static_cast<BumpPile*>(room_->actor_with_unique_id(pile_id_));
  if (pile != NULL) {
    pile->breakPile();

    for_each (VisualPhysicsActor* actor, pile->flattenedChildren()) {
      if (actor->actor_type() == BUMP_DUMMY) {
        pile->breakItemsOutOfPile(QList<VisualPhysicsActorId>() << actor->unique_id());
        room_->removeActor(actor->unique_id());
        delete actor;
      }
    }

    GriddedPile* gridded_pile = new GriddedPile(scene_manager_, physics_, room_, pile->position(), pile_id_);
    gridded_pile->init();
    gridded_pile->set_position(Ogre::Vector3(pile->position().x, gridded_pile->position().y, pile->position().z));

    gridded_pile->addActors(pile->children(),
                            pile->children_offsets(),
                            pile->children_orientations());

    std::pair<bool, Ogre::Vector3> adjusted_position;
    adjusted_position = getPositionConstrainedToRoom(gridded_pile->world_bounding_box(), room_);

    if (adjusted_position.first) {
       VisualPhysicsActorAnimation* actor_animation = new VisualPhysicsActorAnimation(gridded_pile,
                                                                                      400,
                                                                                      adjusted_position.second,
                                                                                      gridded_pile->orientation());
      actor_animation->start();
      gridded_pile->set_initial_position(adjusted_position.second);
    } else {
      gridded_pile->set_initial_position(gridded_pile->position());
    }

    gridded_pile->set_display_name(pile->display_name());

    ToolTipManager::singleton()->hideGriddedPileTooltip();
    room_->removeActor(pile_id_);
    room_->addActor(gridded_pile);

    delete pile;
    did_anything_happen = true;

    if (gridded_pile->children().count() > 0) {
      gridded_pile->children().last()->set_selected(true);
    }
  }

  // this command didn't do anything, skip to the next command on redo stack
  if (did_anything_happen == false) {
    UndoRedoStack::last_command_changed_something = false;
  }
}

GridToPileUndoCommand::GridToPileUndoCommand(VisualPhysicsActorId pile_to_convert_id, Room* room,
                                             Ogre::SceneManager* scene_manager, Physics* physics)
: PileToGridUndoCommand(pile_to_convert_id, room, scene_manager, physics) {
}

void GridToPileUndoCommand::undo() {
  PileToGridUndoCommand::redo();
}

void GridToPileUndoCommand::redo() {
  PileToGridUndoCommand::undo();
}
