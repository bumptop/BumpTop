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

#ifndef BUMPTOP_TIMER_H_
#define BUMPTOP_TIMER_H_

class Timer : public QObject {
  Q_OBJECT

 public:
  Timer();
  ~Timer();

  void start(int milliseconds);
  void stop();

 signals:
  void onTick(Timer* timer);

 public slots: // NOLINT
  void renderTickReceived();

 protected:
  int milliseconds_;
  bool receiving_render_ticks_;
  Stopwatch stopwatch_;
};

#endif  // BUMPTOP_TIMER_H_
