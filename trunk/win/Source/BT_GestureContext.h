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

#ifndef _BT_GESTURE_CONTEXT_
#define _BT_GESTURE_CONTEXT_

#include "BT_TouchPoint.h"
#include "BT_Manipulation.h"

class BumpObject;
class Path;

// Takes the parameter and converts it from hundredth-of-an-inch to pixels
#define TO_PIXELS_X(x)			winOS->GetWindowDPIX() * x / 1000
#define TO_PIXELS_Y(y)			winOS->GetWindowDPIY() * y / 1000
#define TO_PIXELS(x)			min(winOS->GetWindowDPIX(), winOS->GetWindowDPIY()) * x / 1000

class GestureContext
{
private:
	QHash<uint, Path*> _activeTouchPaths;
	QList<Path> _touchPaths;
	ulong _startTime;
	Manipulation manipulation;

public:
	GestureContext();

	void clear();
	void addTouchPoint(TouchPoint &touchPoint, uint id, ulong timestamp);
	Path* getTouchPath(uint position);
	QList<Path*> getActiveTouchPaths();
	uint getNumActiveTouchPoints();
	const Manipulation& getManipulation() const;
	void setManipulationData(const Manipulation& manipData);
	bool isEmpty();
	
};
#endif /* _BT_GESTURE_CONTEXT_ */