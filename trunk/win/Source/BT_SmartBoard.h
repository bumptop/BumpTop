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

//#define SMART_SUPPORT //uncomment to enable Smart Board support


#ifdef SMART_SUPPORT
#ifndef _BT_SMART_BOARD_
#define _BT_SMART_BOARD_

// -----------------------------------------------------------------------------

// Needed to ignore storage class definition
//#define SBSDK_AS_LIBRARY 1

// -----------------------------------------------------------------------------

#include "BT_MousePointer.h"
#include "SBSDK2.h"
#include "SBSDK2Advanced.h"

// -----------------------------------------------------------------------------

class SmartBoardHandler : public CSBSDK2EventHandler, public CSBSDK2AdvancedEventHandler
{
	hash_map<int,MousePointer*> iPointerID_to_mousePointer;

	CSBSDK2 *smartBoard;
	CSBSDK2Advanced *smartBoardAdv;

	MousePointer* mousePointerFromIPointerID(int iPointerID);
public:

	SmartBoardHandler();
	~SmartBoardHandler();

	void Create(HWND windowHwnd);
	LRESULT OnSBSDKNewMessage(WPARAM wParam, LPARAM lParam);

	void OnXYDown(int x, int y, int z, int iPointerID);
	void OnXYMove(int x, int y, int z, int iPointerID);
	void OnXYUp(int x, int y, int z, int iPointerID);

	void OnXYNonProjectedDown(int x, int y, int z, int iPointerID);
	void OnXYNonProjectedMove(int x, int y, int z, int iPointerID);
	void OnXYNonProjectedUp(int x, int y, int z, int iPointerID);

	void OnXMLAnnotA(char *szXMLAnnot);
	void OnXMLToolChangeA(int iBoardID, char *szXMLTool);

	void OnPenTrayButton(SBSDK_BUTTON_ACTION baButton, int iPointerID);
};

// -----------------------------------------------------------------------------

#else
	class SmartBoardHandler;
#endif

#endif