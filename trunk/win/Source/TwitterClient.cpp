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
#include "TwitterClient.h"
#include "BT_DialogManager.h"
#include "BT_FileTransferManager.h"
#include "BT_FileSystemManager.h"
#include "BT_OverlayComponent.h"
#include "BT_SceneManager.h"
#include "BT_StatsManager.h"
#include "BT_Util.h"
#include "BT_WindowsOS.h"

/*
Twitter client implementation
*/
TwitterClient::TwitterClient( QString serverUrl )
: _serverUrl(serverUrl)
{}

void TwitterClient::prepareParameters( QHash<QString, QString>& params )
{
	// add the params
	params.insert("source", "bumptop");
}

QString TwitterClient::getHttpRequest( QString method, QHash<QString, QString> params )
{
#ifdef TWITTER_OAUTH
	// necessary for account_verifyCredentials

	QString queryParams = ftManager->encodeParams(params);
	QString serverAddr = QString("http://%1/%2?%3")
		.arg(_serverUrl)
		.arg(method)
		.arg(queryParams);

	// make the call
	FileTransfer ft(FileTransfer::Download, 0);
		ft.setUrl(serverAddr);
		ft.setTemporaryString();
		ft.setLogin(GLOBAL(settings).tw_login);
		ft.setPassword(GLOBAL(settings).tw_password);
		ft.setTimeout(10);
	bool succeeded = ftManager->addBlockingTransfer(ft);
	return ft.getTemporaryString();
#else
	/* help_test does not require a valid login/password
	assert(!GLOBAL(settings).tw_login.isEmpty());
	assert(!GLOBAL(settings).tw_password.isEmpty());
	*/
	QString queryParams = ftManager->encodeParams(params);
	QString serverAddr = QString("http://%1/%2?%3")
		.arg(_serverUrl)
		.arg(method)
		.arg(queryParams);

	// make the call
	FileTransfer ft(FileTransfer::Download, 0);
	ft.setUrl(serverAddr);
	ft.setTemporaryString();
	ft.setLogin(GLOBAL(settings).tw_login);
	ft.setPassword(GLOBAL(settings).tw_password);
	ft.setTimeout(10);
	bool succeeded = ftManager->addTransfer(ft);
	return ft.getTemporaryString();
#endif
}

QString TwitterClient::postHttpRequest( QString method, QHash<QString, QString> params )
{
#if TWITTER_OAUTH
	QString serverAddr = QString("http://%1/%2").arg(_serverUrl).arg(method);
	return _oauthClient.postRequest(serverAddr, "POST", params);
#else
	/* help_test does not require a valid login/password
	assert(!GLOBAL(settings).tw_login.isEmpty());
	assert(!GLOBAL(settings).tw_password.isEmpty());
	*/
	QString serverAddr = QString("http://%1/%2").arg(_serverUrl).arg(method);

	// make the call
	FileTransfer ft(FileTransfer::Download, 0);
	ft.setUrl(serverAddr);
	ft.setParams(params);
	ft.setTemporaryString();
	ft.setLogin(GLOBAL(settings).tw_login);
	ft.setPassword(GLOBAL(settings).tw_password);
	ft.setTimeout(10);
	bool succeeded = ftManager->addPostTransfer(ft, true);
	return ft.getTemporaryString();
#endif
}

