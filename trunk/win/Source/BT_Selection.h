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

#ifndef _BT_SELECTION_
#define _BT_SELECTION_

// -----------------------------------------------------------------------------

#include "BT_Singleton.h"

class NxActorWrapper;
class BumpObject;
class Pile;

// -----------------------------------------------------------------------------

enum SelectionType
{
	Nothing					= (1 << 0),

	SingleFreeItem			= (1 << 1),
	MultipleFreeItems		= (1 << 2),
	AnyFreeItems			= SingleFreeItem + MultipleFreeItems,

	SingleFullPile			= (1 << 3),
	MultipleFullPile		= (1 << 4),
	AnyFullPile				= SingleFullPile + MultipleFullPile,

	AnyPilesOrFreeItems		= AnyFreeItems + AnyFullPile,

	SinglePileMemberItem	= (1 << 5),
	MultiplePileMemberItems	= (1 << 6),
	AnyPileMemberItems		= SinglePileMemberItem + MultiplePileMemberItems,

	SingleBumpCluster		= (1 << 7),
	MultipleBumpCluster		= (1 << 8),
	AnyBumpCluster			= SingleBumpCluster + MultipleBumpCluster,

	AnySelection			= AnyFreeItems + AnyFullPile + AnyPileMemberItems + AnyBumpCluster,
	UseOnlyThisType			= (1 << 9), // Forces the use of that specific type
	Wall					= (1 << 10)
};

// -----------------------------------------------------------------------------

class Selection
{

	BumpObject * pickedActor;
	BumpObject * _lastPickedActor;
	Vec3 stabPointActorSpace;
	vector<BumpObject *> selectedObjects;
	vector<NxJoint *> joints;

	// Singleton
	friend class Singleton<Selection>;


public:

	Selection();
	~Selection();

	// Actions
	void				update();
	void				clear();
	void				add(BumpObject *obj);
	void				insert(BumpObject *obj, unsigned int index);
	void				replace(const QList<BumpObject *>& objs);
	void				remove(BumpObject *obj);

	// Operations
	void				operator=(const vector<BumpObject *> &in);
	BumpObject			*operator[](uint indx) const;

	// Getters
	vector<BumpObject *>getBumpObjects();
	vector<BumpObject *>getFreeItems();
	vector<BumpObject *>getPileMembers();
	vector<Pile *>		getFullPiles();
	inline uint			getSize();
	uint				getSelectionType();
	vector<NxActorWrapper *> getLegacyActorList();
	BumpObject			* getPickedActor();
	BumpObject			* getLastPickedActor();
	Vec3				getStabPointActorSpace();
	bool				isInSelection(BumpObject *obj);
	bool				isBumpObjectValid(BumpObject *obj);

	// Setters
	void				setPickedActor(BumpObject *obj);
	void				setStabPointActorSpace(Vec3 stab);
};

// -----------------------------------------------------------------------------

#include "BT_Selection.inl"

// -----------------------------------------------------------------------------

#define sel Singleton<Selection>::getInstance()

// -----------------------------------------------------------------------------

#else
	class Selection;
	enum SelectionType;
#endif