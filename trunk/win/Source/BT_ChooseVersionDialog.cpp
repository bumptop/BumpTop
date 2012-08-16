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
#include "BT_ChooseVersionDialog.h"
#include "BT_HoverControl.h"
#include "BT_QtUtil.h"
#include "BT_Authorization.h"

// static inits
WNDPROC ChooseVersionDialog::prevWndProc = NULL;
const int ChooseVersionDialog::MAX_CHARS = 512;

ChooseVersionDialog::ChooseVersionDialog()
{
	resetToDefault();
}

ChooseVersionDialog::~ChooseVersionDialog()
{
	onDestroy();
}

bool ChooseVersionDialog::onInit(HWND hwnd)
{
	_dlgHwnd = hwnd;
	AdjustDialogDPI(_dlgHwnd, 500, 450);

	HWND controlHwnd = NULL;
	
	controlHwnd = GetDlgItem(hwnd, IDC_GET_PRO_BUTTON);
	AdjustControlDPI(_dlgHwnd, controlHwnd);
	_hoverButtons.push_back(new HoverControl(controlHwnd, MAKEINTRESOURCE(IDB_GET_PRO), MAKEINTRESOURCE(IDB_GET_PRO_HOVER), NULL));
	prevWndProc = (WNDPROC) SetWindowLong(controlHwnd, GWL_WNDPROC, (LONG) dialogProc);

	controlHwnd = GetDlgItem(hwnd, IDC_USE_FREE_BUTTON);
	AdjustControlDPI(_dlgHwnd, controlHwnd);
	SetWindowLong(controlHwnd, GWL_WNDPROC, (LONG) dialogProc);
	_hoverButtons.push_back(new HoverControl(controlHwnd, MAKEINTRESOURCE(IDB_USE_FREE), MAKEINTRESOURCE(IDB_USE_FREE_HOVER), NULL));
	
	controlHwnd = GetDlgItem(hwnd, IDC_ACTIVATE_BUTTON);
	AdjustControlDPI(_dlgHwnd, controlHwnd);
	SetWindowLong(controlHwnd, GWL_WNDPROC, (LONG) dialogProc);
	_hoverButtons.push_back(new HoverControl(controlHwnd, MAKEINTRESOURCE(IDB_ACTIVATE), MAKEINTRESOURCE(IDB_ACTIVATE_HOVER), NULL, MAKEINTRESOURCE(IDB_ACTIVATE_DISABLED), true));

	controlHwnd = GetDlgItem(hwnd, IDC_OTHER_GOODNESS_LINK);
	AdjustControlDPI(_dlgHwnd, controlHwnd);
	SetWindowLong(controlHwnd, GWL_WNDPROC, (LONG) dialogProc);
	_hoverButtons.push_back(new HoverControl(controlHwnd, MAKEINTRESOURCE(IDB_OTHER_GOODNESS), NULL, NULL));
	
	// Load and apply the background bitmap
	controlHwnd = GetDlgItem(hwnd, IDC_BACKGROUND_IMAGE);
	
	if (_useExpiredBackground)
		_backgroundBmp = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_CHOOSE_VERSION_EXPIRED));
	else
		_backgroundBmp = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_CHOOSE_VERSION));
	
	SendMessage(controlHwnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) _backgroundBmp);

	// Change the font size for the text box
	HFONT hFont;
	hFont = CreateFont(20,
						0,
						0,
						0,
						FW_DONTCARE,
						0,
						0,
						0,
						ANSI_CHARSET,
						OUT_DEFAULT_PRECIS,
						CLIP_DEFAULT_PRECIS,
						DEFAULT_QUALITY,
						DEFAULT_PITCH | FF_DONTCARE,
						L"Arial");
	
	controlHwnd = GetDlgItem(hwnd, IDC_PRO_KEY_TEXT_BOX);
	AdjustControlDPI(_dlgHwnd, controlHwnd);
	SendMessage(controlHwnd, WM_SETFONT, (WPARAM) hFont, MAKELPARAM(TRUE, 0));
	
	Edit_LimitText(GetDlgItem(hwnd, IDC_PRO_KEY_TEXT_BOX), MAX_CHARS);
	SetFocus(hwnd);
	return true;
}

