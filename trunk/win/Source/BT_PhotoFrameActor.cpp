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
#include "BT_EventManager.h"
#include "BT_FileSystemManager.h"
#include "BT_FlickrPhotoFrameSource.h"
#include "BT_GLTextureManager.h"
#include "BT_LocalPhotoFrameSource.h"
#include "BT_Macros.h"
#include "BT_OverlayComponent.h"
#include "BT_PhotoFrameActor.h"
#include "BT_PhotoFrameDialog.h"
#include "BT_PhotoFrameSource.h"
#include "BT_RSSPhotoFrameSource.h"
#include "BT_SceneManager.h"
#include "BT_ThreadableUnit.h"
#include "BT_Util.h"
#include "BumpTop.pb.h"
#include "PhotoFrameSource.pb.h"

#ifdef DXRENDER
#include "BT_DXRender.h"
#endif

// -----------------------------------

PhotoFrameActor::PhotoFrameActor()
: FileSystemActor()
, _source(NULL)
, _isUpdating(true)
, _hasUpdatedBefore(false)
{
	initActor(NULL);
}

PhotoFrameActor::PhotoFrameActor(PhotoFrameSource * source)
: FileSystemActor()
, _source(NULL)	// NULL because we set it properly later in initActor()
, _isUpdating(true)
, _hasUpdatedBefore(false)
{
	initActor(source);
}

bool PhotoFrameActor::isUpdating() 
{
	return _isUpdating;
}

void PhotoFrameActor::initActor(PhotoFrameSource * source)
{
	// set the actor type and hide the text
	pushFileSystemType(PhotoFrame);
	pushFileSystemType(Image);
	pushFileSystemType(Thumbnail);
	enableThumbnail(true, false);
	hideText(true);

	setSource(source);
	
	// start watching the dirs (if necessary)
	fsManager->addObject(this);
}

PhotoFrameActor::~PhotoFrameActor()
{
	texMgr->deleteTexture(getThumbnailID());
	texMgr->deleteTexture(getAlternateThumbnailId());
	enableThumbnail(false);

	// stop watching the dirs (if necessary)
	fsManager->removeObject(this);

	SAFE_DELETE(_source)
}

void PhotoFrameActor::onUpdate()
{
	if (!_isUpdating)
		return;

	if (_source)
	{
		bool shouldUpdateSource = !_hasUpdatedBefore || 
			(_sourceUpdateTimer.elapsed() > GLOBAL(settings).photoFrameSourceDuration * 1000);
		if (shouldUpdateSource)
		{
			// try updating the source
			_source->requestSourceUpdate();
			_sourceUpdateTimer.restart();
			_hasUpdatedBefore = true;
		}

		bool shouldUpdateImage = !empty() && 
			((_imageUpdateTimer.elapsed() > GLOBAL(settings).photoFrameImageDuration * 1000) ||
			(_source->getSourceUpdateCount() > 0 && _source->getImageUpdateCount() == 0));
		if (shouldUpdateImage)
		{
			// try updating the image
			_source->requestImageUpdate();
			_imageUpdateTimer.restart();
		}
	}
	return;
}

// Store the photo frame source inside the actor
// and initialize the actor and threads
void PhotoFrameActor::setSource(PhotoFrameSource * source)
{
	setUninitializedSource(source);
	initSource();
	_hasUpdatedBefore = false;

}

// ONLY the photo frame source inside the actor 
void PhotoFrameActor::setUninitializedSource(PhotoFrameSource * source)
{
	if (!source)
		return;

	// set the source and have it load all the images to the alternate thumbnail
	// id of this actor
	SAFE_DELETE(_source)
	_source = source;
}

// Initialize the photo frame source
// Spin the necessary threads and add the object to the fsManager
void PhotoFrameActor::initSource()
{
	if (!_source)
		return;

	_numItemsCached = -1;

	// reset the texture id's
	texMgr->deleteTexture(getThumbnailID());
	texMgr->deleteTexture(getAlternateThumbnailId());

	_source->setTextureBuffer(getAlternateThumbnailId());
	syncSourceName();
}

