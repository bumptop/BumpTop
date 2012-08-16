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

#pragma once

#include "BT_Gesture.h"

class ShoveGesture : public Gesture
{
private:
	// Maximum distance between a finger and line of best fit between fingers for recognition
	static const float MAXIMUM_LINE_DEVIATION; 
	// Minimum squared hypotenuse of touch point to be recognized as side of finger / hand
	static const int MINIMUM_SIDE_CONTACT_HYPOTENUSE; 
	bool _horizontal; // whether shove is horizontal or vertical (opposite of contact edge)
	Path * _shoveGesturePath; // The active gesture path used to calculated movement direction
	TouchPoint * _lastTouchPoint; // Touch point in _shoveGesturePath used to calculate movement direction
	
protected:
	virtual void clearGesture();
	virtual Detected isRecognizedImpl(GestureContext *gestureContext);
	virtual bool processGestureImpl(GestureContext *gestureContext);

public:
	ShoveGesture();
};