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

#ifndef _BT_CAMERA_ZOOM_GESTURE_
#define _BT_CAMERA_ZOOM_GESTURE_

#include "BT_Gesture.h"

class CameraZoomGesture : public Gesture
{
private:
	static const float MINIMUM_PATH_LENGTH;
	static const float MINIMUM_LINGERING_ZOOM_SPEED;
	static const float MAXIMUM_TOLERANCE_ANGLE;

	// whether to reset "_original" variables because a finger has lifted and repositioned
	bool _recalculateInitials; 
	
	float _originalDistanceBetweenPoints;
	Vec3 _originalCamEye, _camDirection;
	
	void calculateInitials(TouchPoint & firstFingerEndPoint, TouchPoint & secondFingerEndPoint); 

protected:
	virtual Detected isRecognizedImpl(GestureContext *gestureContext);
	virtual bool processGestureImpl(GestureContext *gestureContext);
	virtual void clearGesture();

public:
	CameraZoomGesture();
};

#endif /* _BT_CAMERA_ZOOM_GESTURE_ */