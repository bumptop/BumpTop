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

#ifndef BUMPTOP_CLICKABLE_H_
#define BUMPTOP_CLICKABLE_H_

#include "BumpTop/VisualPhysicsActorList.h"

class Clickable;
class DropReceiver;
class VisualPhysicsActor;

struct MouseEvent {
  MouseEvent();
  MouseEvent(Ogre::Vector2 mouse_in_window_space, bool intersects_item, Clickable* item,
             Ogre::Vector3 mouse_in_world_space, int num_clicks, int modifier_flags, bool capture,
             bool global_capture, NSDragOperation source_drag_operations = NSDragOperationNone,
             VisualPhysicsActorList items_being_dropped = VisualPhysicsActorList(), Ogre::Real delta_y = 0);
  Ogre::Vector2 mouse_in_window_space;
  bool intersects_item;
  Clickable* item;
  Ogre::Vector3 mouse_in_world_space;
  Ogre::Real delta_y;
  int num_clicks;
  bool handled;
  bool cancelled;
  int modifier_flags;
  bool capture;
  bool global_capture;
  VisualPhysicsActorList items_being_dropped;
#if defined(OS_MACOSX)
  NSDragOperation drag_operations;
  DropReceiver* drop_receiver;
#endif
};

class Clickable : public QObject {
  Q_OBJECT

 public:
  Clickable();
  virtual ~Clickable();

  virtual void mouseDown(MouseEvent* mouse_event);
  virtual void mouseUp(MouseEvent* mouse_event);
  virtual void mouseDragged(MouseEvent* mouse_event);
  virtual void mouseMoved(MouseEvent* mouse_event);

  virtual void rightMouseDown(MouseEvent* mouse_event);
  virtual void rightMouseUp(MouseEvent* mouse_event);
  virtual void rightMouseDragged(MouseEvent* mouse_event);

  virtual void draggingEntered(MouseEvent* mouse_event);
  virtual void draggingUpdated(MouseEvent* mouse_event);

  virtual void scrollWheel(MouseEvent* mouse_event);

  virtual Ogre::Entity* _entity() = 0;
  virtual Clickable* clickable_parent();
  virtual bool capture_mouse_events();

 signals:
  void onMouseDown(MouseEvent* mouse_event);
  void onMouseUp(MouseEvent* mouse_event);
  void onMouseDragged(MouseEvent* mouse_event);
  void onMouseMoved(MouseEvent* mouse_event);

  void onRightMouseDown(MouseEvent* mouse_event);
  void onRightMouseUp(MouseEvent* mouse_event);
  void onRightMouseDragged(MouseEvent* mouse_event);

  void onDraggingEntered(MouseEvent* mouse_event);
  void onDraggingUpdated(MouseEvent* mouse_event);

  void onScrollWheel(MouseEvent* mouse_event);

 protected:
  Clickable* clickable_parent_;
};

class ClickableOverlay : public Clickable {
 public:
  ClickableOverlay();
  ~ClickableOverlay();

  virtual Ogre::Entity* _entity();
};

#endif  // BUMPTOP_CLICKABLE_H_
