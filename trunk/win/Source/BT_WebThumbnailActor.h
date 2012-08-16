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

#pragma once

#ifndef _BT_WEB_THUMBNAIL_ACTOR_
#define _BT_WEB_THUMBNAIL_ACTOR_

// -----------------------------------------------------------------------------

#include "BT_FileSystemActor.h"
#include "BT_FileTransferManager.h"

class WebPage;

class WebThumbnailActor : public QObject, public FileSystemActor
{
	Q_OBJECT 

private:
	QString _site_url;
	QString _thumbnail_url;
#if ENABLE_WEBKIT
	WebPage * _webpage;
#endif

	QDir					getCacheDirectory();
	QFileInfo				getCachedThumbnailPath(QString site_url);
	void					startThumbnailDownload(QString thumb_url);
	void					loadThumbnail(QString path);
	void					loadFileIcon();

public:
	WebThumbnailActor();
	virtual ~WebThumbnailActor();

	static bool				isValidWebThumbnailActorUrlFile(const QString& url);

	virtual void			setFilePath(QString fullPath, bool skipTextureResolution=false);
	QString					getSiteURL(); // parses the .url file and gets the link it's pointing to

	// timer callback
	void					onUpdate();

public slots:
	void					webPageThumbnailLoaded(const QImage&, const QRect&);

};

// -----------------------------------------------------------------------------

#endif