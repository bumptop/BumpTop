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

#include "BumpTop/UndoCommands/InternalDragAndDropUndoCommand.h"

#include "BumpTop/AnimationManager.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/GriddedPile.h"
#include "BumpTop/Room.h"
#include "BumpTop/RoomSurface.h"
#include "BumpTop/UndoCommands/AddToPileUndoCommand.h"
#include "BumpTop/UndoCommands/PileizeUndoCommand.h"
#include "BumpTop/UndoRedoStack.h"
#include "BumpTop/VisualPhysicsActorAnimation.h"

InternalDragAndDropUndoCommand::InternalDragAndDropUndoCommand(VisualPhysicsActorList actors,
                                                               VisualPhysicsActorId receiver_id,
                                                               Room* room)
: first_run_(true),
  receiver_id_(receiver_id),
  room_(room) {
  for_each(VisualPhysicsActor* actor, actors) {
    // dragged_actors_ids_ is a qlist of all the dragged actors' ids involved in this drag and drop operation
    dragged_actors_ids_.append(actor->unique_id());

    actors_ids_to_undo_state_poses_[actor->unique_id()] = actor->actor_pose_before_drag();
    actors_ids_to_undo_state_room_surfaces_[actor->unique_id()] = actor->actor_room_surface_before_drag();
    actors_ids_to_undo_state_actors_types_[actor->unique_id()] = actor->actor_type();
    // actors_ids_to_undo_state_parents_ids_ is a qhash with each of the dragged actors' ids as
    // its keys and its parent id as values
    actors_ids_to_undo_state_parents_ids_[actor->unique_id()] = actor->actor_parent_id_before_drag();

    actors_ids_to_redo_state_room_surfaces_[actor->unique_id()] = actor->room_surface();

    if (actor->actor_type() != BUMP_PILE && actor->actor_type() != BUMP_BOX) {
      // We do not suppose drag and drop operations if the actors are not bump boxes or bump piles
      assert(false);
    } else if (actor->actor_type() == BUMP_PILE) {
      // If the dragged actor is a pile, we want to store its children ids and its offset and undo state poses
      // to the pile
      undo_state_piles_ids_to_children_ids_[actor->unique_id()] = actor->flattenedChildrenIds();

      for_each(VisualPhysicsActor* child, actor->flattenedChildren()) {
        piles_ids_to_undo_state_children_offset_poses_[child->unique_id()] = child->actor_offset_pose_to_its_parent();
      }
    } else {  // actor->actor_type() == BUMP_BOX
      if (actors_ids_to_undo_state_parents_ids_[actor->unique_id()] != 0) {
        // This is for dealing with dragging an actor out of a pile.
        // In this case we want to store its offset to its undo state parent
        VisualPhysicsActorId parent_id = actors_ids_to_undo_state_parents_ids_[actor->unique_id()];
        piles_ids_to_undo_state_children_offset_poses_[actor->unique_id()] = actor->actor_offset_pose_to_its_parent();
        actors_ids_to_undo_state_siblings_offset_poses_[actor->unique_id()] = actor->actor_siblings_offset_poses_to_parent();  // NOLINT
        undo_state_parents_ids_to_parents_types_[parent_id] = actor->actor_parent_type_before_drag();
        undo_state_parents_ids_to_parents_positions_[parent_id] = actor->actor_parent_position_before_drag();
      }
    }

    if (receiver_id_ == 0) {
      // receiver_id == 0 indicate that we are dragging actors out of a pile to somewhere in the room
      // In this case we want to store where the actor endded up at so we could redo it to this state
      actors_ids_to_redo_state_poses_[actor->unique_id()] = actor->pose();
    }
  }
}

InternalDragAndDropUndoCommand::~InternalDragAndDropUndoCommand() {
}

