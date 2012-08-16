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

#ifndef _BT_AUTHORIZATON_
#define _BT_AUTHORIZATON_

#include "BT_SVNRevision.h"
#include "BT_Singleton.h"
#include "BT_Util.h"

enum AuthorizationLevel
{
	AL_FREE,
	AL_PRO
};

// workaround to allow for translated strings
class AuthorizationStrings
{
	Q_DECLARE_TR_FUNCTIONS(AuthorizationStrings)

	QHash<QString, QString> _strs;

private:
	friend class Singleton<AuthorizationStrings>;
	AuthorizationStrings();

public:
	QString getString(QString key);
};
#define AuthStr Singleton<AuthorizationStrings>::getInstance()

// This is the function run at initialization to make sure this is a legitimate copy of BumpTop
bool testAuthorizationProof();
bool validateAuthorization(AuthorizationLevel authLevel, QStringList authHashes);
bool deauthorize();
bool isProBuild();

bool authorizeHashString(QString authHashStr, AuthorizationLevel minAuthLevel, AuthorizationLevel& authLevelOut, QStringList& authHashesOut, QString& errorMessageOut);
bool authorizeCode(QString code, AuthorizationLevel minAuthLevel, QString& errorMessageOut);

QString getBumpTopProPageUrl(QString source, bool skipVersionOptoinScreen = false);
void launchBumpTopProPage(QString source, bool skipVersionOptionScreen = false);

#define reauthorize(forceReauthorize) { }

#endif