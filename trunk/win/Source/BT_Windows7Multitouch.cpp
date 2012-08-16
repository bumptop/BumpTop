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

#include "BT_Common.h"
#include "BT_Windows7Multitouch.h"
#include "BT_ManipulationEventSink.h"
#include "BT_SceneManager.h"
#include "BT_TouchPoint.h"
#include "BT_WindowsOS.h"
#include "BT_Util.h"

Windows7Multitouch::Windows7Multitouch() :
	waitingForMatchingMouseDown(false),
	waitingForMatchingMouseUp(false),
	alreadySimulatedMouseUp(false),
	_pIManipulationProcessor(NULL),
	_pManipulationEventSink(NULL),
	_gestureManager(),
	pRegisterTouchWindow(NULL),
	pUnregisterTouchWindow(NULL),
	pCloseTouchInputHandle(NULL),
	pGetTouchInputInfo(NULL),
	pTKGetGestureMetrics(NULL),
	_hMod(NULL),
	_supportsMultiTouch(false),
	_logFile(NULL),
	_logFileStream(NULL)
{
	_hMod = LoadLibrary(_T("User32.dll"));
	if (_hMod)
	{
		pRegisterTouchWindow = (RegisterTouchWindowSignature) GetProcAddress(_hMod, "RegisterTouchWindow");
		pUnregisterTouchWindow = (UnregisterTouchWindowSignature) GetProcAddress(_hMod, "UnregisterTouchWindow");
		pCloseTouchInputHandle = (CloseTouchInputHandleSignature) GetProcAddress(_hMod, "CloseTouchInputHandle");
		pGetTouchInputInfo = (GetTouchInputInfoSignature) GetProcAddress(_hMod, "GetTouchInputInfo");
		pTKGetGestureMetrics = (TKGetGestureMetricsSignature) GetProcAddress(_hMod, "TKGetGestureMetrics");

		if (pRegisterTouchWindow && pUnregisterTouchWindow &&
			pCloseTouchInputHandle && pGetTouchInputInfo)
		{
			// We must call this to start receiving the WM_TOUCH* events
			if (!pRegisterTouchWindow(winOS->GetWindowsHandle(), TWF_WANTPALM))
			{
				consoleWrite(QString("RegisterTouchWindow failed, GetLastError() = %1").arg(GetLastError()));
			}
			else
			{
				initializeLogFile(); // Do this now so we can record any errors
				_supportsMultiTouch = true;

				HRESULT hr = CoInitialize(0);
				hr = CoCreateInstance(
					CLSID_ManipulationProcessor, 
					NULL, 
					CLSCTX_ALL, 
					IID_IManipulationProcessor, 
					(void **)&_pIManipulationProcessor);
				if (hr == S_OK)
					_pManipulationEventSink = new ManipulationEventSink(_pIManipulationProcessor);
				else
					logError(QString("\nFailed to initialize Manipulation Processor, HRESULT = %1").arg(hr));
			}
		}
	}
}

Windows7Multitouch::~Windows7Multitouch()
{
	if (_logFileStream)
	{
		_logFileStream->flush();
		SAFE_DELETE(_logFileStream);
	}
	if (_logFile)
	{
		_logFile->close();
		SAFE_DELETE(_logFile);
	}

	if (pUnregisterTouchWindow)
		pUnregisterTouchWindow(winOS->GetWindowsHandle());
	if (_hMod)
		FreeLibrary(_hMod);
	if (_pManipulationEventSink)
		delete _pManipulationEventSink;
	if (_pIManipulationProcessor)
		_pIManipulationProcessor->Release();
}

// Initialize the touch debugging log file.
// This is created by default in DEBUG builds, and can be turned on by 
// the -touchDebugLog option in release builds.
void Windows7Multitouch::initializeLogFile()
{
#ifdef DEBUG
	if (scnManager->touchDebugLogFilename.isEmpty())
	{
		QDir logDir = winOS->GetDataDirectory();
		scnManager->touchDebugLogFilename = logDir.absoluteFilePath("touchDebugLog.txt");
	}
#endif 

	// If a filename was specified, open a log file for recording the inner
	// workings of the gestures & the gesture manager.
	if (!scnManager->touchDebugLogFilename.isEmpty())
	{
		bool exists = QFileInfo(scnManager->touchDebugLogFilename).exists();
		_logFile = new QFile(scnManager->touchDebugLogFilename);
		_logFile->open(QFile::WriteOnly | QFile::Truncate);
		_logFileStream = new QTextStream(_logFile);

		if (exists) 
			*_logFileStream << "\n\n";
		QDateTime now = QDateTime::currentDateTime();
		*_logFileStream << "Log opened at " << now.toString() << "\n";
		_logFileStream->flush();

		_gestureManager.setLogFileStream(_logFileStream);
	}
}

