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

#include "BumpTop/UndoCommands/GridViewUndoCommand.h"

#include "BumpTop/AnimationManager.h"
#include "BumpTop/BumpBoxLabel.h"
#include "BumpTop/Room.h"
#include "BumpTop/RoomItemPoseConstraints.h"
#include "BumpTop/RoomSurface.h"
#include "BumpTop/UndoCommands/PileByTypeUndoCommand.h"
#include "BumpTop/UndoCommands/RoomStateUndoCommand.h"
#include "BumpTop/UndoCommands/ShrinkUndoCommand.h"
#include "BumpTop/UndoRedoStack.h"
#include "BumpTop/VisualPhysicsActor.h"
#include "BumpTop/VisualPhysicsActorAnimation.h"

const Ogre::Real kAdditionalLabelPadding = 24;

GridViewUndoCommand::GridViewUndoCommand(VisualPhysicsActorList actors, Room* room,
                                         Ogre::SceneManager* scene_manager, Physics* physics)
: room_(room),
scene_manager_(scene_manager),
physics_(physics),
first_run_(true),
undo_command_(NULL) {
  for_each(VisualPhysicsActor* actor, actors) {
    if (actor != NULL) {
      if (!actor->room_surface()->is_pinnable_receiver()
          && !actor->is_new_items_pile()) {
        actors_ids_.push_back(actor->unique_id());
      }
    }
  }
}

GridViewUndoCommand::~GridViewUndoCommand() {
}

void GridViewUndoCommand::undo() {
  command_changed_something_ = false;
  // if there is 1 or less actors then undo will not change anything
  QList<VisualPhysicsActorId> valid_actors_ids = getFilteredActorIdList(actors_ids_);
  if (valid_actors_ids.count() <= 1) {
    UndoRedoStack::last_command_changed_something = command_changed_something_;
    return;
  }

  // undo shrink command
  for_each(ShrinkUndoCommand* shrink_command, shrink_commands_) {
    shrink_command->undo();
    command_changed_something_ = command_changed_something_ || UndoRedoStack::last_command_changed_something;
  }

  // undo the state of room
  undo_command_->undo();
  command_changed_something_ = command_changed_something_ || UndoRedoStack::last_command_changed_something;

  UndoRedoStack::last_command_changed_something = command_changed_something_;
}

void GridViewUndoCommand::redo() {
  command_changed_something_ = false;

  // get the list actors that are in the room and are not pinned on to walls
  QList<VisualPhysicsActorId> valid_actors_ids = getFilteredActorIdList(actors_ids_);
  if (valid_actors_ids.count() <= 1) {
    UndoRedoStack::last_command_changed_something = command_changed_something_;
    return;
  }

  // on first run, save the state of room for undo command
  // the state saved will not be modified after
  if (first_run_) {
    undo_command_ = new RoomStateUndoCommand(room_);
    room_->updateCurrentState();
    undo_command_->set_last_state(room_->current_state());
  }

  // clear the shrink_commands from last redo
  shrink_commands_ = QList<ShrinkUndoCommand*> ();

  Ogre::Real margin = getMarginBetweenItems(valid_actors_ids);
  QHash<VisualPhysicsActorId, BumpPose> desired_poses = getGriddedPositionOfActorsAndShrinkActorsIfNeeded(valid_actors_ids, margin);  // NOLINT

  QHash<VisualPhysicsActorId, BumpPose> constrained_poses = getActorPosesConstrainedToRoom(desired_poses, room_);
  constrained_poses = getActorPosesConstrainedToNoIntersections(constrained_poses, room_);

  // all actors should be valid actors since this is using the list of actors_ids filtered from
  // getFilteredActorIdList function
  for_each(VisualPhysicsActorId actor_id, desired_poses.keys()) {
    VisualPhysicsActor* actor = room_->actor_with_unique_id(actor_id);
    AnimationManager::singleton()->endAnimationsForActor(actor, AnimationManager::STOP_AT_CURRENT_STATE);  // NOLINT
    VisualPhysicsActorAnimation* actor_animation;
    actor_animation = new VisualPhysicsActorAnimation(actor, 250,
                                                      constrained_poses[actor_id].position,
                                                      constrained_poses[actor_id].orientation);
    actor_animation->start();
    actor->set_selected(true);
    command_changed_something_ = true;
  }

  first_run_ = false;
  UndoRedoStack::last_command_changed_something = command_changed_something_;
}

