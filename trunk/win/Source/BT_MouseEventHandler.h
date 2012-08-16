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

#ifndef _BT_MOUSE_EVENT_HANDLER_
#define _BT_MOUSE_EVENT_HANDLER_

// -----------------------------------------------------------------------------

#include "BT_MousePointer.h"

// -----------------------------------------------------------------------------

class MouseEventHandler
{
	// Register with global Event Handling class
public:

	MouseEventHandler();
	~MouseEventHandler();

	// Actions
	void registerMouseHandler();

	// Events
	virtual bool onMouseUp(Vec2 &pt, MouseButtons button) { return false; };
	virtual bool onMouseDown(Vec2 &pt, MouseButtons button) { return false; };
	virtual void onMouseMove(Vec2 &pt) {};

};

// -----------------------------------------------------------------------------

#else
	class MouseEventHandler;
#endif