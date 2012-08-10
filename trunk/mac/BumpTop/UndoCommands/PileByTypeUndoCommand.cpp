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

#include "BumpTop/UndoCommands/PileByTypeUndoCommand.h"

#include "BumpTop/AnimationManager.h"
#include "BumpTop/BumpPile.h"
#include "BumpTop/FileManager.h"
#include "BumpTop/Room.h"
#include "BumpTop/RoomItemPoseConstraints.h"
#include "BumpTop/RoomSurface.h"
#include "BumpTop/UndoRedoStack.h"
#include "BumpTop/UndoCommands/PileBreakUndoCommand.h"
#include "BumpTop/UndoCommands/PileizeUndoCommand.h"
#include "BumpTop/VisualPhysicsActorAnimation.h"

const Ogre::Real PileByTypeUndoCommand::kMarginInBetweenPiles = 50;

PileByTypeUndoCommand::PileByTypeUndoCommand(VisualPhysicsActorList actors, Room* room,
                                             Ogre::SceneManager* scene_manager, Physics* physics)
: room_(room),
scene_manager_(scene_manager),
physics_(physics),
first_run_(true) {
  unique_id_of_actor_with_unique_type_ = NULL;
  is_pile_by_type_for_all_actors_ = true;

  for_each(VisualPhysicsActor* actor, room->floor_actor_list()) {
    if (!actors.contains(actor)
        && !actor->is_new_items_pile()
        && actor->actor_type() != STICKY_NOTE_PAD) {
      is_pile_by_type_for_all_actors_ = false;
    }
  }

  // get rid of all pinnable actors and volumes that can not be piled
  // and sticky_note_pads!
  VisualPhysicsActorList actors_valid_for_type_by_type;
  for_each(VisualPhysicsActor* actor, actors) {
    // filter out all items on the wall
    if (!actor->room_surface()->is_pinnable_receiver() && actor->actor_type() != STICKY_NOTE_PAD) {
      actors_valid_for_type_by_type.append(actor);
    }
  }

  // flattens actors by breaking all bumppiles to bumpboxes and construct all pilebreakundocommand
  VisualPhysicsActorList flattened_actors = initializePileBreakCommandsAndGetFlattenActorsList(actors_valid_for_type_by_type);  // NOLINT

  if (flattened_actors.count() < 2) {
    will_pile_by_type_change_something_ = false;
  } else {
    will_pile_by_type_change_something_ = true;
  }

  // proceed with pile by type only if there is more than 1 actors
  if (will_pile_by_type_change_something_) {
    // sort actors into QLists based on their extention
    QHash<QString, VisualPhysicsActorList> file_type_and_actors_of_that_type = getQHashOfActorsByType(flattened_actors);

    // construction all piliezeundocommands
    pileize_commands_ = QList<PileizeUndoCommand*>();
    QHash<QString, Ogre::Vector2> position_for_actors = getPositionForPiles(file_type_and_actors_of_that_type);
    for_each(QString file_type, file_type_and_actors_of_that_type.keys()) {
      Ogre::Vector3 position_for_actor;
      position_for_actor.x = position_for_actors[file_type].x;
      position_for_actor.z = position_for_actors[file_type].y;

      // For QList with only 1 actor, track original pose and final position
      if (file_type_and_actors_of_that_type[file_type].count() == 1) {
        unique_id_of_actor_with_unique_type_ = file_type_and_actors_of_that_type[file_type][0]->unique_id();
        original_pose_of_actor_with_unique_type_ = file_type_and_actors_of_that_type[file_type][0]->pose();
        final_position_of_actor_with_unique_type_ = position_for_actor;
        final_position_of_actor_with_unique_type_.y = 10;
      } else {
        PileizeUndoCommand* pileize_command = new PileizeUndoCommand(file_type_and_actors_of_that_type[file_type], room, scene_manager_, physics_, position_for_actor, file_type);  // NOLINT
        pileize_commands_.push_back(pileize_command);
      }
    }
  }
}

PileByTypeUndoCommand::~PileByTypeUndoCommand() {
}

