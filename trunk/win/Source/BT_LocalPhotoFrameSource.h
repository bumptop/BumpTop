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

#ifndef BT_LOCAL_PHOTOFRAME_SOURCE
#define BT_LOCAL_PHOTOFRAME_SOURCE

#include "BT_PhotoFrameSource.h"
#include "BT_Watchable.h"

class PhotoFrameDialog;
class PbPhotoFrameSource;

class LocalPhotoFrameSource : public PhotoFrameSource
{
	Q_DECLARE_TR_FUNCTIONS(LocalPhotoFrameSource)

	QDir	_localPath;

private:
	void updateDirectory(QDir dir, vector<PhotoFrameSourceItem>& items);

public:
	LocalPhotoFrameSource();
	LocalPhotoFrameSource(QString localPath);

	virtual void requestSourceUpdate();
	virtual void onSourceUpdate(const FileTransfer& transfer);
	virtual void requestImageUpdate();
	virtual void onImageUpdate(const FileTransfer& transfer);

	virtual void preparePhotoFrameDialog(PhotoFrameDialog * dialog) const;
	virtual bool unserialize(unsigned char *buf, uint &bufSz);
	virtual bool equals(PhotoFrameSource * other) const;
	virtual void flushCache();
	virtual PhotoFrameSource * clone() const;

	virtual QString getName() const;
	virtual const PhotoFrameSourceType getType() const;

	// watchable overrides
	virtual void onFileAdded();
	virtual vector<QString> getWatchDir();
	
	// protocol buffers
	virtual bool serializeToPb(PbPhotoFrameSource * pbSource);
	virtual bool deserializeFromPb(const PbPhotoFrameSource * pbSource);
};

#endif