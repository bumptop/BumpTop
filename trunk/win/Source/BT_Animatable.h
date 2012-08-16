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

#ifndef _BT_ANIMATABLE_
#define _BT_ANIMATABLE_

// -----------------------------------------------------------------------------

// The state at which an animation is at
enum AnimationState
{
	NoAnim			= 1,
	AnimStarted		= 2,
	AnimPaused		= 3,
};

// -----------------------------------------------------------------------------

// The state at which an animation is at
enum AnimationType
{
	SizeAnim		= (1 << 0),
	PoseAnim		= (1 << 1),
	AlphaAnim		= (1 << 2),
	FreshnessAnim	= (1 << 3)
};

// -----------------------------------------------------------------------------

class Animatable
{

	AnimationState animState;

public:

	Animatable();
	virtual ~Animatable();

	// Animation Actions
	virtual void onAnimTick() = 0;
	virtual void onAnimFinished() {};
	inline void pauseAnimation();
	inline void resumeAnimation();
	virtual void killAnimation() = 0;
	virtual void finishAnimation() = 0;

	// Setters
	virtual inline void setAnimationState(AnimationState newAnimState);

	// Getters
	virtual bool isAnimating(uint animType = SizeAnim | AlphaAnim | PoseAnim | FreshnessAnim) = 0;
	inline AnimationState getAnimationState();
};

// -----------------------------------------------------------------------------

#include "BT_Animatable.inl"

// -----------------------------------------------------------------------------

#else
	class Animatable;
	enum AnimationState;
#endif