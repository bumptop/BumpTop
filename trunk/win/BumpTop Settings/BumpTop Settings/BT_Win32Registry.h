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

#ifndef BT_WIN32REGISTRY
#define BT_WIN32REGISTRY

// ----------------------------------------------------------------------------

#define BT_REGISTRY_KEY "SOFTWARE\\Bump Technologies, Inc.\\BumpTop"
#define BT_PERSISTENT_MARKER_PREFIX "PersistentViewMode_"

// ----------------------------------------------------------------------------

class Win32Registry
{
	wxString _btRegistryKey;

public:	
	Win32Registry();

	bool getValue(const wxString& valueName, DWORD& value);
	bool getValue(const wxString& valueName, wxString& value);

	bool setValue(const wxString& valueName, const DWORD& value);
	bool setValue(const wxString& valueName, const wxString& value);
};

#endif // BT_WIN32REGISTRY