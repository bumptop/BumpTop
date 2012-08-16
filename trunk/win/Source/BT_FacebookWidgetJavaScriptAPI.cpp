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
#include "BT_FacebookWidgetJavaScriptAPI.h"
#include "BT_FileSystemManager.h"
#include "BT_SceneManager.h"
#include "BT_Util.h"
#include "BT_WebActor.h"
#include "BT_WindowsOS.h"
#include "FacebookClient.h"
#include "moc/moc_BT_FacebookWidgetJavaScriptAPI.cpp"


bool FacebookWidgetNewsFeed_GreaterThan(const FacebookWidgetNewsFeedItem * item, const FacebookWidgetNewsFeedItem * other)
{
	return item->creationDate.toTime_t() > other->creationDate.toTime_t();
}

NewsFeedRequestHandler::NewsFeedRequestHandler(FacebookWidgetJavaScriptAPI * api)
: _api(api)
{}

void NewsFeedRequestHandler::onTransferComplete(const FileTransfer& transfer)
{
	FacebookWidgetNewsFeed feed;
	if (_api->_client->onWidget_getNewsFeed(transfer.getTemporaryString(), feed))
	{
		// sort the results
		qSort(feed.items.begin(), feed.items.end(), FacebookWidgetNewsFeed_GreaterThan);

		// call back the client			
		QString * callback = (QString *) transfer.getUserData();
		_api->_jsonFormattedResult = feed.toJsonString();
		_api->_page->evaluateJavaScript(QString_NT("%1(BumpTopNative.internal_getJsonFormattedStringData(), true);")
			.arg(*callback));
		SAFE_DELETE(callback);
	}
	else
		onTransferError(transfer);
}

void NewsFeedRequestHandler::onTransferError(const FileTransfer& transfer)
{
	QString * callback = (QString *) transfer.getUserData();
	_api->_page->evaluateJavaScript(QString_NT("%1('{}', false);")
		.arg(*callback));
	SAFE_DELETE(callback);
}

PageInfoRequestHandler::PageInfoRequestHandler(FacebookWidgetJavaScriptAPI * api)
: _api(api)
{}

void PageInfoRequestHandler::onTransferComplete(const FileTransfer& transfer)
{
	FacebookWidgetPages pages;
	if (_api->_client->onWidget_getPageInfo(transfer.getTemporaryString(), pages))
	{
		// call back the client			
		QString * callback = (QString *) transfer.getUserData();
		_api->_jsonFormattedResult = pages.toJsonString();
		_api->_page->evaluateJavaScript(QString_NT("%1(BumpTopNative.internal_getJsonFormattedStringData(), true);")
			.arg(*callback));
		SAFE_DELETE(callback);
	}
	else
		onTransferError(transfer);
}

void PageInfoRequestHandler::onTransferError(const FileTransfer& transfer)
{
	QString * callback = (QString *) transfer.getUserData();
	_api->_page->evaluateJavaScript(QString_NT("%1('{}', false);")
		.arg(*callback));
	SAFE_DELETE(callback);
}

HighResolutionPhotosRequestHandler::HighResolutionPhotosRequestHandler(FacebookWidgetJavaScriptAPI * api)
: _api(api)
{}

void HighResolutionPhotosRequestHandler::onTransferComplete(const FileTransfer& transfer)
{
	FacebookWidgetPhotos photos;
	if (_api->_client->onWidget_getHighResolutionPhotos(transfer.getTemporaryString(), photos))
	{
		// call back the client			
		QString * callback = (QString *) transfer.getUserData();
		_api->_jsonFormattedResult = photos.toJsonString();
		_api->_page->evaluateJavaScript(QString_NT("%1(BumpTopNative.internal_getJsonFormattedStringData(), true);")
			.arg(*callback));
		SAFE_DELETE(callback);
	}
	else
		onTransferError(transfer);
}

void HighResolutionPhotosRequestHandler::onTransferError(const FileTransfer& transfer)
{
	QString * callback = (QString *) transfer.getUserData();
	_api->_page->evaluateJavaScript(QString_NT("%1('{}', false);")
		.arg(*callback));
	SAFE_DELETE(callback);
}

bool FacebookWidgetPhotoAlbums_GreaterThan(const FacebookWidgetAlbumItem * album, const FacebookWidgetAlbumItem * other)
{
	return album->modifiedDate > other->modifiedDate;
}

AlbumsInfoRequestHandler::AlbumsInfoRequestHandler(FacebookWidgetJavaScriptAPI * api)
: _api(api)
{}

