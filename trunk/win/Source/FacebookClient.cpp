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
#include "FacebookClient.h"
#include "BT_DialogManager.h"
#include "BT_FacebookWizard.h"
#include "BT_FileTransferManager.h"
#include "BT_FileSystemManager.h"
#include "BT_GLTextureManager.h"
#include "BT_JavaScriptAPI.h"
#include "BT_OverlayComponent.h"
#include "BT_QtUtil.h"
#include "BT_Settings.h"
#include "BT_SceneManager.h"
#include "BT_WindowsOS.h"
#include "BT_Util.h"

#define FB_APIKEY "8287b0dc1c727df0a393da413ee2e789"
#define FB_SECRETKEY "d8808715f329aa2b855c3ac825bef3db"
#define FB_SERVERURL "api.facebook.com/restserver.php"

FacebookPhoto::FacebookPhoto( QString path )
: filePath(path)
{}

FacebookPhoto::FacebookPhoto( QString path, QString cap )
: filePath(path)
, caption(cap)
{}

FacebookPhoto::FacebookPhoto( QString path, QString cap, QString alb )
: filePath(path)
, caption(cap)
, album(alb)
{}

QString FacebookStatus::toJsonString()
{
	Json::Value value;
		value["time"] = (int) time.toTime_t();
		value["message"] = stdString(message);
	return qstring(value.toStyledString());
}

Json::Value FacebookWidgetNewsFeedItem::toJsonValue() 
{
	Json::Value value;
		value["postId"] = stdString(postId);
		value["message"] = stdString(message);
		value["sourceId"] = stdString(sourceId);
		value["title"] = stdString(title);
		value["permalink"] = stdString(permalink);
		value["extlink"] = stdString(extlink);
		value["domain"] = stdString(domain);
		value["description"] = stdString(description);
		value["ownerName"] = stdString(ownerName);
		value["ownerProfileLink"] = stdString(ownerProfileLink);
		value["ownerPicSquare"] = stdString(ownerPicSquare);
		value["creationDate"] = (int) creationDate.toTime_t();
		value["photoId"] = stdString(photoId);
		value["imageAlt"] = stdString(imageAlt);
		value["imageSrc"] = stdString(imageSrc);
		value["imageWidth"] = imageWidth;
		value["imageHeight"] = imageHeight;
		value["numComments"] = numComments;
		value["numLikes"] = numLikes;
	return value;
}

QString FacebookWidgetNewsFeed::toJsonString()
{
	if (items.isEmpty())
		return "[]";
	else
	{	
		Json::Value value;
		for (int i = 0; i < items.size(); ++i)
			value.append(items[i]->toJsonValue());
		return qstring(value.toStyledString());
	}
}

Json::Value FacebookWidgetAlbumItem::toJsonValue()
{
	Json::Value value;
		value["aid"] = stdString(aid);
		value["uid"] = stdString(uid);
		value["title"] = stdString(title);
		value["permalink"] = stdString(permalink);
		value["albumCoverUrl"] = stdString(albumCoverUrl);
		value["albumCoverWidth"] = albumCoverWidth;
		value["albumCoverHeight"] = albumCoverHeight;
		value["modifiedDate"] = modifiedDate;
		value["numPhotos"] = numPhotos;
		value["numComments"] = numComments;
	return value;
}

QString FacebookWidgetPhotoAlbums::toJsonString()
{
	if (albums.isEmpty())
		return "[]";
	else
	{	
		Json::Value value;
		for (int i = 0; i < albums.size(); ++i)
			value.append(albums[i]->toJsonValue());
		return qstring(value.toStyledString());
	}
}

Json::Value FacebookWidgetPhoto::toJsonValue()
{
	Json::Value value;
		value["pid"] = stdString(pid);
		value["aid"] = stdString(aid);
		value["owner"] = stdString(owner);
		value["title"] = stdString(title);
		value["permalink"] = stdString(permalink);
		value["url"] = stdString(url);
		value["width"] = width;
		value["height"] = height;
		value["numComments"] = numComments;
		value["creationDate"] = creationDate;
	return value;
}
QString FacebookWidgetPhotos::toJsonString()
{
	if (photos.isEmpty())
		return "[]";
	else
	{	
		Json::Value value;
		for (int i = 0; i < photos.size(); ++i)
			value.append(photos[i]->toJsonValue());
		return qstring(value.toStyledString());
	}
}

Json::Value FacebookWidgetPage::toJsonValue()
{
	Json::Value value;
	value["pageId"] = stdString(pageId);
	value["pageName"] = stdString(name);
	value["pageUrl"] = stdString(pageUrl);
	return value;
}
QString FacebookWidgetPages::toJsonString()
{
	if (pages.isEmpty())
		return "[]";
	else
	{	
		Json::Value value;
		for (int i = 0; i < pages.size(); ++i)
			value.append(pages[i]->toJsonValue());
		return qstring(value.toStyledString());
	}
}

// ----------------------------------------------------------------------------

QString FacebookClient::generateSignature( QHash<QString, QString> params, QString secretKey )
{
	// SEE: http://wiki.developers.facebook.com/index.php/How_Facebook_Authenticates_Your_Application
	QStringList result;

	// create a list of key-value pairs
	QHashIterator<QString, QString> iter(params);
	while (iter.hasNext())
	{
		iter.next();
		result.append(QString("%1=%2").arg(iter.key()).arg(iter.value()));
	}

	// sort the list of pairs
	result.sort();

	// md5 the pairs string with the secret key appended
	QString sig = result.join("").append(secretKey);
	return QCryptographicHash::hash(sig.toUtf8(), QCryptographicHash::Md5).toHex();
}

void FacebookClient::prepareSessionParameters( QHash<QString, QString>& params )
{
	params.insert("api_key", _apiKey);
	params.insert("v", _version);
	params.insert("call_id", QString::number(QDateTime::currentDateTime().toTime_t()));
	params.insert("session_key", _sessionKey);
	params.insert("format", "JSON");
	params.insert("sig", generateSignature(params, _sessionSecretKey));
}

void FacebookClient::prepareSessionlessParameters( QHash<QString, QString>& params )
{
	params.insert("api_key", _apiKey);
	params.insert("v", _version);
	params.insert("format", "JSON");
	params.insert("sig", generateSignature(params, _secretKey));
}

QString FacebookClient::postHttpReqest( QString method, QHash<QString, QString> params )
{
	QString serverAddr = "http://" + _serverUrl;

	// make the call
	FileTransfer ft(FileTransfer::Download, 0);
		ft.setUrl(serverAddr);
		ft.setParams(params);
		ft.setTemporaryString();
		ft.setTimeout(30);
	bool succeeded = ftManager->addPostTransfer(ft, true);
	return ft.getTemporaryString();
}

