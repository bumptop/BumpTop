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

#include "BumpTop/PhysicsActor.h"

#include "BumpTop/Box.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/OgreBulletConverter.h"
#include "BumpTop/Physics.h"
#include "BumpTop/VisualPhysicsActor.h"

RigidBodyThatMarksGlobalStateAsChanged::RigidBodyThatMarksGlobalStateAsChanged(const btRigidBodyConstructionInfo &constructionInfo,  // NOLINT
                                                                               BumpTopApp* app)
: btRigidBody(constructionInfo),
  app_(app) {
}

RigidBodyThatMarksGlobalStateAsChanged::~RigidBodyThatMarksGlobalStateAsChanged() {
}

void RigidBodyThatMarksGlobalStateAsChanged::activateAndMarkGlobalStateAsChanged(bool forceActivation) {
  activate(forceActivation);
  app_->markGlobalStateAsChanged();
}

void RigidBodyThatMarksGlobalStateAsChanged::activate(bool forceActivation) {
  btRigidBody::activate(forceActivation);
}

PhysicsActor::PhysicsActor(Physics* physics, Ogre::Real mass, bool physics_enabled)
: physics_(physics),
  mass_(mass),
  physics_enabled_(physics_enabled),
  owner_(NULL),
  constraint_(NULL),
  constraint_is_added_(false),
  collides_with_walls_only_(false),
  collides_with_gridded_piles_(false),
  friction_(0) {
}

PhysicsActor::~PhysicsActor() {
  btCollisionShape *collision_shape = rigid_body_->getCollisionShape();

  removeConstraints();
  delete rigid_body_->getMotionState();
  if (physics_enabled_) {
    removeActorFromDynamicsWorld();
  }
  delete rigid_body_;
  delete collision_shape;
}

void PhysicsActor::set_scale(Ogre::Vector3 scale) {
  rigid_body_->getCollisionShape()->setLocalScaling(toBt(scale));
  btVector3 local_inertia(0, 0, 0);
  rigid_body_->getCollisionShape()->calculateLocalInertia(mass_, local_inertia);
  rigid_body_->setMassProps(mass_, local_inertia);
  rigid_body_->updateInertiaTensor();

  // This is required to make the new size update immediately, not sure why
  if (physics_enabled_) {
    removeActorFromDynamicsWorld();
    addActorToDynamicsWorld();
  }
}

RigidBodyThatMarksGlobalStateAsChanged* PhysicsActor::rigid_body() {
  return rigid_body_;
}

void PhysicsActor::removeConstraints() {
  if (constraint_is_added_) {
    physics_->dynamics_world()->removeConstraint(constraint_);
    constraint_is_added_ = false;
  }
  if (constraint_ != NULL) {
    delete constraint_;
    constraint_ = NULL;
  }
}

void PhysicsActor::setCollisionsToOnlyWalls() {
  collides_with_walls_only_ = true;
  if (physics_enabled_) {
    removeActorFromDynamicsWorld();
    addActorToDynamicsWorld();
  }
}

void PhysicsActor::setCollisionsToAllActors() {
  collides_with_walls_only_ = false;
  if (physics_enabled_) {
    removeActorFromDynamicsWorld();
    addActorToDynamicsWorld();
  }
}

void PhysicsActor::setCollisionsToGriddedPiles() {
  collides_with_gridded_piles_ = true;
  if (physics_enabled_) {
    removeActorFromDynamicsWorld();
    addActorToDynamicsWorld();
  }
}

int16_t PhysicsActor::collision_group() {
  return actor_group_;
}

int16_t PhysicsActor::collision_mask() {
  return actor_collides_with_;
}

void PhysicsActor::setFriction(Ogre::Real friction) {
  btCollisionObject* collision_object = static_cast<btCollisionObject*>(rigid_body_);
  collision_object->setFriction(friction);
  friction_ = friction;
}

void PhysicsActor::setMass(Ogre::Real mass) {
  btCollisionShape* collision_shape = rigid_body_->getCollisionShape();
  btVector3 inertia = btVector3(0, 0, 0);
  collision_shape->calculateLocalInertia(mass, inertia);
  rigid_body_->setMassProps(mass, inertia);
  mass_ = mass;
  if (physics_enabled_) {
    removeActorFromDynamicsWorld();
    addActorToDynamicsWorld();
  }
}

bool PhysicsActor::collides_with_walls_only() {
  return collides_with_walls_only_;
}

bool PhysicsActor::collides_with_gridded_piles() {
  return collides_with_gridded_piles_;
}

static btRigidBody dummy_constraint_body_ = btRigidBody(0, 0, 0);
void PhysicsActor::set6DofConstraint(Ogre::Vector3 linear_lower_limit, Ogre::Vector3 linear_upper_limit,
                                     Ogre::Vector3 angular_lower_limit, Ogre::Vector3 angular_upper_limit) {
  removeConstraints();

  btTransform identity = btTransform::getIdentity();
  btTransform transform;
  transform.setIdentity();
  transform.setOrigin(rigid_body_->getWorldTransform().getOrigin());

  constraint_ = new btGeneric6DofConstraint(*rigid_body_, dummy_constraint_body_,
                                              identity, transform, false);

  constraint_->setAngularLowerLimit(toBt(angular_lower_limit));
  constraint_->setAngularUpperLimit(toBt(angular_upper_limit));
  constraint_->setLinearLowerLimit(toBt(linear_lower_limit));
  constraint_->setLinearUpperLimit(toBt(linear_upper_limit));

  if (physics_enabled_) {
    physics_->dynamics_world()->addConstraint(constraint_);
    constraint_is_added_ = true;
  }
}


void PhysicsActor::set_position(Ogre::Vector3 position) {
  position_ = toBt(position);
  updateTransform();
}

