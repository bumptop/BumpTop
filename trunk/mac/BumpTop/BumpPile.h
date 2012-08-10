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

#ifndef BUMPTOP_BUMPPILE_H_
#define BUMPTOP_BUMPPILE_H_

#include <string>

#include "BumpTop/AnimationManager.h"
#include "BumpTop/BumpDummy.h"
#include "BumpTop/BumpTopCommands.h"
#include "BumpTop/OgreHelpers.h"
#include "BumpTop/VisualPhysicsActor.h"
#include "BumpTop/VisualPhysicsActorId.h"

class BumpBoxLabel;
class BumpPileDropReceiver;
class DampedSpringMouseHandler;
class HighlightActor;
class Room;

class BumpPile : public VisualPhysicsActor {
  Q_OBJECT
 public:
  static BumpTopCommandSet* context_menu_items_set;
  explicit BumpPile(Ogre::SceneManager *scene_manager,
                    Physics* physics, Room* room, VisualPhysicsActorId unique_id = 0);
  virtual ~BumpPile();
  virtual void init();

  virtual bool initFromBuffer(VisualPhysicsActorBuffer* buffer, bool physics_enabled = true);
  virtual void initWithActors(VisualPhysicsActorList actors,
                              Ogre::Vector3 initial_position = Ogre::Vector3::ZERO,
                              QList<Ogre::Vector3> offsets = QList<Ogre::Vector3>(),
                              QList<Ogre::Quaternion> orientations = QList<Ogre::Quaternion>());
  virtual void addActorToPileAndUpdatePileView(VisualPhysicsActor* actor, Ogre::Vector3 offset_from_pile,
                                               Ogre::Quaternion original_orientation);

  virtual void addActorToPile(VisualPhysicsActor* actor,
                              Ogre::Vector3 offset_from_pile = Ogre::Vector3::ZERO,
                              Ogre::Quaternion original_orientation = Ogre::Quaternion::IDENTITY);
  virtual void stackViewOnInit();
  virtual void stackViewOnActorAddedOrRemoved();
  virtual void stackViewAndScaleChildren(Ogre::Real scale_factor);
  virtual void stackViewOnSortAlphabetically();
  virtual void moveDummyToBottom();

  virtual bool breakable();
  virtual void breakPile();
  virtual void breakPileWithoutConstrainingFinalPoses();
  virtual void breakItemsOutOfPile(QList<VisualPhysicsActorId> actor_ids);
  virtual QStringList pathsOfDescendants();
  virtual VisualPhysicsActorList children();
  virtual VisualPhysicsActorList flattenedChildren();
  virtual QList<VisualPhysicsActorId> flattenedChildrenIds();
  virtual QList<VisualPhysicsActorId> children_ids();
  virtual QList<Ogre::Vector3> children_offsets();
  virtual QList<Ogre::Quaternion> children_orientations();
  virtual void set_size(Ogre::Vector3 size);
  virtual Ogre::Vector3 size();
  virtual Ogre::Vector2 labelPositionForCurrentPosition();
  virtual void rename(QString new_name);
  virtual bool nameable();
  virtual QString display_name();
  virtual void update();
  virtual void set_position_no_physics(const Ogre::Vector3 &pos);
  virtual void set_label_visible(bool label_visible);
  virtual const Ogre::Vector3 world_position();
  virtual void set_display_name(QString display_name);
  virtual VisualPhysicsActorId adjacentActorOfChild(VisualPhysicsActorId child_id, ArrowKey arrow_key);
  virtual void revealChild(VisualPhysicsActorId child_id);
  virtual void launch();

  virtual void sortAlphabetically();

  virtual void set_selected(bool is_selected);
  virtual bool selected();
  virtual void writeToBuffer(VisualPhysicsActorBuffer* buffer);
  virtual bool serializable();
  virtual bool capture_mouse_events();
  virtual Ogre::Plane plane();

  VisualPhysicsActorType actor_type();

  virtual BumpTopCommandSet* supported_context_menu_items();
  virtual bool supportsContextMenuItem(BumpTopCommand* context_menu_option);

  virtual VisualPhysicsActor* lowest_child_with_visual_actor();
  virtual void set_render_queue_group(uint8 queue_id);
  virtual uint8 render_queue_group();
  virtual BumpBoxLabel* label();
  virtual void draggingExited();

  virtual Ogre::Vector2 actor_screen_position_before_drag();
  virtual BumpPose actor_pose_before_drag();
  virtual void updateActorStatusBeforeDrag();

