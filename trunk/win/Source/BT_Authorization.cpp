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
#include "BT_Authorization.h"
#include "BT_CustomizeWizard.h"
#include "BT_FileSystemManager.h"
#include "BT_OverlayComponent.h"
#include "BT_SettingsAppMessageHandler.h"
#include "BT_SceneManager.h"
#include "BT_WindowsOS.h"

//
int authorizationToken = 7;
int authorizationTokenHash = 1;

bool authorizeHashString(QString authHashStr, AuthorizationLevel minAuthLevel, AuthorizationLevel& authLevelOut, QStringList& authHashesOut, QString& errorMessageOut)
{
	authLevelOut = AL_PRO;
	authHashesOut = QStringList();
	return true;
}

bool authorizeCode(QString code, AuthorizationLevel minAuthLevel, QString& errorMessageOut)
{
	errorMessageOut.clear();

	// write the authorization vars to the settings
	GLOBAL(settings).proAuthCode = QT_NT("");
	GLOBAL(settings).authCode.clear();
	GLOBAL(settings).inviteCode.clear();

	winOS->SaveSettingsFile();
	winOS->GetSettingsAppMessageHandler()->updateAuthorizationKey();
	return true;
}

bool validateAuthorization(AuthorizationLevel authLevel, QStringList authHashes)
{
	return true;
}

QString getBumpTopProPageUrl(QString source, bool skipVersionOptionScreen)
{
	return QT_NT("");
}

void launchBumpTopProPage(QString source, bool skipVersionOptionScreen)
{
	//Check for a current pro window
	QWidgetList l = QApplication::topLevelWidgets();
	bool foundWindow = false;
	for (int i = 0; i < l.size(); i++)
	{
		if (l[i]->windowTitle().contains("Upgrade to BumpTop Pro")) {
			l[i]->activateWindow();
			l[i]->show();
			l[i]->move(qApp->desktop()->geometry().center() - l[i]->rect().center());
			foundWindow = true;
			break;
		}
	}
	if (!foundWindow)
	{
		QString proPageUrl = getBumpTopProPageUrl(source, skipVersionOptionScreen);
		CustomizeWizard *proWiz = new CustomizeWizard(CustomizeWizard::PRO,proPageUrl);
		QString text = proWiz->exec();
	}
}

bool deauthorize()
{
	return true;
}

bool isProBuild()
{	
	return true;
}

AuthorizationStrings::AuthorizationStrings()
{
	_strs.insert("AuthCaption", QT_TR_NOOP("BumpTop Authorization"));
	_strs.insert("EnterInviteCode", QT_TR_NOOP("Please enter your BumpTop Pro key:"));
	_strs.insert("FreeEnterInviteCode", QT_TR_NOOP("Please enter your invite code or Pro key:"));
	_strs.insert("EmptyCodePrompt", QT_TR_NOOP("Please enter a valid Pro key before authorizing."));
	_strs.insert("FreeEmptyCodePrompt", QT_TR_NOOP("Please enter a valid invite code or Pro key before authorizing."));
	_strs.insert("CodePromptCaption", QT_TR_NOOP("Authorize BumpTop manually"));
	_strs.insert("InvalidCodePrompt", QT_TR_NOOP("Pro key is invalid. Please double check before authorizing manually."));
	_strs.insert("FreeInvalidCodePrompt", QT_TR_NOOP("Invite code or pro key is invalid. Please double check before authorizing manually."));
	_strs.insert("InvalidCode", QT_TR_NOOP("Not a valid Pro key. "));
	_strs.insert("FreeInvalidCode", QT_TR_NOOP("Not a valid invite code or Pro key. "));
	_strs.insert("ManualAuthorize", QT_TR_NOOP("BumpTop is launching your web browser...\nPlease enter the authorization code shown in your web browser:"));
	_strs.insert("NonExistentCode", QT_TR_NOOP("Couldn't find that Pro key. "));
	_strs.insert("FreeNonExistentCode", QT_TR_NOOP("Couldn't find that Pro key. "));
	_strs.insert("AuthorizationFailed", QT_TR_NOOP("Authorization failed! "));
	_strs.insert("IncorrectCode", QT_TR_NOOP("Pro key or invite code is incorrect! "));
	_strs.insert("ConnectionError", QT_TR_NOOP("Unable to connect to internet! "));
	_strs.insert("ManualAuth", QT_TR_NOOP("BumpTop could not authorize automatically. "
		"Please visit the manual authorization page at \n\t %1.\n"
		"Then enter \n\t \"%2\" in field 1, \n\t \"%3\" in field 2, \n\t \"%4\" in field 3.\n"
		"Finally, copy the authorization code from the web page into the pro-key textbox in BumpTop after pressing OK."));
	_strs.insert("BetaInviteCode", QT_TR_NOOP("Beta invite codes are no longer supported. Please use a Pro key."));
}

QString AuthorizationStrings::getString( QString key )
{
	QHash<QString, QString>::const_iterator iter = _strs.find(key);
	if (iter != _strs.end())
		return iter.value();
	assert(false);
	return QString();
}