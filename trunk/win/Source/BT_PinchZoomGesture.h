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

#ifndef _BT_PINCH_ZOOM_GESTURE_
#define _BT_PINCH_ZOOM_GESTURE_

#include "BT_Gesture.h"

class PinchZoomGesture : public Gesture
{
private:
	static const float MINIMUM_PATH_LENGTH;
	static const float MAXIMUM_ANGLE_THRESHOLD;

	QVector<BumpObject *> _selectedObjects;
	float _originalDistanceBetweenPoints;
	float _cumulativeScaleFactor; // used if one finger has lifted and repositioned
	float _lastScaleFactor; // used to calculate _cumulativeScaleFactor
	bool _recalculateInitials; // true if one finger has lifted and repositioned
	NxPlane _objectPlane;
	Path* _pathOfTranslationPoint;

protected:
	virtual Detected isRecognizedImpl(GestureContext *gestureContext);
	virtual bool processGestureImpl(GestureContext *gestureContext);
	virtual void clearGesture();

public:
	PinchZoomGesture();
	float getDistanceInObjectPlane(TouchPoint &firstPoint, TouchPoint &secondPoint, NxPlane& plane);
};

#endif /* _BT_PINCH_ZOOM_GESTURE_ */