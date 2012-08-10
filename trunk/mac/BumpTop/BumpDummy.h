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

#ifndef BUMPTOP_BUMPDUMMY_H_
#define BUMPTOP_BUMPDUMMY_H_

#include <string>

#include "BumpTop/BumpFlatSquare.h"
#include "BumpTop/QPainterMaterial.h"
#include "BumpTop/VisualActor.h"

const float kDummySizeFactor = 1.25;

class VisualPhysicsActorBuffer;
class BumpDummy: public BumpFlatSquare {
  //Q_OBJECT
 public:
  explicit BumpDummy(Ogre::SceneManager *scene_manager, Physics* physics, Room *room, VisualPhysicsActorId unique_id = 0);
  virtual ~BumpDummy();

  virtual void init();
  virtual bool initFromBuffer(VisualPhysicsActorBuffer* buffer, bool physics_enabled = false);
  virtual void writeToBuffer(VisualPhysicsActorBuffer* buffer);

  virtual VisualPhysicsActorType actor_type();
  virtual BumpTopCommandSet* supported_context_menu_items();
  virtual void set_alpha(Ogre::Real alpha);
  virtual bool nameable();

 protected:
  static BumpTopCommandSet* bump_dummy_context_menu_items_set;
};

#endif  // BUMPTOP_BUMPDUMMY_H_
