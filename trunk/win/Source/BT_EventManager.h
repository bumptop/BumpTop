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

#ifndef _EVENT_MANAGER_
#define _EVENT_MANAGER_

// -----------------------------------------------------------------------------

#include "BT_Singleton.h"
#include "BT_KeyCombo.h"
#include "BT_MousePointer.h"
#include "BT_Stopwatch.h"
#include "BT_Windows7User32Override.h"

class BumpObject;

// -----------------------------------------------------------------------------

enum CustomMessages
{
	SmartBoardMessage	= (WM_USER + 10),
	SystemTrayMessage	= (WM_USER + 11),
	FileSystemMessage	= (WM_USER + 12),
};

// -----------------------------------------------------------------------------

class EventManager
{
public:
	enum PowerStatus
	{
		Unplugged = 0,
		PluggedIn = 1,
		UnknownPower = 255
	};

private:
	Q_DECLARE_TR_FUNCTIONS(EventManager)

	// Windows 7 Logo Certification requires all applications to 
	// gracefully respond to immediate shutdown messages. If an
	// operation that can not be interrupted is being processed, the 
	// ShutdownBlockReasonXXX() functions can be used to display a message
	// explaining the reason why it can't shutdown.
	ShutdownBlockReasonCreateSignature pShutdownBlockReasonCreate;
	ShutdownBlockReasonDestroySignature pShutdownBlockReasonDestroy;
	ShutdownBlockReasonQuerySignature pShutdownBlockReasonQuery;
	HMODULE _hLibModule;
	bool _supportsShutdownBlock;

	bool exitFlag;
	bool asyncExitFlag;
	bool isInsideApp;
	bool isSuspended;
	bool ignoreMessages;
	PowerStatus oldACPowerStatus;
	HWND hWnd;
	Stopwatch finishedBindTimer;
	bool renderOnFinishedBinding;

	// is a remote desktop connection active?
	bool _remoteConnectionActive;

	ConditionalStopwatch idleTimer;
	Stopwatch saveTimeoutTimer;
	int saveMSTimeout;
	map<BumpObject *, Vec3> potentialCreepingObjects;

	WNDPROC _qtGuiWndProc;

	Stopwatch exitAfterTimeoutTimer;
	int exitAfterMSTimeout;
	QString renderReason; //A string explaining what caused isRenderRequired() to be true
	// Private Actions
	void checkMouseCursorPos();
	void checkTextureLoaded();
	void checkSaveSceneRequired();
	QString formatMessageDescription(MSG msg);

	// Generic Events
	void onTimer(uint timerID);
	void onUpdate();
	void onRender();

	// Keyboard Manager
	void onKeyDown(uint keyVal);
	void onKeyUp(uint keyVal);

	// Mouse Manager
	void onMouseDown(UINT msg, WPARAM wParam, LPARAM lParam);
	void onMouseUp(UINT msg, WPARAM wParam, LPARAM lParam);
	void onMouseMove(WPARAM wParam, LPARAM lParam);
	void onMouseEnterWindow();
	void onMouseExitWindow();
	void onDeviceAdded(DEV_BROADCAST_HDR * data);
	void onDeviceRemoved(DEV_BROADCAST_HDR * data);

	// System specific
	void onPowerSuspend();
	void onPowerResume();
	void onPowerStatusChange();

	// Private Getters
	bool isRenderRequired();

	// Singleton
	friend class Singleton<EventManager>;
	EventManager();

public:

	~EventManager();

	// Actions
	void mainLoop();
	LRESULT messageProc(HWND Hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void postMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void forceLoopUpdate();
	void interruptIdleTimer();
	LRESULT forwardEvent(QWidget * widget, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// Remote connections
	void onRDPConnect();
	void onRDPDisconnect();
	bool isRDPConnected();

	// Getters
	bool isExiting();	
	PowerStatus getACPowerStatus();
	QString RenderReason() const { return renderReason; }

	// flick events
	void onFlick(int dir, const Vec3& pt);

	// Setters
	void setExitFlag(bool exitNow);
	void setAsyncExitFlag(bool asyncExit);
	void forceExitAfterTimeout(unsigned int millis);

};

QString GetMessageName(UINT message);

// -----------------------------------------------------------------------------

#define evtManager Singleton<EventManager>::getInstance()

// -----------------------------------------------------------------------------

#else
	class EventManager;
#endif 