void PhotoFrameActor::syncSourceName()
{
	if (_source)
	{
		setText(_source->getName());
		if (getNameableOverlay() &&
			getNameableOverlay()->getTextOverlay())
			getNameableOverlay()->getTextOverlay()->markBackgroundAsDirty();
	}
}

bool PhotoFrameActor::allowNativeContextMenu() const
{
	return false;
}

PhotoFrameSource * PhotoFrameActor::resolveSourceFromString( QString ri )
{
	PhotoFrameSource * newSource = NULL;
	if (ri.isEmpty())
		return NULL;

	if (ri.startsWith(QT_NT("http://api.flickr.com"), Qt::CaseInsensitive))
	{
		newSource = new FlickrPhotoFrameSource(ri);
	}
	else if (ri.startsWith(QT_NT("http://"), Qt::CaseInsensitive))
	{
		// check if it is an rss feed
		// XXX: download the xml and find if it is actually an rss file
		newSource = new RSSPhotoFrameSource(ri);
	}
	else if (ri.startsWith(QT_NT("feed://"), Qt::CaseInsensitive))
	{
		// this just refers to an http feed
		QString httpUrl(QT_NT("http://"));
		if (ri.size() <= httpUrl.size())
			return NULL;
		newSource = new RSSPhotoFrameSource(httpUrl + ri.mid(httpUrl.size()));
	}
	else if (ri.startsWith(QT_NT("flickr://"), Qt::CaseInsensitive))
	{
		newSource = new FlickrPhotoFrameSource(ri);
	}
	else
	{
		// check if it is a local file path
		try
		{
			if (exists(ri))
				newSource = new LocalPhotoFrameSource(ri);
			else
				throw invalid_argument("Invalid Photo Frame source encountered");
		}
		catch (...)
		{
			MessageClearPolicy clearPolicy;
			clearPolicy.setTimeout(3);
			scnManager->messages()->addMessage(new Message("PhotoFrameActor::resolveSourceFromString", QT_TR_NOOP("Invalid photo frame source given!"), Message::Warning, clearPolicy));
		}
	}

	return newSource;
}

void PhotoFrameActor::preparePhotoFrameDialog(PhotoFrameDialog * dialog) const
{
	if (_source)
	{
		// let each source update the dialog for it's own thing
		dialog->resetToDefault();
		_source->preparePhotoFrameDialog(dialog);
	}
}

bool PhotoFrameActor::isSourceType(PhotoFrameSourceType type) const
{
	if (this->_source)
		return this->_source->getType() == type;
	else
		return false;
}

void PhotoFrameActor::onRender( uint flags /*= RenderSideless*/ )
{
#ifdef DXRENDER
	// TODO DXR renderColorSidedBox
	dxr->renderSideLessBox(getGlobalPosition(), getGlobalOrientation(), getDims(), getTextureNum());
#else
	bool useAlpha = texMgr->getTextureAlpha(textureID);

	glPushAttribToken token(GL_ENABLE_BIT);
	if (flags & RenderIgnoreDepth) 
		glDisable(GL_DEPTH_TEST);
	else
		glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);

	// Conditionals
	if (isSelected() || isActorType(Temporary)) glDisable(GL_DEPTH_TEST);
	if (isActorType(Invisible)) return;

	// Set the Texture
	glBindTexture(GL_TEXTURE_2D, getTextureNum());
	glColor4f(1, 1, 1, getAlpha());

	// Render Self
	ShapeVis::renderColorSidedBox(getGlobalPosition(), getGlobalOrientation(), getDims());
#endif
}

void PhotoFrameActor::flushCache()
{
	if (_source)
		_source->flushCache();
}

bool PhotoFrameActor::empty() const
{
	if (_source)
		return (_source->getSize() == 0);
	return false;
}

bool PhotoFrameActor::equals( PhotoFrameSource * source ) const
{
	if (source && _source)
	{
		return (_source->equals(source));
	}
	return false;
}

