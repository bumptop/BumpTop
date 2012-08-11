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

#ifndef BT_SETTINGSAPPEVENTHANDLER
#define BT_SETTINGSAPPEVENTHANDLER

class SettingsAppEventHandler : public wxEvtHandler
{		
	wxTimer _dialogSlideTimer;
	wxFileName _previousTheme;
	bool _reloadTheme;
	int _dialogSlideAmount;
	vector<pair<wxString, wxString> > _languages;
	vector<pair<wxString, wxString> > _cameraAngles;
	vector<pair<int, wxString> > _textureSettings;

private:
	void OnDialogSlideTimer(wxTimerEvent& evt);
	bool subsetEqualsRec(const Json::Value& r1, const Json::Value& r2);
	void overrideMergeRecursive(Json::Value& to, const Json::Value& from);
	void replaceFontsRecursive(Json::Value& to, const Json::Value& from);
	void prependUnique(Json::Value& arr, const Json::Value& val);

public:
	SettingsAppEventHandler();

	// accessors
	bool hasThemeChanged(bool resetAfterQuery);

	// filled events
	virtual void MarkSettingsAsDirty(wxCommandEvent& evt);

	// events
	virtual void OnWindowCheckTimer(wxTimerEvent& evt);
	virtual void OnApplyThrobberTimer(wxTimerEvent& evt);

	// General tab
	virtual void OnTextureLevelChange(wxCommandEvent& evt);

	virtual void OnResetSettings(wxCommandEvent& evt);
	virtual void OnApplyCurrentTheme();
	virtual void OnSaveSettings(wxCommandEvent& evt);
	virtual void OnCancel(wxCommandEvent& evt);

	virtual void OnToggleManualProxyOverrides(wxCommandEvent& evt);
	virtual void OnLaunchIEProxySettings(wxCommandEvent& evt);

	virtual void OnRotationLimitDegreesChange(wxCommandEvent& evt);

	virtual void OnToggleFilenames(wxCommandEvent& evt);

	virtual void OnThemeSelected(wxCommandEvent& evt);
	virtual void OnUpdateThemesListing(wxFileName * selectedTheme, bool recordSelectedThemeAsPrevious);
	virtual void OnReloadThemes(wxCommandEvent& evt);
	virtual void OnUploadTheme(wxCommandEvent& evt);
	virtual void OnToggleFloorOverride(wxCommandEvent& evt);
	virtual void OnFilePickerTextCtrlChanged(wxCommandEvent& evt);
	virtual void OnGetMoreThemesSelected(wxCommandEvent& evt);
	virtual void OnUpdateLanguagesListing(wxString& language, bool fromSettingsFileToDialog);
	virtual void OnUpdateCameraPresets(wxString& cameraPreset, bool fromSettingsFileToDialog);
	virtual void OnUpdateTextureSettings(int& textureSetting, bool fromSettingsFileToDialog);

	// Advanced Tab Event handling
	virtual void OnResetBumpTopLayout(wxCommandEvent& evt);
	virtual void OnClearCache(wxCommandEvent& evt);
	virtual void OnToggleInfiniteDesktop(wxCommandEvent& evt);

	virtual void OnSendFeedback(wxCommandEvent& evt);
	virtual void OnNextMonitor(wxCommandEvent& evt);

	virtual void OnAuthorizeProKey(wxCommandEvent& evt);
	virtual void OnDeauthorizeProKey(wxCommandEvent& evt);

	virtual void OnOverrideDefaultFont(wxCommandEvent& evt);
	virtual void OnOverrideStickyNoteFont(wxCommandEvent& evt);
};

#endif // BT_SETTINGSAPPEVENTHANDLER