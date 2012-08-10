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

#include "BumpTop/UndoCommands/RoomStateUndoCommand.h"

#include "BumpTop/AnimationManager.h"
#include "BumpTop/BumpBox.h"
#include "BumpTop/for_each.h"
#include "BumpTop/Room.h"
#include "BumpTop/RoomItemPoseConstraints.h"
#include "BumpTop/RoomSurface.h"
#include "BumpTop/UndoCommands/RoomUndoRedoState.h"
#include "BumpTop/UndoRedoStack.h"
#include "BumpTop/VisualPhysicsActorAnimation.h"

RoomStateUndoCommand::RoomStateUndoCommand(Room* room)
: QUndoCommand(),
  first_redo_happened_(false),
  room_(room) {
}

RoomStateUndoCommand::~RoomStateUndoCommand() {
}

void RoomStateUndoCommand::updateRoomFromRoomUndoRedoState(boost::shared_ptr<RoomUndoRedoState> state) {
  QHash<VisualPhysicsActorId, BumpPose> desired_poses = state->actor_poses();
  QHash<VisualPhysicsActorId, BumpPose> desired_poses_for_actors_in_room;
  QHash<VisualPhysicsActorId, BumpPose> desired_poses_for_actors_in_room_that_have_moved;


  // getActorPosesConstrainedToNoIntersections requires all items passed to it to be in the room
  // Any items known about by the RoomUndoRedoState but which are no longer in the room will be ignored
  for_each(VisualPhysicsActorId actor_id, desired_poses.keys()) {
    if (room_->containsActorWithId(actor_id)) {
      desired_poses_for_actors_in_room[actor_id] = desired_poses[actor_id];
    }
  }

  for_each(VisualPhysicsActorId actor_id, desired_poses_for_actors_in_room.keys()) {
    if (!(desired_poses[actor_id].approximatelyEquals(room_->actor_with_unique_id(actor_id)->pose()))) {
      desired_poses_for_actors_in_room_that_have_moved[actor_id] = desired_poses_for_actors_in_room[actor_id];
    }
  }

  if (desired_poses_for_actors_in_room_that_have_moved.count() > 0) {
    QHash<VisualPhysicsActorId, BumpPose> constrained_poses = getActorPosesConstrainedToRoom(desired_poses_for_actors_in_room_that_have_moved, room_);  // NOLINT
    // TODO: we don't really need to be repositioning quite this much -- use some more intelligent
    // ogic about when things might actually need to be repositioned. For example, we likely only need to
    // check items' poses against those of items that are new to the room as far as this undo command is conncerend
    constrained_poses = getActorPosesConstrainedToNoIntersections(constrained_poses, room_);  // NOLINT

    // Animate from the initial position to the final position
    VisualPhysicsActorAnimation* actor_animation;

    for_each(VisualPhysicsActorId actor_id, constrained_poses.keys()) {
      VisualPhysicsActor* actor = room_->actor_with_unique_id(actor_id);
      AnimationManager::singleton()->endAnimationsForActor(actor,
                                                          AnimationManager::MOVE_TO_FINAL_STATE);
      actor->set_physics_enabled(true);
      actor_animation = new VisualPhysicsActorAnimation(actor, 250,
                                                        constrained_poses[actor_id].position,
                                                        constrained_poses[actor_id].orientation,
                                                        actor->actor_type() == GRIDDED_PILE ? NULL : state->surface_for_id(actor_id));  // NOLINT

      actor_animation->start();
    }
  } else {
    UndoRedoStack::last_command_changed_something = false;
  }
}

void RoomStateUndoCommand::undo() {
  if (last_state_ != NULL) {
    updateRoomFromRoomUndoRedoState(last_state_);
  }
}

void RoomStateUndoCommand::redo() {
  if (first_redo_happened_) {
    if (current_state_ != NULL) {
      updateRoomFromRoomUndoRedoState(current_state_);
    }
  } else {
    first_redo_happened_ = true;
  }
}

void RoomStateUndoCommand::set_current_state(boost::shared_ptr<RoomUndoRedoState> state) {
  current_state_ = state;
}

void RoomStateUndoCommand::set_last_state(boost::shared_ptr<RoomUndoRedoState> state) {
  last_state_ = state;
}

boost::shared_ptr<RoomUndoRedoState> RoomStateUndoCommand::current_state() {
  return current_state_;
}

boost::shared_ptr<RoomUndoRedoState> RoomStateUndoCommand::last_state() {
  return last_state_;
}
