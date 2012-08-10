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
  return dynamics_world_->stepSimulation(timeStep, maxSubSteps, fixedTimeStep);
}
