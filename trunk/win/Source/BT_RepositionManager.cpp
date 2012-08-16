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
#include "BT_Pile.h"
#include "BT_RenderManager.h"
#include "BT_RepositionManager.h"
#include "BT_SceneManager.h"
#include "BT_Selection.h"
#include "BT_Struct.h"
#include "BT_Util.h"
#include "BT_WindowsOS.h"

RepositionManager::RepositionManager()
: dynamicFloor(NULL)
{
	prevSceneBounds.setEmpty();
	boxPadding=4.0f; //default padding
}

RepositionManager::~RepositionManager()
{
	destroyDynamicFloor();
}

void RepositionManager::addToPileSpace(BumpObject *obj)
{
	if (!obj) return;

	// Ensure that the pile is not already on the space
	vector<BumpObject *>::const_iterator iter = find(objsInSpace.begin(), objsInSpace.end(), obj);
	if (iter != objsInSpace.end()) return;

	// Move the pile to the Pile Space collision group
	objsInSpace.push_back(obj);
	obj->getShapes()[0]->setGroup(PILE_SPACE_GROUP_NUMBER);
	if (dynamicFloor)
		GLOBAL(gScene)->setActorPairFlags(*dynamicFloor, *(obj->getActor()), ~NX_IGNORE_PAIR); 
}

void RepositionManager::removeFromPileSpace(BumpObject *obj)
{
	if (!obj) return;

	// Find and remove the pile from the pile space container and restore it's
	// collision group with the normal dynamic objects (and not the dynamic 
	// floor)
	vector<BumpObject *>::iterator iter = find(objsInSpace.begin(), objsInSpace.end(), obj);
	
	if (iter != objsInSpace.end())
	{
		BumpObject *p = *iter;
		p->getShapes()[0]->setGroup(DYNAMIC_GROUP_NUMBER);
		objsInSpace.erase(iter);
		if (dynamicFloor)
			GLOBAL(gScene)->setActorPairFlags(*dynamicFloor, *(obj->getActor()), NX_IGNORE_PAIR); 
	}
}

bool RepositionManager::isInPileSpace(BumpObject * obj)
{
	return find(objsInSpace.begin(), objsInSpace.end(), obj) != objsInSpace.end();
}

void RepositionManager::update()
{
	if (!dynamicFloor)
		return;

	// we hit this function a lot, so to save on the calls, we will locally use the scenemanager
	SceneManager * sceneManager = scnManager;
	if (sceneManager->isInSharingMode)
		return;

	const vector<BumpObject *>& objs = sceneManager->getBumpObjects();
	unsigned int size = objs.size();
	BumpObject *obj;
	Actor *actor;
	Pile *pile;
	Vec3 extents;
	Box paddedBox = GetDesktopBox(boxPadding);
	Box unpaddedBox = GetDesktopBox(0.0f);
	Box objBox;
	Vec3 points[8];
	bool repoActive = sceneManager->settings.repositionIconsIfOutsideWalls && 
					  sceneManager->DrawWalls && 
					  !sceneManager->isInInfiniteDesktopMode && 
					  !sceneManager->Walls.empty();

	// Loop through all the actors and push them into the cage if necessary
	for (uint i = 0; i < size; i++)
	{
		obj = objs[i];
		pile = obj->isBumpObjectType(BumpPile) ? (Pile *) obj : NULL;
		actor = obj->isBumpObjectType(BumpActor) ? (Actor *) obj : NULL;
		objBox = obj->getBox();
		if (pile)
			pile->getPileBounds(true).getExtents(extents);
		else
			obj->getBoundingBox().getExtents(extents);

		// Ignore the following cases
		if (!pile && !actor) continue;
		if (obj->isAnimating()) continue;
		if (obj->isPinned()) continue;
		if (actor && actor->isActorType(Temporary)) continue;
		if (actor && actor->isActorType(Invisible)) continue;
		if (actor && actor->isParentType(BumpPile)) continue;
		if ((extents.x > paddedBox.extents.x) || (extents.z > paddedBox.extents.z)) continue;

		// ignore the object if it's on one of the other shared desktops
		bool isSharedDesktopObject = false;
		for (int j = 1 /* not the main desktop */; j < GLOBAL(sharedDesktops).size(); ++j)
		{
			SharedDesktop * desktop = GLOBAL(sharedDesktops)[j];
			if (find(desktop->objects.begin(), desktop->objects.end(), obj) != desktop->objects.end())
			{
				isSharedDesktopObject = true;
				break;
			}
		}
		if (isSharedDesktopObject)
			continue;

		if (repoActive)
		{
			// If the icon is completely outside, pop it back in
			if (!NxBoxBoxIntersect(unpaddedBox, objBox, true))
			{
				adjustToDesktopBox(obj);
				continue;
			}

			// Adjust the icon to be inside the desktop box
			obj->getBox().computePoints(points);
			for (uint i = 0; i < 8; i++)
			{
				if (!paddedBox.containsPoint(points[i]))
				{
					adjustToDesktopBox(obj);
					break;
				}
			}
		}
	}
}

