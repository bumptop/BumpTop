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

#ifndef BUMPTOP_BUMPTOPSCENE_H_
#define BUMPTOP_BUMPTOPSCENE_H_

#include "BumpTop/Room.h"
#include "BumpTop/Scene.h"

class QuickLookPreviewPanel;
class Timer;

class BumpTopScene : public Scene {
  Q_OBJECT

 public:
  explicit BumpTopScene(BumpTopApp *app);
  virtual QuickLookPreviewPanel* QuickLook_preview_panel();
  virtual Room* room();
  virtual void markSceneAsChanged();
  virtual void set_surface_that_camera_is_zoomed_to(RoomSurfaceType surface);
  virtual RoomSurfaceType surface_that_camera_is_zoomed_to();

  virtual void init();
 protected slots:  // NOLINT
  virtual void onRenderHandler();
  virtual void applicationWillTerminate();
  virtual void windowRectChanged();
  virtual void repositioningTimerTick(Timer* timer);
  virtual void saveSceneFileTimerTick(Timer* timer);

 protected:
  bool scene_file_should_be_saved_;
  Room *room_;
  QuickLookPreviewPanel* QuickLook_preview_panel_;
  RoomSurfaceType surface_that_camera_is_zoomed_to_;
  Timer* repositioning_timer_;
  Timer* save_scene_file_timer_;
};

#endif  // BUMPTOP_BUMPTOPSCENE_H_
