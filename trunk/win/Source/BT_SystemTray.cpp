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
#include "BT_SystemTray.h"
#include "BT_WindowsOS.h"
#include "BT_DialogManager.h"
#include "BT_Util.h"
#include "BT_SceneManager.h"
#include "BT_StatsManager.h"
#include "BT_OverlayComponent.h"
#include "BT_EventManager.h"
#include "BT_RenderManager.h"
#include "BT_FileSystemManager.h"
#include "BT_Authorization.h"
#include "BumpTop.h"
#include "BT_Training.h"

SysTray::BalloonNotificationLaunchUrl SysTray::_launchUrl = SysTray::InviteFriendsUrl;
bool SysTray::_handleWndProcMessages = false;

SysTray::SysTray()
{
	memset(&iconID, NULL, sizeof(NOTIFYICONDATA));
}

SysTray::~SysTray()
{
	hideTrayIcon();
}

void SysTray::hideTrayIcon()
{
	// Remove the Icon on Exit
	Shell_NotifyIcon(NIM_DELETE, &iconID);
}

void SysTray::CreateLink(QString workingPath, QString target, QString args, QString desc, QString shortcutFileName, QString loc)
{
	HRESULT hres = NULL;
	IShellLink * psl = NULL;
	IPersistFile * ppf = NULL;
	QFileInfo linkName(loc, shortcutFileName);
	QString pathToTarget(workingPath);
	pathToTarget.append(target);

	// Get a pointer to the IShellLink interface.
	hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **) &psl);

	if (SUCCEEDED(hres))
	{
	// Set the path to the shortcut target
		psl->SetPath((LPCWSTR) pathToTarget.utf16());
		psl->SetDescription((LPCWSTR) desc.utf16());
		psl->SetWorkingDirectory((LPCWSTR) workingPath.utf16());
		psl->SetArguments((LPCWSTR) args.utf16());

		// Query IShellLink for the IPersistFile interface for
		// saving the shortcut in persistent storage.
		hres = psl->QueryInterface(IID_IPersistFile, (void **) &ppf);

		if (SUCCEEDED(hres))
		{
			// Save the link by calling IPersistFile::Save.
			hres = ppf->Save((LPCWSTR) native(linkName).utf16(), TRUE);
			ppf->Release();
		}

		psl->Release();
	}
}

