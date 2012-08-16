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

#ifndef _BT_RIGHT_CLICK_GESTURE_
#define _BT_RIGHT_CLICK_GESTURE_

#include "BT_Gesture.h"

class RightClickGesture : public Gesture
{
private:
	static const float MINIMUM_WAIT_TIME;
	static const float MAXIMUM_PATH_LENGTH;

protected:
	virtual Detected isRecognizedImpl(GestureContext *gestureContext);
	virtual bool processGestureImpl(GestureContext *gestureContext);
	virtual void clearGesture();

public:
	RightClickGesture();
};

#endif /* _BT_RIGHT_CLICK_GESTURE_ */