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

#pragma once

#ifndef _BT_SYSTEM_TRAY_
#define _BT_SYSTEM_TRAY_

// -----------------------------------------------------------------------------

#include "BT_Singleton.h"

// -----------------------------------------------------------------------------

class SysTray
{
public:
	enum BalloonNotificationLaunchUrl
	{
		NoURL,
		PurchaseUrl,
		InviteFriendsUrl
	};

private:
	Q_DECLARE_TR_FUNCTIONS(SysTray)

	NOTIFYICONDATA iconID;
	HWND hWndHandle;
	HINSTANCE hInstance;
	static BalloonNotificationLaunchUrl _launchUrl;
	static bool _handleWndProcMessages;

private:
	static void CreateLink(QString workingPath, QString target, QString args, QString desc, QString shortcutFileName, QString loc);
	static void launchInviteFriendsURL();
	static void launchTutorial();

public:
	SysTray();
	~SysTray();

	void Init(HWND hWnd, HINSTANCE hInst, bool showIcon);
	void MinimizeToTray(HWND hWnd);
	void RestoreFromTray(HWND hWnd);
	void postNotification(QString title, QString message, int timeoutInMillis);
	void setLaunchUrl(BalloonNotificationLaunchUrl url);
	void hideTrayIcon();

	// Getters
	HINSTANCE GetInstanceHandle();
	HWND GetWindowHandle();

	static LRESULT MsgProc(HWND Hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	bool isLaunchingBumpTopOnStartup();

};

// -----------------------------------------------------------------------------

#define sysTray Singleton<SysTray>::getInstance()

// -----------------------------------------------------------------------------

#else
	class SysTray;
#endif