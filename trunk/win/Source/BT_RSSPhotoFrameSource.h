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

#ifndef BT_PHOTO_FRAME_RSS_SOURCE
#define BT_PHOTO_FRAME_RSS_SOURCE

#include "BT_PhotoFrameSource.h"

class PhotoFrameDialog;
class PbPhotoFrameSource;

class RSSPhotoFrameSource : public PhotoFrameSource,
							public FileTransferEventHandler
{
	Q_DECLARE_TR_FUNCTIONS(RSSPhotoFrameSource)

protected:
	QString		_rssFeedUrl;
	QDir		_localCacheDirectory;
	virtual void convertToValidDirectoryName(QString& pathStr);

private:
	void createLocalCacheDirectory();
	

public:
	RSSPhotoFrameSource();
	RSSPhotoFrameSource(QString rssFeedUrl);
	virtual ~RSSPhotoFrameSource();

	virtual void requestSourceUpdate();
	virtual void onSourceUpdate(const FileTransfer& transfer);
	virtual void requestImageUpdate();
	virtual void onImageUpdate(const FileTransfer& transfer);
	virtual bool loadPhotosFromCache();

	virtual void preparePhotoFrameDialog(PhotoFrameDialog * dialog) const;
	virtual bool unserialize(unsigned char *buf, uint &bufSz);
	virtual bool equals(PhotoFrameSource * other) const;
	virtual void flushCache();
	virtual PhotoFrameSource * clone() const;

	virtual QString getName() const;
	virtual const PhotoFrameSourceType getType() const;

	bool isFlickrPhotoFrame();
	QString getRSSFeedUrl();
	
	// protocol buffers
	virtual bool serializeToPb(PbPhotoFrameSource * pbSource);
	virtual bool deserializeFromPb(const PbPhotoFrameSource * pbSource);

	// file transfer events
	virtual void onTransferComplete(const FileTransfer& transfer);
	virtual void onTransferError(const FileTransfer& transfer);	
};

#endif