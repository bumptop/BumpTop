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

#ifndef FACEBOOKCLIENT_H
#define FACEBOOKCLIENT_H

#include "BT_FileTransferManager.h"

#define FACEBOOK_APPNAME "BumpTop"

class FacebookClient;
class JavaScriptAPI;

/*
*/
struct FacebookPhoto
{
public:
	// legacy
	QString filePath;
	QString album;

	// new
	QString pid;
	QString aid;
	QString srcSmall;
	int srcSmallWidth;
	int srcSmallHeight;
	QString srcBig;
	int srcBigWidth;
	int srcBigHeight;
	QString src;
	int srcWidth;
	int srcHeight;
	QString link;
	QString caption;
	QDateTime creationDate;
	QDateTime modifiedDate;

public:
	FacebookPhoto(QString path);
	FacebookPhoto(QString path, QString cap);
	FacebookPhoto(QString path, QString cap, QString alb);
};

/*
*/
class FacebookAlbum
{
public:
	QString aid;
	QString coverPid;
	QString ownerUid;
	QString name;
	QString description;
	QString location;
	QDateTime creationDate;
	QDateTime modificationDate;
	QString link;
	QString editLink;
	QString visibility;
	QString type;
	int size;
};

/*
*/
class FacebookProfile
{
public:
	QString uid;
	QString profileLink;
	QString name;
	QString picSquare;
	QString type;
};

/*
*/
class FacebookStreamMediaItem
{
public:
	QString href;
	QString alt;
	QString type;
	QString src;
};
class FacebookStreamPhoto : public FacebookStreamMediaItem
{
public:	
	QString aid;
	QString pid;
	QString ownerUid;
	int index;
	int width;
	int height;
};
class FacebookStreamLink : public FacebookStreamMediaItem
{
public:	
	QString domain;
	QString description;
};
class FacebookStreamAlbum 
{
public:
	QList<FacebookStreamPhoto *> photos;
	QString name;
	QString caption;
};
class FacebookStreamComments
{
public:
	int count;
};
class FacebookStreamLikes
{
public:
	int count;
};
class FacebookStreamPost
{
public: 
	QString sourceId;
	QString postId;
	QString actorUid;
	QString message;
	FacebookStreamAlbum album;
	FacebookStreamLink link;
	FacebookStreamComments comments;
	FacebookStreamLikes likes;
	QDateTime creationDate;
	QDateTime updatedDate;
	QString permalink;
	int type;
};

/*
*/
class FacebookStatus
{
public:
	QDateTime time;
	QString message;

	QString toJsonString();
};

/*
 * Only used for the facebook widget, aggregates data from the other stream entries
 * and pares it down into a single usable item
 */
class FacebookWidgetNewsFeedItem
{
public:
	QString postId;
	QString message;
	QString sourceId;
	QString title;
	QString permalink;
	QString extlink;
	QString domain;
	QString description;
	QString ownerName;
	QString ownerProfileLink;
	QString ownerPicSquare;
	QDateTime creationDate;
	QString photoId;
	QString imageAlt;
	QString imageSrc;
	int imageWidth;
	int imageHeight;
	int numComments;
	int numLikes;

	FacebookWidgetNewsFeedItem() : imageWidth(0), imageHeight(0), numComments(0), 
		numLikes(0) {}

	Json::Value toJsonValue();
};
class FacebookWidgetNewsFeed
{
public:
	QList<FacebookWidgetNewsFeedItem *> items;

	QString toJsonString();
};
class FacebookWidgetAlbumItem
{
public:
	QString aid;
	QString uid;
	QString title;
	QString permalink;
	QString albumCoverUrl;
	int albumCoverWidth;
	int albumCoverHeight;
	int modifiedDate;
	int numPhotos;
	int numComments;

	FacebookWidgetAlbumItem() : albumCoverWidth(0), albumCoverHeight(0),
		modifiedDate(0), numPhotos(0), numComments(0) {}

	Json::Value toJsonValue();
};
class FacebookWidgetPhotoAlbums 
{
public:
	QList<FacebookWidgetAlbumItem *> albums;

	QString toJsonString();
};
class FacebookWidgetPhoto
{
public:
	QString pid;
	QString aid;
	QString owner;
	QString title;
	QString permalink;
	QString url;
	int width;
	int height;
	int numComments;
	int creationDate;

	FacebookWidgetPhoto() : width(0), height(0), numComments(0), creationDate(0) {}

	Json::Value toJsonValue();
};
class FacebookWidgetPhotos 
{
public:
	QList<FacebookWidgetPhoto *> photos;

	QString toJsonString();
};

class FacebookWidgetPage
{
public:
	QString pageId;
	QString name;
	QString pageUrl;

	FacebookWidgetPage(){}

	Json::Value toJsonValue();
};
class FacebookWidgetPages 
{
public:
	QList<FacebookWidgetPage *> pages;

	QString toJsonString();
};

