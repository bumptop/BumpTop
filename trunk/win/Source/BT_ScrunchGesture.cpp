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
#include "BT_Util.h"
#include "BT_SceneManager.h"
#include "BT_OverlayComponent.h"
#include "BT_Selection.h"
#include "BT_WindowsOS.h"

#include "BT_ScrunchGesture.h"

const float ScrunchGesture::MAXIMUM_SCRUNCH_RATIO = 0.75f;
const float ScrunchGesture::MINIMUM_UNSCRUNCH_RATIO = 1.50f;
const float ScrunchGesture::CONVEX_HULL_PUSH_OUT_FACTOR = 0.3f;

ScrunchGesture::ScrunchGesture() :
	Gesture("Scrunch", 3, 5, false)
{
	clearGesture();
}

void ScrunchGesture::clearGesture()
{
	_normalScrunch = true;
	_initialTouchPoints.clear();
	_currentNumTouchPoints = 0;
	_maxNumTouchPoints = 0;
	_scrunchRatio = 1.0f;
}

Gesture::Detected ScrunchGesture::isRecognizedImpl(GestureContext *gestureContext)
{
	_gestureTouchPaths = gestureContext->getActiveTouchPaths();

	Vec3List polygonPoints;

	QListIterator<Path*> idIterator(_gestureTouchPaths);
	while (idIterator.hasNext())
	{
		polygonPoints.push_back(idIterator.next()->getLastTouchPoint().getPositionVector());
	}

	float updatedArea = calculateAreaOfPolygon(polygonPoints);

	// This is called whenever more fingers are placed on the screen
	if (gestureContext->getNumActiveTouchPoints() > _maxNumTouchPoints)
	{
		_maxNumTouchPoints = gestureContext->getNumActiveTouchPoints();
		_initialTouchPoints = polygonPoints;
	}

	if (gestureContext->getNumActiveTouchPoints() != _currentNumTouchPoints)
	{
		_baselineArea = updatedArea / _scrunchRatio;
		_currentNumTouchPoints = gestureContext->getNumActiveTouchPoints();
	}

	_scrunchRatio = updatedArea / _baselineArea;

	// The new area must be 75% or less of the old area in order for the gesture to be recognized.
	if (_scrunchRatio <= MAXIMUM_SCRUNCH_RATIO)
	{
		_normalScrunch = true;
		return gestureAccepted();
	}

	if (_scrunchRatio >= MINIMUM_UNSCRUNCH_RATIO)
	{
		_normalScrunch = false;

		// On un-scrunch, if fingers start very close together, the object
		// to un-scrunch might not fit in the initial selection.
		// Rather, lets use the final finger positions when un-scrunching.
		_initialTouchPoints = polygonPoints;
		return gestureAccepted();
	}
	return Maybe;
}

bool ScrunchGesture::processGestureImpl(GestureContext *gestureContext)
{
	if (gestureContext->getNumActiveTouchPoints() < 3 && gestureContext->getNumActiveTouchPoints() > 5)
	{
		return false;
	}
	
	Vec3List fingerBoundBox = getConvexHull(_initialTouchPoints);
	
	// Because the shape made by a user's fingers on the touch panel is not adequate
	// in encapsulating all objects under their hand, we need to add an extra point
	// in between the two farthest points
	
	float largestDistance = 0.0f;
	Vec3List::iterator largestDistanceIndex1;
	Vec3List::iterator largestDistanceIndex2;
	
	// Find the largest distance between two adjacent points in the convex hull
	Vec3List::iterator currentIndex;
	for (currentIndex = fingerBoundBox.begin(); currentIndex != fingerBoundBox.end(); currentIndex++)
	{
		Vec3List::iterator nextIndex = currentIndex + 1;
		
		// Wrap around the vector is we exceed its size
		if (nextIndex == fingerBoundBox.end())
			nextIndex = fingerBoundBox.begin();
		
		float distance = ((*nextIndex) - (*currentIndex)).magnitude();
		
		if (distance > largestDistance)
		{
			largestDistance = distance;
			largestDistanceIndex1 = currentIndex;
			largestDistanceIndex2 = nextIndex;
		}
	}

	// Compute the left direction of the largest vector. We choose to push the
	// point out to the left because getConvexHull() orders the points in such
	// a way that the left of each vector made by adjacent points is the
	// outside of the convex hull
	Vec3 line = (*largestDistanceIndex2) - (*largestDistanceIndex1);
	Vec3 down(0.0f, 0.0f, -1.0f);
	Vec3 left = line.cross(down);
	left.normalize();
	
	// Find the mid point of the largest vector and create a point that is pushed
	// out in the left direction of the vector
	Vec3 midPoint = (line / 2.0f) + (*largestDistanceIndex1);
	Vec3 newPoint = midPoint + (left * largestDistance * CONVEX_HULL_PUSH_OUT_FACTOR);
	
	// Add the point in the convex hull in between the end points of the largest
	// vector
	fingerBoundBox.insert(largestDistanceIndex2, newPoint);

	// All the objects that exist in Bump Top
	vector<BumpObject*> bumpObjects = scnManager->getBumpObjects();
	
	vector<BumpObject*>::iterator objectIt;
	bool containsPile = false;
	// Check if any of the Bump Objects are in the finger selection
	sel->clear();
	for (objectIt = bumpObjects.begin(); objectIt != bumpObjects.end(); objectIt++)
	{
		if (isPointInPolygon((*objectIt)->getGlobalPosition(), fingerBoundBox))
		{
			// Key_BreakPile() and Key_MakePile() use the bump objects in the current selection as their
			// "arguments". This is why we add the objects in the scrunch gesture selection to
			// the global selection.
			if ((*objectIt)->isPinned())
				continue;
			containsPile |= (*objectIt)->isBumpObjectType(BumpObjectType(BumpPile));
			sel->add(*objectIt);
		}
	}
	
	if (_normalScrunch)
	{
		if (sel->getBumpObjects().size() > 1)
		{	
			//printUnique(QString("MT_Gesture"), QString("Scrunch"));
			// Makes pile out of currently selected objects
			Key_MakePile();
		}	
	}
	else if (sel->getBumpObjects().size() > 0)
	{
		//printUnique(QString("MT_Gesture"), QString("Un-Scrunch"));
		// Break currently selected piles
		if (containsPile)
			Key_BreakPile();
		else 
			Key_GridLayoutSelection();
	}

	return false;
}

void ScrunchGesture::onRender()
{
	/*
	Vec3List boundingBox = getConvexHull(_initialTouchPoints);
	
	Vec3 center = calculatePolygonCentroid(boundingBox);

	glPushAttribToken token(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);

	glPushMatrix();
	//glTranslatef(center.x, winOS->GetWindowHeight() - center.y, center.z);
	glColor4f(0.0f, 1.0f, 0.0f, 0.5f);
	glBegin(GL_POLYGON);
	
	for (int n = 0; n < boundingBox.size(); n++)
	{
		glVertex2f(boundingBox[n].x, winOS->GetWindowHeight() - boundingBox[n].y);
	}
		
	glEnd();
	glPopMatrix();
	*/
}