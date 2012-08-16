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
#include "BT_GestureContext.h"
#include "BT_RotateDesktopGesture.h"
#include "BT_SceneManager.h"
#include "BT_Selection.h"
#include "BT_Util.h"
#include "BT_WindowsOS.h"

const float RotateDesktopGesture::MINIMUM_GESTURE_ANGLE = 20.0f;
const float RotateDesktopGesture::SNAP_MINIMUM_ANGLE = 35.0f;
const float RotateDesktopGesture::TARGET_FINGER_DISTANCE = TO_PIXELS(520.0f);

RotateDesktopGesture::RotateDesktopGesture() :
	Gesture("Rotate Desktop", 2, 2, false)
{
	clearGesture();
}

void RotateDesktopGesture::clearGesture()
{
	_animatingToForward = false;
	_recalculateInitials = false;
}

float RotateDesktopGesture::calculateAngle(GestureContext *gestureContext)
{
	float angle = gestureContext->getManipulation().cumulativeRotation;
	float manipAngle = angle / NxPiF32 * 180.0f;
	if (gestureContext->getNumActiveTouchPoints() == 2)
	{
		QList<Path *> paths = gestureContext->getActiveTouchPaths();
		float dist = paths[0]->getFirstTouchPoint().getPositionVector().distance(paths[1]->getFirstTouchPoint().getPositionVector());
		angle *= sqrt(dist / TARGET_FINGER_DISTANCE);
		angle *= 180.0f / NxPiF32;
		_lastAngle = angle;
	}
	else
		angle = _lastAngle; // angle does not change if number of touch points changed
	return angle; // in degrees
}

Gesture::Detected RotateDesktopGesture::isRecognizedImpl(GestureContext *gestureContext)
{
	_gestureTouchPaths = gestureContext->getActiveTouchPaths();
	if (sel->isInSelection(_gestureTouchPaths[0]->getFirstTouchPoint().getPickedObject()))
		return gestureRejected("First finger is on an object");
	if (sel->isInSelection(_gestureTouchPaths[1]->getFirstTouchPoint().getPickedObject()))
		return gestureRejected("Second finger is on an object");

	float angle = calculateAngle(gestureContext);
	if (abs(angle) >  MINIMUM_GESTURE_ANGLE)
	{
		Vec3 camPos, camDir;
		int wallIndex = getWallCameraIsFacing();
		assert(wallIndex != -1);
		cam->lookAtWall(GLOBAL(Walls)[wallIndex], camPos, camDir);
		// make camera face the "forward" wall, so rotation would be to the sides
		if ((cam->getEye() - camPos).magnitude() > 0.5f)
			cam->animateTo(camPos, camDir, Vec3(0, 1, 0), 10); 
		_animatingToForward = true;
		_startDir = cam->getDir();
		_startUp = cam->getUp();
		_startPos = cam->getEye();
		_recalculateInitials = false;
		return gestureAccepted();
	}
	return Maybe;
}

int RotateDesktopGesture::getWallCameraIsFacing()
{
	Vec3 camDirection(cam->getDir().x, cam->getDir().z, 0.0f);
	Vec3 camPosition(cam->getEye().x, cam->getEye().z, 0.0f);
	Vec3 referenceDirection(-1.0f, 0.0f, 0.0f);

	float angle = getAngleBetween(camDirection, referenceDirection);
	Vec3 desktopDims = GetDesktopBox().GetExtents();
	
	if (camDirection.y > 0.0f)
	{
		float frontLeftAngle = getAngleBetween(Vec3(desktopDims.x, desktopDims.z, 0.0f) - camPosition, referenceDirection);
		float frontRightAngle = getAngleBetween(Vec3(-desktopDims.x, desktopDims.z, 0.0f) - camPosition, referenceDirection);
		if (angle < frontRightAngle)
		{
			return 2;
		}
		else if (angle >= frontRightAngle && angle < frontLeftAngle)
		{
			return 0;
		}
		else if (angle >= frontLeftAngle)
		{
			return 3;
		}
	}
	else
	{
		float backRightAngle = getAngleBetween(Vec3(-desktopDims.x, -desktopDims.z, 0.0f) - camPosition, referenceDirection);
		float backLeftAngle = getAngleBetween(Vec3(desktopDims.x, -desktopDims.z, 0.0f) - camPosition, referenceDirection);
		if (angle < backRightAngle)
		{
			return 2;
		}
		else if (angle >= backRightAngle && angle < backLeftAngle)
		{
			return 1;
		}
		else if (angle >= backLeftAngle)
		{
			return 3;
		}
	}
	return -1;
}

