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

#ifndef _BT_KEYBOARD_MANAGER_
#define _BT_KEYBOARD_MANAGER_

// -----------------------------------------------------------------------------

#include "BT_Singleton.h"

// -----------------------------------------------------------------------------

class KeyCombo;
class KeyboardEventHandler;

// -----------------------------------------------------------------------------
struct KeyCallback
{
	// Blocking specifies whether the call back should only be called when previous call has finished. 
	// Sometimes calls dispatch messages that cause the same call again from the same thread
	bool blocking, inCall;
	VoidCallback callBack;
	KeyCallback() : callBack(NULL), blocking(false), inCall(false) {}
	KeyCallback(VoidCallback CallBack, bool Blocking = false) : callBack(CallBack), blocking(Blocking), inCall(false) {}
};

typedef map<KeyCombo, KeyCallback> KeyCallbacks;

// -----------------------------------------------------------------------------

class KeyboardManager
{
	vector<KeyboardEventHandler *> handlerList;
	KeyCallbacks callbackMap;
	
	// Singleton
	friend class Singleton<KeyboardManager>;
	KeyboardManager();

public:

	~KeyboardManager();

	// Actions
	void mapKeyToFunction(KeyCombo &combo, VoidCallback callback);
	
	void init();

	// Events
	void onKeyDown(KeyCombo &combo);
	void onKeyUp(KeyCombo &combo);

	// Event Handler Registration
	bool addHandler(KeyboardEventHandler *eventHandler);
	bool removeHandler(KeyboardEventHandler *eventHandler);

	// Getters
	bool isKeyMapped(unsigned char key);
};

// -----------------------------------------------------------------------------

#define keyManager Singleton<KeyboardManager>::getInstance()

// -----------------------------------------------------------------------------

#else
	class KeyboardManager;
#endif