void RepositionManager::adjustToDesktopBox( BumpObject *obj )
{
	// Adjust bound by the desktop box
	const float wallWidth = getDimensions(GLOBAL(Walls)[0]).z;
	const float wallHeight = getDimensions(GLOBAL(Walls)[0]).y;
	Vec3 dim;
	Bounds bounds = obj->getBoundingBox();
	Vec3 pos = obj->getGlobalPosition();		
	
	bounds.getExtents(dim);
	dim += Vec3(boxPadding,boxPadding,boxPadding);

	// Find out the closest place to animate to
	pos.x = NxMath::max(NxMath::min(pos.x, GLOBAL(WallsPos)[3].x - wallWidth - dim.x), GLOBAL(WallsPos)[2].x + wallWidth + dim.x);
	pos.y = NxMath::max(NxMath::min(pos.y, GLOBAL(WallsPos)[3].y + wallHeight - dim.y), GLOBAL(WallsPos)[2].y - wallHeight + dim.y); 
	pos.z = NxMath::max(NxMath::min(pos.z, GLOBAL(WallsPos)[0].z - wallWidth - dim.z), GLOBAL(WallsPos)[1].z + wallWidth + dim.z);

	pos.y = NxMath::max(dim.y, pos.y); // Position above floor

	//If it has some velocity reverse the velocity to make it look like it bumped against the wall/roof
	//Otherwise just animate it back in to place
	if (obj->isMoving())
	{
		//Reverse its velocity to make it look like it bumped against the wall/roof
		Vec3 oldVelNorm = obj->getLinearVelocity();
		oldVelNorm.normalize();
		obj->stopAllMotion();
		obj->setLinearVelocity(Vec3(-oldVelNorm.x, 0, -oldVelNorm.z));

		obj->setGlobalPosition(pos);
	}
	else
	{	
		//animate it back in
		obj->setGlobalPosition(pos);
	}

	// Trigger a render
	rndrManager->invalidateRenderer();
}

void RepositionManager::createDynamicFloor()
{
	if (dynamicFloor) return;

	// Create ground plane
	NxPlaneShapeDesc planeDesc;
	NxActorDesc actorDesc;

	// Create the floor
	actorDesc.shapes.pushBack(&planeDesc);
	dynamicFloor = GLOBAL(gScene)->createActor(actorDesc);
	dynamicFloor->getShapes()[0]->setGroup(PILE_SPACE_FLOOR_NUMBER);
	dynamicFloor->getShapes()[0]->isPlane()->setPlane(Vec3(0, 1, 0), getPlaneFloorLevel());
}

void RepositionManager::destroyDynamicFloor()
{
	if (!dynamicFloor) return;

	NxShape **shapes;
	NxU32 nShapes;

	// Get at the shape of this item
	shapes = dynamicFloor->getShapes();
	nShapes = dynamicFloor->getNbShapes();

	// Prune the inactive list to help Novodex process
	while (nShapes--)
	{
		dynamicFloor->releaseShape(*shapes[nShapes]);
		shapes[nShapes] = NULL;
	}

	// Release the actor
	GLOBAL(gScene)->releaseActor(*dynamicFloor);
	dynamicFloor = NULL;
}

float RepositionManager::getPlaneFloorLevel()
{
	// NOTE: This is a hard cap value. If you want dynamically set PSP, roll back to revision 1094
	return 35.0f;
}