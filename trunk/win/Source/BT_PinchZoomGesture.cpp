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
#include "BT_FileSystemActor.h"
#include "BT_OverlayComponent.h"
#include "BT_Pile.h"
#include "BT_PinchZoomGesture.h"
#include "BT_SceneManager.h"
#include "BT_Selection.h"
#include "BT_StickyNoteActor.h"
#include "BT_Util.h"
#include "BT_WindowsOS.h"

const float PinchZoomGesture::MINIMUM_PATH_LENGTH = TO_PIXELS(208.0f);		// 20 pixels
const float PinchZoomGesture::MAXIMUM_ANGLE_THRESHOLD = 30.0f;

PinchZoomGesture::PinchZoomGesture() : 
	Gesture("Pinch Zoom", 2, 2, false)
{
	clearGesture();
}

void PinchZoomGesture::clearGesture()
{
	_selectedObjects.clear();
	_pathOfTranslationPoint = NULL;
	_recalculateInitials = false;
}

Gesture::Detected PinchZoomGesture::isRecognizedImpl(GestureContext *gestureContext)
{	
	_gestureTouchPaths = gestureContext->getActiveTouchPaths();

	DWORD time = max(_gestureTouchPaths[0]->getFirstTouchPoint().relativeTimeStamp, _gestureTouchPaths[1]->getFirstTouchPoint().relativeTimeStamp);
	TouchPoint& firstFingerPoint = _gestureTouchPaths[0]->getTouchPointAt(time);
	TouchPoint& secondFingerPoint = _gestureTouchPaths[1]->getTouchPointAt(time);

	BumpObject * firstFingerObject = firstFingerPoint.getPickedObject();
	BumpObject * secondFingerObject = secondFingerPoint.getPickedObject();

	_cumulativeScaleFactor = 1.0f;
	float initialDistanceBetweenFingers = firstFingerPoint.calculateDisplacementFrom(secondFingerPoint).magnitude();

	TouchPoint& firstFingerEndPoint = _gestureTouchPaths[0]->getLastTouchPoint();
	TouchPoint& secondFingerEndPoint = _gestureTouchPaths[1]->getLastTouchPoint();

	float finalDistanceBetweenFingers = firstFingerEndPoint.calculateDisplacementFrom(secondFingerEndPoint).magnitude();

	if (abs(initialDistanceBetweenFingers - finalDistanceBetweenFingers) < MINIMUM_PATH_LENGTH)
		return Maybe;
	
	BumpObject *targetObject; // Using its plane to calculate world space distance for scaling

	if (firstFingerObject && firstFingerObject == secondFingerObject)
	{ // If both fingers are on the same object, user wants to zoom that object, so add it to selection
		targetObject = firstFingerObject;
		_pathOfTranslationPoint = _gestureTouchPaths[0];
	}	
	else if (firstFingerObject && !secondFingerObject)
	{ // If only first finger is on an object, zoom based on that object
		targetObject = firstFingerObject;
		_pathOfTranslationPoint = _gestureTouchPaths[0];
	}
	else if (secondFingerObject && !firstFingerObject)
	{ // If only second finger is on an object, zoom based on that object
		targetObject = secondFingerObject;
		_pathOfTranslationPoint = _gestureTouchPaths[1];
	}
	else
	{
		return gestureRejected("Fingers are on diff't objs, or no objs (probably camera zoom)");
	}

	if (!sel->isInSelection(targetObject))
		sel->add(targetObject);
	sel->setPickedActor(targetObject); // Set the picked object for selection to allow dragging

	//Work with the bottom most object in the pile if this is a pile
	if(targetObject->getObjectType() == ObjectType(BumpPile))
		targetObject = ((Pile *) targetObject)->getPileItems().back();

	//Calculate the normal and center point of the object
	Vec3 normal = targetObject->getGlobalOrientation() * Vec3(0.0f, 0.0f, 1.0f);
	Vec3 centerPoint = targetObject->getGlobalPosition() + (targetObject->getGlobalOrientation() * Vec3(0.0f, 0.0f, targetObject->getDims().z));

	// Create the plane representing the surface of the object and using that,
	// Find the distance between the two touch points on that plane, for scaling in world space
	_objectPlane = NxPlane(centerPoint, normal);
	_originalDistanceBetweenPoints = getDistanceInObjectPlane(firstFingerEndPoint, secondFingerEndPoint, _objectPlane);

	// Take a snapshot of the currently selected objects
	vector<BumpObject *> selected = sel->getBumpObjects();

	ObjectType griddedPile(BumpPile, SoftPile | HardPile, Grid);

	vector<BumpObject *>::iterator it;
	for(it = selected.begin(); it != selected.end(); it++)
	{
		BumpObject* obj = *it;
		if (obj->isObjectType(griddedPile) || obj->isParentType(griddedPile))
			continue;
		(*it)->updateReferenceDims();
		_selectedObjects.push_back(*it);
	}
	if (sel->getPickedActor())
		sel->getPickedActor()->onDragBegin();
	return gestureAccepted();
}

