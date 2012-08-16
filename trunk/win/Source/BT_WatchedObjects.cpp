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
#include "BT_BumpObject.h"
#include "BT_WatchedObjects.h"
#include "BT_Camera.h"

WatchedObjects::WatchedObjects(const vector<BumpObject *>& actrs)
: trackActors(true)
, actors(actrs)
, highlightedActor(NULL)
{
	bounds.setEmpty();
}

void WatchedObjects::fadeInAll()
{
	// fade in all actors
	for (int i = 0; i < actors.size(); ++i)
	{
		BumpObject *obj = actors[i];

		if (obj != highlightedActor)
		{
			obj->fadeIn();
		}
	}
}

void WatchedObjects::fadeOutAllButHighlighted(int timeStep)
{
	// fade out all other actors
	for (int i = 0; i < actors.size(); ++i)
	{
		BumpObject * obj = actors[i];

		if (obj != highlightedActor)
			obj->setAlphaAnim(obj->getAlpha(), 0.25f, timeStep);
		else
			obj->fadeIn();
	}
}

void WatchedObjects::restoreCamera()
{
	cam->animateTo(camEye, camDir, camUp);
}

void WatchedObjects::storeCamera()
{
	camEye = cam->getEye();
	camDir = cam->getDir();
	camUp = cam->getUp();
}
