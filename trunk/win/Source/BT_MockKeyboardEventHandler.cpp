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

#include "BT_Common.h"
#ifdef BT_UTEST
#include "BT_KeyCombo.h"
#include "BT_MockKeyboardEventHandler.h"

MockKeyboardEventHandler::MockKeyboardEventHandler(void)
: _prevKey(0)
{
}

MockKeyboardEventHandler::~MockKeyboardEventHandler(void)
{
}

bool MockKeyboardEventHandler::onKeyUp(KeyCombo &key)
{
	return true;
}

bool MockKeyboardEventHandler::onKeyDown(KeyCombo &key)
{
	_prevKey = key.subKeys.key;
	return true;
}

unsigned int MockKeyboardEventHandler::getLastKey() const
{
	return _prevKey;
}

void MockKeyboardEventHandler::reset()
{
	_prevKey = 0;
}

#endif // BT_UTEST