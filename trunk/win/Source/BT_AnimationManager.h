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

#ifndef _BT_ANIMATION_MANAGER_
#define _BT_ANIMATION_MANAGER_

// -----------------------------------------------------------------------------

#include "BT_Singleton.h"
#include "BT_Animatable.h"
#include "BT_AnimationEntry.h"

// -----------------------------------------------------------------------------

class AnimationManager
{
	vector<AnimationEntry> animationList;
	//vector<AnimationEntry> queuedAnimationList;
	QList<AnimationEntry> queuedAnimationList;

	// Private Actions
	void finishedAnimation(AnimationEntry entry);

	friend class Singleton<AnimationManager>;
	AnimationManager();

public:

	~AnimationManager();

	// Actions
	void update();
	bool isAnimating();
	bool isObjAnimating(Animatable *anim);
	void addAnimation(AnimationEntry entry);
	void addQueuedAnimation(AnimationEntry entry);
	int getNumObjsAnimating() { return animationList.size(); }
	vector<Animatable*> getAnimatingObjs();

	void finishAnimation(Animatable * anim);

	// Ability to remove animations while in the middle of them
	void removeAnimation(Animatable *anim);
	void removeAllAnimations();
	
	// Ability to pause animations
	void pauseAnimation(Animatable *anim);
	void pauseAllAnimations();

	// Ability to resume animations
	void resumeAnimation(Animatable *anim);
	void resumeAllAnimations();

};

// -----------------------------------------------------------------------------

#define animManager Singleton<AnimationManager>::getInstance()

// -----------------------------------------------------------------------------

#else
	class AnimationManager;
#endif