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

#ifndef BUMPTOP_HIGHLIGHTACTOR_H_
#define BUMPTOP_HIGHLIGHTACTOR_H_

#include "BumpTop/VisualActor.h"

class MaterialLoader;

class HighlightActor : public VisualActor {
  Q_OBJECT

 public:
  explicit HighlightActor(Ogre::SceneManager *scene_manager, Ogre::SceneNode *parent_ogre_scene_node,
                          Ogre::Real height_of_parent);
  virtual void set_height_of_parent(Ogre::Real height_of_parent);
  virtual void set_scale(Ogre::Vector3 scale);

 public slots:  // NOLINT
  void deleteMaterialLoader(MaterialLoader *material_loader);
};
#endif  // BUMPTOP_HIGHLIGHTACTOR_H_
