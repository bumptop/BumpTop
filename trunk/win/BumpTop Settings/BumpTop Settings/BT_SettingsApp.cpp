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
#include "BT_SVNRevision.h"
#include "BT_SettingsApp.h"
#include "BT_SettingsAppEventHandler.h"

// wx app implementation
IMPLEMENT_APP(SettingsApp)

// helper macro
#define GetCtrlHelper(id, ctrlType) \
	GetWXControl(id, ctrlType, _dialog)

SettingsApp::SettingsApp()
: _adminTabDeleted(false)
, _language("", wxConvUTF8)
, _dialog(NULL)
{}

void SettingsApp::syncValue( bool fromValueToDialog, Json::Value& boolVal, wxCheckBox * cb, bool inverse )
{
	if (fromValueToDialog)
		cb->SetValue((inverse ? !boolVal.asBool() : boolVal.asBool()));
	else
		boolVal = (inverse ? !cb->GetValue() : cb->GetValue());
}
/*
void SettingsApp::syncValue( bool fromValueToDialog, const string& boolRegistryVal, wxCheckBox * cb )
{
	if (fromValueToDialog)
	cb->SetValue(_registry.GetBoolValue("key"));
	else
	_registry.SetBoolValue("key", cb->GetValue());
}
*/
void SettingsApp::syncValue( bool fromValueToDialog, Json::Value& val, wxSpinCtrl * spin, bool asFloat)
{
	if (fromValueToDialog)
		spin->SetValue(asFloat ? val.asDouble() : val.asInt());
	else
		val = asFloat ? Json::Value(float(spin->GetValue())) : Json::Value(int(spin->GetValue()));
}
void SettingsApp::syncValue( bool fromValueToDialog, Json::Value& intVal, wxChoice * choice )
{
	if (fromValueToDialog)
	{
		wxString strVal = wxString::Format(wxT("%i"), intVal.asInt());
		wxArrayString choices = choice->GetStrings();
		for (unsigned int i = 0; i < choices.size(); ++i)
		{
			if (choices[i] == strVal)
			{
				choice->Select(i);
				break;
			}
		}
	}
	else
	{
		long l = 0;
		choice->GetStringSelection().ToLong(&l);
		intVal = (int) l;
	}
}
void SettingsApp::syncValue( bool fromValueToDialog, Json::Value& arrVal, wxListBox * list )
{
	if (fromValueToDialog)
	{
		if (!arrVal.isArray())
			throw invalid_argument("Not an array value");

		list->Clear();
		for (unsigned int i = 0; i < arrVal.size(); ++i)
			list->Append(wxString(arrVal[i].asString().c_str(), wxConvUTF8));
	}
	else
	{
		throw runtime_error("Unexpected");
	}
}
void SettingsApp::syncValue( bool fromValueToDialog, Json::Value& strVal, wxTextCtrl * text)
{
	if (fromValueToDialog)
	{
		text->SetValue(wxString(strVal.asString().c_str(), wxConvUTF8));
	}
	else
	{
		strVal = string(text->GetValue().mb_str());
	}	
}
void SettingsApp::syncValue( bool fromValueToDialog, Json::Value& strVal, wxFilePickerCtrl * file )
{
	if (fromValueToDialog)
	{
		file->SetPath(wxString(strVal.asString().c_str(), wxConvUTF8));
	}
	else
	{
		strVal = string(file->GetPath().mb_str());
	}	
}
void SettingsApp::syncValue(bool fromValueToDialog, Json::Value& intVal, wxSlider * slider)
{
	if (fromValueToDialog)
	{
		slider->SetValue(intVal.asInt());
	}
	else
	{
		intVal = slider->GetValue();
	}
}
void SettingsApp::syncTextureToValue( Json::Value& strVal, const wxFileName& strValueDir, wxFilePickerCtrl * filepicker )
{
	wxFileName file(filepicker->GetPath());
	if (file.IsOk() && file.IsAbsolute())
	{
		// copy over to the directory
		wxString name = file.GetFullName();
		wxCopyFile(file.GetFullPath(), strValueDir.GetFullPath() + name);
		strVal = string(name.mb_str());
	}
	else
	{
		// do nothing
		strVal = string(filepicker->GetPath().mb_str());
	}
}

wxFileName& SettingsApp::composePath(wxFileName& path, const Json::Value& dir)
{
	path = wxFileName(path.GetFullPath() + wxString(dir.asString().c_str(), wxConvUTF8));
	path.Normalize();
	return path;
}


wxDialog * SettingsApp::getDialog() const
{
	return _dialog;
}

ThemeListing& SettingsApp::getThemes()
{
	return _themes;
}