void FacebookClient::postHttpReqest( QString method, QHash<QString, QString> params, const QString& callback, FileTransferEventHandler * handler )
{
	QString serverAddr = "http://" + _serverUrl;

	// make the call
	FileTransfer ft(FileTransfer::Download, handler);
		ft.setUrl(serverAddr);
		ft.setParams(params);
		ft.setTemporaryString();
		ft.setTimeout(30);
		ft.setUserData(new QString(callback));
	ftManager->addPostTransfer(ft, false);
}

QString FacebookClient::postHttpUploadRequest( QString method, QHash<QString, QString> params, QString filePath )
{
	QString serverAddr = "http://" + _serverUrl;

	// set the params
	QString fileName = filename(filePath);
	QSet<QString> fileParamKeys;
		fileParamKeys.insert(fileName);
	params.insert(fileName, filePath);

	// make the call
	FileTransfer ft(FileTransfer::Upload, this);
		ft.setUrl(serverAddr);
		ft.setParams(params);
		ft.setFileParamKeys(fileParamKeys);
		ft.setTemporaryString();
		ft.setTimeout(60);
	bool succeeded = ftManager->addPostTransfer(ft, false);
	return ft.getTemporaryString();
}

void FacebookClient::postHttpUploadRequest( QString method, QHash<QString, QString> params, QString filePath, const QString& callback, FileTransferEventHandler * handler )
{
	QString serverAddr = "http://" + _serverUrl;

	// set the params
	QString fileName = filename(filePath);
	QSet<QString> fileParamKeys;
		fileParamKeys.insert(fileName);
	params.insert(fileName, filePath);

	// make the call
	FileTransfer ft(FileTransfer::Upload, handler);
		ft.setUrl(serverAddr);
		ft.setParams(params);
		ft.setFileParamKeys(fileParamKeys);
		ft.setTemporaryString();
		ft.setTimeout(300);
		ft.setUserData(new QString(callback));
	ftManager->addPostTransfer(ft, false);
}

QString FacebookClient::postHttpsRequest( QString method, QHash<QString, QString> params )
{
	QString serverAddr = "https://" + _serverUrl;

	// make the call
	FileTransfer ft(FileTransfer::Download, 0);
		ft.setUrl(serverAddr);
		ft.setParams(params);
		ft.setTemporaryString();
		ft.setTimeout(30);
	bool succeeded = ftManager->addPostTransfer(ft, true);
	return ft.getTemporaryString();
}

FacebookClient::FacebookClient()
: _apiKey(FB_APIKEY)
, _secretKey(FB_SECRETKEY)
, _serverUrl(FB_SERVERURL)
, _version("1.0")
, _numPhotosUploaded(0)
{
	// http://www.iana.org/assignments/media-types/image/
	// http://www.w3schools.com/media/media_mimeref.asp
	_supportedUploadExtensionsContentTypes.insert(".gif", "image/gif");
	_supportedUploadExtensionsContentTypes.insert(".jpg", "image/jpeg");
	_supportedUploadExtensionsContentTypes.insert(".jpeg", "image/jpeg");
	_supportedUploadExtensionsContentTypes.insert(".png", "image/png");
	_supportedUploadExtensionsContentTypes.insert(".psd", "image/vnd.adobe.photoshop");
	_supportedUploadExtensionsContentTypes.insert(".tiff", "image/tiff");
	_supportedUploadExtensionsContentTypes.insert(".tif", "image/tiff");
	_supportedUploadExtensionsContentTypes.insert(".jp2", "image/jp2");
	_supportedUploadExtensionsContentTypes.insert(".iff", "image/iff");
	_supportedUploadExtensionsContentTypes.insert(".bmp", "image/bmp");
	_supportedUploadExtensionsContentTypes.insert(".wbmp", "image/vnd.wap.wbmp");
	_supportedUploadExtensionsContentTypes.insert(".xbm", "image/x-xbitmap");
}

FacebookClient::~FacebookClient()
{
	ftManager->removeTransfersToHandler(this);
}

bool FacebookClient::hasAuthorized() const 
{
	return (!_userid.isEmpty() && !_sessionKey.isEmpty() && !_sessionSecretKey.isEmpty());
}

bool FacebookClient::initialize(bool& usingSavedSession)
{
	// first, check if we have a valid session
	
	// load the saved session keys
	_userid = GLOBAL(settings).fbc_uid;
	_sessionKey = GLOBAL(settings).fbc_session;
	_sessionSecretKey = GLOBAL(settings).fbc_secret;
	if (!_userid.isEmpty() && 
		!_sessionKey.isEmpty() && 
		!_sessionSecretKey.isEmpty())
	{
		bool dummy = false;
		if (users_isAppUser(dummy))
		{
			usingSavedSession = true;
			return true;
		}
	}
	return false;
}

const QString& FacebookClient::getUid() const
{
	return _userid;
}

QString FacebookClient::extractUidFromJsonResult(const QString& key, const QString& jsonSrc) const
{
	// NOTE: we manually extract the Facebook UID, since jsoncpp lib can't handle
	//	64 bit integers
	QString pattern;

	// try quoted
	{
		pattern = QString("\"%1\"\\s*:\\s*\"(\\d+)\"").arg(key);
		QRegExp uidRegex(pattern);
		if (uidRegex.indexIn(jsonSrc) > -1)
		{
			QStringList uidMatches = uidRegex.capturedTexts();
			if (uidMatches.size() > 1)
			{
				return uidMatches[1];
			}
		}	
	}

	// try unquoted
	{
		pattern = QString("\"%1\"\\s*:\\s*(\\d+)").arg(key);
		QRegExp uidRegex(pattern);
		if (uidRegex.indexIn(jsonSrc) > -1)
		{
			QStringList uidMatches = uidRegex.capturedTexts();
			if (uidMatches.size() > 1)
			{
				return uidMatches[1];
			}
		}	
	}

	return QString();
}

int FacebookClient::extractIntFromJsonResult(const QString& key, const Json::Value& value) const 
{
	// NOTE: we manually extract the Facebook integers, since they retardedly return quoted 
	// numbers in one case and not the other
	std::string keyStr = stdString(key);
	if (value.isMember(keyStr))
	{
		// try extracting int
		if (value[keyStr].isNumeric())
			return value[keyStr].asInt();
		else if (value[keyStr].isString()) 
			return qstring(value[keyStr].asString()).toInt();
	}
	return 0;
}

QString FacebookClient::extractStringFromJsonResult(const QString& key, const Json::Value& value) const
{
	// Goddamn, facebook's fql result sets are retarded.  It sometimes returns
	// strings as ints and ints as strings.
	std::string keyStr = stdString(key);
	if (value.isMember(keyStr))
	{
		// try extracting int
		if (value[keyStr].isNumeric())
			return QString::number(value[keyStr].asInt());
		else if (value[keyStr].isString()) 
			return qstring(value[keyStr].asString());
	}
	return 0;
}

