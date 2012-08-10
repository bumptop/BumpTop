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

#ifndef BUMPTOP_OGREHELPERS_H_
#define BUMPTOP_OGREHELPERS_H_

#include <string>

struct BumpPose {
  BumpPose();
  BumpPose(Ogre::Vector3 pos, Ogre::Quaternion orient);
  bool isNull();
  Ogre::Vector3 position;
  Ogre::Quaternion orientation;
  bool approximatelyEquals(BumpPose pose);
};

Ogre::Vector2 worldPositionToScreenPosition(Ogre::Vector3 world_position);
Ogre::Vector2 worldPositionToNormalizedScreenPosition(Ogre::Vector3 world_position);
Ogre::Vector2 screenPositionToNormalizedScreenPosition(Ogre::Vector2 screen_position);
Ogre::Vector2 normalizedScreenPositionToScreenPosition(Ogre::Vector2 screen_position);
Ogre::AxisAlignedBox worldBoundingBoxToScreenBoundingBox(Ogre::AxisAlignedBox world_bounding_box);
Ogre::AxisAlignedBox getUninionOfBoundingBoxes(Ogre::AxisAlignedBox box1, Ogre::AxisAlignedBox box2);
Ogre::AxisAlignedBox addPointToBoundingBox(Ogre::AxisAlignedBox box, Ogre::Vector3 point);


bool yLessThanVec2(const Ogre::Vector2 &v1, const Ogre::Vector2 &v2);
bool yLessThanVec3(const Ogre::Vector3 &v1, const Ogre::Vector3 &v2);
bool xLessThan(const Ogre::Vector2 &v1, const Ogre::Vector2 &v2);

Ogre::String addressToString(void *pointer);
void print(QString string);
void printLine(QString string);
void print(Ogre::Vector3 vector);
void printLine(Ogre::Vector3 vector);
void print(Ogre::Vector2 vector);
void printLine(Ogre::Vector2 vector);
void print(Ogre::Real real);
void printLine(Ogre::Real real);
void print(int integer);
void printLine(int integer);
void printAsHex(int integer);
void printLineAsHex(int integer);
void print(Ogre::AxisAlignedBox box);
void printLine(Ogre::AxisAlignedBox box);
void printLine();

#endif  // BUMPTOP_OGREHELPERS_H_