void AlbumsInfoRequestHandler::onTransferComplete(const FileTransfer& transfer)
{
	FacebookWidgetPhotoAlbums albums;
	if (_api->_client->onWidget_getPhotoAlbumsInfo(transfer.getTemporaryString(), albums))
	{
		// call back the client
		QString * callback = (QString *) transfer.getUserData();
		_api->_jsonFormattedResult = albums.toJsonString();
		_api->_page->evaluateJavaScript(QString_NT("%1(BumpTopNative.internal_getJsonFormattedStringData(), true);")
			.arg(*callback));
		SAFE_DELETE(callback);
	}
	else
		onTransferError(transfer);
}

void AlbumsInfoRequestHandler::onTransferError(const FileTransfer& transfer)
{
	QString * callback = (QString *) transfer.getUserData();
	_api->_page->evaluateJavaScript(QString_NT("%1('{}', false);")
		.arg(*callback));
	SAFE_DELETE(callback);
}

AlbumsRequestHandler::AlbumsRequestHandler(FacebookWidgetJavaScriptAPI * api)
: _api(api)
{}

void AlbumsRequestHandler::onTransferComplete(const FileTransfer& transfer)
{
	FacebookWidgetPhotoAlbums albums;
	if (_api->_client->onWidget_getPhotoAlbums(transfer.getTemporaryString(), albums))
	{
		// sort the results
		qSort(albums.albums.begin(), albums.albums.end(), FacebookWidgetPhotoAlbums_GreaterThan);

		// call back the client
		QString * callback = (QString *) transfer.getUserData();
		_api->_jsonFormattedResult = albums.toJsonString();
		_api->_page->evaluateJavaScript(QString_NT("%1(BumpTopNative.internal_getJsonFormattedStringData(), true);")
			.arg(*callback));
		SAFE_DELETE(callback);
	}
	else
		onTransferError(transfer);
}

void AlbumsRequestHandler::onTransferError(const FileTransfer& transfer)
{
	QString * callback = (QString *) transfer.getUserData();
	_api->_page->evaluateJavaScript(QString_NT("%1('{}', false);")
		.arg(*callback));
	SAFE_DELETE(callback);
}

bool FacebookWidgetPhotos_LessThan(const FacebookWidgetPhoto * photo, const FacebookWidgetPhoto * other)
{
	return photo->creationDate < other->creationDate;
}

AlbumPhotosRequestHandler::AlbumPhotosRequestHandler(FacebookWidgetJavaScriptAPI * api)
: _api(api)
{}

void AlbumPhotosRequestHandler::onTransferComplete(const FileTransfer& transfer)
{
	FacebookWidgetPhotos photos;
	if (_api->_client->onWidget_getPhotoAlbumPhotos(transfer.getTemporaryString(), photos))
	{
		// sort the results
		qSort(photos.photos.begin(), photos.photos.end(), FacebookWidgetPhotos_LessThan);

		// call back the client
		QString * callback = (QString *) transfer.getUserData();
		_api->_jsonFormattedResult = photos.toJsonString();
		_api->_page->evaluateJavaScript(QString_NT("%1(BumpTopNative.internal_getJsonFormattedStringData(), true);")
			.arg(*callback));
		SAFE_DELETE(callback);
	}
	else
		onTransferError(transfer);
}

void AlbumPhotosRequestHandler::onTransferError(const FileTransfer& transfer)
{
	QString * callback = (QString *) transfer.getUserData();
	_api->_page->evaluateJavaScript(QString_NT("%1('{}', false);")
		.arg(*callback));
	SAFE_DELETE(callback);
}

PhotoUploadRequestHandler::PhotoUploadRequestHandler(FacebookWidgetJavaScriptAPI * api)
: _api(api)
{}

void PhotoUploadRequestHandler::removeTemporaryTransferFiles(const FileTransfer& transfer)
{
	QHash<QString, QString> params = transfer.getParams();
	QSet<QString> fileParams = transfer.getFileParamKeys();
	QSetIterator<QString> fileParamsIter(fileParams);
	while (fileParamsIter.hasNext()) {
		QString filePath = params[fileParamsIter.next()];
		QFile::remove(filePath);
	}
}

void PhotoUploadRequestHandler::onTransferComplete(const FileTransfer& transfer)
{
	// call back the client
	QString * callback = (QString *) transfer.getUserData();
	_api->_jsonFormattedResult = transfer.getTemporaryString();
	_api->_page->evaluateJavaScript(QString_NT("%1(BumpTopNative.internal_getJsonFormattedStringData(), true);")
		.arg(*callback));
	SAFE_DELETE(callback);
	removeTemporaryTransferFiles(transfer);
}

