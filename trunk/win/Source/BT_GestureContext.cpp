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
#include "BT_GestureContext.h"
#include "BT_Path.h"
#include "BT_Util.h"

GestureContext::GestureContext()
{}

void GestureContext::clear()
{
	_activeTouchPaths.clear();
	_touchPaths.clear();
	_startTime = 0;
}

void GestureContext::addTouchPoint(TouchPoint &touchPoint, uint id, ulong timestamp)
{
	if (touchPoint.state == TouchPoint::Down)
	{
		// If the paths are empty, this means this is our first touch point,
		// so we have to set the start time.
		if (_touchPaths.isEmpty())
			_startTime = timestamp;
		
		touchPoint.velocity = Vec3(0.0f);
		touchPoint.relativeTimeStamp = timestamp - _startTime;
		// This is for random coloured touch points. Removed for now.
		// _touchPaths.append(Path(QColor(rand() % 256, rand() % 256, rand() % 256)));
		_touchPaths.append(Path());
		assert(!_activeTouchPaths.contains(id));
		_activeTouchPaths[id] = &_touchPaths.last();
	}
	else
	{
		assert(_activeTouchPaths.contains(id));		
		TouchPoint& previousPoint = _activeTouchPaths[id]->getLastTouchPoint();
		touchPoint.relativeTimeStamp = timestamp - _startTime;
		touchPoint.velocity = touchPoint.calculateVelocityFrom(previousPoint);
		_activeTouchPaths[id]->pathLength += touchPoint.calculateDisplacementFrom(previousPoint).magnitude();
	}

	//Add the touch point to the path with the same touch point ids.
	touchPoint.color = _activeTouchPaths[id]->color;
	_activeTouchPaths[id]->points.append(touchPoint);

	if (touchPoint.state == TouchPoint::Up)
	{
		_activeTouchPaths.remove(id);
	}
}

QList<Path*> GestureContext::getActiveTouchPaths()
{
	return _activeTouchPaths.values();
}

Path* GestureContext::getTouchPath(uint position)
{
	return &_touchPaths.value(position);
}

uint GestureContext::getNumActiveTouchPoints()
{
	return _activeTouchPaths.size();
}

bool GestureContext::isEmpty()
{
	return _touchPaths.isEmpty();
}

void GestureContext::setManipulationData(const Manipulation& manipData)
{
	manipulation = manipData;
}

const Manipulation& GestureContext::getManipulation() const
{
	return manipulation;
}