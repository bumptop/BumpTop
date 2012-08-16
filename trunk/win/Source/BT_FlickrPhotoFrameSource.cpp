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
#include "BT_FlickrPhotoFrameSource.h"
#include "BT_GLTextureManager.h"
#include "BT_LegacyPersistenceManager.h"
#include "BT_OverlayComponent.h"
#include "BT_PhotoFrameDialog.h"
#include "BT_QtUtil.h"
#include "BT_SceneManager.h"
#include "BT_Util.h"
#include "BT_WindowsOS.h"
#include "FlickrClient.h"
#include "PhotoFrameSource.pb.h"

FlickrPhotoFrameSource::FlickrPhotoFrameSource( QString rssFeed )
: RSSPhotoFrameSource(rssFeed)
{
	initialize(rssFeed);
	createLocalCacheDirectory();
}

FlickrPhotoFrameSource::FlickrPhotoFrameSource()
: RSSPhotoFrameSource()
{
}

void FlickrPhotoFrameSource::createLocalCacheDirectory()
{
	// use a hash of the feed url as the dir name
	QString dir;
	if (!_tag.isNull() && !_tag.isEmpty())
		dir = _tag;
	else if (!_groupId.isNull() && !_groupId.isEmpty())
		dir = _groupId;
	else
		dir = _userId;
	convertToValidDirectoryName(dir);

	// create this directory if necessary
	QString framesDir = native(winOS->GetFramesDirectory() / "Flickr");
	QDir().mkpath(framesDir);
	_localCacheDirectory = QDir(framesDir) / dir;
	QDir().mkpath(native(_localCacheDirectory));
}

void FlickrPhotoFrameSource::requestSourceUpdate()
{
	// just load the images from cache if there is no internet connection
	if (!ftManager->hasInternetConnection())
		loadPhotosFromCache();

	// ensure we have a valid flickr source
	if (!isValidFlickrPhotoFrame())
		return;

	// remove existing downloads
	ftManager->removeTransfersToHandler(this);

	// See how many photos we currently have cached
	int maxNumFilesToCache = 64;
	createLocalCacheDirectory();
	StrList filesToDelete = fsManager->getDirectoryContents(native(_localCacheDirectory));
	if (filesToDelete.size() > maxNumFilesToCache)
	{
		for (int i = 0; i < filesToDelete.size(); ++i)
		{
			QFile::remove(filesToDelete[i]);
		}
	}

	// Determine what kind of photos to download
	// The order is important here since both getting photos from favourite
	// and getting photos from a specific user both involve the same parameter
	FlickrClient flickrClient(FLICKR_AUTH_TOKEN);
	if (_rssFeedUrl.contains("photos_faves"))
		flickrClient.requestFavouritePhotosFromId(_userId, this);
	else if (!_userId.isEmpty())
		flickrClient.requestPhotosFromId(_userId, false, this);
	else if (!_groupId.isEmpty())
		flickrClient.requestPhotosFromId(_groupId, true, this);
	else
		flickrClient.requestPhotosByTag(_tag, this);
}

void FlickrPhotoFrameSource::onSourceUpdate(const FileTransfer& transfer)
{
	vector<PhotoFrameSourceItem> items;

	// extract the urls froms the transfer result and create associated
	// photo frame source items
	FlickrClient flickrClient(FLICKR_AUTH_TOKEN);
	vector<QString> urls = flickrClient.extractPhotoUrls(transfer);
	for (int i = 0; i < urls.size(); i++)
	{
		PhotoFrameSourceItem item(urls[i]);			
		//convertToValidDirectoryName(urls[i]);
		QString url = urls[i];
		url = url.right(url.size() - url.lastIndexOf("/") - 1);
		QString localFileName = native(make_file(_localCacheDirectory, url));
		item.setTexturePath(localFileName);
		items.push_back(item);
	}
	
	// set the new items and increment update count
	_items = items;
	++_sourceUpdateCount;
}

