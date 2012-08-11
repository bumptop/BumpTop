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
#include "BT_SettingsAppEventHandler.h"

#ifdef WIN32	
	#include "BT_Win32Path.h"
#endif

// helper macro
#define GetCtrlHelper(id, ctrlType) \
	GetWXControl(id, ctrlType, Settings->getDialog())


SettingsAppEventHandler::SettingsAppEventHandler()
: _reloadTheme(false)
{
	_dialogSlideTimer.Connect(wxID_ANY, wxEVT_TIMER, wxTimeEventHandler(SettingsAppEventHandler::OnDialogSlideTimer), 0, this);

	// Unicode converter http://rishida.net/tools/conversion/
	_languages.push_back(make_pair(wxT(""), wxT("Auto Detect")));
	_languages.push_back(make_pair(wxT("zh_cn"), wxT("\u4e2d\u6587(\u7b80\u4f53)"))); // Chinese(Simplified)
	_languages.push_back(make_pair(wxT("zh_tw"), wxT("\u4e2d\u6587(\u7e41\u9ad4)"))); // Chinese(Traditional)
	_languages.push_back(make_pair(wxT("en"), wxT("English")));
	_languages.push_back(make_pair(wxT("fr"), wxT("Français")));
	_languages.push_back(make_pair(wxT("de"), wxT("Deutsch")));
	_languages.push_back(make_pair(wxT("it"), wxT("Italiano")));
	_languages.push_back(make_pair(wxT("ja"), wxT("\u65e5\u672c\u8a9e"))); // Japanese 
	_languages.push_back(make_pair(wxT("ko"), wxT("\ud55c\uad6d\uc5b4"))); // Korean
	_languages.push_back(make_pair(wxT("pt"), wxT("Português")));
	_languages.push_back(make_pair(wxT("ru"), wxT("\u0420\u0443\u0441\u0441\u043a\u0438\u0439"))); // Russian
	_languages.push_back(make_pair(wxT("es"), wxT("Español")));

	_cameraAngles.push_back(make_pair(wxT("def"), wxGetTranslation(wxT("Default"))));
	_cameraAngles.push_back(make_pair(wxT("oh"), wxGetTranslation(wxT("Top Down"))));
	_cameraAngles.push_back(make_pair(wxT("brc"), wxGetTranslation(wxT("Bottom Right Corner"))));

	_textureSettings.push_back(make_pair(1, wxGetTranslation(wxT("Low resolution textures"))));
	_textureSettings.push_back(make_pair(2, wxGetTranslation(wxT("Standard resolution textures"))));
	_textureSettings.push_back(make_pair(3, wxGetTranslation(wxT("High resolution textures"))));
}

bool SettingsAppEventHandler::hasThemeChanged(bool resetAfterQuery)
{
	bool result = _reloadTheme;
	if (resetAfterQuery)
		_reloadTheme = false;
	return result;
}

void SettingsAppEventHandler::OnWindowCheckTimer(wxTimerEvent& evt)
{
	// check if the bumptop window is still open
	if (!Settings->getComm().getWindowHandle() || 
		!::IsWindow(Settings->getComm().getWindowHandle()))
	{
		// stop the timer
		Settings->getComm().getWindowCheckTimer().Stop();

		// notify user if previously the bt window was open
		if (Settings->getComm().getWindowHandle())
		{
			// bumptopsettings.OnWindowCheckTimerClosed.caption
			// bumptopsettings.OnWindowCheckTimerClosed.text
			wxMessageBox(wxGetTranslation(wxT("Uh oh! The BumpTop window that this Settings dialog is editing has closed. Certain changes will not be applied and have been disabled.")), 
				wxGetTranslation(wxT("BumpTop Closed")), wxICON_EXCLAMATION | wxCENTER);
		}

		// and disable all requisite controls
		GetCtrlHelper(ID_RESETLAYOUT, wxButton)->Disable();
		GetCtrlHelper(ID_ENABLEINFINITEDESKTOP, wxCheckBox)->Disable();
		GetCtrlHelper(ID_CHECKFORUPDATES, wxButton)->Disable();
		// GetCtrlHelper(ID_SENDFEEDBACK, wxButton)->Disable();
		GetCtrlHelper(ID_VERSIONSTATUSICON, wxStaticBitmap)->Disable();
		GetCtrlHelper(ID_AUTHORIZEPROKEY, wxButton)->Disable();
		GetCtrlHelper(ID_DEAUTHORIZEPROKEY, wxButton)->Disable();
	}
}

