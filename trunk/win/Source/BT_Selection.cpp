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
#include "BT_MarkingMenu.h"
#include "BT_MouseEventManager.h"
#include "BT_RenderManager.h"
#include "BT_RepositionManager.h"
#include "BT_Selection.h"
#include "BT_SceneManager.h"
#include "BT_TextManager.h"
#include "BT_Util.h"
#include "BT_WidgetManager.h"
#include "BumpTop.h"

#include "BT_Cluster.h"

Selection::Selection()
{
	pickedActor = NULL;
}

Selection::~Selection()
{
}

void Selection::add(BumpObject *obj)
{
	// If the object cannot be added, just don't bother continuing
	if (!isBumpObjectValid(obj)) return;

	// If it has a parent, see what the state of the parent is in
	if (obj->isParentType(BumpPile))
	{
		Pile *parent = (Pile *) obj->getParent();

		// If the pile is in a stack position, ignore this item
		if (parent->getPileState() == Stack)
		{
			// Add the parent instead
			add(obj->getParent());
			
			return;
		}

	}

	// move selected object to end of vector so that it will be drawn last even when deselected
	rndrManager->moveObjectToIndex(obj, rndrManager->getRenderListSize() - 1);

	selectedObjects.push_back(obj);

	// show the name if this is an actor thumbnail
	if (obj->getObjectType() == ObjectType(BumpActor, FileSystem, Thumbnail))
	{
		FileSystemActor * actor = dynamic_cast<FileSystemActor *>(obj);
		Widget * w = widgetManager->getActiveWidgetForFile(actor->getFullPath());
		bool isWidget = (w != NULL) && w->isWidgetOverrideActor(actor);
		if (!actor->isPinned() || 
			(isWidget || actor->isFileSystemType(PhotoFrame)))
			obj->showText();
	}

	if (this == sel) // only make the object green (selected) if this is the primary touch, which uses the global sel
		obj->onSelect();

	// update the text so that text changes are reflected immediately in the next render
	textManager->invalidate();
}

void Selection::replace(const QList<BumpObject *>& objs)
{	
	clear();

	// disable text relayout when altering selection of a large group of items
	textManager->disableForceUpdates();

	for (int i = 0; i < objs.size(); ++i) 
		add(objs[i]);

	// disable text relayout when selecting a large group of items
	textManager->enableForceUpdates();
}

void Selection::insert(BumpObject *obj, unsigned int index)
{
	// Remove the object from the list if it already exists
	vector<BumpObject *>::iterator iter = find(selectedObjects.begin(), selectedObjects.end(), obj);
	if (iter != selectedObjects.end())
	{
		selectedObjects.erase(iter);
	}

	// Bound the index
	index = NxMath::min(index, selectedObjects.size());

	// move selected object to end of vector so that it will be drawn last even when deselected
	rndrManager->moveObjectToIndex(obj, rndrManager->getRenderListSize() - 1);

	// Insert it into the selection
	selectedObjects.insert(selectedObjects.begin() + index, obj);

	// Show the name if this is an actor thumbnail
	if (obj->getObjectType() == ObjectType(BumpActor, FileSystem, Thumbnail))
	{
		if (!obj->isPinned() || obj->getObjectType() == ObjectType(BumpActor, FileSystem, PhotoFrame))
		{
			obj->showText();
		}
	}

	// update the text so that text changes are reflected immediately in the next render
	textManager->invalidate();
}

void Selection::clear()
{
	// disable text relayout when selecting a large group of items
	textManager->disableForceUpdates();

	// Deselect the Items that were in this selection
	for (uint i = 0; i < selectedObjects.size(); i++)
	{
		// hide the name if this is an actor thumbnail
		if (selectedObjects[i]->getObjectType() == ObjectType(BumpActor, FileSystem, Thumbnail))
			selectedObjects[i]->hideText(true);

		// deselect this object
		selectedObjects[i]->onDeselect();
	}

	// disable text relayout when selecting a large group of items
	textManager->enableForceUpdates();

	// Clear the selection
	selectedObjects.clear();

	// Clear the ignore state of the marking menu
	pickedActor = NULL;

	// update the text so that text changes are reflected immediately in the next render
	textManager->forceUpdate();
}

