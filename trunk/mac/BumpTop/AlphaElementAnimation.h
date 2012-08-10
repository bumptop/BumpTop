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

#ifndef BUMPTOP_ALPHAELEMENTANIMATION_H_
#define BUMPTOP_ALPHAELEMENTANIMATION_H_

#if defined(OS_WIN)
#include <boost/cstdint.hpp>
using boost::int64_t;
#endif
#include "CppTweener.h"

class AlphaElement {
 public:
  virtual ~AlphaElement();
  virtual void setAlpha(Ogre::Real alpha);
  virtual Ogre::Real alpha();
 protected:
  Ogre::Real alpha_;
};

class AlphaElementAnimation : public QObject, public tween::TweenerListener {
  Q_OBJECT

 public:
  explicit AlphaElementAnimation(AlphaElement* element,
                                 Ogre::Real final_alpha,
                                 int64_t length);

  AlphaElementAnimation();
  ~AlphaElementAnimation();

  virtual tween::TweenerParam* tweener_param();
  virtual AlphaElement* alpha_element();

  virtual void start();
  virtual void endAnimation();
  virtual void endAnimationAndSetFinalAlpha();

  // Implementations of interfaces from TweenerListener
  void onStart(const tween::TweenerParam& param);
  void onStep(const tween::TweenerParam& param);
  void onComplete(const tween::TweenerParam& param);

 signals:  // NOLINT
  void onAnimationComplete();

 protected:
  Ogre::Real alpha_;
  Ogre::Real final_alpha_;

  AlphaElement* alpha_element_;
  tween::TweenerParam param_;
};

#endif  // BUMPTOP_ALPHAELEMENTANIMATION_H_
