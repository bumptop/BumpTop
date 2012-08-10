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

#include "BumpTop/UndoRedoStack.h"

#include "BumpTop/protoc/AllMessages.pb.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/BumpTopScene.h"
#include "BumpTop/UndoCommands/RoomStateUndoCommand.h"

bool UndoRedoStack::last_command_changed_something = true;
bool UndoRedoStack::is_last_command_grid_view = true;
bool UndoRedoStack::has_current_item_always_been_top_most_item = true;

// Note regarding our implementation of undo / redo. There are currently two types of undo/redo command:
// 1. RoomStateUndoCommand and 2. "All other undo commands". RoomStateUndoCommands keep track of the pose
// of all items in the room, and are used in a sort of snapshot. When another type of undo command is pushed
// onto the stack, the room state is frozen, and anything that moves after that point will be lost from the
// perspective of undo / redo, and will hence create a discontinuity in some undo-redo pairs.
UndoRedoStack::UndoRedoStack(Room* room)
: room_(room),
  room_state_undo_command_is_open_(false),
  is_last_command_undo_command_(false) {
}

UndoRedoStack::~UndoRedoStack() {
  delete undo_stack_;
}

void UndoRedoStack::init() {
  undo_stack_ = new QUndoStack();
}

void UndoRedoStack::closeAndPushAnyOpenRoomStateUndoCommand(boost::shared_ptr<RoomUndoRedoState>
                                       current_state) {
  is_last_command_grid_view = false;
  if (room_state_undo_command_is_open_) {
    has_current_item_always_been_top_most_item = true;
    is_last_command_undo_command_ = false;

    undo_command_->set_current_state(current_state);
    undo_stack_->push(undo_command_);
    room_state_undo_command_is_open_ = false;
  }
#ifndef BUMPTOP_TEST
  BumpTopApp::singleton()->scene()->markSceneAsChanged();
#endif
}

void UndoRedoStack::push(QUndoCommand* undo_command, boost::shared_ptr<RoomUndoRedoState> current_state,
                         bool is_drag_operation) {
  has_current_item_always_been_top_most_item = true;
  is_last_command_undo_command_ = false;

  if (!is_drag_operation) {
    closeAndPushAnyOpenRoomStateUndoCommand(current_state);
  } else {
    clearAnyOpenRoomStateUndoCommands();
  }
  undo_stack_->push(undo_command);
}

void UndoRedoStack::clearAnyOpenRoomStateUndoCommands() {
  if (room_state_undo_command_is_open_) {
    delete undo_command_;
    undo_command_ = NULL;
    room_state_undo_command_is_open_ = false;
  }
}

void UndoRedoStack::openNewRoomStateUndoCommand(boost::shared_ptr<RoomUndoRedoState> current_state) {
  closeAndPushAnyOpenRoomStateUndoCommand(current_state);
  undo_command_ = new RoomStateUndoCommand(room_);
  undo_command_->set_last_state(current_state);
  room_state_undo_command_is_open_ = true;
}

void UndoRedoStack::undo(boost::shared_ptr<RoomUndoRedoState> current_state) {
  closeAndPushAnyOpenRoomStateUndoCommand(current_state);

  last_command_changed_something = true;
  if (is_last_command_undo_command_) {
    has_current_item_always_been_top_most_item = false;
  } else {
    is_last_command_undo_command_ = true;
  }

  // finder type commands will be executed only if has_current_item_always_been_top_most_item is true
  if (undo_stack_->canUndo())
    undo_stack_->undo();
  while (undo_stack_->canUndo() && !last_command_changed_something) {
    last_command_changed_something = true;
    undo_stack_->undo();
  }
}

void UndoRedoStack::redo(boost::shared_ptr<RoomUndoRedoState> current_state) {
  closeAndPushAnyOpenRoomStateUndoCommand(current_state);
  last_command_changed_something = true;
  is_last_command_undo_command_ = false;

  // finder type commands will be executed only if has_current_item_always_been_top_most_item is true
  if (undo_stack_->canRedo())
    undo_stack_->redo();
  while (undo_stack_->canRedo() && !last_command_changed_something) {
    last_command_changed_something = true;
    undo_stack_->redo();
  }
}
