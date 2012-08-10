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

#include "BumpTop/OgreHelpers.h"

#include <string>

#include "BumpTop/BumpTopApp.h"
#include "BumpTop/QStringHelpers.h"

Ogre::Vector2 worldPositionToScreenPosition(Ogre::Vector3 world_position) {
  Ogre::Vector2 normalized_screen_position = worldPositionToNormalizedScreenPosition(world_position);
  return normalizedScreenPositionToScreenPosition(normalized_screen_position);
}

Ogre::Vector2 screenPositionToNormalizedScreenPosition(Ogre::Vector2 screen_position) {
  Ogre::Vector2 window_size = BumpTopApp::singleton()->window_size();
  return Ogre::Vector2(2*screen_position.x / window_size.x - 1,
                       -(2*screen_position.y / window_size.y - 1));
}

Ogre::Vector2 normalizedScreenPositionToScreenPosition(Ogre::Vector2 normalized_screen_position) {
  Ogre::Vector2 window_size = BumpTopApp::singleton()->window_size();
  Ogre::Vector2 ratio_screen_position(1 - (0.5 - normalized_screen_position.x/2),
                                      1 - (0.5 + normalized_screen_position.y/2));
  Ogre::Vector2 pixel_screen_position(ratio_screen_position.x*window_size.x, ratio_screen_position.y*window_size.y);
  return pixel_screen_position;
}

Ogre::Vector2 worldPositionToNormalizedScreenPosition(Ogre::Vector3 world_position) {
  BumpTopApp* bumptop = BumpTopApp::singleton();

  Ogre::Vector3 position = world_position;
  position = bumptop->camera()->getViewMatrix() * position;
  position = bumptop->camera()->getProjectionMatrix() * position;

  return Ogre::Vector2(position.x, position.y);
}

Ogre::AxisAlignedBox worldBoundingBoxToScreenBoundingBox(Ogre::AxisAlignedBox world_bounding_box) {
  const Ogre::Vector3* corners = world_bounding_box.getAllCorners();
  bool first_pass = true;
  Ogre::Real min_x, min_y, max_x, max_y;
  for (int i = 0; i < 8; i++) {
    Ogre::Vector2 corner = worldPositionToScreenPosition(corners[i]);
    if (first_pass || corner.x < min_x)
      min_x = corner.x;
    if (first_pass || corner.y < min_y)
      min_y = corner.y;
    if (first_pass || corner.x > max_x)
      max_x = corner.x;
    if (first_pass || corner.y > max_y)
      max_y = corner.y;
    first_pass = false;
  }
  return Ogre::AxisAlignedBox(min_x, min_y, -1, max_x, max_y, 1);
}

Ogre::AxisAlignedBox getUninionOfBoundingBoxes(Ogre::AxisAlignedBox box1, Ogre::AxisAlignedBox box2) {
  if (box2.getMaximum().x > box1.getMaximum().x)
    box1.setMaximumX(box2.getMaximum().x);
  if (box2.getMaximum().y > box1.getMaximum().y)
    box1.setMaximumY(box2.getMaximum().y);
  if (box2.getMaximum().z > box1.getMaximum().z)
    box1.setMaximumZ(box2.getMaximum().z);

  if (box2.getMinimum().x < box1.getMinimum().x)
    box1.setMinimumX(box2.getMinimum().x);
  if (box2.getMinimum().y < box1.getMinimum().y)
    box1.setMinimumY(box2.getMinimum().y);
  if (box2.getMinimum().z < box1.getMinimum().z)
    box1.setMinimumZ(box2.getMinimum().z);

  return box1;
}

Ogre::AxisAlignedBox addPointToBoundingBox(Ogre::AxisAlignedBox box, Ogre::Vector3 point) {
  if (!box.contains(point)) {
    if (point.x < (box.getMinimum()).x) {
      box.setMinimumX(point.x);
    } else if (point.x > (box.getMaximum()).x) {
      box.setMaximumX(point.x);
    }
    if (point.y < (box.getMinimum()).y) {
      box.setMinimumY(point.y);
    } else if (point.y > (box.getMaximum()).y) {
      box.setMaximumY(point.y);
    }
    if (point.z < (box.getMinimum()).z) {
      box.setMinimumZ(point.z);
    } else if (point.z > (box.getMaximum()).z) {
      box.setMaximumZ(point.z);
    }
  }
  return box;
}

Ogre::String addressToString(void *pointer) {
  return Ogre::StringConverter::toString((uint)pointer);
}

BumpPose::BumpPose()
: position(Ogre::Vector3(0, 0, 0)),
  orientation(Ogre::Quaternion(1, 0, 0, 0)) {
}

BumpPose::BumpPose(Ogre::Vector3 pos, Ogre::Quaternion orient)
: position(pos),
  orientation(orient) {
}

bool BumpPose::isNull() {
  return position == Ogre::Vector3::ZERO && orientation == Ogre::Quaternion::IDENTITY;
}

bool BumpPose::approximatelyEquals(BumpPose pose) {
  return (position.positionEquals(pose.position, 10.0) && orientation.equals(pose.orientation, Ogre::Radian(0.2)));
}

bool yLessThanVec2(const Ogre::Vector2 &v1, const Ogre::Vector2 &v2) {
  return (v1.y < v2.y);
}

bool yLessThanVec3(const Ogre::Vector3 &v1, const Ogre::Vector3 &v2) {
  return (v1.y < v2.y);
}

bool xLessThan(const Ogre::Vector2 &v1, const Ogre::Vector2 &v2) {
  return (v1.x < v2.x);
}

void print(QString string) {
  std::cout << string;
}

void printLine(QString string) {
  print(string), printLine();
}

void print(Ogre::Vector3 vector) {
  std::cout << "(" << vector.x << " " << vector.y << " " << vector.z << ")";
}

void printLine(Ogre::Vector3 vector) {
  print(vector), printLine();
}

void print(Ogre::Vector2 vector) {
  std::cout << "(" << vector.x << " " << vector.y << ")";
}

void printLine(Ogre::Vector2 vector) {
  print(vector), printLine();
}

void print(Ogre::Real real) {
  std::cout << real;
}

void printLine(Ogre::Real real) {
  print(real), printLine();
}

void print(int integer) {
  std::cout << integer;
}

void printLine(int integer) {
  print(integer), printLine();
}

void printAsHex(int integer) {
  std::cout << reinterpret_cast<void*>(integer);
}

void printLineAsHex(int integer) {
  printAsHex(integer), printLine();
}

void print(Ogre::AxisAlignedBox box) {
  print("AABB( "), print(box.getMinimum()), print(" "), print(box.getMaximum()), print(")");
}

void printLine(Ogre::AxisAlignedBox box) {
  print(box), printLine();
}

void printLine() {
  std::cout << std::endl;
}

