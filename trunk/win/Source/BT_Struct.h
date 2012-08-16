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

#ifndef _BT_STRUCTS_
#define _BT_STRUCTS_

#include "BT_Actor.h"
#include "BT_NxActorWrapper.h"
#include "BT_FileSystemActor.h"
#include "BT_BumpObject.h"

class FileSystemPile;
class Pile;
class MenuAction;
class AnimationEntry;

Actor* GetBumpActor(const NxActorWrapper* a);

enum CoordMovement
{
	MovedUp		= 1,
	MovedDown	= 2,
	MovedLeft	= 4,
	MovedRight	= 8
};

struct TaggedList
{
	QString TagName;			// name of the Tag. ex: "Dogs"
	vector <NxActorWrapper *> TagList;	// List of the items that are tagegd with this tag.
};

struct MouseCoordsPerActor
{
	Vec3 origPos;
	Vec3 coord;
	Vec3 textSize;
	NxActorWrapper *actor;
	Pile *pile;
	QString text;
	int actionTaken;
	bool ignoreCollision;

	void setMouseCoords(Vec3 loc, Vec3 size, QString t, NxActorWrapper *a = NULL, Pile *p = NULL)
	{
		origPos = loc;
		coord = loc;
		textSize = size;
		actor = a;
		text = t;
		actionTaken = 0;
		ignoreCollision = false;
		pile = p;
	}	
};

struct less_x_value : public std::binary_function<MouseCoordsPerActor, MouseCoordsPerActor, bool>
{
	bool operator()(MouseCoordsPerActor x, MouseCoordsPerActor y)
	{  return x.coord.x < y.coord.x; }
};

struct less_x_position : public std::binary_function<NxActorWrapper*, NxActorWrapper*, bool>
{
	bool operator()(NxActorWrapper* x, NxActorWrapper* y)
	{  return x->getGlobalPosition().x < y->getGlobalPosition().x; }
};
struct less_y_position : public std::binary_function<NxActorWrapper*, NxActorWrapper*, bool>
{
	bool operator()(NxActorWrapper* x, NxActorWrapper* y)
	{  return x->getGlobalPosition().y < y->getGlobalPosition().y; }
};
struct less_z_position : public std::binary_function<NxActorWrapper*, NxActorWrapper*, bool>
{
	bool operator()(NxActorWrapper* x, NxActorWrapper* y)
	{  return x->getGlobalPosition().z < y->getGlobalPosition().z; }
};

struct more_x_position : public std::binary_function<NxActorWrapper*, NxActorWrapper*, bool>
{
	bool operator()(NxActorWrapper* x, NxActorWrapper* y)
	{  
		if (x->getGlobalPosition().x - y->getGlobalPosition().x > 0.05f)
			return x->getGlobalPosition().x > y->getGlobalPosition().x; 
		return false;
	}
};
struct more_y_position : public std::binary_function<NxActorWrapper*, NxActorWrapper*, bool>
{
	bool operator()(NxActorWrapper* x, NxActorWrapper* y)
	{  
		if (x->getGlobalPosition().y - y->getGlobalPosition().y > 0.05f)
			return x->getGlobalPosition().y > y->getGlobalPosition().y; 
		return false;
	}
};
struct more_z_position : public std::binary_function<NxActorWrapper*, NxActorWrapper*, bool>
{
	bool operator()(NxActorWrapper* x, NxActorWrapper* y)
	{  
		if (x->getGlobalPosition().z - y->getGlobalPosition().z > 0.05f)
			return x->getGlobalPosition().z > y->getGlobalPosition().z; 
		return false;
	}
};

struct top_right_on_top : public std::binary_function<Vec3, Vec3, bool>
{
	bool operator()(Vec3 x, Vec3 y)
	{
		//return x->getCMassGlobalPosition().y < y->getCMassGlobalPosition().y;
		if(x.y == y.y)
		{
			if(x.x == y.x)
			{
				return(x.z > y.z);
			}
			else return(x.x > y.x);
		}
		else return(x.y < y.y);
	}
};

struct actors_top_right_on_top : public std::binary_function<NxActorWrapper *, NxActorWrapper *, bool>
{
	bool operator()(NxActorWrapper * x, NxActorWrapper * y)
	{
		//return x->getCMassGlobalPosition().y < y->getCMassGlobalPosition().y;
		return(x->getGlobalPosition().x > y->getGlobalPosition().x);
	}
};

struct sortIntegers : public std::binary_function<int, int, bool>
{
	bool operator()(int x, int y)
	{
		return (x < y);
	}
};

struct SortByActorName : public std::binary_function<BumpObject *, BumpObject *, bool>
{
	inline bool operator()(BumpObject *x, BumpObject *y)
	{
		QString f1 = ((FileSystemActor *) x)->getFullPath().toLower();
		QString f2 = ((FileSystemActor *) y)->getFullPath().toLower();

		return (f1 < f2);
	}
};

#endif