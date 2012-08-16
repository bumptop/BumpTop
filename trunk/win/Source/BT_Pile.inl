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

#include "BT_FileSystemActor.h"

PileType Pile::getPileType()
{
	// Return the Secondary Mask of the Object Type (See struct ObjectType)
	return (PileType) type.secondaryType;
}

void Pile::setPileType(PileType pType)
{
	type.secondaryType = pType;
	
}

PileState Pile::getPileState()
{
	return (PileState) type.ternaryType;
}

bool Pile::isFanningOut()
{
	return fanningOut;
}

bool Pile::isPilable(uint pileType)
{
	return true;
}

bool Pile::isAnimating(uint animType)
{
	// Check if the pile itself is animating
	if (BumpObject::isAnimating(animType))
	{
		return true;
	}

	// Check if any of the items in this pile are animating
	for (uint i = 0; i < pileItems.size(); i++)
	{
		if (pileItems[i]->isAnimating(animType))
		{
			return true;
		}
	}

	return false;
}

const QList<Vec3>& Pile::getFanoutLasso() const
{
	return fanoutPts;
}

uint Pile::getNumItems()
{
	return pileItems.size();
}

void Pile::sortByType()
{
	sort(pileItems.begin(), pileItems.end(), SortByTextureNum());
	animateItemsToRelativePos();
}

void Pile::sortBySize()
{
	sort(pileItems.begin(), pileItems.end(), SortByActorSize());
	animateItemsToRelativePos();
}

BumpObject *Pile::getLastItem()
{
	if (!pileItems.empty())
	{
		return pileItems.back();
	}

	return NULL;
}

BumpObject *Pile::getFirstItem()
{
	if (!pileItems.empty())
	{
		return pileItems.front();
	}

	return NULL;
}

Vec3 Pile::getPosBeforeLayout()
{
	return phOldCent;
}

BumpObject * Pile::getActiveLeafItem()
{
	if (getPileState() == Leaf) {
		if (leafIndex >= 0 && leafIndex < pileItems.size())
			return pileItems[leafIndex];
	}
	return NULL;
}

int Pile::getLeafIndex()
{
	return leafIndex;
}

bool SortByTextureNum::operator()( BumpObject *x, BumpObject *y )
{
	if (x->isBumpObjectType(BumpActor) && y->isBumpObjectType(BumpActor))
	{
		Actor *a = (Actor *) x, *b = (Actor *) y;
		return a->getTextureNum() < b->getTextureNum();
	}

	return false;
}

bool SortByActorSize::operator()( BumpObject *x, BumpObject *y )
{
	return x->getDims().magnitudeSquared() < y->getDims().magnitudeSquared();
}

bool SortByActorHeight::operator()( BumpObject *x, BumpObject *y )
{
	return x->getGlobalPosition().y < y->getGlobalPosition().y;
}