void Selection::remove(BumpObject *obj)
{
	vector<BumpObject *>::iterator iter = find(selectedObjects.begin(),
		selectedObjects.end(), obj);
	// If this item is in the selection then remove it
	if (iter != selectedObjects.end())
	{
		BumpObject * obj = *iter;
		selectedObjects.erase(iter);

		// hide the name if this is an actor thumbnail
		if (obj->getObjectType() == ObjectType(BumpActor, FileSystem, Thumbnail))
			obj->hideText();

		// deselect it
		obj->onDeselect();
	}

	// update the text so that text changes are reflected immediately in the next render
	textManager->invalidate();
}

vector<BumpObject *> Selection::getBumpObjects()
{
	vector<BumpObject *> objList;

	for (uint i = 0; i < selectedObjects.size(); i++)
	{
		// ensure that the selection is still valid
		if (!scnManager->containsObject(selectedObjects[i]))
			continue;

		if (selectedObjects[i]->isParentType(BumpPile))
		{
			Pile *parent = (Pile *) selectedObjects[i]->getParent();
			// ensure that the selection is still valid
			if (!scnManager->containsObject(parent))
				continue;

			PileState state = parent->getPileState();
			if (state != Stack || (state == Stack && parent->getPileType() == SoftPile))
			{
				objList.push_back(selectedObjects[i]);
			}
		}else{
			objList.push_back(selectedObjects[i]);
		}
	}

	// Return a the entire list
	return objList;
}

BumpObject *Selection::operator[](uint indx) const
{
	// Grab the index requested by the square brackets
	if (indx >= 0 && indx < selectedObjects.size())
	{
		return selectedObjects[indx];
	}else{
		// Bad Index
		return NULL;
	}
}

uint Selection::getSelectionType()
{
	uint selection = Nothing;
	vector<BumpObject *> tempList = selectedObjects;
	vector<Pile *> pileList;

	for (uint i = 0; i < selectedObjects.size(); i++)
	{
		BumpObject * obj = selectedObjects[i];
		// We have a selection, don't mark it as Nothing
		if (selection & Nothing) selection &= ~Nothing;

		if (obj->isBumpObjectType(BumpActor))
		{
			// Check for a Parent, thats how we know if we selected Pile Members
			if (obj->getParent() == NULL || obj->isParentType(BumpCluster))
			{
				// If its another Single Item, Toggle Multiple Items
				if (selection & SingleFreeItem && !(selection & MultipleFreeItems))
				{
					selection |= MultipleFreeItems;
					selection &= ~SingleFreeItem;
				}else{
					// Toggle Single Free Item
					if (!(selection & SingleFreeItem) && !(selection & MultipleFreeItems))
					{
						selection |= SingleFreeItem;
					}
				}
			}
			else if (obj->isParentType(BumpPile)) {
				// Added a single Pile Item
				if (selection & SinglePileMemberItem && !(selection & MultiplePileMemberItems))
				{
					selection |= MultiplePileMemberItems;
					selection &= ~SinglePileMemberItem;
				}else{
					// Toggle Single Pile 
					if (!(selection & SinglePileMemberItem) && !(selection & MultiplePileMemberItems))					
						selection |= SinglePileMemberItem;
					
				}			
			}
			else
				//unknown parent
				assert(false);
		}
		else if (obj->isObjectType(BumpPile))
		{
			// TODO: Piles are still too primitive to have a Pile Actor. Later on a Pile will have a Phantom Actor
			// If its another Single Pile, Toggle Multiple Pile
			if (selection & SingleFullPile && !(selection & MultipleFullPile))
			{
				selection |= MultipleFullPile;
				selection &= ~SingleFullPile;
			}else{
				// Toggle Single Pile 
				if (!(selection & SingleFullPile) && !(selection & MultipleFullPile))
				{
					selection |= SingleFullPile;
				}
			}			
		}
		else if (obj->isObjectType(BumpCluster))
		{

			if (selection & SingleBumpCluster && !(selection & MultipleBumpCluster))
			{
				selection |= MultipleBumpCluster;
				selection &= ~SingleBumpCluster;
			}else{
				// Toggle Single Pile 
				if (!(selection & SingleBumpCluster) && !(selection & MultipleBumpCluster))
				{
					selection |= SingleBumpCluster;
				}
			}		



		}
		
	}

	return selection;
}

