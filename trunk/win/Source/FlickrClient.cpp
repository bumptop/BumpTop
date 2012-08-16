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
#include "FlickrClient.h"
#include "BT_DialogManager.h"
#include "BT_FileTransferManager.h"
#include "BT_FileSystemManager.h"
#include "BT_OverlayComponent.h"
#include "BT_SceneManager.h"
#include "BT_StatsManager.h"
#include "BT_Util.h"
#include "BT_WindowsOS.h"
#include "BT_QtUtil.h"
#include "BT_Settings.h"
#include "BT_GLTextureManager.h"


FlickrClient::FlickrClient() :
_numPhotosToUpload(0),
_numPhotosFailedToUpload(0)
{
	_authToken = GLOBAL(settings).flickr_auth_token;
}

FlickrClient::FlickrClient(QString authToken) :
_numPhotosToUpload(0),
_numPhotosFailedToUpload(0)
{
	_authToken = authToken;
}

/*
* This method is used to go through the necessary steps to authenticate
* a user. 
*/
bool FlickrClient::authenticate()
{
	// prompt for login if we have no twitter login/password
	QString method = "auth/";

	// Get Frob
	_frob = requestAFrob();

	// Create login link
	QString loginLink = createLoginLink();
	fsManager->launchFile(loginLink);
	
	// Launch prompt to see if user allowed bumptop to access their flickr account
	QString msg = QString(WinOSStr->getString("AuthFlickrUser"));
	int rc = MessageBox(winOS->GetWindowsHandle(), (LPCWSTR) msg.utf16(), (LPCTSTR) QT_TR_NOOP("Authorize Flickr").utf16(), MB_YESNO);
	
	if (rc == IDYES)
	{
		// Get Auth Token
		getAuthToken();
	}
	else
	{
		printUnique ("FlickrClient::authenticate", QT_TR_NOOP("User cancelled operation"));
	}
	return true;
}

/*
* Most flickr api calls require an API Signature. 
* This method takes the parameters being sent to Flickr
* and constructs an API Signature for that method call
*/
QString FlickrClient::getAPISignature(QHash<QString, QString> *params)
{
	QString preMD5ApiSignature;
	QStringList result;

	// create a list of key-value pairs
	QHashIterator<QString, QString> iter(*params);
	while (iter.hasNext())
	{
		iter.next();
		result.append(QString("%1%2").arg(iter.key()).arg(iter.value()));
	}

	// We need the parameters in sorted order
	result.sort();

	// Out api signature needs the FLICKR_SECRET_KEY to be at the beginning
	preMD5ApiSignature = result.join("").prepend(FLICKR_SECRET_KEY);

	// Then we simply run an MD5 hash and use that as our API signature
	return QCryptographicHash::hash(preMD5ApiSignature.toUtf8(), QCryptographicHash::Md5).toHex();
}

/*
* Frob is something flickr requires to identify a login session.
* When authenticating a user the first thing we need to do is request a frob 
* for this login session.
*/
QString FlickrClient::requestAFrob()
{
	QString frob;
	QHash<QString, QString> params;

	// Construct method
	QString getFrobMethod = "flickr.auth.getFrob";

	// Add params
	params.insert("method", getFrobMethod);

	Json::Value root = executeAPICall(FLICKR_API_SERVER, &params);
	if (!root.isNull())
	{
		std::string filePath = "frob._content";
		Json::Value val = getValueFromRoot(filePath.c_str(), root);
		_frob = qstring(val.asString());
		return _frob;
	}
	return "";
}

/*
* This method will return a login link to send the user
* so that they can let bumptop use their flickr account to make authenticated API calls to flickr
*/
QString FlickrClient::createLoginLink()
{	
	// Auth Server
	QString server = "flickr.com/services/auth/?";
	
	// Add Params
	QHash<QString, QString> params;
	params.insert("perms", "write"); // Need write permission to upload photos
	params.insert("frob", _frob);
	params.insert("api_key", FLICKR_API_KEY);
	params.insert("api_sig", getAPISignature(&params));
	QString queryParams = ftManager->encodeParams(params);
	QString serverAddr = QString("http://%1&%2")
		.arg(server)
		.arg(queryParams);
	// determine login link or send user to a url to login and allow bumptop access
	// to their flickr photos fs -> launch____
	return serverAddr;
}

