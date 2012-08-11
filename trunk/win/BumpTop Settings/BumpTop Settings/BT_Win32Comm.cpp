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
#include "BT_Win32Comm.h"
#include "BT_SettingsApp.h"


WNDPROC Win32Comm::prevWndProc = NULL;
UINT Win32Comm::LaunchProxySettingsValue = 0;
UINT Win32Comm::AuthorizationDialogCloseValue = 0;
UINT Win32Comm::DeauthorizeProKeyValue = 0;


// helper macro
#define GetCtrlHelper(id, ctrlType) \
	GetWXControl(id, ctrlType, Settings->getDialog())

Win32Comm::Win32Comm()
: _bumptopWindow(0)
{}

void Win32Comm::load(HWND btHwnd)
{
	_bumptopWindow = (btHwnd);

#define QUOTE(message) #message
#define RegMsg(message) \
	_messages.insert(make_pair(message, ::RegisterWindowMessage(wxT(QUOTE(message)))))

	RegMsg(ResetDesktopLayout);
	RegMsg(IsInfiniteDesktopModeEnabled);
	RegMsg(CheckForUpdates);
	RegMsg(SendFeedback);
	RegMsg(ReloadTheme);
	RegMsg(ReloadSettings);
	RegMsg(CycleMonitors);
	RegMsg(AuthorizeProKey);
	RegMsg(LaunchProxySettings);
	RegMsg(DeauthorizeProKey);
	RegMsg(AuthorizationDialogClose);
	RegMsg(UploadTheme);
	RegMsg(SettingsRequireRestart);
	RegMsg(InstallWebWidget);
	RegMsg(StartCustomizeDialog);

	LaunchProxySettingsValue = _messages[LaunchProxySettings];
	AuthorizationDialogCloseValue = _messages[AuthorizationDialogClose];
	DeauthorizeProKeyValue = _messages[DeauthorizeProKey];

}

bool Win32Comm::sendMessage(Win32Message message, bool value)
{
	LRESULT result = 0;
	switch (message)
	{
	case IsInfiniteDesktopModeEnabled:
	case StartCustomizeDialog:		
		// Synchronous message is sent
		result = ::SendMessage(_bumptopWindow, _messages[message], (value ? 1 : 0), 0);
		break;
	case SettingsRequireRestart:
	default:
		// Asynchronous message is sent
		result = ::PostMessage(_bumptopWindow, _messages[message], (value ? 1 : 0), 0);
		break;
	}
	return (result > 0);
}

// Send wm_copydata message http://msdn.microsoft.com/en-us/library/ms649011(VS.85).aspx
bool Win32Comm::sendDataMessage(Win32Message message, COPYDATASTRUCT cpd)
{
	cpd.dwData = _messages[message];
	LRESULT result;
	if(Settings->getDialog())
		result = ::SendMessage(_bumptopWindow, WM_COPYDATA, (WPARAM)((HWND)Settings->getDialog()->GetHandle()), (LPARAM)&cpd);
	else
		result = ::SendMessage(_bumptopWindow, WM_COPYDATA, NULL, (LPARAM)&cpd);
	return (result > 0);
}

LRESULT Win32Comm::wndProcHook( HWND ctrl, UINT msg, WPARAM wParam, LPARAM lParam )
{
	if (msg == LaunchProxySettingsValue)
	{
		GetCtrlHelper(ID_NOTEBOOK, wxNotebook)->SetSelection(4);
	}
	else if (msg == AuthorizationDialogCloseValue || msg == DeauthorizeProKeyValue)
	{
		Json::Value root;
		Settings->loadSettingsFile(root);
		Settings->syncSettings(true, root);
	}
	
	return prevWndProc(ctrl, msg, wParam, lParam);
}

HWND Win32Comm::getWindowHandle() const
{
	return _bumptopWindow;
}

wxTimer& Win32Comm::getWindowCheckTimer() 
{
	return _windowCheckTimer;
}

wxTimer& Win32Comm::getApplyThrobberTimer()
{
	return _applyThrobberTimer;
}

void Win32Comm::hookWndProc( HWND settingsAppHwnd )
{
	prevWndProc = (WNDPROC) SetWindowLong(settingsAppHwnd, GWL_WNDPROC, (LONG) wndProcHook);
}