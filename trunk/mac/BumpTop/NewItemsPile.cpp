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

#include "BumpTop/NewItemsPile.h"

#include <string>

#include "BumpTop/AnimationManager.h"
#include "BumpTop/AppSettings.h"
#include "BumpTop/Authorization.h"
#include "BumpTop/BumpBoxLabel.h"
#include "BumpTop/BumpFlatSquare.h"
#include "BumpTop/BumpPile.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/BumpTopScene.h"
#include "BumpTop/DebugAssert.h"
#include "BumpTop/FileManager.h"
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

Ogre::Real kMaximumNewItemsPileHeight = 50;

BumpTopCommandSet* NewItemsPile::supported_context_menu_items() {
  BumpTopCommandSet* supported_context_menu_items = MakeQSet(3,
                                                             CreatePile::singleton(),
                                                             Copy::singleton(),
                                                             GridView::singleton());
  if (children().count() > 1) {
    supported_context_menu_items->insert(BreakPile::singleton());
    supported_context_menu_items->insert(Compress::singleton());
    supported_context_menu_items->insert(MoveToTrash::singleton());
    if (children().count() > 2) {
      supported_context_menu_items->insert(SortAlphabetically::singleton());
    }
  }
  supported_context_menu_items->insert(Grow::singleton());
  supported_context_menu_items->insert(Shrink::singleton());
  return supported_context_menu_items;
}

bool NewItemsPile::supportsContextMenuItem(BumpTopCommand* context_menu_item) {
  return supported_context_menu_items()->contains(context_menu_item);
}

NewItemsPile::NewItemsPile(Ogre::SceneManager *scene_manager, Physics *physics,
                   Room* room, VisualPhysicsActorId unique_id)
: BumpPile(scene_manager, physics,  room, unique_id),
  new_items_pile_marker_(NULL) {
}

NewItemsPile::~NewItemsPile() {
  if (new_items_pile_marker_ != NULL)
    delete new_items_pile_marker_;
}

void NewItemsPile::init() {
  BumpPile::init();

  // Adding the dummy
  BumpDummy* dummy = new BumpDummy(scene_manager_, physics_, room_);
  dummy->init();
  room_->addActor(dummy);

  dummy->set_position(Ogre::Vector3::ZERO);
  dummy->set_size(Ogre::Vector3(AppSettings::singleton()->default_icon_size()*kDummySizeFactor,
                                AppSettings::singleton()->default_icon_size()*kDummySizeFactor,
                                AppSettings::singleton()->default_icon_size()*kDummySizeFactor));
  addActorToPile(dummy, Ogre::Vector3::ZERO, Ogre::Quaternion::IDENTITY);

  // New Items Piles cannot receive drops but BumpPile::init() give it a drop receiver which needs to be deleted
  delete drop_receiver_;
  drop_receiver_ = NULL;

  new_items_pile_marker_ = new NewItemsPileMarker(scene_manager_, visual_actor_->ogre_scene_node(), size().y);
  new_items_pile_marker_->set_visible(true);

  physics_actor_->setMass(0);
}

void NewItemsPile::stackView(bool first_time, Ogre::Real scale_factor, Ogre::Real animation_duration) {
  if (scale_factor != 1) {
    AppSettings::singleton()->set_default_icon_size(AppSettings::singleton()->default_icon_size()*scale_factor);

    AppSettings::singleton()->saveSettingsFile();
  }

  BumpPile::stackView(first_time, scale_factor, animation_duration);

  physics_actor_->setMass(0);

  Ogre::Real total_height = 0;
  
  total_height = heightOfPileForScaleFactor(scale_factor);

  if (pile_actors_.size() > 0) {
    new_items_pile_marker_->set_to_render_before(highlight_);
  }
  new_items_pile_marker_->set_height_of_parent(total_height);
  new_items_pile_marker_->set_scale(size() / Ogre::Vector3(kOgreSceneNodeScaleFactor));
  highlight_->set_height_of_parent(total_height);
  highlight_->set_scale(size() / Ogre::Vector3(kOgreSceneNodeScaleFactor));

  set_position(Ogre::Vector3(position().x, room_->min_y() + (total_height)/2.0, position().z));

  resetQuickView();
  if (exposed_actor_ != NULL) {
    offsetTopActor();
  }
}

