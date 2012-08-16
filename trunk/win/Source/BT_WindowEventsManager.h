// Copyright 2011 Google Inc. All Rights Reserved.
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

#ifndef _BT_WINDOW_EVENTS_MANAGER_
#define _BT_WINDOW_EVENTS_MANAGER_

class WindowEventsManager
{
	HWINEVENTHOOK winEventHook;


	WindowEventsManager();

	static void CALLBACK HandleWinEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd, 
		LONG idObject, LONG idChild, 
		DWORD dwEventThread, DWORD dwmsEventTime);

public:
	void						init();

};

#endif