void SettingsAppEventHandler::OnApplyThrobberTimer(wxTimerEvent& evt)
{
	// hide the throbber
	wxAnimationCtrl * animCtrl = GetCtrlHelper(ID_THROBBER, wxAnimationCtrl);
	animCtrl->Show(false);
	animCtrl->Stop();
	
	// hide the label
	wxStaticText * animCtrlLabel = GetCtrlHelper(ID_THROBBER_LABEL, wxStaticText);
	animCtrlLabel->Show(false);
}

void SettingsAppEventHandler::OnResetSettings( wxCommandEvent& evt )
{
	// to do, requires copying a default settings file or something...	
}

void SettingsAppEventHandler::OnApplyCurrentTheme()
{
}

// Executes when apply button is pressed or user confirms apply when closing dialog box
void SettingsAppEventHandler::OnSaveSettings( wxCommandEvent& evt )
{
	// start the loading animation control
	wxAnimationCtrl * animCtrl = GetCtrlHelper(ID_THROBBER, wxAnimationCtrl);
	animCtrl->Show(true);
	animCtrl->Play();
	wxStaticText * animCtrlLabel = GetCtrlHelper(ID_THROBBER_LABEL, wxStaticText);
	animCtrlLabel->Show(true);
	// set delay to stop throbber
	Settings->getComm().getApplyThrobberTimer().Connect(wxID_ANY, wxEVT_TIMER, wxTimeEventHandler(SettingsAppEventHandler::OnApplyThrobberTimer), 0, this);
	Settings->getComm().getApplyThrobberTimer().Start(3000);

	// check if the languages were updated
	bool languageUpdated = false;
	wxChoice * langChoice = GetCtrlHelper(ID_LANGUAGE_LIST, wxChoice);
	int sel = langChoice->GetSelection();
	if (sel != wxNOT_FOUND)
	{
		wxString language = Settings->getSavedString("language.active");
		wxString selLang = _languages[sel].first;
		if (language != selLang)
		{
			languageUpdated = true;
		}
	}
	
	// determine which theme was selected and activate it
	wxChoice * themesListingCb = GetCtrlHelper(ID_THEMESLIST, wxChoice);
	if (themesListingCb->IsEnabled())
	{
		wxFileName * themePath = (wxFileName *)themesListingCb->GetClientData(themesListingCb->GetSelection());
		if (themePath)
		{
			if (!_reloadTheme)
				_reloadTheme = (_previousTheme != *themePath);

			// make the specified theme the default theme so it can be referenced easier
			Settings->getThemes().activateTheme(*themePath);

			if (languageUpdated)
			{
				// prompt the user if they want to restart
				if (wxYES != wxMessageBox(wxGetTranslation(wxT("Please restart BumpTop to see your language changes.")),
					wxGetTranslation(wxT("Restart to change languages!")), wxOK | wxCENTER, Settings->getDialog()))
				{
					Settings->markNeedsRestart();
					Settings->setSavedString("notify.bt", 0);
				}
			}
		}
	}	

	Json::Value root;
	Settings->loadSettingsFile(root);
	Settings->syncSettings(false, root);
	Settings->saveSettingsFile(root);
	Settings->markSettingsApplied();
	Settings->markClean();
	// Settings->getDialog()->EndModal(0);

	// send message to bumptop to reload settings
	if (Settings->getSavedStringAsInt("notify.bt") > 0)
		Settings->getComm().sendMessage(Win32Comm::ReloadSettings, true);
}


void SettingsAppEventHandler::OnCancel(wxCommandEvent& evt)
{
	// Check if there are other changes that have yet to be applied
	bool isDirty = Settings->isDirty();
	if (!isDirty)
	{
		// Populates JSON value with settings.json file which contains the settings prior to opening the settings dialog
		Json::Value fileRoot;
		Settings->loadSettingsFile(fileRoot);
	
		// Populates JSON value with new settings options that the user has just set while viewing the settings dialog
		Json::Value existingValuesRoot = fileRoot;
		Settings->syncSettings(false, existingValuesRoot);

		// If the prior settings options are different to the newly set options, then the settings.json file must be updated
		isDirty = !subsetEqualsRec(existingValuesRoot, fileRoot);
	}

	if (isDirty)
	{
		// Prompt to apply the change
		if (wxYES == wxMessageBox(wxGetTranslation(wxT("You have outstanding changes to the settings, do you want to apply them?")),
			wxGetTranslation(wxT("To apply changes, or not to apply changes")), wxYES_NO | wxCENTER, Settings->getDialog()))
		{
			OnSaveSettings(evt);
		}
	}

	bool needsRestart = Settings->needsRestart();
	if(Settings->isSettingsApplied() && needsRestart)
	{
		if (wxYES == wxMessageBox(wxGetTranslation(wxT("Changes that you have applied require BumpTop to restart. Would you like to do that now?")),
			wxGetTranslation(wxT("To restart, or not to restart")), wxYES_NO | wxCENTER, Settings->getDialog()))
		{
			Settings->getComm().sendMessage(Win32Comm::SettingsRequireRestart, true);
		}
	}

	Settings->getDialog()->EndModal(0);
}

