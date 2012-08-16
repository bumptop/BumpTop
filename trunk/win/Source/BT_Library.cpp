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
#include "BT_Library.h"
#include <shobjidl.h>
#include "BT_Windows7User32Override.h"
#include "BT_GLTextureManager.h"
#include "BT_Util.h"

Library::Library()
: _name()
, _folderPaths()
, _textureKey()
, _hashKey()
, _valid(false)
{}

Library::~Library()
{}

QString Library::getName() const
{
	return _name;
}

const QList<QString> Library::getFolderPaths() const
{
	return _folderPaths;
}

const QString Library::getIconTextureKey() const
{
	return _textureKey;
}

bool Library::isValid() const
{
	return _valid;
}

QString Library::getHashKey() const
{
	return _hashKey;
}

LibraryImpl::LibraryImpl(QString& libraryName)
{
	setName(libraryName);
}

LibraryImpl::~LibraryImpl()
{}

void LibraryImpl::setName(QString& name)
{
	_name = name;
}

void LibraryImpl::setIconPath(QString& iconPath)
{
	QString texKey = QString_NT("library.%1.icon").arg(_name);
	setTextureKey(texKey);
	QString oldPath = texMgr->getTexturePathFromId(texKey);
	if (oldPath != iconPath)
	{
		// If the old and new paths are different, delete then load
		texMgr->deleteTexture(texKey);
		texMgr->loadTexture(GLTextureObject(Load, texKey, iconPath, FileIcon, NormalPriority, false, false, true));
	}
}

void LibraryImpl::setTextureKey(QString& key)
{
	_textureKey = key;
}

void LibraryImpl::setFolderPaths(QList<QString>& folderPaths)
{
	_folderPaths = folderPaths;
}

void LibraryImpl::setValid(bool value)
{
	_valid = value;
}

void LibraryImpl::setHashKey(QString &key)
{
	_hashKey = key;
}