LRESULT SysTray::MsgProc(HWND Hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	POINT pos;
	HMENU hMenu;
	HMENU hSubMenu;

	switch (uMsg)
	{
		// This handled the popup initialization (if necessary)
		case WM_INITMENU:
			if (SysTray::_handleWndProcMessages)
			{
				QString tmp;
				MENUITEMINFO mii = {0};
					mii.cbSize = sizeof(MENUITEMINFO);
					mii.fMask = MIIM_STRING;
				// set the menu text
				tmp = QT_TR_NOOP("Sho&w BumpTop \t(Start+D)");
				mii.dwTypeData = (LPWSTR) tmp.utf16();
				SetMenuItemInfo((HMENU) wParam, ID_POPUP_REVEALWINDOWSDESKTOP, FALSE, &mii);
				tmp = QT_TR_NOOP("Send &Feedback");
				mii.dwTypeData = (LPWSTR) tmp.utf16();
				SetMenuItemInfo((HMENU) wParam, ID_POPUP_SENDFEEDBACK, FALSE, &mii);
				tmp = QT_TR_NOOP("&Help");
				mii.dwTypeData = (LPWSTR) tmp.utf16();
				SetMenuItemInfo((HMENU) wParam, 3, TRUE, &mii); // By position
				tmp = QT_TR_NOOP("Online &Help");
				mii.dwTypeData = (LPWSTR) tmp.utf16();
				SetMenuItemInfo((HMENU) wParam, ID_POPUP_BUMPTOPBETAHQ, FALSE, &mii);				
				tmp = QT_TR_NOOP("Run &Tutorial");
				mii.dwTypeData = (LPWSTR) tmp.utf16();
				SetMenuItemInfo((HMENU) wParam, ID_POPUP_RUN_TUTORIAL, FALSE, &mii);
				tmp = QT_TR_NOOP("&Go Pro");
				mii.dwTypeData = (LPWSTR) tmp.utf16();
				SetMenuItemInfo((HMENU) wParam, ID_POPUP_GOPRO, FALSE, &mii);
				tmp = QT_TR_NOOP("Check for &updates");
				mii.dwTypeData = (LPWSTR) tmp.utf16();
				SetMenuItemInfo((HMENU) wParam, ID_POPUP_UPDATE, FALSE, &mii);
				tmp = QT_TR_NOOP("&Settings... \t(Ctrl+,)");
				mii.dwTypeData = (LPWSTR) tmp.utf16();
				SetMenuItemInfo((HMENU) wParam, ID_POPUP_EDITSETTINGS, FALSE, &mii);
				tmp = QT_TR_NOOP("E&xit BumpTop \t(Alt+F4)");
				mii.dwTypeData = (LPWSTR) tmp.utf16();
				SetMenuItemInfo((HMENU) wParam, ID_POPUP_EXITBUMPTOP, FALSE, &mii);

				// update the checked state of the 'Launch BumpTop on Startup' menu

				UINT isLaunchingBumpTopOnStartupCheckmark = sysTray->isLaunchingBumpTopOnStartup() ? MF_CHECKED : MF_UNCHECKED;

				CheckMenuItem((HMENU) wParam, ID_POPUP_LAUNCHBUMPTOPONSTARTUP, MF_BYCOMMAND | isLaunchingBumpTopOnStartupCheckmark);

				// update the checked state of the infinite desktop mode
				UINT isInInfiniteDesktopMode = scnManager->isInInfiniteDesktopMode ? MF_CHECKED : MF_UNCHECKED;
				CheckMenuItem((HMENU) wParam, ID_POPUP_INFINITEBUMPTOPTOGGLE, MF_BYCOMMAND | isInInfiniteDesktopMode);

				// update the checked state of the anti-aliasing (check user override, then auto generated state)
				if (!scnManager->isMultiSamplingSupported)
				{
					ModifyMenu((HMENU) wParam, ID_POPUP_ANTIALIASING, MF_BYCOMMAND | MF_GRAYED | MF_STRING, NULL, (LPCWSTR)QT_TR_NOOP("Enable Anti-Aliasing (Unsupported on this video card)").utf16());
				}
				else
				{
					UINT isAntiAliasingDisabled = (winOS->getRegistryDwordValue("UserDisableAntiAliasing") || winOS->getRegistryDwordValue("DisableAntiAliasing")) ? MF_UNCHECKED : MF_CHECKED;
					CheckMenuItem((HMENU) wParam, ID_POPUP_ANTIALIASING, MF_BYCOMMAND | isAntiAliasingDisabled);
				}
				
				// check if user is already pro. If so remove the "Go Pro" button from system tray
				if (GLOBAL(settings).freeOrProLevel == AL_PRO)
					RemoveMenu((HMENU) wParam, ID_POPUP_GOPRO, MF_BYCOMMAND);
#if DISABLE_PHONING
				RemoveMenu((HMENU) wParam, ID_POPUP_SENDFEEDBACK, MF_BYCOMMAND);
				RemoveMenu((HMENU) wParam, ID_POPUP_INVITEFRIENDS, MF_BYCOMMAND);
#endif

// #ifndef BTDEBUG
// 				// don't show the edit-setting menu when in release mode
// 				RemoveMenu((HMENU) wParam, ID_POPUP_EDITSETTINGS, MF_BYCOMMAND);
// #endif
			}
			break;

		// This handles the different options you select on the menu
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case ID_POPUP_EXITBUMPTOP:
					// Exit Bumptop
					ExitBumptop();

					return 1;

				case ID_POPUP_EDITSETTINGS:
					// Open up the Settings file
					Key_ShowSettingsDialog();
					return 1;

				case ID_POPUP_BUMPTOPBETAHQ:
					// launch the bumptop beta hq page
					{
						QString helpSiteUrl = QString("http://www.bumptop.com/help?rev=%1&lang=%2&ed=%3")
							.arg(winOS->GetBuildNumber())
							.arg(winOS->GetLocaleLanguage())
							.arg(winOS->BumpTopEditionName(winOS->GetBumpTopEdition()));
						// append build information when calling
						fsManager->launchFile(helpSiteUrl);
						printUnique("ForwardUrl", QT_TR_NOOP("Forwarding you to BumpTop Help pages"));
					}
					return 1;

				case ID_POPUP_INFINITEDESKTOPTOGGLE:
					Key_ToggleInfiniteDesktopMode();
					break;

				case ID_POPUP_RESETBUMPTOPLAYOUT:
					// Clears the desktop and re-pulls its positions
					Key_ResetBumpTopLayout();
					return 1;

				case ID_POPUP_SENDFEEDBACK:
					Key_SendFeedback();
					break;

				case ID_POPUP_UPDATE:
					winOS->checkForUpdate();
					break;

				case ID_POPUP_REVEALWINDOWSDESKTOP:
					winOS->ToggleShowWindowsDesktop();
					break;

				case ID_POPUP_INFINITEBUMPTOPTOGGLE:
					Key_ToggleInfiniteDesktopMode();
					break;

				case ID_POPUP_ANTIALIASING:
					{
						bool isAntiAliasingDisabled = (winOS->getRegistryDwordValue("UserDisableAntiAliasing") || winOS->getRegistryDwordValue("DisableAntiAliasing"));
						if (isAntiAliasingDisabled)
						{
							// set the user override to be enabled
							winOS->setRegistryDwordValue("UserDisableAntiAliasing", 0);
							rndrManager->setMultisamplingEnabled(true);							
						}
						else
						{
							// set the user override to be disabled
							winOS->setRegistryDwordValue("UserDisableAntiAliasing", 1);
							rndrManager->setMultisamplingEnabled(false);
						}
					}
					break;

				case ID_POPUP_TOGGLEWALLS:
					// Toggle the Walls
					Key_ToggleInfiniteDesktopMode();

					rndrManager->invalidateRenderer();
					break;

				case ID_POPUP_LAUNCHBUMPTOPONSTARTUP:
					{
						// update the checked state of the 'Launch BumpTop on Startup' menu
						QFileInfo commonStartupPos = make_file(QDir(winOS->GetSystemPath(CSIDL_COMMON_STARTUP)), "BumpTop.lnk");
						QFileInfo userStartupPos = make_file(QDir(winOS->GetSystemPath(CSIDL_STARTUP)), "BumpTop.lnk");
						if (exists(commonStartupPos) || 
							exists(userStartupPos))
						{
							// remove the shortcuts from their locations
							QFile::remove(commonStartupPos.absoluteFilePath());
							QFile::remove(userStartupPos.absoluteFilePath());
							
							// if the previous failed on vista due to uac issues, then try and remove it again using the shell
							if (exists(commonStartupPos))
							{
								fsManager->deleteFileByName(native(commonStartupPos));
							}
						}
						else
						{
							// re-add the shortcut to the user's location
							CreateLink(native(parent(winOS->getRegistryStringValue("ApplicationPath"))), 
								"BumpTop.exe", "", "BumpTop", "BumpTop.lnk", winOS->GetSystemPath(CSIDL_STARTUP));
						}
					}
					break;

				case ID_POPUP_INVITEFRIENDS:
					launchInviteFriendsURL();
					break;

				case ID_POPUP_GOPRO:
					launchBumpTopProPage("sysTray");
					break;

				case ID_POPUP_RUN_TUTORIAL:
					launchTutorial();
					break;
			}
			break;

		case SystemTrayMessage:
			switch (lParam)
			{
				// Left click on icon
				case WM_LBUTTONUP:
					switch(winOS->GetWindowState())
					{
					case Windowed:
						sysTray->RestoreFromTray(Hwnd);
						break;
					case FullWindow:
						sysTray->RestoreFromTray(Hwnd);
						break;
					case WorkArea:
						if (IsIconic(winOS->GetWindowsHandle()))
							ShowWindow(winOS->GetWindowsHandle(), SW_MAXIMIZE);
						else
							ShowWindow(winOS->GetWindowsHandle(), SW_SHOW);
						winOS->BringWindowToForeground();
						break;
					case FullScreen:
						// ?? what's the right course of action here?
						break;
					default:
						assert(false);
						break;
					}

					return 1;
					break;	

				// RIght click on icon
				case WM_RBUTTONUP:
					SysTray::_handleWndProcMessages = true;

					// Load up the Menus
					hMenu = LoadMenu(sysTray->GetInstanceHandle(), MAKEINTRESOURCE(IDR_POPUP_MENU));
					hSubMenu = GetSubMenu(hMenu, 0);

					// Get the point of the mouse click
					GetCursorPos(&pos);

					// Show the menu
					winOS->BringWindowToForeground();
					TrackPopupMenu(hSubMenu, 0, pos.x, pos.y, 0, Hwnd, NULL);

					// BUGFIX: See "PRB: Menus for Notification Icons Don't Work Correctly"
					PostMessage(Hwnd, WM_NULL, 0, 0);
					DestroyMenu(hMenu);	

					SysTray::_handleWndProcMessages = false;
					return 1;
				case NIN_BALLOONUSERCLICK:
					switch (_launchUrl)
					{
					case SysTray::PurchaseUrl:
						launchBumpTopProPage("sysTrayBalloon");
						break;
					case SysTray::InviteFriendsUrl:
						launchInviteFriendsURL();
						break;
					}
					break;
			}
	}
	return 0;
}