void SettingsAppEventHandler::OnResetBumpTopLayout( wxCommandEvent& evt )
{
	Settings->getComm().sendMessage(Win32Comm::ResetDesktopLayout, true);
}

void SettingsAppEventHandler::OnClearCache( wxCommandEvent& evt )
{
// 	// Determine path to cache directory
 	Win32Path win32Path;
 	wxString appDataPath = win32Path.getPath(Win32Path::AppDataDir);
 	wxFileName appDataDir= wxFileName(appDataPath);
 	appDataDir.AppendDir(wxT("Cache"));
 	wxString cacheDirectoryPath = appDataDir.GetFullPath();
 
 	// Create path directory
 	wxDir directory(cacheDirectoryPath);
	
	// Delete everything inside the cache directory
	if (appDataDir.DirExists(appDataDir.GetFullPath()))
		directory.Traverse(RecursiveDeleteTraverser());
}

// This function is to make sure the RotationLimitDegree slider is always on a tick
void SettingsAppEventHandler::OnRotationLimitDegreesChange( wxCommandEvent& evt )
{
	wxSlider *rotationSlider = GetCtrlHelper(ID_ROTATIONLIMIT, wxSlider);
	
	// Round the current value
	int tickFreq = rotationSlider->GetTickFreq();
	int currentVal = rotationSlider->GetValue();
	currentVal = (currentVal + tickFreq/2) / tickFreq;
	currentVal *= tickFreq;

	// Set the slider to the nearest 10
	rotationSlider->SetValue(currentVal);
}

void SettingsAppEventHandler::OnToggleFilenames( wxCommandEvent& evt )
{
	if (!GetCtrlHelper(ID_DRAWTEXT, wxCheckBox)->GetValue())
	{
		GetCtrlHelper(ID_DRAWEXTENSIONS, wxCheckBox)->Disable();
	}
	else
	{
		GetCtrlHelper(ID_DRAWEXTENSIONS, wxCheckBox)->Enable();
	}
}

void SettingsAppEventHandler::OnThemeSelected( wxCommandEvent& evt )
{
	wxChoice * cb = GetCtrlHelper(ID_THEMESLIST, wxChoice);
	wxFileName * fn = NULL;
	bool isDefaultTheme = false;
	if (cb->IsEnabled())
	{	
		fn = (wxFileName *)cb->GetClientData(cb->GetSelection());
		isDefaultTheme = !fn->GetFullPath().CmpNoCase(Settings->getPath(Win32Path::UserDefaultThemeFile));
	}
	else
		fn = new wxFileName();
	OnUpdateThemesListing(fn, false);
	if(!isDefaultTheme)
		Settings->markDirty();
	delete fn;
}

void SettingsAppEventHandler::OnGetMoreThemesSelected(wxCommandEvent& evt) {
	Settings->getComm().sendMessage(Win32Comm::StartCustomizeDialog, true);
	exit(0);
}

