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

#include "BT_QtUtil.h"

bool GLTextureManager::hasTexturesOfState( GLTextureState state ) const
{
	if (_suspendLoading)
		return false;

	// check if we have items of that state
	TexturesList::const_iterator iter = _textures.begin();
	while (iter != _textures.end())
	{
		if (iter.value().state == state)
			return true;
		iter++;
	}
	return false;
}

bool GLTextureManager::hasTextureDetail( QString key, unsigned int detail ) const
{
	if (_suspendLoading)
		return false;

	TexturesList::const_iterator iter = _textures.find(key);
	// assert(iter != _textures.end());
	if (iter != _textures.end())
	{
		// NOTE: this will match ALL of the details specified
		return (iter.value().srcDetail & detail) == detail;
	}
	return false;
}

bool GLTextureManager::isTextureState(QString key, unsigned int state ) const
{
	if (_suspendLoading)
		return UnknownState;

	TexturesList::const_iterator iter = _textures.find(key);
	// assert(iter != _textures.end());

	if (iter != _textures.end())
	{
		// NOTE: this will match ANY of the states specified
		return (iter.value().state & state) > 0;
	}
	return UnknownState;
}
#ifdef DXRENDER
IDirect3DTexture9 * GLTextureManager::getGLTextureId(const QString & key ) const
#else
unsigned int GLTextureManager::getGLTextureId(const QString & key ) const
#endif
{
	if (_suspendLoading)
		return _defaultGLTextureId;

	if (isTextureState(key, TextureLoaded))
	{
		TexturesList::const_iterator iter = _textures.find(key);
		const GLTextureObject& obj = iter.value();
		assert(obj.resultTextureData);

		if (obj.resultCode == NoError)
		{
			assert(obj.resultTextureData->glTextureId);
			return obj.resultTextureData->glTextureId;
		}
	}
	return _defaultGLTextureId;
}

bool GLTextureManager::getTextureAlpha(QString key) const
{
	if (_suspendLoading)
		return false;

	if (isTextureState(key, TextureLoaded))
	{
		TexturesList::const_iterator iter = _textures.find(key);
		const GLTextureObject& obj = iter.value();
		assert(obj.resultTextureData);

		if (obj.resultCode == NoError)
		{
			assert(obj.resultTextureData->glTextureId);
			return obj.resultTextureData->alpha;
		}
	}
	// NOTE: assume false otherwise
	return false;
}

Vec3 GLTextureManager::getTextureDims( QString key ) const
{
	if (_suspendLoading)
		return Vec3(1.0f);

	if (isTextureState(key, TextureLoaded))
	{
		TexturesList::const_iterator iter = _textures.find(key);
		const GLTextureObject& obj = iter.value();
		assert(obj.resultTextureData);

		if (obj.resultCode == NoError)
		{
			return obj.resultTextureData->dimensions;
		}
	}
	// return an un-used Vec3
	return Vec3(0.0f);
}

Vec3 GLTextureManager::getTexturePow2Dims( QString key ) const
{
	if (_suspendLoading)
		return Vec3(1.0f);

	if (isTextureState(key, TextureLoaded))
	{
		TexturesList::const_iterator iter = _textures.find(key);
		const GLTextureObject& obj = iter.value();
		assert(obj.resultTextureData);

		if (obj.resultCode == NoError)
		{
			return obj.resultTextureData->pow2Dimensions;
		}
	}
	// return an un-used Vec3
	return Vec3(0.0f);
}

QString GLTextureManager::getTexturePathFromId( QString key ) const
{
	if (_suspendLoading)
		return "";

	if (isTextureState(key, TextureLoaded))
	{
		TexturesList::const_iterator iter = _textures.find(key);
		const GLTextureObject& obj = iter.value();
		assert(!obj.srcPath.isEmpty());

		if (obj.resultCode == NoError)
		{
			return obj.srcPath;
		}
	}

	return "";
}