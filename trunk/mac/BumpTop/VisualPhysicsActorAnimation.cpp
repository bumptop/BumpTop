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

#include "BumpTop/VisualPhysicsActorAnimation.h"

#include "BumpTop/AnimationManager.h"
#include "BumpTop/BumpBox.h"
#include "BumpTop/OgreHelpers.h"
#include "BumpTop/RoomSurface.h"
#include "BumpTop/VisualPhysicsActor.h"
#include "BumpTop/VisualPhysicsActorList.h"

VisualPhysicsActorAnimation::VisualPhysicsActorAnimation(VisualPhysicsActor* visual_physics_actor,
                                                         int64_t length,
                                                         Ogre::Vector3 final_position,
                                                         Ogre::Quaternion final_orientation,
                                                         RoomSurface* final_surface,
                                                         Ogre::Real final_alpha_modulation_factor,
                                                         Ogre::Real final_actor_size_factor,
                                                         int16_t transition_style)
: final_position_(final_position),
  final_orientation_(final_orientation),
  final_surface_(final_surface),
  pending_deletion_(false),
  alpha_modulation_factor_(1),
  final_alpha_modulation_factor_(final_alpha_modulation_factor),
  final_actor_scale_factor_(final_actor_size_factor) {
  visual_physics_actor_ = visual_physics_actor;
  original_actor_scale_ = visual_physics_actor_->scale();

  is_set_pose_enabled_ = (visual_physics_actor_->position() != final_position_) ||
                             (visual_physics_actor_->orientation() != final_orientation);

  if (is_set_pose_enabled_) {
    position_x_ = visual_physics_actor_->position().x;
    position_y_ = visual_physics_actor_->position().y;
    position_z_ = visual_physics_actor_->position().z;
    orientation_w_ = visual_physics_actor_->orientation().w;
    orientation_x_ = visual_physics_actor_->orientation().x;
    orientation_y_ = visual_physics_actor_->orientation().y;
    orientation_z_ = visual_physics_actor_->orientation().z;
  }

  if (final_actor_scale_factor_ != 1) {
    actor_scale_x_ = visual_physics_actor_->scale().x;
    actor_scale_y_ = visual_physics_actor_->scale().y;
    actor_scale_z_ = visual_physics_actor_->scale().z;
  }

  param_ = tween::TweenerParam(length, transition_style, tween::EASE_OUT);

  param_.setListener(this);

  if (is_set_pose_enabled_) {
    param_.addProperty(&position_x_, final_position.x);
    param_.addProperty(&position_y_, final_position.y);
    param_.addProperty(&position_z_, final_position.z);
    param_.addProperty(&orientation_w_, final_orientation.w);
    param_.addProperty(&orientation_x_, final_orientation.x);
    param_.addProperty(&orientation_y_, final_orientation.y);
    param_.addProperty(&orientation_z_, final_orientation.z);
  }

  param_.addProperty(&alpha_modulation_factor_, final_alpha_modulation_factor_);

  if (final_actor_scale_factor_ != 1) {
    param_.addProperty(&actor_scale_x_, final_actor_scale_factor_*actor_scale_x_);
    param_.addProperty(&actor_scale_y_, final_actor_scale_factor_*actor_scale_y_);
    param_.addProperty(&actor_scale_z_, final_actor_scale_factor_*actor_scale_z_);
  }
  final_scale_ = final_actor_scale_factor_*visual_physics_actor_->scale();

  AnimationManager::singleton()->addVisualPhysicsActorAnimation(this);

  previous_physics_enabled_ = visual_physics_actor_->physics_enabled();
}

VisualPhysicsActorAnimation::~VisualPhysicsActorAnimation() {
}

tween::TweenerParam* VisualPhysicsActorAnimation::tweener_param() {
  return &param_;
}

VisualPhysicsActor* VisualPhysicsActorAnimation::visual_physics_actor() {
  return visual_physics_actor_;
}

bool VisualPhysicsActorAnimation::is_pending_deletion() {
  return pending_deletion_;
}

void VisualPhysicsActorAnimation::start() {
  tween::Tweener::singleton()->addTween(&param_);
}

void VisualPhysicsActorAnimation::onStart(const tween::TweenerParam& param) {
  if (is_set_pose_enabled_) {
    visual_physics_actor_->set_physics_enabled(false);
  }
}

void VisualPhysicsActorAnimation::onStep(const tween::TweenerParam& param) {
  if (is_set_pose_enabled_) {
    visual_physics_actor_->set_position(Ogre::Vector3(position_x_, position_y_, position_z_));
    visual_physics_actor_->set_orientation(Ogre::Quaternion(orientation_w_, orientation_x_,
                                                          orientation_y_, orientation_z_));
  }

  visual_physics_actor_->set_alpha(alpha_modulation_factor_);

  if (final_actor_scale_factor_ != 1) {
    visual_physics_actor_->setSizeForGrowOrShrink(Ogre::Vector3(actor_scale_x_,
                                                                actor_scale_y_,
                                                                actor_scale_z_));
  }
}

void VisualPhysicsActorAnimation::onComplete(const tween::TweenerParam& param) {
  // Step to the final position
  onStep(param);
  endAnimation();
}

void VisualPhysicsActorAnimation::moveToFinalPositionAndEndAnimation() {
  if (is_set_pose_enabled_) {
    visual_physics_actor_->set_position(final_position_);
    visual_physics_actor_->set_orientation(final_orientation_);
  }

  visual_physics_actor_->set_alpha(final_alpha_modulation_factor_);

  if (!is_pending_deletion()) {
    endAnimation();
  }
}

BumpPose VisualPhysicsActorAnimation::final_pose() {
  return BumpPose(final_position_, final_orientation_);
}

Ogre::Vector3 VisualPhysicsActorAnimation::final_scale() {
  return final_scale_;
}

void VisualPhysicsActorAnimation::endAnimation() {
  pending_deletion_ = true;

  if (is_set_pose_enabled_ || final_actor_scale_factor_ != 1) {
    if (final_surface_ != NULL) {
      if (final_surface_->is_pinnable_receiver()) {
        visual_physics_actor_->pinToSurface(final_surface_);
      } else {
        visual_physics_actor_->setPhysicsConstraintsForSurface(final_surface_);
      }
      visual_physics_actor_->set_room_surface(final_surface_);
    }
    visual_physics_actor_->set_physics_enabled(previous_physics_enabled_);
  }

  if (final_actor_scale_factor_ != 1) {
    visual_physics_actor_->setSizeForGrowOrShrink(final_actor_scale_factor_*original_actor_scale_);
  }

  emit onAnimationComplete(this);

  // This deletes this
  AnimationManager::singleton()->VisualPhysicsActorAnimationComplete(this);
}

#include "BumpTop/moc/moc_VisualPhysicsActorAnimation.cpp"
