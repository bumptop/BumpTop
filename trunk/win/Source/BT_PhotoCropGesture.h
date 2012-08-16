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

#ifndef _BT_PHOTO_CROP_GESTURE_
#define _BT_PHOTO_CROP_GESTURE_

#include "BT_Gesture.h"
#include "Qt/qcolor.h"

class PhotoCropGesture : public Gesture
{
private:
	static const QColor CROP_COLOR;
	static const float CROP_LINE_WIDTH;
	static const float CROP_LINE_PATTERN_SCALE;
	static const int MINIMUM_CROP_LINE_LENGTH = 50;
	
	Path* _stationaryPath;
	Path* _cropPath;

	bool isCropPathValid();
	Vec3 windowToActorPercent(Vec3& windowPoint);

protected:
	virtual void clearGesture();
	virtual Detected isRecognizedImpl(GestureContext *gestureContext);
	virtual bool processGestureImpl(GestureContext *gestureContext);

public:
	PhotoCropGesture();

	virtual void onRender();
};

#endif /* _BT_PHOTO_CROP_GESTURE_ */
