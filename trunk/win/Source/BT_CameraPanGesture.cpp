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
#include "BT_CameraPanGesture.h"
#include "BT_GestureContext.h"
#include "BT_OverlayComponent.h"
#include "BT_SceneManager.h"
#include "BT_Util.h"

const float CameraPanGesture::SPEED_MULTIPLIER = 10.0f;

CameraPanGesture::CameraPanGesture() :
Gesture("Camera Pan", 2, 2, false)
{
	clearGesture();
}

void CameraPanGesture::clearGesture()
{
	_lastVector = Vec3(0.0f);
}

Gesture::Detected CameraPanGesture::isRecognizedImpl(GestureContext *gestureContext)
{
	if (gestureContext->getActiveTouchPaths()[0]->getFirstTouchPoint().getPickedObject())
		return gestureRejected("First finger is on an object");
	if (gestureContext->getActiveTouchPaths()[1]->getFirstTouchPoint().getPickedObject())
		return gestureRejected("Second finger is on an object");

	Detected result = isTwoFingerSwipe(gestureContext);
	if (result == Yes)
	{
		_gestureTouchPaths = gestureContext->getActiveTouchPaths();
		tuple<int, NxReal, Vec3, NxPlane> t = _gestureTouchPaths[0]->getLastTouchPoint().unProjectToDesktop();      
		_pressurePoint = t.get<2>();	
		_translationPlane = t.get<3>();
		cam->setIsCameraFreeForm(true, false);
		return gestureAccepted();
	}
	else if (result == No) {
		gestureRejected("Not a two finger swipe");
	}
	return Maybe;
}

bool CameraPanGesture::processGestureImpl(GestureContext *gestureContext)
{
	if (gestureContext->getNumActiveTouchPoints() != 2)
	{
		cam->killAnimation();
		_lastVector.normalize();
		float velocity = _gestureTouchPaths[0]->getCurrentVelocity().magnitude();
		Vec3 cameraVector = _lastVector * velocity * SPEED_MULTIPLIER;
		
		// New eye position
		Vec3 newEye = cam->getEye() + cameraVector;

		cam->animateToWithSliding(newEye, cam->getDir(), cam->getUp(), 10, false);
		
		return false;
	}
	
	cam->revertAnimation();
	
	TouchPoint& touchPoint = _gestureTouchPaths[0]->getLastTouchPoint();
	
	Vec3 worldPoint = touchPoint.unProject(_translationPlane);
	
	// Create the vector between the original point and the new point
	Vec3 cameraVector = _pressurePoint - worldPoint;
	
	// New eye position
	Vec3 newEye = cam->getEye() + cameraVector;
	
	cam->animateToWithSliding(newEye, cam->getDir(), cam->getUp(), 5, false);
	
	_lastVector = cameraVector;
	return true;
}