bool SettingsApp::OnInit()
{
	// This process needs to be DPI aware for dpi scaling to work properly 
	bool versionGreaterThanVista = false;

	// Object used to store OS version information
	OSVERSIONINFO osvi;
	// Allocate memory for osvi
	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	// Get the OS Version information
	GetVersionEx(&osvi);

	versionGreaterThanVista = (osvi.dwMajorVersion >= 6);

	if(versionGreaterThanVista) {
		HMODULE _hMod = LoadLibrary(_T("User32.dll"));
		typedef BOOL (__stdcall *SetProcessDPIAwareSignature)(void);
		SetProcessDPIAwareSignature f = GetProcAddress(_hMod,"SetProcessDPIAware");
		f();

		FreeLibrary(_hMod);
	}

	// call parent init
	wxApp::OnInit();

	// single instance check
	const wxString checkAppName = wxString::Format(wxT("BumpTopSettings-%s"), wxGetUserId().c_str());
	_instanceChecker = new wxSingleInstanceChecker(checkAppName);
	if ( _instanceChecker->IsAnotherRunning() )
	{
		// bumptopsettings.OnInitExistingInstance.caption
		// bumptopsettings.OnInitExistingInstance.text
		wxMessageBox(wxGetTranslation(wxT("There is another BumpTop Settings dialog already open!")), 
			wxGetTranslation(wxT("Am I seeing double?")));
		return false;
	}
	
	if (wxFileName::FileExists(_settingsFile.GetFullPath()))
	{
		_dialog = new SettingsDialog;
		_handler = new SettingsAppEventHandler;
		_themes.load(_paths.getPath(Win32Path::UserThemesDir));
		_themes.load(_paths.getPath(Win32Path::DefaultThemeDir));
		int defaultClientWindowWidth = 486;
		int defaultClientWindowHeight = 416;
		int defaultDPI = 96;

		// init handlers
		wxImage::AddHandler(new wxPNGHandler);
		wxXmlResource::Get()->InitAllHandlers();
		
		// load dialog from resource
		/*
		wxXmlResource::Get()->Load(wxT("Settings.xrc"));
		*/
		InitXmlResource();
		wxXmlResource::Get()->LoadDialog(_dialog, NULL, wxT("ID_SETTINGSDIALOG"));

	
		_bumptopComm.hookWndProc((HWND)_dialog->GetHandle());
		
		// prepare dialog
		Json::Value root;
		if (loadSettingsFile(root))
		{
			syncSettings(true, root);
			bindSettingsEvents();

			// start the window check timer
			if (_bumptopComm.getWindowHandle())
			{
				_bumptopComm.getWindowCheckTimer().Connect(wxID_ANY, wxEVT_TIMER, wxTimeEventHandler(SettingsAppEventHandler::OnWindowCheckTimer), 0, _handler);
				_bumptopComm.getWindowCheckTimer().Start(500);
			}

			// show dialog
			_dialog->SetIcon(wxIcon(wxT("bt_settings.ico")));
			
			// Determine appropriate size for dialog box
			// If a user modifies their default DPI to make the text larger
			// the bumptop settings window needs to be scaled to support this
			HDC hdc = GetDC(NULL);
			if (hdc)
			{
				int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
				int dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
				int width = MulDiv(defaultClientWindowWidth, dpiX, defaultDPI);
				int height = MulDiv(defaultClientWindowHeight, dpiY, defaultDPI);
				_dialog->SetClientSize(width, height);
				ReleaseDC(NULL, hdc);
			}

			wxNotebook* tabs = GetCtrlHelper(ID_NOTEBOOK, wxNotebook);
			if(_focusedTabIndex > 0)
				tabs->SetSelection(_focusedTabIndex);

			// set loading animation
			wxAnimationCtrl * animCtrl = GetCtrlHelper(ID_THROBBER, wxAnimationCtrl);
			animCtrl->LoadFile(_paths.getPath(Win32Path::ResourceBaseDir) + wxT("\\textures\\loader.gif")); 

			return true;
		}
	}

	return false;
}

int SettingsApp::OnRun()
{
	_dialog->ShowModal();
	return 0;
}

int SettingsApp::OnExit()
{
	_dialog->Destroy();
	delete _instanceChecker;
	return 0;
}

void SettingsApp::OnInitCmdLine( wxCmdLineParser& parser )
{
	wxApp::OnInitCmdLine(parser);
	
	// we are looking for these switches/ids
	parser.AddSwitch(wxT("admin"));
	parser.AddOption(wxT("settingsFile"));
	parser.AddOption(wxT("bumptopHWND"));
	parser.AddOption(wxT("inviteCode"));
	parser.AddOption(wxT("language"));
	parser.AddOption(wxT("proUrl"));
	parser.AddOption(wxT("setTheme"));
	parser.AddOption(wxT("addWidget"));
	parser.AddSwitch(wxT("themesTab"));
}

BOOL CALLBACK SettingsApp::EnumProc(HWND hwnd, LPARAM lp)
{
	HWND shellWin = FindWindowEx(hwnd, NULL, L"SHELLDLL_DefView", NULL);
	if (shellWin)
	{
		// Set bumptopComm
		HWND bumptopWindow = FindWindowEx(shellWin, NULL, L"BumpTop", NULL);
		SettingsApp *inst = (SettingsApp *)lp;
		inst->_bumptopComm.load(bumptopWindow);

		// Return false to stop enumerating windows
		return false;
	}
	// We didn't find the window so keep enumerating
	return true;
}

