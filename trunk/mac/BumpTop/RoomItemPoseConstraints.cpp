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

#include "BumpTop/RoomItemPoseConstraints.h"

#include <utility>

#include "BumpTop/AnimationManager.h"
#include "BumpTop/Math.h"
#include "BumpTop/Room.h"
#include "BumpTop/RoomSurface.h"
#include "BumpTop/VisualPhysicsActor.h"
#include "BumpTop/VisualPhysicsActorAnimation.h"

void tightenBoundingBox(Ogre::AxisAlignedBox* box) {
  // TODO: change "1" to the margin
  box->setMinimumX(box->getMinimum().x + 1);
  box->setMinimumY(box->getMinimum().y + 1);
  box->setMinimumZ(box->getMinimum().z + 1);
  box->setMaximumX(box->getMaximum().x - 1);
  box->setMaximumY(box->getMaximum().y - 1);
  box->setMaximumZ(box->getMaximum().z - 1);
}

QHash<VisualPhysicsActor*, BumpPose> getActorPosesConstrainedToRoom(QHash<VisualPhysicsActor*, BumpPose> requested_actor_poses, Room* room) {  // NOLINT
  QHash<VisualPhysicsActor*, BumpPose> original_actor_poses;
  QHash<VisualPhysicsActor*, Ogre::Vector3> original_actor_linear_velocities;
  QHash<VisualPhysicsActor*, Ogre::Vector3> original_actor_angular_velocities;

  // Move the items which are requesting new positions into those positions, and store their current positions
  // (this move is temporary, just to let us figure out how stuff will look in the final state)
  for_each(VisualPhysicsActor* actor, requested_actor_poses.keys()) {
    original_actor_poses.insert(actor, actor->pose());
    original_actor_linear_velocities.insert(actor, actor->linear_velocity());
    original_actor_angular_velocities.insert(actor, actor->angular_velocity());
    actor->set_pose(requested_actor_poses[actor]);
  }

  repositionActorsConstrainedToRoom(requested_actor_poses.keys(), room);

  // Gather the adjusted positions of the items, and restore the items to their original locations
  // (This function doesn't change anything, just gives a list of corrected positions based on those that are requested)
  QHash<VisualPhysicsActor*, BumpPose> constrained_actor_poses = QHash<VisualPhysicsActor*, BumpPose>();
  for_each(VisualPhysicsActor* actor, requested_actor_poses.keys()) {
    constrained_actor_poses.insert(actor, actor->pose());
    actor->set_pose(original_actor_poses[actor]);
    actor->set_linear_velocity(original_actor_linear_velocities[actor]);
    actor->set_angular_velocity(original_actor_angular_velocities[actor]);
  }
  return constrained_actor_poses;
}

// This method takes a list of requested actor poses, and then returns a corresponding _new_ list of actor
// poses which satisfies the constraint that all actors remain in the room.
QHash<VisualPhysicsActorId, BumpPose> getActorPosesConstrainedToRoom(QHash<VisualPhysicsActorId, BumpPose> requested_actor_poses, Room* room) {  // NOLINT
  QHash<VisualPhysicsActorId, VisualPhysicsActor*> room_actors = room->room_actors();
  QHash<VisualPhysicsActor*, BumpPose> requested_actor_poses_as_actors;
  for_each(VisualPhysicsActorId actor_id, requested_actor_poses.keys()) {
    assert(room_actors.contains(actor_id));  // This function requires that all items passed to it be in the room
    requested_actor_poses_as_actors.insert(room_actors[actor_id], requested_actor_poses[actor_id]);
  }
  QHash<VisualPhysicsActor*, BumpPose> constrained_poses = getActorPosesConstrainedToRoom(requested_actor_poses_as_actors, room);  // NOLINT
  QHash<VisualPhysicsActorId, BumpPose> return_poses;
  for_each(VisualPhysicsActor* actor, constrained_poses.keys()) {
    return_poses.insert(actor->unique_id(), constrained_poses[actor]);
  }
  return return_poses;
}

void repositionActorsConstrainedToRoom(QList<VisualPhysicsActor*> actors_to_reposition,
                                       Room* room, bool exclude_actors_with_physics_disabled) {
  QHash<VisualPhysicsActorId, VisualPhysicsActor*> room_actors = room->room_actors();

  // tolerance allows for some low degree of intersection which is also allowed by physics
  Ogre::Real tolerance = 0.5;
  // Make sure all items are in the room, and if not, move them in to the nearest edge of the room
  for_each(VisualPhysicsActor* actor, actors_to_reposition) {
    if (actor->physics_enabled() || !exclude_actors_with_physics_disabled) {
      if (!(actor->world_bounding_box().getMaximum() < (room->max() + Ogre::Vector3(tolerance)))) {
        Ogre::Vector3 adjustment = room->max() + Ogre::Vector3(tolerance) -
        actor->world_bounding_box().getMaximum();
        adjustment = Math::componentwise_min(adjustment, Ogre::Vector3::ZERO);
        actor->set_position(actor->position() + adjustment);
      }
      // Both of these conditions could be satisfied, so not an else if
      if (!(actor->world_bounding_box().getMinimum() > (room->min() - Ogre::Vector3(tolerance)))) {
        Ogre::Vector3 adjustment = room->min() - Ogre::Vector3(tolerance) -
                                  actor->world_bounding_box().getMinimum();
        adjustment = Math::componentwise_max(adjustment, Ogre::Vector3::ZERO);
        actor->set_position(actor->position() + adjustment);
      }
    }
  }
}

