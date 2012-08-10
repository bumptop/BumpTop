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

#include "BumpTop/Box.h"

#include <string>
#include <vector>

#include "BumpTop/OgreBulletConverter.h"
#include "BumpTop/PhysicsBoxActor.h"
#include "BumpTop/VisualActor.h"

Box::Box(Ogre::SceneManager *scene_manager, Physics *physics, Ogre::SceneNode *parent_ogre_scene_node)
: VisualPhysicsActor(scene_manager, physics, parent_ogre_scene_node) {
}

Box::~Box() {
}

// Implementing abstract members of VisualPhysicsActor
std::string Box::meshName() {
  return "Prefab_Cube";
}

Ogre::Vector3 Box::absoluteMeshSizeDividedBy100() {
  return Ogre::Vector3(1.0, 1.0, 1.0);
}

btVector3 Box::physicsSize() {
  return btVector3(1.0, 1.0, 1.0);
}

void Box::makePhysicsActor(bool physics_enabled) {
  physics_actor_ = new PhysicsBoxActor(physics_, mass(), toOgre(physicsSize()), physics_enabled);
}

btScalar Box::mass() {
  return 1.0;
}

VisualPhysicsActorType Box::actor_type() {
  return BOX;
}
// end: Implementing abstract members of VisualPhysicsActor



#include "BumpTop/moc/moc_Box.cpp"
