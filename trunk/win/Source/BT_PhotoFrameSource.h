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

#ifndef BT_PHOTOFRAME_SOURCE
#define BT_PHOTOFRAME_SOURCE

#include "BT_FileTransferManager.h"
#include "BT_GLTextureObject.h"
#include "BT_PhotoFrameSourceItem.h"

class PhotoFrameDialog;
class PbPhotoFrameSource;

enum PhotoFrameSourceType
{
	LocalImageSource	= (1 << 0),
	RemoteImageSource	= (1 << 1),
	PileImageSource		= (1 << 2),
	FlickrImageSource	= (1 << 3)
};

class PhotoFrameSource
{
protected:
	vector<PhotoFrameSourceItem>	_items;
	PhotoFrameSourceItem			_current;
	int								_sourceUpdateCount;
	int								_imageUpdateCount;

	QString							_textureBufferId;
	GLTextureDetail					_textureDetail;
	mutable QMutex					_memberMutex;
	// NOTE: QMutex must be mutable if it's to be accessed from a const function

public:
	PhotoFrameSource();
	virtual ~PhotoFrameSource();

	void setTextureBuffer(QString id);
	QString getTextureBuffer() const;

	void setTextureDetail(GLTextureDetail detail);
	
	virtual void requestSourceUpdate() = 0;
	virtual void onSourceUpdate(const FileTransfer& transfer) = 0;
	virtual void requestImageUpdate() = 0;
	virtual void onImageUpdate(const FileTransfer& transfer) = 0;

	virtual bool skipTo(QString resourceId);

	virtual void preparePhotoFrameDialog(PhotoFrameDialog * dialog) const = 0;
	virtual bool unserialize(unsigned char *buf, uint &bufSz) = 0;
	virtual bool equals(PhotoFrameSource * other) const = 0;
	virtual void flushCache() = 0;
	virtual PhotoFrameSource * clone() const = 0;

	PhotoFrameSourceItem getCurrent() const;
	virtual QString getName() const = 0;
	virtual const PhotoFrameSourceType getType() const = 0;
	int getSize() const;
	int getSourceUpdateCount() const;
	int getImageUpdateCount() const;

	// watchable overrides
	virtual void onFileAdded();
	virtual vector<QString> getWatchDir();
	
	// protocol buffers
	virtual bool serializeToPb(PbPhotoFrameSource * pbSource);
	virtual bool deserializeFromPb(const PbPhotoFrameSource * pbSource);
};

#endif