bool SettingsApp::OnCmdLineParsed( wxCmdLineParser& parser )
{
	// call parent parsed
	wxApp::OnCmdLineParsed(parser);

	// get the language (default to english)
	parser.Found(wxT("language"), &_language);

	// set the locale depending on the input
	if (!_language.IsEmpty())
		setLocale(_language);

	// determine if we are launching this as an admin
	_isAdmin = parser.Found(wxT("admin"));


	if(parser.Found(wxT("themesTab"))) {
		_focusedTabIndex = 3;
	} else {
		_focusedTabIndex = -1;
	}

	// get the settings file
	wxString path;
	if (!parser.Found(wxT("settingsFile"), &path))
	{
		// if no settings file was specified, then just use the settings 
		// in the application folder directory
		path = _paths.getPath(Win32Path::UserSettingsFile);
		_settingsFile = wxFileName(path);
	}
	else
		_settingsFile = wxFileName(_settingsFile);

	// ensure that the settings file exists
	if (!wxFileName::FileExists(path))
	{
		// bumptopsettings.OnInitNoSettings.caption
		// bumptopsettings.OnInitNoSettings.text
		wxString errMsg = wxGetTranslation(wxT("Could not find BumpTop's Settings file."));
		wxMessageBox(errMsg, wxGetTranslation(wxT("Hmm... Missing BumpTop Settings")), wxICON_EXCLAMATION | wxCENTER);
		return false;
	}
	
	// Load theme on double click of .bumptheme file
	wxString themePath;
	if (parser.Found(wxT("setTheme"), &themePath))
	{	
		// Copy the theme file to user themes dir
		wxFileName themeName(themePath);
		wxFileName themesFolder(_paths.getPath(Win32Path::UserThemesDir));
		if(!wxDir::Exists(themesFolder.GetFullPath()))
			wxMkdir(themesFolder.GetFullPath());

		wxFileName themeZipFileName(_paths.getPath(Win32Path::UserThemesDir) + themeName.GetName() + wxT(".zip"));
		if (wxCopyFile(themeName.GetFullPath(), themeZipFileName.GetFullPath()))
		{
			// Unzip contents, delete the zip file if unable to unzip for any reason
			wxString expectedFile(wxT("theme.json"));
			if(UnzipFile(themeZipFileName, themesFolder, expectedFile)) //Add parameter to find an expected file such as smoething.json
			{
				BackUpZipFile(themeZipFileName, themesFolder.GetFullPath());
				_themes.activateTheme(wxFileName(expectedFile));

				// See if a bumptop window exists
				::EnumWindows(EnumProc, (LPARAM)this);
				if (_bumptopComm.getWindowHandle() != NULL) //_bumptopComm is loaded inside EnumWindows
					_bumptopComm.sendMessage(Win32Comm::ReloadTheme, true);	
			}else{
				wxRemoveFile(themeZipFileName.GetFullPath());
			}
		}
		exit(0);		
	}

	// Load widget (google gadget) into bumptop on double click of .bumpwidget file
	wxString widgetPath;
	if (parser.Found(wxT("addWidget"), &widgetPath))
	{	
		wxFileName widgetName(widgetPath);
		wxFileName widgetsFolder(_paths.getPath(Win32Path::UserWebWidgetDir));
		if(!wxDir::Exists(widgetsFolder.GetFullPath()))
			wxMkdir(widgetsFolder.GetFullPath());

		wxFileName widgetZipFileName(widgetsFolder.GetFullPath() + widgetName.GetName() + wxT(".zip"));
		wxString * htmlData = NULL;
		if (wxCopyFile(widgetName.GetFullPath(), widgetZipFileName.GetFullPath()))
		{
			// Unzip contents, delete the zip file if unable to unzip for any reason
			wxString expectedFile(wxT("widget.json"));
			if(UnzipFile(widgetZipFileName, widgetsFolder, expectedFile)) //Add parameter to find an expected file such as smoething.json
			{
				Json::Value root;
				if(loadJSONFile(wxFileName(expectedFile), root))
				{
					htmlData = new wxString(root["widget"]["html"].asString().c_str(), *wxConvCurrent);
					BackUpZipFile(widgetZipFileName, widgetsFolder.GetFullPath());
				}
			}else{
				wxRemoveFile(widgetZipFileName.GetFullPath());
			}
		}

		if(htmlData)
		{
			// Package html data for bumpwidget and send it to bumptop window
			wchar_t * dataToPass = (wchar_t *)htmlData->fn_str();
			COPYDATASTRUCT cpd;
			cpd.cbData = (htmlData->size() * sizeof(wchar_t)); // wxString is made up of wxChar's which are wchar_t's
			cpd.lpData = dataToPass;
			::EnumWindows(EnumProc, (LPARAM)this);
			if (_bumptopComm.getWindowHandle() != NULL) //_bumptopComm is loaded inside EnumWindows
				_bumptopComm.sendDataMessage(Win32Comm::InstallWebWidget, cpd);
			delete htmlData;
		}
		
		exit(0);		
	}

	// get the bumptop window reference
	wxString hwnd;
	if (!parser.Found(wxT("bumptopHWND"), &hwnd))
	{
		// bumptopsettings.OnInitNoWindow.caption
		// bumptopsettings.OnInitNoWindow.text
		wxMessageBox(wxGetTranslation(wxT("Hmm... No BumpTop window specified for this Settings dialog to edit. Certain changes will not be available and have been disabled.")), 
			wxGetTranslation(wxT("No BumpTop Window Specified")), wxICON_EXCLAMATION | wxCENTER);
	}
	else
	{
		unsigned long addr;
		hwnd.ToULong(&addr);
		_bumptopComm.load((HWND) addr);
	}
	
	// get the invite code
	parser.Found(wxT("inviteCode"), &_inviteCode);

	parser.Found(wxT("proUrl"), &_proUrl);

	return true;
}

void SettingsApp::BackUpZipFile(const wxFileName& fileToBackUp,  const wxString& backUpLocation)
{
	// Create backup directory under backUpLocation
	wxFileName backUpDir(backUpLocation);
	backUpDir.AppendDir(wxT("Backup"));
	wxFileName::Mkdir(backUpDir.GetFullPath(), wxS_DEFAULT, wxPATH_MKDIR_FULL);

	// Move the file over there
	backUpDir.SetFullName(fileToBackUp.GetFullName());
	wxCopyFile(fileToBackUp.GetFullPath(), backUpDir.GetFullPath());
	bool b = wxRemoveFile(fileToBackUp.GetFullPath());
}

bool SettingsApp::UnzipFile( const wxFileName& fileToUnzip, const wxFileName& unzipLocation, wxString& expectedFile )
{
	auto_ptr<wxZipEntry> entry;
	wxFFileInputStream in(fileToUnzip.GetFullPath());
	wxZipInputStream zipConfirmation(in);
	wxZipInputStream zip(in);
	bool foundExpectedFile = false;

	// Check to make sure that the zip file contains the expected file before unzipping
	do 
	{
		entry.reset(zipConfirmation.GetNextEntry());
		if (entry.get())
		{
			wxFileName entryName(unzipLocation.GetFullPath() + entry->GetName());
			if(!expectedFile.CmpNoCase(entryName.GetFullName()))
			{
				foundExpectedFile = true;
				expectedFile = entryName.GetFullPath();
				break;
			}
		}
	} while (entry.get());
	
	if(!foundExpectedFile)
		return false;

	// Unzip and extract all files
	do 
	{
		entry.reset(zip.GetNextEntry());
		if (entry.get())
		{
			// Get entries in the zip file and unzip them according to file type
			wxFileName zipFile(unzipLocation.GetFullPath() + entry->GetName());
			if (entry->IsDir())
			{
				// if the directory already exists then try and remove it
				if (zipFile.DirExists())
					// remove all the files in it
					wxDir(zipFile.GetPath()).Traverse(RecursiveDeleteTraverser());					

				// create the directory
				wxFileName::Mkdir(zipFile.GetPath(), wxS_DEFAULT, wxPATH_MKDIR_FULL);
			}
			else
			{
				// Work around since sometimes the top level folder in the zip file will not be unzipped
				// Ex: test.zip\toplevelfolder\info.txt -> toplevelfolder will not be unzipped, it will skip to info.txt
				if (!zipFile.DirExists())
					wxFileName::Mkdir(zipFile.GetPath(), wxS_DEFAULT, wxPATH_MKDIR_FULL);
				
				// Zip file entry is a file, not a folder
				off_t size = entry->GetSize();
				if (size && !entry->GetName().IsEmpty())
				{
					// Read the bytes into the buffer and ensure that the number of bytes read was equal to the size of the file
					char * buffer = new char[size];
					bool result = (size == zip.Read(buffer, size).LastRead());
					if (result)
					{
						// Write the file (overwritting if existing)
						wxFile newFile;
						if (newFile.Create(zipFile.GetFullPath().c_str(), true))
						{
							size_t sizeWritten = newFile.Write((void *)buffer, size);
							newFile.Close();
							result = (size == sizeWritten);
						}
					}

					if (!result)
					{
						wxString errorStr(wxT("Could not extract file:\n"));
						errorStr += entry->GetName();
						errorStr += wxT("\nFrom archive:\n");
						errorStr += fileToUnzip.GetFullPath();
						wxMessageBox(wxGetTranslation(errorStr), wxGetTranslation(wxT("Uh oh! Error extracting Theme")), wxICON_EXCLAMATION | wxCENTER);
						return false;
					}

					delete [] buffer;
				}
			}
		}
	} while (entry.get());

	return foundExpectedFile;
}


