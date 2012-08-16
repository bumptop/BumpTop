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
#include "BT_PhotoFrameSourceItem.h"

PhotoFrameSourceItem::PhotoFrameSourceItem()
: _isValid(false)
{}

PhotoFrameSourceItem::PhotoFrameSourceItem(QString resourceId)
: _resourceId(resourceId)
, _isValid(true)
{}

QString PhotoFrameSourceItem::getResourceId() const
{
	return _resourceId;
}

void PhotoFrameSourceItem::setTexturePath(QString path)
{
	_texturePath = path;
}

QString PhotoFrameSourceItem::getTexturePath() const
{
	return _texturePath;
}

bool PhotoFrameSourceItem::isValid() const
{
	return _isValid;
}

void PhotoFrameSourceItem::invalidate()
{
	_isValid = false;
	_resourceId.clear();
	_texturePath.clear();
}

bool PhotoFrameSourceItem::equals(const PhotoFrameSourceItem& other)
{
	return (_isValid && other._isValid && 
		_resourceId == other._resourceId &&
		_texturePath == other._texturePath);
}