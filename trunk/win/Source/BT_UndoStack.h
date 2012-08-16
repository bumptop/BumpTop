// Copyright 2012 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#ifndef _BT_UNDO_STACK_
#define _BT_UNDO_STACK_

#ifdef USING_UNDO
// -----------------------------------------------------------------------------

#include "BT_Singleton.h"
#include "BT_MenuActionCustomizer.h"
#include "BT_ActorUndoData.h"
#include "BT_UndoStack.h"
#include "BT_MenuAction.h"
#include "BT_UndoStackEntry.h"
#include "BT_Stopwatch.h"

// -----------------------------------------------------------------------------

class UndoStack : public MenuActionCustomizer
{

	deque<UndoStackEntry>			undoStack;
	deque<UndoStackEntry>			redoStack;
	bool sceneNeedsSaving;
	Stopwatch saveTimeout;

	vector<ActorUndoData> getActorOri();
	
	// Singleton
	friend class Singleton<UndoStack>;
	UndoStack();

public:

	~UndoStack();

	// Actions
	void update();
	bool undo();
	bool redo();
	bool saveState();
	void update(MenuAction *action);
	void execute(MenuAction *action);
	void invalidate();

	// Getters
	int getUndoCount();
	int getRedoCount();

};

// -----------------------------------------------------------------------------

#define undoManager Singleton<UndoStack>::getInstance()

// -----------------------------------------------------------------------------

#endif

#else
	class UndoStack;
#endif