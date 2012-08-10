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

#include "BumpTop/BumpPile.h"

#include <string>

#include "BumpTop/AnimationManager.h"
#include "BumpTop/AppSettings.h"
#include "BumpTop/Authorization.h"
#include "BumpTop/BumpBoxLabel.h"
#include "BumpTop/BumpFlatSquare.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/BumpTopScene.h"
#include "BumpTop/DebugAssert.h"
#include "BumpTop/OSX/ContextMenu.h"
#include "BumpTop/DampedSpringMouseHandler.h"
#include "BumpTop/OSX/EventModifierFlags.h"
#include "BumpTop/HighlightActor.h"
#include "BumpTop/MouseEventManager.h"
#include "BumpTop/OgreBulletConverter.h"
#include "BumpTop/OgreHelpers.h"
#include "BumpTop/PhysicsBoxActor.h"
#include "BumpTop/QStringHelpers.h"
#include "BumpTop/UndoCommands/AddToPileUndoCommand.h"
#include "BumpTop/UndoCommands/InternalDragAndDropUndoCommand.h"
#include "BumpTop/UndoCommands/PileBreakUndoCommand.h"
#include "BumpTop/UndoCommands/PileToGridUndoCommand.h"
#include "BumpTop/protoc/AllMessages.pb.h"
#include "BumpTop/ProtocolBufferHelpers.h"
#include "BumpTop/Room.h"
#include "BumpTop/RoomSurface.h"
#include "BumpTop/RoomItemPoseConstraints.h"
#include "BumpTop/Shape.h"
#include "BumpTop/StickyNote.h"
#include "BumpTop/ToolTipManager.h"
#include "BumpTop/UndoRedoStack.h"
#include "BumpTop/VisualActor.h"
#include "BumpTop/VisualPhysicsActorAnimation.h"

Ogre::Real kMaximumPileHeight = 150;

BumpTopCommandSet* BumpPile::context_menu_items_set = MakeQSet(12,  // count, must keep this updated
                                                               BreakPile::singleton(),
                                                               CreatePile::singleton(),
                                                               Grow::singleton(),
                                                               Shrink::singleton(),
                                                               Compress::singleton(),
                                                               MoveToTrash::singleton(),
                                                               Copy::singleton(),
                                                               PileByTypeForSelectedActors::singleton(),
                                                               Rename::singleton(),
                                                               GridView::singleton(),
                                                               SortAlphabetically::singleton(),
                                                               ChangeLabelColour::singleton());

BumpTopCommandSet* BumpPile::supported_context_menu_items() {
  return context_menu_items_set;
}

bool BumpPile::supportsContextMenuItem(BumpTopCommand* context_menu_item) {
  return context_menu_items_set->contains(context_menu_item);
}

BumpPile::BumpPile(Ogre::SceneManager *scene_manager, Physics *physics,
                   Room* room, VisualPhysicsActorId unique_id)
: VisualPhysicsActor(scene_manager, physics,  room->ogre_scene_node(), unique_id),
  room_(room),
  label_(NULL),
  mouse_handler_(NULL),
  display_name_(QString("")),
  label_visible_(true),
  highlight_(NULL),
  pile_actors_(QHash<VisualPhysicsActorId, VisualPhysicsActor*>()),
  original_actor_world_offsets_(QHash<VisualPhysicsActorId, Ogre::Vector3>()),
  original_actor_world_orientations_(QHash<VisualPhysicsActorId, Ogre::Quaternion>()),
  label_visible_from_camera_position_(true),
  is_performing_mouse_up_(false),
  should_delete_self_on_mouse_up_finished_(false),
  exposed_actor_(NULL),
  quick_view_index_(-1),
  scroll_delta_(0) {
}

BumpPile::~BumpPile() {
  if (label_ != NULL)
    delete label_;
  if (highlight_ != NULL)
    delete highlight_;
  if (mouse_handler_ != NULL)
    delete mouse_handler_;
}

void BumpPile::init() {
  VisualPhysicsActor::init();
  highlight_ = new HighlightActor(scene_manager_, visual_actor_->ogre_scene_node(), size().y);
  mouse_handler_ = new DampedSpringMouseHandler(this, room_);

  room_->addActor(this);
  Ogre::Real jiggle_factor = 0.01;
  set_room_surface(room_->getSurface(FLOOR));
  physics_actor_->set6DofConstraint(Ogre::Vector3(1, 1, 1),
                                    Ogre::Vector3(0, 0, 0),
                                    Ogre::Vector3(-0.2, -jiggle_factor, -0.2),
                                    Ogre::Vector3(0.2, jiggle_factor, 0.2));
  drop_receiver_ = new BumpPileDropReceiver(this);
  assert(QObject::connect(visual_actor_, SIGNAL(onDraggingEntered(MouseEvent*)),  // NOLINT
                          this, SLOT(draggingEntered(MouseEvent*))));  // NOLINT
  assert(QObject::connect(visual_actor_, SIGNAL(onDraggingUpdated(MouseEvent*)),  // NOLINT
                          this, SLOT(draggingUpdated(MouseEvent*))));  // NOLINT
  assert(QObject::connect(BumpTopApp::singleton()->mouse_event_manager(), SIGNAL(onMouseDown(MouseEvent*)),  // NOLINT
                          this, SLOT(globalMouseDown(MouseEvent*))));  // NOLINT
}

