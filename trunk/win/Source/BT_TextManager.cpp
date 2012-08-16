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
#include "BT_TextManager.h"
#include "BT_SceneManager.h"
#include "BT_Util.h"
#include "BT_Struct.h"
#include "BT_Pile.h"
#include "BT_FontManager.h"
#include "BT_BumpObject.h"
#include "BT_OverlayComponent.h"
#include "BT_WindowsOS.h"
#include "BT_RenderManager.h"

TextManager::TextManager()
: _isTextDirty(true)
, _ignoreImmediateCollisions(false)
, _disableForceUpdates(false)
{
}

TextManager::~TextManager()
{
}

void TextManager::reshapeTextLabels(bool ignoreCollisions)
{	
#ifdef QT_TEXT_RENDERING
	assert(false);
#else
	// generate a distance map between all bump objects
	map<NxActorWrapper *, pair<NxActorWrapper *, float> > minActorDistance;
	vector<BumpObject *> objects = scnManager->getBumpObjects();
	for (int i = 0; i < objects.size(); ++i)
	{
		for (int j = 0; j < objects.size(); ++j)
		{
			if (!objects[i]->getParent() && !objects[j]->getParent() && (i != j))
			{
				float d = objects[i]->getGlobalPosition().distance(objects[j]->getGlobalPosition());
				// actor i
				if (minActorDistance.find(objects[i]) == minActorDistance.end())
					minActorDistance[objects[i]].second = d;
				else
					minActorDistance[objects[i]].second = NxMath::min(d, minActorDistance[objects[i]].second);
				minActorDistance[objects[i]].first = objects[j];
			}
		}
	}

	// Draw file path names
	vector <NxActorWrapper *> desktopItems = GetDesktopItems();
	vector <MouseCoordsPerActor> coords;
	MouseCoordsPerActor TempMouseCoord;
	int spacingY = 1; // Vertical padding between filenames
	bool centered=true; //centers filename label on icon. 
	float thresholdUp = 4.0, thresholdDown = 2.0;
	int chosenWidth;
	QString TempFileName;
	int dummy;

	for (int i = 0; i < desktopItems.size(); i++)
	{
		Actor *TempData = GetBumpActor(desktopItems[i]);

		if (TempData->getParent())
		{
			BumpObject *obj = TempData->getParent();

			if (!obj->isBumpObjectType(BumpPile))
			{
				continue;
			}
		}

		bool isIsolated = false;
		if (minActorDistance.find(desktopItems[i]) != minActorDistance.end())
		{
			const float margin = 6.0f;

			NxActorWrapper * otherActor = minActorDistance[desktopItems[i]].first;
			float d = minActorDistance[desktopItems[i]].second;
			Vec3 itemDims = desktopItems[i]->getDims();
			float itemDimsVal = itemDims.magnitude();
			Vec3 otherDims = otherActor->getDims();
			float otherDimsVal = otherDims.magnitude();
			float minWidth = itemDimsVal + otherDimsVal;
			isIsolated = (d > minWidth);
		}

		if (TempData && !TempData->isTextHidden() && desktopItems[i]->isDynamic() && TempData->isBumpObjectType(BumpActor) &&
			!(TempData->isActorType(Invisible)))
		{
			// un-comment for pile-grid-only truncation
			/*
			const int inPileTruncationLength = 10;
			if (TempData->getParent())
			{
				Pile * p = dynamic_cast<Pile *>(TempData->getParent());

				if (p->getPileState() == Grid)
				{
			*/
			if (!isIsolated)
			{
					FontDescription font = TempData->getNameableOverlay()->getTextOverlay()->getFont();
					QString text = TempData->getFullText();
					Vec3 pos = TempData->getGlobalPosition();
					Vec3 dim = TempData->getDims();
					Vec3 dims = WorldToClient(-dim) - WorldToClient(dim);
					int width = 0, height = 0;
						fontManager->getTextBounds(text, width, height, font);
					Vec3 textDims(float(width), float(height), 0);
					// Vec3 textDims = TempData->getNameableOverlay()->getTextBounds(0);
					Vec3 ellipsisDims = TempData->getNameableOverlay()->getEllipsisBounds();

					float margin = 1.4f;
					float charDim = textDims.x / float(text.size());
					int numChars = int(dims.x * margin / charDim);
					TempData->setText(text, numChars + 1);
					TempData->setTextTruncation(!TempData->isSelected());
			}
			else
			{
					TempData->setText(TempData->getFullText());
					TempData->setTextTruncation(false);
			}
			/*
				}
			}
			else
			{
				TempData->setText(TempData->getFullText());			
			}
			*/

			// Truncate the filename if its not selected and its long enough to be worth hiding
			TempFileName = TempData->getText();
			Vec3 size = TempData->getNameableOverlay()->getSize();
			chosenWidth = (int)size.x;
			dummy = (int)size.y;

			// Get the actor that is responsible for doing check on text
			if (!TempData->isTextHidden())
			{
				//Convert text offset according to actors dims.  Swap y and z coordinates since actor space is weird.  
				Vec3 fullDims;
				fullDims.z = !(TempData->isActorType(Text)) ? getDimensions(desktopItems[i]).y : 0.0f;
				Vec3 screenCoords = WorldToClient(desktopItems[i]->getGlobalPosition() + Vec3(0, 0, -fullDims.z * 1.25f));
				screenCoords.x -= chosenWidth / 2;
				screenCoords.y += !(TempData->isActorType(Text)) ? dummy : 0;

				TempMouseCoord.setMouseCoords(screenCoords, Vec3(float(chosenWidth), float(dummy), 0), TempFileName, desktopItems[i]);
				Pile * p = dynamic_cast<Pile *>(TempData->getParent());
				if (p && (p->getPileState() == Leaf))
					TempMouseCoord.ignoreCollision = true;
				coords.push_back(TempMouseCoord);
			}
		}
	}

	for (int i = 0; i < GLOBAL(getPiles()).size(); i++)
	{
		Pile *pile = GLOBAL(getPiles())[i];
		Actor *data;

		if (pile->getNumItems()	== 0)
			continue;

		data = (Actor *) pile->getFirstItem();

		// Hard piles only have a FolderOwner.
		if (data && (pile->getPileState() == Stack || pile->getPileState() == Leaf) && !pile->isTextHidden())
		{
			// Truncate the filename if its not selected and its long enough to be worth hiding
			pile->setTextTruncation(!pile->isSelected());
			TempFileName = pile->getText();
			Vec3 size = pile->getNameableOverlay()->getSize();
			chosenWidth = (int)size.x;
			dummy = (int)size.y;

			// Get the actor that is responsible for doing check on text
			if (pile->getNameableOverlay())
			{
				//Convert text offset according to pile dims
				Vec3 fullDims = pile->getDims();
				Vec3 screenCoords = WorldToClient(pile->getFirstItem()->getGlobalPosition() + Vec3(0, 0, -fullDims.y * 1.25f));
				screenCoords.x -= chosenWidth / 2;
				screenCoords.y += dummy; 

				TempMouseCoord.setMouseCoords(screenCoords, Vec3(float(chosenWidth), float(dummy), 0), TempFileName, NULL, pile);
				TempMouseCoord.ignoreCollision = true;

				coords.push_back(TempMouseCoord);
			}
		}
	}

	// Sort the icons' position so that they can be used in collision detection
	if (coords.size() > 0)
	{
	
		if (!ignoreCollisions)
		{
			// Sort actors according to thier X location
			sort(coords.begin(), coords.end(), less_x_value());

			// Greedy search to figure out positions
			for (int i = 1; i < coords.size(); i++)
			{
				if (coords[i].ignoreCollision)
					continue;

				// Loop backwards through the items to see if they intersect with this one
				for (int j = i - 1; j >= 0; j--)
				{
					if (coords[j].ignoreCollision)
						continue;

					// If the begging of this text label falls inside the bounds of a previous label
					if (coords[i].coord.x < coords[j].coord.x + (coords[j].textSize.x))
					{
						// See if these two bodies actually collide in 2D space by checking for the Y axis
						if (!(coords[i].actionTaken & MovedUp) && 
							coords[i].coord.y + coords[i].textSize.y >= coords[j].coord.y - spacingY &&
							coords[i].coord.y + coords[i].textSize.y <= coords[j].coord.y + coords[j].textSize.y + spacingY)
						{
							//          +---------------+
							// +--------+  TextBox 'i'  |
							// |        +---------+-----+
							// +------------------+ 
							// Try moving the text box 'i' up a bit to get it out of the way
							coords[i].coord.y = coords[j].coord.y - coords[i].textSize.y - spacingY;
							coords[i].actionTaken |= MovedUp;
						}

						if (!(coords[i].actionTaken & MovedDown) && 
							coords[i].coord.y >= coords[j].coord.y - spacingY &&
							coords[i].coord.y <= coords[j].coord.y + coords[j].textSize.y + spacingY)
						{
							// +------------------+ 
							// |        +---------+-----+
							// +--------+  TextBox 'i'  |
							//          +---------------+
							// Try moving the text box 'i' down a bit to get it out of the way
							coords[i].coord.y = coords[j].coord.y + coords[j].textSize.y + spacingY;
							coords[i].actionTaken |= MovedDown;
						}

						if (coords[i].actionTaken & MovedDown &&
							coords[i].actionTaken & MovedUp)
						{
							// This actor should be turned off since I cant move it anywhere safe
							if (coords[i].actor) 
							{	
								coords[i].setMouseCoords(Vec3(-100, 0, 0), Vec3(0, 0, 0), "", coords[i].actor);
							}
						}
					}
				}
			}
		}

		// Draw the text in its appropriate locations
		for (int j = 0; j < coords.size(); j++)
		{
			NxActorWrapper *a = coords[j].actor;
			Actor *aData = GetBumpActor(a);
			Pile *pile = coords[j].pile;
			QString text = coords[j].text;
			Nameable * nameable = aData ? GetBumpActor(coords[j].actor) : (Nameable *)pile;
			
			if (aData && text.isEmpty())
				continue;
			
			// Turn off file rendering when the filename gets too far from the icon
			if (!ignoreCollisions && nameable)
			{
				if (coords[j].coord.y - coords[j].origPos.y > (coords[j].textSize.y * thresholdDown) || coords[j].coord.y - coords[j].origPos.y < (coords[j].textSize.y * -thresholdUp))
				{
					// hide the text
					nameable->getNameableOverlay()->getStyle().setVisible(false);
				}
			}

			// Only draw the text if its available
			if ((aData && !aData->isTextHidden()) || (pile && !pile->isTextHidden()))
			{
				if (nameable)
				{
					Vec3 pos = aData ? aData->getGlobalPosition() : pile->getFirstItem()->getGlobalPosition();
					nameable->setRelativeTextPosition(coords[j].coord - WorldToClient(pos));

					bool respectIconExtensionsVisibile = false;
					if (aData && aData->isActorType(FileSystem))
						respectIconExtensionsVisibile = !((FileSystemActor *)aData)->isFileSystemType(Folder);
					nameable->getNameableOverlay()->updateTextFromNameable(respectIconExtensionsVisibile);
				}
			}
		}
	}
#endif
}

