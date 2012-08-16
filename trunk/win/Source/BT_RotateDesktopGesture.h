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

#ifndef _BT_ROTATE_DESKTOP_GESTURE_
#define _BT_ROTATE_DESKTOP_GESTURE_

#include "BT_Gesture.h"

class RotateDesktopGesture : public Gesture
{
private:
	static const float MINIMUM_GESTURE_ANGLE; // minimum angle in degrees to be recognized as rotation gesture
	static const float SNAP_MINIMUM_ANGLE; // minimum angle for snapping to the next wall
	static const float TARGET_FINGER_DISTANCE; // angle *= sqrt(actual / ideal distance)

	Vec3 _originalDir, _originalUp; // camera params after facing "forward" wall; rotated around y axis
	Vec3 _startDir, _startUp, _startPos; // used to revert cam params if rotation gesture is not completed
	float _lastAngle; // last calculated rotation angle
	bool _animatingToForward; // whether camera is animating to face "forward" wall at beginning of rotation
	bool _recalculateInitials; // whether to reset initial params after one finger has lifted probably for repositioning
	float _compensate; // subtracted to avoid rotation when gesture was just recognized and camera is animating to face "forward" wall

	int getWallCameraIsFacing();
	pair<Vec3, Vec3> calculateCameraPosition();
	float calculateAngle(GestureContext *gestureContext); // returns adjusted manipulation angle based on finger separation

protected:
	virtual void clearGesture();
	virtual Detected isRecognizedImpl(GestureContext *gestureContext);
	virtual bool processGestureImpl(GestureContext *gestureContext);

public:
	RotateDesktopGesture();
};

#endif /* _BT_ROTATE_DESKTOP_GESTURE_ */