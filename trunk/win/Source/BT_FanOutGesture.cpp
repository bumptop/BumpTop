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
#include "BT_FanOutGesture.h"
#include "BT_GestureContext.h"
#include "BT_Pile.h"
#include "BT_RenderManager.h"
#include "BT_SceneManager.h"
#include "BT_Selection.h"
#include "BT_TextManager.h"
#include "BT_Util.h"
#include "BT_WindowsOS.h"

const float FanOutGesture::MAXIMUM_SEPARATION_CHANGE = TO_PIXELS(208.0f); 
const float FanOutGesture::MINIMUM_PATH_LENGTH = TO_PIXELS(100.0f);

FanOutGesture::FanOutGesture() :
	Gesture("Fan Out", 2, 2, false)
{
	clearGesture();
}

void FanOutGesture::clearGesture()
{
	_pile = NULL;
}

Gesture::Detected FanOutGesture::isRecognizedImpl(GestureContext *gestureContext)
{
	_gestureTouchPaths = gestureContext->getActiveTouchPaths();

	DWORD time = max(_gestureTouchPaths[0]->getFirstTouchPoint().relativeTimeStamp, _gestureTouchPaths[1]->getFirstTouchPoint().relativeTimeStamp);
	float dist0 = _gestureTouchPaths[0]->getTouchPointAt(time).calculateDisplacementFrom(_gestureTouchPaths[0]->getLastTouchPoint()).magnitude();
	float dist1 = _gestureTouchPaths[1]->getTouchPointAt(time).calculateDisplacementFrom(_gestureTouchPaths[1]->getLastTouchPoint()).magnitude();

	if (dist0 < MINIMUM_PATH_LENGTH || dist1 < MINIMUM_PATH_LENGTH)
		return Maybe; // Wait until fingers has moved enough to tell direction

	float separation0 = _gestureTouchPaths[0]->getTouchPointAt(time).calculateDisplacementFrom(_gestureTouchPaths[1]->getTouchPointAt(time)).magnitude();
	float separation1 = _gestureTouchPaths[0]->getLastTouchPoint().calculateDisplacementFrom(_gestureTouchPaths[1]->getLastTouchPoint()).magnitude();

	if (abs(separation0 - separation1) > MAXIMUM_SEPARATION_CHANGE)
		return gestureRejected("Distance between fingers changed too much, probably a zoom gesture");
	
	if (!_pile) 
	{
		// Check if either the first or second finger is on a pile
		_pile = dynamic_cast<Pile *>(_gestureTouchPaths[0]->getFirstTouchPoint().getPickedObject());
		if (!_pile)
			_pile = dynamic_cast<Pile *>(_gestureTouchPaths[1]->getFirstTouchPoint().getPickedObject());
	}

	if (!_pile)
		return gestureRejected("No finger is on a pile");

	if (_pile->getPileState() != Stack)
	{
		_pile = NULL;
		return gestureRejected("Pile must be stacked to fan out");
	}

	_lastPoint = _gestureTouchPaths[0]->getLastTouchPoint().calculateMidPoint(_gestureTouchPaths[1]->getLastTouchPoint());
	
	FinishModeBasedOnSelection();
	_pile->beginFanout();
	textManager->invalidate();

	return gestureAccepted();	
}

bool FanOutGesture::processGestureImpl(GestureContext *gestureContext)
{
	if (gestureContext->getNumActiveTouchPoints() != 2)
	{
		if (_pile->isFanningOut())
			_pile->endFanout();
		GLOBAL(mode) = None;
		sel->add(_pile->getFirstItem()); // Select first item in pile after fan out (context menu activated fan out does this) 
		// Side effect: this allows the close widget to be clicked on, mouse / touch move, then up and still trigger the close.
		_pile = NULL;
		return false;
	}

	Vec3 point = _gestureTouchPaths[0]->getLastTouchPoint().calculateMidPoint(_gestureTouchPaths[1]->getLastTouchPoint());
	if ((point - _lastPoint).magnitude() < 15)
		return true; // avoid adding point to path if it's too close
	
	_lastPoint = point;
	GLOBAL(mode) = FanoutOnStrokeMode; // Set FanoutOnStrokeMode after the fake mouse up to avoid premature termination

	point = ClientToWorld(int(point.x), int(point.y), 0);

	if (!_pile->fanoutTick(point) || !_pile->isFanningOut())
	{	// Probably due to too many path points
		GLOBAL(mode) = None;
		sel->add(_pile->getFirstItem()); // Select first item in pile after fan out (context menu activated fan out does this) 
		// Side effect: this allows the close widget to be clicked on, mouse / touch move, then up and still trigger the close.
		_pile = NULL;
		return false;
	}

	_pile->updatePileItems(true);
	rndrManager->invalidateRenderer();

	return true;
}
