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

#include "BumpTop/DampedSpringMouseHandler.h"

#include <utility>

#include "BumpTop/AnimationManager.h"
#include "BumpTop/AppSettings.h"
#include "BumpTop/BumpBox.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/BumpTopScene.h"
#include "BumpTop/DragAndDrop.h"
#include "BumpTop/DropReceiver.h"
#include "BumpTop/FileManager.h"
#include "BumpTop/KeyboardEventManager.h"
#include "BumpTop/MouseEventManager.h"
#include "BumpTop/OSX/EventModifierFlags.h"
#include "BumpTop/OSX/OgreController.h"
#include "BumpTop/PhysicsActor.h"
#include "BumpTop/Room.h"
#include "BumpTop/RoomItemPoseConstraints.h"
#include "BumpTop/RoomSurface.h"
#include "BumpTop/UndoCommands/InternalDragAndDropUndoCommand.h"
#include "BumpTop/UndoRedoStack.h"
#include "BumpTop/VisualActor.h"
#include "BumpTop/VisualPhysicsActor.h"
#include "BumpTop/VisualPhysicsActorAnimation.h"

const DraggingMode kWallDragType = PHYSICAL_DRAG;
const Ogre::Real kVelocityThresholdForDeclaringActorMotion = 5;
const Ogre::Real kMouseHasMovedThreshold = 8;

void resizeActor(VisualPhysicsActor* actor, Ogre::Real scale_factor) {
  if (actor->children().empty()) {
    AnimationManager::singleton()->endAnimationsForActor(actor, AnimationManager::STOP_AT_CURRENT_STATE);
    VisualPhysicsActorAnimation* actor_animation;
    actor_animation = new VisualPhysicsActorAnimation(actor, 600,
                                                      actor->position(),
                                                      actor->orientation(),
                                                      NULL, 1,
                                                      scale_factor,
                                                      tween::ELASTIC);
    actor_animation->start();
  } else {
    actor->stackViewAndScaleChildren(scale_factor);
  }
}

DampedSpringMouseHandler::DampedSpringMouseHandler(VisualPhysicsActor* actor, Room* room)
: actor_(actor),
  room_(room),
  mouse_drag_has_exited_bumptop_(false),
  drag_and_drop_started_(false),
  surface_on_mouse_down_(NULL),
  stab_point_object_space_(Ogre::Vector3::ZERO),
  mouse_motion_registered_(false),
  drag_ignored_for_menu_bar_(false),
  begin_drag_operation_on_next_mouse_drag_(false),
  drop_receiver_(NULL),
  is_droppable_(false),
  parent_(NULL) {
  actor_id_ = actor->unique_id();
}

DampedSpringMouseHandler::~DampedSpringMouseHandler() {
  BumpTopApp* bumptop = BumpTopApp::singleton();
  if (bumptop->mouse_event_manager()->global_capture() == actor_->visual_actor()) {
    bumptop->mouse_event_manager()->set_global_capture(NULL);
  }
}

