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

#ifndef BT_HOVER_CONTROL
#define BT_HOVER_CONTROL

// -----------------------------------------------------------------------------

class HoverControl
{
private:
	HWND _hwnd;
	bool _mouseLeft;
	bool _isDisabled;
	bool _isTracking;

	HBITMAP _normalStateBitmap;
	HBITMAP _hoverStateBitmap;
	HBITMAP _downStateBitmap;
	HBITMAP _disabledStateBitmap;
	
	void init(HWND hwnd, LPCTSTR normalBmp, LPCTSTR hoverBmp, LPCTSTR downBmp, LPCTSTR disabledBmp = NULL, bool disabled = false);

public:
	HoverControl();
	HoverControl(HWND hwnd, LPCTSTR normalBmp, LPCTSTR hoverBmp, LPCTSTR downBmp, LPCTSTR disabledBmp = NULL, bool disabled = false);
	~HoverControl();

	void onMouseDown(long x, long y);
	void onMouseUp(long x, long y);
	void onMouseLeave(long x, long y);
	void onMouseMove(long x, long y);
	void enable();
	void disable();
	HWND getHandle();
};

#endif /* BT_HOVER_CONTROL */