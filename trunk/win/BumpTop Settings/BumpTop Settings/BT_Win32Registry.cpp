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
#include "BT_Win32Registry.h"

Win32Registry::Win32Registry()
: _btRegistryKey(wxT(BT_REGISTRY_KEY))
{}

bool Win32Registry::getValue(const wxString& valueName, DWORD& valueOut)
{
	DWORD dwValue = 0;
	DWORD dwType = REG_DWORD;
	DWORD dwSize = sizeof(dwValue);
	HKEY hKey;
	LONG returnStatus = RegOpenKeyEx(HKEY_CURRENT_USER, _btRegistryKey.c_str(), 0L, KEY_READ, &hKey);
	if (returnStatus == ERROR_SUCCESS)
	{
		returnStatus = RegQueryValueEx(hKey, valueName.c_str(), NULL, &dwType, (LPBYTE) &dwValue, &dwSize);
		if (returnStatus == ERROR_SUCCESS)
		{
			valueOut = dwValue;
		}
	}
	RegCloseKey(hKey);
	return (returnStatus == ERROR_SUCCESS);
}

bool Win32Registry::getValue(const wxString& valueName, wxString& valueOut)
{
	char lszValue[MAX_PATH];
	HKEY hKey;
	DWORD dwType = REG_SZ;
	DWORD dwSize = MAX_PATH;
	LONG returnStatus = RegOpenKeyEx(HKEY_CURRENT_USER, _btRegistryKey.c_str(), 0L, KEY_READ, &hKey);
	if (returnStatus == ERROR_SUCCESS)
	{
		returnStatus = RegQueryValueEx(hKey, valueName.c_str(), NULL, &dwType, (LPBYTE) &lszValue, &dwSize);
		if (returnStatus == ERROR_SUCCESS)
		{
			valueOut = wxString(lszValue, wxConvUTF8);
		}
	}
	RegCloseKey(hKey);
	return (returnStatus == ERROR_SUCCESS);
}

bool Win32Registry::setValue(const wxString& valueName, const DWORD& value)
{
	HKEY hKey;
	LONG returnStatus = RegOpenKeyEx(HKEY_CURRENT_USER, _btRegistryKey.c_str(), 0L, KEY_WRITE, &hKey);
	if (returnStatus == ERROR_SUCCESS)
	{
		returnStatus = RegSetValueEx(hKey, valueName.c_str(), 0, REG_DWORD, (LPBYTE)&value, sizeof(value));
	}
	RegCloseKey(hKey);
	return (returnStatus == ERROR_SUCCESS);
}

bool Win32Registry::setValue(const wxString& valueName, const wxString& value)
{
	HKEY hKey;
	LONG returnStatus = RegOpenKeyEx(HKEY_CURRENT_USER, _btRegistryKey.c_str(), 0L, KEY_WRITE, &hKey);
	if (returnStatus == ERROR_SUCCESS)
	{
		returnStatus = RegSetValueEx(hKey, valueName.c_str(), 0, REG_SZ, (LPBYTE) value.c_str(), (DWORD) value.size()+1);
	}
	RegCloseKey(hKey);
	return (returnStatus == ERROR_SUCCESS);
}
