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

#ifndef BUMPTOP_VISUALPHYSICSACTORANIMATION_H_
#define BUMPTOP_VISUALPHYSICSACTORANIMATION_H_

#if defined(OS_WIN)
#include <boost/cstdint.hpp>
using boost::int64_t;
#endif
#include "CppTweener.h"

struct BumpPose;
class RoomSurface;
class VisualPhysicsActor;
class VisualPhysicsActorAnimation : public QObject, public tween::TweenerListener {
  Q_OBJECT

 public:
  explicit VisualPhysicsActorAnimation(VisualPhysicsActor* visual_physics_actor,
                            int64_t length,
                            Ogre::Vector3 final_position,
                            Ogre::Quaternion final_orientation,
                            RoomSurface* final_surface = NULL,
                            Ogre::Real final_alpha_modulation_factor = 1,
                            Ogre::Real final_actor_size_factor = 1,
                            int16_t transition_style = tween::CUBIC);

  // TODO: when I changed this to virtual it seemed
  // to cause bad accesses
  ~VisualPhysicsActorAnimation();

  virtual tween::TweenerParam* tweener_param();
  virtual VisualPhysicsActor* visual_physics_actor();
  virtual bool is_pending_deletion();

  virtual void start();
  virtual void moveToFinalPositionAndEndAnimation();
  virtual void endAnimation();
  virtual BumpPose final_pose();
  virtual Ogre::Vector3 final_scale();

  // Implementations of interfaces from TweenerListener
  void onStart(const tween::TweenerParam& param);
  void onStep(const tween::TweenerParam& param);
  void onComplete(const tween::TweenerParam& param);

 signals:
  void onAnimationComplete(VisualPhysicsActorAnimation* animation);

 protected:
  Ogre::Real position_x_;
  Ogre::Real position_y_;
  Ogre::Real position_z_;
  Ogre::Real orientation_w_;
  Ogre::Real orientation_x_;
  Ogre::Real orientation_y_;
  Ogre::Real orientation_z_;
  Ogre::Real final_alpha_modulation_factor_;
  Ogre::Real alpha_modulation_factor_;

  Ogre::Vector3 final_scale_;
  Ogre::Real final_actor_scale_factor_;
  Ogre::Real actor_scale_x_;
  Ogre::Real actor_scale_y_;
  Ogre::Real actor_scale_z_;
  Ogre::Vector3 original_actor_scale_;

  Ogre::Vector3 final_position_;
  Ogre::Quaternion final_orientation_;

  RoomSurface* final_surface_;
  bool previous_physics_enabled_;
  bool pending_deletion_;
  bool is_set_pose_enabled_;
  VisualPhysicsActor* visual_physics_actor_;
  tween::TweenerParam param_;
};

#endif  // BUMPTOP_VISUALPHYSICSACTORANIMATION_H_