void DampedSpringMouseHandler::mouseDown(MouseEvent* mouse_event) {
  Ogre::Vector2 position = actor_->getScreenPosition();
  //assert(actor_->ogre_scene_node()->getNumWorldTransforms() == 1);
  Ogre::Matrix4 world_transform = actor_->transform();

  // TODO: get rid of this STICKY_NOTE hack
  // TODO: and this new items pile hack
  is_droppable_ = actor_->path() != ""
                  && actor_->actor_type() != STICKY_NOTE
                  && !actor_->is_new_items_pile();
  if (!is_droppable_ && !actor_->is_new_items_pile()) {
    for_each(VisualPhysicsActor* child, actor_->flattenedChildren()) {
      if (child->path() != "") {
        is_droppable_ = true;
        break;
      }
    }
  }

  room_->draggingItemsBegan();

  assert(world_transform.isAffine());
  if (world_transform.isAffine()) {
    Ogre::Matrix4 world_transform_inverse = world_transform.inverseAffine();
    stab_point_object_space_ = world_transform_inverse*mouse_event->mouse_in_world_space;
  }

  BumpTopApp* bumptop = BumpTopApp::singleton();
  screen_resolution_ = bumptop->screen_resolution();
  assert(QObject::connect(bumptop, SIGNAL(onRender()),
                          this, SLOT(renderUpdate())));
  assert(QObject::connect(bumptop->mouse_event_manager(), SIGNAL(onMouseExited()),
                          this, SLOT(mouseExit())));
  assert(QObject::connect(bumptop->mouse_event_manager(), SIGNAL(onDraggingEntered()),
                          this, SLOT(dragEntered())));
  assert(QObject::connect(bumptop->mouse_event_manager(), SIGNAL(onDraggingExited()),
                          this, SLOT(dragExited())));

  bumptop->mouse_event_manager()->set_global_capture(actor_->visual_actor());
  mouse_event->handled = true;

  drag_ignored_for_menu_bar_ = false;
  begin_drag_operation_on_next_mouse_drag_ = false;
  mouse_motion_registered_ = false;
  // Variables necessary to properly handle drag and drop
  mouse_drag_has_exited_bumptop_ = false;
  pose_on_mouse_down_ = actor_->pose();
  surface_on_mouse_down_ = actor_->room_surface();

  last_mouse_in_window_space_ = mouse_event->mouse_in_window_space;

  if (actor_->linear_velocity().length() > kVelocityThresholdForDeclaringActorMotion) {
    mouse_motion_registered_ = true;
    beginDraggingItems();
  }
  if (actor_->children().isEmpty()) {
    size_on_mouse_down_ = actor_->scale();
  } else {
    size_on_mouse_down_ = actor_->children()[0]->scale();
  }
  drop_receiver_ = NULL;
  current_cursor_ = kRegularCursor;
}

void DampedSpringMouseHandler::beginDraggingItems() {
  drag_partners_offsets_.clear();
  drag_partners_collide_with_other_items_.clear();
  drag_partners_collide_with_other_items_.insert(actor_, !actor_->collides_with_walls_only());
  actor_->setCollisionsToOnlyWalls();
  actor_->setFriction(0.5);
  actor_->updateActorStatusBeforeDrag();

  VisualPhysicsActorList selected_actors = room_->selected_actors();
  if (actor_->is_new_items_pile()) {
    for_each(VisualPhysicsActor* actor, selected_actors) {
      if (actor != actor_) {
        actor->set_selected(false);
      }
    }
  } else if (!actor_->room_surface()->is_pinnable_receiver()) {
    for_each(VisualPhysicsActor* actor, selected_actors) {
      if (actor != actor_) {
        if (!actor->room_surface()->is_pinnable_receiver()
            && !actor->is_new_items_pile()
            && actor->parent() == NULL) {
          actor->updateActorStatusBeforeDrag();
          // We are updating the parent id of the actor receiving mouse down in its mouseDown function
          // because it's going to get removed from its parent somewhere before reaching here
          // So all we need to do here is update the parent id of all the other selected actors
          actor->updateActorParentInfoBeforeDrag();
          drag_partner_ids_.push_back(actor->unique_id());
          drag_partners_offsets_.insert(actor, actor_->position() - actor->position());
          drag_partners_collide_with_other_items_.insert(actor, !actor->collides_with_walls_only());
          drag_partners_poses_on_mouse_down_.insert(actor, actor->pose());
          drag_partners_surfaces_on_mouse_down_.insert(actor, actor->room_surface());
          actor->setCollisionsToOnlyWalls();
          actor->setFriction(0.5);
        } else {
          actor->set_selected(false);
        }
      }
    }
  } else {
    for_each(VisualPhysicsActor* actor, selected_actors) {
      if (actor != actor_)
        actor->set_selected(false);
    }
  }

  actor_pose_may_violate_surface_constraints_ = true;
  room_->activatePhysicsForAllActors();
  dragging_as_group_ = drag_partners_offsets_.count() > 0;

  if (kWallDragType == SNAP_TO_WALL) {
    actor_->setPhysicsConstraintsForSurface(actor_->room_surface());
  } else {
    if (actor_->pinnable() && !dragging_as_group_) {
      if (actor_->room_surface()->is_pinnable_receiver()) {
        actor_->removePhysicsConstraints();
      } else {
        actor_pose_may_violate_surface_constraints_ = false;
      }
    }
  }
}

bool DampedSpringMouseHandler::isMouseOverMenuBar() {
  Ogre::Vector2 mouse_in_window_space = BumpTopApp::singleton()->mouse_location();
  return mouse_in_window_space.y <= 0 && mouse_in_window_space.y >= -MENU_BAR_HEIGHT;
}

