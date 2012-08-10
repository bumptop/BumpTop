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

#ifndef BUMPTOP_PERFORMANCESTATSHUD_H_
#define BUMPTOP_PERFORMANCESTATSHUD_H_

#include "BumpTop/Singleton.h"

class PerformanceStatsHUD : public QObject {
  Q_OBJECT
SINGLETON_HEADER(PerformanceStatsHUD)
 public:
  explicit PerformanceStatsHUD();
  void toggleHUD();

 protected slots:  // NOLINT
  void renderTick();

 protected:
  Ogre::Overlay* debug_overlay_;
  Ogre::OverlayElement* avg_FPS_;
  Ogre::OverlayElement* curr_FPS_;
  Ogre::OverlayElement* best_FPS_;
  Ogre::OverlayElement* worst_FPS_;
  Ogre::OverlayElement* memory_use_in_bytes;
  bool show_HUD_;
  static bool HUD_resources_are_loaded_;
};

#endif  // BUMPTOP_PERFORMANCESTATSHUD_H_
