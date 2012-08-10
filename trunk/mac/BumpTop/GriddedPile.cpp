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

#include "BumpTop/GriddedPile.h"

#include <string>
#include <utility>

#include "BumpTop/AnimationManager.h"
#include "BumpTop/AppSettings.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/BumpTopScene.h"
#include "BumpTop/BumpButton.h"
#include "BumpTop/BumpFlatSquare.h"
#include "BumpTop/BumpPile.h"
#include "BumpTop/FileItem.h"
#include "BumpTop/FileManager.h"
#include "BumpTop/HighlightActor.h"
#include "BumpTop/MaterialLoader.h"
#include "BumpTop/Math.h"
#include "BumpTop/MouseEventManager.h"
#include "BumpTop/OgreBulletConverter.h"
#include "BumpTop/PhysicsBoxActor.h"
#include "BumpTop/ProtocolBufferHelpers.h"
#include "BumpTop/QStringHelpers.h"
#include "BumpTop/UndoCommands/AddToPileUndoCommand.h"
#include "BumpTop/UndoCommands/InternalDragAndDropUndoCommand.h"
#include "BumpTop/UndoCommands/PileToGridUndoCommand.h"
#include "BumpTop/Room.h"
#include "BumpTop/RoomItemPoseConstraints.h"
#include "BumpTop/Shape.h"
#include "BumpTop/StickyNote.h"
#include "BumpTop/UndoRedoStack.h"
#include "BumpTop/VisualActor.h"
#include "BumpTop/VisualPhysicsActorAnimation.h"

using std::pair;

const size_t kNumActorsPerRow = 3;
const Ogre::Real kVertOffset = -30;
const Ogre::Real kVertSpacing = 30;
const Ogre::Real kHorizOffset = -30;
const Ogre::Real kHorizSpacing = 30;
const Ogre::Real kViewHeight = 90;

GriddedPile::GriddedPile(Ogre::SceneManager *scene_manager, Physics* physics, Room *room,
                         Ogre::Vector3 position_of_pile_before_gridded, VisualPhysicsActorId unique_id)
: BumpBox(scene_manager, physics, room, unique_id),
  scroll_position_(0.0),
  position_of_pile_before_gridded_(position_of_pile_before_gridded),
  index_of_gap_in_grid_(-1) {
}

GriddedPile::~GriddedPile() {
  room_->gridded_pile_manager()->unregister(this);
}

void GriddedPile::init() {
  BumpBox::init();

  loadBackground();
  room_->addActor(this);
  becomeAxisAligned();
  setCollisionsToGriddedPiles();
  set_physics_enabled(true);
  activatePhysics();

  set_orientation(Ogre::Quaternion::IDENTITY);
  set_size(Ogre::Vector3(5*kInitialActorSize, kGriddedPileThickness, 5*kInitialActorSize));

  ogre_scene_node_for_children_ = ogre_scene_node()->createChildSceneNode();

  close_button_ = new BumpButton(scene_manager_, physics_, ogre_scene_node());
  close_button_->initWithImages(FileManager::getResourcePath() + "/close_button.png",
                                FileManager::getResourcePath() + "/close_button_active.png");
  close_button_->set_size(Ogre::Vector3(30 / 4.8, 0, 30 / 4.8));
  close_button_->set_position(Ogre::Vector3(-45.0, 0.0, -45.0));
  close_button_->visual_actor()->set_to_render_after(visual_actor());
  assert(QObject::connect(close_button_, SIGNAL(onClicked()), this, SLOT(convertToPile())));

  scroll_up_button_ = new BumpButton(scene_manager_, physics_, ogre_scene_node());
  scroll_up_button_->initWithImages(FileManager::getResourcePath() + "/grid-scroll-up.png",
                                    FileManager::getResourcePath() + "/grid-scroll-up-inactive.png");
  scroll_up_button_->set_size(Ogre::Vector3(15/4.8, 0, 12/4.8));
  scroll_up_button_->set_position(Ogre::Vector3(42.0, 0.0, 38.0));
  scroll_up_button_->visual_actor()->set_to_render_after(visual_actor());
  assert(QObject::connect(scroll_up_button_, SIGNAL(onRenderTickWhileDepressed()), this, SLOT(scrollUp())));

  scroll_down_button_ = new BumpButton(scene_manager_, physics_, ogre_scene_node());
  scroll_down_button_->initWithImages(FileManager::getResourcePath() + "/grid-scroll-down.png",
                                      FileManager::getResourcePath() + "/grid-scroll-down-inactive.png");
  scroll_down_button_->set_size(Ogre::Vector3(15/4.8, 0, 12/4.8));
  scroll_down_button_->set_position(Ogre::Vector3(42.0, 0.0, 42.0));
  scroll_down_button_->visual_actor()->set_to_render_after(visual_actor());
  assert(QObject::connect(scroll_down_button_, SIGNAL(onRenderTickWhileDepressed()), this, SLOT(scrollDown())));

  stencil_region_for_actors_ = new ActorStencil(scene_manager_, visual_actor_->ogre_scene_node());
  stencil_region_for_actors_->set_scale(Ogre::Vector3(kViewHeight + 0.5));

  room_->gridded_pile_manager()->registerAndChangeMyPosition(this);

  if (drop_receiver_ != NULL) {
    delete drop_receiver_;
  }
  drop_receiver_ = new GriddedPileDropReceiver(this);
}

