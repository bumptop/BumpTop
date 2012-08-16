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

#ifndef _MOCK_KEYBOARD_EVENTHANDLER_
#ifdef BT_UTEST

#include "BT_KeyboardHandler.h"

class MockKeyboardEventHandler : public KeyboardEventHandler
{
	unsigned int _prevKey;

public:
	MockKeyboardEventHandler(void);
	virtual ~MockKeyboardEventHandler(void);

	// setters
	void reset();
	// getters
	unsigned int getLastKey() const;

	// Events
	virtual bool onKeyUp(KeyCombo &key);
	virtual bool onKeyDown(KeyCombo &key);
};

#endif // BT_UTEST
#endif // _MOCK_KEYBOARD_EVENTHANDLER_
