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

#ifndef BT_FACEBOOKWIDGETJAVASCCRIPTAPI_H
#define BT_FACEBOOKWIDGETJAVASCCRIPTAPI_H

#include "BT_JavaScriptAPI.h"

class FacebookClient;
class FacebookWidgetJavaScriptAPI;

// async handler for the news feed request
class NewsFeedRequestHandler : public FileTransferEventHandler {
	FacebookWidgetJavaScriptAPI * _api;
public:
	NewsFeedRequestHandler(FacebookWidgetJavaScriptAPI * api);

	virtual void onTransferComplete(const FileTransfer& transfer);
	virtual void onTransferError(const FileTransfer& transfer);	
};
// async handler for the high-res request
class HighResolutionPhotosRequestHandler : public FileTransferEventHandler {
	FacebookWidgetJavaScriptAPI * _api;
public:
	HighResolutionPhotosRequestHandler(FacebookWidgetJavaScriptAPI * api);

	virtual void onTransferComplete(const FileTransfer& transfer);
	virtual void onTransferError(const FileTransfer& transfer);	
};
// async handler for the page info request
class PageInfoRequestHandler : public FileTransferEventHandler {
	FacebookWidgetJavaScriptAPI * _api;
public:
	PageInfoRequestHandler(FacebookWidgetJavaScriptAPI * api);

	virtual void onTransferComplete(const FileTransfer& transfer);
	virtual void onTransferError(const FileTransfer& transfer);	
};
// async handler for the albums info request
class AlbumsInfoRequestHandler : public FileTransferEventHandler {
	FacebookWidgetJavaScriptAPI * _api;
public:
	AlbumsInfoRequestHandler(FacebookWidgetJavaScriptAPI * api);

	virtual void onTransferComplete(const FileTransfer& transfer);
	virtual void onTransferError(const FileTransfer& transfer);	
};
// async handler for the albums feed request
class AlbumsRequestHandler : public FileTransferEventHandler {
	FacebookWidgetJavaScriptAPI * _api;
public:
	AlbumsRequestHandler(FacebookWidgetJavaScriptAPI * api);

	virtual void onTransferComplete(const FileTransfer& transfer);
	virtual void onTransferError(const FileTransfer& transfer);	
};
// async handler for the album photos request
class AlbumPhotosRequestHandler : public FileTransferEventHandler {
	FacebookWidgetJavaScriptAPI * _api;
public:
	AlbumPhotosRequestHandler(FacebookWidgetJavaScriptAPI * api);

	virtual void onTransferComplete(const FileTransfer& transfer);
	virtual void onTransferError(const FileTransfer& transfer);	
};
// async handler for the photo upload request
class PhotoUploadRequestHandler : public FileTransferEventHandler {
	FacebookWidgetJavaScriptAPI * _api;

	void removeTemporaryTransferFiles(const FileTransfer& transfer);
public:
	PhotoUploadRequestHandler(FacebookWidgetJavaScriptAPI * api);

	virtual void onTransferComplete(const FileTransfer& transfer);
	virtual void onTransferError(const FileTransfer& transfer);	
};

class FacebookWidgetJavaScriptAPI : public JavaScriptAPI
{
	Q_OBJECT

	FacebookClient * _client;
	QString _jsonFormattedResult;
	QHash<QString, QString> _temporaryStore;
	Json::Value _persistentStore;
	QString _persistentStorePath;

	NewsFeedRequestHandler _newsFeedRequestHandler;
	HighResolutionPhotosRequestHandler _highResolutionPhotosRequestHandler;
	PageInfoRequestHandler _pageInfoRequestHandler;
	AlbumsInfoRequestHandler _albumsInfoRequestHandler;
	AlbumsRequestHandler _albumsRequestHandler;
	AlbumPhotosRequestHandler _albumPhotosRequestHandler;
	PhotoUploadRequestHandler _photoUploadRequestHandler;

	friend class NewsFeedRequestHandler;
	friend class PageInfoRequestHandler;
	friend class HighResolutionPhotosRequestHandler;
	friend class AlbumsInfoRequestHandler;
	friend class AlbumsRequestHandler;
	friend class AlbumPhotosRequestHandler;
	friend class PhotoUploadRequestHandler;

private:
	void serializePersistentStore();
	void deserializePersistentStore();

public:	
	FacebookWidgetJavaScriptAPI(WebActor * actor, WebPage * page);
	~FacebookWidgetJavaScriptAPI();
	bool initialize();

public slots:
	QString internal_getJsonFormattedStringData();

	void setTemporaryStoreValue(const QString& key, const QString& value);
	QString getTemporaryStoreValue(const QString& key);
	void removeTemporaryStoreValue(const QString& key);

	void setPersistentStoreValue(const QString& key, const QString& value);
	QString getPersistentStoreValue(const QString& key);
	void removePersistentStoreValue(const QString& key);

	QString getFilePathForUrl(const QString& url);

	bool launchLoginPage();
	void logout();
	void launchUrl(const QString& url);
	void setStatus(const QString& message);

	void requestNewsFeed(const QString& callback, int earliestPhotoTime = -1);
	void requestHighResolutionPhotos(const QStringList& pidsList, const QString& callback);
	void requestPageInfo(const QStringList& pageIdsList, const QString& callback);
	void requestCreateAlbum(const QString& name, const QString& location, const QString& description);
	void requestAlbumsInfo(const QStringList& aidUids, const QString& callback);
	void requestAlbums(const QString& callback, bool includeProfile);
	void requestPhotosForAlbum(const QString& aid, const QString& callback);
	void requestAddressBookEntries(const QString& callback);
	void requestStatus(const QString& callback);
	void requestUploadPhotos(const QString& aid, const QStringList& fileUrlsList, const QString& callback);

	QString getUid();
	bool isFocused();
	bool isAnimationDisabled();
	bool isUpdatingDisabled();

	int getSlideShowDuration();
};

#endif