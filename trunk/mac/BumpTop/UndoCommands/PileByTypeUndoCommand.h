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

#ifndef BUMPTOP_UNDOCOMMANDS_PILEBYTYPEUNDOCOMMAND_H_
#define BUMPTOP_UNDOCOMMANDS_PILEBYTYPEUNDOCOMMAND_H_

class Room;
class BumpPile;
class PileBreakUndoCommand;
class PileizeUndoCommand;
class Physics;
class VisualPhysicsActor;

#include <QtGui/QUndoCommand>

#include "BumpTop/OgreHelpers.h"
#include "BumpTop/VisualPhysicsActorId.h"
#include "BumpTop/VisualPhysicsActorList.h"

class PileByTypeUndoCommand : public QUndoCommand {
 public:
  explicit PileByTypeUndoCommand(VisualPhysicsActorList actors, Room* room,
                                 Ogre::SceneManager* scene_manager, Physics* physics);
  virtual ~PileByTypeUndoCommand();

  virtual void undo();
  virtual void redo();

  static const Ogre::Real kMarginInBetweenPiles;

 protected:
  VisualPhysicsActorList initializePileBreakCommandsAndGetFlattenActorsList(VisualPhysicsActorList actors);
  QHash<QString, VisualPhysicsActorList > getQHashOfActorsByType(VisualPhysicsActorList actors);
  QHash<QString, Ogre::Vector2> getPositionForPiles(QHash<QString, VisualPhysicsActorList> file_type_and_actors_of_that_type); // NOLINT
  Ogre::Real maximumWidthOfActors(VisualPhysicsActorList actors);
  Ogre::Real maximumHeightOfActors(VisualPhysicsActorList actors);
  QString getFileType(QString path);

  bool first_run_;
  bool is_pile_by_type_for_all_actors_;
  bool will_pile_by_type_change_something_;
  QList<PileBreakUndoCommand*> pile_break_commands_;
  QList<PileizeUndoCommand*> pileize_commands_;
  Room* room_;
  Physics* physics_;
  Ogre::SceneManager* scene_manager_;
  VisualPhysicsActorId unique_id_of_actor_with_unique_type_;
  BumpPose original_pose_of_actor_with_unique_type_;
  Ogre::Vector3 final_position_of_actor_with_unique_type_;
};

#endif  // BUMPTOP_UNDOCOMMANDS_PILEBYTYPEUNDOCOMMAND_H_
