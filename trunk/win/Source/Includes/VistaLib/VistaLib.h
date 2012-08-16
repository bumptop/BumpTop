/////////////////////////
// VistaLib.h - v.1.0

/*
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 2007.  WinAbility Software Corporation. All rights reserved.

Author: Andrei Belogortseff [ http://www.winability.com ]

TERMS OF USE: You are free to use this file in any way you like, 
for both the commercial and non-commercial purposes, royalty-free,
AS LONG AS you agree with the warranty disclaimer above, 
EXCEPT that you may not remove or modify this or any of the 
preceeding paragraphs. If you make any changes, please document 
them in the MODIFICATIONS section below. If the changes are of general 
interest, please let us know and we will consider incorporating them in 
this file, as well.

If you use this file in your own project, an acknowledgement will be appreciated, 
although it's not required.

SUMMARY:

This file contains the declaration of functions exported by VistaLib.DLL. 
Most of such functions are just wrappers around the calls to the VistaTools.cxx 
functions. This DLL exports the wrappers with the extern "C" declarations to make  
it easier to use this DLL with non-C++  projects. See the descriptions of the functions 
below for information on what each function does and how to use it.

See the following article for the details:

http://www.codeproject.com/useritems/RunNonElevated.asp


Make sure you have the latest Windows SDK (see msdn.microsoft.com for more information) 
or this file may not compile!

MODIFICATIONS:

	v.1.0 (2007-May-20, by Andrei Belogortseff)

		The first release.
*/

#ifndef VISTALIB_H_INCLUDED
#define VISTALIB_H_INCLUDED

//////////////////////////////////////////////////////////////////
// if ASSERT was not defined already, let's define our own version,
// to use the CRT assert() 

#ifndef ASSERT
#	ifdef _DEBUG
#		include <assert.h>
#		define ASSERT(x) assert( x )
#		define ASSERT_HERE assert( FALSE )
#	else// _DEBUG
#		define ASSERT(x) 
#		define ASSERT_HERE 
#	endif//_DEBUG
#endif//ASSERT

#ifdef DLL_EXPORTS
#	define DLL_API __declspec(dllexport)
#else
#	ifndef NO_DLL_IMPORTS
#		define DLL_API __declspec(dllimport)
#	else
#		define DLL_API 
#	endif
#endif

//////////////////////////
// IsVista()

extern "C"
BOOL DLL_API CALLBACK 
IsVista();

/*
IsVista() is a wrapper around VistaTools::IsVista() (see VistaTools.cxx)

Use IsVista() to determine whether the current process is running under Windows Vista or 
(or a later version of Windows, whatever it will be)

Return Values:
	If the function succeeds, and the current version of Windows is Vista or later, 
		the return value is TRUE. 
	If the function fails, or if the current version of Windows is older than Vista 
		(that is, if it is Windows XP, Windows 2000, Windows Server 2003, Windows 98, etc.)
		the return value is FALSE.
*/

///////////////////////////
// IsElevated()

// return values:

#define IS_ELEVATED_ERROR	-1
#define IS_ELEVATED_NO		0
#define IS_ELEVATED_YES		1

extern "C"
int DLL_API CALLBACK 
IsElevated();

/*
IsElevated() is a wrapper around VistaTools::IsElevated() (see VistaTools.cxx)

Use IsElevated() to determine whether the current process is elevated or not.

Return Values
	IS_ELEVATED_ERROR - an error occurred.

	IS_ELEVATED_YES - the current process is elevated.

	IS_ELEVATED_NO - the current process is not elevated.
*/


/////////////////////////////
// GetElevationType()

// return values:

#define ELEVATION_TYPE_ERROR	-1
#define ELEVATION_TYPE_DEFAULT	0
#define ELEVATION_TYPE_LIMITED	1
#define ELEVATION_TYPE_FULL		2

extern "C"
int DLL_API CALLBACK 
GetElevationType();

/*
GetElevationType() is a wrapper around VistaTools::GetElevationType() (see VistaTools.cxx)

Use GetElevationType() to determine the elevation type of the current process.

Return Values:

	ELEVATION_TYPE_ERROR - an error ocured.

	ELEVATION_TYPE_DEFAULT - User is not using a "split" token. 
		This value indicates that either UAC is disabled, or the process is started
		by a standard user (not a member of the Administrators group).

	ELEVATION_TYPE_LIMITED - the process is not running elevated.

	ELEVATION_TYPE_FULL - the process is running elevated.
*/

