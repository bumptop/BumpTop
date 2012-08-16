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

#ifndef _ANIMATION_ENTRY_
#define _ANIMATION_ENTRY_

// -----------------------------------------------------------------------------

#include "BT_Stopwatch.h"

// -----------------------------------------------------------------------------

class Animatable;
class AnimationEntry;

// -----------------------------------------------------------------------------

typedef void (* FinishedCallBack)(AnimationEntry &);

// -----------------------------------------------------------------------------

// The animation data holders
class AnimationEntry
{
	FinishedCallBack funcCallBack;
	Animatable *obj;
	void *custom;

	// animation noise
	float delay;
	StopwatchInSeconds delayTimer;

	bool ignoreBattery;
	bool queued;
private:
	void generateDelay();

public:
	inline AnimationEntry(const AnimationEntry &newAnimEntry);
	AnimationEntry(Animatable *animObj = NULL, FinishedCallBack func = NULL, void *customData = NULL, bool addAnimationDelay=false);

	// Overloaded Operators
	inline void operator=(const AnimationEntry &animEntry);

	// Getters
	inline FinishedCallBack getCallback() const;
	inline Animatable *getObject() const;
	inline void *getCustomData() const;

	// animation noise
	void startDelay();
	bool delayComplete();

	void setCallback(FinishedCallBack func, void* customData = 0);
	
	bool getIgnoreBattery();
	void setIgnoreBattery(bool newVal);

	void setQueued (bool val);
	bool isQueued ();
};

// -----------------------------------------------------------------------------

#include "BT_AnimationEntry.inl"

// -----------------------------------------------------------------------------

#else
	class AnimationEntry;
#endif