void GriddedPile::scrollUp() {
  scroll(1.0);
}

void GriddedPile::scrollDown() {
  scroll(-1.0);
}

void GriddedPile::scroll(Ogre::Real scroll_delta) {
  scroll_position_ += scroll_delta;
  Ogre::Real top_scroll_position = 0;
  Ogre::Real bottom_scroll_position = std::min(0.0, kViewHeight + 1 - kVertSpacing*(ceil(pile_actor_ids_.size() / 3.0)));
  scroll_position_ = std::min(top_scroll_position, scroll_position_);
  scroll_position_ = std::max(bottom_scroll_position, scroll_position_);

  if (!scroll_down_button_->disabled() && bottom_scroll_position >= 0) {
    scroll_up_button_->set_disabled(true);
    scroll_down_button_->set_disabled(true);
  } else if (scroll_down_button_->disabled() && bottom_scroll_position < 0) {
    scroll_up_button_->set_disabled(false);
    scroll_down_button_->set_disabled(false);
  }

  ogre_scene_node_for_children_->setPosition(0, 0, scroll_position_);
  update();
}

void GriddedPile::revealChild(VisualPhysicsActorId child_id) {
  if (!pile_actor_ids_.contains(child_id)) {
    return;
  }

  Ogre::Real position = -kVertSpacing*(floor(pile_actor_ids_.indexOf(child_id) / 3.0));
  if (position < scroll_position_ - (kViewHeight - kVertSpacing)) {
    scroll(position - scroll_position_ + (kViewHeight - kVertSpacing));
  } else if (position > scroll_position_) {
    scroll(position - scroll_position_);
  }
}

VisualPhysicsActorId GriddedPile::adjacentActorOfChild(VisualPhysicsActorId child_id, ArrowKey arrow_key) {
  int index = pile_actor_ids_.indexOf(child_id);
  int count = pile_actor_ids_.count();
  switch (arrow_key) {
    case ARROW_UP:
      if (index < 3)
        return 0;
      return pile_actor_ids_.at(index - 3);
    case ARROW_DOWN:
      if (count - index < 4)
        return 0;
      return pile_actor_ids_.at(index + 3);
    case ARROW_LEFT:
      if (index % 3 == 0)
        return 0;
      return pile_actor_ids_.at(index - 1);
    case ARROW_RIGHT:
      if (index % 3 == 2 || index == count - 1)
        return 0;
      return pile_actor_ids_.at(index + 1);
    default:
      return 0;
  }
}

void GriddedPile::update() {
  // TODO: there might be some redundancy here
  // updating this scene node recursively so text labels don't have a weird lag behind the actors themselves
  visual_actor()->ogre_scene_node()->_update(true, true);
  BumpBox::update();
}

void GriddedPile::launch() {
  // do nothing
}

void GriddedPile::closeView() {
  convertToPile();
}

Ogre::SceneNode* GriddedPile::ogre_scene_node_for_children() {
  return ogre_scene_node_for_children_;
}

void GriddedPile::addActors(VisualPhysicsActorList actors,
                            QList<Ogre::Vector3> offsets, QList<Ogre::Quaternion> orientations) {
  assert(actors.size() == offsets.size() && actors.size() == orientations.size());
  for (int i = 0; i < actors.size(); i++) {
    VisualPhysicsActor* actor = actors.value(i);
    Ogre::Vector3 offset = offsets.value(i);
    Ogre::Quaternion orientation = orientations.value(i);
    addActorToPile(actor, false, offset, orientation);
  }
  layoutGriddedPile();
}