/*
*/
class FacebookClient : public FileTransferEventHandler
{
	Q_DECLARE_TR_FUNCTIONS(FacebookClient);

	QString _apiKey;
	QString _secretKey;
	QString _serverUrl;
	QString _version;

	Json::Reader _reader;
	QHash<QString, QString> _supportedUploadExtensionsContentTypes;

	// from auth_createToken
	QString _authToken;
	int _numPhotosUploaded;
	int _numPhotosFailedUpload;
	int _numPhotos;

	// from auth_getSession
	QString _userid;
	QString _sessionKey;
	QString _sessionSecretKey;

	friend class FacebookAlbum;

private:
	QString generateSignature(QHash<QString, QString> params, QString secretKey);
	void prepareSessionParameters(QHash<QString, QString>& params);
	void prepareSessionlessParameters(QHash<QString, QString>& params);
	QString postHttpReqest(QString method, QHash<QString, QString> params);
	void postHttpReqest(QString method, QHash<QString, QString> params, const QString& callback, FileTransferEventHandler * handler);
	QString postHttpUploadRequest(QString method, QHash<QString, QString> params, QString filePath);
	void postHttpUploadRequest(QString method, QHash<QString, QString> params, QString filePath, const QString& callback, FileTransferEventHandler * handler);
	QString postHttpsRequest(QString method, QHash<QString, QString> params);	
	QString extractUidFromJsonResult(const QString& key, const QString& jsonSrc) const;
	int extractIntFromJsonResult(const QString& key, const Json::Value& json) const;
	QString extractStringFromJsonResult(const QString& key, const Json::Value& json) const;

	bool hasAuthorized() const; 

	QHttp _http;
	QHash<int,QString> _pendingHttpRequests;
	void httpRequestFinished( int id, bool error );

public:
	FacebookClient();
	~FacebookClient();

	// initialization
	bool initialize(bool& usingSavedSession);

	const QString& getUid() const;

	// facebook api calls
	bool fbconnect_login(JavaScriptAPI * jsApiRef);
		
	bool validate_photos_upload(vector<FacebookPhoto>& photos, QString& errorReasonOut);
	bool photos_upload(vector<FacebookPhoto>& photos);
	bool photos_upload(const QString& aid, const QStringList& filePaths, const QString& callback, FileTransferEventHandler * api);
	bool photos_getAlbums(QString filter, QString& albumLink);
	bool photos_createAlbum(const QString& name, const QString& location, const QString& description);

	bool users_hasAppPermission(QString permissionId, bool& hasPermission);
	bool users_isAppUser(bool& isUserOut);
	// bool users_getInfo_profileUrl(QString& urlOut);

	QString hasNewsFeedFilter(const QList<QString>& filters);
	bool stream_getFilters(QList<QString>& filterKeysOut);

	bool status_get(FacebookStatus * statusOut);
	bool status_set(const QString& status);

	bool comments_get(const QString& id, int& numCommentsOut);

	// widget specific calls (results are formatted specifically,
	// feel free to refactor code out later as necessary)
	bool widget_getPhotoAlbumsInfo(const QStringList& aidUids, const QString& callback, FileTransferEventHandler * handler);
	bool onWidget_getPhotoAlbumsInfo(const QString& result, FacebookWidgetPhotoAlbums& albumsOut);
	bool widget_getPhotoAlbums(bool includeProfile, const QString& callback, FileTransferEventHandler * handler);
	bool onWidget_getPhotoAlbums(const QString& result, FacebookWidgetPhotoAlbums& albumsOut);
	bool widget_getPhotoAlbumPhotos(const QString& aid, const QString& callback, FileTransferEventHandler * handler);
	bool onWidget_getPhotoAlbumPhotos(const QString& result, FacebookWidgetPhotos& photosOut);
	bool widget_getNewsFeed(const QString& callback, FileTransferEventHandler * handler, const QStringList& postTypes, int earliestPhotoTime);
	bool onWidget_getNewsFeed(const QString& result, FacebookWidgetNewsFeed& entriesOut);
	bool widget_getHighResolutionPhotos(const QList<QString>& pids, const QString& callback, FileTransferEventHandler * handler);
	bool onWidget_getHighResolutionPhotos(const QString& result, FacebookWidgetPhotos& photosOut);
	bool widget_getPageInfo(const QList<QString>& pageIds, const QString& callback, FileTransferEventHandler * handler);
	bool onWidget_getPageInfo(const QString& result, FacebookWidgetPages& pagesOut);
	// bool widget_getFriendsList(QList<FacebookWidgetFriend *>& friendsOut);
	// bool widget_getFriendDetails(const QString& uid);

	bool fql_multiquery(const QMap<QString, QString>& queries, QString& rawResultOut);
	bool fql_multiquery(const QMap<QString, QString>& queries, const QString& callback, FileTransferEventHandler * handler);

	// events
	virtual void onTransferComplete(const FileTransfer& transfer);
	virtual void onTransferError(const FileTransfer& transfer);	
};

#endif // FACEBOOKCLIENT_H