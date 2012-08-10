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

#include "BumpTop/Shape.h"

#include <Ogre.h>
#include <string>

#include "BumpTop/Authorization.h"

SINGLETON_IMPLEMENTATION(Shape)

Shape::Shape() {
  Ogre::ManualObject flat_square = Ogre::ManualObject("flat_square");

  flat_square.begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_TRIANGLE_LIST);
  flat_square.position(-50.0, 0, -50.0);
  flat_square.textureCoord(0, 0);
  flat_square.position(50.0, 0, -50.0);
  flat_square.textureCoord(1, 0);
  flat_square.position(50.0, 0, 50.0);
  flat_square.textureCoord(1, 1);
  flat_square.position(-50.0, 0, 50.0);
  flat_square.textureCoord(0, 1);

  flat_square.index(0);
  flat_square.index(1);
  flat_square.index(2);
  flat_square.index(2);
  flat_square.index(3);
  flat_square.index(0);
  flat_square.index(0);
  flat_square.index(3);
  flat_square.index(2);
  flat_square.index(2);
  flat_square.index(1);
  flat_square.index(0);

  flat_square.end();

  flat_square.convertToMesh("flat_square", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
}

std::string Shape::flat_square() {
  return "flat_square";
}
