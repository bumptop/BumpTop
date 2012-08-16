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
#include "BT_ShoveGesture.h"

#include "BT_GestureContext.h"
#include "BT_MarkingMenu.h"
#include "BT_LassoMenu.h"
#include "BT_SceneManager.h"
#include "BT_Util.h"

const float ShoveGesture::MAXIMUM_LINE_DEVIATION = 50;
const int ShoveGesture::MINIMUM_SIDE_CONTACT_HYPOTENUSE = 11000;

ShoveGesture::ShoveGesture() : 
	Gesture("Shove Gesture", 1, 5, false)
{
	clearGesture();
}

void ShoveGesture::clearGesture()
{
	_shoveGesturePath = NULL;
	_lastTouchPoint = NULL;
}

Gesture::Detected ShoveGesture::isRecognizedImpl(GestureContext *gestureContext)
{
	unsigned int activeTouchPointsCount = gestureContext->getNumActiveTouchPoints();
	_gestureTouchPaths = gestureContext->getActiveTouchPaths();

	if (_shoveGesturePath && !_gestureTouchPaths.contains(_shoveGesturePath))
		_shoveGesturePath = NULL;

	if (!_shoveGesturePath) 
	{ // Find a potential shove gesture touch path
		Detected detected = No; // Whether gesture detected potentially
		int x = _gestureTouchPaths[0]->getLastTouchPoint().x; // Origin used for vector operations
		int y = _gestureTouchPaths[0]->getLastTouchPoint().y;
		int sumDeltaX = 0, sumDeltaY = 0;
		for (unsigned int i = 0; i < activeTouchPointsCount; i++)
		{
			int w = _gestureTouchPaths[i]->getLastTouchPoint().width;
			int h = _gestureTouchPaths[i]->getLastTouchPoint().height;
			sumDeltaX += _gestureTouchPaths[i]->getLastTouchPoint().x - x;
			sumDeltaY += _gestureTouchPaths[i]->getLastTouchPoint().y - y;

			if (w * w + h * h > MINIMUM_SIDE_CONTACT_HYPOTENUSE) 
			{ // Long contact found, probably side of finger / hand
				_shoveGesturePath = _gestureTouchPaths[i];
				_lastTouchPoint = &_shoveGesturePath->getLastTouchPoint();
				detected = Yes;
				break;
			}
		}
		if (detected != Yes && activeTouchPointsCount < 3)
			return Maybe; // 2 Fingers form a straight line, and could be a swipe gesture
		if (detected != Yes) // If there isn't a long contact point, check if contacts form a straight line
		{
			Vec3 edgeDir = Vec3(sumDeltaX, sumDeltaY, 0);
			edgeDir.normalize();
			for (unsigned int i=0; i<activeTouchPointsCount; i++)
			{ 
				TouchPoint & touchPoint = _gestureTouchPaths[i]->getLastTouchPoint();
				Vec3 pos = Vec3(touchPoint.x - x, touchPoint.y - y, 0);
				Vec3 proj = edgeDir * (pos.dot(edgeDir));
				float dist = proj.distance(pos); 
				if (dist > MAXIMUM_LINE_DEVIATION)  // Check if any point is too far from the line
					return Maybe;
			}
			_shoveGesturePath = _gestureTouchPaths[0];
			_lastTouchPoint = &_shoveGesturePath->getLastTouchPoint();
		}
		return Maybe;
	}
	else // A potential touch path is found, check if it moved enough to trigger shove
	{
		Vec3 start = Vec3(_shoveGesturePath->getFirstTouchPoint().x, _shoveGesturePath->getFirstTouchPoint().y, 0);
		_lastTouchPoint = &_shoveGesturePath->getLastTouchPoint();
		Vec3 end = Vec3(_lastTouchPoint->x, _lastTouchPoint->y, 0);
		if ((end - start).magnitude() > 20)
		{
			markingMenu->setEnabled(false);
			lassoMenu->reset();
			return gestureAccepted();
		}
		else
			return Maybe;
	}
}