QHash<VisualPhysicsActorId, BumpPose> GridViewUndoCommand::getGriddedPositionOfActorsAndShrinkActorsIfNeeded(QList<VisualPhysicsActorId> actors_ids, Ogre::Real margin) {  // NOLINT
  VisualPhysicsActorList actors;
  Ogre::Real width_of_largest_actor;
  Ogre::Vector2 centroid_of_original_actors_positions = Ogre::Vector2(0, 0);
  int number_of_actors_per_row;
  int number_of_rows;

  for_each(VisualPhysicsActorId actor_id, actors_ids) {
    VisualPhysicsActor* actor = room_->actor_with_unique_id(actor_id);
    actors.append(actor);
    centroid_of_original_actors_positions += Ogre::Vector2(actor->position().x, actor->position().z);
  }
  centroid_of_original_actors_positions = centroid_of_original_actors_positions/actors.count();
  width_of_largest_actor = getWidthOfLargestActor(actors);
  number_of_actors_per_row = Ogre::Math().ICeil(sqrt(actors.count()));
  number_of_rows = Ogre::Math().ICeil(actors.count()*1.0/number_of_actors_per_row);

  // if bounding box created with width_of_largest_actor, number_of_actors_per_row and number_of_rows
  // is larger than floor's size than shrink the largest actors and try again
  if (width_of_largest_actor*number_of_actors_per_row +
      margin*(number_of_actors_per_row - 1) > room_->floor_width() ||
      width_of_largest_actor*number_of_rows +
      margin*(number_of_rows - 1) > room_->floor_depth()) {
    // recurse only if shrink command did something else continue on
    if (shrinkActorsToFitIntoRoom(actors, width_of_largest_actor)) {
      return getGriddedPositionOfActorsAndShrinkActorsIfNeeded(actors_ids, getMarginBetweenItems(actors_ids));
    }
  }

  int row_number_of_actor = 1;
  int column_number_of_actor = 1;
  QHash<VisualPhysicsActorId, Ogre::Vector3> actors_desired_positions;

  // assign each actor to its position with the first actor (top left) at (0, 0)
  for_each(VisualPhysicsActor* actor, actors) {
    if (column_number_of_actor > number_of_actors_per_row) {
      row_number_of_actor++;
      column_number_of_actor = 1;
    }
    Ogre::Vector3 actor_position = Ogre::Vector3((width_of_largest_actor + margin)*  // NOLINT
                                                 (column_number_of_actor - 1),
                                                 actor->size().y + 5,
                                                 (width_of_largest_actor + margin)*  // NOLINT
                                                 (row_number_of_actor - 1));
    actors_desired_positions.insert(actor->unique_id(), actor_position);
    column_number_of_actor++;
  }


  Ogre::Real half_of_bounding_box_width = (width_of_largest_actor*number_of_actors_per_row +
                                           margin*(number_of_actors_per_row - 1))/2;  // NOLINT
  Ogre::Real half_of_bounding_box_height = (width_of_largest_actor*number_of_rows +
                                            margin*(number_of_rows - 1))/2;

  // adjust centroid_of_original_actors_positions to put all actors in room
  Ogre::AxisAlignedBox bounding_box = Ogre::AxisAlignedBox(centroid_of_original_actors_positions.x - half_of_bounding_box_width,  // NOLINT
                                                           10, centroid_of_original_actors_positions.y - half_of_bounding_box_height,  // NOLINT
                                                           centroid_of_original_actors_positions.x + half_of_bounding_box_width,  // NOLINT
                                                           10, centroid_of_original_actors_positions.y + half_of_bounding_box_height);  // NOLINT
  Ogre::Vector3 adjusted_centroid = getPositionConstrainedToRoom(bounding_box, room_).second;

  // get the centroid of actors_desired_positions
  Ogre::Vector2 centroid_of_actors_at_zero_zero = Ogre::Vector2((width_of_largest_actor*(number_of_actors_per_row - 1) +
                                                                     margin*(number_of_actors_per_row - 1))/2,  // NOLINT
                                                                     (width_of_largest_actor*(number_of_rows - 1) +
                                                                      margin*(number_of_rows - 1))/2);  // NOLINT

  // determine offset of actors_desired_positions to adjusted_centroid
  Ogre::Vector2 offset_of_actors_to_centroid = Ogre::Vector2(adjusted_centroid.x - centroid_of_actors_at_zero_zero.x,
                                                             adjusted_centroid.z - centroid_of_actors_at_zero_zero.y);

  QHash<VisualPhysicsActorId, BumpPose> actors_desired_poses;
  for_each(VisualPhysicsActorId actor_id, actors_desired_positions.keys()) {
    actors_desired_positions[actor_id].x += offset_of_actors_to_centroid.x;
    actors_desired_positions[actor_id].z += offset_of_actors_to_centroid.y;
    BumpPose actor_desired_pose = BumpPose(actors_desired_positions[actor_id], Ogre::Quaternion::IDENTITY);
    actors_desired_poses.insert(actor_id, actor_desired_pose);
  }
  return actors_desired_poses;
}

