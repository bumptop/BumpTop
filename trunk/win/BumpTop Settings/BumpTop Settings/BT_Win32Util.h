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

#ifndef BT_WIN32UTIL
#define BT_WIN32UTIL

// ----------------------------------------------------------------------------

class Win32Path;

// ----------------------------------------------------------------------------

class Win32Util
{
private:
	void createLink(const wxString& workingPath, const wxString& target, 
		const wxString& args, const wxString& desc, 
		const wxString& shortcutFileName, const wxString& loc);

public:
	// operations
	void createUserStartupLink();
};

#endif // BT_WIN32UTIL