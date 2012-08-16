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

#ifndef _BT_MOUSE_POINTER_
#define _BT_MOUSE_POINTER_

// -----------------------------------------------------------------------------

enum MouseButtons
{
	MouseButtonLeft		= (1 << 0),
	MouseButtonMiddle	= (1 << 1),
	MouseButtonRight	= (1 << 2),
	MouseButtonScrollUp	= (1 << 3),
	MouseButtonScrollDn = (1 << 4),
};

// -----------------------------------------------------------------------------

// This class represents an input device: a particular mouse, or a particular 
// finger (described as an input ID) on a smart board. We pass these along with 
// mouse events so we can differentiate pointers and handle them differently.
class MousePointer
{
	int x;
	int y;
	int mouseButtons;

public:

	MousePointer();
	MousePointer(int x, int y);
	~MousePointer();
	
	int getX();
	int getY();
	int getMouseButtons();

	void setX(int value);
	void setY(int value);
	void setMouseButtons(int value);

	bool isMouseLeftDown();
	bool isMouseMiddleDown();
	bool isMouseRightDown();
};

// -----------------------------------------------------------------------------

#endif