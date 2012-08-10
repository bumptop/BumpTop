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

#include "BumpTop/Stopwatch.h"

#include "BumpTop/Math.h"

Stopwatch::Stopwatch()
: paused_(false) {
  anchor_time_ = currentTime();
  restart();
}

uint64_t Stopwatch::restart() {
  uint64_t new_anchor_time = currentTime();
  uint64_t elapsed = std::max((uint64_t)0, new_anchor_time - anchor_time_);
  anchor_time_ = new_anchor_time;
  return elapsed;
}

uint64_t Stopwatch::currentTime() {
  uint64_t absolute_time = mach_absolute_time();
  Nanoseconds current_time_in_nanoseconds = AbsoluteToNanoseconds(*(AbsoluteTime *) &absolute_time);  // NOLINT
  uint64_t current_time_in_milliseconds = (*(uint64_t*) &current_time_in_nanoseconds) / 1000000;  // NOLINT
  return current_time_in_milliseconds;
}

uint64_t Stopwatch::elapsed() const {
  if (paused_) {
    return std::max((uint64_t)0, paused_time_ - anchor_time_);
  } else {
    return std::max((uint64_t)0, currentTime() - anchor_time_);
  }
}

void Stopwatch::increaseBy(uint64_t milliseconds) {
  anchor_time_ -= milliseconds;
}

void Stopwatch::decreaseBy(uint64_t milliseconds) {
  anchor_time_ += milliseconds;
}

void Stopwatch::pause() {
  if (!paused_) {
    paused_time_ = currentTime();
    paused_ = true;
  }
}

void Stopwatch::unpause() {
  if (paused_) {
    uint64_t elapsedSincePaused = std::max((uint64_t)0, currentTime() - paused_time_);
    decreaseBy(elapsedSincePaused);
    paused_ = false;
  }
}