bool DampedSpringMouseHandler::shouldInitiateDragForAutoHideDock(Ogre::Vector2 mouse_in_window_space) {
  // TODO: this should probably be using a mouse position in screen space, not window space
  // to be more general
  if (AppSettings::singleton()->has_auto_hide_dock()) {
    if (AppSettings::singleton()->dock_position() == BOTTOM_DOCK
        && mouse_in_window_space.y > screen_resolution_.y - MENU_BAR_HEIGHT - 2) {
      return true;
    } else if (AppSettings::singleton()->dock_position() == LEFT_DOCK &&
               mouse_in_window_space.x < 2) {
      return true;
    } else if (AppSettings::singleton()->dock_position() == RIGHT_DOCK &&
               mouse_in_window_space.x > screen_resolution_.x - 2) {
      return true;
    }
  }
  return false;
}

void DampedSpringMouseHandler::mouseExit() {
  if (!drag_and_drop_started_) {
    if (!isMouseOverMenuBar()) {
      drag_and_drop_started_ = true;
      mouse_drag_has_exited_bumptop_ = true;
      QStringList paths_to_drag;
      for_each(VisualPhysicsActor *actor, room_->selected_actors()) {
        if (actor->children().empty()) {
          paths_to_drag.append(actor->path());
        } else {
          for_each(VisualPhysicsActor *child, actor->children()) {
            paths_to_drag.append(child->path());
          }
        }
      }
      if (paths_to_drag.size() > 0) {
        // **** important ***
        // initiateDrag is blocking unless called from a mouse event
        BumpTopApp::singleton()->drag_and_drop()->initiateDrag(paths_to_drag);
      }
    } else {
      drag_ignored_for_menu_bar_ = true;
    }
  }
  // dragExited seems to be called right after this anyway, so you get the "snap-back" animation
  // however, don't completely understand why dragExited is called right now
}

void DampedSpringMouseHandler::dragEntered() {
  std::pair<bool, Ogre::Vector3> new_position_result;
  Ogre::Vector3 world_position = actor_->world_position();
  mouse_drag_has_exited_bumptop_ = false;
}

void DampedSpringMouseHandler::dragExited() {
  mouse_drag_has_exited_bumptop_ = true;
}


void DampedSpringMouseHandler::dragAndDropUpdate(MouseEvent* mouse_event) {
  // don't initiate drag and drop unless it's moving slow
  if (is_droppable_ && (drop_receiver_ != NULL || actor_->linear_velocity().length() < 200.0)) {
    std::pair<DropReceiver*, NSDragOperation> DropReceiver_and_drag_operations;
    MouseEventManager *mouse_manager = BumpTopApp::singleton()->mouse_event_manager();

    NSDragOperation drag_operation = NSDragOperationMove | NSDragOperationCopy;
    if (FileManager::isVolume(actor_->path())) {
      drag_operation = NSDragOperationLink;
    }
    if (mouse_event->modifier_flags & ALT_OPTION_KEY_MASK) {
      drag_operation = NSDragOperationCopy;
    }

    // We want the primary drag actor to be at the front of the list of actors being dragged,
    // followed by the drag partners, in no specific order
    VisualPhysicsActorList drop_actors;
    for_each(VisualPhysicsActorId actor_id, drag_partner_ids_) {
      VisualPhysicsActor* actor = room_->actor_with_unique_id(actor_id);
      if (actor != NULL) {
        drop_actors.push_back(actor);
      }
    }
    drop_actors.push_front(actor_);

    DropReceiver_and_drag_operations = mouse_manager->draggingUpdated(mouse_event->mouse_in_window_space.x,
                                                                      mouse_event->mouse_in_window_space.y,
                                                                      drag_operation,
                                                                      drop_actors, true);

    if (DropReceiver_and_drag_operations.first != NULL && drop_receiver_ == NULL) {
      for_each(VisualPhysicsActor* actor, drop_actors) {
        resizeActor(actor, 0.75);
      }
    } else if (DropReceiver_and_drag_operations.first == NULL && drop_receiver_ != NULL) {
      // Need to end animations before getting the size scale factor, or the size might be out of date
      for_each(VisualPhysicsActor* actor, drop_actors) {
        AnimationManager::singleton()->endAnimationsForActor(actor, AnimationManager::STOP_AT_CURRENT_STATE);
        for_each(VisualPhysicsActor* child, actor->children()) {
          AnimationManager::singleton()->endAnimationsForActor(child, AnimationManager::STOP_AT_CURRENT_STATE);
        }
      }
      Ogre::Real scale_factor;
      if (actor_->children().isEmpty()) {
        scale_factor = (size_on_mouse_down_ / actor_->scale()).x;
      } else {
        scale_factor = (size_on_mouse_down_ / actor_->children()[0]->scale()).x;
      }
      for_each(VisualPhysicsActor* actor, drop_actors) {
        resizeActor(actor, scale_factor);
      }
    }

    if (drop_receiver_ != NULL && drop_receiver_ != DropReceiver_and_drag_operations.first) {
      drop_receiver_->draggingExited();
    }
    drop_receiver_ = DropReceiver_and_drag_operations.first;
    drag_operation_ = DropReceiver_and_drag_operations.second;

    if (drop_receiver_ != NULL && drag_operation_ == NSDragOperationCopy) {
      if (current_cursor_ != kCopyCursor) {
        SetThemeCursor(kThemeCopyArrowCursor);
        current_cursor_ = kCopyCursor;
      }
    } else if (drop_receiver_ != NULL && drag_operation_ == NSDragOperationLink) {
      if (current_cursor_ != kLinkCursor) {
        SetThemeCursor(kThemeAliasArrowCursor);
        current_cursor_ = kLinkCursor;
      }
    } else if (current_cursor_ != kRegularCursor) {
      SetThemeCursor(kThemeArrowCursor);
      current_cursor_ = kRegularCursor;
    }
  }
}

