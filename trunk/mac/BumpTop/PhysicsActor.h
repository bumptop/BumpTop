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

#ifndef BUMPTOP_PHYSICSACTOR_H_
#define BUMPTOP_PHYSICSACTOR_H_

class Box;
class BumpTopApp;
class Physics;
class VisualPhysicsActor;

enum CollisionGroup {
  ROOM_WALLS = 1,
  ROOM_ACTORS = 2,
  GRIDDED_PILE_FLOORS = 4
};

class RigidBodyThatMarksGlobalStateAsChanged : public btRigidBody {
 public:
  RigidBodyThatMarksGlobalStateAsChanged(const btRigidBodyConstructionInfo &constructionInfo, BumpTopApp* app);
  ~RigidBodyThatMarksGlobalStateAsChanged();

  void activateAndMarkGlobalStateAsChanged(bool forceActivation = false);
 protected:
  void activate(bool forceActivation = false);
  BumpTopApp* app_;
};

class PhysicsActor : public QObject {
  Q_OBJECT

 public:
  explicit PhysicsActor(Physics* physics, Ogre::Real mass, bool phyics_enabled = true);
  ~PhysicsActor();

  virtual void set_scale(Ogre::Vector3 scale);
  virtual Ogre::Vector3 size() = 0;

  virtual void set_physics_enabled(bool physics_enabled);
  virtual bool physics_enabled();
  virtual RigidBodyThatMarksGlobalStateAsChanged* rigid_body();

  virtual void set6DofConstraint(Ogre::Vector3 linear_lower_limit, Ogre::Vector3 linear_upper_limit,
                                    Ogre::Vector3 angular_lower_limit, Ogre::Vector3 angular_upper_limit);
  virtual void removeConstraints();
  virtual void setCollisionsToOnlyWalls();
  virtual void setCollisionsToAllActors();
  virtual void setCollisionsToGriddedPiles();
  virtual int16_t collision_group();
  virtual int16_t collision_mask();
  virtual void setFriction(Ogre::Real friction);
  virtual void setMass(Ogre::Real mass);

  virtual bool collides_with_walls_only();
  virtual bool collides_with_gridded_piles();

  virtual void setPose(Ogre::Vector3 position, Ogre::Quaternion orientation);
  virtual void set_position(Ogre::Vector3 position);
  virtual void set_orientation(Ogre::Quaternion orientation);
  virtual Ogre::Vector3 position();
  virtual Ogre::Quaternion orientation();
  virtual Ogre::Real mass();
  virtual const Ogre::Vector3 world_position();

  virtual void set_angular_velocity(Ogre::Vector3 angular_velocity);
  virtual void set_linear_velocity(Ogre::Vector3 linear_velocity);
  virtual Ogre::Vector3 linear_velocity();
  virtual Ogre::Vector3 angular_velocity();
  virtual Ogre::AxisAlignedBox world_bounding_box();
  virtual Ogre::Matrix4 transform();

  virtual void _poseUpdatedByPhysics(const btTransform &worldTrans);

  virtual void applyCentralImpulse(const Ogre::Vector3 &impulse);
  virtual void applyCentralForce(const Ogre::Vector3 &force);
  virtual void applyImpulse(const Ogre::Vector3 &impulse, const Ogre::Vector3 &rel_pos);
  virtual void applyForce(const Ogre::Vector3 &force, const Ogre::Vector3 &rel_pos);
  virtual void activate();

  virtual void set_owner(VisualPhysicsActor* visual_physics_actor);
  virtual bool isSleeping();
  virtual bool isWall();
 protected:
  virtual void updateTransform();
  virtual void setTransform(const btTransform &transform);
  void addActorToDynamicsWorld();
  void removeActorFromDynamicsWorld();

  bool collides_with_walls_only_;
  bool collides_with_gridded_piles_;
  Physics *physics_;
  btVector3 position_;
  btQuaternion orientation_;
  Ogre::Real mass_;
  Ogre::Real friction_;
  RigidBodyThatMarksGlobalStateAsChanged* rigid_body_;
  btGeneric6DofConstraint* constraint_;
  bool constraint_is_added_;
  VisualPhysicsActor* owner_;

  bool physics_enabled_;
  int16_t actor_collides_with_;
  int16_t actor_group_;
};

#endif  // BUMPTOP_PHYSICSACTOR_H_

