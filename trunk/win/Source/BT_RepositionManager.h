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

#ifndef _REPOSITION_MANAGER_
#define _REPOSITION_MANAGER_

// -----------------------------------------------------------------------------

#include "BT_Common.h"
#include "BT_Singleton.h"
#include "BT_BumpObject.h"
#include "BT_Pile.h"

// -----------------------------------------------------------------------------

class RepositionManager
{
	Bounds prevSceneBounds;
	vector<BumpObject *> objsInSpace;
	NxActor *dynamicFloor;

	// Private Actions
	void destroyDynamicFloor();

	// Singleton
	friend class Singleton<RepositionManager>;
	RepositionManager();
	
	float boxPadding; //Padding (halfDims) to use when determining whether object is inside/outside walls.  To allow for numerical instability

public:

	~RepositionManager();

	// Actions	
	void createDynamicFloor();
	void update();
	void addToPileSpace(BumpObject *obj);
	void removeFromPileSpace(BumpObject *obj);
	bool isInPileSpace(BumpObject * obj);
	void adjustToDesktopBox(BumpObject *obj);

	// Getters
	float getPlaneFloorLevel();
};

// -----------------------------------------------------------------------------

#define repoManager Singleton<RepositionManager>::getInstance()

// -----------------------------------------------------------------------------

#else
	class RepositionManager;
#endif