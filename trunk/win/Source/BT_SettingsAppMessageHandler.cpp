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
#include "BT_Camera.h"
#include "BT_Authorization.h"
#include "BT_CustomActor.h"
#include "BT_DialogManager.h"
#include "BT_Logger.h"
#include "BT_OverlayComponent.h"
#include "BT_RenderManager.h"
#include "BT_SceneManager.h"
#include "BT_SettingsAppMessageHandler.h"
#include "BT_StatsManager.h"
#include "BT_TextManager.h"
#include "BT_ThemeManager.h"
#include "BT_Util.h"
#include "BT_WindowsOS.h"
#include "BumpTop.h"
#include "BT_FileSystemManager.h"
#include "BT_WebActor.h"
#include "BT_CustomizeWizard.h"

SettingsAppMessageHandler::SettingsAppMessageHandler()
{
#define QUOTE(message) #message
#define RegMsg(message)	message = ::RegisterWindowMessage(QUOTE(message))

	ResetDesktopLayout = ::RegisterWindowMessage(L"ResetDesktopLayout");
	IsInfiniteDesktopModeEnabled = ::RegisterWindowMessage(L"IsInfiniteDesktopModeEnabled");
	ToggleInfiniteDesktopMode = ::RegisterWindowMessage(L"ToggleInfiniteDesktopMode");
	CheckForUpdates = ::RegisterWindowMessage(L"CheckForUpdates");
	SendFeedback = ::RegisterWindowMessage(L"SendFeedback");
	ReloadTheme = ::RegisterWindowMessage(L"ReloadTheme");
	ReloadSettings = ::RegisterWindowMessage(L"ReloadSettings");
	CycleMonitors = ::RegisterWindowMessage(L"CycleMonitors");
	AuthorizeProKey = ::RegisterWindowMessage(L"AuthorizeProKey"); 
	DeauthorizeProKey = ::RegisterWindowMessage(L"DeauthorizeProKey"); 
	LaunchProxySettings = ::RegisterWindowMessage(L"LaunchProxySettings");
	AuthorizationDialogClose = ::RegisterWindowMessage(L"AuthorizationDialogClose");
	UploadTheme = ::RegisterWindowMessage(L"UploadTheme");
	SettingsRequireRestart = ::RegisterWindowMessage(L"SettingsRequireRestart");
	StartCustomizeDialog = ::RegisterWindowMessage(L"StartCustomizeDialog");
	InstallWebWidget = ::RegisterWindowMessage(L"InstallWebWidget");
}

