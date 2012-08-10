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

#include "BumpTop/PhysicsActorMotionState.h"

#include "BumpTop/Box.h"
#include "BumpTop/OgreBulletConverter.h"
#include "BumpTop/PhysicsActor.h"

PhysicsActorMotionState::PhysicsActorMotionState(PhysicsActor *physics_actor)
: physics_actor_(physics_actor) {
    for (int i = 0; i < NUM_FRAMES_TO_UPDATE_AFTER_SLEEPING; i++)
      was_physics_actor_sleeping_n_frames_ago[i] = false;
}

PhysicsActorMotionState::~PhysicsActorMotionState() {
}

void PhysicsActorMotionState::getWorldTransform(btTransform &world_transform) const {  // NOLINT
  world_transform.setRotation(toBt(physics_actor_->orientation()));
  world_transform.setOrigin(toBt(physics_actor_->position()));
}

void PhysicsActorMotionState::setWorldTransform(const btTransform &world_transform) {
  for (int i = NUM_FRAMES_TO_UPDATE_AFTER_SLEEPING - 1; i > 0; i--)
    was_physics_actor_sleeping_n_frames_ago[i] = was_physics_actor_sleeping_n_frames_ago[i-1];
  was_physics_actor_sleeping_n_frames_ago[0] = physics_actor_->isSleeping();

  // A little optimization: check the first and last elements first
  bool update_ogre_scene_node = !was_physics_actor_sleeping_n_frames_ago[0] ||
                                !was_physics_actor_sleeping_n_frames_ago[NUM_FRAMES_TO_UPDATE_AFTER_SLEEPING - 1];

  if (!update_ogre_scene_node) {
    for (int i = 1; i < NUM_FRAMES_TO_UPDATE_AFTER_SLEEPING - 1; i++) {
      if (!was_physics_actor_sleeping_n_frames_ago[i]) {
        update_ogre_scene_node = true;
        break;
      }
    }
  }

  if (update_ogre_scene_node) {
    physics_actor_->_poseUpdatedByPhysics(world_transform);
  }
}