vector<NxActorWrapper *> Selection::getLegacyActorList()
{
	vector<NxActorWrapper *> actorList;

	// REFACTOR: this is done to make this class compatible with the old code
	for (uint i = 0; i < selectedObjects.size(); i++)
	{
		if (selectedObjects[i]->isBumpObjectType(BumpPile))
		{
			Pile *pile = (Pile *) selectedObjects[i];

			// If we have a Pile on our hands, give all its members out
			for (uint j = 0; j < pile->getNumItems(); j++)
			{
				actorList.push_back((*pile)[j]);
			}
		}else{
			// Free floating items
			actorList.push_back(selectedObjects[i]);
		}
	}

	return actorList;
}

vector<Pile *> Selection::getFullPiles()
{
	vector<Pile *> pileList;

	for (uint i = 0; i < selectedObjects.size(); i++)
	{
		// Add only Full piles to the list
		if (selectedObjects[i]->isBumpObjectType(BumpPile))
		{
			pileList.push_back((Pile *) selectedObjects[i]);
		}
	}

	return pileList;
}

vector<BumpObject *> Selection::getFreeItems()
{
	vector<BumpObject *> actors;

	for (uint i = 0; i < selectedObjects.size(); i++)
	{
		// If this actor turns out to be a pile
		if (selectedObjects[i]->isBumpObjectType(BumpActor) &&
			!selectedObjects[i]->getParent())
		{
			// Add up the amount of piles in the selection
			actors.push_back(selectedObjects[i]);
		}
	}

	// Return a concatenation of Piled and UnPiled Items
	return actors;
}

bool Selection::isInSelection(BumpObject *obj)
{
	if (!obj)
		return false;

	for (uint i = 0; i < selectedObjects.size(); i++)
	{
		// Check for existing item
		if (selectedObjects[i] == obj)
		{
			return true;
		}
	}

	return false;
}

vector<BumpObject *> Selection::getPileMembers()
{
	vector<BumpObject *> actors;

	for (uint i = 0; i < selectedObjects.size(); i++)
	{
		// If this actor turns out to be a pile
		if (selectedObjects[i]->isBumpObjectType(BumpActor) &&
			selectedObjects[i]->getParent() != NULL)
		{
			// Add up the amount of piles in the selection
			actors.push_back(selectedObjects[i]);
		}
	}

	// Return a concatenation of Piled and UnPiled Items
	return actors;

}

void Selection::operator=(const vector<BumpObject *> &in)
{
	selectedObjects.clear();
	selectedObjects = in;
}

BumpObject* Selection::getPickedActor()
{
	return pickedActor;
}

BumpObject * Selection::getLastPickedActor()
{
	return _lastPickedActor;
}

Vec3 Selection::getStabPointActorSpace()
{
	return stabPointActorSpace;
}

void Selection::setPickedActor(BumpObject *obj)
{
	_lastPickedActor = pickedActor;
	pickedActor = obj;

	if (obj)
	{
		if (getSize() == 0)
			add(obj);
	}
}

void Selection::setStabPointActorSpace(Vec3 stab)
{
	stabPointActorSpace = stab;
}

