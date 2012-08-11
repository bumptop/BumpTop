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

#ifndef BT_WIN32COMM
#define BT_WIN32COMM

// ----------------------------------------------------------------------------

class Win32Comm
{
public:
	enum Win32Message
	{
		ResetDesktopLayout,
		IsInfiniteDesktopModeEnabled,
		ToggleInfiniteDesktopMode,
		CheckForUpdates, 
		SendFeedback,
		// ResetSettings,
		ReloadTheme,
		ReloadSettings,
		CycleMonitors,
		AuthorizeProKey,
		LaunchProxySettings,
		AuthorizationDialogClose,
		DeauthorizeProKey,
		UploadTheme,
		SettingsRequireRestart,
		InstallWebWidget,
		StartCustomizeDialog
	};

private:
	HWND _bumptopWindow;
	static WNDPROC prevWndProc;
	static UINT LaunchProxySettingsValue;
	static UINT AuthorizationDialogCloseValue;
	static UINT DeauthorizeProKeyValue;
	map<Win32Message, unsigned int> _messages;
	wxTimer _windowCheckTimer;
	wxTimer _applyThrobberTimer;

public:
	Win32Comm();
	void load(HWND btHwnd);
	void hookWndProc(HWND settingsAppHwnd);

	// operations
	bool sendMessage(Win32Message, bool value);
	bool sendDataMessage(Win32Message, COPYDATASTRUCT);
	static LRESULT		wndProcHook(HWND ctrl, UINT msg, WPARAM wParam, LPARAM lParam);

	// accessors
	HWND getWindowHandle() const;
	wxTimer& getWindowCheckTimer();
	wxTimer& getApplyThrobberTimer();
};

#endif // BT_WIN32COMM