void PileByTypeUndoCommand::undo() {
  if (!will_pile_by_type_change_something_) {
    UndoRedoStack::last_command_changed_something = false;
    return;
  }

  // undo pileizecommands which breaks all piles created from pile by type
  bool undo_command_changed_something = false;
  for_each(PileizeUndoCommand* pileize_command, pileize_commands_) {
    pileize_command->undo();
    undo_command_changed_something = undo_command_changed_something || UndoRedoStack::last_command_changed_something;
  }

  // apply animation to the single actor that does not belong to any pile
  VisualPhysicsActor* actor = room_->actor_with_unique_id(unique_id_of_actor_with_unique_type_);
  if (actor != NULL) {
    QHash<VisualPhysicsActorId, BumpPose> desired_poses;
    desired_poses.insert(actor->unique_id(), original_pose_of_actor_with_unique_type_);

    QHash<VisualPhysicsActorId, BumpPose> constrained_poses = getActorPosesConstrainedToRoom(desired_poses, room_);
    constrained_poses = getActorPosesConstrainedToNoIntersections(constrained_poses, room_);

    AnimationManager::singleton()->endAnimationsForActor(actor, AnimationManager::MOVE_TO_FINAL_STATE);  // NOLINT
    VisualPhysicsActorAnimation* actor_animation;
    actor_animation = new VisualPhysicsActorAnimation(actor, 250,
                                                      constrained_poses[actor->unique_id()].position,
                                                      constrained_poses[actor->unique_id()].orientation);
    actor_animation->start();
    actor->set_selected(true);
    undo_command_changed_something = true;
  }

  // undo breakpilecommands which puts actors into their original piles before pile type type happened
  QList<PileBreakUndoCommand*> pile_break_command_reversed = QList<PileBreakUndoCommand*>();
  for_each(PileBreakUndoCommand* pile_break_command, pile_break_commands_) {
    pile_break_command_reversed.push_front(pile_break_command);
  }
  for_each(PileBreakUndoCommand* pile_break_command, pile_break_command_reversed) {
    pile_break_command->undo();
    undo_command_changed_something = undo_command_changed_something || UndoRedoStack::last_command_changed_something;
  }
  UndoRedoStack::last_command_changed_something = undo_command_changed_something;
}

void PileByTypeUndoCommand::redo() {
  if (!will_pile_by_type_change_something_) {
    UndoRedoStack::last_command_changed_something = false;
    return;
  }

  // redo breakpilecommands to break all piles including heirachical piles
  bool redo_command_changed_something = false;
  for_each(PileBreakUndoCommand* pile_break_command, pile_break_commands_) {
    pile_break_command->redo();
    redo_command_changed_something = redo_command_changed_something || UndoRedoStack::last_command_changed_something;
  }

  // apply animation to the single actor that does not belong to any pile
  VisualPhysicsActor* actor = room_->actor_with_unique_id(unique_id_of_actor_with_unique_type_);
  if (actor != NULL) {
    QHash<VisualPhysicsActorId, BumpPose> desired_poses;
    BumpPose pose = BumpPose(final_position_of_actor_with_unique_type_, Ogre::Quaternion(1, 0, 0, 0));
    desired_poses.insert(actor->unique_id(), pose);

    QHash<VisualPhysicsActorId, BumpPose> constrained_poses = getActorPosesConstrainedToRoom(desired_poses, room_);
    constrained_poses = getActorPosesConstrainedToNoIntersections(constrained_poses, room_);

    AnimationManager::singleton()->endAnimationsForActor(actor, AnimationManager::MOVE_TO_FINAL_STATE);  // NOLINT
    VisualPhysicsActorAnimation* actor_animation;
    actor_animation = new VisualPhysicsActorAnimation(actor, 250,
                                                      constrained_poses[actor->unique_id()].position,
                                                      constrained_poses[actor->unique_id()].orientation);
    actor_animation->start();
    actor->set_selected(true);
    redo_command_changed_something = true;
  }

  // redo pileizecommands to put actors of same type into piles
  for_each(PileizeUndoCommand* pileize_command, pileize_commands_) {
    pileize_command->redo();
    redo_command_changed_something = redo_command_changed_something || UndoRedoStack::last_command_changed_something;
  }
  UndoRedoStack::last_command_changed_something = redo_command_changed_something;
}

