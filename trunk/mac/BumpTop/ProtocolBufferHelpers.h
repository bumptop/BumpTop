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

#ifndef BUMPTOP_PROTOCOLBUFFERHELPERS_H_
#define BUMPTOP_PROTOCOLBUFFERHELPERS_H_

class Vector3Buffer;
class QuaternionBuffer;

void Vector3ToBuffer(Ogre::Vector3 v, Vector3Buffer* buffer);
void QuaternionToBuffer(Ogre::Quaternion q, QuaternionBuffer* buffer);
Ogre::Vector3 Vector3BufferToVector3(const Vector3Buffer& buffer);
Ogre::Quaternion QuaternionBufferToQuaternion(const QuaternionBuffer& buffer);
Ogre::Vector3 Vector3BufferToVector3(Vector3Buffer* buffer);
Ogre::Quaternion QuaternionBufferToQuaternion(QuaternionBuffer* buffer);

bool loadBufferFromFile(::google::protobuf::Message* protocol_buffer, QString path);
bool saveBufferToFile(::google::protobuf::Message* protocol_buffer, QString path);

#endif  // BUMPTOP_PROTOCOLBUFFERHELPERS_H_
