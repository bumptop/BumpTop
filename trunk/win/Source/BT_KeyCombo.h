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

#ifndef _BT_KEY_COMBO_
#define _BT_KEY_COMBO_

#include "BT_MousePointer.h"

// -----------------------------------------------------------------------------

// Keyboard values
enum KeyboardValues
{

	KeyBackspace	= 0x08,
	KeyTab			= 0x09,
	KeyEnter		= 0x0D,
	KeyShift		= 0x10,
	KeyLeftShift	= 0xA0,
	KeyRightShift	= 0xA1,
	KeyControl		= 0x11,
	KeyLeftControl	= 0xA2,
	KeyRightControl	= 0xA3,
	KeyAlt			= 0x12,
	KeyLeftAlt		= 0xA4,
	KeyRightAlt		= 0xA5,
	KeyEscape		= 0x1B,

	KeySpace		= 0x20,
	KeyPageUp		= 0x21,
	KeyPageDown		= 0x22,
	KeyEnd			= 0x23,
	KeyHome			= 0x24,
	KeyLeft			= 0x25,
	KeyUp			= 0x26,
	KeyRight		= 0x27,
	KeyDown			= 0x28,
	KeyInsert		= 0x2D,
	KeyDelete		= 0x2E,

	/*
	* Key0 - Key9 are the same as ASCII '0' - '9' (0x30 - 0x39)
	* KeyA - KeyZ are the same as ASCII 'A' - 'Z' (0x41 - 0x5A)
	*/

	KeyNumpad0		= 0x60,
	KeyNumpad1      = 0x61,
	KeyNumpad2      = 0x62,
	KeyNumpad3      = 0x63,
	KeyNumpad4      = 0x64,
	KeyNumpad5      = 0x65,
	KeyNumpad6      = 0x66,
	KeyNumpad7      = 0x67,
	KeyNumpad8      = 0x68,
	KeyNumpad9      = 0x69,

	KeyF1			= 0x70,
	KeyF2			= 0x71,
	KeyF3			= 0x72,
	KeyF4			= 0x73,
	KeyF5			= 0x74,
	KeyF6			= 0x75,
	KeyF7			= 0x76,
	KeyF8			= 0x77,
	KeyF9			= 0x78,
	KeyF10			= 0x79,
	KeyF11			= 0x7A,
	KeyF12			= 0x7B,

	KeyColon		= 0xBA,   // ';:' for US
	KeyPlus			= 0xBB,   // '+' any country
	KeyComma		= 0xBC,   // ',' any country
	KeyMinus		= 0xBD,   // '-' any country
	KeyPeriod		= 0xBE,   // '.' any country
	KeyQuestionMark	= 0xBF,   // '/?' for US
	KeyTilde		= 0xC0,   // '`~' for US
	KeyLeftSqBrkt	= 0xDB,  //  '[{' for US
	KeyPipe			= 0xDC,  //  '\|' for US
	KeyRightSqBrkt	= 0xDD,  //  ']}' for US
	KeyQuote		= 0xDE  //  ''"' for US
};

// -----------------------------------------------------------------------------


class KeyCombo
{
	Q_DECLARE_TR_FUNCTIONS(KeyCombo)

public:
	union
	{
		uint keyStroke;

		struct 
		{
			unsigned char key;
			bool isAltPressed;
			bool isCtrlPressed;
			bool isShiftPressed;
		}subKeys;

		struct 
		{
			unsigned int button;
			bool isDoubleClick;
			bool isCtrlPressed;
		}subMouse;
	};
	bool isUsingMouse;

public:

	KeyCombo(unsigned char key = 0, bool ctrl = false, bool shift = false, bool alt = false);
	KeyCombo(MouseButtons button, bool isDoubleClick = false, bool isCtrl = false);
	bool operator<(const KeyCombo left) const;
	bool operator<=(const KeyCombo left) const;
	bool operator>(const KeyCombo left) const;
	bool operator>=(const KeyCombo left) const;
	bool operator==(const uint key) const;
	bool operator==(const KeyCombo &key) const;

	QString toString() const;
	QChar toAscii() const;

};

// -----------------------------------------------------------------------------

#else
	class KeyCombo;
#endif