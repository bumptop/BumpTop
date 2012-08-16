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
#include "BT_CameraZoomGesture.h"
#include "BT_GestureContext.h"
#include "BT_SceneManager.h"
#include "BT_Selection.h"
#include "BT_Util.h"
#include "BT_WindowsOS.h"

const float CameraZoomGesture::MINIMUM_PATH_LENGTH = TO_PIXELS(520.0f); //50 pixels
const float CameraZoomGesture::MINIMUM_LINGERING_ZOOM_SPEED = TO_PIXELS(7.3f);	// 0.7 pixels
const float CameraZoomGesture::MAXIMUM_TOLERANCE_ANGLE = 120.0f;

CameraZoomGesture::CameraZoomGesture() :
Gesture("Camera Zoom", 2, 2, false)
{
	clearGesture();
}

void CameraZoomGesture::clearGesture()
{
	_recalculateInitials = false;
}

void CameraZoomGesture::calculateInitials(TouchPoint & firstFingerEndPoint, TouchPoint & secondFingerEndPoint)
{
	_originalDistanceBetweenPoints = firstFingerEndPoint.calculateDisplacementFrom(secondFingerEndPoint).magnitude();
	_originalCamEye = cam->getEye();
	
	Vec3 midPoint = firstFingerEndPoint.calculateMidPoint(secondFingerEndPoint);
	midPoint = unProjectToDesktop(midPoint.x, midPoint.y).get<2>();
	_camDirection = midPoint - _originalCamEye;
	_camDirection.normalize();	
}

Gesture::Detected CameraZoomGesture::isRecognizedImpl(GestureContext *gestureContext)
{
	_gestureTouchPaths = gestureContext->getActiveTouchPaths();

	DWORD time = max(_gestureTouchPaths[0]->getFirstTouchPoint().relativeTimeStamp, _gestureTouchPaths[1]->getFirstTouchPoint().relativeTimeStamp);
	TouchPoint& firstFingerStartPoint = _gestureTouchPaths[0]->getTouchPointAt(time);
	TouchPoint& secondFingerStartPoint = _gestureTouchPaths[1]->getTouchPointAt(time);

	TouchPoint& firstFingerEndPoint = _gestureTouchPaths[0]->getLastTouchPoint();
	TouchPoint& secondFingerEndPoint = _gestureTouchPaths[1]->getLastTouchPoint();

	BumpObject * firstFingerObject = firstFingerStartPoint.getPickedObject();
	BumpObject * secondFingerObject = secondFingerStartPoint.getPickedObject();

	if (firstFingerObject && firstFingerObject == secondFingerObject)
		return gestureRejected("Fingers are on the same object (this is pinch zoom)");
	if(sel->isInSelection(firstFingerObject) || sel->isInSelection(secondFingerObject))
		return gestureRejected("Fingers are on objects of an existing selection (this is pinch zoom)");
	
	calculateInitials(firstFingerEndPoint, secondFingerEndPoint);
	float initialDistanceBetweenFingers = firstFingerStartPoint.calculateDisplacementFrom(secondFingerStartPoint).magnitude();
	// NOTE: check depends on _originalDistanceBetweenPoints from calculateInitials
	if (abs(initialDistanceBetweenFingers - _originalDistanceBetweenPoints) < MINIMUM_PATH_LENGTH)
		return Maybe;
	
	Vec3 axis = firstFingerStartPoint.getPositionVector() - secondFingerStartPoint.getPositionVector();
	axis.normalize();
	Vec3 firstFingerDir = _gestureTouchPaths[0]->getTotalDisplacementVector();
	Vec3 secondFingerDir = _gestureTouchPaths[1]->getTotalDisplacementVector();
	firstFingerDir.normalize();
	secondFingerDir.normalize();
	//if sum of smallest angles between fingers' paths and the initial line between fingers > MAXIMUM_TOLERANCE_ANGLE, it's not zoom
	float angleSum = (acos(abs(firstFingerDir.dot(axis))) + acos(abs(secondFingerDir.dot(axis)))) * (180.0f / NxPiF32);
	if (angleSum > MAXIMUM_TOLERANCE_ANGLE)
		return gestureRejected("Fingers not moving in opposite directions");

	cam->setIsCameraFreeForm(true, false);

	return gestureAccepted();
}

bool CameraZoomGesture::processGestureImpl(GestureContext *gestureContext)
{
	QList<Path *> activePaths = gestureContext->getActiveTouchPaths();
	if(gestureContext->getNumActiveTouchPoints() != 2)
	{
		// The gesture is active as long as the one of the two initial fingers is still down
		if (activePaths.count() == 1)
		{
			if (activePaths.contains(_gestureTouchPaths[0]) || activePaths.contains(_gestureTouchPaths[1]))
			{
				_recalculateInitials = true;
				return true; // still remain as zoom gesture, user probably repositioning one finger
			}
		}
		return false;
	}
	TouchPoint& firstFingerEndPoint = activePaths[0]->getLastTouchPoint();
	TouchPoint& secondFingerEndPoint = activePaths[1]->getLastTouchPoint();

	if (_recalculateInitials)
	{
		_recalculateInitials = false;
		calculateInitials(firstFingerEndPoint, secondFingerEndPoint);
	}
	
	_camDirection.normalize();
	Ray cameraRay(_originalCamEye, _camDirection);
	Ray behindCameraRay(_originalCamEye, -_camDirection);
	Vec3 intersectionPoint = RayIntersectDesktop(cameraRay, GLOBAL(ZoomBuffer)).get<2>();
	Vec3 farthestPoint = RayIntersectDesktop(behindCameraRay, GLOBAL(ZoomBuffer)).get<2>();

	float distance = intersectionPoint.distance(_originalCamEye);
	float maxDistance = intersectionPoint.distance(farthestPoint);
	float percent = distance / maxDistance;
	
	float initialFactor = 1.0f / (percent * 2.0f);

	float distanceFactor = 0.003f;	

	float newDistance = firstFingerEndPoint.calculateDisplacementFrom(secondFingerEndPoint).magnitude();
	float fingerDifference = (newDistance - _originalDistanceBetweenPoints) * distanceFactor;
	
	float zoomFactor = initialFactor + fingerDifference;
	
	// 0.25 is the smallest zoomFactor can be in order for newPercent to be a max of 2.0.
	zoomFactor = NxMath::max(0.25f, zoomFactor);
	
	float newPercent = 1.0f / (zoomFactor * 2.0f);
	
	//New Camera position
	Vec3 newEye = farthestPoint + (_camDirection * ((1.0f - newPercent) * maxDistance));
	
	cam->adjustPointInDesktop(farthestPoint, newEye, NxPlane());
	cam->setEye(newEye);
	
	return true;
}