void GriddedPile::addActorToPile(VisualPhysicsActor* actor, bool relayout,
                                 Ogre::Vector3 offset_from_pile, Ogre::Quaternion orientation) {
  offset_from_pile.y = 0;
  original_actor_world_offsets_.insert(actor->unique_id(), offset_from_pile);
  original_actor_world_orientations_.insert(actor->unique_id(), orientation);
  original_actor_sizes_.insert(actor->unique_id(), actor->destinationScale());
  AnimationManager::singleton()->endAnimationsForActor(actor, AnimationManager::STOP_AT_CURRENT_STATE);
  actor->set_size(Ogre::Vector3(kInitialActorSize, kInitialActorSize, kInitialActorSize));
  actor->set_parent(this);
  actor->set_physics_enabled(false);
  actor->set_selected(false);
  actor->visual_actor()->set_inherit_scale(false);
  actor->set_selected(false);
  actor->set_label_visible(true);
  actor->set_render_queue_group(render_queue_group() + 2);

  assert(QObject::connect(actor, SIGNAL(onRemoved(VisualPhysicsActorId)),  // NOLINT
                          this, SLOT(pileActorRemoved(VisualPhysicsActorId))));  // NOLINT

  if (ids_of_actors_that_we_are_saving_a_spot_for_.contains(actor->unique_id())) {
    pile_actor_ids_.insert(indices_of_gaps_in_grid_[0], actor->unique_id());
    indices_of_gaps_in_grid_.removeAt(0);
    ids_of_actors_that_we_are_saving_a_spot_for_.removeAll(actor->unique_id());
  } else {
    pile_actor_ids_.push_back(actor->unique_id());
  }
  pile_actors_.insert(actor->unique_id(), actor);
  room_->removeActor(actor->unique_id());

  if (relayout)
    layoutGriddedPile();

  scroll(0);  // just to force scroll-related stuff to update
}

void GriddedPile::layoutGriddedPile() {
  int i = 0;
  for_each(VisualPhysicsActorId actor_id, pile_actor_ids_) {
    VisualPhysicsActor* actor = pile_actors_[actor_id];

    if (indices_of_gaps_in_grid_.count() > 0 && indices_of_gaps_in_grid_[0] == i) {
      i++;
    }

    VisualPhysicsActorAnimation* actor_animation;
    AnimationManager::singleton()->endAnimationsForActor(actor, AnimationManager::STOP_AT_CURRENT_STATE);

    actor_animation = new VisualPhysicsActorAnimation(actor,
                                                      400,
                                                      Ogre::Vector3(kHorizOffset + kHorizSpacing*(i % kNumActorsPerRow),
                                                                    0,
                                                                    kVertOffset + kVertSpacing*(i/kNumActorsPerRow)),
                                                      Ogre::Quaternion::IDENTITY);
    actor_animation->start();
    i++;
  }
  scroll(0);  // just to force scroll-related stuff to update
}

void GriddedPile::convertToPile() {
  GridToPileUndoCommand* grid_to_pile_command = new GridToPileUndoCommand(unique_id(), room_, scene_manager_, physics_);
  room_->updateCurrentState();
  room_->undo_redo_stack()->push(grid_to_pile_command, room_->current_state());
  // this deletes this
}

bool GriddedPile::selected() {
  return false;
}

