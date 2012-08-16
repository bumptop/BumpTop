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
#include "BT_KeyCombo.h"
#include "BT_WindowsOS.h"

// NOTE: this should have been defined in winuser.h according to MapVirtualKeyEx
#ifndef MAPVK_VK_TO_VSC
	#define MAPVK_VK_TO_VSC 0
#endif

KeyCombo::KeyCombo(unsigned char key, bool ctrl, bool shift, bool alt)
{
	subKeys.key = key;
	subKeys.isAltPressed = alt;
	subKeys.isCtrlPressed = ctrl;
	subKeys.isShiftPressed = shift;
	isUsingMouse = false;
}

KeyCombo::KeyCombo(MouseButtons button, bool isDoubleClick, bool isCtrl)
{
	subMouse.button = (unsigned int) button;
	subMouse.isDoubleClick = isDoubleClick;
	subMouse.isCtrlPressed = isCtrl;
	isUsingMouse = true;
}

bool KeyCombo::operator<(const KeyCombo left) const
{
	return keyStroke < left.keyStroke;
}

bool KeyCombo::operator<=(const KeyCombo left) const
{
	return keyStroke <= left.keyStroke;
}

bool KeyCombo::operator>(const KeyCombo left) const
{
	return keyStroke > left.keyStroke;
}

bool KeyCombo::operator>=(const KeyCombo left) const
{
	return keyStroke >= left.keyStroke;
}

QString KeyCombo::toString() const
{

	QString out;


	if (isUsingMouse)
	{
		if (subMouse.isCtrlPressed) 
			out.append("Ctrl+");

		if (subMouse.isDoubleClick)
			out.append("Double-Click");
		else
		{
			switch (subMouse.button)
			{
			case MouseButtonLeft: out.append(QT_TR_NOOP("Left-Click")); break;
			case MouseButtonMiddle: out.append(QT_TR_NOOP("Middle-Click")); break;
			case MouseButtonRight: out.append(QT_TR_NOOP("Right-Click")); break;
			case MouseButtonScrollDn:
			case MouseButtonScrollUp:
				out.append("ScrollWheel"); 
				break;
			default:
				break;
			}
		}
	}
	else
	{
		QString keyStr;
		switch(subKeys.key)
		{

		case KeyBackspace: keyStr = QString("Backspace"); break;
		case KeyTab: keyStr = QString("Tab"); break;
		case KeyEnter: keyStr = QString("Enter"); break;
		case KeyShift: keyStr = QString("Shift"); break;
		case KeyLeftShift: keyStr = QString("Left Shift"); break;
		case KeyRightShift: keyStr = QString("Right Shift"); break;
		case KeyControl: keyStr = QString("Control"); break;
		case KeyLeftControl: keyStr = QString("Left control"); break;
		case KeyRightControl: keyStr = QString("Right control"); break;
		case KeyAlt: keyStr = QString("Alt"); break;
		case KeyLeftAlt: keyStr = QString("Left Alt"); break;
		case KeyRightAlt: keyStr = QString("Right Alt"); break;
		case KeyEscape: keyStr = QString("Escape"); break;
		case KeySpace: keyStr = QString("Space"); break;
		case KeyPageUp: keyStr = QString("Page up"); break;
		case KeyPageDown: keyStr = QString("Page down"); break;
		case KeyEnd: keyStr = QString("End"); break;
		case KeyHome: keyStr = QString("Home"); break;
		case KeyLeft: keyStr = QString("Left"); break;
		case KeyUp: keyStr = QString("Up"); break;
		case KeyRight: keyStr = QString("Right"); break;
		case KeyDown: keyStr = QString("Down"); break;
		
		case KeyDelete: keyStr = QString("Delete"); break;
		/*
			KeyInsert		= 0x2D,
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
		*/
		case KeyF1: keyStr = QString("F1"); break;
		case KeyF2: keyStr = QString("F2"); break;
		case KeyF3: keyStr = QString("F3"); break;
		case KeyF4: keyStr = QString("F4"); break;
		case KeyF5: keyStr = QString("F5"); break;
		case KeyF6: keyStr = QString("F6"); break;
		case KeyF7: keyStr = QString("F7"); break;
		case KeyF8: keyStr = QString("F8"); break;
		case KeyF9: keyStr = QString("F9"); break;
		case KeyF10: keyStr = QString("F10"); break;
		case KeyF11: keyStr = QString("F11"); break;
		case KeyF12: keyStr = QString("F12"); break;
		/*
			KeyColon		= 0xBA,   // ';:' for US
		*/
		case KeyPlus: keyStr = QString("+"); break;
		case KeyComma: keyStr = QString(","); break;
		case KeyMinus:  keyStr = QString("-"); break;
		case KeyPeriod: keyStr = QString("."); break;
		/*
			KeyQuestionMark	= 0xBF,   // '/?' for US
			KeyTilde		= 0xC0,   // '`~' for US
			KeyLeftSqBrkt	= 0xDB,  //  '[{' for US
			KeyPipe			= 0xDC,  //  '\|' for US
			KeyRightSqBrkt	= 0xDD,  //  ']}' for US
			KeyQuote		= 0xDE  //  ''"' for US
		*/
		default:
			if (subKeys.key >= ' ' && subKeys.key <= '~')
				keyStr = QString(subKeys.key).toUpper();
			else
				keyStr = toAscii().toUpper();
			break;
		};

		// Append it to the String Structure
		if (subKeys.isCtrlPressed) out.append("Ctrl+");
		if (subKeys.isAltPressed) out.append("Alt+");
		if (subKeys.isShiftPressed) out.append("Shift+");
		out.append(keyStr);
	}

	return out;
}

bool KeyCombo::operator==(const uint key) const
{
	return key == keyStroke;
}

bool KeyCombo::operator==(const KeyCombo &key) const
{
	return keyStroke == key.keyStroke &&
		isUsingMouse == key.isUsingMouse;
}

QChar KeyCombo::toAscii() const
{
#ifdef WIN32

	HKL layout = GetKeyboardLayout(0);
	unsigned char State[256];
	wchar_t c = 0;

	// Poll the Windows Keyboard State
	if (GetKeyboardState((PBYTE) State) == FALSE)
		return 0;

	int scan = MapVirtualKeyEx(uint(subKeys.key), 0, layout);
	ToUnicodeEx(uint(subKeys.key), scan, State, &c, 1, 0, layout);

	if (c == 0)
		return QChar();

	return QChar((unsigned int) c);

#else

	return 0;

#endif
}