QHash<QString, Ogre::Vector2> PileByTypeUndoCommand::getPositionForPiles(QHash<QString, VisualPhysicsActorList> file_type_and_actors_of_that_type) {  // NOLINT
  Ogre::Real width_of_row = 0;
  Ogre::Real height_of_row = 0;
  Ogre::Real height_of_rows = 0;
  Ogre::Real start_z_position_of_row;
  Ogre::Real width_of_first_pile;
  int number_of_actors_in_row = 0;
  int number_of_rows = 1;
  int index_of_hash = 0;
  QHash<int, QList<VisualPhysicsActorList > > actors_in_rows;
  QHash<int, Ogre::Real> start_x_position_of_rows;
  QHash<int, Ogre::Real> maximum_height_of_actors_in_rows;
  QHash<QString, Ogre::Vector2> position_for_actors;
  Ogre::Vector3 centroid = Ogre::Vector3::ZERO;
  int number_of_actors = 0;

  if (is_pile_by_type_for_all_actors_) {
    centroid = Ogre::Vector3(room_->center_of_floor().x,0,room_->min_z() + 200);
  } else {
    for_each(VisualPhysicsActorList actor_list, file_type_and_actors_of_that_type) {
      for_each(VisualPhysicsActor* actor, actor_list) {
        if (actor->parent() != NULL) {
          centroid += Ogre::Vector3(actor->parent()->position().x,0,actor->parent()->position().z);
        } else {
          centroid += Ogre::Vector3(actor->position().x,0,actor->position().z);
        }
        number_of_actors++;
      }
    }
    centroid/=number_of_actors;
  }

  // sort QLists of actors into rows depending on the maximum witdh of each QList and determine the x position at the
  // beginning of each row
  for_each(VisualPhysicsActorList actors_of_same_type, file_type_and_actors_of_that_type.values()) {
    index_of_hash++;

    // if the new QList of actors fits into the current row, append it to QHash and increment size of row
    if ((width_of_row + maximumWidthOfActors(actors_of_same_type)) + kMarginInBetweenPiles < room_->floor_width()) {  // NOLINT
      if (width_of_row == 0) {
        width_of_first_pile = maximumWidthOfActors(actors_of_same_type);
        width_of_row = maximumWidthOfActors(actors_of_same_type);
      } else {
        width_of_row += maximumWidthOfActors(actors_of_same_type) + kMarginInBetweenPiles;
      }
      if (height_of_row < maximumHeightOfActors(actors_of_same_type))
        height_of_row = maximumHeightOfActors(actors_of_same_type);
      actors_in_rows[number_of_rows].append(actors_of_same_type);
      number_of_actors_in_row++;
    } else {
      // if the new QList of actors exceeds floor width then calculate the start of position of the current
      // row and set width_of_row to 0 to start a new row
      height_of_rows += height_of_row + kMarginInBetweenPiles;
      Ogre::Real start_x_position_of_row = centroid.x;
      start_x_position_of_row -= width_of_row/2 - width_of_first_pile/2;
      start_x_position_of_rows.insert(number_of_rows, start_x_position_of_row);
      maximum_height_of_actors_in_rows.insert(number_of_rows, height_of_row);
      number_of_rows++;

      // put the actor to a new row
      number_of_actors_in_row = 1;
      width_of_row = maximumWidthOfActors(actors_of_same_type);
      height_of_row = maximumHeightOfActors(actors_of_same_type);
      width_of_first_pile = maximumWidthOfActors(actors_of_same_type);
      QList<VisualPhysicsActorList > actor;
      actor.append(actors_of_same_type);
      actors_in_rows.insert(number_of_rows, actor);
    }

    // this is for dealing with the last QList of actors
    if (index_of_hash == file_type_and_actors_of_that_type.count()) {
      height_of_rows += height_of_row;
      Ogre::Real start_x_position_of_row = centroid.x;
      start_x_position_of_row -= width_of_row/2 - width_of_first_pile/2;
      start_x_position_of_rows.insert(number_of_rows, start_x_position_of_row);
      maximum_height_of_actors_in_rows.insert(number_of_rows, height_of_row);
    }
  }

  // determining the start z position of the first row
  start_z_position_of_row = centroid.z;

  Ogre::Real current_x_position_of_row;
  Ogre::Real current_z_position_of_row = start_z_position_of_row - maximum_height_of_actors_in_rows[1]/2;

  // create a QHash with extention of each QList of actors as key and the desired position as value
  for (int row = 1; row <= number_of_rows; row++) {
    current_x_position_of_row = start_x_position_of_rows[row];
    current_x_position_of_row -= maximumWidthOfActors(actors_in_rows[row][0])/2;
    current_z_position_of_row += maximum_height_of_actors_in_rows[row]/2;
    for_each(VisualPhysicsActorList actors, actors_in_rows[row]) {
      current_x_position_of_row += maximumWidthOfActors(actors)/2;
      Ogre::Vector2 xz_position_of_actors = Ogre::Vector2(current_x_position_of_row, current_z_position_of_row);
      current_x_position_of_row += maximumWidthOfActors(actors)/2 + kMarginInBetweenPiles;
      position_for_actors.insert(file_type_and_actors_of_that_type.key(actors), xz_position_of_actors);
    }
    current_z_position_of_row += maximum_height_of_actors_in_rows[row]/2 + kMarginInBetweenPiles;
  }
  return position_for_actors;
}