void GriddedPile::breakPile() {
  VisualPhysicsActorAnimation* actor_animation;

  if (pile_actor_ids_.count() == 1) {
    // This pile is being broken because its items have been removed externally
    // In this case we don't want to animate the break, and we want to just leave the
    // item unmoved.
    room_->removeActor(unique_id_);
    physics_actor_->set_physics_enabled(false);
    for_each(VisualPhysicsActor* actor, pile_actors_.values()) {
      AnimationManager::singleton()->endAnimationsForActor(actor, AnimationManager::STOP_AT_CURRENT_STATE);
      if (parent_ != NULL) {
        parent_->addActorToPileAndUpdatePileView(actor,
                                                 original_actor_world_offsets_[actor->unique_id()],
                                                 original_actor_world_orientations_[actor->unique_id()]);
      } else {
        actor->set_physics_enabled(true);
        actor->set_selected(false);
        actor->set_parent(parent_);
        actor->set_label_visible(!actor->name_hidden());
        actor->set_render_queue_group(Ogre::RENDER_QUEUE_MAIN);
        actor->visual_actor()->set_inherit_scale(true);
        actor->set_size(original_actor_sizes_[actor->unique_id()]);
        room_->addActor(actor);
      }
    }
  } else {
    // Return all pile members to the room, and determine their new position in the room
    // based on their position relative to the current pile position

    QHash<VisualPhysicsActorId, BumpPose> desired_poses = QHash<VisualPhysicsActorId, BumpPose>();
    for_each(VisualPhysicsActorId actor_id, pile_actor_ids_) {
      BumpPose pose = BumpPose(original_actor_world_offsets_[actor_id] + position(),
                               original_actor_world_orientations_[actor_id]);
      desired_poses.insert(actor_id, pose);
      room_->addActor(pile_actors_[actor_id]);
    }
    // Remove the pile from the room.
    room_->removeActor(unique_id_);
    physics_actor_->set_physics_enabled(false);

    for_each(VisualPhysicsActor* actor, pile_actors_.values()) {
      AnimationManager::singleton()->endAnimationsForActor(actor, AnimationManager::STOP_AT_CURRENT_STATE);
      actor->set_parent(parent_);
      actor->set_label_visible(true);
      actor->set_selected(false);
      actor->set_render_queue_group(Ogre::RENDER_QUEUE_MAIN);
      actor->visual_actor()->set_inherit_scale(true);
      actor->set_size(original_actor_sizes_[actor->unique_id()]);

      actor->set_physics_enabled(true);
      actor_animation = new VisualPhysicsActorAnimation(actor, 400,
                                                        desired_poses[actor->unique_id()].position,
                                                        desired_poses[actor->unique_id()].orientation);
      actor_animation->start();
      actor->set_selected(true);
    }
  }
}

bool GriddedPile::pinnable() {
  return false;
}

VisualPhysicsActorList GriddedPile::children() {
  VisualPhysicsActorList pile_actors_in_order;
  for_each(VisualPhysicsActorId actor_id, pile_actor_ids_) {
    pile_actors_in_order.push_front(pile_actors_[actor_id]);
  }
  return pile_actors_in_order;
}

VisualPhysicsActorList GriddedPile::flattenedChildren() {
  VisualPhysicsActorList children = GriddedPile::children();
  VisualPhysicsActorList flattened_children;
  for_each(VisualPhysicsActor* child, children) {
    if (child->children().count() > 0) {
      VisualPhysicsActorList childs_flattened_children = child->flattenedChildren();
      for_each(VisualPhysicsActor* grandchild, childs_flattened_children)
        flattened_children.push_back(grandchild);
    } else {
      flattened_children.push_back(child);
    }
  }
  return flattened_children;
}

QList<VisualPhysicsActorId> GriddedPile::flattenedChildrenIds() {
  QList<VisualPhysicsActorId> flattened_children_ids;
  for_each(VisualPhysicsActor* actor, flattenedChildren()) {
    flattened_children_ids.push_back(actor->unique_id());
  }
  return flattened_children_ids;
}

QList<Ogre::Vector3> GriddedPile::children_offsets() {
  QList<Ogre::Vector3> offsets;
  for_each(VisualPhysicsActor* actor, children()) {
    offsets.append(original_actor_world_offsets_[actor->unique_id()]);
  }
  return offsets;
}

QList<Ogre::Quaternion> GriddedPile::children_orientations() {
  QList<Ogre::Quaternion> orientations;
  for_each(VisualPhysicsActor* actor, children()) {
    orientations.append(original_actor_world_orientations_[actor->unique_id()]);
  }
  return orientations;
}

std::string GriddedPile::meshName() {
  return Shape::singleton()->flat_square();
}

btVector3 GriddedPile::physicsSize() {
  return btVector3(1.0, 0.125, 1.0);
}

Ogre::Vector3 GriddedPile::absoluteMeshSizeDividedBy100() {
  return Ogre::Vector3(1.0, 0, 1.0);
}


btScalar GriddedPile::mass() {
  return 1.0;
}

VisualPhysicsActorType GriddedPile::actor_type() {
  return GRIDDED_PILE;
}

bool GriddedPile::capture_mouse_events() {
  return true;
}

void GriddedPile::rightMouseDown(MouseEvent* mouse_event) {
  if (isMouseEventForChildOutsideOfGrid(mouse_event)) {
    mouse_event->cancelled = true;
  } else if (!mouse_event->capture) {
    BumpBox::rightMouseDown(mouse_event);
  }
}

