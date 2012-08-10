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

#include "BumpTop/BumpTopApp.h"
#include "BumpTop/Timer.h"

Timer::Timer()
: receiving_render_ticks_(false) {
}

Timer::~Timer() {
  BumpTopApp::singleton()->disconnect(this);
}

void Timer::start(int milliseconds) {
  stopwatch_.restart();
  milliseconds_ = milliseconds;
  if (!receiving_render_ticks_) {
    receiving_render_ticks_ = true;
    assert(QObject::connect(BumpTopApp::singleton(), SIGNAL(onRender()),
                            this, SLOT(renderTickReceived())));
  }
}

void Timer::stop() {
  receiving_render_ticks_ = false;
  BumpTopApp::singleton()->disconnect(this);
}

void Timer::renderTickReceived() {
  if (stopwatch_.elapsed() >= milliseconds_) {
    receiving_render_ticks_ = false;
    BumpTopApp::singleton()->disconnect(this);
    emit onTick(this);
  }
}

#include "moc/moc_Timer.cpp"