extern "C"
BOOL DLL_API CALLBACK 
RunElevated(
	__in		HWND	hwnd, 
	__in		LPCTSTR pszPath, 
	__in_opt	LPCTSTR pszParameters	= NULL, 
	__in_opt	LPCTSTR pszDirectory	= NULL,
	__out_opt	HANDLE *phProcess		= NULL );

extern "C"
BOOL DLL_API CALLBACK 
RunNonElevated(
	__in		HWND	hwnd, 
	__in		LPCTSTR pszPath, 
	__in_opt	LPCTSTR pszParameters	= NULL, 
	__in_opt	LPCTSTR pszDirectory	= NULL,
	__out_opt	HANDLE *phProcess		= NULL );

/*

RunElevated() is a wrapper around VistaTools::RunElevated() (see VistaTools.cxx)

Use RunElevated() to start an elevated process. This function calls ShellExecEx() with the verb "runas" 
to start the elevated process. NOTE: This function will start a process elevated no matter which 
attribute (asInvoker, highestAvailable, or requireAdministrator) is specified in its manifest, 
and even if there is no such attribute at all.

RunNonElevated() is a wrapper around VistaTools::RunNonElevated() (see VistaTools.cxx)

Use RunNonElevated() to start a non-elevated process. If the current process is not elevated,
it calls ShellExecuteEx() to start the new process. If the current process is elevated,
it injects itself into the (non-elevated) shell process, and starts a non-elevated process from there.
NOTE: This function will not work as expected if the requireAdministrator attribute is specified in 
the manifest of the application: in such a case the application will be started elevated anyway.

Parameters:

hwnd
	[in] Window handle to any message boxes that the system might produce while executing this function.

pszPath
	[in] Address of a null-terminated string that specifies the name of the executable file that 
		should be used to start the process.

pszParameters
	[in] [optional] Address of a null-terminated string that contains the command-line parameters for the process. 
		If NULL, no parameters are passed to the process.

pszDirectory
	[in] [optional] Address of a null-terminated string that specifies the name of the working directory. 
		If NULL, the current directory is used as the working directory. .

phProcess
	[out] [optional] Address where to return the handle to the newly started process. 
		You must call CloseHandle( *phProcess ) when the handle is no longer needed.
		If phProcess is NULL, no handle is returned.

Return Values
	If the function succeeds, the return value is TRUE. 
	If the function fails, the return value is FALSE. To get extended error information, 
	call GetLastError().

*/

///////////////////////////////////
// The following are the wrappers around VistaTools::RunElevated() and VistaTools::RunNonElevated()
// that are callable by RunDll32.exe. 
// Refer to MSDN documentation for more information about RunDll32.exe

extern "C"
void CALLBACK RunElevatedW(HWND hwnd, HINSTANCE hInstance, LPWSTR pszCmdLine, int nCmdShow);

extern "C"
void CALLBACK RunNonElevatedW(HWND hwnd, HINSTANCE hInstance, LPWSTR pszCmdLine, int nCmdShow);

/*
How to use these entry points with RunDll32.exe:

Normally, if you want to run SampleApp.exe with the command line parameter /sample_parameter,
you would use the ShellExecute (or SheellExecuteEx) APIs and specify:

File to run: "C:\Program Files\SampleApp.exe"
Parameters: /sample_parameter

However, if you want to control the elevation level of SampleApp.exe at the runtime,
you need to specify the asInvoker attribute in the application manifest of SampleApp.exe,
and use RunDll32.exe with VistaLib32.dll to run SampleApp.exe non-elevated as follows:

File to run: RunDll32.exe (usually located in C:/Windows/System32)
Parameters: "C:\Program Files\VistaLib32.dll",RunNonElevated "C:\Program Files\SampleApp.exe" /some_parameters

If running under Windows XP or 2000, such commands would start the process as usual, 
as if you were using ShellExec directly.

On a 64-bit Vista, replace VistaLib32.dll with VistaLib64.dll.

*/

#endif// VISTALIB_H_INCLUDED
