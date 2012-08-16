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

#ifndef _UPDATE_SERVER_PROXY_
#define _UPDATE_SERVER_PROXY_
#include "BT_FileTransferManager.h"


class DownloadFailedException: public std::exception
{
	virtual const char* what() const throw();

};

class UpdateServer
{
public:
	UpdateServer();
	virtual QString getVersionString() = 0;
	virtual QString getUpdateMessageString() = 0;
	virtual QString getDownloadedInstallerFile() = 0;

	//************************************
	// Method:    onUpdaterCheckHandler
	// FullName:  UpdateServer::onUpdaterCheckHandler
	// Access:    virtual public 
	// Returns:   void
	// Qualifier: /*= 0
	// 
	// Called whenever updater pings the server, and the return value tells the updater
	// whether to quit or not
	//************************************
	virtual bool onUpdaterCheckHandler() = 0;
	virtual void downloadNewVersion() = 0;
};


class UpdateServerImpl : UpdateServer
{
protected:
	QString _downloadDirPath;
	QString _messageFileUrl;
	QString _versionFileUrl;
	QString _descFileUrl;
	QString _md5FileUrl;
	QString _pointerToBumptopInstallerUrl;
	QString _bumptopInstallerFilename;
public:
	UpdateServerImpl(QString downloadDirPath);
	QString getVersionString();
	QString getUpdateMessageString();
	QString getDownloadedInstallerFile();
	bool onUpdaterCheckHandler();
	void downloadNewVersion();


};

class StagingUpdateServer : public UpdateServerImpl
{
public:
	StagingUpdateServer(QString downloadDirPath);
};

class VIP_UpdateServer : public UpdateServerImpl
{
public:
	VIP_UpdateServer(QString downloadDirPath);
};

#endif _UPDATE_SERVER_PROXY_
