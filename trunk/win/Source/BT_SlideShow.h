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

#ifndef _BT_SLIDE_SHOW_
#define _BT_SLIDE_SHOW_

#ifdef ENABLE_SLIDESHOW_CLASS

// -----------------------------------------------------------------------------

#include "BT_Singleton.h"

// -----------------------------------------------------------------------------

class SlideShow
{
	vector<uint> slideTextures;
	int currSlide;
	int queuedSlide;

	// Alpha Channel
	float globalAlpha;
	float globalAlphaInc;
	float queuedSlideAlpha;
	float queuedSlideAlphaInc;
	bool slideShowEnabled;
	bool disableOnFadeOut;

	// Singleton
	friend class Singleton<SlideShow>;
	SlideShow();

public:


	// Actions
	void update(uint elapsedTime);
	void render();
	void nextSlide();
	void prevSlide();
	void restart(int slideNum = 0);
	void fadeIn();
	void fadeOut();
	void disable();

	// Getters
	bool isEnabled();

	// Setters
	void setSlides(vector<uint> &slideTextureNames);

};

// -----------------------------------------------------------------------------

#define slideShow Singleton<SlideShow>::getInstance()

// -----------------------------------------------------------------------------

#endif

#else
	class SlideShow;
#endif