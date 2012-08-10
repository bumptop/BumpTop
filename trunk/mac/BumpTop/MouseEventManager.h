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

#ifndef BUMPTOP_MOUSEEVENTMANAGER_H_
#define BUMPTOP_MOUSEEVENTMANAGER_H_

#include <Ogre.h>
#include <QtCore/QObject>

#include <utility>

#include "BumpTop/Clickable.h"

class BumpTopApp;
class DropReceiver;
class PolygonLevelRaySceneQuery;
class VisualPhysicsActor;

using boost::tuple;

enum MouseEventType {
  MOUSE_DOWN,
  MOUSE_DRAGGED,
  MOUSE_UP,
  RIGHT_MOUSE_DOWN,
  RIGHT_MOUSE_DRAGGED,
  MOUSE_MOVED,
  RIGHT_MOUSE_UP,
  DRAGGING_ENTERED,
  DRAGGING_UPDATED,
  SCROLL_WHEEL
};

class MouseEventManager : public QObject {
  Q_OBJECT
 public:
  explicit MouseEventManager(BumpTopApp* bumptop);
  ~MouseEventManager();

  void mouseDown(float x, float y, int num_clicks, int modifier_flags);
  void mouseDragged(float x, float y, int num_clicks, int modifier_flags);
  void mouseUp(float x, float y, int num_clicks, int modifier_flags);
  void mouseMoved(float x, float y, int num_clicks, int modifier_flags);

  void rightMouseDown(float x, float y, int num_clicks, int modifier_flags);
  void rightMouseDragged(float x, float y, int num_clicks, int modifier_flags);
  void rightMouseUp(float x, float y, int num_clicks, int modifier_flags);

  void scrollWheel(float x, float y, int num_clicks, int modifier_flags, float delta_y);

  void mouseEntered();
  void mouseExited();
  void draggingExited();

  std::pair<bool, Ogre::Vector3> intersectWithEntityMesh(Ogre::Entity *entity, Ogre::Vector2 mouse_position);

#if defined(OS_MACOSX)
  std::pair<DropReceiver*, NSDragOperation> draggingEntered(float x, float y, NSDragOperation source_drag_operations,
                                                            VisualPhysicsActorList items_being_dropped = VisualPhysicsActorList(),  // NOLINT
                                                            bool disable_global_mouse_capture = false);
  std::pair<DropReceiver*, NSDragOperation> draggingUpdated(float x, float y, NSDragOperation source_drag_operations,
                                                            VisualPhysicsActorList items_being_dropped = VisualPhysicsActorList(),  // NOLINT
                                                            bool disable_global_mouse_capture = false);
#else
  void draggingEntered(float x, float y);
  void draggingUpdated(float x, float y);
#endif

  Ogre::Ray createMouseRay(float x, float y);
  void set_global_capture(Clickable *capture_object);
  Clickable *global_capture();

 signals:
  void onMouseDown(MouseEvent* mouse_event);
  void onMouseEntered();
  void onMouseExited();
  void onDraggingExited();
  void onDraggingEntered();
  void onDraggingUpdated();

 protected:
  void sendMouseEvent(MouseEventType mouse_event_type, Ogre::MovableObject* movable,
                      Clickable* clickable, MouseEvent* mouse_event);
  void emitMouseSignal(MouseEventType mouse_event_type, Ogre::MovableObject* movable,
                       Clickable* clickable, MouseEvent* mouse_event);
  MouseEvent routeMouseEventThroughOverlayHierarchy(Ogre::OverlayContainer* overlay_container,
                                                    MouseEventType mouse_event_type, float x, float y,
                                                    int num_clicks, int modifier_flags,
                                                    NSDragOperation source_drag_operations, Ogre::Real delta_y);

  MouseEvent routeMouseEvent(MouseEventType mouse_event_type, float x, float y, int num_clicks,
                             int modifier_flags, NSDragOperation source_drag_operations = NSDragOperationNone,
                             VisualPhysicsActorList items_being_dropped = VisualPhysicsActorList(),
                             bool disable_global_mouse_capture = false, Ogre::Real delta_y = 0);
  void bubbleMouseEvent(MouseEventType mouse_event_type, Ogre::MovableObject* movable,
                        Clickable* clickable, MouseEvent* event);

  BumpTopApp* bumptop_;
  PolygonLevelRaySceneQuery* polygon_level_ray_scene_query_;
  Clickable *global_capture_object_;
  bool is_handling_mouse_up_;
};

#endif  // BUMPTOP_MOUSEEVENTMANAGER_H_
