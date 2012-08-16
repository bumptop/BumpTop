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

#ifndef _BT_UPDATER_
#define _BT_UPDATER_

#include "BT_Stopwatch.h"

class UpdateServer;

// ---------------------------------------------------------------

class UpdateCheckFailedException: public std::exception
{
	virtual const char* what() const throw();

};

class Updater
{
	Q_DECLARE_TR_FUNCTIONS(Updater)

	friend class UpdaterTest;
	friend class UpdateServerTest;
protected:
	bool _manualUpdateOnly; // When true, run() will only do one iteration of update check.
	UpdateServer *_updateServer;
	int _currentVersionNumber;
	QString _downloadDirectory;
	uint _retryTimeAfterFailedUpdateCheck;
	uint _timeBetweenUpdateChecks;
	HWND _bumptopHwnd;


	bool isUpdateDownloaded();
	int getNewestVersionNumber();
	int getDownloadedVersionNumber();
	int getCurrentVersionNumber();
	uint getUpdateDownloadedMessageId();
	QString getUpdateMessage();
	int sendUpdateDownloadedMessageToBumpTop(bool sendTestMessage);
	void logError(QString error);
public:	
	Updater(UpdateServer* updateServer, int currentVersion, QString downloadDirectory, uint retryTimeAfterFailedUpdateCheck, uint timeBetweenUpdateChecks, HWND bumptopHwnd, bool manualUpdateOnly = false);
	void run();
	bool isManualUpdateOnly() { return _manualUpdateOnly; }
	QString getDownloadedInstallerFile();
};
#endif