Ogre::Real PileByTypeUndoCommand::maximumWidthOfActors(VisualPhysicsActorList actors) {
  Ogre::Real width_of_pile = 0;
  for_each(VisualPhysicsActor* actor, actors) {
    if (actor->size().x > width_of_pile)
      width_of_pile = actor->size().x;
  }
  return width_of_pile;
}

Ogre::Real PileByTypeUndoCommand::maximumHeightOfActors(VisualPhysicsActorList actors) {
  Ogre::Real height_of_pile = 0;
  for_each(VisualPhysicsActor* actor, actors) {
    if (actor->size().z > height_of_pile)
      height_of_pile = actor->size().z;
  }
  return height_of_pile;
}

QHash<QString, VisualPhysicsActorList > PileByTypeUndoCommand::getQHashOfActorsByType(VisualPhysicsActorList actors) {
  QHash<QString, VisualPhysicsActorList > file_type_and_actors_of_that_type;
  for_each(VisualPhysicsActor* actor, actors) {
    QString file_type = getFileType(actor->path());
    file_type_and_actors_of_that_type[file_type].push_back(actor);
  }

  // collect all types that have only 1 actor in them and put them into a miscellaneous QList
  VisualPhysicsActorList miscellaneous_actors;
  for_each(VisualPhysicsActorList actors_of_same_type, file_type_and_actors_of_that_type.values()) {
    if (actors_of_same_type.count() == 1) {
      miscellaneous_actors.append(actors_of_same_type[0]);
      file_type_and_actors_of_that_type.remove(file_type_and_actors_of_that_type.key(actors_of_same_type));
    }
  }

  // append miscellaneous to file_type_and_actors_of_that_type if miscellaneous is not empty
  if (!miscellaneous_actors.empty())
    file_type_and_actors_of_that_type[QString("misc.")].append(miscellaneous_actors);
  return file_type_and_actors_of_that_type;
}

QString PileByTypeUndoCommand::getFileType(QString path) {
  FileKind file_kind = FileManager::getFileKind(path);
  if (FileManager::getParentPath(path) == FileManager::getApplicationDataPath() + "Stickies") {
    return "notes";
  } else if (file_kind == VOLUME) {
    return "volume";
  } else if (file_kind == APPLICATION) {
    return "app";
  } else if (file_kind == ALIAS) {
    return getFileType(QFileInfo(path).readLink());
  } else {
    if (QFileInfo(path).isDir()) {
      if (QFileInfo(path).suffix() != "") {
        return QFileInfo(path).suffix();
      } else {
        return "folder";
      }
    } else {
      return QFileInfo(path).suffix();
    }
  }
}

VisualPhysicsActorList PileByTypeUndoCommand::initializePileBreakCommandsAndGetFlattenActorsList(VisualPhysicsActorList actors) {  // NOLINT
  VisualPhysicsActorList actors_to_be_flattened = actors;
  VisualPhysicsActorList flattened_actors;
  pile_break_commands_ = QList<PileBreakUndoCommand*>();
  bool is_flattened;

  // each iteration of the loop gets a QList of VisualPhysicsActor of children of piles and construct breakpile command
  // loop stops when there is no more pile
  do {
    is_flattened = true;
    VisualPhysicsActorList actors_to_be_flattened_copy = actors_to_be_flattened;
    actors_to_be_flattened = VisualPhysicsActorList();
    for_each(VisualPhysicsActor* actor, actors_to_be_flattened_copy) {
      if (actor != NULL) {
        if (!actor->children().empty()) {
          // construct pilebreakundocommand and flatten the current level of actors
          VisualPhysicsActorList actor_to_be_flattened;
          actor_to_be_flattened.append(actor);
          PileBreakUndoCommand* pile_break_command = new PileBreakUndoCommand(actor_to_be_flattened, room_, scene_manager_, physics_, DONT_CONSTRAIN_FINAL_MEMBER_POSITIONS);  // NOLINT
          pile_break_commands_.push_back(pile_break_command);
          is_flattened = false;
          for_each(VisualPhysicsActor* actor_children, actor->children()) {
            // preparing the list of actors to be flattened next iteration
            actors_to_be_flattened.append(actor_children);
          }
        } else {
          flattened_actors.append(actor);
        }
      }
    }
  } while (!is_flattened);

  return flattened_actors;
}
