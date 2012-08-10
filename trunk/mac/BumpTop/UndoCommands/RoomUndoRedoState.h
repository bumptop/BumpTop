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

#ifndef BUMPTOP_UNDOCOMMANDS_ROOMUNDOREDOSTATE_H_
#define BUMPTOP_UNDOCOMMANDS_ROOMUNDOREDOSTATE_H_

#include <QtCore/QHash>
#include <Ogre.h>

#include <BumpTop/OgreHelpers.h>
#include <BumpTop/VisualPhysicsActorId.h>

class Room;
class RoomSurface;
class VisualPhysicsActor;

class RoomUndoRedoState {
 public:
  explicit RoomUndoRedoState();
  virtual ~RoomUndoRedoState();

  virtual void add_actor(VisualPhysicsActor* actor);
  virtual bool contains(VisualPhysicsActorId unique_id);
  virtual Ogre::Vector3 position_for_id(VisualPhysicsActorId unique_id);
  virtual Ogre::Quaternion orientation_for_id(VisualPhysicsActorId unique_id);
  virtual RoomSurface* surface_for_id(VisualPhysicsActorId unique_id);
  virtual QHash<VisualPhysicsActorId, BumpPose> actor_poses();


 protected:
  QHash<VisualPhysicsActorId, BumpPose> actor_poses_;
  QHash<VisualPhysicsActorId, RoomSurface*> actor_surfaces_;
};

#endif  // BUMPTOP_UNDOCOMMANDS_ROOMUNDOREDOSTATE_H_
