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
#include "BT_FileSystemActor.h"
#include "BT_FileSystemManager.h"
#include "BT_OverlayComponent.h"
#include "BT_SceneManager.h"
#include "BT_StickyNoteActor.h"
#include "BT_StickyNoteMenuActionCustomizer.h"
#include "BT_WebThumbnailActor.h"
#include "BumpTop.h"

FileSystemActor* FileSystemActorFactory::createFileSystemActor( const QString& path )
{	
#if ENABLE_WEBKIT
	if (GLOBAL(settings).freeOrProLevel == AL_PRO && path.endsWith(QT_NT(".url"), Qt::CaseInsensitive))
	{
		return new WebThumbnailActor();
	}
	else
#endif
	{
		// check if this is a post it note
		if (fsManager->getFileExtension(path) == ".txt")
		{
			if (getStickyNote(path, NULL))
			{
				if (!StickyNoteMenuActionCustomizer::hasExceededMaxNumStickyNotes())
					return new StickyNoteActor();
				else
					printUnique("FileSystemActor::setFilePath", QT_TR_NOOP("Upgrade to the Pro version of BumpTop to get more Stickies\nand a bunch of other great features!"));
			}
		}	
		return new FileSystemActor();
	}
}

QString FileSystemActorFactory::parseURLFileForLink(const QString& path)
{
	QString url;

	// parse out the URL
	QFile file(path);
	if (file.open(QFile::ReadOnly))
	{
		// set the codec (it will use the local code page otherwise) for decoding the text stream
		QTextStream stream(&file);
		stream.setCodec(QT_NT("UTF-8"));
		assert(stream.status() == QTextStream::Ok);

		// read the contents
		QString line = stream.readLine();
		while (!line.isNull())
		{
			if (line.trimmed() == "[InternetShortcut]")
			{
				QString secondLine = stream.readLine().trimmed();
				if (secondLine.startsWith(QT_NT("URL=")))
				{
					url = secondLine.mid(4);
					break;
				}
			}	
			line = stream.readLine();
		}

		// close the file
		file.close();
	}

	return url;
}