bool PinchZoomGesture::processGestureImpl(GestureContext *gestureContext)
{
	QList<Path *> activePaths = gestureContext->getActiveTouchPaths();
	if(gestureContext->getNumActiveTouchPoints() != 2)
	{
		// Update all PostIt layouts since grow and shrink relies on animation call back to update layout.
		QVectorIterator<BumpObject*> it(_selectedObjects);
		while(it.hasNext())
		{
			StickyNoteActor * actor = dynamic_cast<StickyNoteActor *>(it.next());
			if (actor)
				actor->syncStickyNoteWithFileContents();
		}

		// The gesture is active as long as the one of the two initial fingers is still down
		if (activePaths.count() == 1)
		{
			if (activePaths.contains(_gestureTouchPaths[0]) || activePaths.contains(_gestureTouchPaths[1]))
			{
				_recalculateInitials = true;
				return true;
			}
		}
		// Stop translation of objects by dragging
		if (sel->getPickedActor())
			sel->getPickedActor()->onDragEnd();
		unpick();
		
		return false;
	}
	
	float newDistance = getDistanceInObjectPlane(activePaths[0]->getLastTouchPoint(), activePaths[1]->getLastTouchPoint(), _objectPlane);
	
	if (_recalculateInitials)
	{
		_recalculateInitials = false;
		_cumulativeScaleFactor *= _lastScaleFactor;
		_originalDistanceBetweenPoints = newDistance;
	}
		
	float scaleFactor = newDistance / _originalDistanceBetweenPoints;
	_lastScaleFactor = scaleFactor;
	scaleFactor *= _cumulativeScaleFactor;

	QVectorIterator<BumpObject*> it(_selectedObjects);
	while(it.hasNext())
	{
		BumpObject *obj = it.next();
		obj->scaleFromReferenceDims(scaleFactor);
		//don't reselect the objects within a pile
		if(!sel->isInSelection(obj))
			sel->add(obj);
	}
	
	// Add translation of object by simulating mouse events
	// OnMouseMove sets the primary touch x,y which are used to calculate the force to move objects
	winOS->OnMouseMove(_pathOfTranslationPoint->getLastTouchPoint().x, _pathOfTranslationPoint->getLastTouchPoint().y, MK_LBUTTON);
	return true;
}

float PinchZoomGesture::getDistanceInObjectPlane(TouchPoint &firstPoint, TouchPoint &secondPoint, NxPlane& plane)
{
	// Unproject both points onto the objects plane
	Vec3 firstWorldPoint = firstPoint.unProject(plane);
	Vec3 secondWorldPoint = secondPoint.unProject(plane);

	//Calculate the distance between the world points
	return (secondWorldPoint - firstWorldPoint).magnitude();
}