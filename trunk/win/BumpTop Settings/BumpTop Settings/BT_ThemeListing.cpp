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
#include "BT_ThemeListing.h"
#include "BT_SettingsApp.h"

wxDirTraverseResult ThemeListing::OnDir(const wxString& dirname)
{
	// check if it has a theme.json file
	wxFileName dir(dirname + wxFileName::GetPathSeparator());
	dir.SetName(wxT("theme.json"));
	wxString str = dir.GetFullPath();
	if (dir.FileExists())
		_themes.push_back(dir);

	// do not recurse into any directory
	return wxDIR_IGNORE;
}

wxDirTraverseResult ThemeListing::OnFile(const wxString& filename)
{
	// check if it is a zip file containing a theme, if so, extract it, and remove it (or move it?)
	wxFileName file(filename);
	if (file.HasExt() && !file.GetExt().CmpNoCase(wxT("zip")))
	{
		wxString expectedFile(wxT("theme.json"));
		if (SettingsApp::UnzipFile(file, _themesDirectory, expectedFile))
		{
			_themes.push_back(wxFileName(expectedFile));
			SettingsApp::BackUpZipFile(file, _themesDirectory);
		}else{
			wxRemoveFile(file.GetFullPath());
		}
	}

	// always continue on any files
	return wxDIR_CONTINUE;
}	


void ThemeListing::recursiveZip( wxZipOutputStream &out, wxFileName fileToZip, wxFileName parentDir, wxFileName rootFolderName )
{
	bool result = false;

	// Remove parent directories from the path
	wxFileName temp = fileToZip;
	size_t size = parentDir.GetDirCount();
	for (unsigned int i = 0; i<size;i++)
	{
		temp.RemoveDir(0);
	}
	
	// make sure the parent folder is whatever folder the user wants to name their theme
	if (temp.GetDirCount() > 0)
	{
		temp.RemoveDir(0);
		temp.InsertDir(0, rootFolderName.GetName());
	}

	if (fileToZip.IsDir())
	{
		// Put the directory entry into the zip
		result = out.PutNextDirEntry(temp.GetFullPath());
		if (!result)
		{
			return;
		}

		// Recurse into each item in the directory and zip each of them
		wxDir dirToZip(fileToZip.GetFullPath());
		wxArrayString files;
		dirToZip.GetAllFiles(fileToZip.GetFullPath(), &files);
		for (unsigned int i = 0;i<files.size();i++)
		{
			recursiveZip(out, files[i], parentDir, rootFolderName);
		}
	}
	else
	{
		// Add an entry for the file
		result = out.PutNextEntry(temp.GetFullPath());
		if (!result)
		{
			return;
		}
		
		// Open up the actual file
		wxFile file;
		result = file.Open(fileToZip.GetFullPath().GetData());

		// Make sure the file opened successfully
		if (file.IsOpened())
		{
			// Create a stream to read the file
			wxFileInputStream input(file);
			wxStreamError err = input.GetLastError();
			err = input.GetLastError();
			if (input.IsOk())
			{
				// Read the file into the zipstream
				input.Read(out);
			}
			// Close the file
			file.Close();
		}
	}
}

bool ThemeListing::createArchive(wxFileName& folderToZip, wxFileName saveAsName)
{
	wxString path1, path2;
	path1 = folderToZip.GetFullPath();
	path2 = saveAsName.GetFullPath();

	// Create zip file
	wxFFileOutputStream out(saveAsName.GetFullPath());
	wxZipOutputStream zipStream(out);
	
	// Determine parent folder
	wxFileName parentDir = folderToZip;
	parentDir.RemoveLastDir();
	
	// Recursively zip the default theme folder
	recursiveZip(zipStream, folderToZip, parentDir, saveAsName);
	
	// Close the write streams
	zipStream.Close();
	out.Close();

	// Upload archive

	return true;
}

void ThemeListing::load( const wxString& themesDir )
{
	_themesDirectory = themesDir;
	if (wxFileName(_themesDirectory).DirExists())
		wxDir(_themesDirectory).Traverse(*this, wxEmptyString, wxDIR_FILES | wxDIR_DIRS);
}

void ThemeListing::clear()
{
	_themes.clear();
}

const vector<wxFileName>& ThemeListing::getThemes() const
{
	return _themes;
}

bool ThemeListing::saveTheme( const wxFileName& file, const Json::Value& root )
{
	if (file.FileExists())
	{
		Json::StyledWriter jsonWriter;
		std::string outStr = jsonWriter.write(root);
		ofstream ofstr(file.GetFullPath().c_str(), ofstream::out);
		ofstr << outStr;
		ofstr.close();
		return true;
	}
	return false;
}

void ThemeListing::activateTheme(const wxFileName& file)
{
	// copy files over if the path is not the same as the default path as it is
	bool isDefaultTheme = !file.GetFullPath().CmpNoCase(Settings->getPath(Win32Path::UserDefaultThemeFile));
	if (!isDefaultTheme)
	{
		// remove all the files in the default dir
		wxFileName userDefaultThemeDir(Settings->getPath(Win32Path::UserDefaultThemeFile));
			userDefaultThemeDir.SetFullName(wxEmptyString);
		if (userDefaultThemeDir.DirExists())
		{
			wxDir(userDefaultThemeDir.GetFullPath()).Traverse(RecursiveDeleteTraverser());
		}
		else
		{
			wxMkdir(userDefaultThemeDir.GetFullPath());
		}
		
		// copy over all the files to the default dir
		wxFileName sourceThemeDir(file);
			sourceThemeDir.SetFullName(wxEmptyString);
		if (sourceThemeDir.DirExists())
		{
			RecursiveCopyTraverser copy(sourceThemeDir.GetFullPath(), userDefaultThemeDir.GetFullPath());
			wxDir(sourceThemeDir.GetFullPath()).Traverse(copy);
		}
	}
}

wxDirTraverseResult RecursiveDeleteTraverser::OnFile(const wxString& filename)
{
	// delete file
	wxRemoveFile(filename);
	return wxDIR_CONTINUE;
}

wxDirTraverseResult RecursiveDeleteTraverser::OnDir(const wxString& dirname)
{
	// delete all the children 
	wxDir(dirname).Traverse(*this);
	wxRmdir(dirname);
	return wxDIR_IGNORE;
}

RecursiveCopyTraverser::RecursiveCopyTraverser(const wxString& fromRoot, const wxString& toRoot)
: _fromRoot(fromRoot)
, _toRoot(toRoot)
{}

wxDirTraverseResult RecursiveCopyTraverser::OnFile(const wxString& filename)
{
	// copy the file
	wxFileName relSource(filename);
	relSource.MakeRelativeTo(_fromRoot);
	wxString s= _toRoot + relSource.GetFullPath();
	wxCopyFile(filename, _toRoot + relSource.GetFullPath());

	return wxDIR_CONTINUE;
}

wxDirTraverseResult RecursiveCopyTraverser::OnDir(const wxString& dirname)
{
	// create the directory at the new root
	wxFileName relSource(dirname);
	relSource.MakeRelativeTo(_fromRoot);
	wxString s= _toRoot + relSource.GetFullPath();
	if (!wxDirExists(s))
		wxMkdir(s);

	// copy all the children 
	wxDir(dirname).Traverse(*this);

	return wxDIR_IGNORE;
}
