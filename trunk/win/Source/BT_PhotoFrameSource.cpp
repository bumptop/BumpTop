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
#include "BT_FileTransferManager.h"
#include "BT_PhotoFrameSource.h"
#include "BT_QtUtil.h"
#include "PhotoFrameSource.pb.h"

PhotoFrameSource::PhotoFrameSource()
: _sourceUpdateCount(0)
, _imageUpdateCount(0)
, _textureDetail(SampledImage)
{}

PhotoFrameSource::~PhotoFrameSource()
{}

void PhotoFrameSource::setTextureBuffer(QString id)
{
	_textureBufferId = id;
}

QString PhotoFrameSource::getTextureBuffer() const
{
	return _textureBufferId;
}

void PhotoFrameSource::setTextureDetail(GLTextureDetail detail)
{
	_textureDetail = detail;
}

bool PhotoFrameSource::skipTo( QString resourceId )
{
	// find the item
	bool result = false;
	_current.invalidate();
	for (int i = 0; i < (int)_items.size(); ++i)
	{
		if (_items[i].getResourceId() == resourceId)
		{
			_current = _items[i];
			result = true;
		}
	}
	return result;
}

PhotoFrameSourceItem PhotoFrameSource::getCurrent() const
{
	return _current;
}

int PhotoFrameSource::getSize() const
{
	return _items.size();
}

int PhotoFrameSource::getSourceUpdateCount() const
{
	return _sourceUpdateCount;
}

int PhotoFrameSource::getImageUpdateCount() const
{
	return _imageUpdateCount;
}

void PhotoFrameSource::onFileAdded()
{}

vector<QString> PhotoFrameSource::getWatchDir()
{
	return vector<QString>();
}

bool PhotoFrameSource::serializeToPb(PbPhotoFrameSource * pbSource)
{
	assert(pbSource);

	// write the current resource id
	if (_current.isValid())
		pbSource->set_current_resource_id(stdString(_current.getResourceId()));
	return pbSource->IsInitialized();
}

bool PhotoFrameSource::deserializeFromPb(const PbPhotoFrameSource * pbSource)
{
	assert(pbSource);

	// read the current resource id
	if (pbSource->has_current_resource_id())
		skipTo(qstring(pbSource->current_resource_id()));

	return true;
}