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

#ifndef BUMPTOP_UNDOCOMMANDS_MOVEFILEINTOBUMPTOPUNDOCOMMAND_H_
#define BUMPTOP_UNDOCOMMANDS_MOVEFILEINTOBUMPTOPUNDOCOMMAND_H_

#include <QtGui/QUndoCommand>

class Room;

class MoveFileIntoBumpTopUndoCommand : public QUndoCommand {
 public:
  explicit MoveFileIntoBumpTopUndoCommand(QStringList paths,
                                          Ogre::Vector2 expected_position,
                                          Room* room,
                                          QString target_path);
  virtual ~MoveFileIntoBumpTopUndoCommand();
  virtual void undo();
  virtual void redo();

 protected:
  void updatePaths(QStringList invalid_paths);

  bool first_run_;
  QStringList paths_;
  Ogre::Vector2 expected_position_;
  Room* room_;
  QString target_path_;

  QHash<QString, ino_t> file_paths_to_inodes_;
};
#endif  // BUMPTOP_UNDOCOMMANDS_MOVEFILEINTOBUMPTOPUNDOCOMMAND_H_
