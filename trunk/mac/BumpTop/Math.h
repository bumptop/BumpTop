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

#ifndef BUMPTOP_MATH_H_
#define BUMPTOP_MATH_H_

#if defined(OS_WIN)
#include <boost/cstdint.hpp>
using boost::uint64_t;
using boost::uint32_t;
#endif
#include <Ogre.h>
#include <vector>

class Math {
 public:
  static Ogre::Vector3 componentwise_min(const Ogre::Vector3 &value_1, const Ogre::Vector3 &value_2);
  static Ogre::Vector3 componentwise_max(const Ogre::Vector3 &value_1, const Ogre::Vector3 &value_2);
  static uint32_t nearestPowerOf2(uint32_t num);
};

#endif  // BUMPTOP_MATH_H_