void SettingsAppEventHandler::OnUpdateThemesListing(wxFileName * selectedTheme, bool recordSelectedThemeAsPrevious)
{
	assert(selectedTheme);

	// save this theme as the previous if specified to do so
	if (recordSelectedThemeAsPrevious)
		_previousTheme = *selectedTheme;

	int selectIndex = 0;
	GetCtrlHelper(ID_THEMEINFO, wxStaticText)->SetLabel(wxGetTranslation(wxT("No Information Available")));

	wxChoice * themesChoice = GetCtrlHelper(ID_THEMESLIST, wxChoice);
	for (unsigned int i = 0; themesChoice->IsEnabled() && i < themesChoice->GetCount(); ++i)
	{
		void * p = themesChoice->GetClientData(i);
		if (p && p != selectedTheme)
			delete (wxFileName *) p;
	}
	themesChoice->Clear();

	const vector<wxFileName>& themes = Settings->getThemes().getThemes();
	if (themes.empty())
	{
		// disable all the theme controls?
		themesChoice->Append(wxGetTranslation(wxT("No Themes Found")));
		themesChoice->Disable();
	}
	else
	{
		themesChoice->Enable();
		for (unsigned int i = 0; i < themes.size(); ++i)
		{
			Json::Value themeRoot;
			if (SettingsApp::loadJSONFile(themes[i], themeRoot))
			{
				wxString name(themeRoot["header"]["name"].asString().c_str(), wxConvUTF8);
				bool isDefaultTheme = !themes[i].GetFullPath().CmpNoCase(Settings->getPath(Win32Path::UserDefaultThemeFile));
				if (isDefaultTheme)
					themesChoice->Append(wxT("Current Theme"), new wxFileName(themes[i]));
				else
					themesChoice->Append(name, new wxFileName(themes[i]));
				if (themes[i] == *selectedTheme)
				{
					selectIndex = i;
					
					// build the description
					wxString version(themeRoot["header"]["version"].asString().c_str(), wxConvUTF8);
					wxString author(themeRoot["header"]["author"].asString().c_str(), wxConvUTF8);
					wxString url(themeRoot["header"]["url"].asString().c_str(), wxConvUTF8);
					wxString desc(themeRoot["header"]["description"].asString().c_str(), wxConvUTF8);
					desc.Replace(wxT("\n"), wxT(" "));
					wxString info = (author.IsEmpty() || isDefaultTheme ? name : wxT("By ") + author) + wxT(" (") + version + wxT(")\n") + desc;
					GetCtrlHelper(ID_THEMEINFO, wxStaticText)->SetLabel(info);
					
					// update the wallpaper/background entries
					wxString bg(themeRoot["textures"]["floor"]["desktop"].asString().c_str(), wxConvUTF8);
					wxString topWall(themeRoot["textures"]["wall"]["top"].asString().c_str(), wxConvUTF8);
					wxString leftWall(themeRoot["textures"]["wall"]["left"].asString().c_str(), wxConvUTF8);
					wxString rightWall(themeRoot["textures"]["wall"]["right"].asString().c_str(), wxConvUTF8);
					wxString bottomWall(themeRoot["textures"]["wall"]["bottom"].asString().c_str(), wxConvUTF8);
					GetCtrlHelper(ID_FLOORWALLPAPER, wxFilePickerCtrl)->SetPath(bg);
					GetCtrlHelper(ID_TOPWALLPAPER, wxFilePickerCtrl)->SetPath(topWall);
					GetCtrlHelper(ID_LEFTWALLPAPER, wxFilePickerCtrl)->SetPath(leftWall);
					GetCtrlHelper(ID_RIGHTWALLPAPER, wxFilePickerCtrl)->SetPath(rightWall);
					GetCtrlHelper(ID_BOTTOMWALLPAPER, wxFilePickerCtrl)->SetPath(bottomWall);
				}
			}
		}
	}
	themesChoice->Select(selectIndex);	
}

void SettingsAppEventHandler::OnUpdateLanguagesListing(wxString& language, bool fromSettingsFileToDialog)
{
	wxChoice * langChoice = GetCtrlHelper(ID_LANGUAGE_LIST, wxChoice);
	if (fromSettingsFileToDialog)
	{
		language.MakeLower();
		if (langChoice->GetCount() == 0)
		{
			for (unsigned int i = 0; i < _languages.size(); ++i)
			{
				langChoice->Append(_languages[i].second);
				if (language == _languages[i].first)
					langChoice->Select(i);
			}
		}
		
		// select English if none was selected
		if (langChoice->GetSelection() == wxNOT_FOUND)
			langChoice->SetStringSelection(wxT("English"));
	}
	else
	{
		// backtrace the country/language id from the selection
		int sel = langChoice->GetSelection();
		if (sel != wxNOT_FOUND)
		{
			wxString selLang = _languages[sel].first;
			if (language != selLang)
			{
				language = selLang;
			}
		}
	}
}

void SettingsAppEventHandler::OnUpdateCameraPresets(wxString& cameraPreset, bool fromSettingsFileToDialog)
{
	wxChoice * presetChoice = GetCtrlHelper(ID_CAMERALIST, wxChoice);
	if (fromSettingsFileToDialog)
	{
		if (presetChoice->GetCount() == 0)
		{
			for (unsigned int i = 0; i < _cameraAngles.size(); ++i)
			{
				presetChoice->Append(_cameraAngles[i].second);
				if (cameraPreset == _cameraAngles[i].first)
					presetChoice->Select(i);
			}
		}

		// select default if none was selected
		if (presetChoice->GetSelection() == wxNOT_FOUND)
			presetChoice->SetSelection(0);
	}
	else
	{
		// Determine the camera preset based on the selection
		int sel = presetChoice->GetSelection();
		if (sel != wxNOT_FOUND)
		{
			wxString selCameraPreset = _cameraAngles[sel].first;
			if (cameraPreset != selCameraPreset)
			{
				cameraPreset = selCameraPreset;
			}
		}
	}
}