bool actorsCanCollide(VisualPhysicsActor *a, VisualPhysicsActor *b) {
  return (a->collision_mask() & b->collision_group()) != (int16_t)0x0000 &&
         (a->collision_group() & b->collision_mask()) != (int16_t)0x0000;
}

bool actorsIntersect(VisualPhysicsActor *a, VisualPhysicsActor *b) {
  return actorsCanCollide(a, b) && a->world_bounding_box().intersects(b->world_bounding_box());
}

bool actorIntersectsAnyOtherActor(VisualPhysicsActorId actor_id, BumpPose requested_pose, Room* room) {
  QHash<VisualPhysicsActorId, VisualPhysicsActor*> room_actors = room->room_actors();
  VisualPhysicsActor* actor = room_actors[actor_id];

  Ogre::Vector3 linear_velocity =  actor->linear_velocity();
  Ogre::Vector3 angular_velocity = actor->angular_velocity();
  BumpPose original_actor_pose = actor->pose();
  actor->set_pose(requested_pose);

  bool flag = false;
  for_each(VisualPhysicsActorId conflict_candidate_actor_id, room_actors.keys()) {
    if (conflict_candidate_actor_id != actor_id) {
      VisualPhysicsActor *conflict_candidate_actor = room_actors[conflict_candidate_actor_id];
      if (actorsIntersect(actor, conflict_candidate_actor)) {
        flag = true;
        break;
      }
    }
  }
  actor->set_pose(original_actor_pose);
  actor->set_linear_velocity(linear_velocity);
  actor->set_angular_velocity(angular_velocity);
  return flag;
}

// This method takes a list of requested actor poses, and then returns a corresponding _new_ list of actor
// poses which satisfies the constraint that no actor's bounding box is intersecting that of any others
QHash<VisualPhysicsActor*, BumpPose> getActorPosesConstrainedToNoIntersections(QHash<VisualPhysicsActor*, BumpPose> requested_actor_poses, Room* room) {  // NOLINT
  VisualPhysicsActorList room_actors = room->room_actor_list();
  QHash<VisualPhysicsActor*, BumpPose> original_actor_poses;
  QHash<VisualPhysicsActor*, Ogre::Vector3> original_actor_linear_velocities;
  QHash<VisualPhysicsActor*, Ogre::Vector3> original_actor_angular_velocities;

  // Move the items which are requesting new poses into those poses, and store their original (current) poses
  // (this move is temporary, just to let us figure out how stuff will look in the final state and adjust accordingly)
  for_each(VisualPhysicsActor* actor, requested_actor_poses.keys()) {
    original_actor_poses.insert(actor, actor->pose());
    original_actor_linear_velocities.insert(actor, actor->linear_velocity());
    original_actor_angular_velocities.insert(actor, actor->angular_velocity());
    actor->set_pose(requested_actor_poses[actor]);
  }

  // Any actors which are in animations and which are not in the list of requested poses should be
  // temporarily placed in their final positions to let us figure out how stuff will look in the final state
  // and adjust accordingly)
  QList<VisualPhysicsActorAnimation*> animations = AnimationManager::singleton()->actor_animations();
  for_each(VisualPhysicsActorAnimation* animation, animations) {
    VisualPhysicsActor* actor = animation->visual_physics_actor();
    if (!requested_actor_poses.contains(actor)) {
      original_actor_poses.insert(actor, actor->pose());
      actor->set_pose(animation->final_pose());
    }
  }

  // Check for intersections for each actor requesting a new pose:
  // For any intersection that is found, the offending actor is pushed upward (+y) an amount which
  // ensures that its bounding box no longer intersects the other
  QList<VisualPhysicsActor*> corrected_actors;
  int iteration_count;
  for_each(VisualPhysicsActor* actor, requested_actor_poses.keys()) {
    Ogre::AxisAlignedBox actor_bounding_box;
    bool conflict;
    iteration_count = 0;
    do {
      conflict = false;
      actor_bounding_box = actor->world_bounding_box();
      for_each(VisualPhysicsActor* conflict_candidate_actor, room_actors) {
        if (!conflict_candidate_actor->collides_with_walls_only() &&
            !conflict_candidate_actor->collides_with_gridded_piles() &&
            !actor->collides_with_gridded_piles() &&
            (corrected_actors.contains(conflict_candidate_actor) ||
             !requested_actor_poses.keys().contains(conflict_candidate_actor))) {
          Ogre::AxisAlignedBox conflict_candidate_bounding_box = conflict_candidate_actor->world_bounding_box();
          if (actor_bounding_box.intersects(conflict_candidate_bounding_box)) {
            Ogre::AxisAlignedBox intersection = actor_bounding_box.intersection(conflict_candidate_bounding_box);
            Ogre::Vector3 conflict_candidate_room_surface_normal = conflict_candidate_actor->room_surface()->normal();
            Ogre::Vector3 shift = intersection.getMaximum() - intersection.getMinimum();
            // we make the shift slightly larger to ensure that things don't still intersect
            shift = Math::componentwise_max(1.05*shift, Ogre::Vector3(0.001, 0.001, 0.001));

            // We adjust the actor's position in the direction of the normal of the room surface
            // of the actor it intersects
            actor->set_position(actor->position() + shift*conflict_candidate_room_surface_normal);
            conflict = true;
            break;
          }
        }
      }
      iteration_count++;
    } while (conflict);

    corrected_actors.push_back(actor);
  }

  // Gather the adjusted positions of the items
  QHash<VisualPhysicsActor*, BumpPose> constrained_actor_poses = QHash<VisualPhysicsActor*, BumpPose>();
  for_each(VisualPhysicsActor* actor, requested_actor_poses.keys()) {
    constrained_actor_poses.insert(actor, actor->pose());
  }

  // restore the items to their original poses
  for_each(VisualPhysicsActor* actor, original_actor_poses.keys()) {
    actor->set_pose(original_actor_poses[actor]);
    actor->set_linear_velocity(original_actor_linear_velocities[actor]);
    actor->set_angular_velocity(original_actor_angular_velocities[actor]);
  }

  return constrained_actor_poses;
}

