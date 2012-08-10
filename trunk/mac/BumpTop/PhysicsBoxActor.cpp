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

#include "BumpTop/PhysicsBoxActor.h"

#include "BumpTop/BumpTopApp.h"
#include "BumpTop/OgreBulletConverter.h"
#include "BumpTop/Physics.h"
#include "BumpTop/PhysicsActorMotionState.h"

PhysicsBoxActor::PhysicsBoxActor(Physics* physics, Ogre::Real mass, Ogre::Vector3 size, bool physics_enabled)
: PhysicsActor(physics, mass, physics_enabled) {
  // set up bullet (physics) objects
  btCollisionShape *collision_shape = new btBoxShape(toBt(size) / 2.0);
  PhysicsActorMotionState *motion_state = new PhysicsActorMotionState(this);

  // rigidbody is dynamic if and only if mass is non zero, otherwise static
  bool is_dynamic = (mass_ != 0.f);

  btVector3 local_inertia(0, 0, 0);
  if (is_dynamic) {
    collision_shape->calculateLocalInertia(mass_, local_inertia);
  }

  btTransform start_transform(btQuaternion(1, 0, 0, 0), btVector3(0, 0, 0));

  btRigidBody::btRigidBodyConstructionInfo rigid_body_contructor_info(mass_, motion_state,
                                                                      collision_shape, local_inertia);
  rigid_body_ = new RigidBodyThatMarksGlobalStateAsChanged(rigid_body_contructor_info,
                                                     BumpTopApp::singleton());

  rigid_body_->setWorldTransform(start_transform);

  if (physics_enabled_) {
    addActorToDynamicsWorld();
  }
}

PhysicsBoxActor::~PhysicsBoxActor() {
}

Ogre::Vector3 PhysicsBoxActor::size() {
  btBoxShape *box = static_cast<btBoxShape*> (rigid_body_->getCollisionShape());
  return toOgre(box->getHalfExtentsWithMargin()*2);
}


#include "BumpTop/moc/moc_PhysicsBoxActor.cpp"
