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

#include "BumpTop/AnimationManager.h"

#include "CppTweener.h" // NOLINT

#include "BumpTop/BumpTopApp.h"
// TODO: WINTODO
#if defined(OS_WIN)
#else
#include "BumpTop/Stopwatch.h"
#endif


#include "BumpTop/AlphaElementAnimation.h"
#include "BumpTop/CameraAnimation.h"
#include "BumpTop/MaterialBlendAnimation.h"
#include "BumpTop/QPainterOverlay.h"
#include "BumpTop/VisualPhysicsActorAnimation.h"


SINGLETON_IMPLEMENTATION(AnimationManager)

AnimationManager::AnimationManager() {
  assert(QObject::connect(BumpTopApp::singleton(), SIGNAL(onRender()),
                          this, SLOT(renderTick())));

  actor_animations_ = QHash<VisualPhysicsActorAnimation*, VisualPhysicsActor*>();
  alpha_element_animations_ = QHash<AlphaElement*, AlphaElementAnimation*>();
}

AnimationManager::~AnimationManager() {
}

void AnimationManager::addVisualPhysicsActorAnimation(VisualPhysicsActorAnimation* animation) {
  actor_animations_[animation] = animation->visual_physics_actor();
}

void AnimationManager::endAnimationsForActor(VisualPhysicsActor* actor, AnimationEndBehavior animation_end_behavior) {
  for_each(VisualPhysicsActorAnimation* animation, actor_animations_.keys()) {
    if (animation->visual_physics_actor() == actor && !animation->is_pending_deletion()) {
      // When we end an animation before it completes, we have to remove the tween
      // This is normally dealt with by tweener upon animation completion
      tween::Tweener::singleton()->removeTween(animation->tweener_param());
      if (animation_end_behavior == STOP_AT_CURRENT_STATE)
        animation->endAnimation();
      else if (animation_end_behavior == MOVE_TO_FINAL_STATE)
        animation->moveToFinalPositionAndEndAnimation();
    }
  }
}

void AnimationManager::VisualPhysicsActorAnimationComplete(VisualPhysicsActorAnimation* animation) {
  actor_animations_.remove(animation);
  emit onVisualPhysicsActorAnimationComplete(animation);
  delete animation;
}

QList<VisualPhysicsActorAnimation*> AnimationManager::actor_animations() {
  return actor_animations_.keys();
}

void AnimationManager::addAlphaElementAnimation(AlphaElementAnimation* animation) {
  alpha_element_animations_[animation->alpha_element()] = animation;
}

void AnimationManager::endAnimationsForAlphaElement(AlphaElement* alpha_element, AnimationEndBehavior end_bahaviour) {
  if (alpha_element_animations_.contains(alpha_element)) {
    AlphaElementAnimation* animation = alpha_element_animations_[alpha_element];
    // When we end an animation before it completes, we have to remove the tween
    // This is normally dealt with by tweener upon animation completion
    tween::Tweener::singleton()->removeTween(animation->tweener_param());
    if (end_bahaviour == MOVE_TO_FINAL_STATE) {
      animation->endAnimationAndSetFinalAlpha();
    } else {
      animation->endAnimation();
    }
  }
}

void AnimationManager::AlphaElementAnimationComplete(AlphaElementAnimation* animation) {
  alpha_element_animations_.remove(animation->alpha_element());
  delete animation;
}

void AnimationManager::addMaterialBlendAnimation(MaterialBlendAnimation* animation) {
  material_animations_[animation->material_name()] = animation;
}

void AnimationManager::MaterialBlendAnimationComplete(MaterialBlendAnimation* animation) {
  material_animations_.remove(animation->material_name());
  delete animation;
}

void AnimationManager::endAnimationsForMaterial(QString material_name) {
  if (material_animations_.contains(material_name)) {
    MaterialBlendAnimation* animation = material_animations_[material_name];
    // When we end an animation before it completes, we have to remove the tween
    // This is normally dealt with by tweener upon animation completion
    tween::Tweener::singleton()->removeTween(animation->tweener_param());
    animation->endAnimation();
  }
}

void AnimationManager::addCameraAnimation(CameraAnimation* animation) {
  camera_animations_[animation->camera()] = animation;
}

void AnimationManager::CameraAnimationComplete(CameraAnimation* animation) {
  camera_animations_.remove(animation->camera());
}

void AnimationManager::endAnimationsForCamera(Ogre::Camera* camera) {
  if (camera_animations_.contains(camera)) {
    CameraAnimation* animation = camera_animations_[camera];
    // When we end an animation before it completes, we have to remove the tween
    // This is normally dealt with by tweener upon animation completion
    tween::Tweener::singleton()->removeTween(animation->tweener_param());
    animation->endAnimation();
  }
}

void AnimationManager::renderTick() {
// TODO: WINTODO
#if defined(OS_WIN)
#else
  tween::Tweener::singleton()->step(Stopwatch::currentTime());
#endif
}

#include "BumpTop/moc/moc_AnimationManager.cpp"
