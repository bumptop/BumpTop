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

#ifndef BUMPTOP_UNDOREDOSTACK_H_
#define BUMPTOP_UNDOREDOSTACK_H_

#include <boost/shared_ptr.hpp>
#include <QtGui/QUndoStack>

class Room;
class RoomStateUndoCommand;
class RoomUndoRedoState;

class UndoRedoStack {
 public:

  explicit UndoRedoStack(Room* room);
  virtual ~UndoRedoStack();

  static bool last_command_changed_something;
  static bool is_last_command_grid_view;

  static bool has_current_item_always_been_top_most_item;

  virtual void init();
  virtual void closeAndPushAnyOpenRoomStateUndoCommand(boost::shared_ptr<RoomUndoRedoState> current_state);  // NOLINT
  virtual void openNewRoomStateUndoCommand(boost::shared_ptr<RoomUndoRedoState> current_state);
  virtual void push(QUndoCommand* undo_command, boost::shared_ptr<RoomUndoRedoState> current_state,
                    bool is_drag_operation = false);

  virtual void undo(boost::shared_ptr<RoomUndoRedoState> current_state);
  virtual void redo(boost::shared_ptr<RoomUndoRedoState> current_state);

 protected:
  void clearAnyOpenRoomStateUndoCommands();

  Room *room_;
  RoomStateUndoCommand* undo_command_;
  bool room_state_undo_command_is_open_;
  bool is_last_command_undo_command_;
  QUndoStack* undo_stack_;
};

#endif  // BUMPTOP_UNDOREDOSTACK_H_
