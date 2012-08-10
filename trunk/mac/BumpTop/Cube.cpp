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

#include "BumpTop/Cube.h"

#include "BumpTop/VisualPhysicsActorList.h"

Cube::Cube(Ogre::SceneManager *scene_manager, Physics *physics, Ogre::SceneNode *parent_ogre_scene_node)
: Box(scene_manager, physics, parent_ogre_scene_node) {
}

void Cube::set_size(Ogre::Real size) {
  Box::set_size(Ogre::Vector3(size, size, size));
}


#include "BumpTop/moc/moc_Cube.cpp"
