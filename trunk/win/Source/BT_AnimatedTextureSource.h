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

#ifndef _BT_ANIMATEDTEXTURESOURCE_
#define _BT_ANIMATEDTEXTURESOURCE_

#include "BT_Stopwatch.h"

// -----------------------------------------------------------------------------

class AnimatedTextureFrame
{
public:
	int width;
	int height;
	int bpp;
	unsigned char * pixelData;
	unsigned char * bgData;
	unsigned long duration;

public:
	AnimatedTextureFrame(int width, int height, int dur, int bpp=4, int bgGreyCol=0);
	~AnimatedTextureFrame();
};

class AnimatedTextureSource
{
	QString _texturePath;
	bool _isLoaded;
	bool _isPlaying;
	unsigned int _animatedTextureId;
	unsigned int _animatedTextureGLId;
	unsigned int _currentAnimatedTextureIndex;

	Stopwatch _prevAnimatedTextureTimer;

	vector<AnimatedTextureFrame *> _animatedTextureFrames;

public:
	AnimatedTextureSource();
	~AnimatedTextureSource();

	void setPath(QString texturePath);

	bool load();
	void onUpdate();
	void play();
	void pause();


	AnimatedTextureFrame* getCurrentTextureFrame();
	bool isPlaying() const;
	bool isLoaded() const;
	int numFrames();
	unsigned int GLId();

};

#endif // _BT_ANIMATEDTEXTURESOURCE_