bool TwitterClient::initialize()
{
#if TWITTER_OAUTH
	// XXX: do a quick test of the auth key/secret if they are set to see if they are valid

	// prompt for login if we have no twitter oauth access token
	const char * kTwitterCustomerKey = "JCHH0DSC1pbpQ6j0ROYtww";
	const char * kTwitterCustomerSecret = "HlqAAa9IXc42oZ4fxYMISy0Xuwngya2LPaCIHaLn2A";
	// set the saved access tokens
	_oauthClient.setTokens(kTwitterCustomerKey, kTwitterCustomerSecret, 
		GLOBAL(settings).tw_oauth_key, GLOBAL(settings).tw_oauth_secret);

	if (GLOBAL(settings).tw_oauth_key.isEmpty() ||
		GLOBAL(settings).tw_oauth_secret.isEmpty())
	{
		bool gotReqToken = _oauthClient.getRequestToken("http://twitter.com/oauth/request_token", 
			kTwitterCustomerKey, kTwitterCustomerSecret);
		if (gotReqToken)
		{
			_oauthClient.launchAuthorizationUrl("http://twitter.com/oauth/authorize");

			// wait for them to return
			dlgManager->clearState();
			if (dlgManager->promptDialog(DialogTwitterAuth))
			{
				QString pin = dlgManager->getText().trimmed();
				return _oauthClient.getAccessToken("http://twitter.com/oauth/access_token", pin);
			}
		}
		return false;
	}
	return true;
#else
	// prompt for login if we have no twitter login/password
	if (GLOBAL(settings).tw_login.isEmpty() ||
		GLOBAL(settings).tw_password.isEmpty())
	{
		dlgManager->clearState();
		if (dlgManager->promptDialog(DialogTwitterLogin))
		{
			GLOBAL(settings).tw_login = dlgManager->getText();
			GLOBAL(settings).tw_password = dlgManager->getSubText();

			// test this login and password!
			if (account_verifyCredentials())
			{
				winOS->SaveSettingsFile();
			}
			else
			{
				GLOBAL(settings).tw_login.clear();
				GLOBAL(settings).tw_password.clear();
				printUniqueError("TwitterActorImpl", QT_TRANSLATE_NOOP("TwitpicClient", "Invalid login/password"));
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	return true;
#endif
}

QString TwitterClient::getProfileLink()
{
	return QString("http://%1/%2").arg(_serverUrl).arg(GLOBAL(settings).tw_login);
}

bool TwitterClient::statuses_update( QString message )
{
	const int maxUpdateLength = TWITTER_MAX_CHARS;

	if (message.isEmpty())
		return false;

	// http://apiwiki.twitter.com/REST+API+Documentation#update
	QString method = "statuses/update.json";
	QHash<QString, QString> params;
#if TWITTER_OAUTH
	 	params.insert("status", "\"" + message.left(maxUpdateLength) + "\"");
#else
		params.insert("status", message.left(maxUpdateLength));
#endif
	prepareParameters(params);

	// make the function call
	QString result = postHttpRequest(method, params);
	if (!result.isEmpty())
	{
		// parse the json result
		Json::Value root;
		QByteArray tmp = result.toUtf8();
		if (_reader.parse(tmp.constData(), root))
		{
			if (root.isObject() && !root.isMember("error"))
			{
				return true;
			}
		}
	}
	return false;
}

bool TwitterClient::account_verifyCredentials()
{
	// used to test authentication

	// http://apiwiki.twitter.com/REST+API+Documentation#account/verifycredentials
	QString method = "account/verify_credentials.json";
	QHash<QString, QString> params;
	prepareParameters(params);

	// make the function call
	QString result = getHttpRequest(method, params);
	if (!result.isEmpty())
	{
		// parse the json result
		Json::Value root;
		QByteArray tmp = result.toUtf8();
		if (_reader.parse(tmp.constData(), root))
		{
			if (root.isObject() && !root.isMember("error"))
			{
				return true;
			}
		}
	}
	return false;
}

/*
Twitpic client implementation
*/
TwitpicClient::TwitpicClient( QString serverUrl )
: _serverUrl(serverUrl)
, _deferredMessage(false)
{
	_supportedUploadExtensionsContentTypes.insert(".gif", "image/gif");
	_supportedUploadExtensionsContentTypes.insert(".jpg", "image/jpeg");
	_supportedUploadExtensionsContentTypes.insert(".jpeg", "image/jpeg");
	_supportedUploadExtensionsContentTypes.insert(".png", "image/png");
}

TwitpicClient::~TwitpicClient()
{
	ftManager->removeTransfersToHandler(this);
}

void TwitpicClient::prepareParameters( QHash<QString, QString>& params )
{
	assert(!GLOBAL(settings).tw_login.isEmpty());
	assert(!GLOBAL(settings).tw_password.isEmpty());
	params.insert("username", GLOBAL(settings).tw_login);
	params.insert("password", GLOBAL(settings).tw_password);
	params.insert("source", "bumptop");
}

QString TwitpicClient::postHttpUploadReqest( QString method, QHash<QString, QString> params, QString filePath )
{
	QString serverAddr = QString("http://%1/%2").arg(_serverUrl).arg(method);

	// set the params
	QString fileName = filename(filePath);
	QSet<QString> fileParamKeys;
		fileParamKeys.insert("media");
	params.insert("media", filePath);

	// XXX: there seems to be a crash associated with temporary string writing and multi-curl download?
	// make the call
	FileTransfer ft(FileTransfer::Upload, this);
		ft.setUrl(serverAddr);
		ft.setParams(params);
		ft.setFileParamKeys(fileParamKeys);
		ft.setTemporaryString();
		ft.setTimeout(60);
	bool succeeded = ftManager->addPostTransfer(ft, false);
	return QString();
}

bool TwitpicClient::initialize()
{
	// unfortunately, requires basic auth where as twitter uses oauth
	// prompt for login if we have no twitter login/password
	TwitterClient client(TWITTER_SERVER);

#if TWITTER_OAUTH
	if (GLOBAL(settings).tw_login.isEmpty() ||
		GLOBAL(settings).tw_password.isEmpty())
	{
		dlgManager->clearState();
		if (dlgManager->promptDialog(DialogTwitterLogin))
		{
			GLOBAL(settings).tw_login = dlgManager->getText();
			GLOBAL(settings).tw_password = dlgManager->getSubText();

			// test this login and password!
			if (client.account_verifyCredentials())
			{
				winOS->SaveSettingsFile();
			}
			else
			{
				GLOBAL(settings).tw_login.clear();
				GLOBAL(settings).tw_password.clear();
				return false;
			}
		}
		else
		{
			return false;
		}
	}
#endif

	return client.initialize();
}

QString TwitpicClient::getProfileLink()
{
	return QString("http://%1/%2").arg(TWITTER_SERVER).arg(GLOBAL(settings).tw_login);
}

bool TwitpicClient::validate_uploadAndPost( QString message, QString& photo )
{
	// ensure not empty
	if (photo.isEmpty())
		return false;

	// check if the extensions are supported
	QString ext = fsManager->getFileExtension(photo);
	if (!_supportedUploadExtensionsContentTypes.contains(ext))
	{
		return false;
	}

	return true;
}

bool TwitpicClient::uploadAndPost(QString message, QString path)
{
	const int maxUpdateLength = TWITTER_MAX_CHARS;

	// http://twitpic.com/api.do
	QString method = "uploadAndPost";
	QHash<QString, QString> params;
		params.insert("message", message.left(maxUpdateLength));
	prepareParameters(params);

	// XXX: there is currently an issue with multipart curl and uploads
	// make the function call
	QString result = postHttpUploadReqest(method, params, path);
	printTimedUnique("TwitpicClient::photos_upload", 600, QT_TR_NOOP("Updating twitter with image..."));

	return true;
}

bool TwitpicClient::uploadAndPostDeferMessage(QString path)
{
	const int maxUpdateLength = TWITTER_MAX_CHARS;
	_deferredMessage = true;

	// http://twitpic.com/api.do
	QString method = "upload";
	QHash<QString, QString> params;
	prepareParameters(params);

	QString result = postHttpUploadReqest(method, params, path);
	printTimedUnique("TwitpicClient::photos_upload", 600, QT_TR_NOOP("Sending image to twitpic first..."));

	return true;
}

void TwitpicClient::onTransferComplete( const FileTransfer& transfer )
{
	bool launchTwitterPage = true;
	bool uploadError = false;
	bool cancelTweet = false;

	if (_deferredMessage)
	{
		QString xml = transfer.getTemporaryString();

		// extract the url from the xml results
		QRegExp urlRe("<mediaurl>([^<]*)</mediaurl>");
				urlRe.setCaseSensitivity(Qt::CaseInsensitive);
		if (urlRe.indexIn(xml) > -1)
		{
			QStringList matches = urlRe.capturedTexts();
			if ((matches.size() > 1) && !matches[1].isEmpty())
			{
				QString twitPicUrl = matches[1];

				// prompt for the tweet
				dlgManager->clearState();
				dlgManager->setCaption(QT_TR_NOOP("Update Twitter as %1").arg(GLOBAL(settings).tw_login));
				dlgManager->setPrompt(QT_TR_NOOP("Tell the world more about this image (%1):").arg(filename(transfer.getParams()["media"])));
				dlgManager->setText("\n" + twitPicUrl);
				if (dlgManager->promptDialog(DialogTwitterTweet))
				{
					QString message = dlgManager->getText();


					TwitterClient client(TWITTER_SERVER);
					client.initialize();
					if (client.statuses_update(message))
					{
						statsManager->getStats().bt.interaction.actors.custom.twitter.droppedOn++;
					}
				}
				else
				{
					launchTwitterPage = false;
					uploadError = false;
					cancelTweet = true;
					dismiss("TwitpicClient::photos_upload");
				}
			}
			else
			{
				launchTwitterPage = false;
				uploadError = true;
			}
		}
		else
		{
			launchTwitterPage = false;
			uploadError = true;
		}
	}

	if (uploadError)
	{
		printUniqueError("TwitpicClient::photos_upload", QT_TR_NOOP("Tweet failed"));
	}
	else
	{
		if(!cancelTweet) printUnique("TwitpicClient::photos_upload", QT_TR_NOOP("Tweet sent!"));
		if (launchTwitterPage)
			fsManager->launchFileAsync(getProfileLink());
	}

	_deferredMessage = false;
}

void TwitpicClient::onTransferError( const FileTransfer& transfer )
{
	printUniqueError("TwitpicClient::photos_upload", QT_TR_NOOP("Tweet failed"));
	_deferredMessage = false;
}