void SettingsAppEventHandler::OnUpdateTextureSettings(int& textureSetting, bool fromSettingsFileToDialog)
{
	wxChoice * textureChoice = GetCtrlHelper(ID_TEXTURE_LIST, wxChoice);
	if (fromSettingsFileToDialog)
	{
		if (textureChoice->GetCount() == 0)
		{
			for (unsigned int i = 0; i < _textureSettings.size(); ++i)
			{
				textureChoice->Append(_textureSettings[i].second);
				if (textureSetting == _textureSettings[i].first)
					textureChoice->Select(i);
			}
		}

		// select default if none was selected
		if (textureChoice->GetSelection() == wxNOT_FOUND)
			textureChoice->SetSelection(0);
	}
	else
	{
		// Determine the camera preset based on the selection
		int sel = textureChoice->GetSelection();
		if (sel != wxNOT_FOUND)
		{
			int selTextureSetting = _textureSettings[sel].first;
			if (textureSetting != selTextureSetting)
			{
				textureSetting = selTextureSetting;
			}
		}
	}
}

void SettingsAppEventHandler::OnReloadThemes( wxCommandEvent& evt )
{
	Settings->getThemes().clear();
	Settings->getThemes().load(Settings->getPath(Win32Path::UserThemesDir));
	Settings->getThemes().load(Settings->getPath(Win32Path::DefaultThemeDir));
	OnThemeSelected(wxCommandEvent());
}

/*
void SettingsAppEventHandler::OnApplySelectedTheme( wxCommandEvent& evt )
{
	// ask if the user wants to keep their desktop and walls (keep all, keep floor, keep walls) since the theme contains custom wallpapers
	// if the theme walls/floor is empty, do not merge? WAIT, only merge if the file being referenced exists, ie. if the theme does not want to
	// override a value, then set it as empty, and it will not change the existing theme?
}
*/

void SettingsAppEventHandler::OnToggleFloorOverride( wxCommandEvent& evt )
{
	if (!GetCtrlHelper(ID_USEWINDOWSBACKGROUND, wxCheckBox)->GetValue())
	{
		GetCtrlHelper(ID_FLOORWALLPAPER, wxFilePickerCtrl)->Enable();
		GetCtrlHelper(ID_FLOOR_LABEL, wxStaticText)->Enable();
	}
	else
	{
		GetCtrlHelper(ID_FLOORWALLPAPER, wxFilePickerCtrl)->Disable();
		GetCtrlHelper(ID_FLOOR_LABEL, wxStaticText)->Disable();
	}
}

void SettingsAppEventHandler::MarkSettingsAsDirty( wxCommandEvent& evt )
{
	// Settings->markDirty();
}

void SettingsAppEventHandler::OnFilePickerTextCtrlChanged(wxCommandEvent& evt)
{
	wxTextCtrl * text = (wxTextCtrl *) evt.GetEventObject();
	wxFilePickerCtrl * picker = (wxFilePickerCtrl *) text->GetParent();
	static wxString prevValue;
	wxString value = text->GetValue();
	if (value != prevValue)
	{
		picker->SetPath(value);
		prevValue = value;
	}
	_reloadTheme = true;
	Settings->markDirty();
}

void SettingsAppEventHandler::OnSendFeedback( wxCommandEvent& evt )
{	
	Settings->getComm().sendMessage(Win32Comm::SendFeedback, true);

	// animate the moving of this settings dialog to the left a bit
	_dialogSlideAmount = 200;
	_dialogSlideTimer.Start(10);
}

void SettingsAppEventHandler::OnToggleManualProxyOverrides( wxCommandEvent& evt )
{
	if (GetCtrlHelper(ID_AUTOPROXY_RADIO, wxRadioButton)->GetValue())
	{
		// save the manual fields first
		if (!GetCtrlHelper(ID_HTTPPROXY, wxTextCtrl)->GetValue().IsEmpty())
			Settings->setSavedString("httpProxyUrl", GetCtrlHelper(ID_HTTPPROXY, wxTextCtrl)->GetValue());
		// disable the manual stuff
		GetCtrlHelper(ID_HTTPPROXY, wxTextCtrl)->Disable();
		GetCtrlHelper(ID_HTTPPROXY, wxTextCtrl)->SetValue(wxEmptyString);
		GetCtrlHelper(ID_HTTPPROXY_LABEL, wxStaticText)->Disable();
		// enable the auto stuff
		GetCtrlHelper(ID_LAUNCHIEPROXYDIALOG, wxButton)->Enable();
		GetCtrlHelper(ID_MANUALPROXY_RADIO, wxRadioButton)->SetValue(false);
	}
	else
	{
		// disable the auto stuff
		GetCtrlHelper(ID_LAUNCHIEPROXYDIALOG, wxButton)->Disable();
		GetCtrlHelper(ID_AUTOPROXY_RADIO, wxRadioButton)->SetValue(false);
		// enable the manual stuff
		GetCtrlHelper(ID_HTTPPROXY, wxTextCtrl)->Enable();
		GetCtrlHelper(ID_HTTPPROXY, wxTextCtrl)->SetValue(Settings->getSavedString("httpProxyUrl"));
		GetCtrlHelper(ID_HTTPPROXY_LABEL, wxStaticText)->Enable();
	}
}