bool FacebookClient::fbconnect_login(JavaScriptAPI * jsApiRef)
{
	// http://wiki.developers.facebook.com/index.php/Authorization_and_Authentication_for_Desktop_Applications
	QString successUrl = "http://www.facebook.com/connect/login_success.html";
	QString cancelUrl = "http://www.facebook.com/connect/login_failure.html";
	QHash<QString, QString> params;
		params.insert("api_key", _apiKey);
		params.insert("v", _version);
		params.insert("fbconnect", "true");
		params.insert("connect_display", "popup");
		params.insert("return_session", "true");		
		params.insert("next", successUrl);
		params.insert("cancel_url", cancelUrl);
		params.insert("req_perms", "publish_stream,read_stream,offline_access");
		params.insert("sig", generateSignature(params, _secretKey));
	QString paramsStr;
	QHashIterator<QString, QString> iter(params);
	while (iter.hasNext())
	{
		iter.next();
		if (!paramsStr.isEmpty())
			paramsStr.append("&");
		paramsStr.append(
			QString("%1=")
			.arg(iter.key())
			.append(iter.value()));
	}
	QString loginUrl = "http://www.facebook.com/login.php?" + paramsStr;

	// launch the url in a webkit window
	FacebookWizard login(loginUrl, successUrl, cancelUrl, jsApiRef);
	QString result = login.loginExec();
	if (!result.isEmpty())
	{
		// parse the json result
		// example: "3e4a22bb2f5ed75114b0fc9995ea85f1"
		Json::Value root;
		QByteArray tmp = result.toUtf8();
		if (_reader.parse(tmp.constData(), root))
		{
			if (root.isObject() && !root.isMember("error_code"))
			{
				assert(root.isMember("session_key"));
				assert(root.isMember("uid"));
				assert(root.isMember("secret"));
				assert(root.isMember("expires"));
							
				_userid.clear();
				_sessionKey.clear();
				_sessionSecretKey.clear();
				
				if (root.isMember("session_key") && root.isMember("uid") && root.isMember("secret") && root.isMember("expires"))
				{
					QString uid = extractUidFromJsonResult("uid", result);
					if (root["session_key"].isString() && !uid.isEmpty() && root["secret"].isString())
					{
						_sessionKey = extractStringFromJsonResult("session_key", root);
						_userid = uid;
						_sessionSecretKey = extractStringFromJsonResult("secret", root);
						
						int expires = extractIntFromJsonResult("expires", root);
						if (expires == 0) 
						{
							// save the session for reuse in case the user marks this as an infinite session
							GLOBAL(settings).fbc_uid = _userid;
							GLOBAL(settings).fbc_session = _sessionKey;
							GLOBAL(settings).fbc_secret = _sessionSecretKey;
							winOS->SaveSettingsFile();
						}
						
						return true;
					}
				}
			}
		}
	}

	return false;
}

bool FacebookClient::validate_photos_upload( vector<FacebookPhoto>& photos, QString& errorReason )
{
	if (!hasAuthorized())
		return false;

	// ensure not empty
	if (photos.empty())
	{
		errorReason = "No files specified";
		return false;
	}

	// check if the extensions are supported
	// XXX: how about bmp?
	vector<FacebookPhoto>::iterator iter = photos.begin();
	while (iter != photos.end())
	{
		QString ext = fsManager->getFileExtension(iter->filePath);
		if (!_supportedUploadExtensionsContentTypes.contains(ext))
		{
			errorReason = QString("Image format unsupported by Facebook: %1").arg(filename(iter->filePath));
			return false;
		}
		iter++;
	}

	return true;
}

bool FacebookClient::photos_upload( vector<FacebookPhoto>& imagePaths )
{
	if (!hasAuthorized())
		return false;

	assert(!imagePaths.empty());

	// http://wiki.developers.facebook.com/index.php/Photos.upload
	// http://wiki.developers.facebook.com/index.php/Extended_permission
	QString method = "facebook.photos.upload";
	_numPhotosUploaded = imagePaths.size();
	_numPhotos = imagePaths.size();
	_numPhotosFailedUpload = 0;

	vector<FacebookPhoto>::const_iterator iter = imagePaths.begin();
	while (iter != imagePaths.end())
	{
		const FacebookPhoto& photo = *iter;
		
		// NOTE: according to facebook, each image must have max width of 640x640
		//		 so we'll have to scale these images ourselves
		QString path = texMgr->scaleFacebookImage(photo.filePath);

		QHash<QString, QString> params;
			params.insert("method", method);
			params.insert("auth_token", _authToken);
			/*
			if (!photo.album.isEmpty())
			{
				// get all albums and match it
				params.insert("aid")
			}
			*/
			if (!photo.caption.isEmpty())
				params.insert("caption", photo.caption);
			prepareSessionParameters(params);

		// make the function call
		postHttpUploadRequest(method, params, path);
		iter++;
	}
	printTimedUnique("FacebookClient::photos_upload", 600, QT_TR_NOOP("Uploading images to Facebook..."));
	return true;
}

bool FacebookClient::photos_upload(const QString& aid, const QStringList& filePaths, const QString& callback, FileTransferEventHandler * api)
{
	if (!hasAuthorized())
		return false;

	assert(!filePaths.empty());

	// http://wiki.developers.facebook.com/index.php/Photos.upload
	// http://wiki.developers.facebook.com/index.php/Extended_permission
	QString method = "facebook.photos.upload";
	_numPhotosUploaded = filePaths.size();
	_numPhotos = filePaths.size();
	_numPhotosFailedUpload = 0;

	for (int i = 0; i < filePaths.size(); ++i) {
		// NOTE: according to facebook, each image must have max width of 640x640
		//		 so we'll have to scale these images ourselves
		QString path = texMgr->scaleFacebookImage(filePaths[i]);

		QHash<QString, QString> params;
			params.insert("method", method);
			params.insert("auth_token", _authToken);
			if (!aid.isEmpty())
				params.insert("aid", aid);
			prepareSessionParameters(params);
		postHttpUploadRequest(method, params, path, callback, api);
	}
	return true;
}

bool FacebookClient::photos_getAlbums(QString filter, QString& albumLink)
{
	if (!hasAuthorized())
		return false;

	// http://wiki.developers.facebook.com/index.php/Photos.getAlbums
	QString method = "facebook.photos.getAlbums";
	QHash<QString, QString> params;
		params.insert("method", method);
		params.insert("uid", _userid);
	prepareSessionParameters(params);

	// make the function call
	QString result = postHttpReqest(method, params);
	if (!result.isEmpty())
	{
		// parse the json result
		Json::Value root;
		QByteArray tmp = result.toUtf8();
		if (_reader.parse(tmp.constData(), root))
		{
			if (root.isArray())
			{
				for (int i = 0; i < root.size(); ++i)
				{
					Json::Value album = root[i];
					QString name = extractStringFromJsonResult("name", album);
					if (name.contains(filter))
					{
						albumLink = extractStringFromJsonResult("link", album);
						return true;
					}
				}
			}
		}
	}
	return false;
}