// NOTE: When starting up settings, all checkboxes are initially set to true and must populate their values by syncing with the settings.json file
// This occurs when fromSettingsFileToDialog is set to true
// After this first pass through, the check boxes will contain the same values as the settings.json file
// When saving the new settings.json file, it must check to see what has changed in order to set flags such as needsRestart
void SettingsApp::syncSettings( bool fromSettingsFileToDialog, Json::Value& root)
{
	// disable certain controls if there is no hwnd associated with this settings to call back to
	_handler->OnWindowCheckTimer(wxTimerEvent());

	// general tab
	{
		// update the languages
		wxString language = wxString(root["languageOverride"].asString().c_str(), wxConvUTF8);
		_handler->OnUpdateLanguagesListing(language, fromSettingsFileToDialog);
		if (!fromSettingsFileToDialog)
		{
			root["languageOverride"] = string(language.mbc_str());
		}
		setSavedString("language.active", language);
		if (fromSettingsFileToDialog)
			setSavedString("notify.bt", 1);

		// check if any of the startup folders contains the bumptop link
		wxCheckBox * cb = GetCtrlHelper(ID_LAUNCHONSTARTUP, wxCheckBox);
		wxFileName startupLink(_paths.getPath(Win32Path::StartupLink));
		wxFileName commonStartupLink(_paths.getPath(Win32Path::CommonStartupLink));
		bool hasStartupLink = startupLink.FileExists() || commonStartupLink.FileExists();
		if (fromSettingsFileToDialog)
		{
			cb->SetValue(hasStartupLink);
		}
		else
		{
			if (hasStartupLink && cb->GetValue())
			{
				// do nothing, since the settings already match
			}
			else
			{
				if (cb->GetValue())
				{
					// add startup link 
					Settings->getUtil().createUserStartupLink();
				}
				else
				{
					// remove the startup link
					::wxRemoveFile(startupLink.GetFullPath());
					::wxRemoveFile(commonStartupLink.GetFullPath());
				}
			}
		}

		// Sync Library Overlay setting
		wxCheckBox * enableLibraryOverlayCheckBox = GetCtrlHelper(ID_LIBRARYOVERLAY, wxCheckBox);
		if(!fromSettingsFileToDialog && root["enableLibraryOverlay"].asBool() != enableLibraryOverlayCheckBox->GetValue())
			markNeedsRestart();
		syncValue(fromSettingsFileToDialog, root["enableLibraryOverlay"], enableLibraryOverlayCheckBox);

		// Update Texture Quality
		int textureSetting = root["visuals"].asInt();
		_handler->OnUpdateTextureSettings(textureSetting, fromSettingsFileToDialog);
		if (!fromSettingsFileToDialog)
		{
			if(root["visuals"].asInt() != textureSetting)
				markNeedsRestart();
			root["visuals"] = textureSetting;
		}
		else
		{
			wxChoice *textureList = GetCtrlHelper(ID_TEXTURE_LIST, wxChoice);
			textureList->AddPendingEvent(wxCommandEvent(wxEVT_COMMAND_CHOICE_SELECTED, textureList->GetId()));
			textureList->ProcessEvent(wxCommandEvent(wxEVT_COMMAND_CHOICE_SELECTED, textureList->GetId()));
		}
	}

	// icons and physics tab
	{
		syncValue(fromSettingsFileToDialog, root["launchFoldersAsInGrid"], GetCtrlHelper(ID_LAUNCHFOLDERSASGRID, wxCheckBox));
		syncValue(fromSettingsFileToDialog, root["enableTossing"], GetCtrlHelper(ID_ENABLETOSSING, wxCheckBox));

		// the rotation limits spinner should reflect the enabled state
		syncValue(fromSettingsFileToDialog, root["RotationLimitDegrees"], GetCtrlHelper(ID_ROTATIONLIMIT, wxSlider));
		_handler->OnRotationLimitDegreesChange(wxCommandEvent());
	}

	// visuals and graphics tab
	{
		syncValue(fromSettingsFileToDialog, root["useThemeIconOverrides"], GetCtrlHelper(ID_OVERRIDEOSICONS, wxCheckBox), true);
		syncValue(fromSettingsFileToDialog, root["RenderText"], GetCtrlHelper(ID_DRAWTEXT, wxCheckBox));
		syncValue(fromSettingsFileToDialog, root["showIconExtensions"], GetCtrlHelper(ID_DRAWEXTENSIONS, wxCheckBox));
		syncValue(fromSettingsFileToDialog, root["manualArrowOverlay"], GetCtrlHelper(ID_DRAWLINKOVERLAY, wxCheckBox));
		syncValue(fromSettingsFileToDialog, root["showToolTips"], GetCtrlHelper(ID_SHOWTOOLTIPS, wxCheckBox));
		_handler->OnToggleFilenames(wxCommandEvent());

		// Photo Frame settings
		syncValue(fromSettingsFileToDialog, root["photoFrameImageDuration"], GetCtrlHelper(ID_PHOTOFRAME_IMAGEUPDATE, wxSpinCtrl));
		syncValue(fromSettingsFileToDialog, root["photoFrameSourceDuration"], GetCtrlHelper(ID_PHOTOFRAME_SOURCEUPDATE, wxSpinCtrl));
		
		wxCheckBox * disablePhotoFramesCheckBox = GetCtrlHelper(ID_DISABLE_PHOTOFRAMES_ON_BATTERY, wxCheckBox);
		if(!fromSettingsFileToDialog && root["disablePhotoframesOnBattery"].asBool() != disablePhotoFramesCheckBox->GetValue())
			markNeedsRestart();
		syncValue(fromSettingsFileToDialog, root["disablePhotoframesOnBattery"], disablePhotoFramesCheckBox);
	
		// Update the camera
		wxString cameraPreset = wxString(root["cameraPreset"].asString().c_str(), wxConvUTF8);
		_handler->OnUpdateCameraPresets(cameraPreset, fromSettingsFileToDialog);
		if (!fromSettingsFileToDialog && !cameraPreset.IsEmpty())
		{
			root["cameraPreset"] = string(cameraPreset.mbc_str());
		}
	}

	// themes and wallpapers tab
	{
		if (fromSettingsFileToDialog)
		{
			// find the current theme
			wxFileName userDefaultTheme(_paths.getPath(Win32Path::UserDefaultThemeFile));
			_handler->OnUpdateThemesListing(&userDefaultTheme, true);

			Json::Value themeRoot;
			if (SettingsApp::loadJSONFile(userDefaultTheme, themeRoot))
			{
				// save old values for checking later
				_savedVars["useWindowsBackgroundImage"] = root["useWindowsBackgroundImage"];
				_savedVars["textures.floor.desktop"] = themeRoot["textures"]["floor"]["desktop"];
				_savedVars["textures.wall.top"] = themeRoot["textures"]["wall"]["top"];
				_savedVars["textures.wall.left"] = themeRoot["textures"]["wall"]["left"];
				_savedVars["textures.wall.right"] = themeRoot["textures"]["wall"]["right"];
				_savedVars["textures.wall.bottom"] = themeRoot["textures"]["wall"]["bottom"];
				_savedVars["textures.wall.allowOverlay"] = themeRoot["textures"]["wall"]["allowOverlay"];

				// update the button text
				wxString tmp;
				_savedVars["fonts.override.default"] = themeRoot["ui"]["icon"]["font"]["family"][(unsigned int) 0];
				_savedVars["fonts.override.default.size"] = themeRoot["ui"]["icon"]["font"]["size"];
				tmp.Clear();
				tmp << getSavedString("fonts.override.default") 
					<< L", " << getSavedStringAsInt("fonts.override.default.size")
					<< L"pt";
				GetCtrlHelper(ID_DEFAULT_FONT_OVERRIDE, wxButton)->SetLabel(wxGetTranslation(wxT("Icons")));
				_savedVars["fonts.override.stickyNote"] = themeRoot["ui"]["stickyNote"]["font"]["family"][(unsigned int) 0];
				_savedVars["fonts.override.stickyNote.size"] = themeRoot["ui"]["stickyNote"]["font"]["size"];
				tmp.Clear();
				tmp << getSavedString("fonts.override.stickyNote") 
					<< L", " << getSavedStringAsInt("fonts.override.stickyNote.size")
					<< L"pt";
				GetCtrlHelper(ID_STICKY_FONT_OVERRIDE, wxButton)->SetLabel(wxGetTranslation(wxT("Sticky Notes")));
			}	
		}
		else
		{
			// sync all settings into the default theme (it was made active when 
			// handling the apply button)
			wxFileName defaultThemePath(_paths.getPath(Win32Path::UserDefaultThemeFile));
			Json::Value themeRoot;
			if (SettingsApp::loadJSONFile(defaultThemePath, themeRoot))
			{
				wxFileName path(defaultThemePath);
				path.SetFullName(wxEmptyString);

				// check for changes to the themes
				bool reloadTheme = _handler->hasThemeChanged(true);
				if (!reloadTheme)
				{
					// check if any of the values have changed
					Json::Value tmpValue;
					syncValue(fromSettingsFileToDialog, tmpValue["useWindowsBackgroundImage"], GetCtrlHelper(ID_USEWINDOWSBACKGROUND, wxCheckBox));
					syncTextureToValue(tmpValue["textures.floor.desktop"], path, GetCtrlHelper(ID_FLOORWALLPAPER, wxFilePickerCtrl));
					syncTextureToValue(tmpValue["textures.wall.top"], path, GetCtrlHelper(ID_TOPWALLPAPER, wxFilePickerCtrl));
					syncTextureToValue(tmpValue["textures.wall.left"], path, GetCtrlHelper(ID_LEFTWALLPAPER, wxFilePickerCtrl));
					syncTextureToValue(tmpValue["textures.wall.right"], path, GetCtrlHelper(ID_RIGHTWALLPAPER, wxFilePickerCtrl));
					syncTextureToValue(tmpValue["textures.wall.bottom"], path, GetCtrlHelper(ID_BOTTOMWALLPAPER, wxFilePickerCtrl));

					reloadTheme = 
						(tmpValue["useWindowsBackgroundImage"].asBool() != root["useWindowsBackgroundImage"].asBool()) ||
						(tmpValue["textures.floor.desktop"] != themeRoot["textures"]["floor"]["desktop"]) ||
						(tmpValue["textures.wall.top"] != themeRoot["textures"]["wall"]["top"]) ||
						(tmpValue["textures.wall.left"] != themeRoot["textures"]["wall"]["left"]) ||
						(tmpValue["textures.wall.right"] != themeRoot["textures"]["wall"]["right"]) ||
						(tmpValue["textures.wall.bottom"] != themeRoot["textures"]["wall"]["bottom"]);
				}	

				// update the wallpaper/background entries
				if (!GetCtrlHelper(ID_USEWINDOWSBACKGROUND, wxCheckBox)->GetValue())
				{
					wxFileName path(defaultThemePath);
					path.SetFullName(wxEmptyString);
					composePath(composePath(path, themeRoot["textures"]["relativeRoot"]), themeRoot["textures"]["floor"]["relativeRoot"]);
					syncTextureToValue(themeRoot["textures"]["floor"]["desktop"], path, GetCtrlHelper(ID_FLOORWALLPAPER, wxFilePickerCtrl));
				}
				composePath(composePath(path, themeRoot["textures"]["relativeRoot"]), themeRoot["textures"]["wall"]["relativeRoot"]);
				syncTextureToValue(themeRoot["textures"]["wall"]["top"], path, GetCtrlHelper(ID_TOPWALLPAPER, wxFilePickerCtrl));
				syncTextureToValue(themeRoot["textures"]["wall"]["left"], path, GetCtrlHelper(ID_LEFTWALLPAPER, wxFilePickerCtrl));
				syncTextureToValue(themeRoot["textures"]["wall"]["right"], path, GetCtrlHelper(ID_RIGHTWALLPAPER, wxFilePickerCtrl));
				syncTextureToValue(themeRoot["textures"]["wall"]["bottom"], path, GetCtrlHelper(ID_BOTTOMWALLPAPER, wxFilePickerCtrl));

				// set the fonts
				Json::Value defFontArray = themeRoot["ui"]["icon"]["font"]["family"];
				Json::Value newFontArray;
				newFontArray.append(_savedVars["fonts.override.default"]);
				for (unsigned int i = 0; i < defFontArray.size(); ++i)
				{
					if (_savedVars["fonts.override.default"] != defFontArray[i])
						newFontArray.append(defFontArray[i]);
				}
				themeRoot["ui"]["icon"]["font"]["family"] = newFontArray;
				themeRoot["ui"]["icon"]["font"]["size"] = getSavedStringAsInt("fonts.override.default.size");

				Json::Value stickyFontArray = themeRoot["ui"]["stickyNote"]["font"]["family"];
				Json::Value newStickyFontArray;
				newStickyFontArray.append(_savedVars["fonts.override.stickyNote"]);
				for (unsigned int i = 0; i < stickyFontArray.size(); ++i)
				{
					if (_savedVars["fonts.override.stickyNote"] != stickyFontArray[i])
						newStickyFontArray.append(stickyFontArray[i]);
				}
				themeRoot["ui"]["stickyNote"]["font"]["family"] = newStickyFontArray;
				themeRoot["ui"]["stickyNote"]["font"]["size"] = getSavedStringAsInt("fonts.override.stickyNote.size");

				// save the theme
				_themes.saveTheme(defaultThemePath, themeRoot);

				// reload in bt if necessary
				if (reloadTheme && (Settings->getSavedStringAsInt("notify.bt")  > 0))
					_bumptopComm.sendMessage(Win32Comm::ReloadTheme, true);
			}
		}

		// load the theme file, and sync that to the wallpaper values here
		syncValue(fromSettingsFileToDialog, root["useWindowsBackgroundImage"], GetCtrlHelper(ID_USEWINDOWSBACKGROUND, wxCheckBox));
		_handler->OnToggleFloorOverride(wxCommandEvent());
	}
	// advanced tab	
	{
		// Sync Infinite Desktop
		if (fromSettingsFileToDialog)
		{
			// check with bumptop to see if the infinite desktop is enabled
			GetCtrlHelper(ID_ENABLEINFINITEDESKTOP, wxCheckBox)->SetValue(_bumptopComm.sendMessage(Win32Comm::IsInfiniteDesktopModeEnabled, true));
		}
		else
		{
			// tell bumptop whether to enable the infinite desktop
			_bumptopComm.sendMessage(Win32Comm::ToggleInfiniteDesktopMode, GetCtrlHelper(ID_ENABLEINFINITEDESKTOP, wxCheckBox)->GetValue());
		}

		// Sync Windows Thumbs 
		wxCheckBox * useThumbsDBCheckBox = GetCtrlHelper(ID_USETHUMBSDB, wxCheckBox);
		if(!fromSettingsFileToDialog && root["useThumbsDb"].asBool() != useThumbsDBCheckBox->GetValue())
			markNeedsRestart();
		syncValue(fromSettingsFileToDialog, root["useThumbsDb"], useThumbsDBCheckBox);

		// Sync Anti aliasing setting
		wxCheckBox * enableAntiAliasingCheckBox = GetCtrlHelper(ID_ENABLEANTIALIASING, wxCheckBox);
		if(!fromSettingsFileToDialog && root["useAntiAliasing"].asBool() != enableAntiAliasingCheckBox->GetValue())
			markNeedsRestart();
		syncValue(fromSettingsFileToDialog, root["useAntiAliasing"], enableAntiAliasingCheckBox);

		// Sync Power Level
		syncValue(fromSettingsFileToDialog, root["disableAnimationsOnBattery"], GetCtrlHelper(ID_DISABLE_ANIMATIONS_ON_BATTERY, wxCheckBox));

		// Sync networking settings
		bool isAutoProxy = false;
		if (fromSettingsFileToDialog)
		{
			isAutoProxy = !root["overrideIEProxySettings"].asBool();
			GetCtrlHelper(ID_AUTOPROXY_RADIO, wxRadioButton)->SetValue(isAutoProxy);
			GetCtrlHelper(ID_MANUALPROXY_RADIO, wxRadioButton)->SetValue(!isAutoProxy);
			_savedVars["httpProxyUrl"] = root["httpProxyUrl"];
			_handler->OnToggleManualProxyOverrides(wxCommandEvent());
		}
		else
		{
			isAutoProxy = GetCtrlHelper(ID_AUTOPROXY_RADIO, wxRadioButton)->GetValue();
			root["overrideIEProxySettings"] = !isAutoProxy;
			if (!isAutoProxy)
			{
				syncValue(fromSettingsFileToDialog, root["httpProxyUrl"], GetCtrlHelper(ID_HTTPPROXY, wxTextCtrl));
			}
		}
	}
	// pro tab
	{
		if (fromSettingsFileToDialog)
		{
			string s = root["proInviteCode"].asString();
			if (!root["proInviteCode"].asString().empty())
			{
				wxPanel *authorize = GetCtrlHelper(ID_AUTHORIZEPRO, wxPanel);
				wxPanel *deauthorize = GetCtrlHelper(ID_DEAUTHORIZEPRO, wxPanel);

				authorize->Hide();
				deauthorize->Show(true);
				deauthorize->SetPosition(wxPoint(145, 89));

				wxStaticText *pro_key = GetCtrlHelper(ID_PROKEY, wxStaticText);
				string buttonLabel = string("Authorized: ") + root["proInviteCode"].asString();
				pro_key->SetLabel(wxString::FromAscii(buttonLabel.c_str()));
			}
			else
			{
				wxPanel *authorize = GetCtrlHelper(ID_AUTHORIZEPRO, wxPanel);
				wxPanel *deauthorize = GetCtrlHelper(ID_DEAUTHORIZEPRO, wxPanel);

				deauthorize->Hide();
				authorize->Show(true);
			}
	
			if (!_proUrl.IsEmpty())
				GetCtrlHelper(ID_BUMPTOPPPROURL, wxHyperlinkCtrl)->SetURL(_proUrl);
		}
	}

	// admin tab
	{
		// hide the admin panel if necessary
		if (fromSettingsFileToDialog && !_isAdmin && !_adminTabDeleted)
		{
			_adminTabDeleted = true;
			GetWXControl(ID_NOTEBOOK, wxNotebook, _dialog)->RemovePage(4);
		}

		if (_isAdmin)
		{
			syncValue(fromSettingsFileToDialog, root["showConsoleWindow"], GetCtrlHelper(ID_SHOWCONSOLE, wxCheckBox));
		}
	}

	// pro tab
	{
		// hide the pro panel if necessary
		//if (fromSettingsFileToDialog)
		//	GetWXControl(ID_NOTEBOOK, wxNotebook, _dialog)->RemovePage(_adminTabDeleted ? 4 : 5);
	}

	// about tab
	{
		wxString revStr = wxString::Format(wxT("r%s"), wxString(SVN_VERSION_NUMBER, wxConvUTF8).c_str());		
		GetCtrlHelper(ID_VERSIONSTATUS, wxStaticText)->SetLabel(revStr);
	}
	// misc
	{		
		if (fromSettingsFileToDialog)
		{
			// hide the throbber
			Settings->getComm().getApplyThrobberTimer().Connect(wxID_ANY, wxEVT_TIMER, wxTimeEventHandler(SettingsAppEventHandler::OnApplyThrobberTimer), 0, _handler);
			Settings->getComm().getApplyThrobberTimer().Start(1);
		}
	}
}

