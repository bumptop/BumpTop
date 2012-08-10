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

#include "BumpTop/UndoCommands/RoomUndoRedoState.h"

#include "BumpTop/Room.h"
#include "BumpTop/RoomSurface.h"
#include "BumpTop/BumpBox.h"

RoomUndoRedoState::RoomUndoRedoState() {
}

RoomUndoRedoState::~RoomUndoRedoState() {
}

void RoomUndoRedoState::add_actor(VisualPhysicsActor* actor) {
  actor_poses_.insert(actor->unique_id(), actor->pose());
  actor_surfaces_.insert(actor->unique_id(), actor->room_surface());
}

bool RoomUndoRedoState::contains(VisualPhysicsActorId unique_id) {
  return actor_poses_.contains(unique_id);
}

Ogre::Vector3 RoomUndoRedoState::position_for_id(VisualPhysicsActorId unique_id) {
  return actor_poses_[unique_id].position;
}

Ogre::Quaternion RoomUndoRedoState::orientation_for_id(VisualPhysicsActorId unique_id) {
  return actor_poses_[unique_id].orientation;
}

RoomSurface* RoomUndoRedoState::surface_for_id(VisualPhysicsActorId unique_id) {
  return actor_surfaces_[unique_id];
}

QHash<VisualPhysicsActorId, BumpPose> RoomUndoRedoState::actor_poses() {
  return actor_poses_;
}


