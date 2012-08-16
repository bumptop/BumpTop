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
#include "BT_Camera.h"
#include "BT_PullDownWallsGesture.h"
#include "BT_GestureContext.h"
#include "BT_SceneManager.h"
#include "BT_Util.h"

PullDownWallsGesture::PullDownWallsGesture() :
	Gesture("Pull Down Walls", 2, 2, false)
{
	clearGesture();
}

void PullDownWallsGesture::clearGesture()
{}

Gesture::Detected PullDownWallsGesture::isRecognizedImpl(GestureContext *gestureContext)
{
	Detected result = isTwoFingerSwipe(gestureContext);
	if (result == Yes)
	{
		_gestureTouchPaths = gestureContext->getActiveTouchPaths();
		tuple<int, NxReal, Vec3, NxPlane> t = _gestureTouchPaths[0]->getFirstTouchPoint().unProjectToDesktop();
		_wallNumber = t.get<0>();

		if (_wallNumber == -1)
			return gestureRejected("Touch did not begin on a wall");

		Vec3 firstPoint = t.get<2>();	
		NxPlane translationPlane = t.get<3>();
		Vec3 lastPoint = _gestureTouchPaths[0]->getLastTouchPoint().unProject(translationPlane);

		if (lastPoint.y < firstPoint.y)
			_upDirection = false;
		else
			_upDirection = true;

		return gestureAccepted();
	}
	else if (result == No)
	{
		return gestureRejected("Not a two-finger swipe");
	}
	return result;
}

bool PullDownWallsGesture::processGestureImpl(GestureContext *gestureContext)
{
	if (_upDirection)
	{
		Key_ZoomToAll();
	}
	else
	{
		Vec3 position, direction;
		cam->lookAtWall(GLOBAL(Walls)[_wallNumber], position, direction);
		cam->animateTo(position, direction);
	}
	return false;
}