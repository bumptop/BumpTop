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
#include "BT_SmartBoard.h"
#include "BT_Util.h"
#include "BT_MouseEventManager.h"
#include "BT_WindowsOS.h"
#include "BT_EventManager.h"

#ifdef SMART_SUPPORT
#include "SBSDK2.h"

SmartBoardHandler::SmartBoardHandler() 
{
	smartBoard = new CSBSDK2();
	smartBoardAdv = new CSBSDK2Advanced(smartBoard);
}

SmartBoardHandler::~SmartBoardHandler() 
{
	consoleWrite("SmartBoardHandler::~SmartBoardHandler()  \n");

	// Delete the SDK connection
	//SAFE_DELETE(smartBoardAdv);
	//SAFE_DELETE(smartBoard);
}

void SmartBoardHandler::Create(HWND windowHwnd)
{
	consoleWrite("SmartBoardHandler::Create() \n");
	int iMajor, iMinor, iUpdate, iBuild;

	// Attach to the smart board
	smartBoard->SBSDKAttach(CSBSDKWnd(windowHwnd));
	smartBoard->SetEventHandler(this);

	// Get the versions of the smart board software
	smartBoard->SBSDKGetSoftwareVersion(&iMajor, &iMinor, &iUpdate, &iBuild);
	consoleWrite(QString("SMART Board v%1.%2.%3.%4\n").arg(iMajor).arg(iMinor).arg(iUpdate).arg(iBuild));

	// Turn on multi point mode
	smartBoardAdv->SetEventHandler(this);
	smartBoardAdv->SBSDKSetMultiPointerMode(smartBoard->SBSDKGetCurrentBoard(), true);
	smartBoardAdv->SBSDKGetMultiPointerMode(smartBoard->SBSDKGetCurrentBoard());
	smartBoardAdv->SBSDKDViTStartTrackerMode(smartBoard->SBSDKGetCurrentBoard());
}

LRESULT SmartBoardHandler::OnSBSDKNewMessage(WPARAM wParam, LPARAM lParam)
{
	// Grab new data form the device
	smartBoardAdv->SBSDKProcessData(1);
	return 0;
}

// On Pointer Down
void SmartBoardHandler::OnXYDown(int x, int y, int z, int iPointerID) 
{
	MousePointer* pointer = mousePointerFromIPointerID(iPointerID);
	pointer->setMouseButtons(pointer->getMouseButtons() | MouseButtonLeft);
	POINT pt = {x, y};
	ClientToScreen(winOS->GetWindowsHandle(), &pt);
	pointer->setX(pt.x);
	pointer->setY(pt.y);
	mouseManager->onMouseEvent(WM_LBUTTONDOWN, x, y, 0, pointer);
	consoleWrite(QString("SmartBoardHandler::OnXYDown() %1\n").arg(iPointerID));
}

// On Pointer Move
void SmartBoardHandler::OnXYMove(int x, int y, int z, int iPointerID) 
{
	MousePointer* pointer = mousePointerFromIPointerID(iPointerID);
	POINT pt = {x, y};
	ClientToScreen(winOS->GetWindowsHandle(), &pt);
	pointer->setX(pt.x);
	pointer->setY(pt.y);
	mouseManager->onMouseEvent(WM_MOUSEMOVE, x, y, 0, pointer);
	consoleWrite(QString("SmartBoardHandler::OnXYMove() %1\n").arg(iPointerID));
}

// On Pointer Up
void SmartBoardHandler::OnXYUp(int x, int y, int z, int iPointerID) 
{
	MousePointer* pointer = mousePointerFromIPointerID(iPointerID);
	POINT pt = {x, y};
	ClientToScreen(winOS->GetWindowsHandle(), &pt);
	pointer->setX(pt.x);
	pointer->setY(pt.y);
	pointer->setMouseButtons(pointer->getMouseButtons() & ~MouseButtonLeft);
	mouseManager->onMouseEvent(WM_LBUTTONUP, x, y, 0, pointer);
	consoleWrite(QString("SmartBoardHandler::OnXYUp %1\n").arg(iPointerID));
}

void SmartBoardHandler::OnXYNonProjectedDown(int x, int y, int z, int iPointerID)
{
	consoleWrite("SmartBoardHandler::OnXYNonProjectedDown() \n");
}

void SmartBoardHandler::OnXYNonProjectedMove(int x, int y, int z, int iPointerID)
{
	consoleWrite("SmartBoardHandler::OnXYNonProjectedMove() \n");
}

void SmartBoardHandler::OnXYNonProjectedUp(int x, int y, int z, int iPointerID)
{
	consoleWrite("SmartBoardHandler::OnXYNonProjectedUp() \n");
}

void SmartBoardHandler::OnPenTrayButton(SBSDK_BUTTON_ACTION baButton, int iPointerID)
{
	consoleWrite("SmartBoardHandler::OnPenTrayButton() %d\n", iPointerID);
}

MousePointer* SmartBoardHandler::mousePointerFromIPointerID(int iPointerID)
{
	if (iPointerID_to_mousePointer.find(iPointerID) == iPointerID_to_mousePointer.end())
	{
		iPointerID_to_mousePointer[iPointerID] = new MousePointer();
	}
	return iPointerID_to_mousePointer[iPointerID];
}

void SmartBoardHandler::OnXMLAnnotA(char *szXMLAnnot)
{
	// Multi-Byte
}

void SmartBoardHandler::OnXMLToolChangeA(int iBoardID, char *szXMLTool)
{
	// Multi-Byte
}

#endif