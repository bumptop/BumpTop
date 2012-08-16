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

#ifndef TWITTERCLIENT_H
#define TWITTERCLIENT_H

#include "BT_FileTransferManager.h"
#include "OAuthClient.h"

#define TWITTER_MAX_CHARS 140
#define TWITTER_SERVER "twitter.com"
#define TWITPIC_SERVER "twitpic.com/api"

/*
The twitter api uses basic HTTP auth
*/
class TwitterClient
{
	QString _serverUrl;
	Json::Reader _reader;
	OAuthClient _oauthClient;

private:
	void prepareParameters(QHash<QString, QString>& params);
	QString getHttpRequest(QString method, QHash<QString, QString> params);
	QString postHttpRequest(QString method, QHash<QString, QString> params);

public:
	TwitterClient(QString serverUrl);

	// initialization
	bool initialize();
	QString getProfileLink();
	
	// twitter api calls
	bool statuses_update(QString message);
	bool account_verifyCredentials();
};

/*
Twitpic doesn't even use any kind of auth scheme...
*/
class TwitpicClient : public FileTransferEventHandler
{
	Q_DECLARE_TR_FUNCTIONS(TwitpicClient);

	QString _serverUrl;
	bool _deferredMessage;

	Json::Reader _reader;
	QHash<QString, QString> _supportedUploadExtensionsContentTypes;

private:
	void prepareParameters(QHash<QString, QString>& params);
	QString postHttpUploadReqest(QString method, QHash<QString, QString> params, QString filePath);

public:
	TwitpicClient(QString serverUrl);
	~TwitpicClient();

	// initialization
	bool initialize();
	QString getProfileLink();

	// twitpic api calls
	bool validate_uploadAndPost(QString message, QString& photo);
	bool uploadAndPost(QString message, QString filePath);
	bool uploadAndPostDeferMessage(QString filePath);

	// events
	virtual void onTransferComplete(const FileTransfer& transfer);
	virtual void onTransferError(const FileTransfer& transfer);
};

#endif // TWITTERCLIENT_H