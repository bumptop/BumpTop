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

#ifndef BUMPTOP_MATERIALBLENDANIMATION_H_
#define BUMPTOP_MATERIALBLENDANIMATION_H_

#if defined(OS_WIN)
#include <boost/cstdint.hpp>
using boost::int64_t;
#endif
#include "CppTweener.h"

class MaterialBlendAnimation : public tween::TweenerListener {
 public:
  explicit MaterialBlendAnimation(QString material_name,
                                  Ogre::Real final_red_factor,
                                  Ogre::Real final_green_factor,
                                  Ogre::Real final_blue_factor,
                                  int64_t length);

  virtual QString material_name();
  virtual tween::TweenerParam* tweener_param();

  virtual void start();
  virtual void endAnimation();

  // Implementations of interfaces from TweenerListener
  void onStart(const tween::TweenerParam& param);
  void onStep(const tween::TweenerParam& param);
  void onComplete(const tween::TweenerParam& param);

 protected:
  Ogre::Real red_factor_;
  Ogre::Real green_factor_;
  Ogre::Real blue_factor_;
  Ogre::Real final_red_factor_;
  Ogre::Real final_green_factor_;
  Ogre::Real final_blue_factor_;

  QString material_name_;
  tween::TweenerParam param_;
};

#endif  // BUMPTOP_MATERIALBLENDANIMATION_H_
