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
#include "BT_GLTextureManager.h"
#include "BT_LocalPhotoFrameSource.h"
#include "BT_OverlayComponent.h"
#include "BT_PhotoFrameDialog.h"
#include "BT_QtUtil.h"
#include "BT_SceneManager.h"
#include "BT_Util.h"
#include "PhotoFrameSource.pb.h"


LocalPhotoFrameSource::LocalPhotoFrameSource()
{}

LocalPhotoFrameSource::LocalPhotoFrameSource(QString localPath)
: _localPath(localPath)
{}

void LocalPhotoFrameSource::updateDirectory(QDir dir, vector<PhotoFrameSourceItem>& items)
{
	// assume dir exists
	if (!empty(dir))
	{
		StrList dirListing = fsManager->getDirectoryContents(native(dir));
		for (int i = 0; i < dirListing.size(); ++i)
		{
			QFileInfo file(dirListing[i]);

			if (file.isSymLink()) {
				file = QFile(file.symLinkTarget());
			}

			if (file.isDir())
				// recurse into that directory
				updateDirectory(file.absoluteFilePath(), items);
			else
			{
				// XXX: we can do this smarter with watched directories?!

				// check if this is a valid image file
				QString filename = file.fileName();
				if (filename.size() > 4)
				{
					QString ext = fsManager->getFileExtension(filename);
					bool isValidImage = !ext.isEmpty() && GLOBAL(supportedExtensions).contains(ext + ".");
					if (isValidImage)
					{
						// check if we already have that item in the list
						vector<PhotoFrameSourceItem>::const_iterator itemIter = items.begin();
						vector<PhotoFrameSourceItem>::const_iterator endIter = items.end();
						bool exists = false;
						while (itemIter != endIter)
						{
							if ((*itemIter).getResourceId() == dirListing[i])
								exists = true;
							++itemIter;
						}

						// if not, add it to the list
						if (!exists)
						{
							PhotoFrameSourceItem item(dirListing[i]);
							item.setTexturePath(dirListing[i]);
							items.push_back(item);
						}
					}
				}
			}
		}
	}
}

void LocalPhotoFrameSource::requestSourceUpdate()
{
	// assume dir exists
	vector<PhotoFrameSourceItem> items;

	if(!QFileInfo(_localPath.absolutePath()).isDir()) {
		// check if this is a valid image file
		QString filename = _localPath.absolutePath();
		if (filename.size() > 4)
		{
			QString ext = fsManager->getFileExtension(filename);
			bool isValidImage = !ext.isEmpty() && GLOBAL(supportedExtensions).contains(ext + ".");
			if (isValidImage)
			{
				PhotoFrameSourceItem newItem(filename);
				newItem.setTexturePath(filename);
				items.push_back(newItem);
			} else {
				MessageClearPolicy clearPolicy;
				clearPolicy.setTimeout(4);
				scnManager->messages()->addMessage(new Message("PhotoFrameActor::resolveSourceFromString", QT_TR_NOOP("Invalid photo frame source given!"), Message::Warning, clearPolicy));
			}
		}
	} else {
		updateDirectory(_localPath, items);

		// randomize the items
		PhotoFrameSourceItem tmp;
		int n = items.size();
		srand(time(NULL));
		for (int i = 0; i < n; ++i)
		{
			int j = (rand() + rand()) % n;
			if (i != j)
			{
				tmp = items[i];
				items[i] = items[j];
				items[j] = tmp;
			}
		}
	}
	
	if (!items.empty())
	{
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
		if (!found)
			_current.invalidate();
	}
	else
	{
		_current.invalidate();
	}
}

void LocalPhotoFrameSource::onSourceUpdate(const FileTransfer& transfer)
{
	// not used
}

void LocalPhotoFrameSource::requestImageUpdate()
{
	// find the next valid image
	bool loadCurrent = !_items.empty() && _current.isValid();
	if (_current.isValid())
	{
		for (int i = 0; i < _items.size(); ++i)
		{
			if (_items[i].equals(_current))
			{
				for (int j = 1; j < _items.size(); ++j)
				{
					int nextIndex = (i+j) % _items.size();
					if (_items[nextIndex].isValid())
					{
						_current = _items[nextIndex];
						++_imageUpdateCount;
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
		++_imageUpdateCount;
	}

	if (loadCurrent)
	{
		// load the image (so that it will swap)
		if (this->_textureDetail == HiResImage)
			texMgr->loadTexture(GLTextureObject(Load|Reload, _textureBufferId, _current.getTexturePath(), HiResImage, IdlePriority));
		else
			texMgr->loadTexture(GLTextureObject(Load|Compress|Reload, _textureBufferId, _current.getTexturePath(), SampledImage, IdlePriority));
	}
}

void LocalPhotoFrameSource::onImageUpdate(const FileTransfer& transfer)
{
	// not used
}

void LocalPhotoFrameSource::preparePhotoFrameDialog(PhotoFrameDialog * dialog) const
{
	dialog->setSelectedType(PhotoFrameDialog::LocalDirectory);
	dialog->setDirectory(native(_localPath));
}

bool LocalPhotoFrameSource::unserialize(unsigned char *buf, uint &bufSz)
{
	// XXX
	// xxx-photoframe: versioning eventually
	//		if version 1 then ...
	//		else ...

	QString datStr;
	if (!SERIALIZE_READ_QSTRING(&buf, datStr, bufSz)) return false;	// The last source resource id
	skipTo(datStr);
	if (!SERIALIZE_READ_QSTRING(&buf, datStr, bufSz)) return false;
	_localPath = QDir(datStr);
	return true;
}

bool LocalPhotoFrameSource::equals(PhotoFrameSource * other) const
{
	if (other->getType() == LocalImageSource)
	{
		LocalPhotoFrameSource * otherRss = (LocalPhotoFrameSource *) other;
		return (otherRss->_localPath == this->_localPath);
	}
	return false;
}

const PhotoFrameSourceType LocalPhotoFrameSource::getType() const
{
	return LocalImageSource;
}

QString LocalPhotoFrameSource::getName() const
{
	return QT_TR_NOOP("\"%1\" Frame").arg(make_proper_case(_localPath.dirName()));
}

void LocalPhotoFrameSource::flushCache()
{
	// do nothing since we do not cache any files
}

PhotoFrameSource * LocalPhotoFrameSource::clone() const
{
	return new LocalPhotoFrameSource(native(_localPath));
}

void LocalPhotoFrameSource::onFileAdded()
{
	// just update the source again when a new file is added
	requestSourceUpdate();
}

vector<QString> LocalPhotoFrameSource::getWatchDir()
{
	vector<QString> dirs;
		dirs.push_back(native(_localPath));
	return dirs;
}

bool LocalPhotoFrameSource::serializeToPb(PbPhotoFrameSource * pbSource)
{
	assert(pbSource);

	// serialize the core photoframe source properties
	if (!PhotoFrameSource::serializeToPb(pbSource))
		return false;
	
	// write the local file path
	pbSource->SetExtension(PbLocalPhotoFrameSource::file_path, stdString(native(_localPath)));

	return pbSource->IsInitialized();
}

bool LocalPhotoFrameSource::deserializeFromPb(const PbPhotoFrameSource * pbSource)
{
	assert(pbSource);

	// deserialize the core photoframe source properties
	if (!PhotoFrameSource::deserializeFromPb(pbSource))
		return false;

	// read the local file path
	if (pbSource->HasExtension(PbLocalPhotoFrameSource::file_path))
	{
		QString filePath = qstring(pbSource->GetExtension(PbLocalPhotoFrameSource::file_path));
		_localPath = QDir(filePath);
	}

	return true;
}