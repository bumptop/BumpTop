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

#ifndef _BT_WINDOWS7_MULTITOUCH_
#define _BT_WINDOWS7_MULTITOUCH_

#include "BT_GestureManager.h"
#include "BT_Windows7User32Override.h"

class ManipulationEventSink;
class TouchPoint;

class Windows7Multitouch
{
private:
	// we need these definitions since we are building against the winxp platform
	// under which these are not defined in the windows sdk.
	RegisterTouchWindowSignature pRegisterTouchWindow;
	UnregisterTouchWindowSignature pUnregisterTouchWindow;
	CloseTouchInputHandleSignature pCloseTouchInputHandle;
	GetTouchInputInfoSignature pGetTouchInputInfo;
	TKGetGestureMetricsSignature pTKGetGestureMetrics;
	HMODULE	_hMod;
	bool _supportsMultiTouch;

	// Log file for touch debugging info -- see -touchDebugLog option.
	QFile *_logFile;
	QTextStream *_logFileStream;

	ManipulationEventSink  *_pManipulationEventSink;
	// NB: This is a pointer to a COM object whose lifetime must be properly managed
	IManipulationProcessor *_pIManipulationProcessor;

	// A single touch point should act like the mouse. Windows will do this by
	// default, but we want more control over it. These vars indicate whether
	// we are waiting for the mouse event corresponding to a touch event.
	bool waitingForMatchingMouseDown;
	bool waitingForMatchingMouseUp;

	bool alreadySimulatedMouseUp;

	// FIXME: This should probably be allocated elsewhere and passed in
	GestureManager _gestureManager;

	void initializeLogFile();
	void logError(QString message);

public:
	Windows7Multitouch();
	~Windows7Multitouch();

	bool overridesMouseEvent(UINT uMsg);
	bool isGestureActive();
	bool areMinimumTouchPointsActive(uint numberOfPoints);
	void onTouchInput(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void onRender();

	bool registerTouchWindow(PWND pwnd);
};
#endif /* _BT_WINDOWS7_MULTITOUCH_ */