bool BumpPile::initFromBuffer(VisualPhysicsActorBuffer* buffer,  bool physics_enabled) {
  // return immediately if the size of children is 0-- this happened when loading a scene file once
  DEBUG_ASSERT(buffer->child_size() != 0 || buffer->is_new_items_pile());
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
  initWithActors(actors, pile_position, child_offsets, child_original_orientations);
  stackViewOnInit();
  set_orientation(QuaternionBufferToQuaternion(buffer->orientation()));

  updateLabel();
  set_label_colour((BumpBoxLabelColour)(buffer->label_colour()));

  return true;
}

void BumpPile::initWithActors(VisualPhysicsActorList actors, Ogre::Vector3 initial_position,
                              QList<Ogre::Vector3> offsets, QList<Ogre::Quaternion> orientations) {
  // initial_position is an optional parametre used to specify where the pile want to be created, if
  // initial_position is using the default value, then the pile is to be created with the centroid of
  // the actors and if an initial_position is passed to initWithActors, then pile will be created at
  // the position
  init();

  Ogre::Vector3 centroid;
  centroid.x = initial_position.x;
  centroid.z = initial_position.z;
  set_position(centroid);

  assert(actors.size() == offsets.size() && actors.size() == orientations.size());
  for (int i = 0; i < actors.size(); i++) {
    VisualPhysicsActor* actor = actors.value(i);
    Ogre::Vector3 offset = offsets.value(i);
    Ogre::Quaternion orientation = orientations.value(i);
    addActorToPile(actor, offset, orientation);
  }
}

// This method is used on initialization
void BumpPile::addActorToPile(VisualPhysicsActor* actor, Ogre::Vector3 offset_from_pile, Ogre::Quaternion orientation) {
  // We need to store the parent and the parent_ogre_scene_node; often the parent will tell us the parent node, but the
  // parent node may be NULL, in which case we need to store the parent node independently

  // TODO: the assert below will fail when actors are in the middle of animation; that is, they won't be
  // pinned to a wall, when their current animation ends, but for the time being, since they haven't been forced to
  // to complete the animation, their recorded surface will still be wrong. Fix it.
  // assert(!actor->room_surface()->is_pinnable_receiver() && "Piles cant properly deal with actors from the walls");
  offset_from_pile.y = 0;
  original_actor_world_offsets_.insert(actor->unique_id(), offset_from_pile);
  original_actor_world_orientations_.insert(actor->unique_id(), actor->pose().orientation);

  AnimationManager::singleton()->endAnimationsForActor(actor, AnimationManager::STOP_AT_CURRENT_STATE);
  actor->set_physics_enabled(false);
  actor->set_selected(false);
  actor->set_parent(this);
  actor->set_label_visible(false);

  assert(QObject::connect(actor, SIGNAL(onRemoved(VisualPhysicsActorId)),  // NOLINT
                          this, SLOT(pileActorRemoved(VisualPhysicsActorId))));  // NOLINT

  assert(QObject::connect(actor, SIGNAL(onSizeChanged(VisualPhysicsActorId)),  // NOLINT
                          this, SLOT(pileActorChangedPhysically(VisualPhysicsActorId))));  // NOLINT

  pile_actors_.insert(actor->unique_id(), actor);
  pile_actor_ids_in_order_.push_back(actor->unique_id());
  room_->removeActor(actor->unique_id());
}

// This method is used when adding an actor to the pile after the fact
void BumpPile::addActorToPileAndUpdatePileView(VisualPhysicsActor* actor,
                                               Ogre::Vector3 offset_from_pile,
                                               Ogre::Quaternion orientation) {
  addActorToPile(actor, offset_from_pile, orientation);
  stackViewOnActorAddedOrRemoved();
}

bool BumpPile::breakable() {
  return true;
}

QHash<VisualPhysicsActorId, BumpPose> BumpPile::actor_offsets_and_original_orientations() {
  QHash<VisualPhysicsActorId, BumpPose> return_poses = QHash<VisualPhysicsActorId, BumpPose>();
  for_each(VisualPhysicsActorId actor_id, pile_actors_.keys()) {
    BumpPose pose = BumpPose(original_actor_world_offsets_[actor_id] + position(),
                             original_actor_world_orientations_[actor_id]);
    return_poses.insert(actor_id, pose);
  }
  return return_poses;
}

void BumpPile::breakPileWithoutConstrainingFinalPoses() {
  breakPile(actor_offsets_and_original_orientations(), false);
}

void BumpPile::breakPile() {
  breakPile(actor_offsets_and_original_orientations(), true);
}

void BumpPile::addActorToParentHelper(VisualPhysicsActor* actor) {
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
    actor->set_label_visible(true);
    room_->addActor(actor);
  }
}

void BumpPile::breakPile(QHash<VisualPhysicsActorId, BumpPose> final_poses, bool constrain_final_poses) {
  emit onRemoved(unique_id());
  if (pile_actors_.count() == 1) {
    // This pile is being broken because its items have been removed externally
    // In this case we don't want to animate the break, and we want to just leave the
    // item unmoved.
    room_->removeActor(unique_id_);
    physics_actor_->set_physics_enabled(false);
    for_each(VisualPhysicsActor* actor, pile_actors_.values()) {
      addActorToParentHelper(actor);
    }
  } else {
    // Remove the pile from the room.
    room_->removeActor(unique_id_);
    physics_actor_->set_physics_enabled(false);
    // add the pile actors back to the room
    for_each(VisualPhysicsActorId actor_id, pile_actors_.keys()) {
      addActorToParentHelper(pile_actors_[actor_id]);
    }

    // Determine the constrained poses of the pile items before animating
    if (constrain_final_poses) {
      QHash<VisualPhysicsActorId, BumpPose> constrained_poses = getActorPosesConstrainedToRoom(final_poses, room_);
      constrained_poses = getActorPosesConstrainedToNoIntersections(constrained_poses, room_);
      final_poses = constrained_poses;
    }

    for_each(VisualPhysicsActor* actor, pile_actors_.values()) {
      VisualPhysicsActorAnimation* actor_animation;
      actor_animation = new VisualPhysicsActorAnimation(actor, 325,
                                                        final_poses[actor->unique_id()].position,
                                                        final_poses[actor->unique_id()].orientation);
      actor_animation->start();

      actor->set_selected(true);
      actor->set_label_visible(!actor->name_hidden());
    }
  }
}

