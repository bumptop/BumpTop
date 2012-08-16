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
#include "BT_Util.h"
#include "BT_AnimatedTextureSource.h"
#include "BT_RenderManager.h"
#include "BT_GLTextureManager.h"


AnimatedTextureFrame::AnimatedTextureFrame( int w, int h, int dur, int bitsperpixel, int bgGreyCol)
{
	width = w;
	height = h;
	bpp = bitsperpixel;
	pixelData = new unsigned char[width * height * bpp];
	bgData = new unsigned char[width * height * bpp];
	duration = dur;

	// assume bgra
	for (int i = 0; i < (width * height); ++i)
	{
		bgData[i] = bgData[i+1] = bgData[i+2] = 255;
		bgData[i+3] = 255;
	}
}

AnimatedTextureFrame::~AnimatedTextureFrame()
{
	SAFE_DELETE(pixelData);
}

AnimatedTextureSource::AnimatedTextureSource( )
{
	_isLoaded = false;
	_isPlaying = true;
	_animatedTextureId = 0;
	_currentAnimatedTextureIndex = 0;
}

AnimatedTextureFrame* AnimatedTextureSource::getCurrentTextureFrame()
{
	return _animatedTextureFrames[_currentAnimatedTextureIndex];
}

void AnimatedTextureSource::onUpdate()
{
	if (_animatedTextureFrames.size() <= 1)
		return;
	if (!_isPlaying)
		return;

	AnimatedTextureFrame * frame = _animatedTextureFrames[_currentAnimatedTextureIndex];
	if (_prevAnimatedTextureTimer.elapsed() > (frame->duration))
	{
		_currentAnimatedTextureIndex = (_currentAnimatedTextureIndex+1) % _animatedTextureFrames.size();
		_prevAnimatedTextureTimer.restart();
		rndrManager->invalidateRenderer();
	}
}

void AnimatedTextureSource::play()
{
	_isPlaying = true;
	_prevAnimatedTextureTimer.unpause();
}

void AnimatedTextureSource::pause()
{
	_isPlaying = false;
	_prevAnimatedTextureTimer.pause();
}

bool AnimatedTextureSource::load()
{
#ifdef DXRENDER
	return false;
#else
	if (!_isLoaded && !texMgr->hasTexturesOfState(ImageQueued))
	{
		texMgr->lockIlMutex();

		_animatedTextureId = ilGenImage();
		ilBindImage(_animatedTextureId);
		if (!ilLoad(IL_TYPE_UNKNOWN, (const ILstring) _texturePath.utf16()))
			return false;

		int frameCount = ilGetInteger(IL_NUM_IMAGES) + 1;
		int width = ilGetInteger(IL_IMAGE_WIDTH);
		int height = ilGetInteger(IL_IMAGE_HEIGHT); 
		int duration = 0;

		// skip gifs that are greater than ~10 megs uncompressed for now
		if (width * height * 4 * frameCount > (10 * 1024 * 1024))
		{
			ilDeleteImage(_animatedTextureId);
			return false;
		}

		for (int frame = 0; frame != frameCount; ++frame)
		{		
			ilBindImage(_animatedTextureId);
			if (ilActiveImage(frame))
			{
				duration = ilGetInteger(IL_IMAGE_DURATION); 

				AnimatedTextureFrame * atf = new AnimatedTextureFrame(width, height, duration, 4);
				_animatedTextureFrames.push_back(atf);

				// Scale the image
				// TODO:  Currently we're scaling up 1025x1025 images to 2048x2048.  Instead do "Nearest Square Image Loading".  See case 171 in FogBugz. 					
				ilConvertImage(IL_BGRA, IL_UNSIGNED_BYTE);
				ilCopyPixels(0, 0, 0, width, height, 1, IL_BGRA, IL_UNSIGNED_BYTE, atf->pixelData);
			}
			else
			{
				consoleWrite(QString("Could not load frame (%1) from file (%2)\n").arg(frame).arg(_texturePath));
			}
		}
		_animatedTextureGLId = ilutGLBindTexImage();
		ilDeleteImage(_animatedTextureId);

		texMgr->unlockIlMutex();

		_isLoaded = true;

		_prevAnimatedTextureTimer.restart();
		return true;
	}
	else if (_isLoaded)
	{
		return true;
	}
	return true;
#endif
}

bool AnimatedTextureSource::isPlaying() const
{
	return _isPlaying;
}

AnimatedTextureSource::~AnimatedTextureSource()
{
	if (!_animatedTextureFrames.empty())
	{
		for (int i = 0; i < _animatedTextureFrames.size(); ++i)
			SAFE_DELETE(_animatedTextureFrames[i]);

#ifdef DXRENDER
#else
		// render the current index
		glDeleteTextures(1, &_animatedTextureGLId);
#endif
	}
}

int AnimatedTextureSource::numFrames()
{
	return _animatedTextureFrames.size();
}

unsigned int AnimatedTextureSource::GLId()
{
	return _animatedTextureGLId;
}

void AnimatedTextureSource::setPath( QString texturePath )
{
	_texturePath = texturePath;
}

bool AnimatedTextureSource::isLoaded() const
{
	return _isLoaded;
}