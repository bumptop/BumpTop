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
#include "BT_FileSystemManager.h"
#include "BT_FileTransferManager.h"
#include "BT_GLTextureManager.h"
#include "BT_PhotoFrameDialog.h"
#include "BT_QtUtil.h"
#include "BT_RSSPhotoFrameSource.h"
#include "BT_Util.h"
#include "BT_WindowsOS.h"
#include "PhotoFrameSource.pb.h"

RSSPhotoFrameSource::RSSPhotoFrameSource()
: PhotoFrameSource()
{}

RSSPhotoFrameSource::RSSPhotoFrameSource( QString rssFeedUrl )
: PhotoFrameSource()
, _rssFeedUrl(rssFeedUrl)
{
	// create a temporary directory to store downloaded images
	createLocalCacheDirectory();
}

RSSPhotoFrameSource::~RSSPhotoFrameSource()
{	
	// stop all transfers to this object
	ftManager->removeTransfersToHandler(this);
}

void RSSPhotoFrameSource::requestSourceUpdate()
{
	// just load the images from cache if there is no internet connection
	if (!ftManager->hasInternetConnection())
	{
		loadPhotosFromCache();
		return;
	}

	// remove existing downloads
	ftManager->removeTransfersToHandler(this);

	// ensure cache directory exists
	createLocalCacheDirectory();

	// download the rss feed
	ftManager->addTransfer(FileTransfer(FileTransfer::Download, this)
		.setUrl(_rssFeedUrl)
		.setTimeout(60)
		.setUserData(new bool(false))	// !isImageNotSource
		.setTemporaryString(), false
		);
}

