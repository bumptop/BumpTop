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

#ifndef BUMPTOP_CAMERAANIMATION_H_
#define BUMPTOP_CAMERAANIMATION_H_

#if defined(OS_WIN)
#include <boost/cstdint.hpp>
using boost::int64_t;
#endif
#include "CppTweener.h"

struct BumpPose;
class RoomSurface;
class CameraAnimation : public QObject, public tween::TweenerListener {
  Q_OBJECT

 public:
  explicit CameraAnimation(Ogre::Camera* camera,
                           int64_t length,
                           Ogre::Vector3 final_position,
                           Ogre::Quaternion final_orientation,
                           Ogre::Real final_near_clipping_distance = -1);

  virtual tween::TweenerParam* tweener_param();
  virtual Ogre::Camera* camera();

  virtual void start();
  virtual void moveToFinalStateAndEndAnimation();
  virtual void endAnimation();

  // Implementations of interfaces from TweenerListener
  void onStart(const tween::TweenerParam& param);
  void onStep(const tween::TweenerParam& param);
  void onComplete(const tween::TweenerParam& param);

 signals:
  void onAnimationComplete();

 protected:
  Ogre::Real position_x_;
  Ogre::Real position_y_;
  Ogre::Real position_z_;
  Ogre::Real orientation_w_;
  Ogre::Real orientation_x_;
  Ogre::Real orientation_y_;
  Ogre::Real orientation_z_;
  Ogre::Real near_clipping_distance_;

  Ogre::Vector3 final_position_;
  Ogre::Quaternion final_orientation_;
  Ogre::Real final_near_clipping_distance_;

  Ogre::Camera* camera_;
  tween::TweenerParam param_;
};

#endif  // BUMPTOP_CAMERAANIMATION_H_