void SettingsApp::bindSettingsEvents()
{
#define BindHelper(id, ctrlType, evtType, func) \
	BindWXEvent(GetWXControl(id, ctrlType, _dialog), evtType, _handler, func)

	// bottom buttons
	BindHelper(ID_RESETSETTINGS, wxButton, wxEVT_COMMAND_BUTTON_CLICKED, OnResetSettings);
	BindHelper(ID_SAVESETTINGS, wxButton, wxEVT_COMMAND_BUTTON_CLICKED, OnSaveSettings);
	BindHelper(ID_CANCEL, wxButton, wxEVT_COMMAND_BUTTON_CLICKED, OnCancel);

	// general tab
	{
		BindHelper(ID_RESETLAYOUT, wxButton, wxEVT_COMMAND_BUTTON_CLICKED, OnResetBumpTopLayout);
		BindHelper(ID_CLEARCACHE, wxButton, wxEVT_COMMAND_BUTTON_CLICKED, OnClearCache);
		BindHelper(ID_TEXTURE_LIST, wxChoice, wxEVT_COMMAND_CHOICE_SELECTED, OnTextureLevelChange); // handles when user clicks next to thumb track OR uses arrow keys to move thumb track
	}

	// icons and physics tab
	{
		BindHelper(ID_ROTATIONLIMIT, wxSlider, wxEVT_SCROLL_THUMBRELEASE, OnRotationLimitDegreesChange);
	}

	// visuals tab
	{
		BindHelper(ID_DRAWTEXT, wxCheckBox, wxEVT_COMMAND_CHECKBOX_CLICKED, OnToggleFilenames);
		BindHelper(ID_CYCLEMONITOR, wxButton, wxEVT_COMMAND_BUTTON_CLICKED, OnNextMonitor);
	}

	// themes and wallpapers tab
	{
		BindHelper(ID_THEMESLIST, wxChoice, wxEVT_COMMAND_CHOICE_SELECTED, OnThemeSelected);
		BindHelper(ID_GETMORETHEMES, wxButton, wxEVT_COMMAND_BUTTON_CLICKED, OnGetMoreThemesSelected);
		BindHelper(ID_RELOADTHEMES, wxBitmapButton, wxEVT_COMMAND_BUTTON_CLICKED, OnReloadThemes);
		BindHelper(ID_UPLOADTHEME, wxButton, wxEVT_COMMAND_BUTTON_CLICKED, OnUploadTheme);
		BindHelper(ID_USEWINDOWSBACKGROUND, wxCheckBox, wxEVT_COMMAND_CHECKBOX_CLICKED, OnToggleFloorOverride);
		
		// NOTE: the wx file picker controls are wonky, since the text does not set the file path...
		BindWXEvent(GetWXControl(ID_FLOORWALLPAPER, wxFilePickerCtrl, _dialog)->GetTextCtrl(), wxEVT_KILL_FOCUS , _handler, OnFilePickerTextCtrlChanged);
		BindWXEvent(GetWXControl(ID_TOPWALLPAPER, wxFilePickerCtrl, _dialog)->GetTextCtrl(), wxEVT_KILL_FOCUS , _handler, OnFilePickerTextCtrlChanged);
		BindWXEvent(GetWXControl(ID_LEFTWALLPAPER, wxFilePickerCtrl, _dialog)->GetTextCtrl(), wxEVT_KILL_FOCUS , _handler, OnFilePickerTextCtrlChanged);
		BindWXEvent(GetWXControl(ID_RIGHTWALLPAPER, wxFilePickerCtrl, _dialog)->GetTextCtrl(), wxEVT_KILL_FOCUS , _handler, OnFilePickerTextCtrlChanged);
		BindWXEvent(GetWXControl(ID_BOTTOMWALLPAPER, wxFilePickerCtrl, _dialog)->GetTextCtrl(), wxEVT_KILL_FOCUS , _handler, OnFilePickerTextCtrlChanged);

		BindHelper(ID_DEFAULT_FONT_OVERRIDE, wxButton, wxEVT_COMMAND_BUTTON_CLICKED, OnOverrideDefaultFont);
		BindHelper(ID_STICKY_FONT_OVERRIDE, wxButton, wxEVT_COMMAND_BUTTON_CLICKED, OnOverrideStickyNoteFont);		
	}

	// Advanced Tab
	{
		BindHelper(ID_ENABLEINFINITEDESKTOP, wxCheckBox, wxEVT_COMMAND_CHECKBOX_CLICKED, OnToggleInfiniteDesktop);
		BindHelper(ID_AUTOPROXY_RADIO, wxRadioButton, wxEVT_COMMAND_RADIOBUTTON_SELECTED, OnToggleManualProxyOverrides);
		BindHelper(ID_MANUALPROXY_RADIO, wxRadioButton, wxEVT_COMMAND_RADIOBUTTON_SELECTED, OnToggleManualProxyOverrides);
		BindHelper(ID_LAUNCHIEPROXYDIALOG, wxButton, wxEVT_COMMAND_BUTTON_CLICKED, OnLaunchIEProxySettings);	
	}
	// pro tab
	{
		BindHelper(ID_AUTHORIZEPROKEY, wxButton, wxEVT_COMMAND_BUTTON_CLICKED, OnAuthorizeProKey);
		BindHelper(ID_DEAUTHORIZEPROKEY, wxButton, wxEVT_COMMAND_BUTTON_CLICKED, OnDeauthorizeProKey);
	}

	// about tab
	{
		BindHelper(ID_SENDFEEDBACK, wxButton, wxEVT_COMMAND_BUTTON_CLICKED, OnSendFeedback);
	}
}

