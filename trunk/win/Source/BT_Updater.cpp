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
#include "BT_Updater.h"
#include "BT_UpdateServer.h"
#include "BT_QtUtil.h"
#include "BT_DialogManager.h"
#include "BT_FileSystemManager.h"
#include "BT_SceneManager.h"
#include "BT_OverlayComponent.h"

const char* UpdateCheckFailedException::what() const throw()
{
	return "Parsing the version number failed";
}

UpdateServer::UpdateServer()
{
}

Updater::Updater( UpdateServer* updateServer, int versionNumber, QString downloadDirectory, uint retryTimeAfterFailedUpdateCheck, uint timeBetweenUpdateChecks, HWND bumptopHwnd, bool manualUpdateOnly)
{
	_manualUpdateOnly = manualUpdateOnly;
	_updateServer = updateServer;
	_currentVersionNumber = versionNumber;
	_downloadDirectory = downloadDirectory;
	_retryTimeAfterFailedUpdateCheck = retryTimeAfterFailedUpdateCheck;
	_timeBetweenUpdateChecks = timeBetweenUpdateChecks;
	_bumptopHwnd = bumptopHwnd;
	

}

bool Updater::isUpdateDownloaded()
{
	return exists(make_file(_downloadDirectory, _updateServer->getDownloadedInstallerFile())) 
		&& exists(make_file(_downloadDirectory, "version.txt")) 
		&& exists(make_file(_downloadDirectory, "desc.txt"));
}


int Updater::getNewestVersionNumber()
{
	int version = _updateServer->getVersionString().toInt();
	if (version == 0)
		throw UpdateCheckFailedException();

	return version;
	
}

QString Updater::getUpdateMessage()
{
	return _updateServer->getUpdateMessageString();
}

QString Updater::getDownloadedInstallerFile()
{
	return _updateServer->getDownloadedInstallerFile();
}

void Updater::run()
{

#ifdef DISABLE_PHONING
	return;
#endif
	
	bool updateCheckFailed = false;

	try {
		QString message = getUpdateMessage();
		if (!message.isEmpty())
		{
			dlgManager->clearState();
			dlgManager->setPrompt(message);
			if (dlgManager->promptDialog(DialogUpdateMessage))
			{
				// send them to bumptop.com/notification
				QString notificationBumpTopURL("http://bumptop.com/updateNotification");
				fsManager->launchFileAsync(notificationBumpTopURL);
				printUnique("ForwardURL", QT_TR_NOOP("Forwarding you to Update Notifications page"));
			}
		}

		if (getNewestVersionNumber() > getCurrentVersionNumber())
		{
			if (!isUpdateDownloaded() || getNewestVersionNumber() != getDownloadedVersionNumber())
				_updateServer->downloadNewVersion();

			if (isUpdateDownloaded() && getNewestVersionNumber() == getDownloadedVersionNumber()) {
				sendUpdateDownloadedMessageToBumpTop(false);
				return;
			} else {
				QString error_message = QString("downloadNewVersion finished but still not updating,"
					"and didn't throw a downloadFailedException;"
					"isUpdateDownloaded(): %1;"
					"getNewestVersionNumber(): %2;"
					"getDownloadedVersionNumber(): %3;")
					.arg(isUpdateDownloaded()).arg(getNewestVersionNumber()).arg(getDownloadedVersionNumber());
				logError(error_message);
				throw DownloadFailedException();
			}
		}
	}
	catch(UpdateCheckFailedException&)
	{
		logError("UpdateCheckFailedException");
		updateCheckFailed = true;
	}
	catch(DownloadFailedException&)
	{
		logError("DownloadFailedException");
		updateCheckFailed = true;
	}
}

int Updater::getDownloadedVersionNumber()
{
	int downloadedVersion = -1;

	if (isUpdateDownloaded())
	{
		QString downloadedVersionStringStream = read_file_utf8((_downloadDirectory + "\\version.txt"));
		downloadedVersion = downloadedVersionStringStream.toInt();
	}

	return downloadedVersion;

}

uint Updater::getUpdateDownloadedMessageId()
{
	// if you change this string, you must change it in BT_WindowsOS	
	return RegisterWindowMessage((LPCWSTR) QString("BUMPTOP_UPDATE_DOWNLOADED").utf16());
}

int Updater::sendUpdateDownloadedMessageToBumpTop( bool sendTestMessage )
{
	WPARAM wParam = sendTestMessage ? 1 : 0;

	return PostMessage(_bumptopHwnd, getUpdateDownloadedMessageId(), wParam, 0);

	
}

int Updater::getCurrentVersionNumber()
{
	return _currentVersionNumber;
}

void Updater::logError( QString error )
{
	time_t rawtime;
	time ( &rawtime );
	tm* now = localtime( &rawtime );

	QString contents = QString("r%1\t%2-%3-%4 %5:%6:%7\t%8")
		.arg(_currentVersionNumber)
		.arg(now->tm_year+1900)
		.arg(now->tm_mon+1)
		.arg(now->tm_mday)
		.arg(now->tm_hour)
		.arg(now->tm_min)
		.arg(now->tm_sec)
		.arg(error);
	append_file_utf8(contents, (_downloadDirectory + "\\error.log"));
}