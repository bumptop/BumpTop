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

#ifndef _BT_GESTURE_MANAGER_
#define _BT_GESTURE_MANAGER_

#include "BT_GestureContext.h"
#include "BT_Gesture.h"

class TextOverlay;

class GestureManager
{
	Q_DECLARE_TR_FUNCTIONS(GestureManager);

	GestureContext _gestureContext;
	QList<Gesture *> _gestures;
	QList<Gesture *> _eligibleGestures;
	QList<TextOverlay *> _goProMessages;
	QTextStream *_logFileStream;

	Gesture *_activeGesture;

	void addGesture(Gesture *gesture);
	void renderCursor(TouchPoint *touchPoint);

public:
	GestureManager();
	~GestureManager();

	bool isGestureActive();
	void processGestures();
	void clear();
	GestureContext *getGestureContext() { return &_gestureContext; };
	void onRender();
	void setLogFileStream(QTextStream *stream);
};

#endif /* _BT_GESTURE_MANAGER_ */