bool ChooseVersionDialog::onCommand(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	LPWSTR buffer;
	
	// Process Buttons
	switch (LOWORD(wParam))
	{
	case TRUE:
		_useExpiredBackground = true;
		break;
	case FALSE:
		_useExpiredBackground = false;
		break;
	case IDC_PRO_KEY_TEXT_BOX:
		if (HIWORD(wParam) == EN_CHANGE)
		{
			buffer = new WCHAR[MAX_CHARS];
			memset(buffer, 0, MAX_CHARS);
			GetWindowText(GetDlgItem(hwnd, IDC_PRO_KEY_TEXT_BOX), buffer, MAX_CHARS);
			
			HWND activateButtonHwnd = GetDlgItem(hwnd, IDC_ACTIVATE_BUTTON);
			HoverControl* activateControl = getHoverControl(activateButtonHwnd);
			assert(activateControl);
			
			if (wcslen(buffer) > 0)
				activateControl->enable();	
			else
				activateControl->disable();	
			
			delete [] buffer;
		}
		break;
	default:
		return false;
	}
	return false;
}

bool ChooseVersionDialog::onMouseDown(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	HoverControl* hoverControl = getHoverControl(hwnd);

	if (hoverControl)
		hoverControl->onMouseDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

	return true;
}

bool ChooseVersionDialog::onMouseUp(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	if (hwnd == GetDlgItem(_dlgHwnd, IDC_USE_FREE_BUTTON))
	{
		closeDialog(_dlgHwnd, 0);
	}
	else if (hwnd == GetDlgItem(_dlgHwnd, IDC_GET_PRO_BUTTON))
	{
		launchBumpTopProPage("firstRun", true);
	}
	else if (hwnd == GetDlgItem(_dlgHwnd, IDC_ACTIVATE_BUTTON))
	{
		QString error;
		LPWSTR buffer = new WCHAR[MAX_CHARS];
		memset(buffer, 0, MAX_CHARS);
		GetWindowText(GetDlgItem(_dlgHwnd, IDC_PRO_KEY_TEXT_BOX), buffer, MAX_CHARS);
		if (wcslen(buffer) > 0)
		{
			if (authorizeCode(QString::fromUtf16((const ushort*) buffer), AL_PRO, error))
				closeDialog(_dlgHwnd, 1);
			else
				::MessageBox(_dlgHwnd, (LPCWSTR) error.utf16(), L"Authorization Error", MB_OK);
		}
		delete [] buffer;
	}
	else if (hwnd == GetDlgItem(_dlgHwnd, IDC_OTHER_GOODNESS_LINK))
	{
		launchBumpTopProPage("otherGoodnessLink");
	}
	
	HoverControl* hoverControl = getHoverControl(hwnd);

	if (hoverControl)
		hoverControl->onMouseUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

	return true;
}

bool ChooseVersionDialog::onMouseLeave(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	HoverControl* hoverControl = getHoverControl(hwnd);
	if (hoverControl)
		hoverControl->onMouseLeave(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	return true;
}

bool ChooseVersionDialog::onMouseMove(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	HoverControl* hoverControl = getHoverControl(hwnd);
	if (hoverControl)
		hoverControl->onMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	return true;
}

void ChooseVersionDialog::resetToDefault()
{
	_backgroundBmp = NULL;
	_useExpiredBackground = false;
}

LRESULT ChooseVersionDialog::dialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DialogType currentDialogType = dlgManager->getDialogType();
	Win32Dialog* complexDialog = dlgManager->getComplexDialog(currentDialogType);
	
	switch (msg)
	{
	case WM_LBUTTONUP:
		complexDialog->onMouseUp(hwnd, wParam, lParam);
		break;
	case WM_LBUTTONDOWN:
		complexDialog->onMouseDown(hwnd, wParam, lParam);
		break;
	case WM_MOUSELEAVE:
		complexDialog->onMouseLeave(hwnd, wParam, lParam);
		break;
	case WM_MOUSEMOVE:
		complexDialog->onMouseMove(hwnd, wParam, lParam);
		break;
	default:
		return CallWindowProc(prevWndProc, hwnd, msg, wParam, lParam);
		break;
	}
	return 0;
}

HoverControl* ChooseVersionDialog::getHoverControl(HWND handle)
{
	for (int i = 0; i < _hoverButtons.size(); i++)
	{
		if (handle == _hoverButtons[i]->getHandle())
		{
			return _hoverButtons[i];
		}
	}
	return NULL;
}

bool ChooseVersionDialog::onDestroy()
{
	for (int i = 0; i < _hoverButtons.size(); i++)
	{
		delete _hoverButtons[i];
	}
	_hoverButtons.clear();

	if (_backgroundBmp)
	{
		::DeleteObject(_backgroundBmp);
		_backgroundBmp = NULL;
	}
	return true;
}