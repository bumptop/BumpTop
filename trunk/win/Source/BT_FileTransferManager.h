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

#ifndef _FILE_TRANSFER_MANAGER_
#define _FILE_TRANSFER_MANAGER_

// -----------------------------------------------------------------------------
#include "BT_Common.h" 
#include "BT_Singleton.h"

// -----------------------------------------------------------------------------

// forward declarations
class FileTransferManager;
class FileTransfer;

// -----------------------------------------------------------------------------

/*
 * The handler interface to be implemented by classes which expect to be notified
 * of the progress of a file transfer.
 */
class FileTransferEventHandler
{
public:
	virtual void onTransferComplete(const FileTransfer& transfer) = 0;
	virtual void onTransferError(const FileTransfer& transfer) = 0;
};

// -----------------------------------------------------------------------------

/*
 * A representation of a file transfer.
 */
class FileTransfer
{
public:
	/*
	 * The type of file transfer.
	 */
	enum FileTransferType
	{
		Upload = (1 << 0),
		Download = (1 << 1)
	};

	/*
	 * Common urls to upload to.  
	 * NOTE: remember to change the commonUrls array if you modify the 
	 *		 indices in this enum.
	 */
	enum FileTransferCommonUrl
	{
		BumpTopFtpStatsDirectory,
		BumpTopFtpTestsDirectory,
		BumpTopFtpReleasesDirectory,
		BumpTopFtpStatsDirectoryToshiba,
		BumpTopFtpStatsDirectoryDell,
		BumpTopFtpUpdateDirectory,
		BumpTopFtpStagingUpdateDirectory,
		BumpTopHttpUpdateDirectory,
		BumpTopHttpStagingUpdateDirectory
	};

private:
	FileTransferEventHandler * _handler;
	FileTransferType _type;
	QString _url, _localPath, _fileUploadFieldName, _remoteFileName;
	QString _login, _password;
	QHash<QString, QString> _params;
	QSet<QString> _fileParamKeys;
	FILE * _fileHandle;
	QString * _stringBuffer;
	int _timeout;
	void *_userData;
	

	// list of common urls/logins and passwords
	// NOTE: remember to change the enums above if you modify the indices
	//		 in this array.
	static const QString commonUrls[];
	static const QString commonLogins[];
	static const QString commonPasswords[];

private:
	// private to FileTransferManager only
	friend class FileTransferManager;
	friend class FileTransferWrapper;	
	QString * openTemporaryString();
	void clearTemporaryString();
	bool shouldSaveToFile() const; 

public:
	
	FileTransfer(FileTransferType type, FileTransferEventHandler * handler);
	~FileTransfer();

	// setters
	FileTransfer& setUrl(QString url);
	FileTransfer& setParams(QHash<QString, QString> params);
	FileTransfer& setFileParamKeys(QSet<QString> paramKeys);
	FileTransfer& setContentType(QString type);
	// NOTE: that the common logins/passwords will be set automatically when the url is set
	FileTransfer& setRelativeUrl(FileTransferCommonUrl url, QString file);
	FileTransfer& setLocalPath(QString path);
	FileTransfer& setTemporaryString();
	FileTransfer& setLogin(QString login);
	FileTransfer& setPassword(QString password);
	FileTransfer& setTimeout(int timeout);
	FileTransfer& setRemoteFileName(QString remoteFileName);
	FileTransfer& setUserData(void *userData);

	// getters
	QString getUrl() const;
	QHash<QString, QString> getParams() const;
	QSet<QString> getFileParamKeys() const;
	QString getEncodedParams() const;
	QString getContentType() const;
	QString getLocalPath() const;
	QString getTemporaryString() const;
	QString getLogin() const;
	QString getPassword() const;
	FileTransferType getType() const;
	FileTransferEventHandler * getHandler() const;
	int getTimeout() const;
	QString getRemoteFileName() const;
	void *getUserData() const;

	void cleanup();

	// misc
	bool isValid() const;
};