void GriddedPile::mouseDown(MouseEvent* mouse_event) {
  if (isMouseEventForChildOutsideOfGrid(mouse_event)) {
    mouse_event->cancelled = true;
  } else if (mouse_event->capture && mouse_event->num_clicks == 2) {
  // do nothing, let child deal with double-click event
  } else {
    if (mouse_event->capture) {
      for_each(VisualPhysicsActor* actor, pile_actors_.values()) {
        if (actor->visual_actor() == mouse_event->item) {
          assert(QObject::connect(actor, SIGNAL(onMouseMotionRegistered(VisualPhysicsActor*)),  // NOLINT
                                  this, SLOT(breakActorBeingDraggedOutOfGriddedPile(VisualPhysicsActor*))));  // NOLINT
          break;
        }
      }
      return;
    }
    BumpBox::mouseDown(mouse_event);
    // TODO: I shouldn't inherit from BumpBox anymore
    set_render_queue_group(render_queue_group_on_mouse_down_);
  }
}

void GriddedPile::mouseDragged(MouseEvent* mouse_event) {
  if (isMouseEventForChildOutsideOfGrid(mouse_event)) {
    mouse_event->cancelled = true;
  } else {
    if (mouse_event->capture) {
      return;
    }
    BumpBox::mouseDragged(mouse_event);
  }
}

void GriddedPile::mouseUp(MouseEvent* mouse_event) {
  if (isMouseEventForChildOutsideOfGrid(mouse_event)) {
    mouse_event->cancelled = true;
  } else {
    if (mouse_event->capture) {
      return;
    }
    BumpBox::mouseUp(mouse_event);
  }
}

// This method is only used for dragging out and not generally for breaking the gridded pile
void GriddedPile::breakActorBeingDraggedOutOfGriddedPile(VisualPhysicsActor* actor) {
  actor->updateActorParentInfoBeforeDrag();
  actor->updateActorOffsetPoseToItsParentBeforeDrag();
  actor->updateActorSiblingOffsetPoseToParentBeforeDrag();
  actor->set_parent(NULL);
  // by placing set_physics_enabled to true _after_ setting the parent, I avoid a strange
  // problem where the physics locks up every once in a while. This is likely a consequence
  // of the fact that when we have a parent, our positions are relative, and our physics
  // sync with Ogre hasn't been setup to use relative positioning yet
  actor->set_physics_enabled(true);
  actor->set_label_visible(true);
  actor->visual_actor()->set_inherit_scale(true);
  actor->set_render_queue_group_for_mouse_up(Ogre::RENDER_QUEUE_MAIN);
  actor->set_room_surface(room_->getSurface(FLOOR));
  room_->addActor(actor);
  ids_of_actors_that_we_are_saving_a_spot_for_.clear();
  ids_of_actors_that_we_are_saving_a_spot_for_.push_back(actor->unique_id());
  indices_of_gaps_in_grid_.push_back(pile_actor_ids_.indexOf(actor->unique_id()));
  removeActorFromPileHelper(actor->unique_id());
}

void GriddedPile::addActorToParentHelper(VisualPhysicsActor* actor) {
  if (parent_ != NULL) {
    parent_->addActorToPileAndUpdatePileView(actor,
                                             original_actor_world_offsets_[actor->unique_id()],
                                             original_actor_world_orientations_[actor->unique_id()]);
  } else {
    // it's in the room

    // ending the animation up here because set_parent will end the animation and move the actor to its final position
    // which we don't want
    AnimationManager::singleton()->endAnimationsForActor(actor, AnimationManager::STOP_AT_CURRENT_STATE);
    actor->set_physics_enabled(true);
    actor->set_selected(false);
    actor->set_parent(parent_);
    actor->set_render_queue_group(Ogre::RENDER_QUEUE_MAIN);
    actor->set_label_visible(true);
    room_->addActor(actor);
  }
}

void GriddedPile::breakItemsOutOfPile(QList<VisualPhysicsActorId> actor_ids) {
  QHash<VisualPhysicsActorId, BumpPose> final_poses;
  for_each(VisualPhysicsActorId actor_id, actor_ids) {
    if (pile_actors_.contains(actor_id)) {
      VisualPhysicsActor* actor = pile_actors_[actor_id];
      addActorToParentHelper(actor);

      BumpPose pose = BumpPose(original_actor_world_offsets_[actor_id] + position(),
                               original_actor_world_orientations_[actor_id]);

      final_poses.insert(actor_id, pose);
    }
  }

  // Determine the constrained poses of the pile items before animating
  QHash<VisualPhysicsActorId, BumpPose> constrained_poses = getActorPosesConstrainedToRoom(final_poses, room_);
  constrained_poses = getActorPosesConstrainedToNoIntersections(constrained_poses, room_);
  final_poses = constrained_poses;

  for_each(VisualPhysicsActorId actor_id, actor_ids) {
    if (pile_actors_.contains(actor_id)) {
      VisualPhysicsActor* actor = pile_actors_[actor_id];

      removeActorFromPileHelper(actor_id);

      AnimationManager::singleton()->endAnimationsForActor(actor, AnimationManager::STOP_AT_CURRENT_STATE);
      VisualPhysicsActorAnimation* actor_animation;
      actor_animation = new VisualPhysicsActorAnimation(actor, 325,
                                                        final_poses[actor->unique_id()].position,
                                                        final_poses[actor->unique_id()].orientation);
      actor_animation->start();
    }
  }
}

