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

#ifndef BUMPTOP_UNDOCOMMANDS_ROOMSTATEUNDOCOMMAND_H_
#define BUMPTOP_UNDOCOMMANDS_ROOMSTATEUNDOCOMMAND_H_

class Room;
class RoomUndoRedoState;

#include <QtGui/QUndoCommand>
#include <boost/shared_ptr.hpp>

class RoomStateUndoCommand : public QUndoCommand {
 public:
  explicit RoomStateUndoCommand(Room* room);
  virtual ~RoomStateUndoCommand();

  virtual void undo();
  virtual void redo();

  virtual void updateRoomFromRoomUndoRedoState(boost::shared_ptr<RoomUndoRedoState> state);
  virtual void set_current_state(boost::shared_ptr<RoomUndoRedoState> state);
  virtual void set_last_state(boost::shared_ptr<RoomUndoRedoState> state);
  virtual boost::shared_ptr<RoomUndoRedoState> current_state();
  virtual boost::shared_ptr<RoomUndoRedoState> last_state();

 protected:
  boost::shared_ptr<RoomUndoRedoState> current_state_;
  boost::shared_ptr<RoomUndoRedoState> last_state_;
  bool first_redo_happened_;
  Room *room_;
};

#endif  // BUMPTOP_UNDOCOMMANDS_ROOMSTATEUNDOCOMMAND_H_