Ogre::Real NewItemsPile::maximum_height() {
  return kMaximumNewItemsPileHeight;
}

void NewItemsPile::set_render_queue_group(uint8 queue_id) {
  BumpPile::set_render_queue_group(queue_id);
  if (new_items_pile_marker_ != NULL)
    new_items_pile_marker_->set_render_queue_group(queue_id);
}

void NewItemsPile::resetQuickView() {
  if (children().count() < 2) {
    exposed_actor_ = NULL;
  } else {
    exposed_actor_ = children().first();
    for_each(VisualPhysicsActor* child, children()) {
      child->set_label_visible(false);
    }
    exposed_actor_->set_label_visible(true);
  }
  quick_view_index_ = -1;
  for_each(VisualPhysicsActor* actor, pile_actors_.values()) {
    actor->setMaterialBlendFactors(Ogre::Vector3(1.0, 1.0, 1.0));
  }
}

void NewItemsPile::offsetTopActor() {
  if (pile_actors_.count() < 3) {
    return;
  }
  VisualPhysicsActor* actor = pile_actors_[pile_actor_ids_in_order_.last()];
  AnimationManager::singleton()->endAnimationsForActor(actor, AnimationManager::STOP_AT_CURRENT_STATE);
  VisualPhysicsActorAnimation* actor_animation;
  actor_animation = new VisualPhysicsActorAnimation(actor, 200,
                                                    Ogre::Vector3(-AppSettings::singleton()->default_icon_size()/3, heightOfPile()/2+2,
                                                                  AppSettings::singleton()->default_icon_size()/3),
                                                    Ogre::Quaternion::IDENTITY);
  actor_animation->start();
}

VisualPhysicsActorId NewItemsPile::adjacentActorOfChild(VisualPhysicsActorId child_id, ArrowKey arrow_key) {
  if (arrow_key == ARROW_UP && child_id != pile_actor_ids_in_order_.last()) {
    return pile_actor_ids_in_order_[pile_actor_ids_in_order_.indexOf(child_id) + 1];
  }
  if (arrow_key == ARROW_DOWN && child_id != pile_actor_ids_in_order_[1]) {
    return pile_actor_ids_in_order_[pile_actor_ids_in_order_.indexOf(child_id) - 1];
  }
  return 0;
}

void NewItemsPile::breakAllItemsExceptDummy() {
  VisualPhysicsActorList actors = children();
  QList<VisualPhysicsActorId> actor_ids;
  for_each(VisualPhysicsActor* actor, children()) {
    if (actor->actor_type() != BUMP_DUMMY) {
      actor_ids.push_back(actor->unique_id());
    } else {
      actors.removeAll(actor);
    }
  }
  breakItemsConcentrically(actor_ids);
  for_each(VisualPhysicsActor* actor, actors) {
    actor->set_selected(true);
  }
}

