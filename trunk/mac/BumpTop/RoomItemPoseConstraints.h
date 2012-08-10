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

#ifndef BUMPTOP_ROOMITEMPOSECONSTRAINTS_H_
#define BUMPTOP_ROOMITEMPOSECONSTRAINTS_H_

#include <utility>

#include "BumpTop/OgreHelpers.h"
#include "BumpTop/VisualPhysicsActor.h"
#include "BumpTop/VisualPhysicsActorId.h"

class Room;

void tightenBoundingBox(Ogre::AxisAlignedBox* box);
QHash<VisualPhysicsActorId, BumpPose> getActorPosesConstrainedToRoom(QHash<VisualPhysicsActorId, BumpPose> item_poses, Room* room);  // NOLINT
QHash<VisualPhysicsActor*, BumpPose> getActorPosesConstrainedToRoom(QHash<VisualPhysicsActor*, BumpPose> requested_actor_poses, Room* room);  // NOLINT
QHash<VisualPhysicsActorId, BumpPose> getActorPosesConstrainedToNoIntersections(QHash<VisualPhysicsActorId, BumpPose> requested_actor_poses, Room* room);  // NOLINT
QHash<VisualPhysicsActor*, BumpPose> getActorPosesConstrainedToNoIntersections(QHash<VisualPhysicsActor*, BumpPose> requested_actor_poses, Room* room); // NOLINT

void repositionActorsConstrainedToRoom(QList<VisualPhysicsActor*> actors_to_reposition, Room* room, bool exclude_actors_with_physics_disabled = false);  // NOLINT
BumpPose getActorPoseConstrainedToRoomAndNoIntersections(VisualPhysicsActor* actor, Room* room);
Ogre::Vector3 getPositionConstrainedToRoom(Ogre::Vector3 position, Room* room);
std::pair<bool, Ogre::Vector3> getPositionConstrainedToRoom(Ogre::AxisAlignedBox bounding_box, Room* room);
bool actorIntersectsAnyOtherActor(VisualPhysicsActorId actor_id, BumpPose requested_pose, Room* room);

#endif  // BUMPTOP_ROOMITEMPOSECONSTRAINTS_H_