/*
* After allowing a bumptop to make flickr API calls on behalf of 
* a user, getAuthToken will store the authoToken for a specific user. 
* Currently we are using the BumptopFlickr account's auth token to make
* authenticated calls
*/
void FlickrClient::getAuthToken()
{
	QHash<QString, QString> params;
	params.insert("method", "flickr.auth.getToken");
	params.insert("frob", _frob);

	Json::Value root = executeAPICall(FLICKR_API_SERVER, &params);
	if (!root.isNull())
	{
		std::string filePath = "auth.token._content";
		Json::Value val = getValueFromRoot(filePath.c_str(), root);
		_authToken = qstring(val.asString());
		GLOBAL(settings).flickr_auth_token = _authToken;
		winOS->SaveSettingsFile();
	}

}

/*
* Make an HTTP post request and return a string 
* representing the response in json format
*/
QString FlickrClient::postHttpRequest(QString url, QHash<QString, QString> *params, QString filePath, FileTransferEventHandler * handler)
{
	QString queryParams = ftManager->encodeParams(*params);
	QString serverAddr = QString("%1?%2")
		.arg(url)
		.arg(queryParams);

	QString fileName = filename(filePath);
	QSet<QString> fileParamKeys;
	fileParamKeys.insert("photo");
	params->insert("photo", filePath);

	FileTransfer ft(FileTransfer::Upload, (handler ? handler : this));
	ft.setUrl(serverAddr);
	ft.setParams(*params);
	ft.setFileParamKeys(fileParamKeys);
	ft.setTemporaryString();
	ft.setTimeout(180);

	bool succeeded = ftManager->addPostTransfer(ft, false);
	return ft.getTemporaryString();
}

/*
* Make an http request and return a string 
* representing the response in json format
*/
QString FlickrClient::getHttpRequest(QString server, QHash<QString, QString> *params, FileTransferEventHandler * handler)
{
	QString queryParams = ftManager->encodeParams(*params);
	QString serverAddr = QString("http://%1&%2")
		.arg(server)
		.arg(queryParams);

	// make the call
	FileTransfer ft(FileTransfer::Download, handler);
	ft.setUrl(serverAddr);
	ft.setTemporaryString();
	ft.setTimeout(10);
	ftManager->addTransfer(ft, (handler ? false : true));
	return ft.getTemporaryString();
}

/*
* Given a series of params execute a GET http request and return
* a set of photo urls from the response.
*/
Json::Value FlickrClient::executeAPICall(QString server, QHash<QString, QString> *params, bool useGetHttp)
{
	// Set the format of the api call to be Json
	params->insert(QT_NT("format"), QT_NT("json"));

	// Remove the flickrJson() function name at the beginning of the JSON response
	params->insert(QT_NT("nojsoncallback"), QT_NT("1"));

	// Add API key, API Signature, and API Auth Token
	params->insert(QT_NT("api_key"), FLICKR_API_KEY);
	if (!_authToken.isEmpty())
		params->insert(QT_NT("auth_token"), _authToken);
	
	params->insert(QT_NT("api_sig"), getAPISignature(params));
	// Get HTTP Response
	QString httpResponse;
	if (useGetHttp)
		httpResponse = getHttpRequest(server, params);
	else
	{
		httpResponse = postHttpRequest(server, params, _currentFileToUpload);
	}
	return responseToJson(httpResponse);
}
void FlickrClient::executeAPICallCustom(QString server, QHash<QString, QString> *params, bool useGetHttp, FileTransferEventHandler * handler)
{
	// Set the format of the api call to be Json
	params->insert("format", "json");

	// Remove the flickrJson() function name at the beginning of the JSON response
	params->insert("nojsoncallback", "1");

	// Add API key, API Signature, and API Auth Token
	params->insert("api_key", FLICKR_API_KEY);
	if (!_authToken.isEmpty())
		params->insert("auth_token", _authToken);
	
	params->insert("api_sig", getAPISignature(params));
	// Get HTTP Response
	QString httpResponse;
	if (useGetHttp)
		getHttpRequest(server, params, handler);
	else
		postHttpRequest(server, params, _currentFileToUpload, handler);
}

