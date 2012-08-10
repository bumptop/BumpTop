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

#include "BumpTop/ProtocolBufferHelpers.h"

#include <string>

#include "BumpTop/protoc/AllMessages.pb.h"
#include "BumpTop/QStringHelpers.h"

void Vector3ToBuffer(Ogre::Vector3 v, Vector3Buffer* buffer) {
  buffer->set_x(v.x);
  buffer->set_y(v.y);
  buffer->set_z(v.z);
}

void QuaternionToBuffer(Ogre::Quaternion q, QuaternionBuffer* buffer) {
  buffer->set_w(q.w);
  buffer->set_x(q.x);
  buffer->set_y(q.y);
  buffer->set_z(q.z);
}

Ogre::Vector3 Vector3BufferToVector3(const Vector3Buffer& buffer) {
  return Ogre::Vector3(buffer.x(), buffer.y(), buffer.z());
}
Ogre::Quaternion QuaternionBufferToQuaternion(const QuaternionBuffer& buffer) {
  return Ogre::Quaternion(buffer.w(), buffer.x(), buffer.y(), buffer.z());
}

Ogre::Vector3 Vector3BufferToVector3(Vector3Buffer* buffer) {
  return Ogre::Vector3(buffer->x(), buffer->y(), buffer->z());
}

Ogre::Quaternion QuaternionBufferToQuaternion(QuaternionBuffer* buffer) {
  return Ogre::Quaternion(buffer->w(), buffer->x(), buffer->y(), buffer->z());
}

bool loadBufferFromFile(::google::protobuf::Message* protocol_buffer, QString path) {
  std::fstream file;
  std::string str = utf8(path);
  file.open(str.c_str(), std::ios::in | std::ios::binary);

  // Check if the file opening failed, and if so, the file doesn't exist, so return false
  if (file.fail()) {
    return false;
  }

  bool flag = protocol_buffer->ParseFromIstream(&file);
  file.close();

  // Check if the file was not successfully parsed, and if so, return false
  return flag;
}

bool saveBufferToFile(::google::protobuf::Message* protocol_buffer, QString path) {
  std::fstream file;
  std::string str = utf8(path);
  file.open(str.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
  // Check if the file opening failed, and if so, the file doesn't exist, so return false
  if (file.fail()) {
    return false;
  }
  bool flag = protocol_buffer->SerializeToOstream(&file);
  file.close();
  return flag;
}
