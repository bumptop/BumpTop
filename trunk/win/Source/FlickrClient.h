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

#ifndef FLICKRCLIENT_H
#define FLICKRCLIENT_H

#include "BT_FileTransferManager.h"
#include "BT_BumpObject.h"
#include "BT_FileSystemActor.h"

#define FLICKR_API_KEY "e877c149f1ae996ac3d26e24da423c84"
#define FLICKR_SECRET_KEY "bc5f565b527b978b"
#define FLICKR_API_SERVER "api.flickr.com/services/rest/?"

/*
The flickr api uses basic HTTP auth
*/
class FlickrClient : public FileTransferEventHandler
{
	Q_DECLARE_TR_FUNCTIONS(FlickrClient)

	Json::Reader _reader;
	QString _authToken;
	QString _frob;
	QString _currentFileToUpload;
	int _numPhotosToUpload;
	int _numPhotosFailedToUpload;
	vector<QString> _photoIds;
	

private:
	QString requestAFrob();
	QString createLoginLink();
	QString convertFrobToAToken(QString frob);
	QString getHttpRequest(QString server, QHash<QString, QString> *params, FileTransferEventHandler * handler = NULL);
	QString postHttpRequest(QString url, QHash<QString, QString> *params, QString filePath, FileTransferEventHandler * handler = NULL);
	QString getAPISignature(QHash<QString, QString> *params);
	void getAuthToken();
	Json::Value responseToJson(const QString& response);

public:
	FlickrClient();
	FlickrClient(QString authToken);

	// authenticate
	bool authenticate();
	void requestPhotosByTag(QString tag, FileTransferEventHandler * handler);
	void requestPhotosFromId( QString id, bool isGroupId, FileTransferEventHandler * handler);
	void requestFavouritePhotosFromId(QString id, FileTransferEventHandler * handler);
	vector<QString> extractPhotoUrls(const FileTransfer& transfer);
	Json::Value executeAPICall (QString server, QHash<QString, QString> *params, bool useGetHttp = true);
	void executeAPICallCustom(QString server, QHash<QString, QString> *params, bool useGetHttp, FileTransferEventHandler * handler);

	QString getUsername (QString id);
	QString getGroupName (QString id);

	bool uploadPhotos (const vector<FileSystemActor *> &files);
	bool wasUploadSuccessful ();

	// events
	virtual void onTransferComplete(const FileTransfer& transfer);
	virtual void onTransferError(const FileTransfer& transfer);

};

#endif // FLICKRCLIENT_H