pair<Vec3, Vec3> RotateDesktopGesture::calculateCameraPosition()
{
	// Right Direction
	Vec3 referenceDirection(-1.0f, 0.0f, 0.0f);
	Vec3 position, direction;
	Vec3 startPosition, endPosition;
	Vec3 startDirection, endDirection;
	int startWallIndex, endWallIndex;
	float ratio = 1.0f;
	
	Vec3 camDirection(cam->getDir().x, cam->getDir().z, 0.0f);
	Vec3 camPosition(cam->getEye().x, cam->getEye().z, 0.0f);
	float angle = getAngleBetween(camDirection, referenceDirection);
	
	if (camDirection.y > 0.0f)
	{
		// The camera is pointing in the upper quadrants
		if (angle < 90.0f)
		{
			//first quadrant
			ratio = angle / 90.0f;
			startWallIndex = 2;
			endWallIndex = 0;
		}
		else if (angle >= 90.0f && angle <= 180.0f)
		{
			//second quadrant
			ratio = (angle - 90.0f) / 90.0f;
			startWallIndex = 0;
			endWallIndex = 3;
		}
	}
	else
	{
		// The camera is pointing in the lower quadrants
		if (angle < 90.0f)
		{
			//fourth quadrant
			ratio = angle / 90.0f;
			startWallIndex = 2;
			endWallIndex = 1;
		}
		else if (angle >= 90.0f && angle <= 180.0f)
		{
			//third quadrant
			ratio = (angle - 90.0f) / 90.0f;
			startWallIndex = 1;
			endWallIndex = 3;
		}
	}
	
	// Find the two points in the world to interpolate between
	cam->lookAtWall(GLOBAL(Walls)[startWallIndex], startPosition, startDirection);
	cam->lookAtWall(GLOBAL(Walls)[endWallIndex], endPosition, endDirection);
	position = lerp(startPosition, endPosition, ratio);
	direction = lerp(startDirection, endDirection, ratio);

	return make_pair(position, direction);
}

bool RotateDesktopGesture::processGestureImpl(GestureContext *gestureContext)
{
	float angle = calculateAngle(gestureContext);
	QList<Path *> activePaths = gestureContext->getActiveTouchPaths();
	if (_animatingToForward)
	{
		// user may realize accidental trigger of rotation while camera is orienting to face "forward" wall
		if (gestureContext->getNumActiveTouchPoints() != 2) 
		{
			_animatingToForward = false;
			cam->killAnimation();
			cam->animateTo(_startPos, _startDir, _startUp);
			return false;
		}

		if(cam->isAnimating())
			return true; // let camera animate to face the "forward" wall first
		else
		{
			_originalDir = cam->getDir();
			_originalUp = cam->getUp();
			_compensate = angle;
			_animatingToForward = false;
		}
	}

	if (gestureContext->getNumActiveTouchPoints() != 2) 
	{
		if (activePaths.count() == 1)
		{
			if (activePaths.contains(_gestureTouchPaths[0]) || activePaths.contains(_gestureTouchPaths[1]))
			{
				_recalculateInitials = true;
				return true; // still remain as rotate gesture, user probably lifted and repositioning one finger
			}
		}
		if (abs(angle) < SNAP_MINIMUM_ANGLE) // finished rotation gesture, check if a rotation is achieved
		{
			cam->animateTo(_startPos, _startDir, _startUp); // rotation amount too small, revert to original view
			return false;
		}
		// Snap the camera to the nearest wall
		int wallIndex = getWallCameraIsFacing();
		assert(wallIndex != -1);
		Vec3 camDir, camPos;
		cam->lookAtWall(GLOBAL(Walls)[wallIndex], camPos, camDir);
		cam->animateTo(camPos, camDir);
		return false;
	}

	if (_recalculateInitials)
	{
		_recalculateInitials = false;
		// if rotation after repositioning is canceled, revert camera to before lifting and repositioning
		// not revert camera to before rotation gesture is recognized
		_startUp = cam->getUp(); 
		_startDir = cam->getDir();
		_startPos = cam->getEye();
	}

	cam->revertAnimation();
	
	pair<Vec3, Vec3> camPair = calculateCameraPosition();
	cam->setEye(camPair.first);
	
	// Rotate the camera; take out the amount used to trigger rotation and face "forward" wall to avoid too much initial rotation
	float rotationAngle = angle - _compensate;
	Quat rotation(rotationAngle, Vec3(0.0f, 1.0f, 0.0f));
	Vec3 dir = _originalDir;
	Vec3 up = _originalUp;
	rotation.rotate(dir);
	rotation.rotate(up);

	dir.sety(camPair.second.y);
	
	cam->animateTo(cam->getEye(), dir, up, 5, false);	
	
	return true;
}