Ogre::Real GridViewUndoCommand::getWidthOfLargestActor(VisualPhysicsActorList actors) {
  Ogre::Real width_of_largest_actor = 0;
  for_each(VisualPhysicsActor* actor, actors) {
    if (actor->size().x > width_of_largest_actor)
      width_of_largest_actor = actor->size().x;
  }
  return width_of_largest_actor;
}

QList<VisualPhysicsActorId> GridViewUndoCommand::getFilteredActorIdList(QList<VisualPhysicsActorId> actors_ids) {
  QList<VisualPhysicsActorId> valid_actors_ids;
  // we want to keep only actors that are in the room or not pinned on the walls
  for_each(VisualPhysicsActorId actor_id, actors_ids) {
    VisualPhysicsActor* actor = room_->actor_with_unique_id(actor_id);
    if (actor != NULL) {
      if (!actor->room_surface()->is_pinnable_receiver()) {
        valid_actors_ids.append(actor_id);
      }
    }
  }
  return valid_actors_ids;
}

bool GridViewUndoCommand::shrinkActorsToFitIntoRoom(VisualPhysicsActorList actors, Ogre::Real width_of_largest_actor) {
  // initialize shrink_commands_ and shrink all actors that has the width of the width_of_largest_actor
  for_each(VisualPhysicsActor* actor, actors) {
    if (actor->size().x == width_of_largest_actor) {
      VisualPhysicsActorList actor_to_shrink;
      actor_to_shrink.append(actor);
      ShrinkUndoCommand* shrink_command = new ShrinkUndoCommand(actor_to_shrink, room_);
      shrink_commands_.push_back(shrink_command);
      shrink_command->redo();
      command_changed_something_ = UndoRedoStack::last_command_changed_something;
    }
  }
  return UndoRedoStack::last_command_changed_something;
}

Ogre::Real GridViewUndoCommand::getMarginBetweenItems(QList<VisualPhysicsActorId> actors_ids) {
  Ogre::Real max_label_margin_width = 0;
  for_each(VisualPhysicsActorId actor_id, actors_ids) {
    VisualPhysicsActor* actor = room_->actor_with_unique_id(actor_id);
    if (actor->label() != NULL) {
      if (actor->label()->width_of_drawn_region() - actor->size().z > max_label_margin_width) {
        max_label_margin_width = actor->label()->width_of_drawn_region() - actor->size().z;
      }
    }
  }

  max_label_margin_width += kAdditionalLabelPadding;

  if (max_label_margin_width > PileByTypeUndoCommand::kMarginInBetweenPiles) {
    return max_label_margin_width;
  } else {
    return PileByTypeUndoCommand::kMarginInBetweenPiles;
  }
}