bool Windows7Multitouch::overridesMouseEvent(UINT uMsg)
{
	if (!_supportsMultiTouch)
		return false;

	// If the event corresponds to the primary touch point, let it be handled
	// by the default mouse handler
	if ((uMsg == WM_LBUTTONDOWN) && waitingForMatchingMouseDown)
	{
		waitingForMatchingMouseDown = false;
		return false;
	}
	// Same for mouse up, unless we already faked a mouse up event
	if ((uMsg == WM_LBUTTONUP) && waitingForMatchingMouseUp)
	{
		waitingForMatchingMouseUp = false;
		return alreadySimulatedMouseUp;
	}

	// If touch is active, all other events are swallowed by the touch handler
	if (areMinimumTouchPointsActive(1))
		return true;

	// Otherwise, the mouse event should be handled by the default handler
	return false;
}

void Windows7Multitouch::onTouchInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!_supportsMultiTouch)
		return;

	UINT numTouchPoints = LOWORD(wParam);
	// This could be heap allocated, like this:
	//     PTOUCHINPUT touchInputs = new TOUCHINPUT[numTouchPoints]
	// but using QVarLengthArray allows for stack allocation as long as numTouchPoints <= 12
	QVarLengthArray<TOUCHINPUT, 12> touchInputs(numTouchPoints);

	if (_logFileStream) *_logFileStream << "TouchInput: ";

	if (pGetTouchInputInfo((HTOUCHINPUT)lParam, numTouchPoints, touchInputs.data(), sizeof(TOUCHINPUT)))
	{
		// Sometimes we don't get all the touch up events (e.g. if the window
		// loses focus). So here, we detect when the primary touch point goes
		// down, and clear everything appropriately.
		// Done in a separate loop because we aren't guaranteed that the primary
		// touch point appears at the beginning of the array
		if (!_gestureManager.getGestureContext()->isEmpty())
		{
			for (int i = 0; i < numTouchPoints; i++) {
				TOUCHINPUT ti = touchInputs[i];
				if ((ti.dwFlags & TOUCHEVENTF_PRIMARY) && (ti.dwFlags & TOUCHEVENTF_DOWN))
				{
					_gestureManager.clear();
					break;
				}
			}
		}

		for (int i = 0; i < numTouchPoints; i++) {
			TOUCHINPUT ti = touchInputs[i];
			bool isPrimary = (bool)(ti.dwFlags & TOUCHEVENTF_PRIMARY);

			// Set or update the touch info from the TOUCHINPUT struct
			// TOUCHINPUT fields are in hundredths of a pixel

			TouchPoint touchPoint;
			touchPoint.x = TOUCH_COORD_TO_PIXEL(ti.x);
			touchPoint.y = TOUCH_COORD_TO_PIXEL(ti.y);

			// Fetch width and height (if valid)
			if (ti.dwMask & TOUCHINPUTMASKF_CONTACTAREA) 
			{
				touchPoint.width = TOUCH_COORD_TO_PIXEL(ti.cxContact);
				touchPoint.height = TOUCH_COORD_TO_PIXEL(ti.cyContact);
			}
			else
			{
				touchPoint.width = 1;
				touchPoint.height = 1;
			}

			if (ti.dwFlags & TOUCHEVENTF_DOWN)
			{
				touchPoint.state = TouchPoint::Down;
				if (isPrimary)
				{
					// We are now waiting for the corresponding mouse down and mouse up events
					waitingForMatchingMouseDown = true;
					waitingForMatchingMouseUp = true;

					alreadySimulatedMouseUp = false;
				}
			} 
			else if (ti.dwFlags & TOUCHEVENTF_MOVE)
			{
				touchPoint.state = TouchPoint::Move;
			}
			else if (ti.dwFlags & TOUCHEVENTF_UP)
			{
				touchPoint.state = TouchPoint::Up;
			}

			// Convert the coordinates to client space
			POINT adjustedCoordinates = {touchPoint.x, touchPoint.y};
			ScreenToClient(winOS->GetWindowsHandle(), &adjustedCoordinates);
			touchPoint.x = adjustedCoordinates.x;
			touchPoint.y = adjustedCoordinates.y;

			// Now actually add the touch point to the gesture
			// FIXME: This could be made cleaner. Ideally we wouldn't be 
			// in to get the gesture context, and it's weird that we have to 
			// pass the timestamp in, rather than just setting it in the TouchPoint.
			if (_logFileStream) *_logFileStream << 
				QString("%1 #%2 %3 at (%4, %5); ")
					.arg(ti.dwTime)
					.arg(ti.dwID)
					.arg(TouchPoint::STATE_NAMES[touchPoint.state])
					.arg(touchPoint.x)
					.arg(touchPoint.y);
			_gestureManager.getGestureContext()->addTouchPoint(touchPoint, (uint)ti.dwID, (ulong)ti.dwTime);

			// If more than one touch point exists, simulate a mouse up event
			// which stops any lassoing. This way gestures can be recognized
			// without performing other operations, like selecting.
			
			//if (!alreadySimulatedMouseUp && !waitingForMatchingMouseDown && !isPrimary && (touchPoint.state == TouchPoint::Down)
			//Removed last condition: Allows to check at any moment if two touch points are down.
			//Sometimes Windows fires mouse events at different times so continuous checking
			//is safer.
			if (!alreadySimulatedMouseUp && !waitingForMatchingMouseDown
				&& !isPrimary)
			{
				winOS->OnMouse(WM_LBUTTONUP, touchPoint.x, touchPoint.y, MK_LBUTTON);
				alreadySimulatedMouseUp = true;
			}

			// Pass the events off to the "manipulation processor"
			// This is the suggested way of handling built-in gestures (pinch zoom, rotate, etc.) in Windows 7
			// These methods should just fill in the appropriate fields of the GestureContext
			if(_pManipulationEventSink != NULL)
			{		
				switch (touchPoint.state)
				{
				case TouchPoint::Down:
					_pIManipulationProcessor->ProcessDown(ti.dwID, (float)touchPoint.x, (float)touchPoint.y);
					break;
				case TouchPoint::Move:
					_pIManipulationProcessor->ProcessMove(ti.dwID, (float)touchPoint.x, (float)touchPoint.y);
					break;
				case TouchPoint::Up:
					_pIManipulationProcessor->ProcessUp(ti.dwID, (float)touchPoint.x, (float)touchPoint.y);
					break;
				}
				_gestureManager.getGestureContext()->setManipulationData(_pManipulationEventSink->getManipulationData());
			}
		}
		if (_logFileStream) *_logFileStream << "\n";
		_gestureManager.processGestures();
	}	
	else
	{
		logError(QString("\nGetTouchInputInfo failed, GetLastError = %1").arg(GetLastError()));
	}
	// We must call this after the message has been processed
	pCloseTouchInputHandle((HTOUCHINPUT)lParam);
}

void Windows7Multitouch::onRender()
{
	if (_supportsMultiTouch)
		_gestureManager.onRender();
}

bool Windows7Multitouch::areMinimumTouchPointsActive(uint numberOfPoints)
{
	if (!_supportsMultiTouch)
		return false;
	return _gestureManager.getGestureContext()->getNumActiveTouchPoints() >= numberOfPoints;
}

bool Windows7Multitouch::isGestureActive()
{
	if (!_supportsMultiTouch)
		return false;
	return _gestureManager.isGestureActive();
}

void Windows7Multitouch::logError( QString message )
{
	if (_logFileStream) *_logFileStream << message;
	consoleWrite(message);
}

bool Windows7Multitouch::registerTouchWindow(PWND pwnd)
{
	if (pRegisterTouchWindow)
		return pRegisterTouchWindow(pwnd, 0);
	return false;
}