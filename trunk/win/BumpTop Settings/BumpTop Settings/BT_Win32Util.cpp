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
#include "BT_Win32Util.h"
#include "BT_Win32Path.h"
#include "BT_SettingsApp.h"

void Win32Util::createLink(const wxString& workingPath, const wxString& target, 
						   const wxString& args, const wxString& desc, 
						   const wxString& shortcutFileName, const wxString& loc)
{
	wxString linkPath = loc + shortcutFileName;
	wxString targetPath = workingPath + target;

	HRESULT hres = NULL;
	IShellLinkW * psl = NULL;
	IPersistFile * ppf = NULL;

	// Get a pointer to the IShellLink interface.
	CoInitialize(NULL);
	if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, 
		IID_IShellLink, (void **) &psl)))
	{
		// Set the path to the shortcut target
		psl->SetPath(targetPath.c_str());
		psl->SetDescription(desc.c_str());
		psl->SetWorkingDirectory(workingPath.c_str());
		psl->SetArguments(args.c_str());

		// Query IShellLink for the IPersistFile interface for
		// saving the shortcut in persistent storage.
		hres = psl->QueryInterface(IID_IPersistFile, (void **) &ppf);

		if (SUCCEEDED(hres))
		{
			// Save the link by calling IPersistFile::Save.
			hres = ppf->Save(linkPath.c_str(), TRUE);
			ppf->Release();
		}

		psl->Release();
	}
	CoUninitialize();
}

void Win32Util::createUserStartupLink()
{
	createLink(Settings->getPath(Win32Path::ProgramDir), wxT("BumpTop.exe"), 
		wxEmptyString, wxT("BumpTop"), wxT("BumpTop.lnk"), 
		Settings->getPath(Win32Path::StartupDir));
}