bool FlickrPhotoFrameSource::unserialize(unsigned char *buf, uint &bufSz) 
{
	
	// XXX
	// xxx-photoframe: versioning eventually
	//		if version 1 then ...
	//		else ...

	// Read each parameter
	QString datStr;
	QString rssFeed, groupId, tag, userId;
	bool success = true;

	if (!SERIALIZE_READ_QSTRING(&buf, datStr, bufSz)) return false;	// The last source resource id
	skipTo(datStr);
	
	// In version 14 we removed serializing the rssFeed in FlickrPhotoFrames
	// now the rssFeed is sourced in FlickrPhotoFrames parent, RSSPhotoFrame
	if (SCENE_VERSION_NUMBER < 14)
	{
		if (!SERIALIZE_READ_QSTRING(&buf, datStr, bufSz)) 
			return false;
		rssFeed = datStr;
	}

	if (!SERIALIZE_READ_QSTRING(&buf, datStr, bufSz)) 
		return false;
	groupId = datStr;

	if (!SERIALIZE_READ_QSTRING(&buf, datStr, bufSz)) 
		return false;
	tag = datStr;

	if (!SERIALIZE_READ_QSTRING(&buf, datStr, bufSz)) 
		return false;
	userId = datStr;

	// If we are using an old scene, store the rssFeed inside the RSSPhotoFrameSource
	// member _rssFeedUrl
	if (SCENE_VERSION_NUMBER < 14)
		_rssFeedUrl = rssFeed;
	_tag = tag;
	_groupId = groupId;
	_userId = userId;

	// Create the FlickrFrames directory to dump photos in
	createLocalCacheDirectory();
	return true;
}

bool FlickrPhotoFrameSource::equals(PhotoFrameSource * other) const
{
	if (other->getType() == FlickrImageSource)
	{
		FlickrPhotoFrameSource * otherFlickr = (FlickrPhotoFrameSource *) other;
		return (otherFlickr->_rssFeedUrl == _rssFeedUrl);
	}
	return false;
}

PhotoFrameSource * FlickrPhotoFrameSource::clone() const
{
	FlickrPhotoFrameSource *newPhotoFrame = new FlickrPhotoFrameSource(_rssFeedUrl);
	return newPhotoFrame;
}

/*
* This function will return the name "Flickr Frame: xxx"
* where xxx is either a user name, group name or a tag
*/
QString FlickrPhotoFrameSource::getName() const
{
	FlickrClient flickrClient(FLICKR_AUTH_TOKEN);
	if (!_tag.isEmpty())
	{
		QString tag = _tag;
		tag.replace("+", " ");
		return QT_TR_NOOP("Flickr Frame: %1").arg(tag);
	}
	else if (!_groupId.isEmpty())
	{		
		return QT_TR_NOOP("Flickr Frame: %1").arg(flickrClient.getGroupName(_groupId));
	}
	else if (!_userId.isEmpty())
	{
		return QT_TR_NOOP("Flickr Frame: %1").arg(flickrClient.getUsername(_userId));
	}
	return QT_TR_NOOP("Flickr Frame");
}

const PhotoFrameSourceType FlickrPhotoFrameSource::getType() const
{
	return FlickrImageSource;
}

void FlickrPhotoFrameSource::preparePhotoFrameDialog(PhotoFrameDialog * dialog) const
{
	QString flickrProtocol("flickr://");
	if ((_rssFeedUrl.startsWith(flickrProtocol, Qt::CaseInsensitive))||(_rssFeedUrl.isEmpty()))
	{
		// On BumpTop startup, the rssFeed variable is blank; 
		// so tags should be passed from the _tag variable instead
		// of being parsed from rssFeed.
		dialog->setSelectedType(PhotoFrameDialog::FlickrTags);
		dialog->setFlickrTag(_tag);
	}
	else
	{
		dialog->setSelectedType(PhotoFrameDialog::RSSFeed);
		dialog->setRawFeed(_rssFeedUrl);
	}
}

/*
* Determine what kind of RSS feed was given. 
* Will store the appropriate value in either _groupId, _userId, or _tag
*/
void FlickrPhotoFrameSource::initialize( QString rssFeed )
{
	QString groupPool("groups_pool");
	QString publicPool("photos_public");
	QString favouritePool("photos_faves");
	QString flickrProtocol("flickr://");

	if (rssFeed.contains(groupPool))
	{
		_groupId = getIdFromRSSFeed(rssFeed);
	}
	else if (rssFeed.contains(favouritePool))
	{
		_userId = getIdFromRSSFeed(rssFeed);
	}
	else if (rssFeed.startsWith(flickrProtocol))
	{
		_tag = getTagFromFlickrFeed(rssFeed);
	}
	else if (rssFeed.contains(publicPool))
	{
		QString publicPhotosByTag("photos_public.gne?tags");
		QString publicPhotosById("photos_public.gne?id");
		if (rssFeed.contains(publicPhotosByTag))
			_tag = getTagFromFlickrFeed(rssFeed);
		else if (rssFeed.contains(publicPhotosById))
			_userId = getIdFromRSSFeed(rssFeed);
	}
	else
	{
		printError("Invalid photo frame source given!");
	}
}