bool FacebookClient::photos_createAlbum(const QString& name, const QString& location, const QString& description)
{
	if (!hasAuthorized())
		return false;

	// http://wiki.developers.facebook.com/index.php/Photos.createAlbum
	QString method = "facebook.photos.createAlbum";
	QHash<QString, QString> params;
		params.insert("method", method);
		params.insert("name", name);
		params.insert("location", location);
		params.insert("description", description);
	prepareSessionParameters(params);

	// make the function call
	QString result = postHttpReqest(method, params);
	if (!result.isEmpty())
	{
		// parse the json result
		Json::Value root;
		QByteArray tmp = result.toUtf8();
		if (_reader.parse(tmp.constData(), root))
		{			
			if (root.isObject() && root.isMember("error_code"))
				return false;

			return true;
		}
	}
	return false;
}

QString FacebookClient::hasNewsFeedFilter(const QList<QString>& filterKeys)
{
	if (!hasAuthorized())
		return false;

	QListIterator<QString> iter(filterKeys);
	while (iter.hasNext())
	{
		const QString& filterKey = iter.next();
		if (filterKey.compare("nf", Qt::CaseInsensitive) == 0)
			return filterKey;
	}
	return false;
}

bool FacebookClient::stream_getFilters(QList<QString>& filterKeysOut)
{
	if (!hasAuthorized())
		return false;

	// http://wiki.developers.facebook.com/index.php/Stream.getFilters	
	QString method = "facebook.stream.getFilters";
	QHash<QString, QString> params;
		params.insert("method", method);
	prepareSessionParameters(params);

	// make the function call
	filterKeysOut.clear();
	QString result = postHttpReqest(method, params);
	if (!result.isEmpty())
	{
		// parse the json result
		Json::Value root;
		QByteArray tmp = result.toUtf8();
		if (_reader.parse(tmp.constData(), root))
		{
			assert(root.isArray());
			if (root.isArray())
			{
				for (int i = 0; i < root.size(); ++i)
				{
					const Json::Value& v = root[i];
					assert(v.isObject());
					if (v.isObject() && v.isMember("filter_key")) 
						filterKeysOut.append(extractStringFromJsonResult("filter_key", v));
				}
			}
		}
	}
	return !filterKeysOut.isEmpty();
}

bool FacebookClient::status_get(FacebookStatus * statusOut)
{
	if (!statusOut)
		return false;
	if (!hasAuthorized())
		return false;

	// http://wiki.developers.facebook.com/index.php/Status.get
	QString method = "facebook.status.get";
	QHash<QString, QString> params;
		params.insert("method", method);
		params.insert("uid", _userid);
		params.insert("limit", "1");
	prepareSessionParameters(params);

	// make the function call
	QString result = postHttpReqest(method, params);
	if (!result.isEmpty())
	{
		// parse the json result
		Json::Value root;
		QByteArray tmp = result.toUtf8();
		if (_reader.parse(tmp.constData(), root))
		{			
			if (root.isObject() && root.isMember("error_code"))
				return false;

			for (int i = 0; i < root.size(); ++i) 
			{
				const Json::Value& statusRoot = root[i];
				statusOut->message = extractStringFromJsonResult("message", statusRoot);
				statusOut->time = QDateTime::fromTime_t(extractIntFromJsonResult("time", statusRoot));
				return true;
			}
		}
		else
		{
			if (result == "{}")
			{
				statusOut->message.clear();
				statusOut->time = QDateTime::currentDateTime();
				return true;
			}
		}
	}
	return false;
}

bool FacebookClient::status_set(const QString& status) 
{
	if (!hasAuthorized())
		return false;

	// http://wiki.developers.facebook.com/index.php/Status.set
	QString method = "facebook.status.set";
	QHash<QString, QString> params;
		params.insert("method", method);
		params.insert("status", status);
	prepareSessionParameters(params);

	// make the function call
	QString result = postHttpReqest(method, params);
	if (!result.isEmpty())
	{
		// for some reason, this call returns "true"
		return (result == "true");
	}
	return false;
}

bool FacebookClient::comments_get(const QString& id, int& numCommentsOut)
{
	if (!hasAuthorized())
		return false;

	// http://wiki.developers.facebook.com/index.php/Comments.get
	QString method = "facebook.comments.get";
	QHash<QString, QString> params;
		params.insert("method", method);
		params.insert("object_id", id);
	prepareSessionParameters(params);

	// make the function call
	QString result = postHttpReqest(method, params);
	if (!result.isEmpty())
	{
		// parse the json result
		Json::Value root;
		QByteArray tmp = result.toUtf8();
		if (_reader.parse(tmp.constData(), root))
		{
			if (root.isObject() && root.isMember("error_code"))
				return false;

			numCommentsOut = root.size();
			return true;
		}
	}
	return false;	
}

bool FacebookClient::widget_getPhotoAlbumsInfo(const QStringList& aidUids, const QString& callback, FileTransferEventHandler * handler)
{
	if (!hasAuthorized())
		return false;

	// make the call
	QStringList clauses;
	for (int i = 0; i < aidUids.size(); ++i)
	{
		// clause format: "aid,uid"
		int splitIndex = aidUids[i].indexOf(",");
		QString_NT aid = aidUids[i].left(splitIndex);
		QString_NT uid = aidUids[i].mid(splitIndex + 1);

		clauses.append(QString_NT("(aid = '%1' AND owner = '%2')").arg(aid).arg(uid));
	}

	QString query1 = QString_NT(
		"SELECT aid, owner, name, size \
		FROM album						\
		WHERE %1").arg(clauses.join(QT_NT(" OR ")));

	QString result;
	QMap<QString, QString> queries;
	queries.insert("album", query1);
	fql_multiquery(queries, callback, handler);
	return true;
}

bool FacebookClient::onWidget_getPhotoAlbumsInfo(const QString& result, FacebookWidgetPhotoAlbums& albumsOut)
{
	if (!hasAuthorized())
		return false;

	if (!result.isEmpty())
	{
		// parse the json result
		Json::Value root;
		QByteArray tmp = result.toUtf8();
		if (_reader.parse(tmp.constData(), root))
		{
			if (root.isObject() && root.isMember("error_code"))
				return false;

			const Json::Value& albumsResult = root[(unsigned int)0]["fql_result_set"];
			for (int i = 0; i < albumsResult.size(); ++i) 
			{
				const Json::Value& album = albumsResult[i];

				FacebookWidgetAlbumItem * item = new FacebookWidgetAlbumItem();			
				item->aid = extractStringFromJsonResult(QT_NT("aid"), album);
				item->uid = extractUidFromJsonResult(QT_NT("uid"), qstring(album.toStyledString()));
				item->title = extractStringFromJsonResult(QT_NT("name"), album);
				item->numPhotos = extractIntFromJsonResult(QT_NT("size"), album);

				albumsOut.albums.append(item);
			}

			return true;
		}
	}
	return false;
}

