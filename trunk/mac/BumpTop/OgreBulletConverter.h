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

#ifndef BUMPTOP_OGREBULLETCONVERTER_H_
#define BUMPTOP_OGREBULLETCONVERTER_H_

#include <Ogre.h>

#include <LinearMath/btVector3.h>
#include <LinearMath/btQuaternion.h>

btVector3 toBt(const Ogre::Vector3& vec);
btQuaternion toBt(const Ogre::Quaternion& quaternion);
Ogre::Vector3 toOgre(const btVector3& vec);
Ogre::Quaternion toOgre(const btQuaternion& quaternion);
Ogre::Matrix4 toOgre(const btTransform& transform);


#endif  // BUMPTOP_OGREBULLETCONVERTER_H_
