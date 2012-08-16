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

#ifndef _BT_SCRUNCH_GESTURE_
#define _BT_SCRUNCH_GESTURE_

#include "BT_Gesture.h"

class ScrunchGesture : public Gesture
{
private:
	static const float MAXIMUM_SCRUNCH_RATIO;
	static const float MINIMUM_UNSCRUNCH_RATIO;
	static const float CONVEX_HULL_PUSH_OUT_FACTOR;
	
	Vec3List _initialTouchPoints;
	uint _currentNumTouchPoints;
	uint _maxNumTouchPoints;
	float _baselineArea;
	float _scrunchRatio;
	bool _normalScrunch;

protected:
	virtual void clearGesture();
	virtual Detected isRecognizedImpl(GestureContext *gestureContext);
	virtual bool processGestureImpl(GestureContext *gestureContext);
	
public:
	ScrunchGesture();
	void onRender();
};

#endif /* _BT_SCRUNCH_GESTURE_ */