void SettingsAppEventHandler::OnLaunchIEProxySettings( wxCommandEvent& evt )
{
#ifdef WIN32
	SHELLEXECUTEINFO sei = {0};
	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.hwnd = (HWND) Settings->getDialog()->GetHandle();
	sei.lpFile = wxT("RunDll32.exe");
	sei.lpParameters = wxT("InetCpl.cpl,LaunchConnectionDialog");
	sei.nShow = SW_SHOWNORMAL;

	ShellExecuteEx(&sei);
#endif
}

void SettingsAppEventHandler::OnToggleInfiniteDesktop( wxCommandEvent& evt )
{
	if (GetCtrlHelper(ID_ENABLEINFINITEDESKTOP, wxCheckBox)->GetValue())
	{
		// bumptopsettings.OnToggleInfiniteDesktopPrompt.caption
		// bumptopsettings.OnToggleInfiniteDesktopPrompt.text
		// alert the user
		if (wxYES != wxMessageBox(wxGetTranslation(wxT("This will change the view and behaviour of your BumpTop.  Do you wish to continue?")),
			wxGetTranslation(wxT("Do you want to go infinite?")), wxYES_NO | wxCENTER, Settings->getDialog()))
		{
			GetCtrlHelper(ID_ENABLEINFINITEDESKTOP, wxCheckBox)->SetValue(false);
		}
	}
}

void SettingsAppEventHandler::OnDialogSlideTimer( wxTimerEvent& evt )
{
	if (_dialogSlideAmount > 0)
	{
		// slide to the left a bit
		int slideAmount = 20;

		wxPoint pos = Settings->getDialog()->GetPosition();
		pos.x -= slideAmount;
		if (pos.x < 0)
		{
			pos.x = 0;
			slideAmount = _dialogSlideAmount;
		}
		Settings->getDialog()->SetPosition(pos);

		// decrement the slide amount remaining
		_dialogSlideAmount -= slideAmount;
		if (_dialogSlideAmount <= 0)
			_dialogSlideTimer.Stop();
	}
}

void SettingsAppEventHandler::OnNextMonitor( wxCommandEvent& evt )
{
	Settings->getComm().sendMessage(Win32Comm::CycleMonitors, true);
	Settings->markDirty();
}

void SettingsAppEventHandler::OnOverrideDefaultFont( wxCommandEvent& evt )
{
	wxString defaultFont = Settings->getSavedString("fonts.override.default");
	int defaultFontSize = Settings->getSavedStringAsInt("fonts.override.default.size");
	wxArrayString fonts = wxFontEnumerator::GetFacenames();
	bool fontExists = fonts.Index(defaultFont.c_str()) != wxNOT_FOUND;
	bool isBold = false;
	if (!fontExists)
	{
		// try without the "Bold"/"Italic" substr at the end (we add this one)
		if (defaultFont.Find(L" Bold") > -1)
		{
			defaultFont = defaultFont.Mid(0, defaultFont.size() - 5);
			isBold = true;
		}
	}

	wxFontData fontData;
	fontData.SetInitialFont(wxFont(defaultFontSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
		(isBold ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL), false, defaultFont, wxFONTENCODING_SYSTEM));

	wxFontDialog * fontDlg = new wxFontDialog(Settings->getDialog(), fontData);
	if (wxID_OK == fontDlg->ShowModal())
	{
		fontData = fontDlg->GetFontData();

		wxString fontName = fontData.GetChosenFont().GetFaceName();
		bool altFormat = fontName.StartsWith(L"@");
		fontName.Replace(L"@", L"");
		if (fontName.IsEmpty())
			return;

		if (fontData.GetChosenFont().GetWeight() == wxFONTWEIGHT_BOLD)
		{
			if (altFormat)
				fontName.Append(L"-Bold");
			else
				fontName.Append(L" Bold");
		}
		Settings->setSavedString("fonts.override.default", fontName);
		Settings->setSavedString("fonts.override.default.size", fontData.GetChosenFont().GetPointSize());

		wxString tmp;
		tmp << Settings->getSavedString("fonts.override.default") 
			<< L", " << Settings->getSavedStringAsInt("fonts.override.default.size")
			<< L"pt";

		_reloadTheme = true;
		Settings->markDirty();
	}
}	