void BumpPile::breakItemsOutOfPile(QList<VisualPhysicsActorId> actor_ids) {
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

VisualPhysicsActorList BumpPile::children() {
  VisualPhysicsActorList pile_actors_in_order;
  for_each(VisualPhysicsActorId actor_id, pile_actor_ids_in_order_) {
    pile_actors_in_order.push_front(pile_actors_[actor_id]);
  }
  return pile_actors_in_order;
}

VisualPhysicsActorList BumpPile::flattenedChildren() {
  VisualPhysicsActorList children = BumpPile::children();
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

QList<VisualPhysicsActorId> BumpPile::flattenedChildrenIds() {
  QList<VisualPhysicsActorId> flattened_children_ids;
  for_each(VisualPhysicsActor* actor, flattenedChildren()) {
    flattened_children_ids.push_back(actor->unique_id());
  }
  return flattened_children_ids;
}

QList<VisualPhysicsActorId> BumpPile::children_ids() {
  return pile_actor_ids_in_order_;
}

QList<Ogre::Vector3> BumpPile::children_offsets() {
  QList<Ogre::Vector3> children_offsets_in_order;
  for_each(VisualPhysicsActorId actor_id, pile_actor_ids_in_order_) {
    children_offsets_in_order.push_front(original_actor_world_offsets_[actor_id]);
  }
  return children_offsets_in_order;
}

QList<Ogre::Quaternion> BumpPile::children_orientations() {
  QList<Ogre::Quaternion> children_orientations_in_order;
  for_each(VisualPhysicsActorId actor_id, pile_actor_ids_in_order_) {
    children_orientations_in_order.push_front(original_actor_world_orientations_[actor_id]);
  }
  return children_orientations_in_order;
}

void BumpPile::set_size(Ogre::Vector3 size) {
  if (physics_actor_ != NULL)
    physics_actor_->set_scale((1/PHYSICS_SCALE)*size);
}

Ogre::Vector3 BumpPile::size() {
  if (physics_actor_ != NULL)
    return PHYSICS_SCALE*physics_actor_->size();
  else
    return Ogre::Vector3::ZERO;
}

QStringList BumpPile::pathsOfDescendants() {
  QStringList paths;
  for_each(VisualPhysicsActor* actor, pile_actors_.values()) {
    if (actor->path() != "")
      paths.push_back(actor->path());

    QStringList actor_paths = actor->pathsOfDescendants();
    for_each(QString path, actor_paths) {
      paths.push_back(path);
    }
  }
  return paths;
}

void BumpPile::stackViewOnInit() {
  stackView(true, 1, 325);
}

void BumpPile::stackViewOnActorAddedOrRemoved() {
  stackView(false, 1, 325);
}

void BumpPile::stackViewAndScaleChildren(Ogre::Real scale_factor) {
  stackView(false, scale_factor, 600);
}

void BumpPile::stackViewOnSortAlphabetically() {
  stackView(false, 1, 325);
}

void BumpPile::stackView(bool first_time, Ogre::Real scale_factor, Ogre::Real animation_duration) {
  moveDummyToBottom();
  Ogre::Real actor_y_pos;
  VisualPhysicsActorAnimation* actor_animation;
  Ogre::Real total_height = 0;
  Ogre::Real max_actor_x = 0;
  Ogre::Real max_actor_z = 0;
  Ogre::Real mass = 0;

  AnimationManager::singleton()->endAnimationsForActor(this, AnimationManager::MOVE_TO_FINAL_STATE);

  for_each(VisualPhysicsActor* actor, pile_actors_.values()) {
    if (actor->size().x > max_actor_x)
      max_actor_x = actor->size().x;
    if (actor->size().z > max_actor_z)
      max_actor_z = actor->size().z;
    mass += actor->mass();
  }

  total_height = heightOfPileForScaleFactor(scale_factor);

  int num_actors = pile_actors_.count();
  set_size(scale_factor * Ogre::Vector3(max_actor_x, 0, max_actor_z) + Ogre::Vector3(0, total_height, 0));

  set_position(Ogre::Vector3(position().x, room_->min_y() + (total_height)/2.0 + 1, position().z));
  physics_actor_->setMass(mass);

  Ogre::Real current_height = -total_height/2.0;
  for_each(VisualPhysicsActorId actor_id, pile_actor_ids_in_order_) {
    VisualPhysicsActor* actor = pile_actors_[actor_id];

    if (total_height == maximum_height()) {
      actor_y_pos = current_height + (total_height/(2.0*num_actors));
      current_height += total_height/(num_actors*1.0);
    } else {
      actor_y_pos = current_height + actor->size().y/2;
      current_height += actor->size().y * scale_factor;
    }

    Ogre::Real random_y_rotation = Ogre::Math::RangeRandom(-0.06, 0.06);
    AnimationManager::singleton()->endAnimationsForActor(actor, AnimationManager::STOP_AT_CURRENT_STATE);

    Ogre::Quaternion orientation;
    if (actor_orientations_in_pile_.contains(actor_id)) {
      orientation =  actor_orientations_in_pile_[actor_id];
    } else {
      orientation = Ogre::Quaternion(1, 0, random_y_rotation, 0);
      actor_orientations_in_pile_[actor_id] = orientation;
    }

    int16_t transition_style = scale_factor == 1 ? tween::CUBIC : tween::ELASTIC;

    actor_animation = new VisualPhysicsActorAnimation(actor,
                                                      animation_duration,
                                                      Ogre::Vector3(0, actor_y_pos, 0),
                                                      orientation,
                                                      NULL, 1,
                                                      scale_factor,
                                                      transition_style);
    actor_animation->start();
  }

  if (first_time || parent() == NULL)
    set_pose(getActorPoseConstrainedToRoomAndNoIntersections(this, room_));

  if (pile_actors_.size() > 0)
    highlight_->set_to_render_before(lowest_child_with_visual_actor()->visual_actor());

  highlight_->set_height_of_parent(total_height);
  highlight_->set_scale(size() / Ogre::Vector3(kOgreSceneNodeScaleFactor));
}

Ogre::Real BumpPile::heightOfPile() {
  return heightOfPileForScaleFactor(1);
}

Ogre::Real BumpPile::heightOfPileForScaleFactor(Ogre::Real scale_factor) {
  Ogre::Real height = 0;
  for_each(VisualPhysicsActor* actor, pile_actors_.values()) {
    height += actor->size().y;
  }
  height *= scale_factor;

  if (height > maximum_height())
    height = maximum_height();

  return height;
}

Ogre::Real BumpPile::maximum_height() {
  return kMaximumPileHeight;
}

void BumpPile::moveDummyToBottom() {
  VisualPhysicsActorId dummy_id = 0;
  for_each(VisualPhysicsActor* child, children()) {
    if (child->actor_type() == BUMP_DUMMY) {
      dummy_id = child->unique_id();
    }
  }
  if (dummy_id != 0) {
    pile_actor_ids_in_order_.removeAll(dummy_id);
    pile_actor_ids_in_order_.push_front(dummy_id);
  }
}

void BumpPile::set_selected(bool is_selected) {
  if (is_selected != is_selected_) {
    if (is_selected) {
      ToolTipManager::singleton()->showGriddedPileTooltip(this);
      ToolTipManager::singleton()->showPileFlipTooltip(this);
      ToolTipManager::singleton()->showNamePileTooltip(this);
    }

    is_selected_ = is_selected;
    emit onSelectedChanged(unique_id_);
    if (label_ != NULL)
      label_->set_selected(is_selected);
    if (highlight_ != NULL) {
      highlight_->set_visible(is_selected);
    }
    BumpTopApp::singleton()->markGlobalStateAsChanged();
  }
  if (is_selected) {
    if (exposed_actor_ != NULL && quick_view_index_ != -1) {
      slideActorIn(exposed_actor_);
    }
    resetQuickView();
    room_->set_last_selected_actor(unique_id_);
  }
}

bool BumpPile::selected() {
  return is_selected_;
}

void BumpPile::draggingExited() {
  if (highlight_ != NULL) {
    highlight_->set_visible(false);
  }
}

void BumpPile::slideActorOut(VisualPhysicsActor* actor) {
  AnimationManager::singleton()->endAnimationsForActor(actor, AnimationManager::MOVE_TO_FINAL_STATE);
  VisualPhysicsActorAnimation* actor_animation;
  actor_animation = new VisualPhysicsActorAnimation(actor, 200,
                                                    Ogre::Vector3(-0.6*actor->size().x - size().x/2.0,
                                                                  actor->position().y, 0),
                                                    Ogre::Quaternion::IDENTITY);
  assert(QObject::connect(actor_animation, SIGNAL(onAnimationComplete(VisualPhysicsActorAnimation*)),  // NOLINT
                          this, SLOT(actorAnimationComplete(VisualPhysicsActorAnimation*))));  // NOLINT
  actor_animation->start();
  set_selected(false);

  if (label_ != NULL && pile_actors_.count() == 2)
    label_->set_visible(false);
}

void BumpPile::actorAnimationComplete(VisualPhysicsActorAnimation* animation) {
  animation->visual_physics_actor()->set_label_visible(true);
  animation->visual_physics_actor()->updateLabelPosition();
  room_->deselectActors();
  animation->visual_physics_actor()->set_selected(true);
}

void BumpPile::slideActorIn(VisualPhysicsActor* actor) {
  AnimationManager::singleton()->endAnimationsForActor(actor, AnimationManager::MOVE_TO_FINAL_STATE);
  VisualPhysicsActorAnimation* actor_animation;
  actor_animation = new VisualPhysicsActorAnimation(actor, 200,
                                                    Ogre::Vector3(0, actor->position().y, 0),
                                                    actor_orientations_in_pile_[actor->unique_id()]);
  actor_animation->start();
  actor->set_label_visible(false);
  actor->set_selected(false);
}

void BumpPile::resetQuickView() {
  exposed_actor_ = NULL;
  quick_view_index_ = -1;
  if (label_ != NULL)
  label_->set_visible(true);
  for_each(VisualPhysicsActor* actor, pile_actors_.values()) {
    actor->setMaterialBlendFactors(Ogre::Vector3(1.0, 1.0, 1.0));
  }
}

VisualPhysicsActorId BumpPile::adjacentActorOfChild(VisualPhysicsActorId child_id, ArrowKey arrow_key) {
  if (arrow_key == ARROW_UP && child_id != pile_actor_ids_in_order_.last()) {
    return pile_actor_ids_in_order_[pile_actor_ids_in_order_.indexOf(child_id) + 1];
  }
  if (arrow_key == ARROW_DOWN && child_id != pile_actor_ids_in_order_.first()) {
    return pile_actor_ids_in_order_[pile_actor_ids_in_order_.indexOf(child_id) - 1];
  }
  return 0;
}

void BumpPile::revealChild(VisualPhysicsActorId child_id) {
  if (!(ProAuthorization::singleton()->authorized())) {
    return;
  }

  if (pile_actor_ids_in_order_.contains(child_id)) {
    if (exposed_actor_ != NULL) {
      slideActorIn(pile_actors_[exposed_actor_->unique_id()]);
    }
    slideActorOut(pile_actors_[child_id]);
    exposed_actor_ = pile_actors_[child_id];

    int num_actors = children().count();
    int index = pile_actor_ids_in_order_.indexOf(child_id);
    quick_view_index_ = index;
    for (int i = 0; i <= index; i++) {
      pile_actors_[pile_actor_ids_in_order_[i]]->setMaterialBlendFactors(Ogre::Vector3(1.0, 1.0, 1.0));
    }
    for (int i = index + 1; i < num_actors; i++) {
      if (pile_actors_[pile_actor_ids_in_order_[i]]->material_name() != AppSettings::singleton()->global_material_name(DEFAULT_ICON)) {
        pile_actors_[pile_actor_ids_in_order_[i]]->setMaterialBlendFactors(Ogre::Vector3(0.4, 0.4, 0.4));
      }
    }
  }
}

void BumpPile::scrollWheel(MouseEvent* mouse_event) {
  if (!(ProAuthorization::singleton()->authorized())) {
    return;
  }

  if (is_new_items_pile() && children().count() == 1) {
    return;
  }

  ToolTipManager::singleton()->hidePileFlipTooltip();

  int num_actors = pile_actor_ids_in_order_.count();
  int last_quick_view_index = quick_view_index_;

  scroll_delta_ += mouse_event->delta_y;

  if (scroll_delta_ >= ProAuthorization::singleton()->flip_pile_scroll_delta_threshold()) {
    if (quick_view_index_ == -1) {
      quick_view_index_ = 0;
    } else {
      quick_view_index_ += ProAuthorization::singleton()->flip_pile_scroll_index_advancement();
    }
    scroll_delta_ = 0;
  } else if (scroll_delta_ <= -ProAuthorization::singleton()->flip_pile_scroll_delta_threshold()) {
    if (quick_view_index_ == -1) {
      quick_view_index_ = num_actors - ProAuthorization::singleton()->flip_pile_scroll_index_advancement();
    } else {
      quick_view_index_ -= ProAuthorization::singleton()->flip_pile_scroll_index_advancement();
    }
    scroll_delta_ = 0;
  }

  if (quick_view_index_ < 0)
    quick_view_index_ = 0;
  else if (quick_view_index_ > num_actors - 1)
    quick_view_index_ = num_actors - ProAuthorization::singleton()->flip_pile_scroll_index_advancement();

  if (last_quick_view_index != quick_view_index_) {
    revealChild(pile_actor_ids_in_order_[quick_view_index_]);
  }

  mouse_event->handled = true;
}

void BumpPile::mouseMoved(MouseEvent* mouse_event) {
  VisualPhysicsActor::mouseMoved(mouse_event);
  mouse_event->handled = true;
}

void BumpPile::globalMouseDown(MouseEvent* mouse_event) {
  if (exposed_actor_ != NULL) {
    for_each(VisualPhysicsActor* actor, pile_actors_.values()) {
      if (mouse_event->item == actor->visual_actor()) {
        return;
      }
    }
    slideActorIn(exposed_actor_);
    resetQuickView();
  }
}

void BumpPile::mouseDown(MouseEvent* mouse_event) {
  VisualPhysicsActor::mouseDown(mouse_event);
  if (exposed_actor_ != NULL && mouse_event->item == exposed_actor_->visual_actor()) {
    assert(QObject::connect(exposed_actor_, SIGNAL(onMouseMotionRegistered(VisualPhysicsActor*)),  // NOLINT
                            this, SLOT(breakActorBeingDraggedOutOfPile(VisualPhysicsActor*))));  // NOLINT
    return;
  }

  if (exposed_actor_ != NULL) {
    slideActorIn(exposed_actor_);
    resetQuickView();
  }

  room_->openNewUndoCommand();
  mouse_event->handled = true;

  if (mouse_event->num_clicks == 2) {
    launch();
    return;
  }
  render_queue_group_on_mouse_down_ = render_queue_group();
  set_render_queue_group(99);

  bool command_or_shift_pressed = mouse_event->modifier_flags & COMMAND_KEY_MASK
                                || mouse_event->modifier_flags & SHIFT_KEY_MASK;

  if (!command_or_shift_pressed && !selected())
    room_->deselectActors();

  if (command_or_shift_pressed && selected()) {
    set_selected(false);
  } else {
    set_selected(true);
    if (command_or_shift_pressed)
      room_->updateBumpToolbar();
    mouse_handler_->mouseDown(mouse_event);
  }
}

void BumpPile::mouseDragged(MouseEvent* mouse_event) {
  VisualPhysicsActor::mouseDragged(mouse_event);
  if (exposed_actor_ != NULL && mouse_event->item == exposed_actor_->visual_actor()) {
    return;
  }
  mouse_handler_->mouseDragged(mouse_event);
}

void BumpPile::mouseUp(MouseEvent* mouse_event) {
  // this flag is to prevent the pile getting deleted before the block of code finishes executing
  // so we are going to hold back the deletion until all operation related to mouseUp has been executed
  VisualPhysicsActor::mouseUp(mouse_event);
  if (exposed_actor_ != NULL && mouse_event->item == exposed_actor_->visual_actor()) {
    return;
  }

  is_performing_mouse_up_ = true;
  mouse_handler_->mouseUp(mouse_event);
  is_performing_mouse_up_ = false;
  if (!should_delete_self_on_mouse_up_finished_) {
    if (render_queue_group() == 99) {
      // nobody else changed our render queue group, so let's just change it back
      set_render_queue_group(render_queue_group_on_mouse_down_);
    }
  } else {
    delete this;
  }
}

void BumpPile::rightMouseDown(MouseEvent* mouse_event) {
  VisualPhysicsActor::rightMouseDown(mouse_event);
  if (exposed_actor_ != NULL && mouse_event->item == exposed_actor_->visual_actor()) {
    return;
  }

  if (!(mouse_event->modifier_flags & COMMAND_KEY_MASK) && !selected())
    room_->deselectActors();

  if ((mouse_event->modifier_flags & COMMAND_KEY_MASK) && selected()) {
    set_selected(false);
  } else {
    set_selected(true);
    if (mouse_event->modifier_flags & COMMAND_KEY_MASK)
      room_->updateBumpToolbar();
    BumpEnvironment env(physics_, room_, scene_manager_);
    if (parent() == NULL) {
      launchContextMenu(env, room_->selected_actors(), mouse_event->mouse_in_window_space);
    } else {
      VisualPhysicsActorList context_menu_actors;
      context_menu_actors.append(this);
      launchContextMenu(env, context_menu_actors, mouse_event->mouse_in_window_space);
    }
    mouse_event->handled = true;
  }
}

void BumpPile::draggingEntered(MouseEvent* mouse_event) {
  draggingUpdated(mouse_event);
}

void BumpPile::draggingUpdated(MouseEvent* mouse_event) {
  if (mouse_event->items_being_dropped.size() != 0) {
    bool i_am_being_dragged = false;
    for_each(VisualPhysicsActor* actor, mouse_event->items_being_dropped) {
      if (actor == this) {
        i_am_being_dragged = true;
        break;
      }
    }
    if (!i_am_being_dragged) {
      if (mouse_event->drag_operations & NSDragOperationMove) {
        mouse_event->drag_operations = NSDragOperationMove;
        mouse_event->handled = true;
        mouse_event->drop_receiver = drop_receiver_;

        if (highlight_ != NULL) {
          highlight_->set_visible(true);
        }
      }
    }
  }
}

void BumpPile::set_render_queue_group(uint8 queue_id) {
  VisualPhysicsActor::set_render_queue_group(queue_id);
  if (label_ != NULL)
    label_->set_render_queue_group(queue_id);
  if (highlight_ != NULL)
    highlight_->set_render_queue_group(queue_id);
  for_each(VisualPhysicsActor* actor, pile_actors_)
    actor->set_render_queue_group(queue_id);
}

uint8 BumpPile::render_queue_group() {
  return lowest_child_with_visual_actor()->render_queue_group();
}

// This method is only used for dragging out and not generally for breaking the gridded pile
void BumpPile::breakActorBeingDraggedOutOfPile(VisualPhysicsActor* actor) {
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
  actor->set_room_surface(room_->getSurface(FLOOR));
  room_->addActor(actor);
  actor->disconnect(this);
  // We set the actors collisions temporarily so that the pile isn't repositioned
  // to avoid this actor when it's re-stacked
  actor->setCollisionsToOnlyWalls();
  resetQuickView();
  actor->setCollisionsToAllActors();

  // this can delete "this" if there is only one actor remaining in the pile after deletion
  removeActorFromPileHelper(actor->unique_id());
}

void BumpPile::pileActorRemoved(VisualPhysicsActorId actor_id) {
  if (exposed_actor_ != NULL && actor_id == exposed_actor_->unique_id()) {
    resetQuickView();
  }
  delete pile_actors_[actor_id];
  removeActorFromPileHelper(actor_id);
}

void BumpPile::removeActorFromPileHelper(VisualPhysicsActorId actor_id) {
  pile_actors_.remove(actor_id);
  original_actor_world_offsets_.remove(actor_id);
  original_actor_world_orientations_.remove(actor_id);
  actor_orientations_in_pile_.remove(actor_id);
  pile_actor_ids_in_order_.removeAll(actor_id);

  if (pile_actors_.count() > 1) {
    stackViewOnActorAddedOrRemoved();
    emit onSizeChanged(unique_id_);
  } else {
    if (parent_ != NULL) {
      breakPile();
      emit onRemoved(unique_id_);
    } else if (pile_actors_.count() == 1) {
      breakPile();
      // do not delete pile yet if pile is still being dragged
      // this may happen if we drag a pile to a folder and the folder contains items in the pile
      // a blocking alert will appear but during that time fileSystemEventWatcher will notify us that files are
      // being deleted
      if (is_performing_mouse_up_) {
        should_delete_self_on_mouse_up_finished_ = true;
      } else {
        delete this;
      }
    }
  }
}

void BumpPile::pileActorChangedPhysically(VisualPhysicsActorId actor_id) {
  /*stackView(false);
  emit onSizeChanged(unique_id_);*/
}

void BumpPile::writeToBuffer(VisualPhysicsActorBuffer* buffer) {
  buffer->set_actor_type(actor_type());
  QuaternionToBuffer(orientation(), buffer->mutable_orientation());

  buffer->clear_child();
  buffer->clear_child_position();
  buffer->clear_child_orientation();
  buffer->clear_position();

  Vector3ToBuffer(position(), buffer->mutable_position());
  buffer->set_display_name(utf8(display_name_));
  buffer->set_label_colour((int)label_colour());

  for_each(VisualPhysicsActorId actor_id, pile_actor_ids_in_order_) {
    VisualPhysicsActor* actor = pile_actors_[actor_id];
    if (actor->serializable()) {
      actor->writeToBuffer(buffer->add_child());
      Vector3ToBuffer(original_actor_world_offsets_[actor_id] + position(),
                      buffer->add_child_position());
      QuaternionToBuffer(original_actor_world_orientations_[actor_id], buffer->add_child_orientation());
    }
  }
}

void BumpPile::deleteLabel() {
  if (label_ != NULL)
    delete label_;
  label_ = NULL;
}

void BumpPile::updateLabel(Ogre::Real size_factor) {
  BumpBoxLabelColour colour = label_colour();
  deleteLabel();
  if (display_name_ != "" && label_visible_) {
    label_ = new BumpBoxLabel(display_name_, this);
    set_label_colour(colour);
    if (size_factor == -1) {
      if (size().x == 0) {
        label_->init(1);
      } else {
        label_->init(1 + (size().x / kInitialActorSize - 1)/2);
      }
    } else {
      label_->init(size_factor);
    }
    label_->set_render_queue_group(render_queue_group());
    label_->set_to_render_after(lowest_child_with_visual_actor()->visual_actor());
    updateLabelPosition();
    if (is_selected_) {
      label_->set_selected(true);
    }
    assert(QObject::connect(label_, SIGNAL(onMouseDown(MouseEvent*)),  // NOLINT
                              this, SLOT(labelClicked(MouseEvent*))));  // NOLINT
  }
}

void BumpPile::rename(QString new_name) {
  display_name_ = new_name;
  updateLabel();
}

bool BumpPile::nameable() {
  return true;
}

QString BumpPile::display_name() {
  return display_name_;
}

void BumpPile::set_display_name(QString display_name) {
  display_name_ = display_name;
  updateLabel();
}

void BumpPile::update() {
  VisualPhysicsActor::update();
  updateLabelPosition();
}

void BumpPile::set_position_no_physics(const Ogre::Vector3 &pos) {
  VisualPhysicsActor::set_position_no_physics(pos);
  updateLabelPosition();
}

void BumpPile::sortAlphabetically() {
  VisualPhysicsActorList pile_actor = pile_actors_.values();
  QList<VisualPhysicsActorId> pile_actor_ids_alphabetically;
  qSort(pile_actor.begin(), pile_actor.end(), compareDisplayName);

  for_each(VisualPhysicsActor* actor, pile_actor) {
    pile_actor_ids_alphabetically.push_front(pile_actors_.key(actor));
  }
  pile_actor_ids_in_order_ = pile_actor_ids_alphabetically;
  stackViewOnSortAlphabetically();
}

const Ogre::Vector3 BumpPile::world_position() {
  return PHYSICS_SCALE*physics_actor_->world_position();
}

Ogre::Vector2 BumpPile::labelPositionForCurrentPosition() {
  Ogre::Real y = screenBoundingBox().getMaximum().y;

  Ogre::AxisAlignedBox bounding_box = world_bounding_box();
  const Ogre::Vector3* world_corners_ptr = bounding_box.getAllCorners();
  QList<Ogre::Vector3> world_corners;
  for (int i = 0; i < 8; i++) {
    world_corners.push_back(world_corners_ptr[i]);
  }

  qSort(world_corners.begin(), world_corners.end(), yLessThanVec3);
  QList<Ogre::Vector2> base_screen_corners;
  for (int i = 0; i < 4; i++) {
    base_screen_corners.push_back(worldPositionToScreenPosition(world_corners[i]));
  }
  qSort(base_screen_corners.begin(), base_screen_corners.end(), yLessThanVec2);
  Ogre::Real x = (base_screen_corners[3].x + base_screen_corners[2].x)/2.0;

  return Ogre::Vector2(x, y);
}

void BumpPile::updateLabelPosition() {
  if (label_ != NULL && label_visible_) {
    label_->set_position_in_pixel_coords(labelPositionForCurrentPosition());
    Ogre::Camera* camera = BumpTopApp::singleton()->camera();
    // We don't want to show labels for items behind the camera or for items that are clipped
    if ((world_position() - camera->getPosition()).dotProduct(camera->getDirection()) < 0 ||
        (world_position() - camera->getPosition()).length() < camera->getNearClipDistance()) {
      set_label_visible_from_camera_position(false);
    } else {
      set_label_visible_from_camera_position(true);
    }
  }
}

void BumpPile::set_label_visible_from_camera_position(bool visible) {
  if (label_visible_from_camera_position_ != visible) {
    label_visible_from_camera_position_ = visible;
    if (!label_visible_from_camera_position_ && label_visible_) {
      if (label_ != NULL)
        label_->set_visible(false);
    } else if (label_visible_from_camera_position_ && label_visible_) {
      if (label_ != NULL)
        label_->set_visible(true);
    }
  }
}

void BumpPile::set_label_visible(bool label_visible) {
  if (label_visible_ && !label_visible) {
    if (label_ != NULL)
      label_->set_visible(false);
  } else if (!label_visible_ && label_visible) {
    if (label_ != NULL && label_visible_from_camera_position_) {
      label_->set_visible(true);
      updateLabelPosition();
    }
  }
  label_visible_ = label_visible;

  BumpTopApp::singleton()->markGlobalStateAsChanged();
}

void BumpPile::labelClicked(MouseEvent* mouse_event) {
  mouse_event->handled = true;
  if ((mouse_event->num_clicks == 1 && selected()) || mouse_event->num_clicks == 2) {
    VisualPhysicsActorList list_with_just_me;
    list_with_just_me.append(this);
    BumpEnvironment bump_environment = BumpEnvironment(BumpTopApp::singleton()->physics(),
                                                       BumpTopApp::singleton()->scene()->room(),
                                                       BumpTopApp::singleton()->ogre_scene_manager());
    if (Rename::singleton()->canBeAppliedToActors(bump_environment, list_with_just_me)) {
      Rename::singleton()->applyToActors(bump_environment, list_with_just_me);
    }
  } else {
    bool command_or_shift_pressed = mouse_event->modifier_flags & COMMAND_KEY_MASK
                                    || mouse_event->modifier_flags & SHIFT_KEY_MASK;

    if (!selected() && !command_or_shift_pressed) {
      room_->deselectActors();
    }
    if (command_or_shift_pressed) {
      room_->updateBumpToolbar();
    }
    set_selected(true);
  }
}

bool BumpPile::serializable() {
  return true;
}

// Implementing abstract members of VisualPhysicsActor
std::string BumpPile::meshName() {
  return "";
}

Ogre::Vector3 BumpPile::absoluteMeshSizeDividedBy100() {
  return Ogre::Vector3(0, 0, 0);
}

btVector3 BumpPile::physicsSize() {
  return btVector3(1.0, 1.0, 1.0);
}

void BumpPile::makePhysicsActor(bool physics_enabled) {
  physics_actor_ = new PhysicsBoxActor(physics_, 1.0, toOgre(physicsSize()), physics_enabled);
}

VisualPhysicsActorType BumpPile::actor_type() {
  return BUMP_PILE;
}

Ogre::Vector2 BumpPile::actor_screen_position_before_drag() {
  return screen_position_before_drag_;
}

BumpPose BumpPile::actor_pose_before_drag() {
  return pose_before_drag_;
}

void BumpPile::updateActorStatusBeforeDrag() {
  screen_position_before_drag_ = getScreenPosition();
  pose_before_drag_ = pose();

  for_each(VisualPhysicsActor* child, children()) {
    child->updateActorOffsetPoseToItsParentBeforeDrag();
  }
}

BumpPose BumpPile::children_offset_pose(VisualPhysicsActor* actor) {
  if (flattenedChildren().contains(actor)) {
    return BumpPose(original_actor_world_offsets_[actor->unique_id()],
                    original_actor_world_orientations_[actor->unique_id()]);
  } else {
    return BumpPose(Ogre::Vector3::ZERO, Ogre::Quaternion::IDENTITY);
  }
}

void BumpPile::set_children_offset_pose(QHash<VisualPhysicsActorId, BumpPose> children_ids_to_offset_poses) {
  for_each(VisualPhysicsActorId child_id, children_ids_to_offset_poses.keys()) {
    original_actor_world_offsets_[child_id] = children_ids_to_offset_poses[child_id].position;
    original_actor_world_orientations_[child_id] = children_ids_to_offset_poses[child_id].orientation;
  }
}

// end: Implementing abstract members of VisualPhysicsActor

VisualPhysicsActor* BumpPile::lowest_child_with_visual_actor() {
  return pile_actors_[pile_actor_ids_in_order_.first()]->lowest_child_with_visual_actor();
}

bool BumpPile::capture_mouse_events() {
  return true;
}

BumpBoxLabel* BumpPile::label() {
  return label_;
}

void BumpPile::launch() {
  PileToGridUndoCommand* pile_to_grid_command = new PileToGridUndoCommand(unique_id(), room_,
                                                                          scene_manager_, physics_);
  room_->updateCurrentState();
  room_->undo_redo_stack()->push(pile_to_grid_command, room_->current_state());
  return;
}

Ogre::Plane BumpPile::plane() {
  VisualPhysicsActor* lowest_actor = lowest_child_with_visual_actor();
  if (lowest_actor != NULL && lowest_actor->visual_actor() != NULL) {
    return lowest_actor->visual_actor()->plane();
  }
  return Ogre::Plane();
}

QString BumpPileDropReceiver::target_path() {
  return pile_->path();
}

BumpPileDropReceiver::BumpPileDropReceiver(BumpPile* pile)
: pile_(pile) {
}

bool BumpPileDropReceiver::prepareForDragOperation(Ogre::Vector2 mouse_in_window_space, QStringList list_of_files,
                                                   NSDragOperation drag_operation,
                                                   VisualPhysicsActorList list_of_actors) {
  return drag_operation & NSDragOperationMove;
}

bool BumpPileDropReceiver::performDragOperation(Ogre::Vector2 mouse_in_window_space, QStringList list_of_files,
                                                NSDragOperation drag_operation,
                                                VisualPhysicsActorList list_of_actors) {
  Room* room = BumpTopApp::singleton()->scene()->room();
  InternalDragAndDropUndoCommand* drag_and_drop_undo_command = new InternalDragAndDropUndoCommand(list_of_actors,
                                                                                                  pile_->unique_id(),
                                                                                                  room);
  room->undo_redo_stack()->push(drag_and_drop_undo_command, room->current_state(), true);
  return true;
}

void BumpPileDropReceiver::concludeDragOperation() {
}

void BumpPileDropReceiver::draggingExited() {
  pile_->draggingExited();
}

#include "BumpTop/moc/moc_BumpPile.cpp"

