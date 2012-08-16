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
#include "BT_OverlayComponent.h"
#include "BT_OverlayEvent.h"
#include "BT_WindowsOS.h"


//
// OverlayEvent implementation
//
OverlayEvent::OverlayEvent( OverlayComponent * target )
: _target(target)
, _time(winOS->GetTime())
{
	assert(target);
}

OverlayEvent::~OverlayEvent()
{}

OverlayComponent * OverlayEvent::getTarget() const
{
	return _target;
}

unsigned int OverlayEvent::getTime() const
{
	return _time;
}


//
// MouseOverlayEvent implementation
//
MouseOverlayEvent::MouseOverlayEvent( OverlayComponent * target, const Vec3& screenPos, const Vec3& absScreenPos, int button, bool intersectsTarget )
: OverlayEvent(target)
, _screenPos(screenPos)
, _absScreenPos(absScreenPos)
, _button(button)
, _intersectsTarget(intersectsTarget)
{}

MouseOverlayEvent::~MouseOverlayEvent()
{}

const Vec3& MouseOverlayEvent::getPosition() const
{
	return _screenPos;	
}

const Vec3& MouseOverlayEvent::getAbsolutePosition() const
{
	return _absScreenPos;
}

int MouseOverlayEvent::getButton() const
{
	return _button;
}

bool MouseOverlayEvent::intersectsTarget() const
{
	return _intersectsTarget;
}

//
// TimerOverlayEvent implementation
//
TimerOverlayEvent::TimerOverlayEvent( OverlayComponent * target, unsigned int millisElapsed )
: OverlayEvent(target)
, _millisElapsed(millisElapsed)
{}

TimerOverlayEvent::~TimerOverlayEvent()
{}

unsigned int TimerOverlayEvent::getElapsed() const
{
	return _millisElapsed;
}


//
// MouseOverlayEventHandler implementation
//
MouseOverlayEventHandler::MouseOverlayEventHandler()
: _wasHovered(false)
{}

bool MouseOverlayEventHandler::wasMouseLastHovered() const
{
	return _wasHovered;
}

void MouseOverlayEventHandler::setMouseLastHovered(bool value)
{
	_wasHovered = value;
}

bool MouseOverlayEventHandler::intersects( const Vec3& pos, const Bounds& bounds )
{
	const Vec3& min = bounds.getMin();
	const Vec3& max = bounds.getMax();

	return (pos.x >= min.x) && (pos.x <= max.x) && (pos.y > min.y) && (pos.y < max.y);
}