void RSSPhotoFrameSource::onSourceUpdate(const FileTransfer& transfer)
{
	// load the rss feed
	vector<PhotoFrameSourceItem> items;
	QString rssFeedStr = transfer.getTemporaryString();
	if (!rssFeedStr.isEmpty())
	{
		// NOTE: most sites use the <media:content ... /> tag to denote full images, with the exception of flickr
		// which uses <link .../>, however, all sites have an attribute 'type="image/<format>"' which is what we
		// first search for.  Once we have these set of tags, we drill down for the actual image path, and extract
		// the image from that.
		// 
		// On the first try, use tags with 'image/type' mime type set
		// On the second try, use any tag with a height
		// On the third and final try, use all text
		QRegExp imgTagRe("(<[^>]*image/(jpg|jpeg|gif|png)[^>]*>)");
		QRegExp imgUrlRe("(http://[\\w\\.-]+(:\\d+)?(/[\\w\\.\\+-]+)+\\.(jpg|jpeg|gif|png))");
		imgTagRe.setCaseSensitivity(Qt::CaseInsensitive);
		imgUrlRe.setCaseSensitivity(Qt::CaseInsensitive);
		int tries = 0;
		while (tries < 3)
		{
			int offset = 0;
			while (offset > -1 && offset < rssFeedStr.size())
			{
				offset = imgTagRe.indexIn(rssFeedStr, offset);
				offset += imgTagRe.matchedLength();
				QStringList tagMatches = imgTagRe.capturedTexts();
				for (int i = 1; i < tagMatches.size(); ++i)
				{
					if (tagMatches[i].isEmpty())
						continue;

					imgUrlRe.indexIn(tagMatches[i]);
					QStringList urlMatches = imgUrlRe.capturedTexts();
					for (int j = 1; j < urlMatches.size(); ++j)
					{
						if (urlMatches[j].isEmpty())
							continue;

						// extract the image filename from the url
						QString imgUrl = urlMatches[j];
						QString imgFilename = imgUrl.mid(imgUrl.lastIndexOf('/') + 1);
						QString localFilename = native(make_file(_localCacheDirectory, imgFilename));

						// set the expected local texture path
						if (!localFilename.isEmpty())
						{
							// ensure that the url does not already exist
							bool resourceAlreadyExists = false;
							vector<PhotoFrameSourceItem>::const_iterator citer = items.begin();
							while ((citer != items.end()) && !resourceAlreadyExists)
							{
								resourceAlreadyExists = (citer->getResourceId() == imgUrl);
								citer++;
							}

							if (!resourceAlreadyExists)
							{
								PhotoFrameSourceItem item(imgUrl);			
								convertToValidDirectoryName(imgUrl);
								item.setTexturePath(localFilename);
								items.push_back(item);
							}
						}
						break;
					}
					break;
				}
			}

			// fall back to using a height attribute check if the image type attribute check fails
			if (items.empty())
			{
				if (tries == 0)
					imgTagRe = QRegExp("(<[^>]*height[^>]*>)");
				else
					imgTagRe = imgUrlRe;
				imgTagRe.setCaseSensitivity(Qt::CaseInsensitive);
				++tries;
			}
			else
				break;
		}

		// only keep the intersection of the photos preloaded in the directory (if any) and
		// the xml file
		if (exists(_localCacheDirectory) && !items.empty())
		{
			StrList cacheDirListing = fsManager->getDirectoryContents(native(_localCacheDirectory));
			StrList::iterator iter = cacheDirListing.begin();
			bool wasNotEmptyBefore = !cacheDirListing.empty();
			while (iter != cacheDirListing.end())				
			{	
				const QString& filename = *iter;

				// remove it if it's an xml file
				QString ext = fsManager->getFileExtension(filename);
				if (ext == ".xml")
				{
					iter = cacheDirListing.erase(iter);
					wasNotEmptyBefore = !cacheDirListing.empty();
					continue;
				}

				bool found = false;
				for (int j = 0; j < items.size(); ++j)
				{
					if (items[j].getTexturePath() == filename)
					{
						found = true;
						break;
					}
				}

				// remove it if it is not found in the current xml
				if (!found)
				{
					QFile(filename).remove();
				}

				iter++;
			}	

			// assert that there are still items left?
			StrList newCacheDirListing = fsManager->getDirectoryContents(native(_localCacheDirectory));			
			assert(!newCacheDirListing.empty() || !wasNotEmptyBefore);
		}

		// set the items
		_items = items;
		++_sourceUpdateCount;

		// check if the current item is still valid
		bool found = false;
		for (int i = 0; i < _items.size(); ++i)
		{
			if (_items[i].equals(_current))
			{
				found = true;
			}
		}
		// reset the current image to the first in the new list if it is not valid
		if (!found && !_items.empty())
			_current = _items[0];
	}
}

void RSSPhotoFrameSource::requestImageUpdate()
{
	// don't start another transfer if one is already in progress
	if (ftManager->hasTransfersToHandler(this))
		return;

	// find the next image to load
	bool loadCurrent = !_items.empty() && _current.isValid();
	if (_current.isValid())
	{
		for (int i = 0; i < _items.size(); ++i)
		{
			// find the current item
			if (_items[i].equals(_current))
			{
				// get the next item 
				for (int j = 1; j < _items.size(); ++j)
				{
					int nextIndex = (i+j) % _items.size();
					if (_items[nextIndex].isValid())
					{
						_current = _items[nextIndex];	
						loadCurrent = true;
						break;
					}
				}
				break;
			}
		}
	}
	else if (!_items.empty())
	{
		_current = _items[0];
		loadCurrent = _current.isValid();		
	}

	if (loadCurrent)
	{
		if (exists(_current.getTexturePath()))
		{
			// load the image (so that it will swap)
			GLTextureObject obj(Load|Compress|Reload, _textureBufferId, _current.getTexturePath(), SampledImage, IdlePriority);
			texMgr->loadTexture(obj);

			// bump up image loaded count
			++_imageUpdateCount;
		}
		else
		{
			// download the image to a tmp filename
			QString tmpFilename = _current.getTexturePath() + QT_NT(".img_tmp");
			ftManager->addTransfer(FileTransfer(FileTransfer::Download, this)
				.setUrl(_current.getResourceId())
				.setUserData(new bool(true))
				.setLocalPath(tmpFilename), false
				);
		}
	}
}

