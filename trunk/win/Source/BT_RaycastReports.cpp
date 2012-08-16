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
#include "BT_Camera.h"
#include "BT_RaycastReports.h"
#include "BT_SceneManager.h"
#include "BT_FileSystemActor.h"
#include "BT_Actor.h"
#include "BT_Pile.h"
#include "BT_Util.h"
#include "BT_Selection.h"

bool SimpleRaycast::onHit(const NxRaycastHit &hit)
{
	// we hit this function a lot, so to save on the calls, we will locally use the scenemanager
	SceneManager * sceneManager = scnManager;

	// If walls disabled, mouse is not over wall
	if(!sceneManager->DrawWalls)
	{
		sceneManager->MouseOverWall=false;
		return false;
	}

	// adding quick check to ensure that the wall point is above the floor 
	if (hit.impact.y > 0)
	{
		// Record information here
		const vector<NxActorWrapper*>& walls = scnManager->Walls;
		unsigned int size = walls.size();
		for(int i=0; i < size; i++)
		{
			NxActorWrapper * nxActor = walls[i];

			if(nxActor->getShapes()[0] == hit.shape && hit.distance > 0.005f)
			{
				// ignore the walls when we are in top down view
				if (cam->getCurrentCameraView() == Camera::TopDownView)
					continue;

				sceneManager->MouseOverWall = true;
				sceneManager->PinPoint = hit.impact;
				sceneManager->PinWall = nxActor;

				if (sel->getPickedActor() && (sel->getBumpObjects().size() == 1))
				{					
					// downsize any large actors to fit on the wall
					BumpObject * obj = sel->getPickedActor();
					if (obj->getObjectType() == ObjectType(BumpActor))
					{
						Actor * actor = (Actor *) obj;
						Vec3 dims = actor->getDims();
						Vec3 extents = GetDesktopBox().GetExtents() * 0.65f;
						if (dims.y > extents.y)
						{
							float ratio = extents.y / dims.y;
							dims *= ratio;
						}
						actor->setDims(dims);
					}
				}

				return false;
			}
		}
	}

	sceneManager->MouseOverWall = false;
	return true; //or false to stop the raycast
}

// -----------------------------------------------------------------------------

bool MultiRaycast::onHit(const NxRaycastHit &hit)
{
	Actor *data;
	vector<BumpObject *> objList = scnManager->getBumpObjects();
	bool okToAdd = false;

	// Loop through all items on the desktop to see which ones we stab
	for (uint i = 0; i < objList.size(); i++)
	{
		okToAdd = false;

		if (objList[i]->getShapes()[0] == hit.shape && 
			!objList[i]->isSelected())
		{
			Pile *parent = objList[i]->getPileParent();
			if (!(parent && parent->getPileState() == Stack))
			{
				if (objList[i]->isBumpObjectType(BumpActor))
				{
					data = (Actor *) objList[i];
					okToAdd = !data->isActorType(Invisible);
				}
				else if (objList[i]->isBumpObjectType(BumpPile))
				{
					okToAdd = true;
				}
			}
		}

		if (okToAdd)
		{
			raycastList.push_back(objList[i]);
		}
	}

	return true;
}

vector<BumpObject *> MultiRaycast::getLastRaycast()
{
	return raycastList;
}

// -----------------------------------------------------------------------------

PickRaycast::PickRaycast()
{
	obj = NULL;
}

bool PickRaycast::onHit(const NxRaycastHit &hit)
{
	NxShape* pickedShape = hit.shape;

	// Save the hit location and object
	objHit.push_back(&pickedShape->getActor());
	hitLoc.push_back(hit);

	return true;
}

BumpObject *PickRaycast::getObject()
{
	findTopMostObject();

	return obj;
}

NxRaycastHit PickRaycast::getRaycastHit(Ray worldStab)
{
	findTopMostObject();

	if (obj)
		GLOBAL(gScene)->raycastClosestShape(worldStab, NX_DYNAMIC_SHAPES, hit);

	return hit;
}

void PickRaycast::findTopMostObject()
{
	vector<BumpObject *> objList;
	float shortestDist = FLT_MAX;

	// Only act on new cases of ray casting
	if (obj) return;

	objList = scnManager->getBumpObjects();
	for (uint i = 0; i < objList.size(); i++)
	{
		// Specific ignore states for Actors
		if (objList[i]->isBumpObjectType(BumpActor))
		{
			Actor *aData = (Actor *) objList[i];

			if (aData->isActorType(Invisible)) continue;
			if (aData->isActorType(Temporary)) continue;
		}

		// Specific Ignore cases for Piles
		if (objList[i]->isBumpObjectType(BumpPile))
		{
			Pile *pile = (Pile *) objList[i];

			if (pile->getPileState() == LaidOut ||
				pile->getPileState() == LayingOut ||
				pile->getPileState() == NoState) continue;
		}
		
		// Check if we hit any BumpWidgets. They have higher priority, and so
		// are handled a bit differently
		for (int j = objHit.size() - 1; j >= 0; j--)
		{
			if (objList[i]->getActor() == objHit[j] &&
				objList[i]->getObjectType() == ObjectType(BumpWidget))
			{
				Actor * actor = (Actor *) objList[i];
				if (!actor->isActorType(Invisible))
				{
					// If the previous "shortest distance" object is not a BumpWidget
					// then we have to reset the shortestDist variable since we are
					// no longer looking at non BumpWidget objects
					if (obj && obj->getObjectType() != ObjectType(BumpWidget))
					{
						shortestDist = FLT_MAX;
					}
					
					if (hitLoc[j].distance < shortestDist)
					{
						obj = objList[i];
						hit = hitLoc[j];
						shortestDist = hitLoc[j].distance;
					}
				}
			}
		}

		// Skip the main picking code if we have picked BumpWidgets before
		if (obj && obj->getObjectType() == ObjectType(BumpWidget))
			return;
		
		// Actual picking code
		for (int j = objHit.size() - 1; j >= 0; j--)
		{
			// Figure out which objects were hit and where
			if (objList[i]->getActor() == objHit[j] &&
				hitLoc[j].distance < shortestDist)
			{
				// BumpObjects were hit
				obj = objList[i];
				hit = hitLoc[j];

				// Save the threshold for the next object that was hit
				shortestDist = hitLoc[j].distance;
			}
		}
	}
}