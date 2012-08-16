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
#include "BT_DesktopLock.h"
#include "BT_WindowsOS.h"
#include "BT_QtUtil.h"

DesktopLock::DesktopLock(QString lockFileName)
{
	this->lockFileName = lockFileName;
	this->lockFileHandle = INVALID_HANDLE_VALUE;
}

DesktopLock::~DesktopLock()
{
	unlock();
}

bool DesktopLock::tryLock()
{
	if (isLocked())
		return true;

	lockFileHandle = CreateFile((LPCTSTR) native(QFileInfo(winOS->GetUserLocalApplicationDataPath(), QString(lockFileName))).utf16(),
		GENERIC_READ | GENERIC_WRITE,
		0, // don't share the file, this locks it
		NULL,
		OPEN_ALWAYS, // create the file if it doesn't exist
		FILE_ATTRIBUTE_NORMAL,
		NULL);


		
	
	return isLocked();
}
void DesktopLock::unlock()
{
	if (lockFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(lockFileHandle);
		lockFileHandle = INVALID_HANDLE_VALUE;
	}
}

bool DesktopLock::isLocked()
{
	return lockFileHandle != INVALID_HANDLE_VALUE;
}