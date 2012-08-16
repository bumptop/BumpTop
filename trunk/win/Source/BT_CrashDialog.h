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

#ifndef _CRASH_DIALOG_
#define _CRASH_DIALOG_

// -----------------------------------------------------------------------------

#include "BT_DialogManager.h"

// -----------------------------------------------------------------------------

class CrashDialog : public Win32Dialog
{
	Q_DECLARE_TR_FUNCTIONS(CrashDialog)

public: 

private:
	bool _resetBumpTopSettings;
	bool _sendFeedback;
	// need to separate these out for default value reasons

private:
	void syncCheckboxButtonState(HWND hwnd);

public:
	CrashDialog();
	virtual ~CrashDialog();

	// dialog overrides
	virtual bool onInit(HWND hwnd);
	virtual bool onCommand(HWND hwnd, WPARAM wParam, LPARAM lParam);
	
	// reset
	virtual void resetToDefault();

	// accessors
	void setResetBumpTopSettings(bool val);
	void setSendFeedback(bool val);

	bool getSendFeedback();
	bool getResetBumpTopSettings();
};

#endif // _CRASH_DIALOG_