void RSSPhotoFrameSource::onImageUpdate(const FileTransfer& transfer)
{
	// rename the image back to the original filename
	QString texPath = _current.getTexturePath();
	QFile::rename(transfer.getLocalPath(), texPath);

	// load the image (so that it will swap)
	GLTextureObject obj(Load|Compress|Reload, _textureBufferId, _current.getTexturePath(), SampledImage, IdlePriority);
	texMgr->loadTexture(obj);

	// bump up image loaded count
	++_imageUpdateCount;
}

void RSSPhotoFrameSource::preparePhotoFrameDialog(PhotoFrameDialog * dialog) const
{
	QString flickrProtocol("flickr://");
	if (_rssFeedUrl.startsWith(flickrProtocol, Qt::CaseInsensitive))
	{
		dialog->setSelectedType(PhotoFrameDialog::FlickrTags);
		QString tags = _rssFeedUrl.mid(flickrProtocol.size());
		dialog->setFlickrTag(tags);
	}
	else
	{
		dialog->setSelectedType(PhotoFrameDialog::RSSFeed);
		dialog->setRawFeed(_rssFeedUrl);
	}
}

bool RSSPhotoFrameSource::unserialize(unsigned char *buf, uint &bufSz)
{
	// XXX
	// xxx-photoframe: versioning eventually
	//		if version 1 then ...
	//		else ...

	QString datStr;
	if (!SERIALIZE_READ_QSTRING(&buf, datStr, bufSz)) return false;	// The last source resource id
	skipTo(datStr);
	if (!SERIALIZE_READ_QSTRING(&buf, datStr, bufSz)) return false;
	_rssFeedUrl = datStr;
	createLocalCacheDirectory();
	return true;
}

bool RSSPhotoFrameSource::equals(PhotoFrameSource * other) const
{
	if (other->getType() == RemoteImageSource)
	{
		RSSPhotoFrameSource * otherRss = (RSSPhotoFrameSource *) other;
		return (otherRss->_rssFeedUrl == this->_rssFeedUrl);
	}
	return false;
}

const PhotoFrameSourceType RSSPhotoFrameSource::getType() const
{
	return RemoteImageSource;
}

QString RSSPhotoFrameSource::getName() const
{
	if (_rssFeedUrl.contains("http://api.flickr.com"))
	{
		// the default feed
		if (_rssFeedUrl == "http://api.flickr.com/services/feeds/photos_faves.gne?nsid=26799028@N06&lang=en-us&format=rss_200")
			return QT_TR_NOOP("Flickr Frame: BumpTop Favorites");

		// for backwards compatibility, we're going to just parse the url...
		// we'll have to update this once we use the proper way to access flickr
		const QString pre("http://api.flickr.com/services/feeds/photos_public.gne?tags=");
		const QString post("&lang=en-us&format=atom");
		int preIter = _rssFeedUrl.indexOf(pre);
		int postIter = _rssFeedUrl.indexOf(post);
		if ((preIter > -1) && (postIter > -1))
		{
			preIter += pre.size();
			QString tags = _rssFeedUrl.mid(preIter, postIter - preIter);
			tags.replace("%20", " ");
			tags = make_proper_case(tags);
			return QT_TR_NOOP("Flickr Frame: %1").arg(tags);
		}
		else
			return QT_TR_NOOP("Flickr Frame");
	}
	else
	{
		return "RSS Photoframe";
	}
}

void RSSPhotoFrameSource::createLocalCacheDirectory()
{
	// use a hash of the feed url as the dir name
	QString dir = _rssFeedUrl;
	convertToValidDirectoryName(dir);

	// create this directory if necessary
	QString framesDir = native(winOS->GetFramesDirectory() / "RSS");
	QDir().mkpath(framesDir);
	_localCacheDirectory = QDir(framesDir) / dir;
	QDir().mkpath(native(_localCacheDirectory));
}

