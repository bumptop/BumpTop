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
#include "BT_AnimationManager.h"
#include "BT_AnimationEntry.h"
#include "BT_RenderManager.h"
#include "BT_TextManager.h"
#include "BT_Animatable.h"
#include "BT_UndoStack.h"
#include "BT_Util.h"
#include "BT_EventManager.h"
#include "BT_Settings.h"
#include "BT_SceneManager.h"

AnimationManager::AnimationManager()
{
}

AnimationManager::~AnimationManager()
{
	// Remove all the animations that are still kicking around
	removeAllAnimations();
}

// This function calls any function that should be called when this animation finishes
void AnimationManager::finishedAnimation(AnimationEntry entry)
{
	FinishedCallBack fnCb = entry.getCallback();
	AnimationEntry tempEntry(entry.getObject(), entry.getCallback(), entry.getCustomData());
	Animatable *obj = entry.getObject();

	// Kill off the entry so there are no dangling pointers
	removeAnimation(entry.getObject());

	if (obj)
	{
		// ALow the object to finish properly
		obj->onAnimFinished();
	}	

	if (fnCb)
	{
		// Finish this animation by calling the callback function specified
		fnCb(tempEntry);
	}

	textManager->invalidate();
	rndrManager->invalidateRenderer();
}

// Increment all Objects in the Manager
void AnimationManager::update()
{
	for (int i = 0; i < animationList.size(); i++)
	{
		AnimationEntry& entry = animationList[i];

		if (!entry.getIgnoreBattery() && (scnManager->skipAnimations || (GLOBAL(settings).disableAnimationsOnBattery && (evtManager->getACPowerStatus() == EventManager::Unplugged))))
		{
			finishAnimation(entry.getObject());
			i--;
			continue;
		}

		// If this entry is Started, increment its animation
		if (entry.getObject()->getAnimationState() == AnimStarted && !entry.isQueued())
		{
			// Increment Animation
			if (entry.delayComplete())
			{
				entry.getObject()->onAnimTick();

				if (!entry.getObject()->isAnimating())
				{
					// No animation is left, finish the animation
					finishedAnimation(entry);
					i--;
				}
			}
		}
	}
}

// This function adds an animation to the list
void AnimationManager::addAnimation(AnimationEntry entry)
{
	if (entry.getObject())
	{
		// Iterate over the animation list to see if this item already exists
		for (uint i = 0; i < animationList.size(); i++)
		{	
			if (entry.getObject() == animationList[i].getObject())
			{
				// The item exists in the list, so check if the callback function can be replaced by the new entry
				// Only overwrite a callback if it was NULL and the new one is not NULL
				if (entry.getCallback() != NULL && animationList[i].getCallback() == NULL)
					animationList[i].setCallback(entry.getCallback(), entry.getCustomData());
				animationList[i].getObject()->setAnimationState(AnimStarted);	
				return;
			}
		}

		// This animation is not in the list so add it
		entry.startDelay();
		animationList.push_back(entry);
		entry.getObject()->setAnimationState(AnimStarted);
	}
}
void AnimationManager::addQueuedAnimation(AnimationEntry entry)
{
	if (entry.getObject())
	{
		entry.setQueued(true);
		queuedAnimationList.append(entry);
	}
}

// This function removes a specific animation
void AnimationManager::removeAnimation(Animatable *anim)
{
	AnimationEntry animEntry;
	bool deleteSuccessful = false;
	// Loop through the animation list and remove the animation
	for (int i = 0; i < animationList.size(); i++)
	{
		animEntry = animationList[i];

		if (animEntry.getObject() == anim)
		{
			// Remove this animation from the list
			animationList.erase(animationList.begin() + i);
			animEntry.getObject()->setAnimationState(NoAnim);
			i--;
			deleteSuccessful = true;
		}
	}

	if (deleteSuccessful)
	{
		for (int i = 0;i<queuedAnimationList.size();i++)
		{
			if (queuedAnimationList[i].getObject() == anim)
			{
				// Mark the animationEntry as not queued and started
				queuedAnimationList[i].setQueued(false);
				queuedAnimationList[i].getObject()->setAnimationState(AnimStarted);
				queuedAnimationList[i].startDelay();
				
				// Add the new animation to the animation List
				animationList.push_back(queuedAnimationList[i]);

				// Remove the queued animation
				queuedAnimationList.erase(queuedAnimationList.begin() + i);
				break;
			}
		}
	}
}

// This function removes all animations form the list
void AnimationManager::removeAllAnimations()
{
	AnimationEntry animEntry;

	// Loop through the animation list and remove all animations
	for (int i = 0; i < animationList.size(); i++)
	{
		animEntry = animationList[i];
		finishAnimation(animEntry.getObject());
	}

	animationList.clear();
}

// This function Pauses a specific animation that matches the passed actor
void AnimationManager::pauseAnimation(Animatable *anim)
{
	AnimationEntry animEntry;

	// Loop through the animation list and pause this specific animation
	for (int i = 0; i < animationList.size(); i++)
	{
		animEntry = animationList[i];

		// Pause the animation that matches this actor
		if (animEntry.getObject() == anim)
		{
			animEntry.getObject()->pauseAnimation();
			return;
		}
	}
}

// This function pauses all animations
void AnimationManager::pauseAllAnimations()
{
	AnimationEntry animEntry;

	// Loop through the animation list and pause all animations
	for (int i = 0; i < animationList.size(); i++)
	{
		animEntry = animationList[i];
		animEntry.getObject()->setAnimationState(AnimPaused);
	}
}

void AnimationManager::resumeAnimation(Animatable *anim)
{
	AnimationEntry animEntry;

	// Loop through the animation list and resume this specific animation
	for (int i = 0; i < animationList.size(); i++)
	{
		animEntry = animationList[i];

		// Pause the animation that matches this actor
		if (animEntry.getObject() == anim)
		{
			animEntry.getObject()->setAnimationState(AnimStarted);
			return;
		}
	}
}

// Resume all animations
void AnimationManager::resumeAllAnimations()
{
	// Loop through the animation list and resume all animations
	for (int i = 0; i < animationList.size(); i++)
	{
		animationList[i].getObject()->setAnimationState(AnimStarted);
	}
}

// Are there animations currently in action
bool AnimationManager::isAnimating()
{
	return animationList.size() > 0 ? true : false;
}

bool AnimationManager::isObjAnimating(Animatable *anim)
{
	for (uint i = 0; i < animationList.size(); i++)
	{
		if (animationList[i].getObject() == anim) 
			return true;
	}

	return false;
}

vector<Animatable*> AnimationManager::getAnimatingObjs()
{
	vector<Animatable*> objs;

	for_each(AnimationEntry entry, animationList)
		objs.push_back( entry.getObject() );

	return objs;
}

void AnimationManager::finishAnimation( Animatable * anim )
{
	// Loop through the animation list and remove the animation
	for (int i = 0; i < animationList.size(); i++)
	{
		const AnimationEntry& animEntry = animationList[i];

		if (animEntry.getObject() == anim)
		{
			animEntry.getObject()->finishAnimation();
			finishedAnimation(animEntry);
			i--;
		}
	}
}