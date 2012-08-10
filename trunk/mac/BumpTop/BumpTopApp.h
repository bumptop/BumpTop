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

#ifndef BUMPTOP_BUMPTOPAPP_H_
#define BUMPTOP_BUMPTOPAPP_H_

#include <Ogre.h>
#include <OgrePanelOverlayElement.h>
#include <QtCore/QObject>
#include <string>

#include "BumpTop/Singleton.h"
#include "BumpTop/Stopwatch.h"
#include "BumpTop/UsageTracker.h"

class BumpTopScene;
class DragAndDrop;
class KeyboardEventManager;
class MouseEventManager;
class Physics;
@class FullScreenWindow;

class BumpTopApp : public QObject {
  Q_OBJECT
  SINGLETON_HEADER(BumpTopApp)

 public:
  explicit BumpTopApp();
  virtual ~BumpTopApp();

  virtual void init();
  virtual void mouseDown(float x, float y, int num_clicks, int modifier_flags);
  virtual void mouseUp(float x, float y, int num_clicks, int modifier_flags);
  virtual void mouseDragged(float x, float y, int num_clicks, int modifier_flags);
  virtual void mouseMoved(float x, float y, int num_clicks, int modifier_flags);
  virtual void rightMouseDown(float x, float y, int num_clicks, int modifier_flags);
  virtual void rightMouseUp(float x, float y, int num_clicks, int modifier_flags);
  virtual void rightMouseDragged(float x, float y, int num_clicks, int modifier_flags);
  virtual void scrollWheel(float x, float y, int num_clicks, int modifier_flags, float delta_y);
  virtual void applicationWillTerminate();
  virtual void renderTick();
  virtual QString platform() = 0;
  virtual void processOneEvent(int milliseconds = -1) = 0;
  virtual void markGlobalStateAsChanged();
  virtual bool isInIdleMode();
  static void makeSelfForegroundApp();
  virtual QString bumptopVersion() = 0;

  virtual void pushGLContextAndSwitchToOgreGLContext() = 0;
  virtual void popGLContext() = 0;

  Ogre::RenderWindow* render_window();
  Ogre::SceneManager* ogre_scene_manager();
  Ogre::Camera* camera();
  Ogre::SceneNode* camera_node();
  Ogre::Viewport* viewport();
  Physics* physics();
  MouseEventManager* mouse_event_manager();
  KeyboardEventManager* keyboard_event_manager();
  virtual void windowRectChanged();

  virtual void set_scene(BumpTopScene* scene);
  virtual BumpTopScene* scene();

  virtual Ogre::Vector2 screen_resolution() = 0;
  virtual Ogre::Vector2 window_size() = 0;
  virtual Ogre::Vector2 mouse_location() = 0;
  virtual void terminate_application() = 0;
  virtual FullScreenWindow* window() = 0;
  virtual void set_window(FullScreenWindow* window) = 0;
  virtual Ogre::Vector2 taskbar_item_location() = 0;
  virtual DragAndDrop* drag_and_drop();
  virtual bool should_force_bumptop_window_to_front();
  virtual void set_should_force_bumptop_window_to_front(bool force_to_front);
  virtual void set_context_menu_open(bool open);
  virtual bool context_menu_open();

 signals:
  void onMouseDown(float x, float y, int num_clicks, int modifier_flags);
  void onMouseDragged(float x, float y, int num_clicks, int modifier_flags);
  void onMouseUp(float x, float y, int num_clicks, int modifier_flags);
  void onMouseMoved(float x, float y, int num_clicks, int modifier_flags);
  void onRightMouseDown(float x, float y, int num_clicks, int modifier_flags);
  void onRightMouseDragged(float x, float y, int num_clicks, int modifier_flags);
  void onRightMouseUp(float x, float y, int num_clicks, int modifier_flags);
  void onRender();
  void onUndo();
  void onRedo();
  void onPileize();
  void onPileBreak();
  void onApplicationWillTerminate();
  void onWindowRectChanged();

 protected:
  virtual void createRootNode();
  virtual void setRenderSystem();
  virtual void initOgreCore() = 0;
  virtual void speedUpRenderTimer() = 0;
  virtual void createSceneManager();
  virtual void createCamera();
  virtual void createViewports();
  virtual void initResources();
  virtual void initGlobalBumpTopMaterials();
  virtual void createMouseEventManager();
  virtual void createKeyboardEventManager();
  virtual void initPhysics();
  virtual void initRenderStopwatch();
  virtual void initUsageTracker();

  bool global_state_changed_this_frame_;
  bool global_state_changed_last_frame_;
  bool should_force_bumptop_window_to_front_;
  bool context_menu_open_;

  Stopwatch render_stopwatch_;

  Ogre::Camera* camera_;
  Ogre::SceneNode* camera_node_;
  Ogre::SceneManager* scene_manager_;
  Ogre::Viewport* viewport_;
  Ogre::RenderWindow* render_window_;
  Physics* physics_;
  MouseEventManager* mouse_event_manager_;
  KeyboardEventManager* keyboard_event_manager_;
  DragAndDrop* drag_and_drop_;
  BumpTopScene* scene_;
  UsageTracker usage_tracker_;
};

#endif  // BUMPTOP_BUMPTOPAPP_H_
