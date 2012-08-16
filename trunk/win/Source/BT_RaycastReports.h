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

#ifndef _RAYCAST_REPORTS_
#define _RAYCAST_REPORTS_

// -----------------------------------------------------------------------------

class BumpObject;

// -----------------------------------------------------------------------------

class SimpleRaycast : public NxUserRaycastReport
{
public:

	bool onHit(const NxRaycastHit &hit);
};

// -----------------------------------------------------------------------------

class PickRaycast : public NxUserRaycastReport
{
	vector<NxActor *> objHit;
	vector<NxRaycastHit> hitLoc;
	BumpObject *obj;
	NxRaycastHit hit;

	// Private Actions
	void findTopMostObject();

public:

	PickRaycast();

	// Events
	bool onHit(const NxRaycastHit &hit);

	// Getters
	BumpObject *getObject();
	NxRaycastHit getRaycastHit(Ray worldStab);
};

// -----------------------------------------------------------------------------

class MultiRaycast : public NxUserRaycastReport
{

	vector<BumpObject *> raycastList;

public:

	// Events
	bool onHit(const NxRaycastHit &hit);

	// Getters
	vector<BumpObject *> getLastRaycast();
};

// -----------------------------------------------------------------------------

#else
	class SimpleRaycast;
	class MultiRaycast;
#endif