void PhysicsActor::set_orientation(Ogre::Quaternion orientation) {
  orientation_ = toBt(orientation);
  updateTransform();
}

void PhysicsActor::set_angular_velocity(Ogre::Vector3 angular_velocity) {
  rigid_body_->setAngularVelocity(toBt(angular_velocity));
}

void PhysicsActor::set_linear_velocity(Ogre::Vector3 linear_velocity) {
  rigid_body_->setLinearVelocity(toBt(linear_velocity));
}


Ogre::Vector3 PhysicsActor::linear_velocity() {
  return toOgre(rigid_body_->getLinearVelocity());
}

Ogre::Vector3 PhysicsActor::angular_velocity() {
  return toOgre(rigid_body_->getAngularVelocity());
}

Ogre::AxisAlignedBox PhysicsActor::world_bounding_box() {
  btVector3 aabbMin, aabbMax;
  rigid_body_->getAabb(aabbMin, aabbMax);
  Ogre::AxisAlignedBox bounding_box(toOgre(aabbMin), toOgre(aabbMax));
  return bounding_box;
}

Ogre::Matrix4 PhysicsActor::transform() {
  return toOgre(rigid_body_->getCenterOfMassTransform());
}

Ogre::Vector3 PhysicsActor::position() {
  return toOgre(position_);
}

Ogre::Quaternion PhysicsActor::orientation() {
  return toOgre(orientation_);
}

Ogre::Real PhysicsActor::mass() {
  return mass_;
}

void PhysicsActor::setPose(Ogre::Vector3 position, Ogre::Quaternion orientation) {
  position_ = toBt(position);
  orientation_ = toBt(orientation);
  updateTransform();
}

void PhysicsActor::updateTransform() {
  setTransform(btTransform(orientation_, position_));
}

void PhysicsActor::applyCentralImpulse(const Ogre::Vector3 &impulse) {
  rigid_body_->applyCentralImpulse(toBt(impulse));
}

void PhysicsActor::applyImpulse(const Ogre::Vector3 &impulse, const Ogre::Vector3 &rel_pos) {
  rigid_body_->applyImpulse(toBt(impulse), toBt(rel_pos));
}

void PhysicsActor::applyCentralForce(const Ogre::Vector3 &force) {
  rigid_body_->applyCentralForce(toBt(force));
}

void PhysicsActor::applyForce(const Ogre::Vector3 &force, const Ogre::Vector3 &rel_pos) {
  rigid_body_->applyForce(toBt(force), toBt(rel_pos));
}

void PhysicsActor::activate() {
  rigid_body_->activateAndMarkGlobalStateAsChanged(true);
}

void PhysicsActor::setTransform(const btTransform& transform) {
  rigid_body_->proceedToTransform(transform);
  rigid_body_->setAngularVelocity(btVector3(0, 0, 0));
  rigid_body_->setLinearVelocity(btVector3(0, 0, 0));
  activate();
}

void PhysicsActor::_poseUpdatedByPhysics(const btTransform& transform) {
  if (owner_ != NULL) {
    owner_->poseUpdatedByPhysics(transform);
  }
  BumpTopApp::singleton()->markGlobalStateAsChanged();
}

void PhysicsActor::set_owner(VisualPhysicsActor* visual_physics_actor) {
  owner_ = visual_physics_actor;
}

const Ogre::Vector3 PhysicsActor::world_position() {
  return toOgre(rigid_body_->getCenterOfMassPosition());
}

bool PhysicsActor::isSleeping() {
  return rigid_body_->getActivationState() == ISLAND_SLEEPING;
}

bool PhysicsActor::isWall() {
  if (owner_ != NULL) {
    return owner_->isWall();
  }
  return false;
}

void PhysicsActor::addActorToDynamicsWorld() {
  bool is_wall = isWall();

  if (collides_with_gridded_piles_ && is_wall) {
    // Floor for gridded pile
    actor_group_ = GRIDDED_PILE_FLOORS;
    actor_collides_with_ = ROOM_ACTORS;
  } else if (collides_with_gridded_piles_) {
    // Gridded pile
    actor_group_ = ROOM_ACTORS;
    actor_collides_with_ = ROOM_WALLS | GRIDDED_PILE_FLOORS;
  } else if (collides_with_walls_only_) {
    // Dragged items (don't collide with other actors)
    actor_group_ = ROOM_ACTORS;
    actor_collides_with_ = ROOM_WALLS;
  } else if (is_wall)  {
    actor_group_ = ROOM_WALLS;
    actor_collides_with_ = ROOM_WALLS | ROOM_ACTORS;
  } else {  // Group for dynamic actors, collide with other dynamics and static
    actor_group_ = ROOM_ACTORS;
    actor_collides_with_ = ROOM_WALLS | ROOM_ACTORS;
  }

  physics_->dynamics_world()->addRigidBody(rigid_body_, actor_group_, actor_collides_with_);

  if (constraint_ != NULL) {
    physics_->dynamics_world()->addConstraint(constraint_);
    constraint_is_added_ = true;
  }
}

void PhysicsActor::removeActorFromDynamicsWorld() {
  if (constraint_is_added_) {
    physics_->dynamics_world()->removeConstraint(constraint_);
    constraint_is_added_ = false;
  }
  physics_->dynamics_world()->removeCollisionObject(rigid_body_);
}


void PhysicsActor::set_physics_enabled(bool physics_enabled) {
  if (physics_enabled != physics_enabled_) {
    physics_enabled_ = physics_enabled;
    if (physics_enabled) {
      addActorToDynamicsWorld();
    } else {
      removeActorFromDynamicsWorld();
    }
  }
}

bool PhysicsActor::physics_enabled() {
  return physics_enabled_;
}


#include "BumpTop/moc/moc_PhysicsActor.cpp"
