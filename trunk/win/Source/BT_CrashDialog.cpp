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
#include "BT_CrashDialog.h"
#include "BT_WindowsOS.h"
#include "BT_QtUtil.h"
#include "BT_Util.h"
#include "BT_FileSystemManager.h"
#include "BT_ThemeManager.h"
#include "BT_SceneManager.h"


/*
* CrashDialog implementation
*/
CrashDialog::CrashDialog()
{
	resetToDefault();
}

CrashDialog::~CrashDialog()
{}

bool CrashDialog::onInit(HWND hwnd)
{
	// reset the radio states
	syncCheckboxButtonState(hwnd);
	QString prompt = WinOSStr->getString("BumpTopError");

	// set the window strings
	SetWindowText(hwnd, (LPCTSTR) QT_TR_NOOP("Send Feedback to BumpTop").utf16());
	SetDlgItemText(hwnd, IDOK, (LPCTSTR) QT_TR_NOOP("Restart").utf16());
	SetDlgItemText(hwnd, IDNO, (LPCTSTR) QT_TR_NOOP("Close BumpTop").utf16());
	SetDlgItemText(hwnd, IDC_RESET_BUMPTOP_SETTINGS, (LPCTSTR) QT_TR_NOOP("Reset BumpTop Settings").utf16());
	SetDlgItemText(hwnd, IDC_SEND_FEEDBACK, (LPCTSTR) QT_TR_NOOP("Send Feedback").utf16());
	SetDlgItemText(hwnd, IDC_EMAIL_LABEL, (LPCTSTR) QT_TR_NOOP("Your Email Address (We will email you once this issue is resolved):").utf16());
	SetDlgItemText(hwnd, IDC_CRASHBOX_PROMPT, (LPCTSTR) prompt.utf16());

	SetDlgItemText(hwnd, IDC_CRASHBOX_DLG_EDIT, (LPCTSTR) QT_TR_NOOP("").utf16());

	// caption
	SetWindowText(hwnd, (LPCTSTR) QT_TR_NOOP("Send Feedback to BumpTop").utf16());

	return true;
}

bool CrashDialog::onCommand(HWND hwnd, WPARAM wParam, LPARAM lParam )
{
	// Process Buttons
	switch (LOWORD(wParam))
	{
		// update the type
	case IDNO:
		winOS->ExitBumpTop();
		break;
	case IDOK:
		break;
	case IDC_RESET_BUMPTOP_SETTINGS:
		_resetBumpTopSettings = (BST_CHECKED == SendMessage(GetDlgItem(hwnd, IDC_RESET_BUMPTOP_SETTINGS), BM_GETCHECK, 0, 0));
		return false;
	case IDC_SEND_FEEDBACK:
		_sendFeedback = (BST_CHECKED == SendMessage(GetDlgItem(hwnd, IDC_SEND_FEEDBACK), BM_GETCHECK, 0, 0));
		return false;
	default:
		return false;
	}
	
	QString dumpFileName;
	QString errorMessage;
	
	// Variables used to read input
	uint maxChar = 65536;
	TCHAR buf[65536];

	// Specify dump file name
	dumpFileName = native(QFileInfo(winOS->GetDataDirectory(), "Bump.dmp"));

	// Get the email address
	GetWindowText(GetDlgItem(hwnd, IDC_CRASHBOX_EMAIL), buf, maxChar);
	QString email(QString::fromUtf16((const ushort *) buf));

	// Get the comments
	memset(buf, 0, maxChar);
	GetWindowText(GetDlgItem(hwnd, IDC_CRASHBOX_DLG_EDIT), buf, maxChar);
	QString enteredText(QString::fromUtf16((const ushort *) buf));
	
	if (_sendFeedback && EmailCrashReport("feedback@bumptop.com", "Crash Report", enteredText, email, dumpFileName))
	{
		errorMessage = WinOSStr->getString("ErrorInfoSent");
		dlgManager->clearState();
		dlgManager->setPrompt(BtUtilStr->getString("FeedbackSent"));
		dlgManager->promptDialog(DialogOK);
	}
	else
		errorMessage = QString(WinOSStr->getString("ErrorConnectBumpHQ")).arg(dumpFileName);

	if (_resetBumpTopSettings)
	{
		Key_ResetBumpTopSettings();
		Key_DeleteSceneFiles();
		
		// Rename default theme folder
		QDir userDefaultThemePath = winOS->GetUserThemesDirectory(true);
		QDir themesDir = winOS->GetUserApplicationDataPath() / "Themes";
		vector<QString> themeDirectoryContents = fsManager->getDirectoryContents(themesDir.absolutePath());

		// Try renaming the directory to a backup
		QDir backupThemeDirectory = themesDir.absolutePath() / ("Default backup " + QString::number(themeDirectoryContents.size()));
		if (!QFile::rename(userDefaultThemePath.absolutePath(), backupThemeDirectory.absolutePath()))
		{
			// Since the rename failed that means the directory already exists. So instead we will
			// try renaming the Default theme dir to "Default Backup i" where "i" is between 1 and the number of files/folders 
			// inside the Themes Dir.
			for (int i = 0;i<themeDirectoryContents.size();i++)
			{
				backupThemeDirectory = themesDir.absolutePath() / ("Default backup " + QString::number(i));
				if (QFile::rename(userDefaultThemePath.absolutePath(), backupThemeDirectory.absolutePath()))
					break;
			}
		}

		// Reload theme will copy over original default theme
		themeManager->reloadDefaultTheme();
	}

	// The last thing we do is relaunch bumptop
	if (LOWORD(wParam) == IDOK)
		winOS->RelaunchBumpTop();

	// Close the dialog box
	EndDialog(hwnd, 1);
	return true;
}

void CrashDialog::resetToDefault()
{
	// reset internal vars
	_sendFeedback = true;
	_resetBumpTopSettings = winOS->getRegistryDwordValue("ShutdownIncomplete");
}

void CrashDialog::setResetBumpTopSettings(bool val)
{
	_resetBumpTopSettings = val;
}

void CrashDialog::setSendFeedback(bool val)
{
	_sendFeedback = val;
}

bool CrashDialog::getResetBumpTopSettings()
{
	return _resetBumpTopSettings;
}

bool CrashDialog::getSendFeedback()
{
	return _sendFeedback;
}

void CrashDialog::syncCheckboxButtonState(HWND hwnd)
{
	// uncheck all non-type radio buttons
	SendMessage(GetDlgItem(hwnd, IDC_RESET_BUMPTOP_SETTINGS), BM_SETCHECK, _resetBumpTopSettings ? BST_CHECKED : BST_UNCHECKED, 0);
	SendMessage(GetDlgItem(hwnd, IDC_SEND_FEEDBACK), BM_SETCHECK, _sendFeedback ? BST_CHECKED : BST_UNCHECKED, 0);
}