void DampedSpringMouseHandler::mouseDragged(MouseEvent* mouse_event) {
  mouse_event->handled = true;

  if (shouldInitiateDragForAutoHideDock(mouse_event->mouse_in_window_space) ||
      begin_drag_operation_on_next_mouse_drag_) {
    // here we want to emulate a mouse exit to initiate a drag operation
    mouseExit();
  }
}

void DampedSpringMouseHandler::mouseUp(MouseEvent* mouse_event) {
  BumpTopApp* bumptop = BumpTopApp::singleton();
  if (bumptop->mouse_event_manager()->global_capture() == actor_->visual_actor()) {
    bumptop->mouse_event_manager()->set_global_capture(NULL);
  }
  bumptop->disconnect(this);
  bumptop->mouse_event_manager()->disconnect(this);
  mouse_event->handled = true;
  drag_and_drop_started_ = false;

  room_->draggingItemsComplete();

  if (mouse_motion_registered_) {
    // if there's a drop target, call prepare drag operation, and then perform drag operation
    //   otherwise, animate back to the correct size, if necessary
    //   (we need to  disconnect from render events before this, or we might keep processing render ticks during
    //    performing the drag operation)
    bool slide_back_after_drag_and_drop_operation = false;
    bool performed_drag_operation = false;

    VisualPhysicsActorId actor_id_saved_on_stack = actor_id_;
    if (is_droppable_) {
      // one last update, in case we've moved off the target
      dragAndDropUpdate(mouse_event);

      VisualPhysicsActorList selected_actors = room_->selected_actors();
      // Need to end animations before getting the size scale factor, or the size might be out of date
      for_each(VisualPhysicsActor* actor, selected_actors) {
        AnimationManager::singleton()->endAnimationsForActor(actor, AnimationManager::STOP_AT_CURRENT_STATE);
        for_each(VisualPhysicsActor* child, actor->children()) {
          AnimationManager::singleton()->endAnimationsForActor(child, AnimationManager::STOP_AT_CURRENT_STATE);
        }
      }

      Ogre::Real size_scale_factor;

      if (actor_->children().isEmpty()) {
        size_scale_factor = (size_on_mouse_down_ / actor_->scale()).x;
      } else {
        size_scale_factor = (size_on_mouse_down_ / actor_->children()[0]->scale()).x;
      }

      if (size_scale_factor != 1.0f) {
        for_each(VisualPhysicsActor* actor, selected_actors) {
          resizeActor(actor, size_scale_factor);
        }
      }
      if (is_droppable_ && drop_receiver_ != NULL) {
        QStringList selected_actor_paths;
        for_each(VisualPhysicsActor* actor, selected_actors) {
          if (actor->path() != "") {
            selected_actor_paths.append(actor->path());
          }
          for_each(VisualPhysicsActor* child, actor->flattenedChildren()) {
            if (child->path() != "") {
              selected_actor_paths.append(child->path());
            }
          }
        }
        bool should_perform_operation = drop_receiver_->prepareForDragOperation(mouse_event->mouse_in_window_space,
                                                                                selected_actor_paths,
                                                                                drag_operation_,
                                                                                selected_actors);
        if (should_perform_operation) {
          performed_drag_operation = drop_receiver_->performDragOperation(mouse_event->mouse_in_window_space,
                                                                          selected_actor_paths,
                                                                          drag_operation_,
                                                                          selected_actors);
          slide_back_after_drag_and_drop_operation = !performed_drag_operation || (drag_operation_ != NSDragOperationMove);  // NOLINT
        }
      }

      if (current_cursor_ != kRegularCursor) {
        SetThemeCursor(kThemeArrowCursor);
      }
    }

    // just in case the drag operation involved deleting ourselves
    // dont use anything that actually requires this object to exist
    // TODO: do this differently... this is ugly
    if (!BumpTopApp::singleton()->scene()->room()->containsDirectlyOrThroughChild(actor_id_saved_on_stack)) {
      return;
    }
    if (drag_partners_collide_with_other_items_[actor_]) {
      actor_->setCollisionsToAllActors();
    }

    for_each(VisualPhysicsActorId actor_id, drag_partner_ids_) {
      VisualPhysicsActor* actor = room_->actor_with_unique_id(actor_id);
      if (actor != NULL) {
        actor->setCollisionsToAllActors();
      }
    }

    if ((performed_drag_operation && actor_->parent() != NULL) /* ie actor no longer at root level in room */) {
      // do nothing
    } else if (mouse_drag_has_exited_bumptop_ || slide_back_after_drag_and_drop_operation) {
      // we need to deal with the case where the item has been dragged out of bumptop
      // in this case, we animate back to the original position
      VisualPhysicsActorAnimation *actor_animation;
      AnimationManager::singleton()->endAnimationsForActor(actor_, AnimationManager::STOP_AT_CURRENT_STATE);

      actor_animation = new VisualPhysicsActorAnimation(actor_, 100,
                                                        pose_on_mouse_down_.position,
                                                        pose_on_mouse_down_.orientation,
                                                        surface_on_mouse_down_);
      actor_animation->start();
      for_each(VisualPhysicsActorId actor_id, drag_partner_ids_) {
        VisualPhysicsActor* actor = room_->actor_with_unique_id(actor_id);
        if (actor != NULL) {
          actor_animation = new VisualPhysicsActorAnimation(actor_, 100,
                                                            drag_partners_poses_on_mouse_down_[actor].position,
                                                            drag_partners_poses_on_mouse_down_[actor].orientation,
                                                            drag_partners_surfaces_on_mouse_down_[actor]);
          actor_animation->start();
        }
      }
    } else {
      // The standard case for a mouse-up
      if (actor_->room_surface()->is_pinnable_receiver()) {
        actor_->set_pose(actor_->getPoseForSurface(actor_->room_surface()));
        actor_->pinToSurface(actor_->room_surface());
      } else {  // (ie. the floor)
        Ogre::Vector3 linear_velocity = actor_->linear_velocity();
        Ogre::Vector3 angular_velocity = actor_->angular_velocity();

        if (room_->actor_with_unique_id(actor_->unique_id()) != NULL) {  // ie if its still in the room
          if (actor_->pinnable() && !dragging_as_group_ && actor_pose_may_violate_surface_constraints_)
            actor_->set_pose(actor_->getPoseForSurface(actor_->room_surface()));
          else
            actor_->set_pose(getActorPoseConstrainedToRoomAndNoIntersections(actor_, room_));
        }

        actor_->set_linear_velocity(linear_velocity*Ogre::Vector3(1 , 0, 1));
        actor_->set_angular_velocity(angular_velocity);
        actor_->setPhysicsConstraintsForSurface(actor_->room_surface());

        for_each(VisualPhysicsActorId actor_id, drag_partner_ids_) {
          VisualPhysicsActor* actor = room_->actor_with_unique_id(actor_id);
          if (actor != NULL) {
            Ogre::Vector3 linear_velocity = actor->linear_velocity();
            Ogre::Vector3 angular_velocity = actor->angular_velocity();

            if (room_->actor_with_unique_id(actor->unique_id()) != NULL) {  // ie if its still in the room
              actor->set_pose(getActorPoseConstrainedToRoomAndNoIntersections(actor, room_));
            }

            actor->set_linear_velocity(linear_velocity);
            actor->set_angular_velocity(angular_velocity);
            actor->setPhysicsConstraintsForSurface(actor->room_surface());
            actor->setFriction(DEFAULT_FRICTION);
          }
        }
      }

      stab_point_object_space_ = Ogre::Vector3::ZERO;

      if (is_droppable_ && !performed_drag_operation &&
          actor_->actor_parent_id_before_drag() != 0) {
        InternalDragAndDropUndoCommand* drag_and_drop_undo_command = new InternalDragAndDropUndoCommand(room_->selected_actors(),  // NOLINT
                                                                                                        0,
                                                                                                        room_);
        room_->undo_redo_stack()->push(drag_and_drop_undo_command, room_->current_state(), true);
      }
    }
    actor_->setFriction(DEFAULT_FRICTION);
    for_each(VisualPhysicsActorId actor_id, drag_partner_ids_) {
      VisualPhysicsActor* actor = room_->actor_with_unique_id(actor_id);
      if (actor != NULL) {
        actor->setFriction(DEFAULT_FRICTION);
      }
    }
  }

  // This stuff just ensures that an extra mouse-up without a corresponding mouse-down won't
  // result in things happening that shouldn't
  drag_partner_ids_.clear();
  drag_partners_offsets_.clear();
  drag_partners_collide_with_other_items_.clear();
  drag_partners_poses_on_mouse_down_.clear();
  drag_partners_surfaces_on_mouse_down_.clear();
  mouse_motion_registered_ = false;
  mouse_drag_has_exited_bumptop_ = false;
}

