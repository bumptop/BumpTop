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

#include "BumpTop/CameraAnimation.h"

#include "BumpTop/AnimationManager.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/BumpTopScene.h"
#include "BumpTop/OgreHelpers.h"
#include "BumpTop/Room.h"
#include "BumpTop/RoomSurface.h"

CameraAnimation::CameraAnimation(Ogre::Camera* camera, int64_t length,
                                 Ogre::Vector3 final_position, Ogre::Quaternion final_orientation,
                                 Ogre::Real final_near_clipping_distance)
: final_position_(final_position),
  final_orientation_(final_orientation),
  camera_(camera) {
  if (final_near_clipping_distance == -1) {
    final_near_clipping_distance_ = camera->getNearClipDistance();
  } else {
    final_near_clipping_distance_ = final_near_clipping_distance;
  }

  Ogre::Vector3 current_position = camera_->getPosition();
  Ogre::Quaternion current_orientation = camera_->getOrientation();

  position_x_ = current_position.x;
  position_y_ = current_position.y;
  position_z_ = current_position.z;
  orientation_w_ = current_orientation.w;
  orientation_x_ = current_orientation.x;
  orientation_y_ = current_orientation.y;
  orientation_z_ = current_orientation.z;
  near_clipping_distance_ = camera->getNearClipDistance();

  param_ = tween::TweenerParam(length, tween::CUBIC, tween::EASE_IN_OUT);
  param_.setListener(this);

  param_.addProperty(&position_x_, final_position.x);
  param_.addProperty(&position_y_, final_position.y);
  param_.addProperty(&position_z_, final_position.z);
  param_.addProperty(&orientation_w_, final_orientation_.w);
  param_.addProperty(&orientation_x_, final_orientation_.x);
  param_.addProperty(&orientation_y_, final_orientation_.y);
  param_.addProperty(&orientation_z_, final_orientation_.z);
  param_.addProperty(&near_clipping_distance_, final_near_clipping_distance_);

  AnimationManager::singleton()->addCameraAnimation(this);
}

tween::TweenerParam* CameraAnimation::tweener_param() {
  return &param_;
}

Ogre::Camera* CameraAnimation::camera() {
  return camera_;
}

void CameraAnimation::start() {
  tween::Tweener::singleton()->addTween(&param_);
}

void CameraAnimation::onStart(const tween::TweenerParam& param) {
}

void CameraAnimation::onStep(const tween::TweenerParam& param) {
  camera_->setPosition(Ogre::Vector3(position_x_, position_y_, position_z_));
  camera_->setOrientation(Ogre::Quaternion(orientation_w_, orientation_x_,
                                           orientation_y_, orientation_z_));
  camera_->setNearClipDistance(near_clipping_distance_);
  BumpTopApp::singleton()->scene()->room()->updateAllLabelPositions();
  BumpTopApp::singleton()->markGlobalStateAsChanged();
}

void CameraAnimation::onComplete(const tween::TweenerParam& param) {
  // Step to the final position
  onStep(param);
  endAnimation();
}

void CameraAnimation::moveToFinalStateAndEndAnimation() {
  camera_->setPosition(final_position_);
  camera_->setOrientation(final_orientation_);

  BumpTopApp::singleton()->markGlobalStateAsChanged();
  endAnimation();
}

void CameraAnimation::endAnimation() {
  emit onAnimationComplete();

  // This deletes this
  AnimationManager::singleton()->CameraAnimationComplete(this);
}

#include "BumpTop/moc/moc_CameraAnimation.cpp"