void SettingsAppEventHandler::OnOverrideStickyNoteFont( wxCommandEvent& evt )
{
	wxString defaultFont = Settings->getSavedString("fonts.override.stickyNote");
	int defaultFontSize = Settings->getSavedStringAsInt("fonts.override.stickyNote.size");
	wxArrayString fonts = wxFontEnumerator::GetFacenames();
	bool fontExists = fonts.Index(defaultFont.c_str()) != wxNOT_FOUND;
	bool isBold = false;
	if (!fontExists)
	{
		// try without the "Bold"/"Italic" substr at the end (we add this one)
		if (defaultFont.Find(L" Bold") > -1)
		{
			defaultFont = defaultFont.Mid(0, defaultFont.size() - 5);
			isBold = true;
		}
	}

	wxFontData fontData;
	fontData.SetInitialFont(wxFont(defaultFontSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
		(isBold ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL), false, defaultFont, wxFONTENCODING_SYSTEM));

	wxFontDialog * fontDlg = new wxFontDialog(Settings->getDialog(), fontData);
	if (wxID_OK == fontDlg->ShowModal())
	{
		fontData = fontDlg->GetFontData();

		wxString fontName = fontData.GetChosenFont().GetFaceName();
		bool altFormat = fontName.StartsWith(L"@");
		fontName.Replace(L"@", L"");
		if (fontName.IsEmpty())
			return;

		if (fontData.GetChosenFont().GetWeight() == wxFONTWEIGHT_BOLD)
		{
			if (altFormat)
				fontName.Append(L"-Bold");
			else
				fontName.Append(L" Bold");
		}
		Settings->setSavedString("fonts.override.stickyNote", fontName);
		Settings->setSavedString("fonts.override.stickyNote.size", fontData.GetChosenFont().GetPointSize());

		wxString tmp;
		tmp << Settings->getSavedString("fonts.override.stickyNote") 
			<< L", " << Settings->getSavedStringAsInt("fonts.override.stickyNote.size")
			<< L"pt";
	
		_reloadTheme = true;
		Settings->markDirty();
	}
}

bool SettingsAppEventHandler::subsetEqualsRec( const Json::Value& r1, const Json::Value& r2 )
{
	// compare each entry in the two roots
	vector<string> r1members = r1.getMemberNames();
	
	for (unsigned int i = 0; i < r1members.size(); ++i)
	{
		const std::string& memberName = r1members[i];

		if (!r2.isMember(memberName))
			return false;

		// and is of the correct value
		if ((r1[memberName].isBool() == r2[memberName].isBool()) ||
			(r1[memberName].isInt() == r2[memberName].isInt()) ||
			(r1[memberName].isUInt() == r2[memberName].isUInt()) ||
			(r1[memberName].isIntegral() == r2[memberName].isIntegral()) ||
			(r1[memberName].isDouble() == r2[memberName].isDouble()) ||
			(r1[memberName].isNumeric() == r2[memberName].isNumeric()) ||
			(r1[memberName].isString() == r2[memberName].isString()))
		{
			// Work around: Rotation degrees is written as a float from bumptop and an int from settings
			if (r1[memberName].isNumeric() && r2[memberName].isNumeric()) {
				double r1Double = r1[memberName].asDouble();
				double r2Double = r2[memberName].asDouble();
				if ( abs(r1Double - r2Double) > 0.005 )
					return false;
			} else {
				if (r1[memberName] != r2[memberName])
					return false;
			}
		}

		// and is of the correct size
		if (r1[memberName].size() != r2[memberName].size())
			return false;

		// recurse down to that member
		if (r1[memberName].isObject())
			subsetEqualsRec(r1[memberName], r2[memberName]);
	}
	return true;
}

void SettingsAppEventHandler::OnAuthorizeProKey( wxCommandEvent& evt )
{
	Settings->getComm().sendMessage(Win32Comm::AuthorizeProKey, true);

	// animate the moving of this settings dialog to the left a bit
	_dialogSlideAmount = 400;
	_dialogSlideTimer.Start(10);
}

void SettingsAppEventHandler::OnDeauthorizeProKey( wxCommandEvent& evt )
{
	Settings->getComm().sendMessage(Win32Comm::DeauthorizeProKey, true);

	// animate the moving of this settings dialog to the left a bit
	_dialogSlideAmount = 400;
	_dialogSlideTimer.Start(10);
}