void PhotoFrameActor::onTextureLoadComplete(QString textureKey)
{
	// update the buffer to point to the new id
	QString thumbnailId = getThumbnailID();
	if (_source)
		_source->setTextureBuffer(thumbnailId);

	// do the swap
	setThumbnailID(_alternateThumbnailId);
	_alternateThumbnailId = thumbnailId;

	// set the current file path
	if (_source->getCurrent().isValid())
		setFilePath(texMgr->getTexturePathFromId(textureKey));
}

#ifdef DXRENDER
IDirect3DTexture9 * PhotoFrameActor::getTextureNum()
#else
uint PhotoFrameActor::getTextureNum()
#endif
{
	if (_numItemsCached < 0)
	{
		if (_source && _source->getSourceUpdateCount() > 0)
			_numItemsCached = _source->getSize();
	}

	if (_numItemsCached == 0)
		return texMgr->getGLTextureId("photoframe.empty");
	else if (_numItemsCached > 0 && 
		texMgr->isTextureState(getThumbnailID(), TextureLoaded))
		return FileSystemActor::getTextureNum();
	return texMgr->getGLTextureId("photoframe.loading");
}

void PhotoFrameActor::setMinThumbnailDetail( GLTextureDetail detail )
{
	if (_source)
		_source->setTextureDetail(detail);
	FileSystemActor::setMinThumbnailDetail(detail);
}

void PhotoFrameActor::disableUpdate()
{
	_isUpdating = false;
}

void PhotoFrameActor::enableUpdate()
{
	_isUpdating = true;
	_sourceUpdateTimer.restart();
	_imageUpdateTimer.restart();
}

void PhotoFrameActor::onFileAdded( QString strFileName )
{
	if (_source)
		_source->onFileAdded();
}

void PhotoFrameActor::onFileRemoved( QString strFileName )
{}

void PhotoFrameActor::onFileNameChanged( QString strOldFileName, QString strNewFileName )
{}

void PhotoFrameActor::onFileModified( QString strFileName )
{}

vector<QString> PhotoFrameActor::getWatchDir()
{
	if (_source)
		return _source->getWatchDir();
	return vector<QString>();
}

bool PhotoFrameActor::serializeToPb(PbBumpObject * pbObject)
{
	assert(pbObject);

	// serialize the core actor properties
	if (!FileSystemActor::serializeToPb(pbObject))
		return false;

	if (_source)
	{
		// write the photo frame type
		pbObject->SetExtension(PbPhotoFrameActor::source_type, (unsigned int) _source->getType());

		// serialize the photoframe source
		if (!_source->serializeToPb(pbObject->MutableExtension(PbPhotoFrameActor::source)))
			return false;
	}

	return pbObject->IsInitialized();
}

bool PhotoFrameActor::deserializeFromPb(const PbBumpObject * pbObject)
{
	assert(pbObject);

	// deserialize the core actor properties
	if (!FileSystemActor::deserializeFromPb(pbObject))
		return false;

	// read the photoframe source
	if (pbObject->HasExtension(PbPhotoFrameActor::source_type))
	{
		PhotoFrameSource * source = NULL;
		RSSPhotoFrameSource * rssSource = NULL;
		FlickrPhotoFrameSource * flickrSource = NULL;

		// create the photo frame source
		unsigned int type = pbObject->GetExtension(PbPhotoFrameActor::source_type);
		switch (type)
		{
		case LocalImageSource:
			source = new LocalPhotoFrameSource();
			break;
		case RemoteImageSource:
			source = rssSource = new RSSPhotoFrameSource();
			break;
		case FlickrImageSource:
			source = flickrSource = new FlickrPhotoFrameSource();
			break;
		default: break;
		}

		// deserialize the photoframe source
		if (source)
		{
			const PbPhotoFrameSource& pbSource = pbObject->GetExtension(PbPhotoFrameActor::source);
			if (!source->deserializeFromPb(&pbSource))
			 	return false;

			// convert rss frames to flickr frames if possible
			if (rssSource && rssSource->isFlickrPhotoFrame())
			{
				source = flickrSource = new FlickrPhotoFrameSource(rssSource->getRSSFeedUrl());
				SAFE_DELETE(rssSource);
			}

			// set the source
			setUninitializedSource(source);
		}
	}

	return true;
}