bool SettingsApp::loadSettingsFile( Json::Value& root )
{
	Json::Reader reader;
	string line;
	ostringstream settingsStr;
	ifstream ifstr(_settingsFile.GetFullPath().c_str(), ifstream::in);
	while (getline(ifstr, line))
		settingsStr << line << endl;
	ifstr.close();

	// load the description file
	return reader.parse(settingsStr.str(), root);
}

bool SettingsApp::loadJSONFile( const wxFileName& file, Json::Value& root )
{
	if (file.FileExists())
	{
		Json::Reader reader;
		string line;
		ostringstream settingsStr;
		ifstream ifstr(file.GetFullPath().c_str(), ifstream::in);
		while (getline(ifstr, line))
			settingsStr << line << endl;
		ifstr.close();

		// load the description file
		return reader.parse(settingsStr.str(), root);
	}
	return false;
}

bool SettingsApp::setLocale(const wxString& langId)
{
	const wxLanguageInfo * langInfo = wxLocale::FindLanguageInfo(langId);
	if (langInfo && wxLocale::IsAvailable(langInfo->Language))
	{
		wxLocale * locale = new wxLocale(langInfo->Language, wxLOCALE_CONV_ENCODING);
		locale->AddCatalogLookupPathPrefix(_paths.getPath(Win32Path::LanguagesDir));

		wxString catalog = wxString::Format(wxT("Settings_%s"), langId.c_str());
		return locale->AddCatalog(catalog);
	}
	return false;
}