void SysTray::Init(HWND hWnd, HINSTANCE hInst, bool showIcon)
{
	hWndHandle = hWnd;
	hInstance = hInst;

	// Set up the Icon data
	iconID.cbSize = sizeof(NOTIFYICONDATA);
	iconID.hWnd = hWndHandle;
	iconID.uID = 0;
	iconID.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	iconID.uCallbackMessage = SystemTrayMessage;
	iconID.hIcon = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 16, 16, 0);

	QString sysTrayLabel = "BumpTop";
	if(scnManager->isInSandboxMode) sysTrayLabel.append("SandBox");
#ifdef DEBUG
	sysTrayLabel.append("DEBUG");
#endif
	lstrcpy(iconID.szTip, (LPCWSTR) sysTrayLabel.utf16());	

	// Show the Icon
	Shell_NotifyIcon(NIM_ADD, &iconID);
}

void SysTray::MinimizeToTray(HWND hWnd)
{
	// Doing a HIDE kills the window form the desktop
	if (hWnd)
	{
		ShowWindow((HWND) hWnd, SW_HIDE);
	}
}

void SysTray::RestoreFromTray(HWND hWnd)
{
	if (hWnd)
	{
		statsManager->getStats().bt.window.activatedFromSystemTray++;
		ShowWindow((HWND) hWnd, SW_SHOW);
		// bring window to foreground (for the windowed mode)
		if (winOS->GetWindowState() == Windowed ||
			winOS->GetWindowState() == FullWindow)
			winOS->BringWindowToForeground();
	}
}