void InternalDragAndDropUndoCommand::undo() {
  if (receiver_id_ != 0) {
    // receiver_id != 0 means that we dragged actorss into a pile or gridded pile
    // In this case, we have to break the actors out of pile first
    QList<VisualPhysicsActorId> actors_ids_to_remove;
    VisualPhysicsActor* receiver = room_->actor_with_unique_id(receiver_id_);

    if (receiver != NULL) {
      for_each(VisualPhysicsActor* child, receiver->flattenedChildren()) {
        if (dragged_actors_ids_.contains(child->unique_id())) {
          actors_ids_to_remove.append(child->unique_id());
        } else {
          // Since adding piles to a pile or gridded pile will cause the added piles to be broken,
          // we need search for the undo_state piles' children in the receiver and break those instead
          for_each(QList<VisualPhysicsActorId> children_ids, undo_state_piles_ids_to_children_ids_.values()) {
            if (children_ids.contains(child->unique_id())) {
              actors_ids_to_remove.append(child->unique_id());
            }
          }
        }
      }
      receiver->breakItemsOutOfPile(actors_ids_to_remove);
    } else {
      UndoRedoStack::last_command_changed_something = false;
      return;
    }
  }

  for_each(VisualPhysicsActorId actor_id, dragged_actors_ids_) {
    VisualPhysicsActor* actor = room_->actor_with_unique_id(actor_id);

    if (actor != NULL && actors_ids_to_undo_state_actors_types_[actor_id] != BUMP_BOX &&
        actors_ids_to_undo_state_actors_types_[actor_id] != BUMP_PILE) {
      // We dont support dragging anything other than bump boxes and bump piles
      assert(false);
    } else if (actors_ids_to_undo_state_actors_types_[actor_id] == BUMP_BOX) {
      if (actor != NULL) {
        // Since our undo and redo doesn't contain the actual dragging of actors, thus when we are
        // dealing with actors on the wall, its room surface and constraints are unchanged
        // So we need to set the surfaces and constraints ourselfves and for actors that will be
        // put back to the wall, we are assuming that VisualPhysicsActorAnimation will set the
        // surfaces and constraints once the animation completes
        actor->set_room_surface(room_->getSurface(FLOOR));
        actor->setPhysicsConstraintsForSurface(actors_ids_to_undo_state_room_surfaces_[actor_id]);
      }

      if (actors_ids_to_undo_state_parents_ids_[actor_id] != 0) {
        // This indicate that the actor was part of a pile or gridded pile before.
        // In this case we want to add the actor back to its undo state parent
        VisualPhysicsActorId parent_id = actors_ids_to_undo_state_parents_ids_[actor_id];
        VisualPhysicsActor* parent = room_->actor_with_unique_id(parent_id);
        if (actor != NULL) {
          if (parent != NULL) {
            VisualPhysicsActorList pile_members;
            pile_members.append(actor);
            AddToPileUndoCommand* add_to_pile_undo_command = new AddToPileUndoCommand(parent, pile_members, room_,
                                                                                      BumpTopApp::singleton()->ogre_scene_manager(),  // NOLINT
                                                                                      BumpTopApp::singleton()->physics());  // NOLINT
            add_to_pile_undo_command->redo();

            QHash<VisualPhysicsActorId, BumpPose> children_ids_to_offset_poses;
            children_ids_to_offset_poses[actor_id] = piles_ids_to_undo_state_children_offset_poses_[actor_id];
            parent->set_children_offset_pose(children_ids_to_offset_poses);
          } else {
            // Dragging the actor out of its original pile could cause the pile to be broken thus
            // here we want to restore the pile
            VisualPhysicsActorList siblings;
            QHash<VisualPhysicsActorId, BumpPose> siblings_ids_to_offset_poses;

            for_each(VisualPhysicsActorId sibling_id, actors_ids_to_undo_state_siblings_offset_poses_[actor_id].keys()) {  // NOLINT
              VisualPhysicsActor* sibling = room_->actor_with_unique_id(sibling_id);
              if (sibling != NULL) {
                siblings.append(sibling);
                siblings_ids_to_offset_poses[sibling_id] = actors_ids_to_undo_state_siblings_offset_poses_[actor_id][sibling_id];  // NOLINT
              }
            }

            if (siblings.count() >= 1) {
              // recreate the pile if more than 1 of the piles children exists other than the one dragged out
              siblings.append(actor);
              siblings_ids_to_offset_poses[actor_id] = piles_ids_to_undo_state_children_offset_poses_[actor_id];

              if (undo_state_parents_ids_to_parents_types_[parent_id] == BUMP_PILE) {
                PileizeUndoCommand* pileize_command = new PileizeUndoCommand(siblings,
                                                                             room_,
                                                                             BumpTopApp::singleton()->ogre_scene_manager(),  // NOLINT
                                                                             BumpTopApp::singleton()->physics(),
                                                                             undo_state_parents_ids_to_parents_positions_[parent_id],  // NOLINT
                                                                             "",
                                                                             parent_id);
                pileize_command->redo();
                room_->actor_with_unique_id(parent_id)->set_children_offset_pose(siblings_ids_to_offset_poses);
                room_->actor_with_unique_id(parent_id)->set_selected(false);
              } else if (undo_state_parents_ids_to_parents_types_[parent_id] == GRIDDED_PILE) {
                GriddedPile* gridded_pile = new GriddedPile(BumpTopApp::singleton()->ogre_scene_manager(),
                                                            BumpTopApp::singleton()->physics(),
                                                            room_, undo_state_parents_ids_to_parents_positions_[parent_id],  // NOLINT
                                                            parent_id);
                QList<Ogre::Vector3> offsets;
                QList<Ogre::Quaternion> orientations;
                for_each(VisualPhysicsActorId sibling_id, siblings_ids_to_offset_poses.keys()) {
                  offsets.append(siblings_ids_to_offset_poses[sibling_id].position);
                  orientations.append(siblings_ids_to_offset_poses[sibling_id].orientation);
                }
                gridded_pile->initWithActors(siblings, offsets,
                                             orientations, undo_state_parents_ids_to_parents_positions_[parent_id]);
              }
            }
          }
        }
      } else {
        // This indicate that the actor was not of any form of piles so we just need to
        // animate the actor back to where it was dragged from
        AnimationManager::singleton()->endAnimationsForActor(actor, AnimationManager::STOP_AT_CURRENT_STATE);
        VisualPhysicsActorAnimation* actor_animation;
        actor_animation = new VisualPhysicsActorAnimation(actor, 250,
                                                          actors_ids_to_undo_state_poses_[actor_id].position,
                                                          actors_ids_to_undo_state_poses_[actor_id].orientation,
                                                          actors_ids_to_undo_state_room_surfaces_[actor_id]);
        actor_animation->start();
      }
    } else {  // actors_ids_to_undo_state_actors_types_[actor_id] == BUMP_PILE
      // For dealing with piles we want to recreate the pile using pileize command
      // and restore its children's offsets and undo state poses
      VisualPhysicsActorList children;
      QHash<VisualPhysicsActorId, BumpPose> children_ids_to_offset_poses;
      for_each(VisualPhysicsActorId child_id, undo_state_piles_ids_to_children_ids_[actor_id]) {
        VisualPhysicsActor* child = room_->actor_with_unique_id(child_id);
        if (child != NULL) {
          children.append(child);
          children_ids_to_offset_poses[child_id] = piles_ids_to_undo_state_children_offset_poses_[child_id];
        }
      }

      if (children.count() >= 2) {
        // We only want to create the pile if there's at least 2 children of the pile still exists
        PileizeUndoCommand* pileize_command = new PileizeUndoCommand(children,
                                                                     room_,
                                                                     BumpTopApp::singleton()->ogre_scene_manager(),
                                                                     BumpTopApp::singleton()->physics(),
                                                                     actors_ids_to_undo_state_poses_[actor_id].position,
                                                                     "",
                                                                     actor_id);
        pileize_command->redo();
        room_->actor_with_unique_id(actor_id)->set_children_offset_pose(children_ids_to_offset_poses);
        room_->actor_with_unique_id(actor_id)->set_selected(false);
      } else if (children.count() == 1) {
        // The pile only having 1 valid child is caused by its children being deleted or moved in finder
        // In this case, we will animation the child to where the pile was and add it to our QHashes so
        // that it would be included in the undo redo commands afterwards
        AnimationManager::singleton()->endAnimationsForActor(children[0], AnimationManager::STOP_AT_CURRENT_STATE);
        VisualPhysicsActorAnimation* actor_animation;
        actor_animation = new VisualPhysicsActorAnimation(children[0], 250,
                                                          actors_ids_to_undo_state_poses_[actor_id].position,
                                                          actors_ids_to_undo_state_poses_[actor_id].orientation,
                                                          actors_ids_to_undo_state_room_surfaces_[actor_id]);
        actor_animation->start();

        dragged_actors_ids_.append(children[0]->unique_id());

        actors_ids_to_undo_state_poses_[children[0]->unique_id()] = actors_ids_to_undo_state_poses_[actor_id];
        actors_ids_to_undo_state_room_surfaces_[children[0]->unique_id()] = room_->getSurface(FLOOR);
        actors_ids_to_undo_state_actors_types_[children[0]->unique_id()] = BUMP_BOX;
        actors_ids_to_undo_state_parents_ids_[children[0]->unique_id()] = 0;

        actors_ids_to_redo_state_room_surfaces_[children[0]->unique_id()] = room_->getSurface(FLOOR);
      }
    }
  }
}

