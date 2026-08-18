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

#include <cmath>

#include "BumpTop/Box.h"
#include "BumpTop/OgreBulletConverter.h"
#include "BumpTop/PhysicsActor.h"
#include "BumpTop/QStringHelpers.h"
#include "BumpTop/VisualPhysicsActor.h"

PhysicsActorMotionState::PhysicsActorMotionState(PhysicsActor *physics_actor)
: physics_actor_(physics_actor),
  has_last_good_transform_(false) {
    for (int i = 0; i < NUM_FRAMES_TO_UPDATE_AFTER_SLEEPING; i++)
      was_physics_actor_sleeping_n_frames_ago[i] = false;
}

PhysicsActorMotionState::~PhysicsActorMotionState() {
}

void PhysicsActorMotionState::restoreLastGoodTransform(btRigidBody* body) {
  btTransform restore_transform;
  if (has_last_good_transform_) {
    restore_transform = last_good_transform_;
  } else {
    getWorldTransform(restore_transform);
  }
  body->setCenterOfMassTransform(restore_transform);
  body->setLinearVelocity(btVector3(0, 0, 0));
  body->setAngularVelocity(btVector3(0, 0, 0));
  body->clearForces();
  // The visuals may already show the poisoned pose; snap them back too.
  physics_actor_->_poseUpdatedByPhysics(restore_transform);
}

void PhysicsActorMotionState::getWorldTransform(btTransform &world_transform) const {  // NOLINT
  world_transform.setRotation(toBt(physics_actor_->orientation()));
  world_transform.setOrigin(toBt(physics_actor_->position()));
}

void PhysicsActorMotionState::setWorldTransform(const btTransform &world_transform) {
  const btVector3& origin = world_transform.getOrigin();
  btQuaternion rotation = world_transform.getRotation();
  bool transform_is_finite =
      std::isfinite(origin.x()) && std::isfinite(origin.y()) && std::isfinite(origin.z()) &&
      std::isfinite(rotation.x()) && std::isfinite(rotation.y()) &&
      std::isfinite(rotation.z()) && std::isfinite(rotation.w());
  if (!transform_is_finite) {
    // Bullet's solver can emit NaN from degenerate contact configurations
    // (e.g. coincident bodies -> zero-length contact normal). Never let it
    // reach the visuals or persist in the body: snap the body back to last
    // frame's pose and kill all motion, which also stops the NaN spreading
    // to other bodies through further collisions.
    static int nan_log_count = 0;
    if (nan_log_count++ < 40) {
      VisualPhysicsActor* owner = physics_actor_->owner();
      fprintf(stderr, "[nan] quarantined NaN from Bullet solver: type=%d path=%s\n",
              owner != NULL ? (int)owner->actor_type() : -1,
              owner != NULL ? utf8(owner->path()).c_str() : "?");
    }
    btRigidBody* body = physics_actor_->rigid_body();
    if (body != NULL) {
      restoreLastGoodTransform(body);
    }
    return;
  }
  last_good_transform_ = world_transform;
  has_last_good_transform_ = true;

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
