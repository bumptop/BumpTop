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

#ifndef BUMPTOP_UNDOCOMMANDS_GRIDVIEWUNDOCOMMAND_H_
#define BUMPTOP_UNDOCOMMANDS_GRIDVIEWUNDOCOMMAND_H_

class Room;
class Physics;
class RoomStateUndoCommand;
class ShrinkUndoCommand;
class VisualPhysicsActor;

#include <QtGui/QUndoCommand>

#include "BumpTop/BumpTopCommands.h"
#include "BumpTop/OgreHelpers.h"
#include "BumpTop/VisualPhysicsActorId.h"

class GridViewUndoCommand : public QUndoCommand {
 public:
  explicit GridViewUndoCommand(VisualPhysicsActorList actors, Room* room,
                               Ogre::SceneManager* scene_manager, Physics* physics);
  virtual ~GridViewUndoCommand();
  virtual void undo();
  virtual void redo();

 protected:
  QHash<VisualPhysicsActorId, BumpPose> getGriddedPositionOfActorsAndShrinkActorsIfNeeded(QList<VisualPhysicsActorId> actors_ids, Ogre::Real margin);  // NOLINT
  Ogre::Real getWidthOfLargestActor(VisualPhysicsActorList actors);
  QList<VisualPhysicsActorId> getFilteredActorIdList(QList<VisualPhysicsActorId> actors_ids);
  bool shrinkActorsToFitIntoRoom(VisualPhysicsActorList actors, Ogre::Real width_of_largest_actor);
  Ogre::Real getMarginBetweenItems(QList<VisualPhysicsActorId> actors_ids);

  bool first_run_;
  bool command_changed_something_;
  Room* room_;
  Physics* physics_;
  Ogre::SceneManager* scene_manager_;
  QList<VisualPhysicsActorId> actors_ids_;
  QList<ShrinkUndoCommand*> shrink_commands_;
  RoomStateUndoCommand* undo_command_;
};
#endif  // BUMPTOP_UNDOCOMMANDS_GRIDVIEWUNDOCOMMAND_H_
