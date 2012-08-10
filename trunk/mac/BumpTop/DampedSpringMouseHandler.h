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

#ifndef BUMPTOP_DAMPEDSPRINGMOUSEHANDLER_H_
#define BUMPTOP_DAMPEDSPRINGMOUSEHANDLER_H_

#include <QtCore/QObject>

#include "BumpTop/Clickable.h"
#include "BumpTop/OgreHelpers.h"
#include "BumpTop/VisualPhysicsActorId.h"

enum DraggingMode {
  SNAP_TO_WALL,
  PHYSICAL_DRAG
};

enum MouseCursor {
  kRegularCursor,
  kCopyCursor,
  kLinkCursor
};

class Room;
class RoomSurface;
class VisualPhysicsActor;

class DampedSpringMouseHandler : public QObject {
  Q_OBJECT

 public:
  explicit DampedSpringMouseHandler(VisualPhysicsActor* parent, Room* room);
  virtual ~DampedSpringMouseHandler();

  virtual void mouseDown(MouseEvent* mouse_event);
  virtual void mouseDragged(MouseEvent* mouse_event);
  virtual void mouseUp(MouseEvent* mouse_event);
  virtual void beginDraggingItems();

 public slots:  // NOLINT
  void renderUpdate();

  void mouseExit();
  void dragEntered();
  void dragExited();

 protected:
  void dragAndDropUpdate(MouseEvent* mouse_event);
  bool isMouseOverMenuBar();
  bool shouldInitiateDragForAutoHideDock(Ogre::Vector2 mouse_in_window_space);
  BumpPose getConstrainedPoseForDragging(VisualPhysicsActor* actor, Ogre::Vector3 desired_dragging_position);
  Room* room_;
  VisualPhysicsActor* actor_;
  VisualPhysicsActorId actor_id_;
  VisualPhysicsActor* parent_;
  Ogre::Vector3 stab_point_object_space_;
  RoomSurface* surface_on_mouse_down_;
  BumpPose pose_on_mouse_down_;
  Ogre::Vector2 last_mouse_in_window_space_;
  Ogre::Vector2 screen_resolution_;
  bool drag_ignored_for_menu_bar_;
  bool begin_drag_operation_on_next_mouse_drag_;
  bool mouse_motion_registered_;
  bool mouse_drag_has_exited_bumptop_;
  bool drag_and_drop_started_;
  bool dragging_as_group_;
  bool actor_pose_may_violate_surface_constraints_;
  QList<VisualPhysicsActorId> drag_partner_ids_;
  QHash<VisualPhysicsActor*, BumpPose> drag_partners_poses_on_mouse_down_;
  QHash<VisualPhysicsActor*, RoomSurface*> drag_partners_surfaces_on_mouse_down_;
  QHash<VisualPhysicsActor*, Ogre::Vector3> drag_partners_offsets_;
  QHash<VisualPhysicsActor*, bool> drag_partners_collide_with_other_items_;
  DropReceiver *drop_receiver_;
  bool is_droppable_;
  NSDragOperation drag_operation_;
  Ogre::Vector3 size_on_mouse_down_;
  MouseCursor current_cursor_;
};

#endif  // BUMPTOP_DAMPEDSPRINGMOUSEHANDLER_H_
