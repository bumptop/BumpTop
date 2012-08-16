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
#include "BT_MousePointer.h"

MousePointer::MousePointer()
{
	POINT pt;
	GetCursorPos(&pt);
	x = pt.x;
	y = pt.y;
	mouseButtons = 0;
}

MousePointer::MousePointer(int x, int y)
{
	this->x = x;
	this->y = y;
}

MousePointer::~MousePointer()
{
}

int MousePointer::getX()
{
	return x;
}

int MousePointer::getY()
{
	return y;
}

int MousePointer::getMouseButtons()
{
	return mouseButtons;
}

void MousePointer::setX(int value)
{
	x = value;
}

void MousePointer::setY(int value)
{
	y = value;
}

void MousePointer::setMouseButtons(int value)
{
	mouseButtons = value;
}

bool MousePointer::isMouseLeftDown()
{
	return (mouseButtons & MouseButtonLeft) != 0;
};

bool MousePointer::isMouseMiddleDown()
{
	return (mouseButtons & MouseButtonMiddle) != 0;
};

bool MousePointer::isMouseRightDown()
{
	return (mouseButtons & MouseButtonRight) != 0;
};