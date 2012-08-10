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

#include "BumpTop/OgreBulletConverter.h"

btVector3 toBt(const Ogre::Vector3& vec) {
  return btVector3(vec.x, vec.y, vec.z);
}

btQuaternion toBt(const Ogre::Quaternion& quaternion) {
  return btQuaternion(quaternion.x, quaternion.y, quaternion.z, quaternion.w);
}

Ogre::Vector3 toOgre(const btVector3& vec) {
  return Ogre::Vector3(vec.x(), vec.y(), vec.z());
}

Ogre::Quaternion toOgre(const btQuaternion& quaternion) {
  return Ogre::Quaternion(quaternion.w(), quaternion.x(), quaternion.y(), quaternion.z());
}

Ogre::Matrix4 toOgre(const btTransform& transform) {
  Ogre::Matrix4 matrix = Ogre::Matrix4(toOgre(transform.getRotation()));
  matrix.setTrans(toOgre(transform.getOrigin()));
  return matrix;
}