void GriddedPile::scrollWheel(MouseEvent* mouse_event) {
  Ogre::Real magic_scroll_factor = 1.0;  // tweaked until felt good, subjectively
  scroll(mouse_event->delta_y / magic_scroll_factor);
}

void GriddedPile::draggingEntered(MouseEvent* mouse_event) {
  draggingUpdated(mouse_event);
}

void GriddedPile::draggingUpdated(MouseEvent* mouse_event) {
  if (mouse_event->items_being_dropped.size() != 0) {
    for_each(VisualPhysicsActor* actor, mouse_event->items_being_dropped) {
      if (actor == this) {
        return;
      }
    }

    if (mouse_event->drag_operations & NSDragOperationMove) {
      mouse_event->drag_operations = NSDragOperationMove;
      mouse_event->handled = true;
      mouse_event->drop_receiver = drop_receiver_;
    }
    ids_of_actors_that_we_are_saving_a_spot_for_.clear();
    for_each(VisualPhysicsActor* actor, mouse_event->items_being_dropped) {
      ids_of_actors_that_we_are_saving_a_spot_for_.push_back(actor->unique_id());
    }

    Ogre::Matrix4 world_transform = ogre_scene_node_for_children_->_getFullTransform();

    assert(world_transform.isAffine());
    if (world_transform.isAffine()) {
      Ogre::Matrix4 world_transform_inverse = world_transform.inverseAffine();
      Ogre::Vector3 stab_point_object_space = world_transform_inverse*mouse_event->mouse_in_world_space;

      int vertical_pos = (stab_point_object_space.z - kVertOffset + kVertSpacing/2) / kVertSpacing;
      vertical_pos = MAX(0, MIN(vertical_pos, pile_actors_.size()/kNumActorsPerRow));
      int horizontal_pos = (stab_point_object_space.x - kHorizOffset + kHorizSpacing/2) / kHorizSpacing;
      horizontal_pos = MAX(0, MIN(horizontal_pos, kNumActorsPerRow - 1));
      size_t index = vertical_pos*kNumActorsPerRow + horizontal_pos;
      if (indices_of_gaps_in_grid_.count() == 0 || indices_of_gaps_in_grid_[0] != index) {
        indices_of_gaps_in_grid_.clear();
        for (int i = 0; i < mouse_event->items_being_dropped.count(); i++) {
          indices_of_gaps_in_grid_.push_back(index);
          index++;
        }
        // TODO: assert that it's within the length of the grid
        layoutGriddedPile();
        emit onSizeChanged(unique_id_);
      }
    }
  }
}

void GriddedPile::draggingExited() {
  indices_of_gaps_in_grid_.clear();
  ids_of_actors_that_we_are_saving_a_spot_for_.clear();
  layoutGriddedPile();
}

void GriddedPile::set_selected(bool selected) {
}

bool GriddedPile::isMouseEventForChildOutsideOfGrid(MouseEvent* mouse_event) {
  if (mouse_event->capture && !mouse_event->global_capture) {
    MouseEventManager *mouse_manager = BumpTopApp::singleton()->mouse_event_manager();
    pair<bool, Ogre::Vector3> intersection = mouse_manager->intersectWithEntityMesh(visual_actor()->_entity(),
                                                                                    mouse_event->mouse_in_window_space);
    return !intersection.first;
  }
  return false;
}

void GriddedPile::loadBackground() {
  set_material_name(AppSettings::singleton()->global_material_name(GRIDDED_PILE_BACKGROUND));
}

void GriddedPile::addActorToPileAndUpdatePileView(VisualPhysicsActor* actor,
                                                  Ogre::Vector3 offset_from_pile,
                                                  Ogre::Quaternion orientation) {
  addActorToPile(actor, true, offset_from_pile, orientation);
}

