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

#include "BumpTop/UndoCommands/BreakSinglePileUndoCommand.h"

#include "BumpTop/AnimationManager.h"
#include "BumpTop/AppSettings.h"
#include "BumpTop/BumpPile.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/Room.h"
#include "BumpTop/UndoRedoStack.h"
#include "BumpTop/VisualPhysicsActorAnimation.h"

BreakSinglePileUndoCommand::BreakSinglePileUndoCommand(VisualPhysicsActorId pile_id, Room* room,
                                           Ogre::SceneManager* scene_manager, Physics* physics,
                                           BreakType break_type)
: room_(room),
  scene_manager_(scene_manager),
  physics_(physics),
  piles_members_ids_(QList<VisualPhysicsActorId>()),
  first_run_(true),
  pile_id_(pile_id),
  break_type_(break_type),
  label_colour_(COLOURLESS) {
}

BreakSinglePileUndoCommand::~BreakSinglePileUndoCommand() {
}

void BreakSinglePileUndoCommand::undo() {
  VisualPhysicsActorList pile_actors = VisualPhysicsActorList();
  QList<Ogre::Vector3> filtered_piles_members_offsets;
  QList<Ogre::Quaternion> filtered_piles_members_original_orientations;

  int i = 0;
  for_each(VisualPhysicsActorId unique_id, piles_members_ids_) {
    VisualPhysicsActor* actor = room_->actor_with_unique_id(unique_id);
    if (actor != NULL) {
      pile_actors.push_back(actor);
      filtered_piles_members_offsets.push_back(piles_members_offsets_.at(i));
      filtered_piles_members_original_orientations.push_back(piles_members_original_orientations_.at(i));
    }
    i++;
  }
  if (!AppSettings::singleton()->use_new_items_pile_setting()) {
    pile_is_new_items_pile_ = false;
  }

  if (pile_is_new_items_pile_
      && AppSettings::singleton()->use_new_items_pile_setting()) {
    if (pile_actors.size() > 1) {
      i = 0;
      VisualPhysicsActor* pile = room_->actor_with_unique_id(pile_id_);
      for_each(VisualPhysicsActor* actor, pile_actors) {
        if (actor->actor_type() != BUMP_DUMMY) {
          pile->addActorToPileAndUpdatePileView(actor,
                                              filtered_piles_members_offsets.at(i),
                                              filtered_piles_members_original_orientations.at(i));
        }
        i++;
      }
      pile->set_selected(false);
      pile->set_selected(true);
    } else {
      UndoRedoStack::last_command_changed_something = false;
    }
  } else if (pile_actors.count() > 1) {
    BumpPile* pile = new BumpPile(scene_manager_, physics_, room_, pile_id_);
    pile->initWithActors(pile_actors, piles_original_position_,
                         filtered_piles_members_offsets, filtered_piles_members_original_orientations);
    pile->stackViewOnInit();
    AnimationManager::singleton()->endAnimationsForActor(pile, AnimationManager::MOVE_TO_FINAL_STATE);
    VisualPhysicsActorAnimation* actor_animation = new VisualPhysicsActorAnimation(static_cast<VisualPhysicsActor*> (pile),  // NOLINT
                                                                                   250,
                                                                                   pile->position(),
                                                                                   piles_original_orientation_);  // NOLINT
    actor_animation->start();
    pile->set_display_name(piles_original_display_name_);
    pile->set_label_colour(label_colour_);
    // TODO: why is it necessary to deselect first to make highlight show up reliably?
    pile->set_selected(false);
    pile->set_selected(true);
  } else {
    // this command didn't do anything, skip to the next command on undo stack
    UndoRedoStack::last_command_changed_something = false;
  }
}

void BreakSinglePileUndoCommand::redo() {
  bool is_anything_broken = false;

  VisualPhysicsActor* pile = room_->actor_with_unique_id(pile_id_);
  if (pile != NULL) {
    label_colour_ = pile->label_colour();
    AnimationManager::singleton()->endAnimationsForActor(pile, AnimationManager::MOVE_TO_FINAL_STATE);
    is_anything_broken = true;
    if (first_run_) {
      pile_is_new_items_pile_ = pile->is_new_items_pile();
      piles_members_ids_ = pile->children_ids();
      piles_original_position_ = pile->position();
      piles_original_orientation_ = pile->orientation();
      piles_original_display_name_ = pile->display_name();

      piles_members_offsets_ = pile->children_offsets();
      piles_members_original_orientations_ = pile->children_orientations();
    }
    if (pile_is_new_items_pile_) {
      // The dummy is always the first child
      // If the dummy is no longer in the room
      // The new items pile was removed
      if (!room_->containsDirectlyOrThroughChild(piles_members_ids_[0])) {
        // Remove the dummy from the list
        piles_members_ids_.pop_front();
        piles_members_offsets_.pop_front();
        piles_members_original_orientations_.pop_front();
      }

      pile->breakAllItemsExceptDummy();
      pile->stackViewAndScaleChildren(1);
      pile->set_selected(false);
    } else {
      if (break_type_ == CONSTRAIN_FINAL_MEMBER_POSITIONS) {
        pile->breakPile();
      } else {  // (break_type_ == DONT_CONSTRAIN_FINAL_MEMBER_POSITIONS)
        pile->breakPileWithoutConstrainingFinalPoses();
      }
      VisualPhysicsActorDeletionManager::singleton()->deleteActorOnNextRenderTick(pile);
    }
  } else {
    UndoRedoStack::last_command_changed_something = false;
  }

  first_run_ = false;
}
