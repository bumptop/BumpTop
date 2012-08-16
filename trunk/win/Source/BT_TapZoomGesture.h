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

#ifndef BT_TAP_ZOOM_GESTURE
#define BT_TAP_ZOOM_GESTURE

#include "BT_Gesture.h"

class BumpObject;

class TapZoomGesture : public Gesture
{
private:
	static const long MAXIMUM_DELAY;
	vector<BumpObject*> _objects;

	// Keep processing until camera finishes zooming to prevent mouse event handler from canceling animation
	// in case processGestureImpl happens before MOUSEDOWN and MOUSEUP
	bool _keepProcessing;

protected:
	virtual void clearGesture();
	virtual Detected isRecognizedImpl(GestureContext *gestureContext);
	virtual bool processGestureImpl(GestureContext *gestureContext);

public:
	TapZoomGesture();
};

#endif /* _BT_TAP_ZOOM_GESTURE_ */