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

#ifndef BUMPTOP_ANIMATIONMANAGER_H_
#define BUMPTOP_ANIMATIONMANAGER_H_

#include "BumpTop/Singleton.h"

class AlphaElementAnimation;
class AlphaElement;
class CameraAnimation;
class MaterialBlendAnimation;
class Stopwatch;
class VisualPhysicsActor;
class VisualPhysicsActorAnimation;

class AnimationManager: public QObject {
  Q_OBJECT
  SINGLETON_HEADER(AnimationManager)

 public:
  enum AnimationEndBehavior {
    STOP_AT_CURRENT_STATE,
    MOVE_TO_FINAL_STATE
  };

  virtual ~AnimationManager();

  virtual void addVisualPhysicsActorAnimation(VisualPhysicsActorAnimation* animation);
  virtual void VisualPhysicsActorAnimationComplete(VisualPhysicsActorAnimation* animation);
  virtual void endAnimationsForActor(VisualPhysicsActor* actor, AnimationEndBehavior animation_end_behavior);
  virtual QList<VisualPhysicsActorAnimation*> actor_animations();

  virtual void addAlphaElementAnimation(AlphaElementAnimation* animation);
  virtual void AlphaElementAnimationComplete(AlphaElementAnimation* animation);
  virtual void endAnimationsForAlphaElement(AlphaElement* alpha_element,
                                            AnimationEndBehavior animation_end_behavior = MOVE_TO_FINAL_STATE);

  virtual void addMaterialBlendAnimation(MaterialBlendAnimation* animation);
  virtual void MaterialBlendAnimationComplete(MaterialBlendAnimation* animation);
  virtual void endAnimationsForMaterial(QString material_name);

  virtual void addCameraAnimation(CameraAnimation* animation);
  virtual void CameraAnimationComplete(CameraAnimation* animation);
  virtual void endAnimationsForCamera(Ogre::Camera* camera);

 public slots: // NOLINT
  void renderTick();

 signals: // NOLINT
  void onVisualPhysicsActorAnimationComplete(VisualPhysicsActorAnimation* animation);

 protected:
  explicit AnimationManager();

  QHash<VisualPhysicsActorAnimation*, VisualPhysicsActor*> actor_animations_;
  QHash<AlphaElement*, AlphaElementAnimation*> alpha_element_animations_;
  QHash<QString, MaterialBlendAnimation*> material_animations_;
  QHash<Ogre::Camera*, CameraAnimation*> camera_animations_;
};
#endif  // BUMPTOP_ANIMATIONMANAGER_H_


