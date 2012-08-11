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

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <urlmon.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <tlhelp32.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <assert.h>
#include <wchar.h>
#include <tchar.h>
#include "msi.h"
#include "../../Source/BT_SVNRevision.h"

#define BT_REGISTRY_KEY L"SOFTWARE\\Bump Technologies, Inc.\\BumpTop"
#ifdef UNICODE
	#define ANSI_UNICODE(ansi, unicode)  unicode
#else
	#define ANSI_UNICODE(ansi, unicode)  ansi
#endif
#define myGetProcAddress(hDLL, functionName) \
	((PFN_##functionName)GetProcAddress(hDLL, (#functionName ANSI_UNICODE("A", "W"))))

int KILL_PROC_BY_NAME(const TCHAR *szToTerminate)
// Created: 6/23/2000  (RK)
// Last modified: 3/10/2002  (RK)
// Please report any problems or bugs to kochhar@physiology.wisc.edu
// The latest version of this routine can be found at:
//     http://www.neurophys.wisc.edu/ravi/software/killproc/
// Terminate the process "szToTerminate" if it is currently running
// This works for Win/95/98/ME and also Win/NT/2000/XP
// The process name is case-insensitive, i.e. "notepad.exe" and "NOTEPAD.EXE"
// will both work (for szToTerminate)
// Return codes are as follows:
//   0   = Process was successfully terminated
//   603 = Process was not currently running
//   604 = No permission to terminate process
//   605 = Unable to load PSAPI.DLL
//   602 = Unable to terminate process for some other reason
//   606 = Unable to identify system type
//   607 = Unsupported OS
//   632 = Invalid process name
//   700 = Unable to get procedure address from PSAPI.DLL
//   701 = Unable to get process list, EnumProcesses failed
//   702 = Unable to load KERNEL32.DLL
//   703 = Unable to get procedure address from KERNEL32.DLL
//   704 = CreateToolhelp32Snapshot failed
// Change history:
//   modified 3/8/2002  - Borland-C compatible if BORLANDC is defined as
//                        suggested by Bob Christensen
//   modified 3/10/2002 - Removed memory leaks as suggested by
//					      Jonathan Richard-Brochu (handles to Proc and Snapshot
//                        were not getting closed properly in some cases)
{
	BOOL bResult,bResultm;
	DWORD aiPID[1000],iCb=1000,iNumProc,iV2000=0;
	DWORD iCbneeded,i,iFound=0;
	TCHAR szName[MAX_PATH],szToTermUpper[MAX_PATH];
	HANDLE hProc,hSnapShot,hSnapShotm;
	OSVERSIONINFO osvi;
	HINSTANCE hInstLib;
	int iLen,iLenP,indx;
	HMODULE hMod;
	PROCESSENTRY32 procentry;      
	MODULEENTRY32 modentry;

	// Transfer Process name into "szToTermUpper" and
	// convert it to upper case
	iLenP=(int)wcslen(szToTerminate);
	if(iLenP<1 || iLenP>MAX_PATH) return 632;
	for(indx=0;indx<iLenP;indx++)
		szToTermUpper[indx]=toupper(szToTerminate[indx]);
	szToTermUpper[iLenP]=0;

	// PSAPI Function Pointers.
	BOOL (WINAPI *lpfEnumProcesses)( DWORD *, DWORD cb, DWORD * );
	BOOL (WINAPI *lpfEnumProcessModules)( HANDLE, HMODULE *,
		DWORD, LPDWORD );
	DWORD (WINAPI *lpfGetModuleBaseName)( HANDLE, HMODULE,
		LPTSTR, DWORD );

	// ToolHelp Function Pointers.
	HANDLE (WINAPI *lpfCreateToolhelp32Snapshot)(DWORD,DWORD) ;
	BOOL (WINAPI *lpfProcess32First)(HANDLE,LPPROCESSENTRY32) ;
	BOOL (WINAPI *lpfProcess32Next)(HANDLE,LPPROCESSENTRY32) ;
	BOOL (WINAPI *lpfModule32First)(HANDLE,LPMODULEENTRY32) ;
	BOOL (WINAPI *lpfModule32Next)(HANDLE,LPMODULEENTRY32) ;

	// First check what version of Windows we're in
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	bResult=GetVersionEx(&osvi);
	if(!bResult)     // Unable to identify system version
		return 606;

	// At Present we only support Win/NT/2000/XP or Win/9x/ME
	if((osvi.dwPlatformId != VER_PLATFORM_WIN32_NT) &&
		(osvi.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS))
		return 607;

	if(osvi.dwPlatformId==VER_PLATFORM_WIN32_NT)
	{
		// Win/NT or 2000 or XP

		// Load library and get the procedures explicitly. We do
		// this so that we don't have to worry about modules using
		// this code failing to load under Windows 9x, because
		// it can't resolve references to the PSAPI.DLL.
		hInstLib = LoadLibraryA("PSAPI.DLL");
		if(hInstLib == NULL)
			return 605;

		// Get procedure addresses.
		lpfEnumProcesses = (BOOL(WINAPI *)(DWORD *,DWORD,DWORD*))
			GetProcAddress( hInstLib, "EnumProcesses" ) ;
		lpfEnumProcessModules = (BOOL(WINAPI *)(HANDLE, HMODULE *,
			DWORD, LPDWORD)) GetProcAddress( hInstLib,
			"EnumProcessModules" ) ;
		lpfGetModuleBaseName =(DWORD (WINAPI *)(HANDLE, HMODULE,
			LPTSTR, DWORD )) GetProcAddress( hInstLib,
			"GetModuleBaseNameW" ) ;

		if(lpfEnumProcesses == NULL ||
			lpfEnumProcessModules == NULL ||
			lpfGetModuleBaseName == NULL)
		{
			FreeLibrary(hInstLib);
			return 700;
		}

		bResult=lpfEnumProcesses(aiPID,iCb,&iCbneeded);
		if(!bResult)
		{
			// Unable to get process list, EnumProcesses failed
			FreeLibrary(hInstLib);
			return 701;
		}

		// How many processes are there?
		iNumProc=iCbneeded/sizeof(DWORD);

		// Get and match the name of each process
		for(i=0;i<iNumProc;i++)
		{
			// Get the (module) name for this process

			wcscpy_s(szName,L"Unknown");
			// First, get a handle to the process
			hProc=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE,
				aiPID[i]);
			// Now, get the process name
			if(hProc)
			{
				if(lpfEnumProcessModules(hProc,&hMod,sizeof(hMod),&iCbneeded) )
				{
					iLen=lpfGetModuleBaseName(hProc,hMod,szName,MAX_PATH);
				}
			}
			CloseHandle(hProc);
			// We will match regardless of lower or upper case
#ifdef BORLANDC
			if(strcmp(strupr(szName),szToTermUpper)==0)
#else
			if(wcscmp(_wcsupr(szName),szToTermUpper)==0)
#endif
			{
				// Process found, now terminate it
				iFound=1;
				// First open for termination
				hProc=OpenProcess(PROCESS_TERMINATE,FALSE,aiPID[i]);
				if(hProc)
				{
					if(TerminateProcess(hProc,0))
					{
						// process terminated
						CloseHandle(hProc);
						FreeLibrary(hInstLib);
						return 0;
					}
					else
					{
						// Unable to terminate process
						CloseHandle(hProc);
						FreeLibrary(hInstLib);
						return 602;
					}
				}
				else
				{
					// Unable to open process for termination
					FreeLibrary(hInstLib);
					return 604;
				}
			}
		}
	}

	if(osvi.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS)
	{
		// Win/95 or 98 or ME

		hInstLib = LoadLibraryA("Kernel32.DLL");
		if( hInstLib == NULL )
			return 702;

		// Get procedure addresses.
		// We are linking to these functions of Kernel32
		// explicitly, because otherwise a module using
		// this code would fail to load under Windows NT,
		// which does not have the Toolhelp32
		// functions in the Kernel 32.
		lpfCreateToolhelp32Snapshot=
			(HANDLE(WINAPI *)(DWORD,DWORD))
			GetProcAddress( hInstLib,
			"CreateToolhelp32Snapshot" ) ;
		lpfProcess32First=
			(BOOL(WINAPI *)(HANDLE,LPPROCESSENTRY32))
			GetProcAddress( hInstLib, "Process32First" ) ;
		lpfProcess32Next=
			(BOOL(WINAPI *)(HANDLE,LPPROCESSENTRY32))
			GetProcAddress( hInstLib, "Process32Next" ) ;
		lpfModule32First=
			(BOOL(WINAPI *)(HANDLE,LPMODULEENTRY32))
			GetProcAddress( hInstLib, "Module32First" ) ;
		lpfModule32Next=
			(BOOL(WINAPI *)(HANDLE,LPMODULEENTRY32))
			GetProcAddress( hInstLib, "Module32Next" ) ;
		if( lpfProcess32Next == NULL ||
			lpfProcess32First == NULL ||
			lpfModule32Next == NULL ||
			lpfModule32First == NULL ||
			lpfCreateToolhelp32Snapshot == NULL )
		{
			FreeLibrary(hInstLib);
			return 703;
		}

		// The Process32.. and Module32.. routines return names in all uppercase

		// Get a handle to a Toolhelp snapshot of all the systems processes.

		hSnapShot = lpfCreateToolhelp32Snapshot(
			TH32CS_SNAPPROCESS, 0 ) ;
		if( hSnapShot == INVALID_HANDLE_VALUE )
		{
			FreeLibrary(hInstLib);
			return 704;
		}

		// Get the first process' information.
		procentry.dwSize = sizeof(PROCESSENTRY32);
		bResult=lpfProcess32First(hSnapShot,&procentry);

		// While there are processes, keep looping and checking.
		while(bResult)
		{
			// Get a handle to a Toolhelp snapshot of this process.
			hSnapShotm = lpfCreateToolhelp32Snapshot(
				TH32CS_SNAPMODULE, procentry.th32ProcessID) ;
			if( hSnapShotm == INVALID_HANDLE_VALUE )
			{
				CloseHandle(hSnapShot);
				FreeLibrary(hInstLib);
				return 704;
			}
			// Get the module list for this process
			modentry.dwSize=sizeof(MODULEENTRY32);
			bResultm=lpfModule32First(hSnapShotm,&modentry);

			// While there are modules, keep looping and checking
			while(bResultm)
			{
				if(wcscmp(modentry.szModule,szToTermUpper)==0)
				{
					// Process found, now terminate it
					iFound=1;
					// First open for termination
					hProc=OpenProcess(PROCESS_TERMINATE,FALSE,procentry.th32ProcessID);
					if(hProc)
					{
						if(TerminateProcess(hProc,0))
						{
							// process terminated
							CloseHandle(hSnapShotm);
							CloseHandle(hSnapShot);
							CloseHandle(hProc);
							FreeLibrary(hInstLib);
							return 0;
						}
						else
						{
							// Unable to terminate process
							CloseHandle(hSnapShotm);
							CloseHandle(hSnapShot);
							CloseHandle(hProc);
							FreeLibrary(hInstLib);
							return 602;
						}
					}
					else
					{
						// Unable to open process for termination
						CloseHandle(hSnapShotm);
						CloseHandle(hSnapShot);
						FreeLibrary(hInstLib);
						return 604;
					}
				}
				else
				{  // Look for next modules for this process
					modentry.dwSize=sizeof(MODULEENTRY32);
					bResultm=lpfModule32Next(hSnapShotm,&modentry);
				}
			}

			//Keep looking
			CloseHandle(hSnapShotm);
			procentry.dwSize = sizeof(PROCESSENTRY32);
			bResult = lpfProcess32Next(hSnapShot,&procentry);
		}
		CloseHandle(hSnapShot);
	}
	if(iFound==0)
	{
		FreeLibrary(hInstLib);
		return 603;
	}
	FreeLibrary(hInstLib);
	return 0;
}

void getCurrentDirectory(TCHAR * buffer, int len)
{
	GetModuleFileName(NULL, buffer, len);
	PathRemoveFileSpec(buffer);
}

int createProcess(TCHAR * appStr, TCHAR * commandLine)
{
	PROCESS_INFORMATION ProcessInfo = {0};
	STARTUPINFO StartupInfo = {0};
	StartupInfo.cb = sizeof(StartupInfo);
	if(CreateProcess((LPWSTR) appStr, (LPWSTR) commandLine, 
		NULL,NULL, FALSE, CREATE_NO_WINDOW | DETACHED_PROCESS,NULL,
		NULL, &StartupInfo, &ProcessInfo))
	{
		DWORD result = WaitForSingleObject(ProcessInfo.hProcess,INFINITE);
		GetExitCodeProcess(ProcessInfo.hProcess, &result);
		CloseHandle(ProcessInfo.hThread);
		CloseHandle(ProcessInfo.hProcess);

		return result;
	}
	return -1;
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	// get the wide command line args
	TCHAR * commandLine = lpCmdLine;
	if (wcslen(commandLine) == 0)
		return 0;

	// First character is an Identifier, the rest is the TargetPath
	TCHAR actionType = commandLine[0];
	TCHAR * targetPath = &commandLine[1];

	// act on the input
	if (actionType == L'K')
	{
		// kill instances of BumpTop and the Settings
		KILL_PROC_BY_NAME(L"BumpTop.exe");
		KILL_PROC_BY_NAME(L"BumpTop Settings.exe");
	}
	else if (actionType == L'I') // install
	{
		TCHAR appPath[MAX_PATH] = {0};
		TCHAR cwd[MAX_PATH] = {0};
		TCHAR * params = NULL;
		getCurrentDirectory(cwd, MAX_PATH);
		wcscpy_s(appPath, cwd);
		PathAppend(appPath, L"BumpTopUpdater.exe");
		int installLevel = _ttoi(targetPath);
		switch (installLevel)
		{
		case INSTALLUILEVEL_NONE: // completely silent installation
		case INSTALLUILEVEL_REDUCED:
			params = L"/SILENT /SUPPRESSMSGBOXES /NORESTART /LOG";
			break;
		default:
			params = L"/SP /LOG";
			break;
		}
		ShellExecute(NULL, NULL, appPath, params, cwd, SW_SHOW);
	}
	else if (actionType == L'U')
	{
		// first check if a previous version is even installed
		HMODULE hMSIDll = LoadLibrary(L"Msi.dll");
		if (hMSIDll)
		{
			TCHAR productCode[MAX_PATH];
			typedef UINT (WINAPI *PFN_MsiEnumRelatedProducts)(LPCTSTR lpUpgradeCode, DWORD dwReserved, DWORD iProductIndex, LPTSTR lpProductBuf);
			const PFN_MsiEnumRelatedProducts pfnMsiEnumRelatedProducts = myGetProcAddress(hMSIDll, MsiEnumRelatedProducts);
			typedef UINT (WINAPI *PFN_MsiQueryProductState)(LPCTSTR szProduct);
			const PFN_MsiQueryProductState pfnMsiQueryProductState = myGetProcAddress(hMSIDll, MsiQueryProductState);

			bool refreshWithDummyMSI = false;
			if (pfnMsiEnumRelatedProducts)
			{
				int i = 0; 
				while (true)
				{
					UINT result = pfnMsiEnumRelatedProducts(L"{67F5070F-3518-4B01-9024-10AB27C58446}", 0, i, productCode);
					if (result == ERROR_SUCCESS)
					{
						if (INSTALLSTATE_UNKNOWN != pfnMsiQueryProductState(productCode))
						{
							refreshWithDummyMSI = true;
							break;
						}
					}
					else
					{
						break;
					}
					++i;
				}
			} 

			if (refreshWithDummyMSI)
			{
				return 1;
			}

			FreeLibrary(hMSIDll);
		}
	}
	else if (actionType == L'S')
	{
#ifndef DISABLE_PHONING
		// only launch the survey if we aren't upgrading (upgrade product id should be empty parameter)
		if (wcslen(targetPath) == 0)
		{
			TCHAR link[256] = {0};
			TCHAR * address = L"http://www.bumptop.com/uninstall?version=";
			TCHAR * version = _T(SVN_VERSION_NUMBER);
			_tcscat(link, address);
			_tcscat(link, version);

			ShellExecute(NULL, NULL, link, NULL, NULL, SW_SHOW);
		}
#endif
	}
	else if (actionType == L'X')
	{
		TCHAR appPath[MAX_PATH] = {0};
		TCHAR cwd[MAX_PATH] = {0};
		TCHAR * params = NULL;
		getCurrentDirectory(cwd, MAX_PATH);
		wcscpy_s(appPath, cwd);
		PathAppend(appPath, L"unins000.exe");
		// uninstall without prompts if we are upgrading (upgrade product id will be non-empty parameter)
		if (wcslen(targetPath) == 0)
		{
			params = L"/SILENT /SUPPRESSMSGBOXES /NORESTART";
		}
		else
		{
			params = L"/VERYSILENT /SUPPRESSMSGBOXES /NORESTART";
		}
		ShellExecute(NULL, NULL, appPath, params, cwd, SW_SHOW);
	}

	return 0;
}