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

#include "BumpTop/UndoCommands/AddToPileUndoCommand.h"

#include "BumpTop/BumpPile.h"
#include "BumpTop/UndoCommands/PileBreakUndoCommand.h"
#include "BumpTop/Room.h"
#include "BumpTop/RoomSurface.h"
#include "BumpTop/UndoRedoStack.h"

AddToPileUndoCommand::AddToPileUndoCommand(VisualPhysicsActor* pile, VisualPhysicsActorList pile_members, Room* room,
                                       Ogre::SceneManager* scene_manager, Physics* physics)
: room_(room),
scene_manager_(scene_manager),
physics_(physics),
first_run_(true),
pile_break_undo_command_(NULL) {
  pile_id_ = pile->unique_id();
  pile_member_ids_ = QList<VisualPhysicsActorId>();
  // We don't want to pile items from the walls
  for_each(VisualPhysicsActor* actor, pile_members) {
    if (!actor->room_surface()->is_pinnable_receiver()) {
      pile_member_ids_.push_back(actor->unique_id());
      if (!actor->children().isEmpty()) {
        VisualPhysicsActorList flattened_children = actor->flattenedChildren();
        for_each(VisualPhysicsActor* child, flattened_children) {
          pile_flattened_member_ids_.push_back(child->unique_id());
        }
      } else {
         pile_flattened_member_ids_.push_back(actor->unique_id());
      }
    } else {
      actor->set_selected(false);
    }
  }
}

AddToPileUndoCommand::~AddToPileUndoCommand() {
}

void AddToPileUndoCommand::undo() {
  VisualPhysicsActor* pile = room_->actor_with_unique_id(pile_id_);
  if (pile != NULL) {
    pile->breakItemsOutOfPile(pile_flattened_member_ids_);
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

void AddToPileUndoCommand::redo() {
  VisualPhysicsActor* pile = room_->actor_with_unique_id(pile_id_);
  if (pile == NULL) {
    UndoRedoStack::last_command_changed_something = false;
    return;
  }

  VisualPhysicsActorList flattened_pile_members;
  VisualPhysicsActorList piles_to_break;

  int num_actors_dragged_in = 0;
  for_each(VisualPhysicsActorId unique_id, pile_member_ids_) {
    VisualPhysicsActor* actor = room_->actor_with_unique_id(unique_id);
    if (actor != NULL) {
      num_actors_dragged_in++;
      if (!actor->breakable()) {
        flattened_pile_members.push_back(actor);
        BumpPose final_pose_of_actor = actor->destination_pose();
        pile_member_offsets_.push_back(final_pose_of_actor.position - pile->position());
        pile_member_original_orientations_.push_back(final_pose_of_actor.orientation);
      } else {
        // TODO: we rely here on a bunch of lists being in the same order
        // they are, so it's "ok", but not a great architecture
        // The list orders are now all tied to pile_actor_ids_in_order_
        // so this is not a problem anymore. REMOVE THIS TODO!
        QList<Ogre::Vector3> children_offsets = actor->children_offsets();
        QList<Ogre::Quaternion> children_original_orientations = actor->children_orientations();
        int i = 0;
        for_each(VisualPhysicsActor* child_actor, actor->children()) {
          flattened_pile_members.push_back(child_actor);
          pile_member_offsets_.push_back(children_offsets.at(i));
          pile_member_original_orientations_.push_back(children_original_orientations.at(i));
          i++;
        }
        piles_to_break.push_back(actor);
      }
    }
  }

  if (num_actors_dragged_in == 0) {
    UndoRedoStack::last_command_changed_something = false;
    return;
  }

  if (piles_to_break.count() > 0) {
    pile_break_undo_command_ = new PileBreakUndoCommand(piles_to_break, room_,
                                                        scene_manager_, physics_,
                                                        DONT_CONSTRAIN_FINAL_MEMBER_POSITIONS);
    pile_break_undo_command_->redo();
  }

  for_each(VisualPhysicsActor* actor, flattened_pile_members) {
    pile->addActorToPileAndUpdatePileView(actor, actor->position() - pile->position(), actor->orientation());
  }
  pile->set_selected(true);
}