void RSSPhotoFrameSource::convertToValidDirectoryName( QString& pathStr )
{
	pathStr = QString(QCryptographicHash::hash(pathStr.toUtf8(), QCryptographicHash::Md5).toHex());
}

void RSSPhotoFrameSource::flushCache()
{
	// delete all the files in the cache dir
	createLocalCacheDirectory();
	StrList cacheDirListing = fsManager->getDirectoryContents(native(_localCacheDirectory));
	for (int i = 0; i < cacheDirListing.size(); ++i)
	{
		QFile::remove(cacheDirListing[i]);
	}
	QFile::remove(native(_localCacheDirectory));
}

PhotoFrameSource * RSSPhotoFrameSource::clone() const
{
	return new RSSPhotoFrameSource(_rssFeedUrl);
}

bool RSSPhotoFrameSource::loadPhotosFromCache()
{
	QDir cacheDir = _localCacheDirectory;
	if (exists(cacheDir))
	{
		if (_items.empty())
		{
			StrList cacheDirListing = fsManager->getDirectoryContents(native(cacheDir));
			StrList::iterator iter = cacheDirListing.begin();
			while (iter != cacheDirListing.end())
			{	
				const QString& filename = *iter;

				// remove it if it's an xml file
				QString ext = fsManager->getFileExtension(filename);
				if (ext == ".xml" ||
					ext == ".tmp")
				{
					iter = cacheDirListing.erase(iter);
					continue;
				}

				PhotoFrameSourceItem item(*iter);
				item.setTexturePath(*iter);
				_items.push_back(item);
				++_sourceUpdateCount;
				iter++;
			}
		}
	}
	return true;
}

bool RSSPhotoFrameSource::isFlickrPhotoFrame()
{
	return (_rssFeedUrl.startsWith(QT_NT("http://api.flickr.com"), Qt::CaseInsensitive) || 
			_rssFeedUrl.startsWith(QT_NT("flickr://"), Qt::CaseInsensitive));
}

QString RSSPhotoFrameSource::getRSSFeedUrl()
{
	return _rssFeedUrl;
}

bool RSSPhotoFrameSource::serializeToPb(PbPhotoFrameSource * pbSource)
{
	assert(pbSource);

	// serialize the core photoframe source properties
	if (!PhotoFrameSource::serializeToPb(pbSource))
		return false;
	
	// write the feed url
	if (_current.isValid())
		pbSource->SetExtension(PbRSSPhotoFrameSource::feed_url, stdString(_rssFeedUrl));

	return pbSource->IsInitialized();
}

bool RSSPhotoFrameSource::deserializeFromPb(const PbPhotoFrameSource * pbSource)
{
	assert(pbSource);

	// deserialize the core photoframe source properties
	if (!PhotoFrameSource::deserializeFromPb(pbSource))
		return false;

	// read the feed url
	if (pbSource->HasExtension(PbRSSPhotoFrameSource::feed_url))
	{
		_rssFeedUrl = qstring(pbSource->GetExtension(PbRSSPhotoFrameSource::feed_url));
		createLocalCacheDirectory();
	}

	return true;
}

void RSSPhotoFrameSource::onTransferComplete(const FileTransfer& transfer)
{
	bool * isImageNotSource = (bool *) transfer.getUserData();

	// delegate to the correct function
	if (isImageNotSource && *isImageNotSource)
		onImageUpdate(transfer);
	else
		onSourceUpdate(transfer);

	// free temporary data
	if (isImageNotSource)
		delete isImageNotSource;
}

void RSSPhotoFrameSource::onTransferError(const FileTransfer& transfer)
{
	// mark it down, but just skip the handling
	consoleWrite(QString("Could not load PhotoFrame URL: %1\n").arg(transfer.getUrl()));
	
	bool * isImageNotSource = (bool *) transfer.getUserData();
	if (isImageNotSource)
		delete isImageNotSource;
}