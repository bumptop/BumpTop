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

#include "BumpTop/BumpFlatSquare.h"

#include <string>

#include "BumpTop/Shape.h"

BumpFlatSquare::BumpFlatSquare(Ogre::SceneManager *scene_manager, Physics* physics,
                 Room *room, VisualPhysicsActorId unique_id)
: BumpBox(scene_manager, physics, room, unique_id) {
}

BumpFlatSquare::~BumpFlatSquare() {
}

bool BumpFlatSquare::pinnable() {
  return true;
}

BumpBox* BumpFlatSquare::constructCopyOfMyType() {
  return new BumpFlatSquare(scene_manager_, physics_, room_, 0);
}

std::string BumpFlatSquare::meshName() {
  return Shape::singleton()->flat_square();
  // return "Prefab_Cube";
}

Ogre::Vector3 BumpFlatSquare::absoluteMeshSizeDividedBy100() {
  return Ogre::Vector3(1.0, 0, 1.0);
  // return Ogre::Vector3(1.0, 1.0, 1.0);
}

btVector3 BumpFlatSquare::physicsSize() {
  return btVector3(1.0, 0.125, 1.0);
  // return btVector3(1.0, 1.0, 1.0);
}

#include "moc/moc_BumpFlatSquare.cpp"

