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

#include "BumpTop/HighlightActor.h"

#include "BumpTop/AppSettings.h"
#include "BumpTop/MaterialLoader.h"
#include "BumpTop/OgreHelpers.h"
#include "BumpTop/QStringHelpers.h"
#include "BumpTop/Shape.h"

HighlightActor::HighlightActor(Ogre::SceneManager *scene_manager, Ogre::SceneNode *parent_ogre_scene_node,
                               Ogre::Real height_of_parent)
: VisualActor(scene_manager, parent_ogre_scene_node, Shape::singleton()->flat_square(), false) {
  init();
  set_material_name(AppSettings::singleton()->global_material_name(HIGHLIGHT));
  set_inherit_orientation(true);
  set_position(Ogre::Vector3(0, -height_of_parent/2.0, 0));
  ogre_scene_node()->setScale(Ogre::Vector3(1.1));
  set_visible(false);
}

void HighlightActor::set_height_of_parent(Ogre::Real height_of_parent) {
  set_position(Ogre::Vector3(0, -height_of_parent/2.0, 0));
}

void HighlightActor::set_scale(Ogre::Vector3 scale) {
  ogre_scene_node()->setScale(1.1*scale);
}

void HighlightActor::deleteMaterialLoader(MaterialLoader *material_loader) {
  delete material_loader;
}

#include "moc/moc_HighlightActor.cpp"