  virtual BumpPose children_offset_pose(VisualPhysicsActor* actor);
  virtual void set_children_offset_pose(QHash<VisualPhysicsActorId, BumpPose> children_ids_to_offset_poses);

 public slots: // NOLINT
  virtual void mouseDown(MouseEvent* mouse_event);
  virtual void mouseDragged(MouseEvent* mouse_event);
  virtual void mouseUp(MouseEvent* mouse_event);
  virtual void mouseMoved(MouseEvent* mouse_event);
  virtual void rightMouseDown(MouseEvent* mouse_event);
  virtual void scrollWheel(MouseEvent* mouse_event);
  virtual void globalMouseDown(MouseEvent* mouse_event);
  void draggingEntered(MouseEvent* mouse_event);
  void draggingUpdated(MouseEvent* mouse_event);
  virtual void pileActorRemoved(VisualPhysicsActorId actor_id);
  virtual void pileActorChangedPhysically(VisualPhysicsActorId actor_id);
  virtual void updateLabel(Ogre::Real size_factor = -1);
  virtual void updateLabelPosition();
  virtual void actorAnimationComplete(VisualPhysicsActorAnimation* animation);
  virtual void breakActorBeingDraggedOutOfPile(VisualPhysicsActor* actor);
  virtual void labelClicked(MouseEvent* mouse_event);

 protected:
  virtual void removeActorFromPileHelper(VisualPhysicsActorId actor_id);
  virtual void addActorToParentHelper(VisualPhysicsActor* actor);
  virtual void breakPile(QHash<VisualPhysicsActorId, BumpPose> final_poses,
                         bool constrain_final_poses);
  virtual void resetQuickView();
  virtual QHash<VisualPhysicsActorId, BumpPose> actor_offsets_and_original_orientations();

  virtual std::string meshName();
  virtual Ogre::Vector3 absoluteMeshSizeDividedBy100();
  virtual btVector3 physicsSize();
  virtual void makePhysicsActor(bool physics_enabled);
  virtual void deleteLabel();
  virtual void set_label_visible_from_camera_position(bool visible);
  virtual void slideActorOut(VisualPhysicsActor* actor);
  virtual void slideActorIn(VisualPhysicsActor* actor);

  virtual void stackView(bool is_first_time, Ogre::Real scale_factor, Ogre::Real animation_duration);
  virtual Ogre::Real heightOfPile();
  virtual Ogre::Real heightOfPileForScaleFactor(Ogre::Real scale_factor);
  virtual Ogre::Real maximum_height();

  DampedSpringMouseHandler* mouse_handler_;
  BumpBoxLabel* label_;
  bool label_visible_;
  bool label_visible_from_camera_position_;
  Ogre::Real min_x_, max_x_;
  Ogre::Real last_mouse_x_;
  int quick_view_index_;
  Ogre::Real scroll_delta_;

  QString display_name_;
  Room* room_;
  HighlightActor* highlight_;
  bool is_selected_;
  BumpPileDropReceiver* drop_receiver_;
  bool is_performing_mouse_up_;
  bool should_delete_self_on_mouse_up_finished_;
  VisualPhysicsActor* exposed_actor_;

  QHash<VisualPhysicsActorId, VisualPhysicsActor*> pile_actors_;
  QList<VisualPhysicsActorId> pile_actor_ids_in_order_;
  QHash<VisualPhysicsActorId, Ogre::Vector3> original_actor_world_offsets_;
  QHash<VisualPhysicsActorId, Ogre::Quaternion> original_actor_world_orientations_;
  QHash<VisualPhysicsActorId, Ogre::Quaternion> actor_orientations_in_pile_;

  uint8 render_queue_group_on_mouse_down_;
  Ogre::Vector2 screen_position_before_drag_;
  BumpPose pose_before_drag_;
};

class BumpPileDropReceiver : public DropReceiver {
 public:
  explicit BumpPileDropReceiver(BumpPile* pile);

  virtual QString target_path();
  virtual bool prepareForDragOperation(Ogre::Vector2 mouse_in_window_space, QStringList list_of_files,
                                       NSDragOperation drag_operation,
                                       VisualPhysicsActorList list_of_actors = VisualPhysicsActorList());
  virtual bool performDragOperation(Ogre::Vector2 mouse_in_window_space, QStringList list_of_files,
                                    NSDragOperation drag_operation,
                                    VisualPhysicsActorList list_of_actors = VisualPhysicsActorList());
  virtual void concludeDragOperation();
  virtual void draggingExited();
 protected:
  BumpPile* pile_;
};


#endif  // BUMPTOP_BUMPPILE_H_
