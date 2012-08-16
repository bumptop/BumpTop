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
#include "BT_WindowsOS.h"
#include "BT_MarkingMenu.h"
#include "BT_Util.h"

#include "BT_RightClickGesture.h"

// This value should correspond to the system values when the new version 
// of the SDK comes out.
// http://msdn.microsoft.com/en-us/library/ee220938.aspx
const float RightClickGesture::MINIMUM_WAIT_TIME = 901.0f;
const float RightClickGesture::MAXIMUM_PATH_LENGTH = TO_PIXELS(197.0f);

RightClickGesture::RightClickGesture() :
	Gesture("RightClick", 1, 1, true)
{
	clearGesture();
}

void RightClickGesture::clearGesture()
{}

RightClickGesture::Detected RightClickGesture::isRecognizedImpl(GestureContext *gestureContext)
{
	_gestureTouchPaths = gestureContext->getActiveTouchPaths();

	Path* fingerPath = _gestureTouchPaths.first();
	TouchPoint& initialPoint = fingerPath->getFirstTouchPoint();
	TouchPoint& finalPoint = fingerPath->getLastTouchPoint();

	float displacement = fingerPath->getTotalDisplacementVector().magnitude();
	if (displacement > MAXIMUM_PATH_LENGTH)
		return gestureRejected(QString("Total path displacement (%1) greater than maximum (%2)").arg(displacement).arg(MAXIMUM_PATH_LENGTH));

	if (finalPoint.relativeTimeStamp - initialPoint.relativeTimeStamp > MINIMUM_WAIT_TIME)
	{
		// We bring up right click menu by faking right clicks and let MouseEventManager invoke the menu.
		winOS->OnMouse(WM_RBUTTONDOWN, finalPoint.x, finalPoint.y, MK_RBUTTON);
		winOS->OnMouse(WM_RBUTTONUP, finalPoint.x, finalPoint.y, MK_RBUTTON);
		return gestureAccepted();
	}

	return Maybe;
}

bool RightClickGesture::processGestureImpl(GestureContext *gestureContext)
{
	TouchPoint& clickLocation = _gestureTouchPaths.first()->getLastTouchPoint();

	if (gestureContext->getNumActiveTouchPoints() == 0)
	{
 		// Previously we destroyed the menu if no item was selected, but to match Windows behaviour,
		// we keep the menu on screen after the finger has been lifted. 

		// Finish processing the gesture
		return false;
	}
	
	// Mouse move-events are blocked during gestures, so forward the mouse move events.
	winOS->OnMouseMove(clickLocation.x, clickLocation.y, MK_LBUTTON);
	
	// Continue processing the gesture
	return true;
}
