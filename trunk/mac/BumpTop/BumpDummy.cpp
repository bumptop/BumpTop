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

#include "BumpTop/BumpDummy.h"

#include <string>

#include "BumpTop/AppSettings.h"
#include "BumpTop/Authorization.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/FileItem.h"
#include "BumpTop/FileManager.h"
#include "BumpTop/Math.h"
#include "BumpTop/MouseEventManager.h"
#include "BumpTop/ProtocolBufferHelpers.h"
#include "BumpTop/QStringHelpers.h"
#include "BumpTop/RoomSurface.h"
#include "BumpTop/Shape.h"
#include "BumpTop/VisualPhysicsActorAnimation.h"

BumpTopCommandSet* BumpDummy::bump_dummy_context_menu_items_set = MakeQSet(2,  // count, must keep this updated
                                                              Grow::singleton(),
                                                              Shrink::singleton());

BumpDummy::BumpDummy(Ogre::SceneManager *scene_manager, Physics* physics, Room *room, VisualPhysicsActorId unique_id)
: BumpFlatSquare(scene_manager, physics, room, unique_id){
}

BumpDummy::~BumpDummy() {
}

BumpTopCommandSet* BumpDummy::supported_context_menu_items() {
  return bump_dummy_context_menu_items_set;
}

void BumpDummy::init() {
  BumpBox::init();

  set_material_name(AppSettings::singleton()->global_material_name(DEFAULT_ICON));

  visual_actor_->set_visible(false);
}

bool BumpDummy::initFromBuffer(VisualPhysicsActorBuffer* buffer, bool physics_enabled) {
  return false;
}

void BumpDummy::writeToBuffer(VisualPhysicsActorBuffer* buffer) {
  buffer->set_actor_type(actor_type());
  Vector3ToBuffer(position(), buffer->mutable_position());
  QuaternionToBuffer(orientation(), buffer->mutable_orientation());
  Vector3ToBuffer(scale(), buffer->mutable_size());
}

void BumpDummy::set_alpha(Ogre::Real alpha) {
  BumpFlatSquare::set_alpha(alpha);
}

bool BumpDummy::nameable() {
  return false;
}

VisualPhysicsActorType BumpDummy::actor_type() {
  return BUMP_DUMMY;
}
