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

#include "BumpTop/UndoCommands/ShrinkUndoCommand.h"

#include "BumpTop/AppSettings.h"
#include "BumpTop/AnimationManager.h"
#include "BumpTop/BumpBoxLabel.h"
#include "BumpTop/BumpDummy.h"
#include "BumpTop/Room.h"
#include "BumpTop/UndoRedoStack.h"
#include "BumpTop/VisualPhysicsActor.h"
#include "BumpTop/VisualPhysicsActorAnimation.h"

ShrinkUndoCommand::ShrinkUndoCommand(VisualPhysicsActorList actors_to_shrink, Room* room)
: room_(room),
  first_run_(true) {
  actors_to_shrink_ = QList<VisualPhysicsActorId>();
  for_each(VisualPhysicsActor* actor, actors_to_shrink) {
    actors_to_shrink_.push_back(actor->unique_id());
    actors_size_was_changed_.insert(actor->unique_id(), true);
  }
}

ShrinkUndoCommand::~ShrinkUndoCommand() {
}

void ShrinkUndoCommand::undo() {
  if (!did_redo_change_something_) {
    UndoRedoStack::last_command_changed_something = false;
    return;
  }

  bool is_anything_grown = false;

  for_each(VisualPhysicsActorId actor_id, actors_to_shrink_) {
    VisualPhysicsActor* actor = room_->actor_with_unique_id(actor_id);
    if (actor != NULL) {
      if (actors_size_was_changed_[actor_id]) {
        if (actor->children().empty()) {
          if (Ogre::Vector3(MAX_ACTOR_SIZE) > VPA_GROW_FACTOR*actor->scale()) {
            updateLabelSize(actor, VPA_GROW_FACTOR);
            is_anything_grown = true;
            AnimationManager::singleton()->endAnimationsForActor(actor, AnimationManager::STOP_AT_CURRENT_STATE);  // NOLINT
            VisualPhysicsActorAnimation* actor_animation;
            actor_animation = new VisualPhysicsActorAnimation(actor, 600,
                                                              actor->position(),
                                                              actor->orientation(),
                                                              actor->room_surface(), 1,
                                                              VPA_GROW_FACTOR,
                                                              tween::ELASTIC);
            actor_animation->start();
            actor->set_selected(true);
          }
        } else {
          if (willShrinkCommandChangeSomething(actor->children(), VPA_GROW_FACTOR)) {
            actor->stackViewAndScaleChildren(VPA_GROW_FACTOR);
            updateLabelSize(actor, VPA_GROW_FACTOR);
            is_anything_grown = true;
          }
        }
      }
    }
  }

  UndoRedoStack::last_command_changed_something = is_anything_grown;
}

void ShrinkUndoCommand::redo() {
  did_redo_change_something_ = false;

  for_each(VisualPhysicsActorId actor_id, actors_to_shrink_) {
    VisualPhysicsActor* actor = room_->actor_with_unique_id(actor_id);
    if (actor != NULL) {
      if (actor->children().empty()) {
        if (Ogre::Vector3(MIN_ACTOR_SIZE) < (1/VPA_GROW_FACTOR)*actor->scale()) {
          updateLabelSize(actor, 1/VPA_GROW_FACTOR);
          did_redo_change_something_ = true;
          AnimationManager::singleton()->endAnimationsForActor(actor, AnimationManager::STOP_AT_CURRENT_STATE);  // NOLINT
          VisualPhysicsActorAnimation* actor_animation;
          actor_animation = new VisualPhysicsActorAnimation(actor, 600,
                                                            actor->position(),
                                                            actor->orientation(),
                                                            actor->room_surface(), 1,
                                                            1/VPA_GROW_FACTOR,
                                                            tween::ELASTIC);
          actor_animation->start();
          actor->set_selected(true);
        } else {
          actors_size_was_changed_[actor_id] = false;
        }
      } else  {
        if (willShrinkCommandChangeSomething(actor->children(), 1/VPA_GROW_FACTOR)) {
          actor->stackViewAndScaleChildren(1/VPA_GROW_FACTOR);
          updateLabelSize(actor, 1/VPA_GROW_FACTOR);
          did_redo_change_something_ = true;
        } else {
          actors_size_was_changed_[actor_id] = false;
        }
      }
    }
  }
  first_run_ = false;

  UndoRedoStack::last_command_changed_something = did_redo_change_something_;
}

bool ShrinkUndoCommand::willShrinkCommandChangeSomething(VisualPhysicsActorList actors, Ogre::Real scale_factor) {
  for_each(VisualPhysicsActor* actor, actors) {
    if (Ogre::Vector3(MAX_ACTOR_SIZE) < scale_factor*actor->scale()
        || Ogre::Vector3(MIN_ACTOR_SIZE) > scale_factor*actor->scale()
        || actor->actor_type() == BUMP_DUMMY
        && Ogre::Vector3(MIN_ACTOR_SIZE) > scale_factor*actor->scale()/kDummySizeFactor) {  // NOLINT
      return false;
    }
  }
  return true;
}

void ShrinkUndoCommand::updateLabelSize(VisualPhysicsActor* actor, Ogre::Real size_factor) {
  if (actor->label() != NULL) {
    Ogre::Real ratio_of_actor_current_size_vs_initial_size = 1 + ((actor->size().x * size_factor) / kInitialActorSize - 1)/2;  // NOLINT
    actor->updateLabel(ratio_of_actor_current_size_vs_initial_size);
  }
}
