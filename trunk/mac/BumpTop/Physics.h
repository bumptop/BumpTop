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

#ifndef BUMPTOP_PHYSICS_H_
#define BUMPTOP_PHYSICS_H_

#include <btBulletDynamicsCommon.h>

class Physics {
 public:
  Physics();
  virtual ~Physics();
  void init();

  btDiscreteDynamicsWorld* dynamics_world();
  virtual int stepSimulation(btScalar timeStep,
                             int maxSubSteps = 1,
                             btScalar fixedTimeStep = btScalar(1.) / btScalar(60.));

 protected:
  btDefaultCollisionConfiguration* collision_configuration_;
  btCollisionDispatcher* dispatcher_;
  btDbvtBroadphase* broadphase_;
  btSequentialImpulseConstraintSolver* solver_;
  btDiscreteDynamicsWorld* dynamics_world_;
};

#endif  // BUMPTOP_PHYSICS_H_
