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

#ifndef BT_SETTINGSAPP
#define BT_SETTINGSAPP

#include "BT_SettingsDialog.h"
#include "BT_ThemeListing.h"

#ifdef WIN32
	#include "BT_Win32Registry.h"
	#include "BT_Win32Path.h"
	#include "BT_Win32Util.h"	
	#include "BT_Win32Comm.h"
#endif 

// ----------------------------------------------------------------------------

class SettingsAppEventHandler;

// ----------------------------------------------------------------------------

class SettingsApp : public wxApp
{
	wxSingleInstanceChecker * _instanceChecker;
	SettingsDialog * _dialog;
	SettingsAppEventHandler * _handler;
	ThemeListing _themes;
	bool _isAdmin;
	int _focusedTabIndex;
	wxFileName _settingsFile;
	Json::Value _savedVars;
	wxString _inviteCode;
	wxString _language;
	wxString _proUrl;
	bool _adminTabDeleted;

#ifdef WIN32
	Win32Registry _registry;
	Win32Path _paths;
	Win32Util _util;
	Win32Comm _bumptopComm;
#endif
	

private:
	void syncValue(bool fromValueToDialog, Json::Value& boolVal, wxCheckBox * cb, bool inverse=false);
	void syncValue(bool fromValueToDialog, Json::Value& intVal, wxSpinCtrl * spin, bool asFloat=false);
	void syncValue(bool fromValueToDialog, Json::Value& intVal, wxChoice * choice);
	void syncValue(bool fromValueToDialog, Json::Value& arrVal, wxListBox * list);
	void syncValue(bool fromValueToDialog, Json::Value& strVal, wxTextCtrl * text);
	void syncValue(bool fromValueToDialog, Json::Value& strVal, wxFilePickerCtrl * file);
	void syncValue(bool fromValueToDialog, Json::Value& intVal, wxSlider * slider);
	void syncTextureToValue(Json::Value& strVal, const wxFileName& strValueDir, wxFilePickerCtrl * file);
	wxFileName& composePath(wxFileName& path, const Json::Value& dir);

	void bindSettingsEvents();

public:
	SettingsApp();
	// operations
	void syncSettings(bool fromSettingsFileToDialog, Json::Value& root);
	bool loadSettingsFile(Json::Value& root);
	static bool loadJSONFile( const wxFileName& file, Json::Value& root );
	static bool UnzipFile( const wxFileName& fileToUnzip, const wxFileName& unzipLocation, wxString& expectedFile);
	static void BackUpZipFile(const wxFileName& fileToBackUp, const wxString& backUpLocation);
	void saveSettingsFile(const Json::Value& root);
	bool isDifferent(const Json::Value& otherRoot);

	void markDirty();
	void markClean();
	bool isDirty();

	void markNeedsRestart();
	void unmarkNeedsRestart();
	bool needsRestart();

	void markSettingsApplied();
	bool isSettingsApplied();

	// accessors
	wxDialog * getDialog() const;
	ThemeListing& getThemes();
	wxString getSavedString(const string& key);
	int getSavedStringAsInt(const string& key);
	void setSavedString(const string& key, const wxString& str);
	void setSavedString(const string& key, int val);
	bool setLocale(const wxString& langId);

	static BOOL CALLBACK EnumProc(HWND hwnd, LPARAM lp);
#ifdef WIN32
	wxString getPath(Win32Path::Win32PathKey key);
	Win32Util& getUtil();
	Win32Comm& getComm();
#endif

	// overrides
	virtual bool OnInit();
	virtual int OnRun();
	virtual int OnExit();
	virtual void OnInitCmdLine(wxCmdLineParser& parser);
	virtual bool OnCmdLineParsed(wxCmdLineParser& parser);
};

// ----------------------------------------------------------------------------

// wx app declaration
DECLARE_APP(SettingsApp)

// oi, wx doesn't seem to have a timer event function definition
typedef void (wxEvtHandler::*wxTimeEventFunction)(wxTimerEvent&);
#define wxTimeEventHandler(func) \
	(wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(wxTimeEventFunction, &func)


// from auto generated XRC cpp
extern void InitXmlResource();

#define Settings (&wxGetApp())

#define QUOTE(txt) #txt
#define GetWXControl(idName, type, dialog) dynamic_cast<type *>(wxWindow::FindWindowById(XRCID(QUOTE(idName)), dialog)) 
#define BindWXEvent(obj, eventType, handler, handlerFunction) obj->Connect(wxID_ANY, eventType, wxCommandEventHandler(SettingsAppEventHandler::handlerFunction), 0, handler)
#define BindWXEventData(obj, eventType, handler, handlerFunction, data) obj->Connect(wxID_ANY, eventType, wxCommandEventHandler(SettingsAppEventHandler::handlerFunction), data, handler)

#endif // BT_SETTINGSAPP