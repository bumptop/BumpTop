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

#ifndef BUMPTOP_UNDOCOMMANDS_GROWUNDOCOMMAND_H_
#define BUMPTOP_UNDOCOMMANDS_GROWUNDOCOMMAND_H_

#include <QtGui/QUndoCommand>

#include "BumpTop/VisualPhysicsActorId.h"
#include "BumpTop/VisualPhysicsActorList.h"

#define VPA_GROW_FACTOR 1.25
#define MAX_ACTOR_SIZE 300
#define MIN_ACTOR_SIZE 35

class Room;
class VisualPhysicsActor;

class GrowUndoCommand: public QUndoCommand {
 public:
  explicit GrowUndoCommand(VisualPhysicsActorList actors_to_grow, Room* room);
  virtual ~GrowUndoCommand();
  virtual void undo();
  virtual void redo();

 protected:
  bool willGrowCommandChangeSomething(VisualPhysicsActorList actors, Ogre::Real scale_factor);
  void updateLabelSize(VisualPhysicsActor* actor, Ogre::Real size_factor);

  bool first_run_;
  bool did_redo_change_something_;
  Room* room_;
  QList<VisualPhysicsActorId> actors_to_grow_;
  QHash<VisualPhysicsActorId, bool> actors_size_was_changed_;
};
#endif  // BUMPTOP_UNDOCOMMANDS_GROWUNDOCOMMAND_H_
