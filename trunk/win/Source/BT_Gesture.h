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

#ifndef _BT_GESTURE_
#define _BT_GESTURE_

#include "BT_BumpObject.h"
#include "BT_TouchPoint.h"
#include "BT_Path.h"

class GestureContext;

class Gesture
{
	Q_DECLARE_TR_FUNCTIONS(Gesture);

public:
	enum Detected { No, Maybe, Yes };

private:
	bool _isProcessing;
	QTextStream *_logFileStream;

	inline void outputLogMessage(bool accepted, QString& explanation);

protected:
	Gesture(const char* name, unsigned int minAllowedTouchPoints, unsigned int maxAllowedTouchPoints, bool allowedInSlideShowMode);
	QString _name;

	unsigned int _minAllowedTouchPoints; // If there are less than _minAllowedTouchPoints active touch points, isRecognized returns Maybe
	unsigned int _maxAllowedTouchPoints; // If there are more than _maxAllowedTouchPoints active touch points, isRecognized return No
	bool _allowedInSlideShowMode; // If not _allowedInSlideShowMode and is in slide show mode, isRecognized returns No
	
	static const float MINIMUM_SWIPE_PATH_LENGTH;
	static const float MAXIMUM_SWIPE_ANGLE_THRESHOLD;

	// Stores the touch Ids of all active touchPoints at the moment of gesture recognition.
	// Useful for accessing touch point path data of touch points that are no longer active.
	QList<Path*> _gestureTouchPaths;
	
	// Function that subclasses override to perform clears
	virtual void clearGesture();

	// Subclasses must implement these two methods

	// Return true when the gesture is detected
	virtual Detected isRecognizedImpl(GestureContext *gestureContext) = 0;
	
	// This will only be called after isRecognized has returned true.
	// It will continue to be called for every delta in the GestureContext.
	// Returns false when the gesture has completed, otherwise returns true.
	virtual bool processGestureImpl(GestureContext *gestureContext) = 0;

	// These should always be used when a gesture is accepted or rejected
	// They will handle logging, and any other housekeeping required
	Detected Gesture::gestureAccepted();
	Detected Gesture::gestureRejected(const char *str) { return gestureRejected(QString(str)); }
	Detected Gesture::gestureRejected(QString& reason);

public:
	Detected isRecognized(GestureContext *gestureContext);
	bool processGesture(GestureContext *gestureContext);

	// Used to clear any objects or variables initialized during the
	// processing of the gesture.
	void clear();

	// Returns the name of the gesture.
	QString getName();

	bool isSameDirection(Vec3 &firstVector, Vec3 &secondVector, float threshold);	
	bool isOppositeDirection(Vec3 &firstVector, Vec3 &secondVector, float threshold);
	Detected isTwoFingerSwipe(GestureContext* gestureContext, Vec3& direction = Vec3(0.0f));

	virtual void onRender();

	bool isProcessing() { return _isProcessing; }

	// This should only be called from the GestureManager
	void setLogFileStream(QTextStream *stream) { _logFileStream = stream; }
};
#endif /* _BT_GESTURE_ */
