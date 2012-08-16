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
#include "BT_UpdateInstaller.h"
#include "BT_FileSystemManager.h"
#include "BT_QtUtil.h"

UpdateInstaller::UpdateInstaller( QString installerPath )
{
	_installerPath = installerPath;
}

void UpdateInstaller::runInstaller()
{
	DWORD returnValue;
	if (exists(_installerPath))
		ShellExecute(returnValue, 0, "", _installerPath, native(parent(_installerPath)), "");
}

BOOL UpdateInstaller::ShellExecute( DWORD& dwRet , int nShow , QString tcVerb, QString tcExe , QString tcWorkingDir /*= NULL */, QString tcArgs /*= NULL */ )
{
	// code taken from http://www.experts-exchange.com/Programming/Languages/CPP/Q_22753163.html
	BOOL bRet = FALSE;
	SHELLEXECUTEINFO sexi = {0};
	sexi.cbSize = sizeof( SHELLEXECUTEINFO );
	// No Parent ( Note this is just for the example )
	sexi.hwnd = NULL;
	// Set flag so the hProcess handle is initialized by the call to ShellExecuteEx
	sexi.fMask = SEE_MASK_NOCLOSEPROCESS;
	sexi.lpDirectory = (LPCWSTR) tcWorkingDir.utf16();
	sexi.lpFile = (LPCWSTR) tcExe.utf16();
	sexi.lpParameters = (LPCWSTR) tcArgs.utf16();
	sexi.nShow = nShow;
	sexi.lpVerb = NULL;

	// Returns Non-Zero on success
	return ::ShellExecuteEx( &sexi );

}