void SettingsApp::saveSettingsFile( const Json::Value& root )
{
	setLocale(wxT("en"));
	Json::StyledWriter jsonWriter;
	std::string outStr = jsonWriter.write(root);
	ofstream ofstr(_settingsFile.GetFullPath().c_str(), ofstream::out);
	ofstr << outStr;
	ofstr.close();
	if (!_language.IsEmpty())
		setLocale(_language);

}

wxString SettingsApp::getPath( Win32Path::Win32PathKey key )
{
	return _paths.getPath(key);
}

Win32Util& SettingsApp::getUtil()
{
	return _util;
}

Win32Comm& SettingsApp::getComm()
{
	return _bumptopComm;
}

void SettingsApp::markDirty()
{
	setSavedString("outstandingChanges", 1);
}

void SettingsApp::markClean()
{
	setSavedString("outstandingChanges", 0);
}

bool SettingsApp::isDirty()
{
	return (getSavedStringAsInt("outstandingChanges") > 0);
}

void SettingsApp::markNeedsRestart()
{
	setSavedString("changesRequireRestart", 1);
}

void SettingsApp::unmarkNeedsRestart()
{
	setSavedString("changesRequireRestart", 0);
}

bool SettingsApp::needsRestart()
{
	return (getSavedStringAsInt("changesRequireRestart") > 0);
}

void SettingsApp::markSettingsApplied()
{
	setSavedString("settingsHaveBeenApplied", 1);
}

bool SettingsApp::isSettingsApplied()
{
	return (getSavedStringAsInt("settingsHaveBeenApplied") > 0);
}

wxString SettingsApp::getSavedString( const string& key )
{
	return wxString(_savedVars[key].asString().c_str(), wxConvUTF8);
}

int SettingsApp::getSavedStringAsInt( const string& key )
{
	int i = _savedVars[key].asInt();
	return i;
}

void SettingsApp::setSavedString( const string& key, const wxString& str )
{
	_savedVars[key] = string(str.mbc_str());
}

void SettingsApp::setSavedString(const string& key, int val)
{
	_savedVars[key] = val;
}

