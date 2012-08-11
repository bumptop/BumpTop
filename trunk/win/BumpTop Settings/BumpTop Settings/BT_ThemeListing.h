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

#ifndef BT_THEMELISTING
#define BT_THEMELISTING

// ----------------------------------------------------------------------------

class RecursiveCopyTraverser : public wxDirTraverser
{
	wxString _fromRoot;
	wxString _toRoot;

private:
	virtual wxDirTraverseResult OnFile(const wxString& filename);
	virtual wxDirTraverseResult OnDir(const wxString& dirname);

public:
	RecursiveCopyTraverser(const wxString& fromRoot, const wxString& toRoot);
};

// ----------------------------------------------------------------------------

class RecursiveDeleteTraverser : public wxDirTraverser
{
	virtual wxDirTraverseResult OnFile(const wxString& filename);
	virtual wxDirTraverseResult OnDir(const wxString& dirname);
};

// ----------------------------------------------------------------------------

class ThemeListing : public wxDirTraverser
{
	wxString _themesDirectory;

	// set of theme properties files
	vector<wxFileName> _themes;

private:
	virtual wxDirTraverseResult OnFile(const wxString& filename);
	virtual wxDirTraverseResult OnDir(const wxString& dirname);

	void recursiveZip (wxZipOutputStream &out, wxFileName fileToZip, wxFileName parentDir, wxFileName rootFolderName);

public:
	// operations
	void load(const wxString& themesDir);
	bool saveTheme(const wxFileName& file, const Json::Value& root);
	void activateTheme(const wxFileName& file);
	void clear();
	bool createArchive(wxFileName& fileToZip, wxFileName saveAsName);

	// accessors
	const vector<wxFileName>& getThemes() const;
};

#endif // BT_THEMELISTING