// This method does exactly as above, but takes a list of actor ids, and hence relies on the actors being in the room
QHash<VisualPhysicsActorId, BumpPose> getActorPosesConstrainedToNoIntersections(QHash<VisualPhysicsActorId, BumpPose> requested_actor_poses, Room* room) {  // NOLINT
  QHash<VisualPhysicsActorId, VisualPhysicsActor*> room_actors = room->room_actors();

  QHash<VisualPhysicsActor*, BumpPose> requested_actor_poses_as_actors;
  for_each(VisualPhysicsActorId actor_id, requested_actor_poses.keys()) {
    assert(room_actors.contains(actor_id));  // This function requires that all items passed to it be in the room
    requested_actor_poses_as_actors.insert(room_actors[actor_id], requested_actor_poses[actor_id]);
  }
  QHash<VisualPhysicsActor*, BumpPose> constrained_poses = getActorPosesConstrainedToNoIntersections(requested_actor_poses_as_actors, room);  // NOLINT
  QHash<VisualPhysicsActorId, BumpPose> return_poses;  // NOLINT
  for_each(VisualPhysicsActor* actor, constrained_poses.keys()) {
    return_poses.insert(actor->unique_id(), constrained_poses[actor]);
  }
  return return_poses;
}

BumpPose getActorPoseConstrainedToRoomAndNoIntersections(VisualPhysicsActor* actor, Room* room) {
  QHash<VisualPhysicsActor*, BumpPose> desired_pose;
  desired_pose.insert(actor, actor->pose());

  QHash<VisualPhysicsActor*, BumpPose> constrained_pose = getActorPosesConstrainedToRoom(desired_pose, room);
  constrained_pose = getActorPosesConstrainedToNoIntersections(constrained_pose, room);
  return constrained_pose[actor];
}

Ogre::Vector3 getPositionConstrainedToRoom(Ogre::Vector3 position, Room* room) {
  if (!(room->max() > position)) {
    position = Math::componentwise_min(position, room->max());
  }
  if (!(room->min() < position)) {
    position = Math::componentwise_max(position, room->min());
  }
  return position;
}

std::pair<bool, Ogre::Vector3> getPositionConstrainedToRoom(Ogre::AxisAlignedBox bounding_box, Room* room) {
  // tolerance allows for some low degree of intersection which is also allowed by physics
  Ogre::Real tolerance = 0.5;
  bool adjusted = false;
  Ogre::Vector3 adjusted_position = bounding_box.getCenter();
  if (!(bounding_box.getMaximum() < room->max())) {
    Ogre::Vector3 adjustment = room->max() + Ogre::Vector3(tolerance)
                              - bounding_box.getMaximum();
    adjusted_position += Math::componentwise_min(adjustment, Ogre::Vector3::ZERO);
    adjusted = true;
  }
  // Both of these conditions could be satisfied, so not an else if
  if (!(bounding_box.getMinimum() > room->min())) {
    Ogre::Vector3 adjustment = room->min() - Ogre::Vector3(tolerance)
                               - bounding_box.getMinimum();
    adjusted_position += Math::componentwise_max(adjustment, Ogre::Vector3::ZERO);
    adjusted = true;
  }
  return std::pair<bool, Ogre::Vector3>(adjusted, adjusted_position);
}