void GriddedPile::set_render_queue_group(uint8 queue_id) {
  if (queue_id != render_queue_group()) {
    BumpBox::set_render_queue_group(queue_id);
    close_button_->set_render_queue_group(queue_id);
    scroll_up_button_->set_render_queue_group(queue_id);
    scroll_down_button_->set_render_queue_group(queue_id);
    stencil_region_for_actors_->set_render_queue_group(queue_id + 1);
    for_each(VisualPhysicsActor *actor, pile_actors_) {
      actor->set_render_queue_group(queue_id + 2);
    }
  }
}

void GriddedPile::setMaterialNameAndDeleteMaterialLoader(MaterialLoader *material_loader) {
  set_material_name(material_loader->name());
  delete material_loader;
}

void GriddedPile::initWithActors(VisualPhysicsActorList actors, QList<Ogre::Vector3> offsets,
                                 QList<Ogre::Quaternion> orientations, Ogre::Vector3 position) {
  init();
  addActors(actors, offsets, orientations);

  set_position(position);
  set_initial_position(position);
}

bool GriddedPile::initFromBuffer(VisualPhysicsActorBuffer* buffer,  bool physics_enabled) {
  // return immediately if the size of children is 0-- this happened when loading a scene file once
  init();

  if (buffer->child_size() == 0 && !buffer->is_new_items_pile())
    return false;

  if (buffer->has_display_name()) {
    display_name_ = QStringFromUtf8(buffer->display_name());
  }

  Ogre::Vector3 pile_position =  Vector3BufferToVector3(buffer->position());
  QList<Ogre::Vector3> child_offsets;
  QList<Ogre::Quaternion> child_original_orientations;

  VisualPhysicsActor* new_actor;
  VisualPhysicsActorList actors;
  // Creating pile's children
  for (int i = 0; i < buffer->child_size(); i++) {
    if (buffer->mutable_child(i)->actor_type() == BUMP_BOX) {
      new_actor = new BumpFlatSquare(scene_manager_, physics_, room_);
    } else if (buffer->mutable_child(i)->actor_type() == BUMP_PILE) {
      new_actor = new BumpPile(scene_manager_, physics_, room_);
    } else if (buffer->mutable_child(i)->actor_type() == STICKY_NOTE) {
      new_actor = new StickyNote(scene_manager_, physics_, room_);
    } else {
      continue;
    }

    if (new_actor->initFromBuffer(buffer->mutable_child(i))) {
      child_original_orientations.push_back(QuaternionBufferToQuaternion(buffer->mutable_child_orientation(i)));
      child_offsets.push_back(Vector3BufferToVector3(buffer->mutable_child_position(i)) - pile_position);
      new_actor->set_position(Vector3BufferToVector3(buffer->mutable_child_position(i)));
      new_actor->set_orientation(QuaternionBufferToQuaternion(buffer->mutable_child_orientation(i)));
      actors.push_back(new_actor);
    } else {
      delete new_actor;
    }
  }
  if (actors.size() == 0 && !buffer->is_new_items_pile()) {
    return false;
  }

  position_of_pile_before_gridded_ = pile_position;
  set_position(pile_position);
  set_orientation(QuaternionBufferToQuaternion(buffer->orientation()));

  addActors(actors, child_offsets, child_original_orientations);

  std::pair<bool, Ogre::Vector3> adjusted_position;
  adjusted_position = getPositionConstrainedToRoom(this->world_bounding_box(), room_);

  if (adjusted_position.first) {
    set_position(adjusted_position.second);
  }
  set_initial_position(position());

  room_->addActor(this);

  return true;
}

void GriddedPile::writeToBuffer(VisualPhysicsActorBuffer* buffer) {
  buffer->set_actor_type(actor_type());
  QuaternionToBuffer(orientation(), buffer->mutable_orientation());

  buffer->clear_child();
  buffer->clear_child_position();
  buffer->clear_child_orientation();
  buffer->clear_position();

  Vector3ToBuffer(position(), buffer->mutable_position());
  buffer->set_display_name(utf8(display_name_));

  for_each(VisualPhysicsActorId actor_id, pile_actor_ids_) {
    VisualPhysicsActor* actor = pile_actors_[actor_id];
    if (actor->serializable()) {
      actor->writeToBuffer(buffer->add_child());
      Vector3ToBuffer(original_actor_world_offsets_[actor_id] + position(),
                      buffer->add_child_position());
      QuaternionToBuffer(original_actor_world_orientations_[actor_id], buffer->add_child_orientation());
    }
  }

  if (unique_id_ == room_->new_items_pile()) {
    buffer->set_is_new_items_pile(true);
  }
}