void PhotoUploadRequestHandler::onTransferError(const FileTransfer& transfer)
{
	QString * callback = (QString *) transfer.getUserData();
	_api->_page->evaluateJavaScript(QString_NT("%1('{}', false);")
		.arg(*callback));
	SAFE_DELETE(callback);
	removeTemporaryTransferFiles(transfer);
}

// --------------------------------------------------------------------------------------------------------

FacebookWidgetJavaScriptAPI::FacebookWidgetJavaScriptAPI(WebActor * actor, WebPage * page)
: JavaScriptAPI(actor, page, true)
, _client(NULL)
, _newsFeedRequestHandler(this)
, _pageInfoRequestHandler(this)
, _highResolutionPhotosRequestHandler(this)
, _albumsInfoRequestHandler(this)
, _albumsRequestHandler(this)
, _albumPhotosRequestHandler(this)
, _photoUploadRequestHandler(this)
{
	_client = new FacebookClient();
}

FacebookWidgetJavaScriptAPI::~FacebookWidgetJavaScriptAPI()
{
	SAFE_DELETE(_client); 
	ftManager->removeTransfersToHandler(&_newsFeedRequestHandler);
	ftManager->removeTransfersToHandler(&_albumsInfoRequestHandler);
	ftManager->removeTransfersToHandler(&_albumsRequestHandler);
	ftManager->removeTransfersToHandler(&_albumPhotosRequestHandler);
	ftManager->removeTransfersToHandler(&_photoUploadRequestHandler);
}

bool FacebookWidgetJavaScriptAPI::initialize() {		
	// load the persistent store
	_persistentStorePath = native(make_file((winOS->GetCacheDirectory() / "Webkit"), "persistentStore.facebook.json"));
	deserializePersistentStore();

	bool usingSavedSession = false;
	return _client->initialize(usingSavedSession);
}

QString FacebookWidgetJavaScriptAPI::internal_getJsonFormattedStringData()
{
	return _jsonFormattedResult;
}

void FacebookWidgetJavaScriptAPI::setTemporaryStoreValue(const QString& key, const QString& value)
{
	_temporaryStore.insert(key, value);
}

QString FacebookWidgetJavaScriptAPI::getTemporaryStoreValue(const QString& key)
{
	if (_temporaryStore.contains(key))
		return _temporaryStore[key];
	return QString();
}

void FacebookWidgetJavaScriptAPI::serializePersistentStore()
{
	Json::StyledWriter jsonWriter;
	QString outStr = QString::fromUtf8(jsonWriter.write(_persistentStore).c_str());
	write_file_utf8(outStr, _persistentStorePath);
}

void FacebookWidgetJavaScriptAPI::deserializePersistentStore()
{
	if (exists(_persistentStorePath)) {
		Json::Reader reader;
		QByteArray tmp = read_file_utf8(_persistentStorePath).toUtf8();
		reader.parse(tmp.constData(), _persistentStore);	 
	}
}

void FacebookWidgetJavaScriptAPI::removeTemporaryStoreValue(const QString& key)
{
	_temporaryStore.remove(key);
}

void FacebookWidgetJavaScriptAPI::setPersistentStoreValue(const QString& key, const QString& value)
{
	_persistentStore[stdString(key)] = stdString(value);
	serializePersistentStore();
}

QString FacebookWidgetJavaScriptAPI::getPersistentStoreValue(const QString& key)
{
	if (_persistentStore.isMember(stdString(key)))
		return qstring(_persistentStore[stdString(key)].asString());
	return QString();
}

void FacebookWidgetJavaScriptAPI::removePersistentStoreValue(const QString& key)
{
	_persistentStore.removeMember(stdString(key));
	serializePersistentStore();
}

QString FacebookWidgetJavaScriptAPI::getFilePathForUrl(const QString& url)
{
	if (_fileDataUrlsToPaths.contains(url))
		return QUrl::fromLocalFile(_fileDataUrlsToPaths[url]).toString();
	return QString();
}

bool FacebookWidgetJavaScriptAPI::launchLoginPage()
{
	return _client->fbconnect_login(this);
}

