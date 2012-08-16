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

HoverControl::HoverControl()
{
	init(NULL, NULL, NULL, NULL, NULL, false);
}

HoverControl::HoverControl(HWND hwnd, LPCTSTR normalBmp, LPCTSTR hoverBmp, LPCTSTR downBmp, LPCTSTR disabledBmp, bool disabled)
{
	init(hwnd, normalBmp, hoverBmp, downBmp, disabledBmp, disabled);
}

HoverControl::~HoverControl()
{
	if (_normalStateBitmap)
		::DeleteObject(_normalStateBitmap);
	if (_hoverStateBitmap)
		::DeleteObject(_hoverStateBitmap);
	if (_downStateBitmap)
		::DeleteObject(_downStateBitmap);
	if (_disabledStateBitmap)
		::DeleteObject(_disabledStateBitmap);
}

void HoverControl::init(HWND hwnd, LPCTSTR normalBmp, LPCTSTR hoverBmp, LPCTSTR downBmp, LPCTSTR disabledBmp, bool disabled)
{
	_hwnd = hwnd;
	_mouseLeft = true;
	_isTracking = false;
	_isDisabled = disabled;

	_normalStateBitmap = _hoverStateBitmap = _downStateBitmap = _disabledStateBitmap = NULL;
	
	if (normalBmp)
		_normalStateBitmap = LoadBitmap(GetModuleHandle(NULL), normalBmp);
	if (hoverBmp)
		_hoverStateBitmap = LoadBitmap(GetModuleHandle(NULL), hoverBmp);
	if (downBmp)
		_downStateBitmap = LoadBitmap(GetModuleHandle(NULL), downBmp);
	if (disabledBmp)
		_disabledStateBitmap = LoadBitmap(GetModuleHandle(NULL), disabledBmp);
	
	if (_isDisabled && _disabledStateBitmap)
		SendMessage(_hwnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) _disabledStateBitmap);
	else if (!_isDisabled && _normalStateBitmap)
		SendMessage(_hwnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) _normalStateBitmap);
}	

void HoverControl::onMouseDown(long x, long y)
{
	if (!_isDisabled)
		SetCursor(LoadCursor(NULL, IDC_HAND));
}

void HoverControl::onMouseUp(long x, long y)
{

}

void HoverControl::onMouseLeave(long x, long y)
{
	if (!_mouseLeft)
	{
		// MOUSE LEAVE ACTION
		if (!_isDisabled && _normalStateBitmap)
			SendMessage(_hwnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) _normalStateBitmap);
		_mouseLeft = true;
	}

	_isTracking = false;
}

void HoverControl::onMouseMove(long x, long y)
{
	if (!_isTracking)
	{
		// TrackMouseEvents disable after one message is received, so we need to
		// re-enable them.
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(tme);
		tme.dwFlags = TME_LEAVE;
		tme.dwHoverTime = HOVER_DEFAULT;
		tme.hwndTrack = _hwnd;
		TrackMouseEvent(&tme);
		_isTracking = true;
	}

	if (_mouseLeft)
	{
		// MOUSE OVER ACTION
		if (!_isDisabled && _hoverStateBitmap)
			SendMessage(_hwnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) _hoverStateBitmap);
		_mouseLeft = false;
	}

	if (!_isDisabled)
		SetCursor(LoadCursor(NULL, IDC_HAND));
}

void HoverControl::enable()
{
	if (!_isDisabled)
		return;

	if (_mouseLeft && _normalStateBitmap)
		SendMessage(_hwnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) _normalStateBitmap);
	else if (!_mouseLeft && _hoverStateBitmap)
		SendMessage(_hwnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) _hoverStateBitmap);
	_isDisabled = false;
}

void HoverControl::disable()
{
	if (_isDisabled)
		return;

	if (_disabledStateBitmap)
		SendMessage(_hwnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) _disabledStateBitmap);
	_isDisabled = true;
}

HWND HoverControl::getHandle()
{
	return _hwnd;
}