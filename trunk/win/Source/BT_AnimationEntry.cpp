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
#include "BT_AnimationEntry.h"
#include "BT_Pile.h"

// Animation noise
#define MAX_ANIMATION_DELAY 0.075f

AnimationEntry::AnimationEntry(Animatable *animObj, FinishedCallBack func, void *customData, bool addAnimationDelay)
{
	// Default Constructor
	obj = animObj;
	custom = customData;
	funcCallBack = func;
	ignoreBattery = false;
	queued = false;
	if (addAnimationDelay)
	{
		if (!dynamic_cast<Pile *>(animObj))
			generateDelay();
	}
}

void AnimationEntry::generateDelay()
{
	delay = ((rand() % 1000) / 1000.0f) * MAX_ANIMATION_DELAY;
	delayTimer.restart();
}

void AnimationEntry::startDelay()
{
	delayTimer.restart();
}

bool AnimationEntry::delayComplete()
{
	// NOTE: This stops PSP bouncing, undo when relative animations are incorporated.
	return true;
	return (delayTimer.elapsed() > delay);
}

void AnimationEntry::setCallback(FinishedCallBack func, void* customData)
{
	funcCallBack = func;
	custom = customData;
}

void AnimationEntry::setIgnoreBattery(bool newVal)
{
	ignoreBattery = newVal;
}

bool AnimationEntry::getIgnoreBattery()
{
	return ignoreBattery;
}

void AnimationEntry::setQueued( bool val )
{
	queued = val;
}

bool AnimationEntry::isQueued()
{
	return queued;
}