/*
* Given a Json value of photo objects
* Construct a series of urls to photos
*/
vector<QString> FlickrClient::extractPhotoUrls(const FileTransfer& transfer)
{
	vector<QString> urls;
	Json::Value root = responseToJson(transfer.getTemporaryString());
	if (!root.isNull())
	{
		Json::Value arrayOfPhotos = getValueFromRoot("photos.photo", root);
		Json::Value photo, id, secret, server, farm;
		if (arrayOfPhotos.isArray())
		{
			for (uint i = 0; i < arrayOfPhotos.size(); i++)
			{
				// Flickr Photo URLs have the following format
				// http://farm{farm-id}.static.flickr.com/{server-id}/{id}_{secret}_[mstb].jpg
				// Where [mstb] is size. Leaving [mstb] blank will get a medium sized format. Flickr's
				// definition of medium is 500px wide.
				// Go here to see more details about it
				// http://www.flickr.com/services/api/misc.urls.html
				photo = arrayOfPhotos[i];
				id = getValueFromRoot("id", photo);
				secret = getValueFromRoot("secret", photo);
				server = getValueFromRoot("server", photo);
				farm = getValueFromRoot("farm", photo);
				QString url = QString(QT_NT("http://farm%1.static.flickr.com/%2/%3_%4.jpg"))
					.arg(farm.asInt())
					.arg(qstring(server.asString()))
					.arg(qstring(id.asString()))
					.arg(qstring(secret.asString()));
				urls.push_back(url);
			}
		}
	}
	return urls;
}

/*
* Get the latest batch of photos from a specific
* user_id or group_id
*/
void FlickrClient::requestPhotosFromId( QString id, bool isGroupId, FileTransferEventHandler * handler)
{
	QHash<QString, QString> params;
	params.insert("method", "flickr.photos.search");
	if (isGroupId)
		params.insert("group_id", id);
	else
		params.insert("user_id", id);
	params.insert("page", "1");
	params.insert("per_page", "20");

	executeAPICallCustom(FLICKR_API_SERVER, &params, true, handler);
}

/*
* Given a user id, get their 20 most recent favourite photos
*/
void FlickrClient::requestFavouritePhotosFromId(QString id, FileTransferEventHandler * handler)
{
	QHash<QString, QString> params;
	params.insert("method", "flickr.favorites.getList");
	params.insert("user_id", id);
	params.insert("page", "1");
	params.insert("per_page", "20");
	
	executeAPICallCustom(FLICKR_API_SERVER, &params, true, handler);
}

/*
*	Get the 20 most recent photos for a specific tag
*/
void FlickrClient::requestPhotosByTag(QString tag, FileTransferEventHandler * handler)
{
	QHash<QString, QString> params;
	params.insert("method", "flickr.photos.search");
	params.insert("tags", tag);
	params.insert("page", "1");
	params.insert("per_page", "20");

	executeAPICallCustom(FLICKR_API_SERVER, &params, true, handler);
}

/*
* Get the username for a specific user_id
*/
QString FlickrClient::getUsername(QString id)
{
	QHash<QString, QString> params;
	params.insert("method", "flickr.people.getInfo");
	params.insert("user_id", id);

	Json::Value root = executeAPICall(FLICKR_API_SERVER, &params);
	QString username;
	if (!root.isNull())
	{
		username =	qstring(getValueFromRoot("person.username._content", root).asString());
	}
	return username;
}

/*
* Get the group name for a specific group_id
*/
QString FlickrClient::getGroupName(QString id)
{
	QHash<QString, QString> params;
	params.insert("method", "flickr.groups.getInfo");
	params.insert("group_id", id);
	params.insert("auth_token", _authToken);

	Json::Value root = executeAPICall(FLICKR_API_SERVER, &params);
	QString groupName;
	if (!root.isNull())
	{
		groupName =	qstring(getValueFromRoot("group.name._content", root).asString());
	}
	return groupName;
}