HINSTANCE SysTray::GetInstanceHandle()
{
	return hInstance;
}

HWND SysTray::GetWindowHandle()
{
	return hWndHandle;
}

bool SysTray::isLaunchingBumpTopOnStartup()
{
	QFileInfo commonStartupPos(QDir(winOS->GetSystemPath(CSIDL_COMMON_STARTUP)), "BumpTop.lnk");
	QFileInfo userStartupPos(QDir(winOS->GetSystemPath(CSIDL_STARTUP)), "BumpTop.lnk");
	return exists(commonStartupPos) || exists(userStartupPos);
}

void SysTray::postNotification( QString title, QString message, int timeoutInMillis)
{
	/*
	bool isVista = winOS->IsWindowsVersion(WindowsVista);
	SHSTOCKICONINFO icon = {0};
		icon.cbSize = sizeof(icon);
	if (isVista)
		SHGetStockIconInfo(SIID_USERS, SHGSI_ICON, &icon);
	*/

	NOTIFYICONDATA data = {0};
		data.cbSize = sizeof(data);
		data.hWnd = winOS->GetWindowsHandle();
		data.uFlags = NIF_INFO;
		StringCbCopy(data.szInfo, 256 * sizeof(TCHAR), (LPTSTR) message.utf16());
		StringCbCopy(data.szInfoTitle, 64 * sizeof(TCHAR), (LPTSTR) title.utf16());
		data.uTimeout = timeoutInMillis;	// only used for xp systems
		/*
		if (isVista)
		{
			data.hIcon = icon.hIcon;
			data.dwInfoFlags = NIIF_USER | NIIF_LARGE_ICON | NIIF_NOSOUND;
		}
		else
		*/
			data.dwInfoFlags = NIIF_INFO | NIIF_NOSOUND;
	Shell_NotifyIcon(NIM_MODIFY, &data);
}

void SysTray::launchInviteFriendsURL()
{
}

void SysTray::setLaunchUrl( BalloonNotificationLaunchUrl url )
{
	_launchUrl = url;
}

void SysTray::launchTutorial()
{
	if (!GLOBAL(isInTrainingMode))
	{
		// this will prevent saving of the scene file
		GLOBAL(skipSavingSceneFile) = true;
		GLOBAL(isInTrainingMode) = true;

		// Let's clear out the desktop for the trade show
		ClearBumpTop();

		TrainingIntroRunner* training = new TrainingIntroRunner();
		scnManager->setTrainingIntroRunner(training);
		training->start();
	}
	else
	{
		printUnique("SysTray::launchTutorial", QT_TR_NOOP("Already running tutorial"))
	}
}