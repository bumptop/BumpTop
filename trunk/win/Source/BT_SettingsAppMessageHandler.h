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

#ifndef BT_SETTINGSAPPMESSAGEHANDLER
#define BT_SETTINGSAPPMESSAGEHANDLER

class SettingsAppMessageHandler
{
	Q_DECLARE_TR_FUNCTIONS(SettingsAppMessageHandler)

	bool _updatedThemes;

	unsigned int ResetDesktopLayout;
	unsigned int IsInfiniteDesktopModeEnabled;
	unsigned int ToggleInfiniteDesktopMode;
	unsigned int CheckForUpdates;
	unsigned int SendFeedback;
	unsigned int AutoDetectProxy;
	// unsigned int ResetSettings;
	unsigned int ReloadTheme;
	unsigned int ReloadSettings;
	unsigned int CycleMonitors;
	unsigned int AuthorizeProKey;
	unsigned int DeauthorizeProKey;
	unsigned int LaunchProxySettings;
	unsigned int AuthorizationDialogClose;
	unsigned int UploadTheme;
	unsigned int SettingsRequireRestart;
	unsigned int InstallWebWidget;
	unsigned int StartCustomizeDialog;

public:
	SettingsAppMessageHandler();

	bool handleMessage(UINT msg, WPARAM wParam, LPARAM lParamm, LRESULT& resultOut);
	bool showProxySettings();
	bool updateAuthorizationKey();
};

#endif // BT_SETTINGSAPPMESSAGEHANDLER