bool FlickrClient::uploadPhotos( const vector<FileSystemActor *> &files )
{
	QString msg;
	if (files.size() > 1)
		msg = QT_TR_NOOP("Are you sure you want to upload these photos?");
	else
		msg = QT_TR_NOOP("Are you sure you want to upload this photo?");

	int rc = MessageBox(GetForegroundWindow(), (LPCWSTR) msg.utf16(), (LPCTSTR) QT_TR_NOOP("Upload Photos to Flickr").utf16(), MB_YESNO);
	if (rc == IDYES)
	{
		// To upload photos to flickr we use an HTTP Post
		QString server = "http://api.flickr.com/services/upload/";
		QHash<QString, QString> params;
		_numPhotosToUpload += files.size();
		for (int i = 0;i<files.size();i++)
		{
			// Specify a bumptop tag
			params.insert("tags", "BumpTop");

			// Get the full path of the file to upload
			_currentFileToUpload = files[i]->getFullPath();

			// Execute API call
			executeAPICall(server, &params, false);

			// Clear all params
			params.clear();
		}
		printTimedUnique("FlickrClient::uploadPhotos", 9999, QT_TR_NOOP("Uploading Photos"));
		return true;
	}
	// http://www.flickr.com/services/api/upload.api.html
	return false;
}

void FlickrClient::onTransferComplete( const FileTransfer& transfer )
{
	bool uploadFailed = false;
	QString editPage = "http://www.flickr.com/tools/uploader_edit.gne?ids=";
	QString response = transfer.getTemporaryString();
	QStringRef responseTag, comments;
	QString responseValue;
	QXmlStreamReader xmlReader(response);
	QXmlStreamReader::TokenType tokenType;

	// Decrement the number of photos to upload
	_numPhotosToUpload--;

	// Parse XML Response
	while(!xmlReader.atEnd())
	{
		tokenType = xmlReader.readNext();
		if (tokenType == QXmlStreamReader::StartElement)
		{
			responseTag = xmlReader.name();
			comments = xmlReader.text();
			if (responseTag == "photoid")
 			{
				responseValue = xmlReader.readElementText();
 				_photoIds.push_back(responseValue);
 			}
			else if (responseTag == "err")
			{
				_numPhotosFailedToUpload++;
			}
		}
	}

	if (_numPhotosToUpload == 0)
	{
		// Remove Uploading photos message
		if (scnManager->messages()->getMessage("FlickrClient::uploadPhotos") != NULL)
			dismiss("FlickrClient::uploadPhotos");

		if (_photoIds.size() > 0)
		{
			// Since there are no more photos to upload we will construct the edit_photos page url
			// start by concatenating all the photo ids, separated by commas
			for (int i = 0;i<_photoIds.size();i++)
			{
				editPage.append(_photoIds[i]);
				if (i + 1 < _photoIds.size())
					editPage.append(",");
			}

			// Print message indicating if transfer succeeded
			if (_numPhotosFailedToUpload > 0)
			{
				QString errMsg;
				if (_numPhotosFailedToUpload == 1)
					errMsg = QT_TR_NOOP("Could not upload photo");
				else
					errMsg = QT_TR_NOOP("%1 photos failed to upload").arg(QString::number(_numPhotosFailedToUpload));

				printUniqueError("FlickrClient::onTransferComplete", errMsg);
			}
			else
				printUnique("FlickrClient::onTransferComplete", QT_TR_NOOP("Transfer Complete!"));

			// Send user to edit page
			fsManager->launchFileAsync(editPage);

			// Clear list of photo ids and reset counter
			_photoIds.clear();
			_numPhotosFailedToUpload = 0;
		}		
	}
}

void FlickrClient::onTransferError( const FileTransfer& transfer )
{
	// The transfer failed so inform user that something went wrong

	QString response = transfer.getTemporaryString();
	_numPhotosToUpload--;
	_numPhotosFailedToUpload++;
	printUnique("FlickrClient::onTransferError", QT_TR_NOOP("Transfer Failed. Try again"));
}

Json::Value FlickrClient::responseToJson(const QString& response)
{
	Json::Value root;

	// Check if the http response was valid
	QByteArray tmp = response.toUtf8();
	QString status;
	if (_reader.parse(tmp.constData(), root))
	{
		if (root.isObject())
		{
			// All Flickr API calls have a stat value. If the call was successful
			// stat will equal ok
			status = qstring(getValueFromRoot("stat", root).asString());
			if (status == "ok")
				return root;
		}
	}
	return Json::Value::null;
}