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

#include "BumpTop/Math.h"

Ogre::Vector3 Math::componentwise_min(const Ogre::Vector3 &value_1, const Ogre::Vector3 &value_2) {
  return Ogre::Vector3(std::min(value_1.x, value_2.x), std::min(value_1.y, value_2.y), std::min(value_1.z, value_2.z));
}

Ogre::Vector3 Math::componentwise_max(const Ogre::Vector3 &value_1, const Ogre::Vector3 &value_2) {
  return Ogre::Vector3(std::max(value_1.x, value_2.x), std::max(value_1.y, value_2.y), std::max(value_1.z, value_2.z));
}

uint32_t Math::nearestPowerOf2(uint32_t num) {
  // http://jeffreystedfast.blogspot.com/2008/06/calculating-nearest-power-of-2.html
  uint32_t n = num > 0 ? num - 1 : 0;

  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  n++;

  return n;
}