bool ShoveGesture::processGestureImpl(GestureContext *gestureContext)
{
	unsigned int activeGesturePathCount = gestureContext->getNumActiveTouchPoints();
	if (activeGesturePathCount < 1)
	{
		_shoveGesturePath = NULL;
		return false;
	}
	
	if (!gestureContext->getActiveTouchPaths().contains(_shoveGesturePath)) 
	{ // Lost touch point, find a new one to get movement direction
		_shoveGesturePath = gestureContext->getActiveTouchPaths()[0];
		_lastTouchPoint = &_shoveGesturePath->getLastTouchPoint();
		return true;
	}

	Vec3 origin = Vec3(_lastTouchPoint->x, _lastTouchPoint->y, 0);
	Vec3 movement = Vec3(_shoveGesturePath->getLastTouchPoint().x, _shoveGesturePath->getLastTouchPoint().y, 0);
	if ((movement - origin).magnitude() < 15)
		return true; // If hand hasn't moved much, don't process to save CPU
	
	origin = unProjectToDesktop(origin.x, origin.y).get<2>(); 
	movement = unProjectToDesktop(movement.x, movement.y).get<2>();
	movement -= origin;
	movement.y = 0;
	float movementDist = movement.normalize();
		
	Vec3 nrm = movement.cross(Vec3(0,1,0));
	nrm.y = 0;
	nrm.normalize();
	TouchPoint * touchPoint = &gestureContext->getActiveTouchPaths()[0]->getLastTouchPoint();
	int w2 = touchPoint->width / 2, h2 = touchPoint->height / 2;
	int minX = touchPoint->x - w2, maxX = touchPoint->x + w2;
	int minY = touchPoint->y - h2, maxY = touchPoint->y + h2;
	int temp;
	for (unsigned int i = 1; i < activeGesturePathCount; i++)
	{
		touchPoint = &gestureContext->getActiveTouchPaths()[i]->getLastTouchPoint();
		w2 = touchPoint->width / 2; h2 = touchPoint->height / 2;
		if ((temp = touchPoint->x + w2) > maxX) maxX = temp;
		if ((temp = touchPoint->x - w2) < minX) minX = temp;
		if ((temp = touchPoint->y + h2) > maxY) maxY = temp;
		if ((temp = touchPoint->y - h2) < minY) minY = temp;
	}
	
	float dots [4] = {
		nrm.dot(unProjectToDesktop(minX, minY).get<2>() - origin), 
		nrm.dot(unProjectToDesktop(maxX, maxY).get<2>() - origin),
		nrm.dot(unProjectToDesktop(maxX, minY).get<2>() - origin), 
		nrm.dot(unProjectToDesktop(minX, maxY).get<2>() - origin)};
	float min = dots[0], max = dots[0];
	// Min and max specify the projection of touch points onto the line perpendicular to the shove direction.
	// Any object inside the min and max when projected onto this line should be affected by shove.
	for (unsigned int i = 1; i < _countof(dots); i++)
	{
		if (dots[i] > max) max = dots[i];
		else if (dots[i] < min) min = dots[i];
	}
	
	vector<BumpObject *> objs = scnManager->getBumpObjects();
	unsigned int objsSize = objs.size();
	for (int i = 0; i < objsSize; i++)
	{
		Vec3 dir = objs[i]->getGlobalPosition() - origin;
		dir.y = 0;
		if (objs[i]->isPinned())
			continue;
		if (dir.dot(movement) < 0)
			continue; // Object behind moving edge
		if (dir.dot(nrm) > max || dir.dot(nrm) < min)
			continue; // Outside of shove contact area
		float dist = dir.magnitude() * abs(dir.dot(movement));
		float shoveSpeed = movementDist * 8000.0f / dist;
		// Speed cap so that objects too close to the contact wouldn't move too much.
		// If the cap is too low, then objects maybe trail a fast moving contact.
		shoveSpeed = shoveSpeed > 200 ? 200 : shoveSpeed;
		Vec3 vel = movement * shoveSpeed;
		objs[i]->setLinearVelocity(vel);
	}

	_lastTouchPoint = &_shoveGesturePath->getLastTouchPoint();
	return true;
}