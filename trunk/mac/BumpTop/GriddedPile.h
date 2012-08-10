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

#ifndef BUMPTOP_GRIDDEDPILE_H_
#define BUMPTOP_GRIDDEDPILE_H_

#include <string>

#include "BumpTop/BumpBox.h"
#include "BumpTop/VisualActor.h"

class ActorStencil;
class BumpButton;
class HighlightActor;
class MaterialLoader;

const Ogre::Real kGriddedPileThickness = 1.0;

class  GriddedPile : public BumpBox {
  Q_OBJECT

 public:
  explicit GriddedPile(Ogre::SceneManager *scene_manager, Physics* physics, Room *room,
                       Ogre::Vector3 position_of_pile_before_gridded, VisualPhysicsActorId unique_id = 0);
  virtual ~GriddedPile();

  virtual void init();
  virtual bool initFromBuffer(VisualPhysicsActorBuffer* buffer,  bool physics_enabled);
  virtual void initWithActors(VisualPhysicsActorList actors, QList<Ogre::Vector3> offsets,
                              QList<Ogre::Quaternion> orientations, Ogre::Vector3 position);
  virtual void writeToBuffer(VisualPhysicsActorBuffer* buffer);

  virtual void addActors(VisualPhysicsActorList actors,
                         QList<Ogre::Vector3> offsets = QList<Ogre::Vector3>(),
                         QList<Ogre::Quaternion> orientations = QList<Ogre::Quaternion>());
  virtual bool pinnable();
  virtual VisualPhysicsActorList children();
  virtual VisualPhysicsActorList flattenedChildren();
  virtual QList<VisualPhysicsActorId> flattenedChildrenIds();
  virtual QList<Ogre::Vector3> children_offsets();
  virtual QList<Ogre::Quaternion> children_orientations();

  virtual void setPhysicsConstraintsForSurface(RoomSurface* surface);

  virtual btScalar mass();
  virtual VisualPhysicsActorType actor_type();

  virtual bool capture_mouse_events();
  virtual void rightMouseDown(MouseEvent* mouse_event);
  virtual void mouseDown(MouseEvent* mouse_event);
  virtual void mouseDragged(MouseEvent* mouse_event);
  virtual void mouseUp(MouseEvent* mouse_event);
  virtual void scrollWheel(MouseEvent* mouse_event);
  virtual void revealChild(VisualPhysicsActorId child_id);
  virtual VisualPhysicsActorId adjacentActorOfChild(VisualPhysicsActorId child_id, ArrowKey arrow_key);
  virtual void breakItemsOutOfPile(QList<VisualPhysicsActorId> actor_ids);


  virtual void addActorToPileAndUpdatePileView(VisualPhysicsActor* actor,
                                               Ogre::Vector3 offset_from_pile,
                                               Ogre::Quaternion orientation);
  virtual void set_render_queue_group(uint8 queue_id);
  virtual bool selected();
  virtual Ogre::SceneNode* ogre_scene_node_for_children();
  virtual void update();
  void launch();
  virtual void closeView();
  virtual QString display_name();
  virtual void set_display_name(QString display_name);
  virtual Ogre::Vector3 position_of_pile_before_gridded();
  virtual void set_initial_position(const Ogre::Vector3 &pos);
  virtual Ogre::Vector3 initial_position();

  virtual void draggingExited();
  virtual void set_selected(bool selected);

  virtual BumpPose children_offset_pose(VisualPhysicsActor* actor);

  virtual bool is_new_items_pile();

 public slots:  // NOLINT
  void setMaterialNameAndDeleteMaterialLoader(MaterialLoader *material_loader);
  virtual void breakPile();
  void convertToPile();
  void scrollUp();
  void scrollDown();
  virtual void pileActorRemoved(VisualPhysicsActorId actor_id);
  virtual void draggingEntered(MouseEvent* mouse_event);
  virtual void draggingUpdated(MouseEvent* mouse_event);
  void breakActorBeingDraggedOutOfGriddedPile(VisualPhysicsActor* actor);

 protected:
  void removeActorFromPileHelper(VisualPhysicsActorId actor_id);
  void addActorToParentHelper(VisualPhysicsActor* actor);
  void scroll(Ogre::Real scroll_delta);
  virtual std::string meshName();
  virtual Ogre::Vector3 absoluteMeshSizeDividedBy100();
  virtual btVector3 physicsSize();
  void loadBackground();
  void becomeAxisAligned();
  void layoutGriddedPile();
  void addActorToPile(VisualPhysicsActor* actor, bool relayout,
                      Ogre::Vector3 offset_from_pile = Ogre::Vector3::ZERO,
                      Ogre::Quaternion orientation = Ogre::Quaternion::IDENTITY);
  bool isMouseEventForChildOutsideOfGrid(MouseEvent* mouse_event);

  QList<VisualPhysicsActorId> pile_actor_ids_;
  QHash<VisualPhysicsActorId, VisualPhysicsActor*> pile_actors_;
  QHash<VisualPhysicsActorId, Ogre::Vector3> original_actor_world_offsets_;
  QHash<VisualPhysicsActorId, Ogre::Quaternion> original_actor_world_orientations_;
  QHash<VisualPhysicsActorId, Ogre::Vector3> original_actor_sizes_;

  QString display_name_;
  BumpButton *close_button_;
  BumpButton *scroll_up_button_;
  BumpButton *scroll_down_button_;
  Ogre::Real scroll_position_;
  Ogre::SceneNode *ogre_scene_node_for_children_;
  Ogre::Vector3 position_of_pile_before_gridded_;
  Ogre::Vector3 initial_position_;
  QList<int> indices_of_gaps_in_grid_;
  int index_of_gap_in_grid_;
  QList<VisualPhysicsActorId> ids_of_actors_that_we_are_saving_a_spot_for_;
  ActorStencil* stencil_region_for_actors_;
};

class GriddedPileDropReceiver : public DropReceiver {
 public:
  explicit GriddedPileDropReceiver(GriddedPile* pile);

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
  GriddedPile* pile_;
};

class ActorStencil : public VisualActor {
  Q_OBJECT
 public:
  explicit ActorStencil(Ogre::SceneManager *scene_manager, Ogre::SceneNode *parent_ogre_scene_node);

 public slots:  // NOLINT
  void deleteMaterialLoader(MaterialLoader *material_loader);
};

#endif  // BUMPTOP_GRIDDEDPILE_H_
