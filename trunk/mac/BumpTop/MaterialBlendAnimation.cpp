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

#include "BumpTop/MaterialBlendAnimation.h"

#include "BumpTop/AnimationManager.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/OgreHelpers.h"
#include "BumpTop/QStringHelpers.h"

MaterialBlendAnimation::MaterialBlendAnimation(QString material_name,
                                               Ogre::Real final_red_factor,
                                               Ogre::Real final_green_factor,
                                               Ogre::Real final_blue_factor,
                                               int64_t length)
: final_red_factor_(final_red_factor),
  final_green_factor_(final_green_factor),
  final_blue_factor_(final_blue_factor) {
  material_name_ = material_name;
  Ogre::MaterialPtr material = Ogre::MaterialPtr(Ogre::MaterialManager::getSingleton().getByName(utf8(material_name_)));
  if (!material.isNull()) {
    Ogre::ColourValue colour_value = material->getTechnique(0)->getPass(0)->getAmbient();
    red_factor_ = colour_value.r;
    green_factor_ = colour_value.g;
    blue_factor_ = colour_value.b;
  }
  param_ = tween::TweenerParam(length, tween::LINEAR, tween::EASE_OUT);
  param_.setListener(this);

  param_.addProperty(&red_factor_, final_red_factor);
  param_.addProperty(&green_factor_, final_green_factor);
  param_.addProperty(&blue_factor_, final_blue_factor);

  // addFadeActorAnimation has to added to animation manager
  AnimationManager::singleton()->addMaterialBlendAnimation(this);
}

QString MaterialBlendAnimation::material_name() {
  return material_name_;
}

tween::TweenerParam* MaterialBlendAnimation::tweener_param() {
  return &param_;
}

void MaterialBlendAnimation::start() {
  tween::Tweener::singleton()->addTween(&param_);
}

void MaterialBlendAnimation::onStart(const tween::TweenerParam& param) {
}

void MaterialBlendAnimation::onStep(const tween::TweenerParam& param) {
  Ogre::MaterialPtr material = Ogre::MaterialPtr(Ogre::MaterialManager::getSingleton().getByName(utf8(material_name_)));
  if (!material.isNull()) {
    material->setAmbient(red_factor_, green_factor_, blue_factor_);
    BumpTopApp::singleton()->markGlobalStateAsChanged();
  }
}

void MaterialBlendAnimation::onComplete(const tween::TweenerParam& param) {
  onStep(param);
  endAnimation();
}

void MaterialBlendAnimation::endAnimation() {
  Ogre::MaterialPtr material = Ogre::MaterialPtr(Ogre::MaterialManager::getSingleton().getByName(utf8(material_name_)));
  if (!material.isNull()) {
    material->setAmbient(final_red_factor_, final_green_factor_, final_blue_factor_);
    BumpTopApp::singleton()->markGlobalStateAsChanged();
  }

  // This deletes this
  AnimationManager::singleton()->MaterialBlendAnimationComplete(this);
}