/*
* Strip the nsid from a flickr rss feed
*/
QString FlickrPhotoFrameSource::getIdFromRSSFeed (QString rssFeed)
{
	// This regular expression will capture ids of the form
	// {digits}@N{digits} e.g. 47208242@N00
	// All flickr user ids will have this form
	// http://tech.groups.yahoo.com/group/yws-flickr/message/3509
	QRegExp validId("\\d+@N\\d+");

	// strip out the ID from the RSS Feed
	// example RSS Feed: http://api.flickr.com/services/feeds/photos_public.gne?id=47208242@N00&lang=en-us&format=rss_200

	QString id = rssFeed.right(rssFeed.size() - rssFeed.indexOf("id=") - 3);
	id = id.left(id.indexOf("&"));
	if (validId.exactMatch(id))
	{
		return id;
	}
	else
	{
		printError(QT_TR_NOOP("Invalid RSS feed given!"));
		return "";
	}
}

/*
* Given a flickr feed remove the flickr:// and also replace all remaining white space
* with %20
*/
QString FlickrPhotoFrameSource::getTagFromFlickrFeed (QString rssFeed)
{
	QString tag;
	if (rssFeed.startsWith("flickr://", Qt::CaseInsensitive))
	{
		// Strip the flickr:// portion from the rssFeed
		tag = rssFeed.mid(9);
	}
	else if (rssFeed.startsWith("http://", Qt::CaseInsensitive))
	{
		// The form of a flickr rss feed with tags is
		// http://api.flickr.com/services/feeds/photos_public.gne?tags=XXX&
		// We need to strip out all the text and determine what XXX is
		// Documentation for the format of Flickr RSS Feeds can be found at
		// http://www.flickr.com/services/feeds/docs/photos_public/
		QString tagParam("tags=");
		tag = rssFeed.right(rssFeed.size() - rssFeed.indexOf(tagParam) - tagParam.length());
		tag = tag.left(tag.indexOf("&"));
	}

	// Tags are separated by comma's. Check each tag and see if any of them contain spaces, if they do replace the space with a +
	QStringList tagTokens = tag.split(",", QString::SkipEmptyParts);
	for (int i = 0;i<tagTokens.size();i++)
	{
		// if the first character is a space, remove it
		if (tagTokens[i].startsWith(" "))
			tagTokens[i].remove(0, 1);

		tagTokens[i].replace(" ", "+");
	}

	// Reconstruct tag
	tag = "";
	for (int i = 0;i<tagTokens.size();i++)
	{
		tag.append(tagTokens[i]);
		if (i + 1 < tagTokens.size())
			tag.append(",");
	}

	// Return tag
	return tag;
}

/*
* Return true if one of the three main parameters is non-empty
*/
bool FlickrPhotoFrameSource::isValidFlickrPhotoFrame()
{             
	return (!_tag.isEmpty() || !_userId.isEmpty() || !_groupId.isEmpty());
}

void FlickrPhotoFrameSource::printError( QString errorMessage )
{
	// Print error message
	MessageClearPolicy clearPolicy;
	clearPolicy.setTimeout(3);
	scnManager->messages()->addMessage(new Message("FlickrPhotoFrameSource::printError", errorMessage, Message::Warning, clearPolicy));
}

bool FlickrPhotoFrameSource::serializeToPb(PbPhotoFrameSource * pbSource)
{
	assert(pbSource);

	// serialize the core photoframe source properties
	if (!PhotoFrameSource::serializeToPb(pbSource))
		return false;
	
	// write the group id, tag id, user id
	pbSource->SetExtension(PbFlickrPhotoFrameSource::group_id, stdString(_groupId));
	pbSource->SetExtension(PbFlickrPhotoFrameSource::tag_id, stdString(_tag));
	pbSource->SetExtension(PbFlickrPhotoFrameSource::user_id, stdString(_userId));

	return pbSource->IsInitialized();
}

bool FlickrPhotoFrameSource::deserializeFromPb(const PbPhotoFrameSource * pbSource)
{
	assert(pbSource);

	// deserialize the core photoframe source properties
	if (!RSSPhotoFrameSource::deserializeFromPb(pbSource))
		return false;

	// read the group id, tag id, user id
	if (pbSource->HasExtension(PbFlickrPhotoFrameSource::group_id))
		_groupId = qstring(pbSource->GetExtension(PbFlickrPhotoFrameSource::group_id));	
	if (pbSource->HasExtension(PbFlickrPhotoFrameSource::tag_id))
		_tag = qstring(pbSource->GetExtension(PbFlickrPhotoFrameSource::tag_id));	
	if (pbSource->HasExtension(PbFlickrPhotoFrameSource::user_id))
		_userId = qstring(pbSource->GetExtension(PbFlickrPhotoFrameSource::user_id));
	createLocalCacheDirectory();

	return true;
}