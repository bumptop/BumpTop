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

#ifndef BT_WIN32PATH
#define BT_WIN32PATH

// ----------------------------------------------------------------------------

class Win32Path
{
public:
	enum Win32PathKey 
	{
		AppDataDir,			// the user's app data/bumptop directory
		UserSettingsFile,	// the user's setting file path
		
		UserThemesDir,			// the user's theme directory
		UserDefaultThemeDir,	// the user's default theme directory
		UserDefaultThemeFile,	// the user's default theme file path

		UserWebWidgetDir,		// the user's widget directory
		
		ProgramDir,			// the (expected) bumptop application install directory
		ResourceBaseDir,	// one level above ProgramDir
		ApplicationFile,	// the bumptop application
		ShellExtensionFile,	// the bumptop shell extension
		DefaultThemeDir,	// bumptop's theme directory
		LanguagesDir,		// bumptop's languages directory

		StartupDir,			// the user's start menu startup directory 
		StartupLink,		// the user's bumptop start menu startup link file

		CommonStartupDir,	// the common start menu startup directory
		CommonStartupLink,	// the global bumptop start menu startup link file
	};

private:
	map<Win32PathKey, wxString> _paths;

public:
	Win32Path();

	// accessors
	const wxString& getPath(Win32PathKey key);
};

#endif // BT_WIN32PATH