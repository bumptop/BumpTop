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
#include "BT_SettingsApp.h"
#include "BT_Win32Path.h"

Win32Path::Win32Path()
{
	// create the default paths
	wxString path;
	wxFileName dir;

	// AppDataDir
		path = wxStandardPaths::Get().GetUserConfigDir();
		path.Append(wxFileName::GetPathSeparator());
		dir = wxFileName(path);
		dir.AppendDir(wxT("Bump Technologies, Inc"));
		dir.AppendDir(wxT("BumpTop"));
	_paths.insert(make_pair(AppDataDir, dir.GetFullPath()));

	// UserSettingsFile
		dir = wxFileName(_paths[AppDataDir]);
		dir.SetName(wxT("settings.json"));
	_paths.insert(make_pair(UserSettingsFile, dir.GetFullPath()));

	// UserThemesDir
		dir = wxFileName(_paths[AppDataDir]);
		dir.AppendDir(wxT("Themes"));
	_paths.insert(make_pair(UserThemesDir, dir.GetFullPath()));

	// UserDefaultThemeDir
		dir = wxFileName(_paths[UserThemesDir]);
		dir.AppendDir(wxT("Default"));
	_paths.insert(make_pair(UserDefaultThemeDir, dir.GetFullPath()));

	// UserDefaultThemeFile
		dir = wxFileName(_paths[UserThemesDir]);
		dir.AppendDir(wxT("Default"));
		dir.SetName(wxT("theme.json"));
	_paths.insert(make_pair(UserDefaultThemeFile, dir.GetFullPath()));

	// UserWebWidgetDir
		dir = wxFileName(_paths[AppDataDir]);
		dir.AppendDir(wxT("WebWidgets"));
	_paths.insert(make_pair(UserWebWidgetDir, dir.GetFullPath()));
	
	// ProgramDir
	{
		dir = wxFileName(wxStandardPaths::Get().GetExecutablePath());
		dir.SetFullName(wxString());
		_paths.insert(make_pair(ProgramDir, dir.GetFullPath()));
	}

	// ResourceBaseDir
		dir = wxFileName(_paths[ProgramDir]);
		dir.RemoveLastDir();
		dir.SetFullName(wxString());
		_paths.insert(make_pair(ResourceBaseDir, dir.GetFullPath()));

	// ApplicationFile
		dir = wxFileName(_paths[ProgramDir]);
		dir.SetName(wxT("BumpTop.exe"));
	_paths.insert(make_pair(ApplicationFile, dir.GetFullPath()));

	// ShellExtensionFile
		dir = wxFileName(_paths[ProgramDir]);
		dir.SetName(wxT("BTShExt.dll"));
	_paths.insert(make_pair(ShellExtensionFile, dir.GetFullPath()));

	// DefaultThemeDir
		dir = wxFileName(_paths[ProgramDir]);
		dir.AppendDir(wxT("Themes"));
		if (!dir.DirExists()) {
			dir = wxFileName(_paths[ResourceBaseDir]);
			dir.AppendDir(wxT("Themes"));
		} else {
			_paths[ResourceBaseDir] = _paths[ProgramDir];
		}
	_paths.insert(make_pair(DefaultThemeDir, dir.GetFullPath()));

	// LanguagesDir
		dir = wxFileName(_paths[ResourceBaseDir]);
		dir.AppendDir(wxT("Languages"));
	_paths.insert(make_pair(LanguagesDir, dir.GetFullPath()));

	// StartupDir, UserStartupFile
	{
		TCHAR szPath[MAX_PATH];
		if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_STARTUP, NULL, SHGFP_TYPE_CURRENT, szPath))) 
		{
			path = wxString(szPath, wxConvUTF8);
			path.Append(wxFileName::GetPathSeparator());
			_paths.insert(make_pair(StartupDir, path));		

			dir = wxFileName(_paths[StartupDir]);
			dir.SetName(wxT("BumpTop.lnk"));
			_paths.insert(make_pair(StartupLink, dir.GetFullPath()));
		}
	}

	// CommonStartupDir, CommonStartupFile
	{
		TCHAR szPath[MAX_PATH];
		if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_COMMON_STARTUP, NULL, SHGFP_TYPE_CURRENT, szPath))) 
		{
			path = wxString(szPath, wxConvUTF8);
			path.Append(wxFileName::GetPathSeparator());
			_paths.insert(make_pair(CommonStartupDir, path));		

			dir = wxFileName(_paths[CommonStartupDir]);
			dir.SetName(wxT("BumpTop.lnk"));
			_paths.insert(make_pair(CommonStartupLink, dir.GetFullPath()));
		}
	}	
}

const wxString& Win32Path::getPath( Win32PathKey key )
{
	assert(_paths.find(key) != _paths.end());
	return _paths[key];
}