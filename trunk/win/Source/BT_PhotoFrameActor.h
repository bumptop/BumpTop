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

#ifndef BT_PHOTOFRAME_ACTOR
#define BT_PHOTOFRAME_ACTOR

#include "BT_FileSystemActor.h"
#include "BT_PhotoFrameSource.h"
#include "BT_Watchable.h"

class ThreadableUnit;
class PhotoFrameDialog;

class PhotoFrameActor : public FileSystemActor,
						public Watchable
{
	PhotoFrameSource *		_source;
	Stopwatch				_sourceUpdateTimer;
	Stopwatch				_imageUpdateTimer;
	int						_numItemsCached;
	bool					_isUpdating;
	bool					_hasUpdatedBefore;

public:
	bool pNullFunc();
	bool pSourceUpdateThread();
	bool pImageUpdateThread();

	// converts a string resource to a valid photo frame source (if one exists)
	static PhotoFrameSource * resolveSourceFromString(QString ri);

private:
	void initActor(PhotoFrameSource * source);

public:
	PhotoFrameActor();
	PhotoFrameActor(PhotoFrameSource * source);
	virtual ~PhotoFrameActor();

#ifdef DXRENDER
	virtual IDirect3DTexture9 * getTextureNum();
#else
	virtual uint getTextureNum();
#endif

	bool equals(PhotoFrameSource * source) const;
	virtual bool allowNativeContextMenu() const;
	void setSource(PhotoFrameSource * source);
	void setUninitializedSource(PhotoFrameSource * source);
	void initSource();
	virtual void setMinThumbnailDetail(GLTextureDetail detail);
	void syncSourceName();
	void preparePhotoFrameDialog(PhotoFrameDialog * dialog) const;
	bool isSourceType(PhotoFrameSourceType type) const;
	
	void flushCache();
	bool empty() const;
	
	virtual void onUpdate();
	virtual void onRender(uint flags = RenderSideless);
	virtual void onTextureLoadComplete(QString textureKey);

	// system events
	bool isUpdating();
	void disableUpdate();
	void enableUpdate();

	// watchable
	virtual void onFileAdded(QString strFileName);
	virtual void onFileRemoved(QString strFileName);
	virtual void onFileNameChanged(QString strOldFileName, QString strNewFileName);
	virtual void onFileModified(QString strFileName);
	virtual StrList getWatchDir();
	
	// protocol buffers
	virtual bool serializeToPb(PbBumpObject * pbObject);
	virtual bool deserializeFromPb(const PbBumpObject * pbObject);
};

#endif