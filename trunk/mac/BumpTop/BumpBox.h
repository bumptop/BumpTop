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

#ifndef BUMPTOP_BUMPBOX_H_
#define BUMPTOP_BUMPBOX_H_

#include <string>
#include <utility>

#include "BumpTop/DropReceiver.h"
#include "BumpTop/Room.h"
#include "BumpTop/VisualPhysicsActor.h"

class BumpBox;
class BumpBoxLabel;
class ContextMenuItem;
class DampedSpringMouseHandler;
class FileItem;
class MaterialLoader;
class HighlightActor;
class RoomSurface;
class Room;
class VisualPhysicsActorAnimation;
class VisualPhysicsActorBuffer;

class BumpBox : public VisualPhysicsActor {
  Q_OBJECT

 public:
  explicit BumpBox(Ogre::SceneManager *scene_manager, Physics* physics, Room *room, VisualPhysicsActorId unique_id = 0);
  virtual ~BumpBox();

  virtual void init();
  virtual void initWithPath(QString file_path, bool physics_enabled = true);
  virtual bool initFromBuffer(VisualPhysicsActorBuffer* buffer, bool physics_enabled = true);
  virtual void initAsVisualCopyOfActor(VisualPhysicsActor* actor);
  virtual VisualPhysicsActor* createVisualCopyOfSelf();
  virtual void set_selected(bool is_selected);
  virtual bool selected();
  virtual const QString& path();
  virtual void writeToBuffer(VisualPhysicsActorBuffer* buffer);
  virtual bool serializable();
  virtual void setPhysicsConstraintsForSurface(RoomSurface* surface);
  virtual BumpPose getPoseForSurface(RoomSurface* surface);
  virtual void pinToSurface(RoomSurface* surface);
  virtual BumpBox* constructCopyOfMyType();

  virtual Ogre::Vector2 labelPositionForCurrentPosition();
  virtual void launch();
  virtual void launch(QString app);
  virtual void rename(QString new_name);
  virtual QString display_name();
  virtual bool nameable();

  std::pair<Ogre::Vector3, Ogre::Vector3> angularConstraintsForSurface(RoomSurface* surface);

  virtual void set_size(Ogre::Vector3 size);
  virtual void set_position_no_physics(const Ogre::Vector3 &pos);
  virtual void set_parent(VisualPhysicsActor* parent);
  virtual void set_label_visible(bool label_visible);
  virtual void set_render_queue_group(uint8 queue_id);
  virtual void set_render_queue_group_for_mouse_up(uint8 queue_id);

  virtual bool label_visible();
  virtual void set_room_surface(RoomSurface* room_surface);
  virtual bool is_an_image_on_wall();
  virtual void set_name_hidden(bool name_hidden);
  virtual bool name_hidden();

  void set_surface_and_position(RoomSurface* surface, Ogre::Vector3 position);

  VisualPhysicsActorType actor_type();

  virtual BumpTopCommandSet* supported_context_menu_items();
  bool supportsContextMenuItem(BumpTopCommand* context_menu_option);
  VisualPhysicsActor* lowest_child_with_visual_actor();
  void startLaunchAnimation();
  virtual void update();
  virtual BumpBoxLabel* label();
  virtual void draggingExited();

  virtual Ogre::Vector2 actor_screen_position_before_drag();
  virtual BumpPose actor_pose_before_drag();
  virtual RoomSurface* actor_room_surface_before_drag();
  virtual void updateActorStatusBeforeDrag();

  virtual VisualPhysicsActorId actor_parent_id_before_drag();
  virtual VisualPhysicsActorType actor_parent_type_before_drag();
  virtual Ogre::Vector3 actor_parent_position_before_drag();
  virtual BumpPose actor_offset_pose_to_its_parent();
  virtual QHash<VisualPhysicsActorId, BumpPose> actor_siblings_offset_poses_to_parent();
  virtual void updateActorParentInfoBeforeDrag();
  virtual void updateActorOffsetPoseToItsParentBeforeDrag();
  virtual void updateActorSiblingOffsetPoseToParentBeforeDrag();

 public slots: // NOLINT
  virtual void mouseDown(MouseEvent* mouse_event);
  virtual void mouseDragged(MouseEvent* mouse_event);
  virtual void mouseUp(MouseEvent* mouse_event);
  virtual void labelClicked(MouseEvent* mouse_event);
  virtual void rightMouseDown(MouseEvent* mouse_event);
  virtual void draggingEntered(MouseEvent* mouse_event);
  virtual void draggingUpdated(MouseEvent* mouse_event);
  virtual void fileRemoved(const QString& path);
  virtual void updateIcon();
  virtual void updateLabel(Ogre::Real size_factor = -1);
  virtual void updateLabelPosition();
  virtual void set_label_visible_from_camera_position(bool visible);
  virtual void performDragOperation();

  void loadQuickLookIcon(MaterialLoader *material_loader);
  void setMaterialNameAndDeleteMaterialLoader(MaterialLoader *material_loader);

 protected:
  static BumpTopCommandSet* context_menu_items_set;
  virtual std::string meshName();
  virtual Ogre::Vector3 absoluteMeshSizeDividedBy100();
  virtual btVector3 physicsSize();
  virtual void makePhysicsActor(bool physics_enabled);
  virtual void deleteLabel();
  virtual void clearMaterialLoader();

  virtual void set_path(QString file_name);

  BumpBoxLabel *label_;
  bool label_visible_;
  bool label_visible_from_camera_position_;

  Room *room_;
  FileItem *file_item_;
  HighlightActor* highlight_;
  DampedSpringMouseHandler* mouse_handler_;

  MaterialLoader* material_loader_;
  bool is_material_dirty_;

  bool is_selected_;
  QString icon_material_name_;
  DropReceiver *drop_receiver_;

  bool is_dir_;
  uint8 render_queue_group_on_mouse_down_;

  Ogre::Vector2 screen_position_before_drag_;
  BumpPose pose_before_drag_;
  RoomSurface* room_surface_before_drag_;
  VisualPhysicsActorId actor_parent_id_before_drag_;
  BumpPose actor_offset_pose_to_its_parent_;
  QHash<VisualPhysicsActorId, BumpPose> actor_siblings_offset_poses_to_parent_;
  VisualPhysicsActorType actor_parent_type_before_drag_;
  Ogre::Vector3 actor_parent_position_before_drag_;
};

#endif  // BUMPTOP_BUMPBOX_H_
