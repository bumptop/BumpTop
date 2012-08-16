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

#pragma once

#ifndef _INPUT_BOX_
#define _INPUT_BOX_

// -----------------------------------------------------------------------------

#include "BT_Singleton.h"
#include "BT_Windows7CommonControlsOverride.h"

// -----------------------------------------------------------------------------

enum DialogType
{
	DialogOK					= MB_OK,
	DialogOKCancel				= MB_OKCANCEL,
	DialogYesNo					= MB_YESNO,
	DialogInput					= IDD_INPUTBOX,
	DialogInput2				= IDD_INPUTBOX2,
	DialogInputAuth				= IDD_INPUTBOX_AUTHORIZATION,
	DialogInputBrowse			= IDD_INPUTBOX_BROWSE,
	DialogCrash					= IDD_CRASHBOX,
	DialogFeedback				= IDD_FEEDBACK,
	DialogUpdateBumptop			= IDD_UPDATE_BUMPTOP,
	DialogPhotoFrame			= IDD_PHOTO_FRAME,
	DialogThemeConflict			= IDD_THEME_CONFLICT, 
	DialogCaptionOnly			= IDD_PROCESS,
	DialogDirectoryListing		= IDC_BROWSE,
	DialogBreakPile				= IDD_BREAKPILE,
	DialogChangeIcon			= IDD_CHANGEICON,
	DialogUpdateGraphicsDrivers = IDD_UPDATEDRIVERS,
	DialogAuthorizationFailed	= IDD_AUTHFAILED,
	DialogUpdateMessage			= IDD_UPDATEMESSAGE,
	DialogTwitterTweet			= IDD_TWITTER,
	DialogTwitterAuth			= IDD_TWITTER_AUTH,
	DialogTwitterLogin			= IDD_TWITTER_LOGIN,
	DialogFacebookAuth			= IDD_FACEBOOK_AUTH,
	DialogFacebookConfirm		= IDD_FACEBOOK_CONFIRM,
	DialogAuthorization			= IDD_AUTHORIZE, 
	DialogAuthorizationChoose	= IDD_AUTHORIZATION_CHOOSE,
	DialogDeauthorizeConfirm	= IDD_DEAUTHORIZE_CONFIRM,
	DialogChooseVersion			= IDD_CHOOSE_VERSION,
	DialogThankYou				= IDD_THANK_YOU,
	DialogEula					= IDD_EULA
};

// -----------------------------------------------------------------------------

class Win32Dialog
{
	double dpiScaleX, dpiScaleY; // Not exactly 96 / DPI

public:

	// reset instance
	virtual void resetToDefault() = 0;

	// dialog initialization

	// Make dialog look the same in any DPI as 96;  Must call AdjustDialogDPI before using AdjustControlDPI  
	// Calculates the x and y scaling using hard coded dialog size,
	// x and y scaling is slightly different than 96 / DPI X and 96 / DPI Y.
	void AdjustDialogDPI(PWND dialog, int targetClientW, int targetClientH); 
	
	// Make dialog controls look the same in any DPI as 96 using calculated scales in AdjustDialogDPI
	void AdjustControlDPI(PWND owner, PWND control); 
	
	virtual bool onInit(HWND hwnd) = 0;
	virtual bool onDestroy();
	bool closeDialog(HWND hwnd, int returnCode);

	// dialog interaction
	virtual bool onCommand(HWND hwnd, WPARAM wParam, LPARAM lParam); 
	virtual bool onMouseDown(HWND hwnd, WPARAM wParam, LPARAM lParam);
	virtual bool onMouseUp(HWND hwnd, WPARAM wParam, LPARAM lParam);
	virtual bool onMouseLeave(HWND hwnd, WPARAM wParam, LPARAM lParam);
	virtual bool onMouseMove(HWND hwnd, WPARAM wParam, LPARAM lParam);
};

// -----------------------------------------------------------------------------

class DialogManager
{
	Q_DECLARE_TR_FUNCTIONS(DialogManager)

	QString		caption;
	QString		prompt;
	QString		text, subText;
	POINT		selText, selSubText;
	QString		type;
	QString		emailAddress;
	QString		image;
	bool		checked;
	bool		pressedAuthorizeManually;
	bool		pressedFree;
	bool		expiredChooseVersionDialog;
	DialogType	dialogType;
	HWND		hDlg;
	bool		hasParent;
	static WNDPROC prevRichEditCtrlWndProc;

	// task dialogs
	HMODULE		_comctrlModule;
	TaskDialogIndirect _pTaskDialogIndirect;

	// set of active complex win32 dialogs (can only have one of each active)
	typedef hash_map<DialogType, Win32Dialog *> ComplexDialogsContainer;
	ComplexDialogsContainer complexDialogs;

	// Singleton
	friend class Singleton<DialogManager>;
	DialogManager();

	// Private Actions
	bool		promptStock();
	bool		promptCustom();
	void		setType(QString type);

	// task dialogs
	bool		vistaPromptDialogOverride(DialogType type, bool& successOut);
	static HRESULT CALLBACK vistaTaskDialogCallbackProc(HWND hwnd, UINT uNotification, WPARAM wParam, LPARAM lParam, LONG_PTR dwRefData);

public:
	~DialogManager();

	// Callback Functions for custom Dialogs
	static LRESULT		dlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT		richEditCtrlProc(HWND ctrl, UINT msg, WPARAM wParam, LPARAM lParam);

	// Actions
	bool		promptDialog(DialogType boxType = DialogOK);
	bool		promptDirListing();
	bool		clearState();

	// Setters
	void		setPrompt(QString newPrompt);
	void		setCaption(QString newCaption);
	void		setText(QString in);
	void		setTextSelection(int start, int end);
	void		setSubText(QString in);
	void		setSubTextSelection(int start, int end);
	void		setEmail(QString newEmail);
	void		setImage(QString filePath);
	void		setChecked(bool checked);
	void		setHasParent(bool hasParent);
	void		setPressedAuthorizeManually(bool val);
	void		setPressedFree(bool val);
	void		setExpiredChooseVersionDialog(bool val);

	// Getters
	QString		getText() const;
	POINT		getTextSelection() const;
	QString		getSubText() const;
	POINT		getSubTextSelection() const;
	QString		getType() const;
	QString		getEmail() const;
	QString		getCaption() const;
	QString		getPrompt() const;
	QString		getImage() const;
	bool		getChecked() const;
	DialogType	getDialogType() const;
	bool		getHasParent();
	bool		getPressedAuthorizeManually();
	bool		getPressedFree();
	bool		getExpiredChooseVersionDialog() const;

	Win32Dialog *getComplexDialog(DialogType dialogType);
};

// -----------------------------------------------------------------------------

#define dlgManager Singleton<DialogManager>::getInstance()

#endif