void SettingsAppEventHandler::overrideMergeRecursive( Json::Value& to, const Json::Value& from )
{
	vector<std::string> fromMembers = from.getMemberNames();
	for (unsigned int i = 0; i < fromMembers.size(); ++i)
	{
		const std::string& memberName = fromMembers[i];

		if (!to.isMember(memberName))
			to[memberName] = from[memberName];
		else
		{
			if (from[memberName].isObject())
			{
				overrideMergeRecursive(to[memberName], from[memberName]);
			}
			else
			{
				// if it's an array of objects (like a font list) then just merge the
				// arrays
				const Json::Value& val = from[memberName];
				if (val.isArray())
				{
					for (int j = val.size()-1; j >= 0; --j)
					{
						prependUnique(to[memberName], val[j]);
					}
				}
				else
				{
					to[memberName] = from[memberName];
				}
			}
		}
	}
}

void SettingsAppEventHandler::prependUnique( Json::Value& arr, const Json::Value& val )
{
	assert(arr.isArray());

	// prepend the item to the array
	wxString s = wxString(val.asString().c_str(), wxConvUTF8);
	Json::Value newArr;
		newArr.append(val);
	for (unsigned int i = 0; i < arr.size(); ++i)
	{
		if (arr[i] != val)
		{
			newArr.append(arr[i]);
		}
	}
	arr = newArr;	
}

void SettingsAppEventHandler::replaceFontsRecursive( Json::Value& to, const Json::Value& from )
{
	vector<std::string> fromMembers = from.getMemberNames();
	for (unsigned int i = 0; i < fromMembers.size(); ++i)
	{
		const std::string& memberName = fromMembers[i];

		// replace the fonts
		if (memberName == "font")
		{
			to[memberName] = from[memberName];
		}
		else
		{
			if (from[memberName].isObject())
				replaceFontsRecursive(to[memberName], from[memberName]);
		}
	}
}

void SettingsAppEventHandler::OnTextureLevelChange( wxCommandEvent& evt )
{
	wxChoice *choice = GetCtrlHelper(ID_TEXTURE_LIST, wxChoice);
	int value = choice->GetSelection();
	wxStaticText *animationLevel = GetCtrlHelper(ID_TEXTUREQUALITY_LABEL, wxStaticText);

	if (value == 0)
		animationLevel->SetLabel(wxGetTranslation(wxT("Uses less video memory for photo frames and pictures")));
	else if (value == 1)
		animationLevel->SetLabel(wxGetTranslation(wxT("A good balance between performance and appearance")));
	else 
		animationLevel->SetLabel(wxGetTranslation(wxT("Use high quality images for photo frames and pictures")));
}

static bool containsIllegalChars(wxString stringToTest) 
{	
	const wxString invalidChars(wxT("\\:/*?\"<>|"));

	for (unsigned int i = 0; i < invalidChars.size(); i++)
	{
		if (stringToTest.Contains(invalidChars[i]))
		{
			return true;
		}
	}
	return false;
}

void SettingsAppEventHandler::OnUploadTheme( wxCommandEvent& evt )
{
	// Get the theme listings
	ThemeListing theme = Settings->getThemes();

	// Get path to defaults theme folder
	wxString folderToZip, saveAsName;
	folderToZip = Settings->getPath(Win32Path::UserDefaultThemeDir);

	// Create dialog prompt to get theme name
	wxTextEntryDialog * filePicker;
	wxTextEntryDialog firstTryFilePicker(Settings->GetTopWindow(), wxT("Please enter a name for your theme"), wxT("Save Theme"));
	wxTextEntryDialog illCharFilePicker(Settings->GetTopWindow(), wxT("Please enter a name for your theme (Characters including \\:/*?\"<>| are not allowed)"), wxT("Save Theme"));
	filePicker = &firstTryFilePicker;
	
	//Continuously show dialog box until name is correct or cancel is pressed
	do 
	{
		if(filePicker->ShowModal() == wxID_CANCEL)
			return;
		else if(containsIllegalChars(filePicker->GetValue())) {
			illCharFilePicker.SetValue(filePicker->GetValue());
			filePicker = &illCharFilePicker;
		}else
			break;

	} while (true);

	// Check to make sure the theme name isn't empty
	if (filePicker->GetValue().IsEmpty())
		filePicker->SetValue(wxT("Your BumpTop Theme"));

	// Set the saveAsName for the theme
	wxString themeFileName = filePicker->GetValue();
	saveAsName = Settings->getPath(Win32Path::UserThemesDir) + themeFileName + wxT(".bumptheme");
	
	// Archive the theme
	theme.createArchive(wxFileName(folderToZip), wxFileName(saveAsName));
	
	wchar_t * passString = (wchar_t *)themeFileName.fn_str();
	
	COPYDATASTRUCT cpd;
	cpd.cbData = (themeFileName.size() * sizeof(wchar_t));
	cpd.lpData = passString;

	Settings->getComm().sendDataMessage(Win32Comm::UploadTheme, cpd);
}