void NewItemsPile::breakItemsConcentrically(QList<VisualPhysicsActorId> actor_ids) {
  int i = 1;
  QList<Ogre::Vector3> offsets = children_offsets();
  QHash<VisualPhysicsActorId, BumpPose> final_poses;
  for_each(VisualPhysicsActorId actor_id, actor_ids) {
    if (pile_actors_.contains(actor_id)) {
      VisualPhysicsActor* actor = pile_actors_[actor_id];
      addActorToParentHelper(actor);

      BumpPose pose = BumpPose(offsets[i] + position(),
                               original_actor_world_orientations_[actor_id]);
      
      final_poses.insert(actor_id, pose);
    }
    i++;
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

QList<Ogre::Vector3> NewItemsPile::children_offsets() {
  int angle = 0;
  int ring = 1;
  QList<Ogre::Vector3> children_offsets;
  for (int i = 0; i < pile_actor_ids_in_order_.count(); i++) {
    children_offsets.push_front(Ogre::Vector3 (1.5*ring*AppSettings::singleton()->default_icon_size()*sin(M_PI*2*angle/ring/8),
                                              0,
                                              -1.5*ring*AppSettings::singleton()->default_icon_size()*cos(M_PI*2*angle/ring/8)));
    angle++;
    if (angle == ring * 8) {
      ring++;
      angle = 0;
    }
  }

  return children_offsets;
}

QList<Ogre::Quaternion> NewItemsPile::children_orientations() {
  QList<Ogre::Quaternion> children_orientations;
  for (int i = 0; i < pile_actor_ids_in_order_.count(); i++) {
    children_orientations.push_back(Ogre::Quaternion::IDENTITY);
  }

  return children_orientations;
}

void NewItemsPile::removeActorFromPileHelper(VisualPhysicsActorId actor_id) {
  pile_actors_.remove(actor_id);
  original_actor_world_offsets_.remove(actor_id);
  original_actor_world_orientations_.remove(actor_id);
  actor_orientations_in_pile_.remove(actor_id);
  pile_actor_ids_in_order_.removeAll(actor_id);

  stackViewOnActorAddedOrRemoved();
  emit onSizeChanged(unique_id_);
}

void NewItemsPile::mouseDown(MouseEvent* mouse_event) {
  physics_actor_->setMass(1);

  BumpPile::mouseDown(mouse_event);

  if (quick_view_index_ != 0 && (mouse_event->num_clicks == 1)) {
    offsetTopActor();
  }
}

void NewItemsPile::mouseDragged(MouseEvent* mouse_event) {
  BumpPile::mouseDragged(mouse_event);
}

void NewItemsPile::mouseUp(MouseEvent* mouse_event) {
  BumpPile::mouseUp(mouse_event);
  physics_actor_->setMass(0);

  set_position(Ogre::Vector3(position().x, room_->min_y() + (heightOfPile())/2.0, position().z));
}

void NewItemsPile::set_selected(bool is_selected) {
  BumpPile::set_selected(is_selected);
  if (is_selected && exposed_actor_ != NULL && quick_view_index_ == -1) {
    offsetTopActor();
  }
}

void NewItemsPile::draggingUpdated(MouseEvent* mouse_event) {
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
          highlight_->set_visible(false);
        }
      }
    }
  }
}

void NewItemsPile::scrollWheel(MouseEvent* mouse_event) {
  if (!(ProAuthorization::singleton()->authorized())) {
    return;
  }
  if (mouse_event->delta_y <= -ProAuthorization::singleton()->flip_pile_scroll_delta_threshold()) {
    if (quick_view_index_ == 1) {
      return;
    }
    if (quick_view_index_ == -1) {
      slideActorIn(pile_actors_[pile_actor_ids_in_order_.last()]);
      quick_view_index_ = pile_actors_.size()-1;
    }
  } else if (mouse_event->delta_y >= ProAuthorization::singleton()->flip_pile_scroll_delta_threshold()) {
    if (quick_view_index_ == -1) {
      quick_view_index_ = 0;
    }
  }
  BumpPile::scrollWheel(mouse_event);
}

void NewItemsPile::globalMouseDown(MouseEvent* mouse_event) {
  BumpPile::globalMouseDown(mouse_event);
  if (quick_view_index_ != 0) {
    offsetTopActor();
  }
}

void NewItemsPile::writeToBuffer(VisualPhysicsActorBuffer* buffer) {
  BumpPile::writeToBuffer(buffer);
  buffer->set_is_new_items_pile(true);
}

bool NewItemsPile::is_new_items_pile() {
  return true;
}

bool NewItemsPile::nameable() {
  return false;
}

NewItemsPileMarker::NewItemsPileMarker(Ogre::SceneManager *scene_manager, Ogre::SceneNode *parent_ogre_scene_node,
                               Ogre::Real height_of_parent)
: HighlightActor(scene_manager, parent_ogre_scene_node, height_of_parent) {
  set_material_name(AppSettings::singleton()->global_material_name(NEW_ITEMS_PILE_ICON));
}

#include "BumpTop/moc/moc_NewItemsPile.cpp"

