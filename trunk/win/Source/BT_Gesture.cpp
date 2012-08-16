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
#include "BT_Gesture.h"
#include "BT_GestureContext.h"
#include "BT_Util.h"
#include "BT_WindowsOS.h"

const float Gesture::MINIMUM_SWIPE_PATH_LENGTH = TO_PIXELS(156.0f);	// 15 pixels
const float Gesture::MAXIMUM_SWIPE_ANGLE_THRESHOLD = 30.0f;

Gesture::Gesture(const char* name, unsigned int minAllowedTouchPoints, unsigned int maxAllowedTouchPoints, bool allowedInSlideShowMode) :
	_name(name),
	_logFileStream(NULL),
	_minAllowedTouchPoints(minAllowedTouchPoints),
	_maxAllowedTouchPoints(maxAllowedTouchPoints),
	_allowedInSlideShowMode(allowedInSlideShowMode),
	_gestureTouchPaths(),
	_isProcessing(false)
{}

QString Gesture::getName()
{
	return _name;
}

// Subclasses can override this method to clear any of their own structures
void Gesture::clearGesture()
{}

void Gesture::clear()
{
	_isProcessing = false;
	_gestureTouchPaths.clear();
	clearGesture();
}

// Returns true if the two vectors are parallel and point in the same direction,
// given a certain error threshold in degrees.
bool Gesture::isSameDirection(Vec3 &firstVector, Vec3 &secondVector, float threshold)
{
	float angle = getAngleBetween(firstVector, secondVector);
	return (angle < threshold);
}

// Returns true if the two vectors are parallel and point in opposite directions,
// given a certain error threshold in degrees.
bool Gesture::isOppositeDirection(Vec3 &firstVector, Vec3 &secondVector, float threshold)
{
	return isSameDirection(firstVector, -secondVector, threshold);
}

// The gesture can render to the screen while it is active.
// The touch point blobs are drawn by the GestureManager.
void Gesture::onRender()
{}

// Simple wrapper around isRecognizedImpl, which is subclass responsibility
// This method only exists to make sure _isProcessing is set properly.
Gesture::Detected Gesture::isRecognized( GestureContext *gestureContext )
{
	Gesture::Detected result = Maybe;
	if (!_allowedInSlideShowMode && isSlideshowModeActive())
		result = No;
	else if (gestureContext->getNumActiveTouchPoints() < _minAllowedTouchPoints)
		result = Maybe;
	else if (gestureContext->getNumActiveTouchPoints() > _maxAllowedTouchPoints)
		result = No;
	else
		result = isRecognizedImpl(gestureContext);
	_isProcessing = (result == Yes);
	return result;
}

// Simple wrapper around processGestureImpl, which is subclass responsibility
// This method only exists to make sure _isProcessing is set properly.
bool Gesture::processGesture(GestureContext *gestureContext)
{
	_isProcessing = processGestureImpl(gestureContext);
	return _isProcessing;
}

Gesture::Detected Gesture::isTwoFingerSwipe(GestureContext* gestureContext, Vec3& direction)
{
	if (gestureContext->getNumActiveTouchPoints() == 2)
	{
		QList<Path*> touchPaths = gestureContext->getActiveTouchPaths();
		
		// Make sure the touch path is long enough for us to do some useful recognition
		if (touchPaths[0]->pathLength > MINIMUM_SWIPE_PATH_LENGTH && 
			touchPaths[1]->pathLength > MINIMUM_SWIPE_PATH_LENGTH)
		{
			Vec3 firstVector = touchPaths[0]->getTotalDisplacementVector();
			Vec3 secondVector = touchPaths[1]->getTotalDisplacementVector();
			
			// If the lines are not parallel within error of 30 degrees, we do not recognize the gesture
			if (isSameDirection(firstVector, secondVector, MAXIMUM_SWIPE_ANGLE_THRESHOLD))
			{
				direction = firstVector;
				direction.normalize();
				return Yes;
			}
			return No;
		}
	}
	return Maybe;
}

inline void Gesture::outputLogMessage(bool accepted, QString& explanation)
{
	// Sample output: 
	// NO:  CameraPanGesture - Fingers are moving apart
	// YES: PinchZoomGesture - Gesture detected
	*_logFileStream << (accepted ? "YES: " : "NO:  ") << _name << " - " << explanation << "\n";
}

Gesture::Detected Gesture::gestureAccepted()
{
	// If we are logging, and a yes or no decision is made, log it

	if (_logFileStream) outputLogMessage(true, QString("Gesture accepted"));
	return Gesture::Yes;
}

Gesture::Detected Gesture::gestureRejected(QString& reason)
{
	if (_logFileStream) outputLogMessage(false, reason);
	return Gesture::No;
}
