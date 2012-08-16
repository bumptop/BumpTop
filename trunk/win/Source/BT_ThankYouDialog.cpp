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
#include "BT_HoverControl.h"
#include "BT_ThankYouDialog.h"

// static inits
WNDPROC ThankYouDialog::prevWndProc = NULL;

ThankYouDialog::ThankYouDialog()
{
	resetToDefault();
}

ThankYouDialog::~ThankYouDialog()
{
	onDestroy();
}

bool ThankYouDialog::onInit(HWND hwnd)
{
	_dlgHwnd = hwnd;
	AdjustDialogDPI(_dlgHwnd, 500, 299);

	HMODULE hMod = GetModuleHandle(NULL);
	
	// Load and initialize the rollover button
	HWND controlHwnd = GetDlgItem(hwnd, IDC_NO_PROBLEM_BUTTON);
	AdjustControlDPI(_dlgHwnd, controlHwnd);
	prevWndProc = (WNDPROC) SetWindowLong(controlHwnd, GWL_WNDPROC, (LONG) dialogProc);
	_hoverButton = new HoverControl(controlHwnd, MAKEINTRESOURCE(IDB_NO_PROBLEM), MAKEINTRESOURCE(IDB_NO_PROBLEM_HOVER), NULL);

	// Load and apply the background bitmap
	controlHwnd = GetDlgItem(hwnd, IDC_BACKGROUND_IMAGE);
	_backgroundBmp = LoadBitmap(hMod, MAKEINTRESOURCE(IDB_THANK_YOU));
	SendMessage(controlHwnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) _backgroundBmp);

	return true;
}

bool ThankYouDialog::onCommand(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool ThankYouDialog::onMouseDown(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	if (_hoverButton && hwnd == _hoverButton->getHandle())
		_hoverButton->onMouseDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

	return true;
}

bool ThankYouDialog::onMouseUp(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	if (hwnd == GetDlgItem(_dlgHwnd, IDC_NO_PROBLEM_BUTTON))
	{
		closeDialog(_dlgHwnd, 0);
	}

	if (_hoverButton && hwnd == _hoverButton->getHandle())
		_hoverButton->onMouseUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

	return true;
}

bool ThankYouDialog::onMouseLeave(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	if (_hoverButton && _hoverButton->getHandle() == hwnd)
		_hoverButton->onMouseLeave(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	return true;
}

bool ThankYouDialog::onMouseMove(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	if (_hoverButton && _hoverButton->getHandle() == hwnd)
		_hoverButton->onMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	return true;
}

void ThankYouDialog::resetToDefault()
{
	_hoverButton = NULL;
	_backgroundBmp = NULL;
}

LRESULT ThankYouDialog::dialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DialogType currentDialogType = dlgManager->getDialogType();
	Win32Dialog* complexDialog = dlgManager->getComplexDialog(currentDialogType);

	switch (msg)
	{
	case WM_LBUTTONDOWN:
		complexDialog->onMouseDown(hwnd, wParam, lParam);
		break;
	case WM_LBUTTONUP:
		complexDialog->onMouseUp(hwnd, wParam, lParam);
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

bool ThankYouDialog::onDestroy()
{
	SAFE_DELETE(_hoverButton);
	
	if (_backgroundBmp)
	{
		::DeleteObject(_backgroundBmp);
		_backgroundBmp = NULL;
	}
	return true;
}