void TextManager::invalidate(bool ignoreCollisions)
{
	// update reshape flags
	_ignoreImmediateCollisions = _ignoreImmediateCollisions || ignoreCollisions;
	_isTextDirty = true;
}

bool TextManager::isValid() const 
{
	return !_isTextDirty;
}

void TextManager::forceValidate()
{
	_ignoreImmediateCollisions = false;
	_isTextDirty = false;
}

bool TextManager::updateRelativeNameablePositions()
{
#ifdef QT_TEXT_RENDERING
	if (_isTextDirty) {
		vector<BumpObject *> objects = scnManager->getBumpObjects();
		for (int i = 0; i < objects.size(); ++i)
		{
			Nameable * nameable = objects[i];
			if (nameable)
			{				
				nameable->getNameableOverlay()->updateTextFromNameable();

				Vec3 relPos(0.0f);
				relPos.x -= nameable->getNameableOverlay()->getSize().x / 2;				
				nameable->setRelativeTextPosition(relPos);
			}
		}		
		_ignoreImmediateCollisions = false;
		_isTextDirty = false;
		return true;
	}
#endif
	return false;
}

void TextManager::updateAbsoluteNameablePositions() 
{
	SceneManager * sceneManager = scnManager;

	// update the nameable text overlay (absolute) positions
	if (sceneManager->settings.RenderText)
	{
		const vector<BumpObject *>& objects = sceneManager->getBumpObjects();
		for (int i = 0; i < objects.size(); ++i)
		{				
			Nameable * nameable = objects[i];

			if (nameable && !nameable->getText().isEmpty())
			{
				// project the abb of the object
				Vec3 dims(0.0f);
				Vec3 worldOffsetPoint(0.0f);
				Vec3 pos(0.0f);
				bool useActor = true;
				if (objects[i]->isObjectType(BumpPile))
				{
					BumpObject * lastItem = ((Pile *) objects[i])->getLastItem();
					if (lastItem) 
					{
						dims = lastItem->getDims();
						Mat34 finalPose = lastItem->getAnimFinalPose();
						worldOffsetPoint = finalPose.M * Vec3(0, -dims.y, 0);
						pos = WorldToClient(finalPose.t + worldOffsetPoint);
						useActor = false;
					}
				}
				
				if (useActor)
				{
					Mat34 pose = objects[i]->getGlobalPose();
					dims = objects[i]->getDims();
					worldOffsetPoint = pose.M * Vec3(0, -dims.y, 0);
					pos = WorldToClient(pose.t + worldOffsetPoint);
				}

				if (!cam->isCameraFreeForm() && GLOBAL(settings).cameraPreset.startsWith("def")) 
				{
					// just use the abb to get the min
					Bounds abb = objects[i]->getBoundingBox();
					Vec3 screenMin = WorldToClient(abb.getMin());
					pos.y = screenMin.y;
				}
				else 
				{				
					// check all four corners and get the min
					Vec3 cornerPoint1 = objects[i]->getGlobalOrientation() * Vec3(-dims.x, -dims.y, 0);
					Vec3 cornerPoint2 = objects[i]->getGlobalOrientation() * Vec3(dims.x, -dims.y, 0);
					Vec3 cornerPoint3 = objects[i]->getGlobalOrientation() * Vec3(dims.x, dims.y, 0);
					Vec3 cornerPoint4 = objects[i]->getGlobalOrientation() * Vec3(-dims.x, dims.y, 0);
					pos.y = max(WorldToClient(objects[i]->getGlobalPosition() + cornerPoint1).y, pos.y);
					pos.y = max(WorldToClient(objects[i]->getGlobalPosition() + cornerPoint2).y, pos.y);
					pos.y = max(WorldToClient(objects[i]->getGlobalPosition() + cornerPoint3).y, pos.y);
					pos.y = max(WorldToClient(objects[i]->getGlobalPosition() + cornerPoint4).y, pos.y);
				}	
				pos += nameable->getRelativeTextPosition();

				int winHeight = winOS->GetWindowHeight();
				pos.y = (float)winHeight - pos.y;

				nameable->getNameableOverlay()->getStyle().setOffset(pos);
			}
		}

		// force the nameables to relayout themselves
		sceneManager->nameables()->reLayout();
		
		// Remove the text for all the items that intersect with the shadow of gridded piles
		sceneManager->nameables()->determineVisibility();
	}
	sceneManager->cursor(NULL, NULL)->reLayout();
}

void TextManager::enableForceUpdates()
{
	_disableForceUpdates = false;
	if (updateRelativeNameablePositions())
		updateAbsoluteNameablePositions();
}

void TextManager::disableForceUpdates()
{
	_disableForceUpdates = true;
}

void TextManager::forceUpdate()
{	
	invalidate();
	if (!_disableForceUpdates)
	{
		updateRelativeNameablePositions();
		updateAbsoluteNameablePositions();
	}
}