void InternalDragAndDropUndoCommand::redo() {
  if (first_run_) {
    // On first run, drgging and moving the actors were already handled by DampSpringMouseHandler
    // thus we only need to handle the case the actors endded up in a drop receiver
    first_run_ = false;
    if (receiver_id_ != 0) {
      VisualPhysicsActor* receiver = room_->actor_with_unique_id(receiver_id_);
      VisualPhysicsActorList pile_members;

      for_each(VisualPhysicsActorId actor_id, dragged_actors_ids_) {
        VisualPhysicsActor* actor = room_->actor_with_unique_id(actor_id);
        if (actor != NULL)
          pile_members.append(actor);
      }

      AddToPileUndoCommand* add_to_pile_undo_command = new AddToPileUndoCommand(receiver, pile_members, room_,
                                                                                BumpTopApp::singleton()->ogre_scene_manager(),  // NOLINT
                                                                                BumpTopApp::singleton()->physics());
      add_to_pile_undo_command->redo();

      QHash<VisualPhysicsActorId, BumpPose> children_ids_to_offset_poses;
      for_each(VisualPhysicsActor* pile_member, pile_members) {
        if (piles_ids_to_undo_state_children_offset_poses_.keys().contains(pile_member->unique_id())) {
          children_ids_to_offset_poses[pile_member->unique_id()] = piles_ids_to_undo_state_children_offset_poses_[pile_member->unique_id()];  // NOLINT
        }
      }
      receiver->set_children_offset_pose(children_ids_to_offset_poses);
    }
  } else {
    if (receiver_id_ != 0) {
      VisualPhysicsActor* receiver = room_->actor_with_unique_id(receiver_id_);
      if (receiver == NULL) {
        UndoRedoStack::last_command_changed_something = false;
        return;
      }
    }
    // After the first run, we need to simulate the drag performed by DampSpringMouseHandler
    for_each(VisualPhysicsActorId actor_id, dragged_actors_ids_) {
      VisualPhysicsActor* actor = NULL;
      VisualPhysicsActorId parent_id = actors_ids_to_undo_state_parents_ids_[actor_id];

      if (parent_id != 0) {
        // If the actor's undo state parent id isn't 0, then we need to break it out of
        // its parent
        VisualPhysicsActor* parent = room_->actor_with_unique_id(parent_id);
        if (parent != NULL) {
          QList<VisualPhysicsActorId> actor_id_to_remove;
          for_each(VisualPhysicsActor* child, parent->flattenedChildren()) {
            if (child->unique_id() == actor_id) {
              actor = child;
              actor_id_to_remove.append(actor_id);
            }
          }
          parent->breakItemsOutOfPile(actor_id_to_remove);
        }
      } else {
        actor = room_->actor_with_unique_id(actor_id);
      }

      if (actor != NULL) {
        // Since our undo and redo doesn't contain the actual dragging of actors, thus when we are
        // dealing with actors on the wall, its room surface and constraints are unchanged
        // So we need to set the surfaces and constraints ourselfves and for actors that will be
        // put back to the wall, we are assuming that VisualPhysicsActorAnimation will set the
        // surfaces and constraints once the animation completes
        actor->set_room_surface(room_->getSurface(FLOOR));
        actor->setPhysicsConstraintsForSurface(actors_ids_to_redo_state_room_surfaces_[actor_id]);

        if (receiver_id_ != 0) {
          // If we have receiver then we are going to add each of the dragged actors to it
          VisualPhysicsActorList pile_members;
          pile_members.append(actor);

          AddToPileUndoCommand* add_to_pile_undo_command = new AddToPileUndoCommand(room_->actor_with_unique_id(receiver_id_),  // NOLINT
                                                                                    pile_members, room_,
                                                                                    BumpTopApp::singleton()->ogre_scene_manager(),  // NOLINT
                                                                                    BumpTopApp::singleton()->physics());
          add_to_pile_undo_command->redo();

          QHash<VisualPhysicsActorId, BumpPose> children_ids_to_offset_poses;
          if (piles_ids_to_undo_state_children_offset_poses_.keys().contains(actor_id)) {
            children_ids_to_offset_poses[actor_id] = piles_ids_to_undo_state_children_offset_poses_[actor_id];
            room_->actor_with_unique_id(receiver_id_)->set_children_offset_pose(children_ids_to_offset_poses);
          }
        } else {
          // If we don't have a receiver, then we are going to animate them to its redo_state
          AnimationManager::singleton()->endAnimationsForActor(actor, AnimationManager::STOP_AT_CURRENT_STATE);
          VisualPhysicsActorAnimation* actor_animation;
          actor_animation = new VisualPhysicsActorAnimation(actor, 250,
                                                            actors_ids_to_redo_state_poses_[actor_id].position,
                                                            actors_ids_to_redo_state_poses_[actor_id].orientation,
                                                            actors_ids_to_redo_state_room_surfaces_[actor->unique_id()]);  // NOLINT
          actor_animation->start();
        }
      } else if (actors_ids_to_undo_state_actors_types_[actor_id] == BUMP_PILE && receiver_id_ != 0)  {
        VisualPhysicsActorList children;
        for_each(VisualPhysicsActorId child_id, undo_state_piles_ids_to_children_ids_[actor_id]) {
          VisualPhysicsActor* child = room_->actor_with_unique_id(child_id);
          if (child != NULL) {
            children.append(child);
          }
        }

        if (children.count() == 1) {
          // The pile only having 1 valid child is caused by its children being deleted or moved in finder
          // In this case, we will animation the child to where the pile was and add it to our QHashes so
          // that it would be included in the undo redo commands afterwards
          VisualPhysicsActorList pile_members;
          pile_members.append(children[0]);

          AddToPileUndoCommand* add_to_pile_undo_command = new AddToPileUndoCommand(room_->actor_with_unique_id(receiver_id_),  // NOLINT
                                                                                    pile_members, room_,
                                                                                    BumpTopApp::singleton()->ogre_scene_manager(),  // NOLINT
                                                                                    BumpTopApp::singleton()->physics());
          add_to_pile_undo_command->redo();

          QHash<VisualPhysicsActorId, BumpPose> children_ids_to_offset_poses;
          if (piles_ids_to_undo_state_children_offset_poses_.keys().contains(children[0]->unique_id())) {
            children_ids_to_offset_poses[actor_id] = piles_ids_to_undo_state_children_offset_poses_[children[0]->unique_id()];  // NOLINT
            room_->actor_with_unique_id(receiver_id_)->set_children_offset_pose(children_ids_to_offset_poses);
          }

          dragged_actors_ids_.append(children[0]->unique_id());

          actors_ids_to_undo_state_poses_[children[0]->unique_id()] = actors_ids_to_undo_state_poses_[actor_id];
          actors_ids_to_undo_state_room_surfaces_[children[0]->unique_id()] = room_->getSurface(FLOOR);
          actors_ids_to_undo_state_actors_types_[children[0]->unique_id()] = BUMP_BOX;
          actors_ids_to_undo_state_parents_ids_[children[0]->unique_id()] = 0;

          actors_ids_to_redo_state_room_surfaces_[children[0]->unique_id()] = room_->getSurface(FLOOR);
        }
      }
    }
  }
}
