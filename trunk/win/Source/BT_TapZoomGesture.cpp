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
#include "BT_SceneManager.h"
#include "BT_Camera.h"
#include "BT_GestureContext.h"
#include "BT_Pile.h"
#include "BT_Selection.h"
#include "BT_TapZoomGesture.h"
#include "BT_Util.h"
#include "BT_WebActor.h"

// The maximum amount of time (in milliseconds) that is allowed between the
// first finger going down, and the seconds. Also applies when they come up.
// TODO: Maybe this is supposed to be the total time between the fingers going
// down, and coming up?
const long TapZoomGesture::MAXIMUM_DELAY = 500;

TapZoomGesture::TapZoomGesture() :
	Gesture("Tap Zoom", 0, 2, false) // Min of 0 because it's recognized when 2 fingers are lifted
{
	clearGesture();
}

void TapZoomGesture::clearGesture()
{
	_keepProcessing = false;
}

Gesture::Detected TapZoomGesture::isRecognizedImpl(GestureContext *gestureContext)
{
	// TODO: Currently the time that the fingers spend on the screen has no
	// bearing on the gesture, as long as the fingers go down within 
	// MAXIMUM_DELAY milliseconds of each other. Same for coming up.
	// In between, they can sit in contact with the screen for an unlimited
	// amount of time. Is this right?
	if (_gestureTouchPaths.size() == 2)
	{
		if (gestureContext->getNumActiveTouchPoints() == 1)
		{
			long firstFingerUp = _gestureTouchPaths[0]->getLastTouchPoint().relativeTimeStamp;
			long secondFingerUp = _gestureTouchPaths[1]->getLastTouchPoint().relativeTimeStamp;
			long delta = abs(firstFingerUp - secondFingerUp);
			if (delta > MAXIMUM_DELAY)
				return gestureRejected(QString("Time between two fingers coming up (%1) greater than maximum (%2)").arg(delta).arg(MAXIMUM_DELAY));
		}
		else if (gestureContext->getNumActiveTouchPoints() == 0)
		{
			return gestureAccepted();
		}
	}
	else
	{
		if (gestureContext->getNumActiveTouchPoints() == 2)
		{
			_objects = sel->getBumpObjects();
			_gestureTouchPaths = gestureContext->getActiveTouchPaths();			
			long firstFingerDown = _gestureTouchPaths[0]->getFirstTouchPoint().relativeTimeStamp;
			long secondFingerDown = _gestureTouchPaths[1]->getFirstTouchPoint().relativeTimeStamp;
			long delta = abs(firstFingerDown - secondFingerDown);
			if (delta > MAXIMUM_DELAY)
				return gestureRejected(QString("Time between two fingers going down (%1) greater than maximum (%2)").arg(delta).arg(MAXIMUM_DELAY));
		}
	}
	return Maybe;	
}

bool TapZoomGesture::processGestureImpl(GestureContext *gestureContext)
{
	bool zoomIn = false;
	if (_keepProcessing)
	{
		// Keep processing until camera finishes zooming to prevent mouse event handler from canceling animation
		// in case processGestureImpl happens before MOUSEDOWN and MOUSEUP
		if (cam->isAnimating())
			return true;
		else
		{
			_keepProcessing = false;
			return false;
		}
	}
	if (_objects.size() > 1)
	{
		zoomIn = true;
		// Check if we have the same selection as before
		if (_objects.size() == cam->getZoomedObjects().size())
		{
			zoomIn = false;
			for (int i = 0; i < _objects.size(); i++)
			{
				if (!cam->getZoomedObjects().contains(_objects[i]))
				{
					zoomIn = true;
					break;
				}
			}
		}
	}
	else
	{
		_objects.clear();
		BumpObject* obj1 = _gestureTouchPaths[0]->getLastTouchPoint().getPickedObject();
		BumpObject* obj2 = _gestureTouchPaths[1]->getLastTouchPoint().getPickedObject();
		
		ObjectType griddedPile(BumpPile, SoftPile | HardPile, Grid);

		if (obj1 && obj1->isParentType(griddedPile))
			obj1 = obj1->getParent();
		if (obj2 && obj2->isParentType(griddedPile))
			obj2 = obj2->getParent();
		
		bool sameObj = obj1 == obj2;
		if (sameObj && obj1 || obj1 && !obj2)
			_objects.push_back(obj1);
		else if (obj2 && !obj1)
			_objects.push_back(obj2);
		
		if (_objects.size() == 1)
		{
			BumpObject* obj = _objects.front();
			
			if (obj->isObjectType(griddedPile) ||
				obj->isParentType(griddedPile))
			{
				if ((!cam->getZoomedObjects().isEmpty() && cam->getZoomedObjects().front() != obj) || cam->getZoomedObjects().isEmpty())
				{
					zoomIn = true;
				}
			}
			else if (obj->isObjectType(ObjectType(BumpActor, FileSystem, Image | PhotoFrame)))
			{
				sel->clear();
				sel->add(obj);
				Key_EnableSlideShow();
				// Keep processing until camera finishes zooming to prevent mouse event handler from canceling animation
				// in case processGestureImpl happens before MOUSEDOWN and MOUSEUP
				_keepProcessing = true;
				return true;
			}
			else if (obj->isObjectType(ObjectType(BumpActor, Webpage)))
			{
				sel->clear();
				sel->add(obj);
				((WebActor*)obj)->onLaunch();
				_keepProcessing = true;
				return true;
			}
		}
	}

	cam->getZoomedObjects().clear();

	if (zoomIn)
	{
		cam->zoomToIncludeObjects(_objects);
		for (int i = 0; i < _objects.size(); i++)
		{
			cam->getZoomedObjects().append(_objects[i]);
		}
	}
	else
		cam->loadCameraFromPreset(GLOBAL(settings).cameraPreset);

	// Keep processing until camera finishes zooming to prevent mouse event handler from canceling animation
	// in case processGestureImpl happens before MOUSEDOWN and MOUSEUP
	_keepProcessing = true;
	return true;
}