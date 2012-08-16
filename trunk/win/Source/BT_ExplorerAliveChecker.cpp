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
#include "BT_ExplorerAliveChecker.h"
#include "EnumProc.h"
#include "BT_WindowsOS.h"

ExplorerAliveChecker::ExplorerAliveChecker(HWND bumptopHwnd, HWND explorerHwnd)
{
	_bumptopHwnd = explorerHwnd;
	_explorerHwnd = explorerHwnd;
}

VOID CALLBACK ExplorerQuitCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
	// TimerOrWaitFired will always be false, since we don't have any timeout params
	ExplorerAliveChecker *e = (ExplorerAliveChecker*) lpParameter;

	if (!IsTopLevelWindow(e->getBumptopHwnd()))
	{
		winOS->AsyncExitBumpTop();

	}

};

void ExplorerAliveChecker::init()
{
	DWORD dwExplorerProcessId = NULL;

	GetWindowThreadProcessId(_explorerHwnd, &dwExplorerProcessId);

	if (dwExplorerProcessId == NULL)
		ExplorerQuitCallback(this, FALSE);

	HANDLE explorerProcessHandle = OpenProcess(SYNCHRONIZE,
									FALSE,
									dwExplorerProcessId);
	
	if (explorerProcessHandle != NULL)
	{
		HANDLE waitObjectHandle;
		// request function to t
		RegisterWaitForSingleObject(&waitObjectHandle,
									explorerProcessHandle,
									ExplorerQuitCallback,
									this, // argument to pass to callback function
									INFINITE,
									WT_EXECUTEDEFAULT | WT_EXECUTEONLYONCE);
	}

			
}

HWND ExplorerAliveChecker::getBumptopHwnd() { return _bumptopHwnd; }
