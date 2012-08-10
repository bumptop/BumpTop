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

#include "BumpTop/AlphaElementAnimation.h"

#include "BumpTop/AnimationManager.h"
#include "BumpTop/OgreHelpers.h"

AlphaElement::~AlphaElement() {
}


Ogre::Real AlphaElement::alpha() {
  return alpha_;
}

void AlphaElement::setAlpha(Ogre::Real alpha) {
  alpha_ = alpha;
}

AlphaElementAnimation::AlphaElementAnimation(AlphaElement* alpha_element,
                                             Ogre::Real final_alpha,
                                             int64_t length)
: final_alpha_(final_alpha),
  alpha_element_(alpha_element) {
  alpha_ = alpha_element->alpha();

  param_ = tween::TweenerParam(length, tween::CUBIC, tween::EASE_IN_OUT);
  param_.setListener(this);

  param_.addProperty(&alpha_, final_alpha_);

  // addFadeActorAnimation has to added to animation manager
  AnimationManager::singleton()->addAlphaElementAnimation(this);
}

AlphaElementAnimation::AlphaElementAnimation() {
}

AlphaElementAnimation::~AlphaElementAnimation() {
}

tween::TweenerParam* AlphaElementAnimation::tweener_param() {
  return &param_;
}

AlphaElement* AlphaElementAnimation::alpha_element() {
  return alpha_element_;
}

void AlphaElementAnimation::start() {
  tween::Tweener::singleton()->addTween(&param_);
}

void AlphaElementAnimation::onStart(const tween::TweenerParam& param) {
}

void AlphaElementAnimation::onStep(const tween::TweenerParam& param) {
  alpha_element_->setAlpha(alpha_);
}

void AlphaElementAnimation::onComplete(const tween::TweenerParam& param) {
  onStep(param);
  endAnimation();
}

void AlphaElementAnimation::endAnimationAndSetFinalAlpha() {
  alpha_element_->setAlpha(final_alpha_);
  endAnimation();
}

void AlphaElementAnimation::endAnimation() {
  emit onAnimationComplete();

  // This deletes this
  AnimationManager::singleton()->AlphaElementAnimationComplete(this);
}

#include "BumpTop/moc/moc_AlphaElementAnimation.cpp"