bool FacebookClient::widget_getPhotoAlbums(bool includeProfile, const QString& callback, FileTransferEventHandler * handler) 
{
	if (!hasAuthorized())
		return false;

	// make the call
	QString result;
	QMap<QString, QString> queries;
		queries.insert("albums", QString(
			"SELECT aid, cover_pid, name, description, size, link, modified_major	\
				FROM album															\
				WHERE owner = '%1' %2").arg(_userid).arg(includeProfile ? "" : "AND type <> 'profile'"));
		queries.insert("cover", QString(
			"SELECT pid, aid, src_big, src_big_height, src_big_width	\
				FROM photo												\
				WHERE pid IN (SELECT cover_pid FROM #albums)"));	
	fql_multiquery(queries, callback, handler);
	return true;
}

bool FacebookClient::onWidget_getPhotoAlbums(const QString& result, FacebookWidgetPhotoAlbums& albumsOut)
{
	if (!hasAuthorized())
		return false;

	if (!result.isEmpty())
	{
		// parse the json result
		Json::Value root;
		QByteArray tmp = result.toUtf8();
		if (_reader.parse(tmp.constData(), root))
		{
			if (root.isObject() && root.isMember("error_code"))
				return false;

			QList<FacebookAlbum *> albums;
			QHash<QString, FacebookAlbum *> aidToAlbumsLookup;
			const Json::Value& albumsResult = root[(unsigned int)0]["fql_result_set"];
			for (int i = 0; i < albumsResult.size(); ++i) 
			{
				const Json::Value& album = albumsResult[i];

				FacebookAlbum * newAlbum = new FacebookAlbum();
				newAlbum->aid = extractStringFromJsonResult("aid", album);
				newAlbum->coverPid = extractStringFromJsonResult("cover_pid", album);
				newAlbum->name = extractStringFromJsonResult("name", album);
				newAlbum->description = extractStringFromJsonResult("description", album);
				newAlbum->link = extractStringFromJsonResult("link", album);
				newAlbum->size = extractIntFromJsonResult("size", album);
				newAlbum->modificationDate = QDateTime::fromTime_t(extractIntFromJsonResult("modified_major", album));

				albums.append(newAlbum);
				aidToAlbumsLookup.insert(newAlbum->aid, newAlbum);
			}

			QHash<QString, FacebookWidgetAlbumItem *> aidToCoversLookup;
			const Json::Value& coversResult = root[(unsigned int)1]["fql_result_set"];
			for (int i = 0; i < coversResult.size(); ++i) 
			{
				const Json::Value& cover = coversResult[i];

				FacebookAlbum * album = aidToAlbumsLookup[extractStringFromJsonResult("aid", cover)];
				FacebookWidgetAlbumItem * item = new FacebookWidgetAlbumItem();			
				item->aid = album->aid;
				item->title = album->name;
				item->permalink = album->link;
				item->modifiedDate = album->modificationDate.toTime_t();
				item->albumCoverUrl = extractStringFromJsonResult("src_big", cover);
				item->albumCoverWidth = extractIntFromJsonResult("src_big_width", cover);
				item->albumCoverHeight = extractIntFromJsonResult("src_big_height", cover);
				item->numPhotos = album->size;
				// TODO: get the real comment count!
				item->numComments = 0;

				aidToCoversLookup.insert(item->aid, item);
			}

			for (int i = 0; i < albums.size(); ++i) 
			{
				FacebookAlbum * album = albums[i];
				FacebookWidgetAlbumItem * item = new FacebookWidgetAlbumItem();			
				item->aid = album->aid;
				item->title = album->name;
				item->permalink = album->link;
				item->modifiedDate = album->modificationDate.toTime_t();
				if (aidToCoversLookup.contains(item->aid)) 
				{
					FacebookWidgetAlbumItem * cover = aidToCoversLookup[item->aid];
					item->albumCoverUrl = cover->albumCoverUrl;
					item->albumCoverWidth = cover->albumCoverWidth;
					item->albumCoverHeight = cover->albumCoverHeight;
				}
				item->numPhotos = album->size;
				// TODO: get the real comment count!
				item->numComments = 0;

				albumsOut.albums.append(item);
			}

			// free up all the memory from above
			for (int i = 0; i < albums.size(); ++i)
				SAFE_DELETE(albums[i]);

			return true;
		}
	}
	return false;
}

bool FacebookClient::widget_getPhotoAlbumPhotos(const QString& aid, const QString& callback, FileTransferEventHandler * handler)
{
	if (!hasAuthorized())
		return false;

	// make the call
	QString result;
	QMap<QString, QString> queries;
		queries.insert("photos", QString(
			"SELECT pid, src_big, src_big_height, src_big_width, link, caption, created	\
				FROM photo																\
				WHERE aid = '%1'").arg(aid));	
	fql_multiquery(queries, callback, handler);
	return true;
}

bool FacebookClient::onWidget_getPhotoAlbumPhotos(const QString& result, FacebookWidgetPhotos& photosOut)
{
	if (!hasAuthorized())
		return false;

	if (!result.isEmpty())
	{
		// parse the json result
		Json::Value root;
		QByteArray tmp = result.toUtf8();
		if (_reader.parse(tmp.constData(), root))
		{			
			if (root.isObject() && root.isMember("error_code"))
				return false;

			const Json::Value& photosResult = root[(unsigned int)0]["fql_result_set"];
			for (int i = 0; i < photosResult.size(); ++i) 
			{
				const Json::Value& photo = photosResult[i];

				FacebookWidgetPhoto * newPhoto = new FacebookWidgetPhoto();

				newPhoto->pid = extractStringFromJsonResult("pid", photo);
				newPhoto->title = extractStringFromJsonResult("caption", photo);
				newPhoto->permalink = extractStringFromJsonResult("link", photo);
				newPhoto->url = extractStringFromJsonResult("src_big", photo);
				newPhoto->width = extractIntFromJsonResult("src_big_width", photo);
				newPhoto->height = extractIntFromJsonResult("src_big_height", photo);
				newPhoto->numComments = 0;
				newPhoto->creationDate = extractIntFromJsonResult("created", photo);

				photosOut.photos.append(newPhoto);
			}

			return true;
		}
	}
	return false;
}