void FacebookWidgetJavaScriptAPI::logout()
{
	// log the user out
	GLOBAL(settings).fbc_uid.clear();
	GLOBAL(settings).fbc_session.clear();
	GLOBAL(settings).fbc_secret.clear();
	winOS->SaveSettingsFile();

	// clear the persistent, temporary stores
	_temporaryStore.clear();
	_persistentStore.clear();
	QFile::remove(_persistentStorePath);

	// set all the facebook widgets to the login page
	vector<BumpObject *> objs = scnManager->getBumpObjects(ObjectType(BumpActor, Webpage));
	for (int i = 0; i < objs.size(); ++i)
	{
		WebActor * actor = (WebActor *) objs[i];
		if (actor->isFacebookWidgetUrl()) 
			actor->load(QT_NT("bumpwidget-facebook://login"));
	}
}

void FacebookWidgetJavaScriptAPI::requestNewsFeed(const QString& callback, int earliestPhotoTime /*= -1*/)
{
	// which types of posts to request (status updates, links, photos, videos, etc)
	QStringList filters = QStringList(getPersistentStoreValue(QT_NT("newsfeed-posttypes")).split(","));
	_client->widget_getNewsFeed(callback, &_newsFeedRequestHandler, filters, earliestPhotoTime);
}

void FacebookWidgetJavaScriptAPI::requestHighResolutionPhotos(const QStringList& pidsList, const QString& callback)
{
	_client->widget_getHighResolutionPhotos(pidsList, callback, &_highResolutionPhotosRequestHandler);
}

void FacebookWidgetJavaScriptAPI::requestPageInfo(const QStringList& pageIdsList, const QString& callback)
{
	_client->widget_getPageInfo(pageIdsList, callback, &_pageInfoRequestHandler);
}

void FacebookWidgetJavaScriptAPI::requestCreateAlbum(const QString& name, const QString& location, const QString& description)
{
	_client->photos_createAlbum(name, location, description);
}

void FacebookWidgetJavaScriptAPI::requestAlbumsInfo(const QStringList& aidUids, const QString& callback)
{
	_client->widget_getPhotoAlbumsInfo(aidUids, callback, &_albumsInfoRequestHandler);
}

void FacebookWidgetJavaScriptAPI::requestAlbums(const QString& callback, bool includeProfile)
{
	_client->widget_getPhotoAlbums(includeProfile, callback, &_albumsRequestHandler);
}

void FacebookWidgetJavaScriptAPI::requestPhotosForAlbum(const QString& aid, const QString& callback)
{
	_client->widget_getPhotoAlbumPhotos(aid, callback, &_albumPhotosRequestHandler);
}

void FacebookWidgetJavaScriptAPI::requestAddressBookEntries(const QString& callback)
{
}

void FacebookWidgetJavaScriptAPI::requestStatus(const QString& callback)
{
	FacebookStatus status;
	if (_client->status_get(&status))
	{
		// call back the client
		_jsonFormattedResult = status.toJsonString();
		_page->evaluateJavaScript(QString_NT("%1(BumpTopNative.internal_getJsonFormattedStringData());")
			.arg(callback));
	}
}

void FacebookWidgetJavaScriptAPI::requestUploadPhotos(const QString& aid, const QStringList& fileUrlsList, const QString& callback)
{
	// convert all the urls to filepaths
	QStringList filePaths;
	for (int i = 0; i < fileUrlsList.size(); ++i) {
		if (_fileDataUrlsToPaths.contains(fileUrlsList[i]))
			filePaths.append(_fileDataUrlsToPaths[fileUrlsList[i]]);
	}
	// upload the file paths
	if(!filePaths.empty())
		_client->photos_upload(aid, filePaths, callback, &_photoUploadRequestHandler);
}

void FacebookWidgetJavaScriptAPI::setStatus(const QString& message)
{
	_client->status_set(message);
}

void FacebookWidgetJavaScriptAPI::launchUrl(const QString& url)
{
	fsManager->launchFile(url);
}

bool FacebookWidgetJavaScriptAPI::isFocused()
{	
	return _actor->isFocused();
}

bool FacebookWidgetJavaScriptAPI::isAnimationDisabled()
{
	// value of checkbox option in feed to turn off slideshow
	bool slideshowOn = getPersistentStoreValue("newsfeed-slideshow") == "false" ? false : true;
	return !slideshowOn || _actor->isAnimationDisabled();
}

bool FacebookWidgetJavaScriptAPI::isUpdatingDisabled()
{
	return _actor->isUpdatingDisabled();
}

QString FacebookWidgetJavaScriptAPI::getUid() 
{
	return _client->getUid();
}

int FacebookWidgetJavaScriptAPI::getSlideShowDuration()
{
	return GLOBAL(settings).photoFrameImageDuration;
}
