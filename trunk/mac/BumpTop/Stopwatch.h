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

#ifndef BUMPTOP_STOPWATCH_H_
#define BUMPTOP_STOPWATCH_H_

class Stopwatch {
  uint64_t anchor_time_;
  uint64_t paused_time_;
  bool paused_;


 public:
  Stopwatch();

  static uint64_t currentTime();

  // Actions
  uint64_t restart();
  uint64_t elapsed() const;
  void increaseBy(uint64_t milliseconds);
  void decreaseBy(uint64_t milliseconds);
  void pause();
  void unpause();
};

#endif  // BUMPTOP_STOPWATCH_H_