bool FacebookClient::widget_getNewsFeed(const QString& callback, FileTransferEventHandler * handler, const QStringList& postTypes, int earliestPhotoTime)
{
	if (!hasAuthorized())
		return false;

	if (earliestPhotoTime == -1)
		earliestPhotoTime = (int)QDateTime::currentDateTime().toTime_t();
	
	QString clauses = QString();
	for (int i = 0; i < postTypes.size(); i++)
	{
		clauses.append(QT_NT(" OR name='") + postTypes[i] + QT_NT("'"));
	}
	if (clauses.length() > 0)
		clauses = clauses.mid(4);
	else
		clauses = QT_NT("name='Photos' OR name='Status Updates' OR name='Links'");

	QString query1 = QString_NT("SELECT post_id, actor_id, message, attachment, permalink, created_time, comments, likes \
							FROM stream \
							WHERE filter_key IN (\
								SELECT filter_key \
								FROM stream_filter \
								WHERE uid=%1 AND (%2)) \
							AND is_hidden = 0 AND created_time<%3 \
							ORDER BY created_time DESC").arg(getUid()).arg(clauses).arg(earliestPhotoTime); 

	// make the call
	QString result;
	QMap<QString, QString> queries;
		queries.insert("user_stream", query1);
		queries.insert("profiles", QString_NT(
			"SELECT uid, name, pic_big, profile_url	\
				FROM user										\
				WHERE uid IN (SELECT actor_id FROM #user_stream) OR uid=%1").arg(getUid()));
	fql_multiquery(queries, callback, handler);
	return true;
}

bool FacebookClient::onWidget_getNewsFeed(const QString& result, FacebookWidgetNewsFeed& entriesOut)
{
	if (!hasAuthorized())
		return false;

	QHash<QString, FacebookProfile *> uidToProfilesLookup;	// Map each user profile to its uid for easy lookup
	if (!result.isEmpty())
	{
		// parse the json result
		Json::Value root;
		QByteArray tmp = result.toUtf8();
		if (_reader.parse(tmp.constData(), root))
		{
			if (root.isObject() && root.isMember("error_code"))
				return false;

			// Extract Facebook each user's profile information
			const Json::Value& profilesResult = root[(unsigned int)1]["fql_result_set"];
			// Iterate through each user
			for (int i = 0; i < profilesResult.size(); ++i) 
			{
				const Json::Value& jsonProfile = profilesResult[i];
				QString profileStr = qstring(jsonProfile.toStyledString());
				
				// Fields from Facebook user profile JSON results are explained here: http://wiki.developers.facebook.com/index.php/User_(FQL)
				FacebookProfile * newProfile = new FacebookProfile();
				newProfile->uid = extractUidFromJsonResult(QT_NT("uid"), profileStr);
				newProfile->name = extractStringFromJsonResult(QT_NT("name"), jsonProfile);
				newProfile->profileLink = extractStringFromJsonResult(QT_NT("profile_url"), jsonProfile);
				newProfile->picSquare = extractStringFromJsonResult(QT_NT("pic_big"), jsonProfile);

				uidToProfilesLookup.insert(newProfile->uid, newProfile);
			}
			
			// Extract Facebook pictures from the each user's stream (newsfeed)
			QList<FacebookStreamPost *> streamPhotoPosts;				// This is a list of lists; Store list of each user's list of photos from stream
			QHash<QString, FacebookStreamPhoto *> pidToPhotosLookup;	// Map each pid to its photo for easy lookup
			const Json::Value& userStreamResult = root[(unsigned int)0]["fql_result_set"];
			// Iterate through each user
			for (int i = 0; i < userStreamResult.size(); ++i) 
			{
				const Json::Value& post = userStreamResult[i];

				QString postStr = qstring(post.toStyledString());
				FacebookStreamPost * newStreamPost = new FacebookStreamPost();	// Holds list of individual user's photos

				// We have added user photos to the list so add this list to the parent list
				// Extract information about the post on the user's stream
				newStreamPost->postId = extractStringFromJsonResult(QT_NT("post_id"), post);
				newStreamPost->sourceId = extractUidFromJsonResult(QT_NT("source_id"), postStr);
				newStreamPost->actorUid = extractUidFromJsonResult(QT_NT("actor_id"), postStr);
				newStreamPost->message = extractStringFromJsonResult(QT_NT("message"), post);
				newStreamPost->creationDate = QDateTime::fromTime_t(extractIntFromJsonResult(QT_NT("created_time"), post));
				newStreamPost->permalink = extractStringFromJsonResult(QT_NT("permalink"), post);
				newStreamPost->comments.count = extractIntFromJsonResult(QT_NT("count"), post[QT_NT("comments")]);
				newStreamPost->likes.count = extractIntFromJsonResult(QT_NT("count"), post[QT_NT("likes")]);

				// posts without thumbnails (some links)
				QString_NT linkHref = extractStringFromJsonResult(QT_NT("href"), post["attachment"]);
				if (linkHref != "" && linkHref != "http://www.facebook.com/")
				{
					FacebookStreamLink * streamLink = new FacebookStreamLink();
					streamLink->href = linkHref;
					streamLink->alt = extractStringFromJsonResult(QT_NT("name"), post["attachment"]);
					streamLink->domain = extractStringFromJsonResult(QT_NT("caption"), post["attachment"]);
					streamLink->description = extractStringFromJsonResult(QT_NT("description"), post["attachment"]); 
					newStreamPost->link = *streamLink;
				}

				// posts with thumbnails (photos, links, videos)
				if (post["attachment"].isMember("media"))
				{
					// Fields from Facebook stream JSON results are explained here: http://wiki.developers.facebook.com/index.php/Stream_(FQL)
					// Iterate through the stream photos from each individual user and append them
					const Json::Value& mediasRoot = post["attachment"]["media"];

					QString commonAlt = QString(); // photos come in groups, but only one has alt (caption) assigned to it, so apply it to all

					for (int j = 0; j < mediasRoot.size(); ++j) 
					{
						const Json::Value& mediaRoot = mediasRoot[j];

						// photo posts
						if (mediaRoot["type"] == "photo") 
						{
							const Json::Value& mediaPhotoRoot = mediaRoot["photo"];
							QString mediaPhotoRootStr = qstring(mediaPhotoRoot.toStyledString());

							// Extract information from the photo album
							FacebookStreamPhoto * streamPhoto = new FacebookStreamPhoto();
							streamPhoto->href = extractStringFromJsonResult(QT_NT("href"), mediaRoot);
							streamPhoto->type = extractStringFromJsonResult(QT_NT("type"), mediaRoot);
							streamPhoto->src = extractStringFromJsonResult(QT_NT("src"), mediaRoot);
							streamPhoto->aid = extractStringFromJsonResult(QT_NT("aid"), mediaPhotoRoot);
							streamPhoto->pid = extractStringFromJsonResult(QT_NT("pid"), mediaPhotoRoot);
							streamPhoto->ownerUid = extractUidFromJsonResult(QT_NT("owner"), mediaPhotoRootStr);
							streamPhoto->index = extractIntFromJsonResult(QT_NT("index"), mediaPhotoRoot);
							streamPhoto->width = extractIntFromJsonResult(QT_NT("width"), mediaPhotoRoot);
							streamPhoto->height = extractIntFromJsonResult(QT_NT("height"), mediaPhotoRoot);

							streamPhoto->alt = extractStringFromJsonResult(QT_NT("alt"), mediaRoot);
							if (streamPhoto->alt != "")
								commonAlt = streamPhoto->alt;

							newStreamPost->album.photos.append(streamPhoto);
							pidToPhotosLookup.insert(streamPhoto->pid, streamPhoto);
						} 
						// link posts with thumbnails
						else if (mediaRoot["type"] == "link" || mediaRoot["type"] == "video") 
						{
							for (int j = 0; j < mediasRoot.size(); ++j) 
							{
								const Json::Value& mediaRoot = mediasRoot[j];
								
								FacebookStreamLink * streamLink = new FacebookStreamLink();
								streamLink->href = extractStringFromJsonResult(QT_NT("href"), mediaRoot);
								streamLink->alt = extractStringFromJsonResult(QT_NT("name"), post["attachment"]);
								streamLink->domain = extractStringFromJsonResult(QT_NT("caption"), post["attachment"]);
								streamLink->description = extractStringFromJsonResult(QT_NT("description"), post["attachment"]);
								streamLink->type = extractStringFromJsonResult(QT_NT("type"), mediaRoot);
								streamLink->src = extractStringFromJsonResult(QT_NT("src"), mediaRoot);
								newStreamPost->link = *streamLink;
							}
						}
					}
					// apply the caption given to one photo in a post to the other photos in the post
					if (commonAlt != "")
					{
						for (int j = 0; j < newStreamPost->album.photos.size(); ++j) 
						{
							newStreamPost->album.photos[j]->alt = commonAlt; 
						}
					}
				}
				streamPhotoPosts.append(newStreamPost);
			}

			if (!streamPhotoPosts.isEmpty())
			{
				// Cross reference all the data and just pull out the information we need
				for (int i = 0; i < streamPhotoPosts.size(); ++i) 
				{
					FacebookStreamPost * post = streamPhotoPosts[i];
					FacebookProfile * profile = uidToProfilesLookup[post->actorUid];
					FacebookStreamLink * link = &(post->link);

					int photoIndex = 0;
					do
					{
						FacebookWidgetNewsFeedItem * item = new FacebookWidgetNewsFeedItem();

						item->postId = post->postId;
						item->message = post->message;
						item->creationDate = post->creationDate;
						item->numComments = post->comments.count;
						item->numLikes = post->likes.count;
						item->sourceId = post->sourceId;
						item->permalink = post->permalink;

						if (profile)
						{
							item->ownerName = profile->name;
							item->ownerProfileLink = profile->profileLink;
							item->ownerPicSquare = profile->picSquare;
						}

						if (link)
						{
							item->extlink = link->href;
							item->imageAlt = link->alt;
							item->domain = link->domain;
							item->description = link->description;
							item->imageSrc = link->src;
						}

						if (!post->album.photos.isEmpty())
						{
							FacebookStreamPhoto * photo = post->album.photos[photoIndex];
							item->title = photo->alt;
							item->photoId = photo->pid;
							item->imageAlt = photo->alt;
							item->imageSrc = photo->src;
							item->imageWidth = photo->width;
							item->imageHeight = photo->height;
							item->permalink = photo->href;
						}

						entriesOut.items.append(item);
					}
					while (++photoIndex < post->album.photos.size());
				}
			}

			// Free all the memory from above
			for (int i = 0; i < streamPhotoPosts.size(); ++i)
				SAFE_DELETE(streamPhotoPosts[i]);
		}
	}
	// append dummy item for updating status
	FacebookWidgetNewsFeedItem * item = new FacebookWidgetNewsFeedItem();
	FacebookProfile * profile = uidToProfilesLookup[getUid()];

	item->postId = QT_NT("dummy_status_update");
	item->message = "";
	item->creationDate = QDateTime::fromTime_t((uint)9999999999);
	item->numComments = 0;
	item->numLikes = 0;
	item->sourceId = "";
	item->permalink = "";

	if (profile)
	{
		item->ownerName = profile->name;
		item->ownerProfileLink = profile->profileLink;
		item->ownerPicSquare = profile->picSquare;
	}

	entriesOut.items.append(item);
	return true;
}

