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

#include "BumpTop/Physics.h"

#include "BumpTop/PhysicsActorMotionState.h"

Physics::Physics()
: dynamics_world_(NULL) {
}

Physics::~Physics() {
  delete dynamics_world_;
  delete solver_;
  delete broadphase_;
  delete dispatcher_;
  delete collision_configuration_;
}

void Physics::init() {
  // taken from bullet physics hello world example
  // collision configuration contains default setup for memory, collision setup. Advanced users can
  //  create their own configuration.
  collision_configuration_ = new btDefaultCollisionConfiguration();

  // use the default collision dispatcher. For parallel processing you can use a diffent dispatcher
  //  (see Extras/BulletMultiThreaded)
  dispatcher_ = new btCollisionDispatcher(collision_configuration_);


  broadphase_ = new btDbvtBroadphase();

  // the default constraint solver. For parallel processing you can use a different solver
  //  (see Extras/BulletMultiThreaded)
  solver_ = new btSequentialImpulseConstraintSolver();

  dynamics_world_ = new btDiscreteDynamicsWorld(dispatcher_, broadphase_,
                                                solver_, collision_configuration_);

  dynamics_world_->setGravity(btVector3(0, -20, 0));
}

btDiscreteDynamicsWorld* Physics::dynamics_world() {
  return dynamics_world_;
}

int Physics::stepSimulation(btScalar timeStep, int maxSubSteps, btScalar fixedTimeStep) {
  int num_steps = dynamics_world_->stepSimulation(timeStep, maxSubSteps, fixedTimeStep);
  // When a body's transform goes NaN mid-step, Bullet's AABB-overflow guard
  // (btCollisionWorld::updateSingleAabb) silently freezes it with
  // DISABLE_SIMULATION — leaving the item invisible (NaN pose) and
  // unmovable forever. The app never uses DISABLE_SIMULATION itself, so any
  // body in that state was guard-frozen: restore its last finite pose and
  // let it sleep normally so it stays grabbable.
  btCollisionObjectArray& objects = dynamics_world_->getCollisionObjectArray();
  for (int i = 0; i < objects.size(); i++) {
    btRigidBody* body = btRigidBody::upcast(objects[i]);
    if (body != NULL && body->getActivationState() == DISABLE_SIMULATION &&
        body->getMotionState() != NULL) {
      static int rescue_log_count = 0;
      if (rescue_log_count++ < 40)
        fprintf(stderr, "[nan] rescuing body frozen by Bullet's AABB-overflow guard\n");
      // All motion states in this app are PhysicsActorMotionState.
      static_cast<PhysicsActorMotionState*>(body->getMotionState())->restoreLastGoodTransform(body);
      body->forceActivationState(ISLAND_SLEEPING);
    }
  }
  return num_steps;
}
