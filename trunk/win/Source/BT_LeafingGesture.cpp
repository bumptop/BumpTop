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
#include "BT_LeafingGesture.h"
#include "BT_OverlayComponent.h"
#include "BT_Pile.h"
#include "BT_SceneManager.h"
#include "BT_Util.h"
#include "BT_WindowsOS.h"

const float LeafingGesture::MAXIMUM_SEPARATION_CHANGE = TO_PIXELS(208.0f); 
const float LeafingGesture::MINIMUM_PATH_LENGTH = TO_PIXELS(100.0f);
const float LeafingGesture::MAXIMUM_ANGLE_THRESHOLD = 60.0f;
const float LeafingGesture::SLOW_SPEED_THRESHHOLD = TO_PIXELS(3.125f); //3.125 hundredths of inches per millisecond

LeafingGesture::LeafingGesture() :
Gesture(QT_NT("Leafing"), 2, 2, false) // Gesture is recognized when 1 finger is lifted
{
	clearGesture();
}

void LeafingGesture::clearGesture()
{
	_leafPile = NULL;
	_startPoint = Vec3(0.0f);
	_wasLastSpeedFast = false;
	_startAtTop = false;
	_leafIndex = -1;
	_leafPileCenter = Vec3(0.0f);
	_lastTime = 0;
}

Gesture::Detected LeafingGesture::isRecognizedImpl(GestureContext *gestureContext)
{
	_gestureTouchPaths = gestureContext->getActiveTouchPaths();

	DWORD time = max(_gestureTouchPaths[0]->getFirstTouchPoint().relativeTimeStamp, _gestureTouchPaths[1]->getFirstTouchPoint().relativeTimeStamp);
	float dist0 = _gestureTouchPaths[0]->getTouchPointAt(time).calculateDisplacementFrom(_gestureTouchPaths[0]->getLastTouchPoint()).magnitude();
	float dist1 = _gestureTouchPaths[1]->getTouchPointAt(time).calculateDisplacementFrom(_gestureTouchPaths[1]->getLastTouchPoint()).magnitude();

	if (dist0 < MINIMUM_PATH_LENGTH || dist1 < MINIMUM_PATH_LENGTH)
		return Maybe; // Wait until fingers have moved enough to tell direction

	float separation0 = _gestureTouchPaths[0]->getTouchPointAt(time).calculateDisplacementFrom(_gestureTouchPaths[1]->getTouchPointAt(time)).magnitude();
	float separation1 = _gestureTouchPaths[0]->getLastTouchPoint().calculateDisplacementFrom(_gestureTouchPaths[1]->getLastTouchPoint()).magnitude();

	if (abs(separation0 - separation1) > MAXIMUM_SEPARATION_CHANGE)
		return gestureRejected(QT_NT("Distance between fingers changed too much, probably a zoom gesture"));
	
	Vec3 direction;
	Gesture::Detected isSwipe = isTwoFingerSwipe(gestureContext, direction);
	if (isSwipe == Maybe)
		return Maybe;
	else if (isSwipe == No)
		return gestureRejected(QT_NT("Not a swipe"));	
	
	if (!isSameDirection(direction, Vec3(0.0f, 1.0f, 0.0f), 45.0f))
	{
		if (!isSameDirection(direction, Vec3(0.0f, -1.0f, 0.0f), 45.0f))
			return gestureRejected(QT_NT("Not down or up"));
		_startAtTop = false;
	}
	else
		_startAtTop = true;

	if (!_leafPile)
	{
		// Check if either the first or second finger is on a pile
		_leafPile = dynamic_cast<Pile *>(_gestureTouchPaths[0]->getFirstTouchPoint().getPickedObject());
		if (!_leafPile)
			_leafPile = dynamic_cast<Pile *>(_gestureTouchPaths[1]->getFirstTouchPoint().getPickedObject());
	}

	if (!_leafPile)
		return gestureRejected(QT_NT("No finger is on a pile"));

	if (_leafPile->getPileState() != Stack && _leafPile->getPileState() != Leaf)
	{
		_leafPile = NULL;
		return gestureRejected(QT_NT("Pile must be stacked or leafed in order to leaf"));
	}

	_startPoint = _gestureTouchPaths[0]->getFirstTouchPoint().getPositionVector();
	_leafPile->getScreenBoundingBox().getCenter(_leafPileCenter);
	_wasLastSpeedFast = false;
	
	if (!_startAtTop && _leafPile->getPileState() != Leaf)
		_leafPile->leafToBottom();
	
	_leafIndex = _leafPile->getLeafIndex();

	return gestureAccepted();
}

bool LeafingGesture::processGestureImpl(GestureContext *gestureContext)
{
	// Some constants. May need tweaking
	const ulong timeThreshold = 2000;
	const ulong itemsToLeafInOneStroke = 10;

	Vec3 currentPoint = _gestureTouchPaths[0]->getLastTouchPoint().getPositionVector();
	Vec3 averageVelocity = _gestureTouchPaths[0]->getLastAverageVelocity(10);
	if (averageVelocity.magnitude() > SLOW_SPEED_THRESHHOLD)
	{
		if (!_wasLastSpeedFast)
		{
			_lastTime = _gestureTouchPaths[0]->getLastTouchPoint().relativeTimeStamp;
		}
		else
		{
			int verticalDisplacement = (int)roundOffDecimal(currentPoint.y - _startPoint.y);
			ulong currentTime = _gestureTouchPaths[0]->getLastTouchPoint().relativeTimeStamp;
			if (_lastTime - currentTime > timeThreshold / averageVelocity.magnitude())
			{
				if (averageVelocity.y < 0)
					_leafPile->leafUp();
				else
					_leafPile->leafDown();
			}
		}
		_wasLastSpeedFast = true;
	}
	else
	{
		if (_wasLastSpeedFast)
		{
			// Set the anchor point to determine how much we have moved
			_startPoint = _gestureTouchPaths[0]->getLastTouchPoint().getPositionVector();
			_leafIndex = _leafPile->getLeafIndex();
		}
		else
		{
			// Code here to determine how many items to move
			int verticalDisplacement = (int)roundOffDecimal(currentPoint.y - _startPoint.y);
			int interval = max(winOS->GetWindowHeight() - _leafPileCenter.y, _leafPileCenter.y) / itemsToLeafInOneStroke;
			int leafIndex = (verticalDisplacement / interval) + _leafIndex;
			_leafPile->leafTo(leafIndex);
		}
		_wasLastSpeedFast = false;
	}
	return true;
}