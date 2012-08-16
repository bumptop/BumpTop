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
#include "BT_Authorization.h"
#include "BT_FileSystemActorFactory.h"
#include "BT_GLTextureManager.h"
#include "BT_StatsManager.h"
#include "BT_WebPage.h"
#include "BT_WebThumbnailActor.h"
#include "BT_WindowsOS.h"
#include "moc/moc_BT_WebThumbnailActor.cpp"

bool WebThumbnailActor::isValidWebThumbnailActorUrlFile(const QString& url)
{
	return (GLOBAL(settings).freeOrProLevel == AL_PRO) && 
		url.endsWith(QT_NT(".url"), Qt::CaseInsensitive);
}

void WebThumbnailActor::setFilePath( QString fullPath, bool skipTextureResolution/*=false*/ )
{
	FileSystemActor::setFilePath(fullPath, false);

	// use the file name up to the '.url' suffix for the name
	QFileInfo info(fullPath);
	setText(info.completeBaseName());

	if (getSiteURL().isEmpty())
	{
		loadFileIcon();
	}
	else
	{
		if (exists(getCachedThumbnailPath(getSiteURL())) && getCachedThumbnailPath(getSiteURL()).size() != 0) {
			loadThumbnail(native(getCachedThumbnailPath(getSiteURL())));
		}
		else if (statsManager->getStats().bt.interaction.actors.fs_types.webthumbnailsCreated > 1000)
		{
			loadFileIcon();
		} 
		else {
			loadFileIcon();

			_thumbnail_url = getSiteURL();
			QSize texDims(256, 256); 
			switch (GLOBAL(settings).visuals)
			{
			case LowVisuals:
				texDims = QSize(128, 128);
				break;
			case HighVisuals:
				texDims = QSize(512, 512);
				break;
			}
#if ENABLE_WEBKIT
			// create a new webpage instance
			SAFE_DELETE(_webpage);
			_webpage = new WebPage();

			// intialize and load the page
			connect(_webpage, SIGNAL(pageUpdate(const QImage&, const QRect&)), this, SLOT(webPageThumbnailLoaded(const QImage&, const QRect&)));
			_webpage->init(texDims, false);
			_webpage->load(_thumbnail_url);
#endif
		}
	}

}

WebThumbnailActor::WebThumbnailActor()
: FileSystemActor()
#if ENABLE_WEBKIT
, _webpage(NULL)
#endif
{
	pushFileSystemType(WebThumbnail);
	statsManager->getStats().bt.interaction.actors.fs_types.webthumbnailsCreated++;
}

WebThumbnailActor::~WebThumbnailActor()
{

}

QDir WebThumbnailActor::getCacheDirectory()
{
	QDir webThumbnailsDir = winOS->GetDataDirectory() / "WebThumbnails";
	if (!exists(webThumbnailsDir))
		QDir().mkpath(native(webThumbnailsDir));
	return webThumbnailsDir;
}

QFileInfo WebThumbnailActor::getCachedThumbnailPath( QString site_url )
{
	QString string_hash_value = QString(QT_NT("%1.png")).arg(QString(QCryptographicHash::hash(site_url.toUtf8(), QCryptographicHash::Md5).toHex()));
	return QFileInfo(getCacheDirectory(), string_hash_value);
}

void WebThumbnailActor::loadThumbnail( QString path )
{
	textureID = getSiteURL();
	loadThumbnailTexture(GLTextureObject(Load|Compress, textureID, path, HiResImage, NormalPriority));
}

void WebThumbnailActor::loadFileIcon()
{
	loadThumbnailTexture(GLTextureObject(Load|Compress, textureID, getTargetPath(), FileIcon, NormalPriority));
}

QString WebThumbnailActor::getSiteURL()
{
	if (_site_url.isEmpty())
		_site_url = FileSystemActorFactory::parseURLFileForLink(getTargetPath());
	return _site_url;
}

void WebThumbnailActor::webPageThumbnailLoaded( const QImage& image, const QRect& )
{
#if ENABLE_WEBKIT
	// check if the qimage is empty
	if (image.width() == 0 || image.height() == 0)
		return;

	// otherwise, save the image and then load it
	QString cachedImagePath = native(getCachedThumbnailPath(getSiteURL()));
	image.save(cachedImagePath);
	loadThumbnail(cachedImagePath);

	// set the filename to the title of the page
	if (!_webpage->getTitle().isEmpty())
		setText(_webpage->getTitle());

	// destroy the webpage instance
	SAFE_DELETE(_webpage);
#endif
}

void WebThumbnailActor::onUpdate()
{
#if ENABLE_WEBKIT
	if (_webpage)
		_webpage->onUpdate();
#endif
}