bool SettingsAppMessageHandler::handleMessage( UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& resultOut )
{
	if (msg == ResetDesktopLayout)
	{
		LOG("_settingsAppHandler.handleMessage_ResetDesktopLayout");
		Key_ResetBumpTopLayout();
		resultOut = 0;
		return true;
	}
	else if (msg == IsInfiniteDesktopModeEnabled)
	{
		LOG("_settingsAppHandler.handleMessage_IsInfiniteDesktopModeEnabled");
		resultOut = (GLOBAL(isInInfiniteDesktopMode) ? 1 : 0);
		return true;
	}
	else if (msg == ToggleInfiniteDesktopMode)
	{
		LOG("_settingsAppHandler.handleMessage_ToggleInfiniteDesktopMode");
		if ((wParam > 0) != GLOBAL(isInInfiniteDesktopMode))
			Key_ToggleInfiniteDesktopMode();
		resultOut = 0;
		return true;
	}
	else if (msg == CheckForUpdates)
	{
		LOG("_settingsAppHandler.handleMessage_CheckForUpdates");
		::MessageBox(NULL, L"Check For Updates", (LPCTSTR) QT_TR_NOOP("Check For Updates").utf16(), MB_OK);
		resultOut = 0;
		return true;
	}
	else if (msg == SendFeedback)
	{
		LOG("_settingsAppHandler.handleMessage_SendFeedback");
		Key_SendFeedback();
		resultOut = 0;
		return true;
	}
	else if (msg == ReloadTheme)
	{
		LOG("_settingsAppHandler.handleMessage_ReloadTheme");
		themeManager->reloadDefaultTheme(true);
		rndrManager->invalidateRenderer();
		printTimedUnique("SettingsAppMessageHandler::handleMessage", 6, QT_TR_NOOP("Reloading BumpTop Theme - this may take a few seconds!"));
		resultOut = 0;
		_updatedThemes = true;
		
		statsManager->getStats().bt.theme.timesChanged++;
		return true;
	}
	else if (msg == ReloadSettings)
	{
		LOG("_settingsAppHandler.handleMessage_ReloadSettings");
		QDir dataPath = winOS->GetDataDirectory();
		QFileInfo settingsPath = make_file(dataPath, "settings.json");
		if (exists(settingsPath)) 
		{
			GLOBAL(settings).load(settingsPath.absoluteFilePath());
			rndrManager->invalidateRenderer();
			textManager->invalidate(false);
		}
		resultOut = 0;

		// only show the update message if we haven't already updated themes
		if (!_updatedThemes)
			printTimedUnique("SettingsAppMessageHandler::handleMessage", 2, QT_TR_NOOP("Reloading BumpTop settings"));

		cam->loadCameraFromPreset(GLOBAL(settings).cameraPreset);
		return true;
	}
	else if (msg == CycleMonitors)
	{
		LOG("_settingsAppHandler.handleMessage_CycleMonitors");
		winOS->MoveToNextMonitor();
		resultOut = 0;
		return true;
	}
	else if (msg == AuthorizeProKey)
	{
		LOG("_settingsAppHandler.handleMessage_AuthorizeProKey");
		dlgManager->clearState();
		if (dlgManager->promptDialog(DialogChooseVersion))
		{
			dlgManager->clearState();
			dlgManager->setHasParent(false);
			dlgManager->promptDialog(DialogThankYou);	

			HWND btSettings = FindWindowEx(NULL, NULL, NULL, L"BumpTop Settings");
			::MessageBox(btSettings, (LPCTSTR) QT_TR_NOOP("BumpTop will now restart to enable all Pro features").utf16(), (LPCTSTR) QT_TR_NOOP("Restarting BumpTop").utf16(), MB_OK|MB_ICONWARNING);

			::PostMessage(btSettings, WM_ENDSESSION, 0, 0);

			winOS->ExitBumpTop();
			winOS->RelaunchBumpTop();
		}
	}
	else if (msg == DeauthorizeProKey)
	{
		HWND btSettings = FindWindowEx(NULL, NULL, NULL, L"BumpTop Settings");
		dlgManager->clearState();
		if (dlgManager->promptDialog(DialogDeauthorizeConfirm)) 
		{
			if (deauthorize())
			{
				GLOBAL(settings).freeOrProLevel = AL_FREE;
				GLOBAL(settings).proAuthCode.clear();
				GLOBAL(settings).authCode.clear();
				GLOBAL(settings).proInviteCode.clear();
				GLOBAL(settings).inviteCode.clear();
				winOS->SaveSettingsFile();

				HWND btSettings = FindWindowEx(NULL, NULL, NULL, L"BumpTop Settings");
				LRESULT result = 0;
				if (btSettings != NULL)
				{
					BringWindowToTop(btSettings);
					result = ::PostMessage(btSettings, DeauthorizeProKey, 0, 0);
				}
			}
			else
			{
				if (ftManager->hasInternetConnection())
#ifdef DISABLE_PHONING
					::MessageBox(winOS->GetWindowsHandle(), (LPCTSTR) QT_TR_NOOP("Deauthorization Failed.").utf16(), (LPCTSTR) QT_TR_NOOP("Deauthorization Failed").utf16(), MB_OK|MB_ICONERROR);
#else
					::MessageBox(winOS->GetWindowsHandle(), (LPCTSTR) QT_TR_NOOP("Deauthorization Failed.  Please contact feedback@bumptop.com!").utf16(), (LPCTSTR) QT_TR_NOOP("Deauthorization Failed").utf16(), MB_OK|MB_ICONERROR);
#endif
				else
					::MessageBox(winOS->GetWindowsHandle(), (LPCTSTR) QT_TR_NOOP("Unable to connect to BumpTop server, please check your\ninternet connection before trying again.").utf16(), (LPCTSTR) QT_TR_NOOP("Deauthorization Failed").utf16(), MB_OK|MB_ICONERROR);
			}
		}
	}
	else if (msg == WM_COPYDATA) 
	{
		PCOPYDATASTRUCT pMyCDS = (PCOPYDATASTRUCT)lParam;
		if (pMyCDS->dwData == UploadTheme)
		{
			QString themeFileName = QString::fromUtf16((const ushort *)pMyCDS->lpData, pMyCDS->cbData/sizeof(wchar_t));
			QString themeFileNameExt = themeFileName + ".bumptheme";
			QString themesDir = winOS->GetUserThemesDirectory(false).absolutePath();
			StrList bumpThemes = fsManager->getDirectoryContents(themesDir, themeFileNameExt);
	
			// Check to make sure that it found the file
			if (bumpThemes.size() == 1)
			{
				QString bumptThemePath(bumpThemes.at(0));
				QString screenShotPath = make_file(themesDir, themeFileName + ".jpg").absoluteFilePath();
				
				texMgr->takeScreenshot(screenShotPath);
				
				// Used to identify parameters that are files
				QSet<QString> fileParamKeys;
				fileParamKeys.insert("file");
				fileParamKeys.insert("image");
				
				// All parameters, text and files
				QHash<QString, QString> params;
				params.insert("file", bumptThemePath);
				params.insert("image", screenShotPath);
				params.insert("submission[title]", "testing title");
				params.insert("submission[notes]", "testing notes");
				params.insert("debug_mode", "true");

	/*			Below is the block of code going to be used to do the HTTP post to customize.org
				We currently don't have the logic in place to do this http post yet.			
	*/
				//Customize website is currently not uploading .bumptheme files correctly
				//This will be uncommented when this issue is resolved
//   				QString destination = QString("http://customize.org/bumps/new");
//   				ftManager->addPostTransfer(
//   					FileTransfer(FileTransfer::Upload, 0)
//   					.setUrl(destination)
//   					.setParams(params)
//   					.setFileParamKeys(fileParamKeys)
//   					//.setLogin("bumptop")
//   					//.setPassword("")
//   					.setTimeout(999)
//   					.setTemporaryString(), true
//   					);

				// Copy the users theme to their working directory
				fsManager->copyFileByName(bumptThemePath, scnManager->getWorkingDirectory().absolutePath(), themeFileNameExt, false, true, true);
				fsManager->copyFileByName(screenShotPath, scnManager->getWorkingDirectory().absolutePath(), themeFileName + ".jpg", false, true, true);
				
				// Delete the original
				fsManager->deleteFileByName(bumptThemePath, true, true);
				fsManager->deleteFileByName(screenShotPath, true, true);

				printTimedUnique("UploadTheme", 5, QT_TR_NOOP("Theme '%1' and screenshot saved to desktop").arg(themeFileName));

				return true;
			}

		}
		else if(pMyCDS->dwData == InstallWebWidget)
		{
			QString htmlData = QString::fromUtf16((const ushort *)pMyCDS->lpData, pMyCDS->cbData/sizeof(wchar_t));
			WebActor * widgetActor = new WebActor();
			widgetActor->loadHTML(htmlData);
		}
	}
	else if (msg == SettingsRequireRestart)
	{
		winOS->ExitBumpTop();
		winOS->RelaunchBumpTop();
	}
	else if (msg == StartCustomizeDialog)
	{
		Key_ThemeDialog();
	}
	return false;
}

bool SettingsAppMessageHandler::showProxySettings()
{
	// we're not searching by classname because that's generated by wxWidgets,
	// and might change in the future
	HWND btSettings = FindWindowEx(NULL, NULL, NULL, L"BumpTop Settings");
	LRESULT result = 0;
	if (btSettings != NULL)
	{
		BringWindowToTop(btSettings);
		 result = ::PostMessage(btSettings, LaunchProxySettings, 0, 0);
	}
	return (result > 0);
}

bool SettingsAppMessageHandler::updateAuthorizationKey()
{
	// we're not searching by classname because that's generated by wxWidgets,
	// and might change in the future
	HWND btSettings = FindWindowEx(NULL, NULL, NULL, L"BumpTop Settings");
	LRESULT result = 0;
	if (btSettings != NULL)
	{
		BringWindowToTop(btSettings);
		result = ::PostMessage(btSettings, AuthorizationDialogClose, 0, 0);
	}
	return (result > 0);
}


