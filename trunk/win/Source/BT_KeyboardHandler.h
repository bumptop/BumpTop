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

#ifndef _BT_KEYBOARD_HANDLER_
#define _BT_KEYBOARD_HANDLER_

// -----------------------------------------------------------------------------

class KeyCombo;

// -----------------------------------------------------------------------------

class KeyboardEventHandler
{

public:

	KeyboardEventHandler();
	~KeyboardEventHandler();

	// Actions
	void registerKeyboardHandler();

	// Events
	virtual bool onKeyUp(KeyCombo &key) = 0;
	virtual bool onKeyDown(KeyCombo &key) = 0;

};

// -----------------------------------------------------------------------------

#else
	class KeyboardEventHandler;
#endif