void GriddedPile::removeActorFromPileHelper(VisualPhysicsActorId actor_id) {
  if (pile_actors_.contains(actor_id)) {
    pile_actors_[actor_id]->disconnect(this);
  }
  pile_actor_ids_.removeAll(actor_id);
  pile_actors_.remove(actor_id);
  if (pile_actors_.count() > 1 || is_new_items_pile()) {
    layoutGriddedPile();
    emit onSizeChanged(unique_id_);
  } else {
    if (parent_ != NULL) {
      breakPile();
      emit onRemoved(unique_id_);
    } else {
      breakPile();
      delete this;
    }
  }
}

void GriddedPile::pileActorRemoved(VisualPhysicsActorId actor_id) {
  VisualPhysicsActor* actor_to_remove = NULL;
  if (pile_actors_.contains(actor_id))
    actor_to_remove = pile_actors_[actor_id];
  removeActorFromPileHelper(actor_id);
  if (actor_to_remove != NULL)
    delete actor_to_remove;
}

bool GriddedPile::is_new_items_pile() {
  return unique_id() == room_->new_items_pile();
}

void GriddedPile::setPhysicsConstraintsForSurface(RoomSurface* surface) {
}

void GriddedPile::becomeAxisAligned() {
  Ogre::Vector3 angular_lower_limit, angular_upper_limit;
  angular_lower_limit = Ogre::Vector3(0, 0, 0);
  angular_upper_limit = Ogre::Vector3(0, 0, 0);

  physics_actor_->set6DofConstraint(Ogre::Vector3(0, 0, 0), Ogre::Vector3(-1, -1, -1),
                                    angular_lower_limit, angular_upper_limit);
}

QString GriddedPile::display_name() {
  return display_name_;
}

void GriddedPile::set_display_name(QString display_name) {
  display_name_ = display_name;
}

Ogre::Vector3 GriddedPile::position_of_pile_before_gridded() {
  return position_of_pile_before_gridded_;
}

void GriddedPile::set_initial_position(const Ogre::Vector3 &pos) {
  initial_position_ = pos;
}

Ogre::Vector3 GriddedPile::initial_position() {
  return initial_position_;
}

BumpPose GriddedPile::children_offset_pose(VisualPhysicsActor* actor) {
  if (flattenedChildren().contains(actor)) {
    return BumpPose(original_actor_world_offsets_[actor->unique_id()],
                    original_actor_world_orientations_[actor->unique_id()]);
  } else {
    return BumpPose(Ogre::Vector3::ZERO, Ogre::Quaternion::IDENTITY);
  }
}

QString GriddedPileDropReceiver::target_path() {
  return pile_->path();
}

GriddedPileDropReceiver::GriddedPileDropReceiver(GriddedPile* pile)
: pile_(pile) {
}

bool GriddedPileDropReceiver::prepareForDragOperation(Ogre::Vector2 mouse_in_window_space, QStringList list_of_files,
                                                   NSDragOperation drag_operation,
                                                   VisualPhysicsActorList list_of_actors) {
  return drag_operation & NSDragOperationMove;
}

bool GriddedPileDropReceiver::performDragOperation(Ogre::Vector2 mouse_in_window_space, QStringList list_of_files,
                                                NSDragOperation drag_operation,
                                                VisualPhysicsActorList list_of_actors) {
  Room* room = BumpTopApp::singleton()->scene()->room();
  InternalDragAndDropUndoCommand* drag_and_drop_undo_command = new InternalDragAndDropUndoCommand(list_of_actors,
                                                                                                  pile_->unique_id(),
                                                                                                  room);
  room->undo_redo_stack()->push(drag_and_drop_undo_command, room->current_state(), true);
  return true;
}

void GriddedPileDropReceiver::concludeDragOperation() {
}

void GriddedPileDropReceiver::draggingExited() {
  pile_->draggingExited();
}

ActorStencil::ActorStencil(Ogre::SceneManager *scene_manager, Ogre::SceneNode *parent_ogre_scene_node)
: VisualActor(scene_manager, parent_ogre_scene_node, Shape::singleton()->flat_square(), false) {
  init();
  set_material_name(AppSettings::singleton()->global_material_name(TRANSPARENT_PIXEL));
}

void ActorStencil::deleteMaterialLoader(MaterialLoader *material_loader) {
  delete material_loader;
}

#include "moc/moc_GriddedPile.cpp"