BumpPose DampedSpringMouseHandler::getConstrainedPoseForDragging(VisualPhysicsActor* actor,
                                                                 Ogre::Vector3 desired_dragging_position) {
  Ogre::Vector3 linear_velocity = actor->linear_velocity();
  Ogre::Vector3 angular_velocity = actor->angular_velocity();
  BumpPose orig_pose = actor->pose();
  actor->set_position(desired_dragging_position);
  // TODO: May have issues for back wall
  actor->set_orientation(actor->room_surface()->orientation());
  BumpPose new_pose = actor->getPoseConstrainedToRoomAndNoIntersections();
  actor->set_pose(orig_pose);
  actor->set_linear_velocity(linear_velocity);
  actor->set_angular_velocity(angular_velocity);
  return new_pose;
}

void DampedSpringMouseHandler::renderUpdate() {
  std::pair<bool, Ogre::Vector3> new_position_result;

  Ogre::Vector2 mouse_in_window_space = BumpTopApp::singleton()->mouse_location();
  if (mouse_in_window_space != last_mouse_in_window_space_) {
    if (!mouse_motion_registered_ &&
        (mouse_in_window_space - last_mouse_in_window_space_).length() > kMouseHasMovedThreshold) {
      mouse_motion_registered_ = true;
      actor_->mouseMotionRegistered();
      beginDraggingItems();
    }
  }

  if (mouse_motion_registered_) {
    int key_modifiers;
    if (BumpTopApp::singleton()->keyboard_event_manager()->option_key_down()) {
      key_modifiers = ALT_OPTION_KEY_MASK;
    } else {
      key_modifiers = NO_KEY_MODIFIERS_MASK;
    }
    MouseEvent mouse_event = MouseEvent(mouse_in_window_space, false, NULL, Ogre::Vector3::ZERO, 0,
                                        key_modifiers, false, false);
    dragAndDropUpdate(&mouse_event);

    if (drag_ignored_for_menu_bar_ && mouse_in_window_space.y < -MENU_BAR_HEIGHT && !drag_and_drop_started_) {
      begin_drag_operation_on_next_mouse_drag_ = true;
    }

    if (mouse_drag_has_exited_bumptop_)
      mouse_in_window_space = last_mouse_in_window_space_;

    last_mouse_in_window_space_ = mouse_in_window_space;
    Ogre::Matrix4 world_transform = actor_->transform();
    assert(world_transform.isAffine());
    Ogre::Matrix4 world_transform_inverse = world_transform.inverseAffine();

    Ogre::Vector3 stab_point_delta = world_transform*stab_point_object_space_ - actor_->world_position();

    RoomSurface *surface = actor_->room_surface();
    Ogre::Vector3 stab_point_world_space = world_transform*stab_point_object_space_;
    Ogre::Vector3 perspective_delta_for_drag_partners;
    Ogre::Vector3 mouse_floor_position;

    BumpPose new_pose;
    Ogre::Real mouse_height;
    if (!actor_->room_surface()->is_pinnable_receiver()) {
      new_position_result = surface->mouseIntersectionAbove(mouse_in_window_space,
                                                            actor_->size().y/2.0,
                                                            IGNORE_SURFACE_BOUNDS);
      new_pose = getConstrainedPoseForDragging(actor_, new_position_result.second
                                               - stab_point_delta*Ogre::Vector3(1, 0, 1));
      mouse_floor_position = new_position_result.second;
      mouse_height = surface->distanceAbove(new_pose.position);
    } else {
      mouse_height = actor_->size().y/2.0;
    }

    if (kWallDragType == SNAP_TO_WALL)
      new_position_result = surface->mouseIntersectionAbove(mouse_in_window_space,
                                                            surface->distanceAbove(stab_point_world_space));
    else  // if (wall_drag_type == PHYSICAL_DRAG)
      new_position_result = surface->mouseIntersectionAbove(mouse_in_window_space,
                                                            stab_point_object_space_.y + mouse_height,
                                                            dragging_as_group_ ? IGNORE_SURFACE_BOUNDS : ENFORCE_SURFACE_BOUNDS);  // NOLINT

    perspective_delta_for_drag_partners = (Ogre::Vector3(1, 0, 1)*new_position_result.second - mouse_floor_position);

    if (new_position_result.first) {
      Ogre::Vector3 new_item_position = new_position_result.second;
      const Ogre::Vector3 k_damping = sqrt(PHYSICS_SCALE)*Ogre::Vector3(5, 7, 5);
      const Ogre::Vector3 k_spring = Ogre::Vector3(10.0, 15.0, 10.0);
      if (dragging_as_group_)
        new_item_position = getPositionConstrainedToRoom(new_item_position, room_);

      Ogre::Vector3 force = actor_->mass()*(k_spring*(new_item_position - stab_point_world_space)
                                            - k_damping*actor_->physics_actor()->linear_velocity());
      actor_->activatePhysics();
      actor_->physics_actor()->applyCentralForce(force);

      for_each(VisualPhysicsActorId actor_id, drag_partner_ids_) {
        VisualPhysicsActor* actor = room_->actor_with_unique_id(actor_id);
        if (actor != NULL) {
          new_pose = getConstrainedPoseForDragging(actor, Ogre::Vector3(new_item_position*Ogre::Vector3(1, 0, 1)
                                                                        - stab_point_delta
                                                                        - perspective_delta_for_drag_partners
                                                                        - drag_partners_offsets_[actor]));
          Ogre::Vector3 offset_new_item_position = getPositionConstrainedToRoom(new_pose.position, room_);
          Ogre::Vector3 offset_force = actor->mass()*(k_spring*(offset_new_item_position - actor->world_position())
                                                      - k_damping*actor->physics_actor()->linear_velocity());
          actor->activatePhysics();
          actor->physics_actor()->applyCentralForce(offset_force);
        }
      }
    } else {
      // User is dragging the object onto a new surface
      if (actor_->pinnable()) {
        if (!actor_pose_may_violate_surface_constraints_) {
          actor_pose_may_violate_surface_constraints_ = true;
          actor_->removePhysicsConstraints();
        }

        QList<RoomSurface*> room_surfaces = room_->surfaces();
        for_each(RoomSurface* surface, room_surfaces) {
          if (surface->mouseIntersection(mouse_in_window_space).first) {
              actor_->set_room_surface(surface);
            if (kWallDragType == SNAP_TO_WALL) {
              actor_->setPhysicsConstraintsForSurface(surface);
              BumpPose pose = actor_->getPoseForSurface(surface);
              AnimationManager::singleton()->endAnimationsForActor(actor_, AnimationManager::MOVE_TO_FINAL_STATE);
              VisualPhysicsActorAnimation* actor_animation = new VisualPhysicsActorAnimation(actor_, 30,
                                                                                             pose.position,
                                                                                             pose.orientation);
              actor_animation->start();
            }
            break;
          }
        }
      }
    }
  }
}

#include "BumpTop/moc/moc_DampedSpringMouseHandler.cpp"