bool FacebookClient::widget_getPageInfo(const QList<QString>& pageIds, const QString& callback, FileTransferEventHandler * handler)
{
	if (!hasAuthorized())
		return false;
	if (pageIds.isEmpty())
		return false;

	// make the call
	QStringList clauses;
	for (int i = 0; i < pageIds.size(); ++i)
		clauses.append(QString("page_id = '%1'").arg(pageIds[i]));
			
	QMap<QString, QString> queries;
	queries.insert("user_stream", QString(
		"SELECT page_id, name, page_url	\
		FROM page						\
		WHERE							\
		%1").arg(clauses.join(" OR ")));
	fql_multiquery(queries, callback, handler);
	return true;
}

bool FacebookClient::onWidget_getPageInfo(const QString& result, FacebookWidgetPages& pagesOut)
{
	if (!hasAuthorized())
		return false;

	if (!result.isEmpty())
	{
		// Parse the json result
		Json::Value root;
		QByteArray tmp = result.toUtf8();
		if (_reader.parse(tmp.constData(), root))
		{
			// One page result is returned for each unique source_id
			const Json::Value& pagesResult = root[(unsigned int)0]["fql_result_set"];
			for (int i = 0; i < pagesResult.size(); ++i) 
			{
				const Json::Value pageResult = pagesResult[i];
				// Iterate through all values in the hash that contain the same source Id
				QString pageStr = qstring(pagesResult.toStyledString());
				QString pageId = extractUidFromJsonResult("page_id", pageStr);
				if (!pageId.isEmpty())
				{
					FacebookWidgetPage * page = new FacebookWidgetPage();
					page->pageId = pageId;
					page->name = extractStringFromJsonResult("name", pageResult);
					page->pageUrl = extractStringFromJsonResult("page_url", pageResult);
					pagesOut.pages.append(page);
				}
			}
			return true;
		}
	}
	return false;
}

bool FacebookClient::widget_getHighResolutionPhotos(const QList<QString>& pids, const QString& callback, FileTransferEventHandler * handler)
{
	if (!hasAuthorized())
		return false;
	if (pids.isEmpty())
		return false;

	// make the call
	QStringList clauses;
	for (int i = 0; i < pids.size(); ++i)
		clauses.append(QString("pid = '%1'").arg(pids[i]));
	
	QMap<QString, QString> queries;
	queries.insert("user_stream", QString_NT(
		"SELECT pid, aid, owner, src_big, src_big_height, src_big_width	\
			FROM photo										\
			WHERE											\
			%1").arg(clauses.join(" OR ")));
	fql_multiquery(queries, callback, handler);
	return true;
}

