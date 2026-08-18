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

#include "BumpTop/BumpTopScene.h"

#include "BumpTop/BumpTopApp.h"
#include "BumpTop/BumpTopInstanceLock.h"
#include "BumpTop/FileManager.h"
#include "BumpTop/PersistenceManager.h"
#include "BumpTop/PhysicsOnlyBox.h"
#include "BumpTop/QuickLookPreviewPanel.h"
#include "BumpTop/RoomItemPoseConstraints.h"
#include "BumpTop/MouseEventManager.h"
#include "BumpTop/StickyNotePad.h"
#include "BumpTop/Timer.h"
#include "BumpTop/VisualPhysicsActorList.h"



BumpTopScene::BumpTopScene(BumpTopApp *app)
: Scene(app),
  room_(NULL),
  scene_file_should_be_saved_(false) {
  assert(QObject::connect(app, SIGNAL(onRender()),
                          this, SLOT(onRenderHandler())));
}

void BumpTopScene::init() {
  QString room_path;
  if (BumpTopInstanceLock::is_running_in_sandbox()) {
    RedDot* red_dot = new RedDot();
    red_dot->init();

    QDir sandbox = QDir(FileManager::getApplicationDataPath() + "Sandbox/");
    if (!sandbox.exists())
      sandbox.mkpath(sandbox.absolutePath());
    room_path = sandbox.absolutePath();
  } else {
    room_path = FileManager::getDesktopPath();
  }

  assert(QObject::connect(app_, SIGNAL(onApplicationWillTerminate()),
                          this, SLOT(applicationWillTerminate())));
  assert(QObject::connect(app_, SIGNAL(onWindowRectChanged()),
                          this, SLOT(windowRectChanged())));

  room_ = new Room(app_, NULL, app_->ogre_scene_manager(), app_->physics(), room_path);
  QuickLook_preview_panel_ = new QuickLookPreviewPanel(room_);

  repositioning_timer_ = new Timer();
  save_scene_file_timer_ = new Timer();
  assert(QObject::connect(repositioning_timer_, SIGNAL(onTick(Timer*)),  // NOLINT
                          this, SLOT(repositioningTimerTick(Timer*))));  // NOLINT
  assert(QObject::connect(save_scene_file_timer_, SIGNAL(onTick(Timer*)),  // NOLINT
                          this, SLOT(saveSceneFileTimerTick(Timer*))));  // NOLINT
  repositioning_timer_->start(1000);
  save_scene_file_timer_->start(5000);

  if (!loadRoomFromFile(room_, FileManager::getApplicationDataPath(), app_->screen_resolution())) {
    loadRoomFromDesktop(room_);
  }

  room_->deselectActors();

  // Parity test: select the named desktop item so the selection rendering
  // (label background + icon outline) can be diffed against Finder's.
  if (getenv("BUMPTOP_PARITY_SELECT") != NULL) {
    QString wanted = QString::fromUtf8(getenv("BUMPTOP_PARITY_SELECT"));
    for_each(VisualPhysicsActor* actor, room_->room_actor_list()) {
      if (QFileInfo(actor->path()).fileName() == wanted) {
        actor->set_selected(true);
      }
    }
  }

  /*
  ThemeDownloader* theme_downloader = new ThemeDownloader();
  theme_downloader->init();
  theme_downloader->launch();
  */
  
}

Room* BumpTopScene::room() {
  return room_;
}

void BumpTopScene::markSceneAsChanged() {
  scene_file_should_be_saved_ = true;
}

void BumpTopScene::repositioningTimerTick(Timer* timer) {
  repositionActorsConstrainedToRoom(room_->room_actor_list(), room_, true);
  repositioning_timer_->start(1000);
}

void BumpTopScene::saveSceneFileTimerTick(Timer* timer) {
  // Serializing the room blocks the render/physics loop; never do it while
  // the user is mid-drag (it read as a visible hitch), just retry next tick.
  bool mouse_interaction_in_progress = app_->mouse_event_manager()->global_capture() != NULL;
  if (scene_file_should_be_saved_ && !mouse_interaction_in_progress) {
    writeRoomToFile(room_, FileManager::getApplicationDataPath());
    scene_file_should_be_saved_ = false;
  }
  save_scene_file_timer_->start(5000);
}

void BumpTopScene::onRenderHandler() {
}

void BumpTopScene::applicationWillTerminate() {
  writeRoomToFile(room_, FileManager::getApplicationDataPath());
}

void BumpTopScene::windowRectChanged() {
  if (room_ != NULL) {
    // Room dimensions are in points (Finder desktop coordinates); with Retina
    // rendering window_size() is in device pixels, so use the screen size.
    Ogre::Vector2 window_size = app_->screen_resolution();
    room_->resizeRoomForResolution(window_size.x, window_size.y);
  }
}

QuickLookPreviewPanel* BumpTopScene::QuickLook_preview_panel() {
  return QuickLook_preview_panel_;
}

void BumpTopScene::set_surface_that_camera_is_zoomed_to(RoomSurfaceType surface) {
  surface_that_camera_is_zoomed_to_ = surface;
}

RoomSurfaceType BumpTopScene::surface_that_camera_is_zoomed_to() {
  return surface_that_camera_is_zoomed_to_;
}