//Wraps FileTransfer so it can be used as an QObject
// Used for the network manager callback mechanism to authenticate connections
class FileTransferWrapper : public QObject
{
	Q_OBJECT
private:
	FileTransfer _ft;

	//These are used to keep the post data during uploading async
	QByteArray *_postData;
	QBuffer *_postBuffer;
	QNetworkReply *_reply;
	bool _aborted;
	// for handling redirects in async transfers
	bool _isBlocking;
	int _maxNumberOfRedirects;
	QList<FileTransferWrapper*> *_parent;

	//This class is declared here because QNetworkAccessManager can not be declared in FileTransferManager
	//because QObject can only create children running on the same thread.  So we must create one of these for each thread
	//that calls FTM->post/addblock.  Its easier if we just have a manager for each file transfer
	QNetworkAccessManager *_manager;
	friend class FileTransferManager;
public:
	FileTransferWrapper(FileTransfer &ft, QList<FileTransferWrapper*> *parent, bool isBlocking, int maxNumRedirects);
	~FileTransferWrapper();
	FileTransfer& getFileTransfer();
	void abort();
	bool isBlocking() const;
	int getMaxNumberOfRedirects() const;

	// called to clean up after a network request (also called by the finished() slot)
	bool finishedReply(QNetworkReply * reply);

public slots:
	void authenticate(QNetworkReply *reply, QAuthenticator *authenticator);
	void finished(QNetworkReply * reply);
	void error( QNetworkReply::NetworkError  code );

};

// -----------------------------------------------------------------------------

/*
* FileTransfer comparator
*/
struct CompareFileTransfer
{
	bool operator()(const FileTransfer& ft1, const FileTransfer& ft2) const
	{
		if (ft1.getType() != ft2.getType())
			return ft1.getType() < ft2.getType();
		else
		{
			if (ft1.getLocalPath() != ft2.getLocalPath())
				return ft1.getLocalPath() < ft2.getLocalPath();
			else
			{
				if (ft1.getUrl() != ft2.getUrl())
					return ft1.getUrl() < ft2.getUrl();
				else
				{
					if (ft1.getHandler() != ft2.getHandler())
						return ft1.getHandler() < ft2.getHandler();
					else
					{
						return ft1.getEncodedParams() < ft2.getEncodedParams();
					}
				}
			}
		}
	}
};

/*
 * Takes care of transfering multiple files in a non-blocking way.
 * XXX: think about sending multiple files over the same connection for optimization.
 */
class FileTransferManager
{
	HMODULE _hMod;	
	typedef BOOL (__stdcall *InternetCheckConnectionW)(LPCTSTR, DWORD, DWORD);
	typedef BOOL (__stdcall *InternetGetConnectedState)(LPDWORD, DWORD);
	InternetCheckConnectionW _pfnICC;
	InternetGetConnectedState _pfnIGCC;		
	
	// Singleton
	friend class Singleton<FileTransferManager>;
	FileTransferManager();	

private:	
	QList<FileTransferWrapper*> transferList;
	void removeIncompleteFile(const FileTransfer& transfer);	
	friend class FileTransferWrapper;
	

public:
	~FileTransferManager();

	// transfer management	
	bool addTransfer(FileTransfer& transfer, bool blocking = true, int numRedirects = 5);	
	bool addPostTransfer(FileTransfer& transfer, bool blocking, QObject* progressListener = NULL);
	bool hasTransfersToHandler(FileTransferEventHandler * handler);
	void removeTransfersToHandler(FileTransferEventHandler * handler);
	void removeAllTransfers();
	// utility
	QString encodeParams(QHash<QString, QString> params);

	// guess the state of the internet connection
	bool hasInternetConnection(QString url = "http://www.microsoft.com");	
};

// -----------------------------------------------------------------------------

#define ftManager Singleton<FileTransferManager>::getInstance()

// -----------------------------------------------------------------------------

#else
class FileTransferManager;
#endif