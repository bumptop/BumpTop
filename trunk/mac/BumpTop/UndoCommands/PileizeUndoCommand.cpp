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

#include "BumpTop/UndoCommands/PileizeUndoCommand.h"

#include "BumpTop/BumpPile.h"
#include "BumpTop/UndoCommands/PileBreakUndoCommand.h"
#include "BumpTop/Room.h"
#include "BumpTop/RoomSurface.h"
#include "BumpTop/ToolTipManager.h"
#include "BumpTop/UndoRedoStack.h"

PileizeUndoCommand::PileizeUndoCommand(VisualPhysicsActorList pile_members, Room* room,
                                       Ogre::SceneManager* scene_manager, Physics* physics,
                                       Ogre::Vector3 initial_position,
                                       QString pile_name, VisualPhysicsActorId pile_id)
: room_(room),
  scene_manager_(scene_manager),
  physics_(physics),
  initial_position_(initial_position),
  first_run_(true),
  pile_break_undo_command_(NULL),
  pile_name_(pile_name),
  pile_id_(pile_id) {
  pile_member_ids_ = QList<VisualPhysicsActorId>();
  // We don't want to pile items from the walls
  for_each(VisualPhysicsActor* actor, pile_members) {
    if (!actor->room_surface()->is_pinnable_receiver() && actor->actor_type() != STICKY_NOTE_PAD) {
      pile_member_ids_.push_back(actor->unique_id());
    } else {
      actor->set_selected(false);
    }
  }
  initial_position_provided_ = (initial_position != Ogre::Vector3::ZERO);
}

PileizeUndoCommand::~PileizeUndoCommand() {
}

void PileizeUndoCommand::undo() {
  VisualPhysicsActor* pile = room_->actor_with_unique_id(pile_id_);
  if (pile != NULL) {
    pile->breakPile();
    delete pile;
    if (pile_break_undo_command_ != NULL) {
      pile_break_undo_command_->undo();
      delete pile_break_undo_command_;
      pile_break_undo_command_ = NULL;
    }
  } else {
  // if the pile no longer exists, move to the next command in the undo stack
    UndoRedoStack::last_command_changed_something = false;
  }
}

void PileizeUndoCommand::redo() {
  VisualPhysicsActorList flattened_pile_members = VisualPhysicsActorList();

  Ogre::Vector3 centroid;
  // This is different from the number of flattened pile members because if we combine two piles, we
  // only average the centroids of the piles (not their children)
  int number_of_actors_contributing_to_centroid = 0;
  pile_member_offsets_.clear();
  pile_member_original_orientations_.clear();
  VisualPhysicsActorList piles_to_break;

  for_each(VisualPhysicsActorId unique_id, pile_member_ids_) {
    VisualPhysicsActor* actor = room_->actor_with_unique_id(unique_id);
    if (actor != NULL) {
      if (actor->children().count() != 1) {
        centroid += actor->destination_pose().position;
        number_of_actors_contributing_to_centroid++;
      }
    }
  }
  if (number_of_actors_contributing_to_centroid > 1) {
    centroid /= number_of_actors_contributing_to_centroid;
    if (!initial_position_provided_) {
      initial_position_ = centroid;
    }

    for_each(VisualPhysicsActorId unique_id, pile_member_ids_) {
      VisualPhysicsActor* actor = room_->actor_with_unique_id(unique_id);
      if (actor != NULL) {
        if (!actor->breakable()) {
          flattened_pile_members.push_back(actor);
          BumpPose final_pose_of_actor = actor->destination_pose();
          pile_member_offsets_.push_back(final_pose_of_actor.position - initial_position_);
          pile_member_original_orientations_.push_back(final_pose_of_actor.orientation);
        } else {
          // TODO: we rely here on a bunch of lists being in the same order
          // they are, so it's "ok", but not a great architecture
          QList<Ogre::Vector3> children_offsets = actor->children_offsets();
          QList<Ogre::Quaternion> children_original_orientations = actor->children_orientations();
          int i = 0;
          for_each(VisualPhysicsActor* child_actor, actor->children()) {
            if (child_actor->actor_type() != BUMP_DUMMY) {
              flattened_pile_members.push_back(child_actor);
                pile_member_offsets_.push_back(children_offsets.at(i));
              pile_member_original_orientations_.push_back(children_original_orientations.at(i));
              i++;
            }
          }
          piles_to_break.push_back(actor);
        }
      }
    }

    if (piles_to_break.count() > 0) {
      pile_break_undo_command_ = new PileBreakUndoCommand(piles_to_break, room_,
                                                          scene_manager_, physics_,
                                                          DONT_CONSTRAIN_FINAL_MEMBER_POSITIONS);
      pile_break_undo_command_->redo();
    }
  }

  if (number_of_actors_contributing_to_centroid > 1) {
    BumpPile* pile;
    if (first_run_) {
      pile = new BumpPile(scene_manager_, physics_, room_, pile_id_);
      pile_id_ = pile->unique_id();
      first_run_ = false;
    } else {
      pile = new BumpPile(scene_manager_, physics_, room_, pile_id_);
    }
    pile->initWithActors(flattened_pile_members, initial_position_,
                         pile_member_offsets_, pile_member_original_orientations_);
    pile->stackViewOnInit();

    if (pile_name_ != "") {
      pile->rename(pile_name_);
    }

    // TODO: why is it necessary to deselect first to make highlight show up reliably?
    pile->set_selected(false);
    pile->set_selected(true);
  } else {
    // if there are less than 2 actors, we can't make this pile, so we should should
    // execute the next command in the redo stack.
      UndoRedoStack::last_command_changed_something = false;
  }
}
