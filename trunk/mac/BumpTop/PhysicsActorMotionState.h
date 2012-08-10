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

#ifndef BUMPTOP_PHYSICSACTORMOTIONSTATE_H_
#define BUMPTOP_PHYSICSACTORMOTIONSTATE_H_

#include <btBulletDynamicsCommon.h>
#include <Ogre.h>

#define NUM_FRAMES_TO_UPDATE_AFTER_SLEEPING 10

class PhysicsActor;

class PhysicsActorMotionState : public btMotionState {
 public:
  explicit PhysicsActorMotionState(PhysicsActor *physics_actor);
  virtual ~PhysicsActorMotionState();

  virtual void getWorldTransform(btTransform &worldTrans) const; // NOLINT
  virtual void setWorldTransform(const btTransform &worldTrans);
 protected:
  PhysicsActor *physics_actor_;
  bool was_physics_actor_sleeping_n_frames_ago[NUM_FRAMES_TO_UPDATE_AFTER_SLEEPING];
};

#endif  // BUMPTOP_PHYSICSACTORMOTIONSTATE_H_

