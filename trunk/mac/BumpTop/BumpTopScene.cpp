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

#include <QtCore/QTimer>

#include "BumpTop/BumpTopApp.h"
#include "BumpTop/BumpTopCommands.h"
#include "BumpTop/BumpTopInstanceLock.h"
#include "BumpTop/FileManager.h"
#include "BumpTop/PersistenceManager.h"
#include "BumpTop/PhysicsOnlyBox.h"
#include "BumpTop/QuickLookPreviewPanel.h"
#include "BumpTop/RoomItemPoseConstraints.h"
#include "BumpTop/MouseEventManager.h"
#include "BumpTop/OgreHelpers.h"
#include "BumpTop/QStringHelpers.h"
#include "BumpTop/StickyNotePad.h"
#include "BumpTop/VisualActor.h"
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

  // Repro hooks for debugging without mouse input.
  // BUMPTOP_TEST_GROW=1: grow every desktop item once (like toolbar Grow).
  // BUMPTOP_TEST_PILE_GRID=1: pile the first two file items, then open the
  // pile as a grid (like double-clicking it).
  if (getenv("BUMPTOP_TEST_GROW") != NULL || getenv("BUMPTOP_TEST_PILE_GRID") != NULL) {
    fprintf(stderr, "[test] hooks active, sandbox=%d, room actors=%d\n",
            (int)BumpTopInstanceLock::is_running_in_sandbox(), (int)room_->room_actor_list().size());
    BumpEnvironment env(app_->physics(), room_, app_->ogre_scene_manager());
    VisualPhysicsActorList test_actors;
    for_each(VisualPhysicsActor* actor, room_->room_actor_list()) {
      if (actor->actor_type() == BUMP_BOX)
        test_actors.append(actor);
    }
    // room_actor_list() is hash-ordered; sort for a deterministic pick.
    std::sort(test_actors.begin(), test_actors.end(),
              [](VisualPhysicsActor* a, VisualPhysicsActor* b) { return a->path() < b->path(); });
    if (getenv("BUMPTOP_TEST_GROW") != NULL && test_actors.size() > 0) {
      int grow_times = std::max(1, atoi(getenv("BUMPTOP_TEST_GROW")));
      for (int i = 0; i < grow_times; i++)
        Grow::singleton()->applyToActors(env, test_actors);
      fprintf(stderr, "[test] grew %d actors %d times\n", (int)test_actors.size(), grow_times);
    }
    if (getenv("BUMPTOP_TEST_PILE_GRID") != NULL && test_actors.size() >= 2) {
      VisualPhysicsActorList pile_members;
      pile_members.append(test_actors[0]);
      pile_members.append(test_actors[1]);
      CreatePile::singleton()->applyToActors(env, pile_members);
      for_each(VisualPhysicsActor* actor, room_->room_actor_list()) {
        // Skip the (empty) New Items Pile: only launch a pile with members.
        if (actor->actor_type() == BUMP_PILE && actor->children().size() >= 2) {
          fprintf(stderr, "[test] launching pile (%d members) as grid\n",
                  (int)actor->children().size());
          actor->launch();
          break;
        }
      }
      // After the grid settles, click its close button through the real
      // mouse pipeline, then dump again to verify it closed.
      BumpTopApp* app_for_click = app_;
      Room* room_for_click = room_;
      QTimer::singleShot(4000, [app_for_click, room_for_click]() {
        for_each(VisualPhysicsActor* actor, room_for_click->room_actor_list()) {
          if (actor->actor_type() == GRIDDED_PILE) {
            Ogre::Vector3 close_world = actor->world_position() + Ogre::Vector3(-213, 20, -213);
            Ogre::Vector2 sp = worldPositionToScreenPosition(close_world);
            fprintf(stderr, "[test] clicking close button at screen (%.0f,%.0f)\n", sp.x, sp.y);
            app_for_click->mouseDown(sp.x, sp.y, 1, 0);
            app_for_click->mouseUp(sp.x, sp.y, 1, 0);
            break;
          }
        }
      });
      QTimer::singleShot(6000, [room_for_click]() {
        int grids = 0;
        for_each(VisualPhysicsActor* actor, room_for_click->room_actor_list()) {
          if (actor->actor_type() == GRIDDED_PILE)
            grids++;
        }
        fprintf(stderr, "[test] after close click: %d gridded piles remain\n", grids);
      });
      Room* room_for_dump = room_;
      QTimer::singleShot(3000, [room_for_dump]() {
        for_each(VisualPhysicsActor* actor, room_for_dump->room_actor_list()) {
          Ogre::Vector3 p = actor->world_position();
          Ogre::Vector3 vp = actor->visual_actor() != NULL ?
              actor->visual_actor()->ogre_scene_node()->_getDerivedPosition() : Ogre::Vector3::ZERO;
          fprintf(stderr, "[dump] type=%d phys=(%.0f,%.0f,%.0f) visual=(%.0f,%.0f,%.0f) children=%d path=%s\n",
                  actor->actor_type(), p.x, p.y, p.z, vp.x, vp.y, vp.z,
                  (int)actor->children().size(),
                  utf8(QFileInfo(actor->path()).fileName()).c_str());
          for_each(VisualPhysicsActor* child, actor->children()) {
            Ogre::Vector3 cp = child->world_position();
            Ogre::Vector3 cvp = child->visual_actor() != NULL ?
                child->visual_actor()->ogre_scene_node()->_getDerivedPosition() : Ogre::Vector3::ZERO;
            fprintf(stderr, "[dump]    child type=%d phys=(%.0f,%.0f,%.0f) visual=(%.0f,%.0f,%.0f) path=%s\n",
                    child->actor_type(), cp.x, cp.y, cp.z, cvp.x, cvp.y, cvp.z,
                    utf8(QFileInfo(child->path()).fileName()).c_str());
          }
        }
      });
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