// Moves the picked actor. Applies the appropriate force to the selected actors.  
// Based on the old tickDrag() function.
void Selection::update()
{
	if (pickedActor && pickedActor->isDragging() && pickedActor->isObjectType(BumpCluster))
	{				
		((Cluster*)pickedActor)->onDrag(pointOnFloor(mouseManager->primaryTouchX, mouseManager->primaryTouchY));
	}


	else if (pickedActor && pickedActor->isDragging())
	{
		Vec3 worldMousePos;
		Vec3 pickedActorPos;	
		Vec3 pickedActorLocalPos = stabPointActorSpace;
		Mat34 refPose = pickedActor->getGlobalPoseReference();
		refPose.multiply(pickedActorLocalPos, pickedActorPos);
		int numItemsInSelection = sel->getBumpObjects().size();
		bool isTemporaryActor = pickedActor->isObjectType(ObjectType(BumpActor, Temporary));
		bool isPile = pickedActor->isBumpObjectType(BumpPile);

		if ((numItemsInSelection == 1) && !isPile && !isTemporaryActor && GLOBAL(MouseOverWall))
		{
			Vec3 closePoint, farPoint, dir;
			window2world(mouseManager->primaryTouchX, mouseManager->primaryTouchY, closePoint, farPoint);
			dir = farPoint - closePoint;
			dir.normalize();
			Ray worldRay(closePoint, dir);
			tuple<int, NxReal, Vec3, NxPlane> t = RayIntersectDesktop(worldRay, 0.0f);
			worldMousePos = t.get<2>();

			// project the actor's position onto the plane so that we can get an offset vector
			// from the point of intersection and then use that as the world position
			NxPlane p = t.get<3>();
			p.d += pickedActor->getDims().z;
				
			Vec3 pt;
			NxF32 dist;
			NxRayPlaneIntersect(worldRay, p, dist, worldMousePos);
		}
		else
		{
			float z = 0.0f;
			if (pickedActor->getObjectType() == ObjectType(BumpActor))
			{
				Actor * a = (Actor *) pickedActor;
				if (a->getObjectToMimic())
					z = a->getObjectToMimic()->getGlobalPosition().y;
				else
					z = pickedActor->getDims().z;
			}
			else if (pickedActor->getObjectType() == ObjectType(BumpPile))
			{
				Pile * p = (Pile *) pickedActor;				
				if (p->getPileState() == Grid)
				{
					z = repoManager->getPlaneFloorLevel();
				}
				else
				{
					// use the pos.y (actor.z) as the min level so we don't get flying
					if (p->getNumItems() > 0)
					 	z = pickedActorPos.y;					
				}
			}
			worldMousePos = ClientToWorld(mouseManager->primaryTouchX, mouseManager->primaryTouchY, z);
		}
		
		const float kSpring = 150.0f;
		const float kDamping = 20.0f;
		adjustPointToInsideWalls(worldMousePos);
		Vec3 deltaDist = (worldMousePos - pickedActorPos);
		if (deltaDist.magnitude() > 0.005f)
		{
			Vec3 force = pickedActor->getMass() * 
						(kSpring * deltaDist - 
						 kDamping * pickedActor->getLinearVelocity());		
			pickedActor->addForceAtLocalPos(force, pickedActorLocalPos);
		
			// note: we only want the item over the wall to be pinned			
			{
				// we want each actor to spring to their original relative positions
				// to the picked actor
				const Mat34& pickedActorPose = pickedActor->stateBeforeDrag().pose;
				Vec3 curPickedActorPos = pickedActor->getGlobalPosition();
				curPickedActorPos.y = pickedActorPose.t.y;
				
				for (uint i = 0; i < selectedObjects.size(); i++)
				{
					if (selectedObjects[i] != pickedActor)
					{
						const Mat34& actorPose = selectedObjects[i]->stateBeforeDrag().pose;
						Vec3 relOffset = actorPose.t - pickedActorPose.t;
						Vec3 newActorPos = curPickedActorPos + relOffset;
						adjustPointToInsideWalls(newActorPos);
						Vec3 deltaDist = newActorPos - selectedObjects[i]->getGlobalPosition();
												
						Vec3 force = selectedObjects[i]->getMass() * 
									((2.0f * kSpring) * deltaDist - 
									 (1.0f * kDamping) * selectedObjects[i]->getLinearVelocity());		
						selectedObjects[i]->addForceAtLocalPos(force, Vec3(0.0f));
					}
				}
			}
		}
	}
}

bool Selection::isBumpObjectValid(BumpObject *obj)
{
	// Check to see if this item is already here
	if (!obj) return false;
	if (isInSelection(obj)) return false;

	// Specific ignore states for Actors
	if (obj->isBumpObjectType(BumpActor))
	{
		Actor *aData = (Actor *) obj;

		if (aData->isActorType(Invisible)) return false;
	}

	// Specific Ignore cases for Piles
	if (obj->isBumpObjectType(BumpPile))
	{
		Pile *pile = (Pile *) obj;

		if (pile->getPileState() == LaidOut ||
			pile->getPileState() == LayingOut ||
			pile->getPileState() == NoState) return false;
	}

	// Widgets are excluded
	if (obj->isBumpObjectType(BumpWidget)) return false;

	return true;
}