bool FacebookClient::onWidget_getHighResolutionPhotos(const QString& result, FacebookWidgetPhotos& photosOut)
{
	if (!hasAuthorized())
		return false;

	if (!result.isEmpty())
	{
		// parse the json result
		Json::Value root;
		QByteArray tmp = result.toUtf8();
		if (_reader.parse(tmp.constData(), root))
		{
			if (root.isObject() && root.isMember("error_code"))
				return false;

			const Json::Value& photosResult = root[(unsigned int)0]["fql_result_set"];
			for (int i = 0; i < photosResult.size(); ++i) 
			{
				const Json::Value photoResult = photosResult[i];
				QString newHref = extractStringFromJsonResult(QT_NT("src_big"), photoResult);
				if (!newHref.isEmpty())
				{
					FacebookWidgetPhoto * photo = new FacebookWidgetPhoto();
					photo->pid = extractStringFromJsonResult(QT_NT("pid"), photoResult);
					photo->aid = extractStringFromJsonResult(QT_NT("aid"), photoResult);
					photo->owner = extractUidFromJsonResult(QT_NT("owner"), qstring(photoResult.toStyledString()));
					photo->url = newHref;
					photo->width = extractIntFromJsonResult(QT_NT("src_big_width"), photoResult);
					photo->height = extractIntFromJsonResult(QT_NT("src_big_height"), photoResult);
					photosOut.photos.append(photo);
				}
			}

			return true;
		}
	}
	return false;
}

bool FacebookClient::fql_multiquery(const QMap<QString, QString>& queries, QString& rawResultOut)
{
	if (!hasAuthorized())
		return false;

	// http://wiki.developers.facebook.com/index.php/Fql.multiquery
	QString method = "facebook.fql.multiquery";
	QHash<QString, QString> params;
		params.insert("method", method);
		
		// append the queries
		Json::Value queriesRoot;
		QMapIterator<QString, QString> iter(queries);
		while (iter.hasNext())
		{
			iter.next();
			queriesRoot[stdString(iter.key())] = stdString(iter.value());
		}
		QString queriesStr = qstring(queriesRoot.toStyledString());
		params.insert("queries", queriesStr);
	prepareSessionParameters(params);

	// make the function call
	rawResultOut = postHttpReqest(method, params);
	return true;
}

bool FacebookClient::fql_multiquery(const QMap<QString, QString>& queries, const QString& callback, FileTransferEventHandler * handler)
{
	if (!hasAuthorized())
		return false;

	// http://wiki.developers.facebook.com/index.php/Fql.multiquery
	QString method = "facebook.fql.multiquery";
	QHash<QString, QString> params;
		params.insert("method", method);
		
		// append the queries
		Json::Value queriesRoot;
		QMapIterator<QString, QString> iter(queries);
		while (iter.hasNext())
		{
			iter.next();
			queriesRoot[stdString(iter.key())] = stdString(iter.value());
		}
		QString queriesStr = qstring(queriesRoot.toStyledString());
		params.insert("queries", queriesStr);
	prepareSessionParameters(params);

	// make the function call
	postHttpReqest(method, params, callback, handler);
	return true;
}

bool FacebookClient::users_hasAppPermission( QString permissionId, bool& hasPermission )
{
	if (!hasAuthorized())
		return false;

	// http://wiki.developers.facebook.com/index.php/Users.hasAppPermission
	QString method = "facebook.users.hasAppPermission";
	QHash<QString, QString> params;
		params.insert("method", method);
		params.insert("ext_perm", permissionId);
	prepareSessionParameters(params);

	// make the function call
	QString result = postHttpReqest(method, params);
	if (!result.isEmpty())
	{
		// parse the json result
		Json::Value root;
		QByteArray tmp = result.toUtf8();
		if (_reader.parse(tmp.constData(), root))
		{
			if (root.isBool())
			{
				hasPermission = root.asBool();
				return true;
			}
		}
	}
	return false;
}

bool FacebookClient::users_isAppUser(bool& isUserOut)
{
	if (!hasAuthorized())
		return false;

	// http://wiki.developers.facebook.com/index.php/Users.isAppUser
	QString method = "facebook.users.isAppUser";
	QHash<QString, QString> params;
	params.insert("method", method);
	prepareSessionParameters(params);

	// make the function call
	QString result = postHttpReqest(method, params);
	if (!result.isEmpty())
	{
		// parse the json result
		Json::Value root;
		QByteArray tmp = result.toUtf8();
		if (_reader.parse(tmp.constData(), root))
		{
			if (root.isBool())
			{
				isUserOut = root.asBool();
				return true;
			}
		}
	}
	return false;
}

void FacebookClient::onTransferComplete( const FileTransfer& transfer )
{
	bool uploadFailed = false;
	--_numPhotosUploaded;

	/*
	if (!transfer.getTemporaryString().isEmpty())
	{
		// parse the json result
		// example: 
		Json::Value root;
		QByteArray tmp = result.toUtf8();
		if (_reader.parse(tmp.constData(), root))
		{
			if (!root.isMember("error_code"))
				uploadFailed = true;
			else
			{
	*/
				if ((_numPhotos > 1) && (_numPhotosUploaded > 0))
				{
					if (_numPhotosFailedUpload > 0)
					{
						printTimedUnique("FacebookClient::photos_upload", 600, QT_TR_NOOP("Image %1 of %2 uploaded to Facebook (%3 failed)")
							.arg(_numPhotos - _numPhotosUploaded).arg(_numPhotos).arg(_numPhotosFailedUpload));
					}
					else
					{
						printTimedUnique("FacebookClient::photos_upload", 600, QT_TR_NOOP("Image %1 of %2 uploaded to Facebook")
						.arg(_numPhotos - _numPhotosUploaded).arg(_numPhotos));
					}
				}
				else
				{
					QString albumLink;
					if (photos_getAlbums(FACEBOOK_APPNAME, albumLink))
					{
						// try and launch the album path
						fsManager->launchFileAsync(albumLink);
					}
					printUnique("FacebookClient::photos_upload", QT_TR_NOOP("All image(s) uploaded to Facebook!"));
				}
	/*
			}
		}
	}
	else
	{
		uploadFailed = true;
	}
	*/

	if (uploadFailed)
	{
		// NOTE: onTransferError also decrements the count, so bump it back to before
		++_numPhotosUploaded;
		onTransferError(transfer);
	}
}

void FacebookClient::onTransferError( const FileTransfer& transfer )
{
	--_numPhotosUploaded;
	++_numPhotosFailedUpload;

	if (_numPhotos > 1)
	{
		printTimedUnique("FacebookClient::photos_upload", 600, QT_TR_NOOP("Image %1 of %2 uploaded to Facebook (%3 failed)")
			.arg(_numPhotos - _numPhotosUploaded).arg(_numPhotos).arg(_numPhotosFailedUpload));
	}
	else
		printUniqueError("FacebookClient::photos_upload", QT_TR_NOOP("Upload to Facebook failed"));
}
