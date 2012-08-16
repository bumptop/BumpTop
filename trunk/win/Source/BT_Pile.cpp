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
#include "BT_AnimationEntry.h"
#include "BT_FileSystemManager.h"
#include "BT_GLTextureManager.h"
#include "BT_Macros.h"
#include "BT_OverlayComponent.h"
#include "BT_PbPersistenceManager.h"
#include "BT_PbPersistenceHelpers.h"
#include "BT_Pile.h"
#include "BT_RepositionManager.h"
#include "BT_Selection.h"
#include "BT_StatsManager.h"
#include "BT_Struct.h"
#include "BT_SceneManager.h"
#include "BT_TextManager.h"
#include "BT_Util.h"
#include "BT_UndoStack.h"
#include "BT_WindowsOS.h"
#include "BumpTop.pb.h"

#ifdef DXRENDER
#include "BT_DXRender.h"
#endif

// A comparison function that sorts by the creation time of the files.
// Files most recently created will come first.
// The items to be compared must both be FileSystemActors. 
struct sortByCreationTime : public std::binary_function<BumpObject *, BumpObject *, bool>
{
	inline bool operator()(BumpObject *x, BumpObject *y)
	{
		QFileInfo fileInfo1(((FileSystemActor *)x)->getFullPath());
		QFileInfo fileInfo2(((FileSystemActor *)y)->getFullPath());

		return (fileInfo1.created() > fileInfo2.created());
	}
};

Pile::Pile()
: gridView(ContainerView<BumpObject *>(pileItems, PILE_GRID_ROWSIZE, PILE_GRID_COLSIZE))
{
 	setBumpObjectType(BumpPile);
	setPileType(SoftPile);
	setPileState(NoState);
	setAlpha(0.0f);
	fanningOut = false;
	fanoutLen = 0.0f;
	bufferZone = PILE_SLATE_THICKNESS;
	closeWidget = launchOwnerWidget = nextPageWidget = prevPageWidget = NULL;
	launchExternalWidget = NULL;
	phUnhinged = false;
	leafIndex = -1;
	animationStepOverride = -1;
	displayLaunchOwnerWidget = false;

	setGlobalOrientation(GLOBAL(straightIconOri));
	
	savedStackPosition.setNotUsed();

	// Turn off rotation for the phantom actor
	setRotation(false);

	// update the shell extension status bar if necessary
	if (scnManager->isShellExtension)
		winOS->ShellExtUpdateStatusBar();

	// record this creation
	statsManager->getStats().bt.interaction.piles.created++;
}

Pile::~Pile()
{
	clear();
	repoManager->removeFromPileSpace(this);

	// record this destruction
	statsManager->getStats().bt.interaction.piles.destroyed++;

	destroyPileWidgets();
}

void Pile::grow(uint numSteps, float growFactor)
{
	if (!_onGrowHandler.empty())
		_onGrowHandler();

	assert(!pileItems.empty());

	// Bounce Grow each item
	for (uint i = 0; i < pileItems.size(); i++)
	{
		pileItems[i]->grow(numSteps, growFactor);
	}

	updatePhantomActorDims();
}

void Pile::shrink(uint numSteps, float shrinkFactor)
{
	assert(!pileItems.empty());

	// Bounce Shrink each item
	for (uint i = 0; i < pileItems.size(); i++)
	{
		pileItems[i]->shrink(numSteps, shrinkFactor);
	}

	updatePhantomActorDims();
}

// Scales all pile items' reference dimensions by the scale factor.
void Pile::scaleFromReferenceDims(float scaleFactor)
{
	for (uint i = 0; i < pileItems.size(); i++)
	{
		pileItems[i]->scaleFromReferenceDims(scaleFactor);
	}

	updateObject();
	updatePileState();
}

bool shouldShowFileNameForLeafItem(BumpObject * obj)
{
	if (obj->isObjectType(ObjectType(BumpActor, FileSystem, StickyNote)))
		return false;
	return true;
}

void Pile::leafTo(int index)
{
	int collapseTicks = 20;
	int expandTicks = 20;
	int expandTextTicks = 15;

	if ((getPileState() & (Stack|Leaf)) == 0) return;

	int newLeafIndex = NxMath::max(0, NxMath::min(index, (int)pileItems.size() - 1));
	
	// don't do anything if we are at the bottom of the pile
	if (leafIndex == newLeafIndex)
		return;
		
	// animate the currently leafed item back into the pile
	if (leafIndex > -1) 
	{
		BumpObject * leafItem = pileItems[leafIndex];
		
		Vec3 pos = getGlobalPosition();
		Mat34 finalLeafItemPose = leafItem->getGlobalPose();
			finalLeafItemPose.t.x = pos.x;
			finalLeafItemPose.t.z = pos.z;
			
		float originalAnimDistance = (pos - leafItem->getAnimFinalPose().t).magnitude();
		float distanceRemainingInAnim = (leafItem->getGlobalPosition() - leafItem->getAnimFinalPose().t).magnitude();
		float percentAnimLeft = distanceRemainingInAnim / originalAnimDistance;
		int remainingTicks = percentAnimLeft * collapseTicks;
		
		Mat34 leafedOutPose = leafItem->getAnimFinalPose();
		if (percentAnimLeft > 0.5f)
		{
			collapseTicks = 5;
			Vec3 midPoint = ((leafedOutPose.t - pos) / 3.0f) + pos;
			leafedOutPose.t = midPoint;
			remainingTicks = (percentAnimLeft + 0.5f) * collapseTicks;
		}
		deque<Mat34> animOut;
		if (remainingTicks > 0)
			animOut = lerpMat34RangeRaw(leafItem->getGlobalPose(), leafedOutPose, remainingTicks, Ease);
		deque<Mat34> animIn = lerpMat34RangeRaw(leafedOutPose, finalLeafItemPose, collapseTicks, NoEase);
		animOut.insert(animOut.end(), animIn.begin(), animIn.end());
		
		pileItems[leafIndex]->setPoseAnim(animOut);
		pileItems[leafIndex]->getNameableOverlay()->getStyle().setVisible(false);	
		relPosToPhantomCentroid[leafIndex] = finalLeafItemPose.t - getGlobalPosition();
	}

	// animate the next leafed item out
	BumpObject * newLeafItem = pileItems[newLeafIndex];
	
	Vec3 pos = getGlobalPosition();
	float testOffsetX = 2.0f * getDims().x + 30.0f;	// this is to ensure that all slides follow the largest item slide
	float offsetX = getDims().x + newLeafItem->getAnimFinalDims().x;
	Vec3 tmp(pos.x + testOffsetX, 0, 0);
	bool canSlideLeft = getGlobalPosition().x <= 0.0f;
	Mat34 finalLeafItemPose = newLeafItem->getGlobalPose();			
		finalLeafItemPose.t.x = pos.x + (canSlideLeft ? offsetX : -offsetX);
		finalLeafItemPose.t.z = pos.z;
	pileItems[newLeafIndex]->setPoseAnim(newLeafItem->getGlobalPose(), finalLeafItemPose, expandTicks);
	if (shouldShowFileNameForLeafItem(pileItems[newLeafIndex]))
	{
		pileItems[newLeafIndex]->getNameableOverlay()->getStyle().setVisible(true);
		pileItems[newLeafIndex]->getNameableOverlay()->setAlphaAnim(0.0f, 1.0f, expandTextTicks);
	}
	relPosToPhantomCentroid[newLeafIndex] = finalLeafItemPose.t - getGlobalPosition();
	
	leafIndex = newLeafIndex;

	// set the pile state
	setPileState(Leaf);

	// remove the selection on any pile item and the pile itself
	sel->remove(this);
	for (int i = 0; i < pileItems.size(); ++i)
		sel->remove(pileItems[i]);
	// mark the new leaf item as selected
	sel->add(newLeafItem);
	
	// increment the stats
	statsManager->getStats().bt.interaction.piles.leafed++;

	// invalidate the text manager
	textManager->invalidate(false);
}

// Leafs to a particular pile item. Does nothing if the object is not a child item.
void Pile::leafTo(BumpObject * pileItem)
{
	for (int i = 0; i < pileItems.size(); ++i)
	{
		if (pileItems[i] == pileItem)
		{
			leafTo(i);
			return;
		}
	}
}

void Pile::leafUp(int numItems)
{
	if (numItems < 0)
		return;

	for (int i = 1; i <= numItems; i++)
	{
		leafTo(getLeafIndex() - i);
	}
}

void Pile::leafDown(int numItems)
{
	if (numItems < 0)
		return;

	for (int i = 1; i <= numItems; i++)
	{
		leafTo(getLeafIndex() + i);
	}
}

// Leafs to the bottom of the pile
void Pile::leafToBottom()
{
	leafTo(pileItems.size() - 1);
}

// Removes the current leafed item from the current pile and
// moves onto the next leafed item if there is one
void Pile::tearLeafedItem()
{
	BumpObject * nextItem = NULL;
	if (pileItems.size() > 1) 
	{
		if (leafIndex == (pileItems.size() - 1))
			nextItem = pileItems[(int) pileItems.size() - 2];
		else 
			nextItem = pileItems[(int) leafIndex + 1];
	}
	
	// remove it from the pile
	removeFromPile(getActiveLeafItem());
}

void Pile::beginFanout()
{
	assert(!fanningOut);

	Vec3 phPos;

	if (pileItems.size() > 1)
	{
		if (Grid == getPileState())
			restoreDimsBeforeGrid();

		// If we are already fanning out, don't reinitialize it
		fanningOut = true;
		fanoutPts.clear();
		fanoutLen = 0.0f;

		// Add the first point
		phPos = pileItems[0]->getGlobalPosition();
		fanoutPts.push_back(Vec3(phPos.x, 0.0f, phPos.z));

		// Turn on text for all items
		saveRelativePositions(phOldCent);
		setPileState(LayingOut);
		unhingePhantomActor();

		// reset orientations to straight icon ori for actors
		for (int i = 0; i < pileItems.size(); ++i)
		{
			pileItems[i]->setGlobalOrientation(GLOBAL(straightIconOri));

			// enable all the text labels
			if (pileItems[i]->getNameableOverlay())
				pileItems[i]->getNameableOverlay()->getStyle().setVisible(true);
		}
	}else{
		MessageClearPolicy clearPolicy;
		clearPolicy.setTimeout(3);
		scnManager->messages()->addMessage(new Message("Pile::beginFanout", QT_TR_NOOP("Piles with 1 item in them cannot be fanned out."), Message::Ok, clearPolicy));
	}
}

bool Pile::fanoutTick(Vec3 nextPt)
{
	uint maxLassoSize = 300;
	Vec3List pts;
	Box box;

	if (fanningOut && !fanoutPts.empty())
	{
		// Limit on lasso
		if (fanoutPts.size() > maxLassoSize)
		{
			endFanout();
			return false;
		}

		// Set up the way the phantom actor was when it was Stacked
		box.center = nextPt;
		box.extents = Vec3(phOldDims.x, 0, phOldDims.z);
		box.rot = Mat33(NX_IDENTITY_MATRIX);

		// Adjust the point to be inside the walls.
		if (adjustBoxToInsideWalls(box))
		{
			nextPt = box.center;
		}

		// Keep adding points into our lasso
		fanoutLen += nextPt.distance(fanoutPts[fanoutPts.size() - 1]);
		fanoutPts.push_back(nextPt);

		// Resample the Fanout Path
		pts = resamplePath();

		// Apply the re-sampled lasso to our items
		// adding an offset to prevent flickering from items on the same y level
		uint j = 0;
		const float yDelta = 0.01f;
		float yOffset = pileItems.back()->getGlobalPosition().y + (yDelta * pts.size());
		for (int i = pts.size() - 1; i >= 0; i--)
		{
			pts[i].y += yOffset;
			pileItems[j]->setGlobalPosition(pts[i]); // Vec3(pts[i].x, pileItems[j]->getGlobalPosition().y, pts[i].z));
			j++;
			yOffset -= yDelta;
		}

		return true;
	}

	return false;
}

void Pile::endFanout()
{
	assert(fanningOut);

	// Stop the fanning out mode
	fanningOut = false;
	createPileWidgets();
	setPileState(LaidOut);
	saveRelativePositions(getGlobalPosition());

	// Update the close Widget Position
	updateCloseWidgetPos();
}

Vec3List Pile::resamplePath()
{
	Vec3List equalPts;
	float resolution;
	float accumLenSq = 0;
	float localInterval;
	float overshotLen;

	// Figure out the optimal size between two points for this path
	resolution = fanoutLen / (pileItems.size() - 1);

	// Save the first one because it doesn't move
	equalPts.push_back(fanoutPts[0]);

	// Distribute the points evenly
	for (uint i = 0; i < fanoutPts.size() - 1; i++)
	{
		// Distance between two points
		localInterval = fanoutPts[i].distance(fanoutPts[i + 1]);
		accumLenSq += localInterval;

		// We passed the threshold, double back to get accurate point
		while (accumLenSq >= resolution)
		{
			// X---------X-------*--X--------X
			//           ^       ^
			//           |-------|
			//       Overshot Distance
			overshotLen = accumLenSq - resolution;

			// Figure out the appropriate point between these two points
			Vec3 newPt = fanoutPts[i + 1] - ((fanoutPts[i + 1] - fanoutPts[i]) * (overshotLen / localInterval));

			equalPts.push_back(newPt);

			// Move back the resolution for further testing
			accumLenSq -= resolution;
		}
	}

	// Just in case our math is off due to rounding errors, add the last point
	if (equalPts.size() < pileItems.size())
	{
		equalPts.push_back(fanoutPts.back());
	}

	return equalPts;
}

void Pile::finishAnimation()
{
	// Finish the animation of all items in the pile
	for (uint i = 0; i < pileItems.size(); i++)
	{
		animManager->finishAnimation(pileItems[i]);
	}

	BumpObject::finishAnimation();
}

BumpObject* Pile::operator[](const uint indx)
{
	assert(indx < pileItems.size());
	return pileItems[indx];
}

bool Pile::getLogHeightSteps(float& logHeightStepOut)
{
	const int maxLinearHeightPileSize = GLOBAL(settings).maxLinearPileHeight;
	bool useLogHeight = pileItems.size() > maxLinearHeightPileSize;

	if (useLogHeight) 
	{
		int lhExtraItems = pileItems.size() - maxLinearHeightPileSize;
		float lhPileHeight = pow((float) log((float) (1 + lhExtraItems)), 2.4f);

		// Calculate the height of the max linear pile
		for(int i=0; i < maxLinearHeightPileSize; ++i) 
		{
			lhPileHeight += 2 * pileItems[i]->getDims().z;
		}

		// Distribute the logHeight evenly between all items
		logHeightStepOut = float(lhPileHeight) / pileItems.size();
	}

	return useLogHeight;
}

// Calculate the relative positions from the given centroid
void Pile::saveRelativePositions(Vec3 centroid)
{
	// This method is only used in leaf and fanout, and the assumption is that
	// relPosToPhantomCentroid is for a pile oriented parallel to the floor.
	assert(getPileState() != Grid);

	relPosToPhantomCentroid.clear();
	for (uint i = 0; i < pileItems.size(); i++)
	{
		relPosToPhantomCentroid[i] = pileItems[i]->getGlobalPosition() - centroid;
	}
	assert(relPosToPhantomCentroid.size() == pileItems.size());
}

void Pile::unhingePhantomActor()
{
	// Move the Phantom Actor out of sight
	phOldCent = getGlobalPosition();
	phOldDims = getDims();
	setDims(Vec3(0.1f, 0.1f, 0.1f));
	setCollisions(false);
	setGravity(false);
	PushBelowGround(this);
	phUnhinged = true;
}

void Pile::rehingePhantomActor()
{
	// Bring the phantom actor back
	setCollisions(true);
	setGravity(true);
	setGlobalPosition(phOldCent);
	phUnhinged = false;

	// Update the Dims of the phantom actor
	updatePhantomActorDims();
}

void Pile::createPileWidgets()
{
	if (!closeWidget)
	{
		closeWidget = new Actor();
		closeWidget->setText("closeWidget");
		closeWidget->hideText(true);
		closeWidget->setBumpObjectType(BumpWidget);
		closeWidget->setDims(Vec3(GLOBAL(settings).xDist, GLOBAL(settings).zDist, GLOBAL(settings).yDist) * 0.9f);
		closeWidget->setGlobalOrientation(GLOBAL(straightIconOri));
		closeWidget->setGlobalPosition(Vec3(0.0f));
		closeWidget->setCollisions(false);
		closeWidget->setGravity(false);
		closeWidget->setParent(this);
		closeWidget->setFrozen(true);
		closeWidget->setTextureID("widget.close");
		closeWidget->setAlpha(0.0f);
	}

	if (getPileState() == Grid)
	{
		if (displayLaunchOwnerWidget && !launchOwnerWidget)
		{
			launchOwnerWidget = new Actor();
			launchOwnerWidget->setText("launchOwnerWidget");
			launchOwnerWidget->hideText(true);
			launchOwnerWidget->setBumpObjectType(BumpWidget);
			launchOwnerWidget->pushActorType(Invisible);
			launchOwnerWidget->pushActorType(Temporary);
			launchOwnerWidget->setDims(Vec3(GLOBAL(settings).xDist, GLOBAL(settings).zDist, GLOBAL(settings).yDist) * 0.7f);
			launchOwnerWidget->setGlobalOrientation(GLOBAL(straightIconOri));
			launchOwnerWidget->setGlobalPosition(Vec3(0.0f));
			launchOwnerWidget->setCollisions(false);
			launchOwnerWidget->setGravity(false);
			launchOwnerWidget->setParent(this);
			launchOwnerWidget->setFrozen(true);
			launchOwnerWidget->setTextureID("widget.launchInExplorer");
			launchOwnerWidget->setAlpha(0.0f);
		}

		if (!nextPageWidget)
		{
			nextPageWidget = new Actor();
			nextPageWidget->setText("nextPageWidget");
			nextPageWidget->hideText(true);
			nextPageWidget->setBumpObjectType(BumpWidget);
			nextPageWidget->pushActorType(Invisible);
			nextPageWidget->pushActorType(Temporary);
			nextPageWidget->setDims(Vec3(GLOBAL(settings).xDist, GLOBAL(settings).zDist, GLOBAL(settings).yDist) * 0.7f);
			nextPageWidget->setGlobalOrientation(GLOBAL(straightIconOri));
			nextPageWidget->setGlobalPosition(Vec3(0.0f));
			nextPageWidget->setCollisions(false);
			nextPageWidget->setGravity(false);
			nextPageWidget->setParent(this);
			nextPageWidget->setFrozen(true);
			nextPageWidget->setTextureID("widget.scroll.down");
			nextPageWidget->setAlpha(0.0f);
		}

		if (!prevPageWidget)
		{
			prevPageWidget = new Actor();
			prevPageWidget->setText("prevPageWidget");
			prevPageWidget->hideText(true);
			prevPageWidget->setBumpObjectType(BumpWidget);
			prevPageWidget->pushActorType(Invisible);
			prevPageWidget->pushActorType(Temporary);
			prevPageWidget->setDims(Vec3(GLOBAL(settings).xDist, GLOBAL(settings).zDist, GLOBAL(settings).yDist) * 0.7f);
			prevPageWidget->setGlobalOrientation(GLOBAL(straightIconOri));
			prevPageWidget->setGlobalPosition(Vec3(0.0f));
			prevPageWidget->setCollisions(false);
			prevPageWidget->setGravity(false);
			prevPageWidget->setParent(this);
			prevPageWidget->setFrozen(true);
			prevPageWidget->setTextureID("widget.scroll.up");
			prevPageWidget->setAlpha(0.0f);
		}

		if (GLOBAL(enableSharingMode) && !isHorizontal() && !launchExternalWidget)
		{
			launchExternalWidget = new Actor();	
			launchExternalWidget->setText("launchExternalWidget");
			launchExternalWidget->hideText(true);
			launchExternalWidget->setBumpObjectType(BumpWidget);
			launchExternalWidget->pushActorType(Invisible);
			launchExternalWidget->pushActorType(Temporary);
			Vec3 textureDims = texMgr->getTextureDims("icon.live_mesh");
			float aspectRatio = textureDims.x / textureDims.y;
			float x = GLOBAL(settings).xDist;
			float actorHeight = GLOBAL(settings).zDist; // Same height as the other widgets
			float z = GLOBAL(settings).zDist;
			launchExternalWidget->setDims(Vec3(actorHeight * aspectRatio, actorHeight, GLOBAL(settings).yDist) * 0.7f);
			launchExternalWidget->setGlobalOrientation(GLOBAL(straightIconOri));
			launchExternalWidget->setGlobalPosition(Vec3(0.0f));
			launchExternalWidget->setCollisions(false);
			launchExternalWidget->setGravity(false);
			launchExternalWidget->setParent(this);
			launchExternalWidget->setFrozen(true);
			launchExternalWidget->setTextureID("icon.live_mesh");
			launchExternalWidget->setAlpha(0.0f);
		}
	}
}

void Pile::updateGridWidgets()
{
	if (displayLaunchOwnerWidget)
		assert(launchOwnerWidget);
	assert(prevPageWidget);
	assert(nextPageWidget);
	assert(getPileState() == Grid);

	Vec3 pileDims = getDims();
	Mat33 pileOrientation = getGlobalOrientation();
	const float buffer = 1.0f;

	// show the widgets if necessary
	if (displayLaunchOwnerWidget)
	{
		launchOwnerWidget->popActorType(Invisible);
		launchOwnerWidget->popActorType(Temporary);
		if ((launchOwnerWidget->getAlpha() == 0.0f) && 
			!launchOwnerWidget->isAnimating(AlphaAnim))
			launchOwnerWidget->setAlphaAnim(launchOwnerWidget->getAlpha(), 1.0f, 25);
	}
	if (launchExternalWidget)
	{
		launchExternalWidget->popActorType(Invisible);
		launchExternalWidget->popActorType(Temporary);
		if ((launchExternalWidget->getAlpha() == 0.0f) && 
			!launchExternalWidget->isAnimating(AlphaAnim))
			launchExternalWidget->setAlphaAnim(launchExternalWidget->getAlpha(), 1.0f, 25);
	}
	if ((prevPageWidget->getAlpha() == 0.0f) && 
		!prevPageWidget->isAnimating(AlphaAnim))
		prevPageWidget->setAlphaAnim(prevPageWidget->getAlpha(), 1.0f, 25);
	if ((nextPageWidget->getAlpha() == 0.0f) && 
		!nextPageWidget->isAnimating(AlphaAnim))
		nextPageWidget->setAlphaAnim(nextPageWidget->getAlpha(), 1.0f, 25);

	// Calculate the position of the actors relative to the center of the grid,
	// assuming a flat grid (exactly like item positions).

	// Put the close widget in the top-back-left
	float edgeBuffer = 1.0f;
	Vec3 relativePos(
		pileDims.x - edgeBuffer,
		pileDims.z - closeWidget->getDims().z + 0.1f,
		pileDims.y - edgeBuffer);
	closeWidget->setGlobalPose(Mat34(pileOrientation, getGlobalPositionForItem(relativePos)));	

	// Place the other elements relative to the top-back-right
	relativePos.x = -pileDims.x;
	if (!nextPageWidget->isActorType(Invisible))
	{
		Vec3 nextDims = nextPageWidget->getDims();
		relativePos.x += nextDims.x;
		nextPageWidget->setGlobalPose(Mat34(pileOrientation, getGlobalPositionForItem(relativePos)));
		relativePos.x += nextDims.x;
	}
	if (!prevPageWidget->isActorType(Invisible))
	{
		Vec3 prevDims = prevPageWidget->getDims();
		relativePos.x += prevDims.x;
		prevPageWidget->setGlobalPose(Mat34(pileOrientation, getGlobalPositionForItem(relativePos)));
		relativePos.x += prevDims.x;
	}
	if (displayLaunchOwnerWidget)
	{
		Vec3 launchDims = launchOwnerWidget->getDims();
		relativePos.x += launchDims.x;
		launchOwnerWidget->setGlobalPose(Mat34(pileOrientation, getGlobalPositionForItem(relativePos)));
		relativePos.x += launchDims.x;
	}
	if (launchExternalWidget)
	{
		Vec3 launchExternalDims = launchExternalWidget->getDims();
		relativePos.x += launchExternalDims.x;
		launchExternalWidget->setGlobalPose(Mat34(pileOrientation, getGlobalPositionForItem(relativePos)));
		relativePos.x += launchExternalDims.x;
	}

}

void Pile::syncScrollWidgets()
{
	const vector<BumpObject*>& objects = gridView.getContainer();
	bool showScrollWidgets = objects.size() > (gridView.getRowSize() * gridView.getColSize());

	// Temporary change for sharing prototype: don't let vertical grids scroll
	if (!isHorizontal())
		showScrollWidgets = false;
	
	if (showScrollWidgets)
	{
		if (prevPageWidget) 
		{
			prevPageWidget->popActorType(Invisible);
			prevPageWidget->popActorType(Temporary);
		}
		if (nextPageWidget) 
		{
			nextPageWidget->popActorType(Invisible);
			nextPageWidget->popActorType(Temporary);
		}
	}
	else
	{
		if (prevPageWidget) 
		{
			prevPageWidget->pushActorType(Invisible);
			prevPageWidget->pushActorType(Temporary);
		}
		if (nextPageWidget) 
		{
			nextPageWidget->pushActorType(Invisible);
			nextPageWidget->pushActorType(Temporary);
		}
	}

	if (prevPageWidget)
	{
		QString prevPageTextureId = prevPageWidget->getTextureID();
		if (!canScrollGridRow(-1))
			prevPageWidget->setTextureID("widget.scroll.up_disabled");
		else
			prevPageWidget->setTextureID("widget.scroll.up");	
	}

	if (nextPageWidget)
	{
		QString nextPageTextureId = nextPageWidget->getTextureID();
		if (!canScrollGridRow(1))
			nextPageWidget->setTextureID("widget.scroll.down_disabled");
		else
			nextPageWidget->setTextureID("widget.scroll.down");	
	}
}

void Pile::destroyPileWidgets()
{
	SAFE_DELETE(closeWidget);
	SAFE_DELETE(launchOwnerWidget);
	SAFE_DELETE(prevPageWidget);
	SAFE_DELETE(nextPageWidget);
	SAFE_DELETE(launchExternalWidget);
}

// Look at the dims of the visible grid items, and adjust currentDims so that
// the item is no bigger than the biggest
void Pile::adjustToVisibleGridItems(Vec3& dims)
{
	int visibleCount = gridView.getVisibleContainer().size();
	int visibleOffset = gridView.getVisibleItemOffsetIndex();

	// Give the item a new size no bigger than the biggest, no smaller than the smallest

	Vec3 minDims, maxDims;
	assert((visibleOffset + visibleCount) <= pileItems.size());
	for (uint i = 0; i < visibleCount; i++)
	{
		Vec3 itemDims = pileItems[visibleOffset + i]->getDims();
		if (i == 0)
		{
			minDims = maxDims = itemDims;
		}
		else
		{
			if (minDims.x > itemDims.x) minDims.x = itemDims.x;
			if (minDims.y > itemDims.y) minDims.y = itemDims.y;
			if (maxDims.x < itemDims.x) maxDims.x = itemDims.x;
			if (maxDims.y < itemDims.y) maxDims.y = itemDims.y;
		}
	}

	float scale = 1;
	if (dims.x > dims.y)
	{
		if (dims.x > maxDims.x)
			scale = maxDims.x / dims.x;
	}
	else
	{
		if (dims.y > maxDims.y)
			scale = maxDims.y / dims.y;
	}

	if (1 != scale)
	{
		dims.x *= scale;
		dims.y *= scale;
	}
}

bool Pile::addToPile(BumpObject *obj)
{
	Vec3 cent = Vec3(0.0f);

	int existingIndex = isInPile(obj);
	if (existingIndex > -1)
	{
		if (obj != pileItems[existingIndex])
		{
			BumpObject * tmp = pileItems[existingIndex];
			Pile::removeFromPile(tmp, NoUpdate);
			tmp->setGravity(false);
			tmp->setCollisions(false);
			tmp->setRotation(false);
			tmp->setFrozen(true);
			animManager->finishAnimation(tmp);
			SAFE_DELETE(tmp);
			existingIndex = -1;
		}
	}

	// If this item is already in our pile, ignore it
	if (existingIndex == -1 && !obj->isParentType(BumpPile))
	{
		if (obj->isBumpObjectType(BumpPile))
		{
			Pile *pile = (Pile *) obj;
			uint sz = pile->getNumItems();

			// Merge the piles by removing form one and adding to another
			for (uint j = 0; j < sz; j++)
			{
				BumpObject *obj = (*pile)[0];

				pile->removeFromPile(obj, NoUpdate);
				addToPile(obj);
			}

			// End after adding things to the pile recursively
			return true;
		}

		// Save its previous position and size
		if (getPileState() != NoState) cent = getGlobalPosition();
		savedMessyPoses[obj] = Mat34(obj->getGlobalOrientation(), obj->getGlobalPosition() - cent);

		// Adjust the size of the item to match those already in the grid
		if (getPileState() == Grid)
		{
			Vec3 dims = obj->getDims();
			adjustToVisibleGridItems(dims);
			obj->setDims(dims);

			// We must call this because the animation uses the dims before drag
			// to set the final size of the item
			obj->stateBeforeDrag().dims = dims;
		}
		// close the leafed pile first if we are dragging and dropping
		else if (getPileState() == Leaf)
			close();

		// Put the item at the bottom of the pileItems vector
		pileItems.insert(pileItems.begin(), obj);

		// Set up the new Object
		obj->setParent(this);
		obj->setGravity(false);
		obj->setCollisions(false);
		obj->setRotation(false);
		obj->setFrozen(true);

		// hide all the text labels
		if (obj->getNameableOverlay())
			obj->getNameableOverlay()->getStyle().setVisible(false);
		
		// Give it a random orientation
		NxQuat ori;
		NxReal angle;
		NxVec3 axis;
		srand((unsigned int) time(NULL));
		obj->getGlobalOrientation().toQuat(ori);
		ori.getAngleAxis(angle, axis);
		Mat33 newOri = GLOBAL(straightIconOri);
		int maxRotationLimits = NxMath::min(int(GLOBAL(settings).RotationLimitDegrees), 20);
		if (maxRotationLimits > 0)
			newOri *= Quat(float((rand() % maxRotationLimits) - (maxRotationLimits / 2.0f)), Vec3(0,0,1));
		obj->setGlobalOrientation(newOri);

		// Record this add and force a save to the filesystem
		statsManager->getStats().bt.interaction.actors.addedToPile++;

		// finish animation if we are in stack mode, so that we can 
		// re-stack
		if (getPileState() == Stack)
			animManager->finishAnimation(this);
		updatePileState();

		// Stop all pile motion because bit items may interpenetrates it
		stopAllMotion();
		putToSleep();

		// make the text fit a single line
		if (obj->getNameableOverlay() && obj->getNameableOverlay()->getTextOverlay())
		{
			obj->getNameableOverlay()->getTextOverlay()->getTextBuffer().pushFlag(TextPixmapBuffer::TruncateToSingleLine);
			obj->getNameableOverlay()->getTextOverlay()->getTextBuffer().update();
		}

		return true;
	}

	return false;
}

bool Pile::removeFromPile(BumpObject *obj, UpdateFlag updateFlag)
{
	assert(isInPile(obj) > -1);

	// Set the parent of this removed Object to the parent of this pile (For recursive Purposes)
	obj->setParent(getParent());
	obj->setGravity(true);
	obj->setCollisions(true);
	obj->setRotation(true);
	obj->setFrozen(false);

	if (Grid == getPileState())
	{
		obj->setDims(itemDimsBeforeGrid[obj]);
		itemDimsBeforeGrid.erase(obj);
	}

	// restore the normal text truncation
	if (getNameableOverlay() && getNameableOverlay()->getTextOverlay())
	{
		obj->getNameableOverlay()->getTextOverlay()->getTextBuffer().popFlag(TextPixmapBuffer::TruncateToSingleLine);
		obj->getNameableOverlay()->getTextOverlay()->getTextBuffer().update();
	}

	// enable the text label
	if (obj->getNameableOverlay())
		obj->getNameableOverlay()->getStyle().setVisible(true);

	// Remove this item form the pileItems list
	vector<BumpObject *>::iterator iter = find(pileItems.begin(),
		pileItems.end(), obj);
	if (iter != pileItems.end())
		pileItems.erase(iter);

	// record this remove
	statsManager->getStats().bt.interaction.actors.removedFromPile++;

	if (updateFlag == Update)
	{
		switch (getPileState())
		{
		case Stack:
		case Leaf:
			stack(getGlobalPosition(), false);
			break;
		case Grid:
			if (canScrollGridRow(1))
			{
				scrollGridRow(1);
				scrollGridRow(-1);
			}
			syncScrollWidgets();
			break;
		default:
			break;
		}
		updatePileState();
	}

	return true;
}

Bounds Pile::getPileBounds(bool includeWidgetsBounds)
{
	Bounds bounds;
	Vec3 largestDims;

	// Clear the bounds
	bounds.setEmpty();
	
	if (getPileState() == Grid)
	{
		largestDims = getDimsOfLargestPileItem();
	}

	// Calculate the Bounding Box Size
	for (int i = 0; i < pileItems.size(); i++)
	{
		Actor * actor = (Actor *) pileItems[i];
		if (actor && actor->isActorType(Invisible))
			continue;
		if (getActiveLeafItem() == actor)
			continue;
		
		Bounds itemBounds = pileItems[i]->getBoundingBox();
		if (getPileState() == Grid)
		{
			Vec3 center = pileItems[i]->getGlobalPosition();
			itemBounds.setCenterExtents(center, pileItems[i]->getGlobalOrientation() * largestDims);
		}
		bounds.combine(itemBounds);
	}
	
	// include the widgets in the bounds if necessary
	if (includeWidgetsBounds)
	{
		if (closeWidget && !closeWidget->isActorType(Invisible)) bounds.combine(closeWidget->getBoundingBox());
		if (launchOwnerWidget && !launchOwnerWidget->isActorType(Invisible)) bounds.combine(launchOwnerWidget->getBoundingBox());
		if (prevPageWidget && !prevPageWidget->isActorType(Invisible)) bounds.combine(prevPageWidget->getBoundingBox());
		if (nextPageWidget && !nextPageWidget->isActorType(Invisible)) bounds.combine(nextPageWidget->getBoundingBox());
		if (launchExternalWidget && !launchExternalWidget->isActorType(Invisible)) bounds.combine(launchExternalWidget->getBoundingBox());
	}

	// assert(!bounds.isEmpty());  //If its returning empty bounds something is wrong

	return bounds;
}

bool Pile::shiftItems(int index, int moveDistance)
{
	BumpObject *temp;

	// ensure that this is a valid move
	if ((moveDistance > 0 && index < pileItems.size() - 1) || 
		(moveDistance < 0 && index > 0))
	{
		animManager->finishAnimation(pileItems[index]);
		animManager->finishAnimation(this);

		// Actual Shuffle Code
		temp = pileItems[index];
		pileItems[index] = pileItems[index + moveDistance];
		pileItems[index + moveDistance] = temp;

		// Prevent weird Flipping that occurs when shifting
		pileItems[index]->setGlobalOrientation(GLOBAL(straightIconOri));
		pileItems[index + moveDistance]->setGlobalOrientation(GLOBAL(straightIconOri));

		return true;
	}
	return false;
}

// If reorient is true, the items will be re-oriented to match the orientation
// of the phantom actor. Default is false.
void Pile::animateItemsToRelativePos()
{
	for (uint i = 0; i < pileItems.size(); i++)
	{
		pileItems[i]->killAnimation();

		Vec3 pos = getGlobalPositionForItem(relPosToPhantomCentroid[i]);
		Mat34 newPose(pileItems[i]->getGlobalOrientation(), pos);
		int steps = (animationStepOverride > -1) ? animationStepOverride : 25;

		pileItems[i]->setPoseAnim(pileItems[i]->getGlobalPose(), newPose, steps);
	}
}

void Pile::updatePileItems(bool forceUpdate)
{
	if (getPileState() == NoState)
	{
		if (!isAnimating())
		{
			// Self delete if there is no animation and if its lingering
			animManager->addAnimation(AnimationEntry(this, (FinishedCallBack) DeletePileAfterAnim));
		}

		return;
	}
	
	Vec3 phCentre = phUnhinged ? phOldCent : getGlobalPosition();

	// Auto delete a pile
	if (pileItems.size() <= 1 && 
		!animManager->isObjAnimating(this) &&
		getPileType() == SoftPile)
	{
		breakPile();
		return;
	}

	if ((phCentre.distanceSquared(phLastPos) > 0.0f) || forceUpdate)
	{
		//TODO Renable animation of piles
		// Don't bother updating if there are animations present
		if (isAnimating())
		{
			BumpObject * obj = sel->getPickedActor();
			if (!obj || (obj && !obj->isBumpObjectType(BumpPile)))
			{
				return;
			}
		}

		// Sets the items in their proper places
		if (!phUnhinged)
		{
			if (!relPosToPhantomCentroid.empty() && !pileItems.empty())
			{
				assert(relPosToPhantomCentroid.size() == pileItems.size());
				for (uint i = 0; i < pileItems.size(); i++)
				{
					// This prevents the relPosToPhantomCentroid from auto-magically
					// adding an index when you try to access an out-of-bounds index
					if (i < relPosToPhantomCentroid.size())
					{
						Vec3 oldPos = pileItems[i]->getGlobalPosition();
						Vec3 newPos = getGlobalPositionForItem(relPosToPhantomCentroid[i]);
						if ((newPos - oldPos).magnitudeSquared() > 0.0f)
							pileItems[i]->setGlobalPosition(newPos);
					}
				}
			}
		}

		// Update the location of the close widget
		if (closeWidget)
		{
			updateCloseWidgetPos();
		}
	}

	// Save the position for comparison later
	phLastPos = actor->getGlobalPosition();
}

// Update the dimensions of the phantom actors based on the sizes of the items
// in the pile, and return an adjusted position based on those dims.
Vec3 Pile::updatePhantomActorDims()
{
	assert(!phUnhinged);
	assert(actor);

	Box box;
	Bounds pileBounds;
	Vec3 phDims;
	Vec3 phPos;

	pileBounds.setEmpty();
	pileBounds = getPileBounds();

	// Create the Bounds of the phantom Actor

	// Bounds returns full dims, we need half dims
	pileBounds.getExtents(phDims);
	pileBounds.getCenter(phPos);

	// Convert the dims from world space to actor space
	phDims = getGlobalOrientation() % phDims;
	phDims = Vec3(abs(phDims.x), abs(phDims.y), abs(phDims.z));

	phDims += getPileState() == Grid ? Vec3(6.5f, 10.0f, bufferZone) : Vec3(bufferZone, bufferZone, bufferZone);

	// Flatten the phantom actor so it is not at the mercy of the Novodex Flip Bug
	if (getPileState() == Grid)
		phDims.z = GLOBAL(settings).yDist;

	// Make sure the phantom actor is not below the floor (phDims.z is the actor thickness)
	if (phPos.y <= phDims.z)
		phPos.y = phDims.z + 0.1; // Add a small offset to avoid messed up physics

	// Set the Size and place of the phantom Actor
	setDims(phDims);

	// update the widgets
	if (closeWidget)
		updateCloseWidgetPos();

	// invalidate the text manager
	textManager->invalidate(true);

	return phPos;
}

bool Pile::breakPile()
{
	Mat34 newPose;
	Vec3 cent = getGlobalPosition();
	BumpObject *obj;
	cent.y = 0;

	// Animate to the messy Poses
	if (getPileState() == Stack || getPileState() == Leaf)
	{
		for (uint i = 0; i < pileItems.size(); i++)
		{
			obj = pileItems[i];

			// ensure that the centroid of the pile + the messy relative offset
			// is bounded by the desktop
			Box obb = obj->getBox();
			obb.center = cent + savedMessyPoses[obj].t;
			adjustBoxToInsideWalls(obb);
			newPose = Mat34(savedMessyPoses[obj].M, obb.center);

			obj->setPoseAnim(obj->getGlobalPose(), newPose, 25);
		}
	}

	setPileState(NoState);

	// Remove Items from the Pile
	QList<BumpObject *> newSelection;
	while (!pileItems.empty())
	{
		BumpObject * pile_element = pileItems[0];
		pile_element->addForceAtPos(Vec3(0, 10,0), Vec3(0.0f));
		removeFromPile(pile_element, NoUpdate);
		newSelection.append(pile_element);
	}
	sel->replace(newSelection);

	// Use the animation manager to self Delete
	animManager->addAnimation(AnimationEntry(this, (FinishedCallBack) DeletePileAfterAnim));

	return true;
}

void *stackAnimationCompleteCallback(AnimationEntry *entry)
{
	Pile *pile = (Pile *) entry->getObject();
	Vec3 cent = pile->getGlobalPosition(); 
	Vec3 dims = pile->getDims();
	Mat34 pose = pile->getGlobalPose();

	//XXX: Is this manual anim below necessary?  Best would be to re-awaken its physics and let it fall to the floor?  

	// Get the center of the pile
	if (pile->getNumItems() > 0)
		pile->getPileBounds().getCenter(cent);

	// Create an animation to animate to the floor
	pose.t.y = dims.z;
	pile->setEnforcePinning(true);
	pile->setPoseAnim(Mat34(pose.M, cent), pose, 10);
	pile->setEnforcePinning(false);
	
	// update the text visibility
	textManager->invalidate();

	return NULL;
}

bool Pile::stack(Vec3 stackLocation, bool reOrientActors)
{
	if (pileItems.empty())
		return false;

	Vec3 firstItemPos;
	float lhStep = 0.0f;
	bool useLogHeight;
	Vec3 newPos;
	Mat34List oldPose;

 	// Save the Messy State of all objects in the pile
 	if (getPileState() == NoState)
 	{
 		getPileBounds().getCenter(newPos);
 		newPos.y = 0;
 
		// Update messy positions so that they are all relative to the centroid of the phantom actor
 		for (uint i = 0; i < pileItems.size(); i++)
 			savedMessyPoses[pileItems[i]].t = pileItems[i]->getGlobalPosition() - newPos;
	}
	else if (Grid == getPileState())
		restoreDimsBeforeGrid();

	// If the pile does not have enough items in it to be a pile, break it
	if (pileItems.size() == 1 && getPileType() == SoftPile)
	{
		breakPile();
		return false;
	}
	
	// Set as a Stack
	setPileState(Stack);

	// Fade out the phantom actor if there is one
	setAlpha(0.0f);

	// Clear out the relative positions list and get the position of the first item
	relPosToPhantomCentroid.clear();	
	float yOffset = repoManager->isInPileSpace(this) ? pileItems.back()->getGlobalPosition().y : 0.0f;
	firstItemPos = Vec3(stackLocation.x, yOffset + pileItems.back()->getDims().z, stackLocation.z);

	// If the pile is too tall, use the Log Step Algorithm
	useLogHeight = getLogHeightSteps(lhStep);
	newPos = firstItemPos;

	// Index Structure:
	// +-------+
	// |   0   |
	// +-------+
	// |   1   |
	// +-------+
	// |   2   |
	// +-------+
	// |   3   |
	// +-------+
	// |   4   |
	// +-------+
	// Create the Vertically Stacked Pile
	srand((unsigned int) time(NULL));
	for (int i = pileItems.size() - 1; i >= 0; --i)
	{
		// disable all the items' text labels
		if (pileItems[i]->getNameableOverlay())
			pileItems[i]->getNameableOverlay()->getStyle().setVisible(false);

		pileItems[i]->killAnimation();

		// Set the item's location in the pile relative to the Phantom Actor Centroid
		relPosToPhantomCentroid[i] = Vec3(0, newPos.y, 0);
		oldPose.push_back(pileItems[i]->getGlobalPose());

		if (reOrientActors)
		{
			Mat33 newOri = GLOBAL(straightIconOri);
			int maxRotationLimits = NxMath::min(int(GLOBAL(settings).RotationLimitDegrees), 20);
			if (maxRotationLimits > 0)
				newOri *= Quat(float((rand() % maxRotationLimits) - (maxRotationLimits / 2.0f)), Vec3(0,0,1));
			pileItems[i]->setGlobalPose(Mat34(newOri, newPos));		
		}
		else
			pileItems[i]->setGlobalPosition(newPos);
		
		if (useLogHeight)
		{
			// Using the Log Algorithm, figure out the locations of the items
			newPos += Vec3(0, lhStep, 0);
		}
		else if (i > 0)
		{
			// Add up the dims of the next and previous items to get an accurate stack
			newPos.y += pileItems[i]->getDims().z + pileItems[i - 1]->getDims().z;
		}
	}

	// Figure out the center of the Phantom Actor and its size
	// Must be in the correct orientation before calling updatePhantomActorDims
	setGlobalOrientation(GLOBAL(straightIconOri));
	newPos = updatePhantomActorDims();
	setGlobalPosition(newPos);

	// Restore positions of the Items in the List
	Mat33 ori;
	for (uint i = 0; i < pileItems.size(); i++)
	{
		relPosToPhantomCentroid[i] = relPosToPhantomCentroid[i] - Vec3(0, newPos.y, 0);
		ori = pileItems[i]->getGlobalOrientation();
		pileItems[i]->setGlobalPose(oldPose[pileItems.size() - 1 - i]);
		pileItems[i]->setGlobalOrientation(ori);
	}

	// Set as Stacked and animate to the relative positions
	destroyPileWidgets();
	animateItemsToRelativePos();
	animManager->addAnimation(AnimationEntry(this, (FinishedCallBack)stackAnimationCompleteCallback));

	// Unpick internal items when being stacked
	for (uint i = 0; i < pileItems.size(); i++)
	{
		if (pileItems[i]->isSelected())
		{
			sel->remove(pileItems[i]);
		}
	}
	assert(relPosToPhantomCentroid.size() == pileItems.size());
	return true;
}

void *gridAnimationCompleteCallback(AnimationEntry *entry)
{
	Pile *pile = (Pile *) entry->getObject();
	pile->setCollisions(true);
	pile->setFrozen(false);
	pile->setRotation(false); // setting frozen to false also seems to disable frozen rotations on nx
	
	// If it's a regular horizontal grid, add it to the special "pile space"
	// which prevents it from colliding with other objects on the desktop
	if (pile->isHorizontal())
		repoManager->addToPileSpace(pile);

	// update the text visibility
	textManager->invalidate();

	return NULL;
}

void scaleDim(Vec3 & dim, float scale, const Vec3 & minDim)
{
	// scale the bigger dim, and base the smaller dim on aspect ratio
	float aspect = dim.x / dim.y;
	if (aspect < 1)
	{
		dim.y = NxMath::max(dim.y * scale, minDim.y);
		dim.x = dim.y * aspect;
	}
	else
	{
		dim.x = NxMath::max(dim.x * scale, minDim.x);
		dim.y = dim.x / aspect;
	}
}

bool Pile::grid(Vec3 gridCenter)
{
	const float gridSpacingX = 10.0f; // Horizontal spacing of the grid layout
	const float gridSpacingZ = 8.0f; // Vertical spacing of the grid layout

	if (Grid != getPileState())
	{
		itemDimsBeforeGrid.clear();
		savedStackPosition = getGlobalPosition();
	}
	
	const hash_map<BumpObject *, Vec3>::iterator & endIterator = itemDimsBeforeGrid.end();
	for (uint i = 0; i < pileItems.size(); i++)
	{
		const hash_map<BumpObject *, Vec3>::iterator & iterator = itemDimsBeforeGrid.find(pileItems[i]);
		if (endIterator != iterator)
		{
			Vec3 newDim = (*iterator).second;
			scaleDim(newDim, _gridItemsScale, pileItems[i]->getMinimumDims());

			// Item size changed while in grid, so change the itemDimsBeforeGrid
			if (!qFuzzyCompare(newDim.x, pileItems[i]->getDims().x)	|| !qFuzzyCompare(newDim.y, pileItems[i]->getDims().y))		
				(*iterator).second = pileItems[i]->getDims();
		}
		else  //Item was added to pile after gridding, so store it's original dim
			itemDimsBeforeGrid[pileItems[i]] = pileItems[i]->getDims();
	}
	

	Mat33 pileOrientation = getGlobalOrientation();

	if (!_onGridHandler.empty())
		_onGridHandler();

	if (pileItems.empty())
		return false;
	if (gridView.getVisibleContainer().empty())
		return false;

	// For vertical piles, sort by the creation time of the files.
	if (!isHorizontal())
		sort(pileItems.begin(), pileItems.end(), sortByCreationTime());

	// finish any existing animation first
	animManager->finishAnimation(this);

	// Set as Grid
	setPileState(Grid);

	// Clear the relative positions
	relPosToPhantomCentroid.clear();

	vector<BumpObject *> visibleItems = gridView.getVisibleContainer();
	int visibleItemsCount = visibleItems.size();
	int visibleItemsOffset = gridView.getVisibleItemOffsetIndex();

	int gridSizeX, gridSizeZ;
	if (!isHorizontal())
	{
		// XXX: Hack for sharing prototype + vertical grids
		// Force a single row containing no more than 6 items
		visibleItemsCount = min(visibleItemsCount, 6);
		gridSizeX = visibleItemsCount;
		gridSizeZ = 1;
	}
	else
	{
		// For horizontal grids, make the grid approximately square
		gridSizeX = int(ceil(sqrt(float(visibleItemsCount))));
		gridSizeZ = visibleItemsCount / gridSizeX + ((visibleItemsCount % gridSizeX > 0) ? 1 : 0);
	}

	// Find the largest dimensions in the pile
	Vec3 dims(0.0f, PILE_SLATE_THICKNESS, 0.0f);
	for (uint i = 0; i < pileItems.size(); i++)
	{
		Vec3 indvDims = itemDimsBeforeGrid[pileItems[i]];
		if (dims.x < indvDims.x) dims.x = indvDims.x;
		if (dims.z < indvDims.y) dims.z = indvDims.y;
	}

	// Check if need to scale down items to fit in desktop
	_gridItemsScale = 1;
	_gridItemsScale = NxMath::min((GetDesktopBox().GetExtents().x * 2) / (dims.x * 2 * gridSizeX + (gridSizeX * 2) * gridSpacingX + 80), _gridItemsScale);
	_gridItemsScale = NxMath::min((GetDesktopBox().GetExtents().z * 2) / (dims.z * 2 * gridSizeZ + (gridSizeZ * 2) * gridSpacingZ + 80), _gridItemsScale);
	dims.x *= _gridItemsScale;
	dims.z *= _gridItemsScale;

	// hide each of the items that aren't in the visible set
	for (int i = 0; i < pileItems.size(); ++i)
	{
		Vec3 newDim = itemDimsBeforeGrid[pileItems[i]];
		scaleDim(newDim, _gridItemsScale, pileItems[i]->getMinimumDims());
		pileItems[i]->setDims(newDim);

		if (visibleItemsOffset <= i && i < (visibleItemsOffset + visibleItemsCount))
		{
			if (dynamic_cast<Actor *>(pileItems[i]))
				dynamic_cast<Actor *>(pileItems[i])->popActorType(Invisible);

			// enable the text labels for the visible items
			if (pileItems[i]->getNameableOverlay())
				pileItems[i]->getNameableOverlay()->getStyle().setVisible(true);
		}
		else
		{
			if (dynamic_cast<Actor *>(pileItems[i]))
				dynamic_cast<Actor *>(pileItems[i])->pushActorType(Invisible);
			if (!pileItems[i]->isAnimating(AlphaAnim))
				pileItems[i]->setAlpha(0.0f);

			// disable the text labels for the hidden items
			if (pileItems[i]->getNameableOverlay())
				pileItems[i]->getNameableOverlay()->getStyle().setVisible(false);
		}
	}

	// Index Structure:
	// +---+  +---+  +---+
	// | 0 |  | 1 |  | 2 |
	// +---+  +---+  +---+
	//                    
	// +---+  +---+  +---+
	// | 3 |  | 4 |  | 5 |
	// +---+  +---+  +---+
	//                    
	// +---+  +---+  +---+
	// | 6 |  | 7 |  | 8 |
	// +---+  +---+  +---+
	// Find out where to start the grid and calculate outwards form the center
	// All of these calculations are done assuming that the grid is flat; they
	// will be re-oriented to match the true pile orientation after.

	Mat34List oldPose;

	setGlobalPosition(gridCenter);

	float fullDimsX = dims.x * 2;
	float fullDimsZ = dims.z * 2;
	float gridHalfWidth = (fullDimsX * gridSizeX + (gridSizeX - 1) * gridSpacingX) / 2.0f;
	float gridHalfHeight = (fullDimsZ * gridSizeZ + (gridSizeZ - 1) * gridSpacingZ) / 2.0f;
	
	// The starting point to draw the first object should be half the dimensions of
	// the largest item away from the top left corner of the grid. This is the center
	// point of the first object
	Vec3 startPt(gridHalfWidth - dims.x, 0, gridHalfHeight - dims.z);
	startPt.z += ceil((float)visibleItemsOffset / gridView.getColSize()) * (fullDimsZ + gridSpacingZ);

	// Move the Actors into their rightful places on the grid
	for (int i = 0; i < pileItems.size(); i++)
	{
		uint row = i / gridSizeX;
		uint col = i % gridSizeX;

		Vec3 pos = startPt;
		oldPose.push_back(pileItems[i]->getGlobalPose());

		// Set the new relative coordinates of the object
		pos.x -= col * (fullDimsX + gridSpacingX);
		pos.y += pileItems[i]->getDims().z + dims.y;
		pos.z -= row * (fullDimsZ + gridSpacingZ);
		
		// Store the relative coordinates of the object
		relPosToPhantomCentroid[i] = pos;
		
		Mat34 newPose(pileOrientation, getGlobalPositionForItem(relPosToPhantomCentroid[i]));
		pileItems[i]->setGlobalPose(newPose);
	}

	// create and update the widgets (so that the bounds can be calculated)
	createPileWidgets();
	syncScrollWidgets(); // must come BEFORE updateGridWidgets
	updateGridWidgets();

	// update the phantom actor size for the new item positions
	// and ensure that the phantom actor is within the desktop box
	updatePhantomActorDims();
	Box gridBox = getBox();
	Box desktopBox = GetDesktopBox();
	Box normalizedBox(desktopBox.center, gridBox.extents, gridBox.rot);
	if (normalizedBox.isInside(desktopBox))
	{
		if (adjustBoxToInsideWalls(gridBox))
		{
			setGlobalPosition(gridBox.center);

			// we need to re-update the grid widgets to their correct location
			updateGridWidgets();
		}
	}

	// Update the Phantom Actor size for the new item and widget positions
	updatePhantomActorDims();

	// revert the actors back to their original spots
	for (int i = 0; i < pileItems.size(); i++)
	{
		pileItems[i]->setGlobalPose(Mat34(getGlobalOrientation(), oldPose[i].t));
		pileItems[i]->setDims(itemDimsBeforeGrid[pileItems[i]]);
		pileItems[i]->setAlphaAnim(0.75f, 1.0f, 25);
	}

	// Set as a Gridded Pile, update Text and fade in the phantom Actor
	animateItemsToRelativePos();
	for (uint i = 0; i < pileItems.size(); i++)
	{
		Vec3 newDim = itemDimsBeforeGrid[pileItems[i]];
		scaleDim(newDim, _gridItemsScale, pileItems[i]->getMinimumDims());
		pileItems[i]->setSizeAnim(pileItems[i]->getDims(), newDim, (animationStepOverride > -1) ? animationStepOverride : 25);
	}
	setAlphaAnim(getAlpha(), 0.9f, 25);
	setFrozen(true);
	setCollisions(false);

	textManager->invalidate();
	animManager->removeAnimation(this);
	animManager->addAnimation(AnimationEntry(this, (FinishedCallBack)gridAnimationCompleteCallback));

	return true;
}

void Pile::restoreDimsBeforeGrid()
{
	_ASSERT(Grid == getPileState()); // Should only be called when exiting grid mode
	const hash_map<BumpObject *, Vec3>::iterator & endIterator = itemDimsBeforeGrid.end();
	for (uint i = 0; i < pileItems.size(); i++)
	{
		const hash_map<BumpObject *, Vec3>::iterator & iterator = itemDimsBeforeGrid.find(pileItems[i]);
		if (endIterator != iterator)
			pileItems[i]->setDims((*iterator).second);
		else
			_ASSERT(false);
	}
}

bool Pile::canScrollGridRow(int numRows)
{
	// determine the new visible index
	numRows = -numRows;
	vector<BumpObject *> visibleObjects = gridView.getVisibleContainer();
	const vector<BumpObject *>& allObjects = gridView.getContainer();
	int numItems = abs(numRows * gridView.getRowSize());
	int visibleIndex = gridView.getVisibleItemOffsetIndex();
	int visibleEndIndex = visibleIndex + visibleObjects.size();
	int newUnboundedVisIndex = visibleIndex - numRows * gridView.getRowSize();
	int newBoundedVisIndex = NxMath::min(NxMath::max(0, newUnboundedVisIndex), 
		NxMath::max(0, (gridView.getRowCount() - gridView.getRowSize()) * gridView.getRowSize()));

	// ensure actual scroll requested
	return (visibleIndex != newBoundedVisIndex);
}

bool Pile::scrollGridRow(int numRows, bool animateBeforeGridWorkaround)
{
	// determine the new visible index
	numRows = -numRows;
	vector<BumpObject *> visibleObjects = gridView.getVisibleContainer();
	const vector<BumpObject *>& allObjects = gridView.getContainer();
	int numItems = abs(numRows * gridView.getRowSize());
	int visibleIndex = gridView.getVisibleItemOffsetIndex();
	int visibleEndIndex = visibleIndex + visibleObjects.size();
	int newUnboundedVisIndex = visibleIndex - numRows * gridView.getRowSize();
	int newBoundedVisIndex = NxMath::min(NxMath::max(0, newUnboundedVisIndex), 
		NxMath::max(0, (gridView.getRowCount() - gridView.getRowSize()) * gridView.getRowSize()));

	// ensure actual scroll requested
	if (visibleIndex == newBoundedVisIndex) return false;

	// determine the objects that we will be fading away and in
	vector<BumpObject *> fadeAwayObjects;
	vector<BumpObject *> fadeInObjects;

	if (newBoundedVisIndex > visibleIndex)
	{		
		for (int i = visibleIndex; i < (visibleIndex + numItems); ++i)
			fadeAwayObjects.push_back(allObjects[i]);	
		for (int i = visibleEndIndex; i < NxMath::min((int)allObjects.size(), (visibleEndIndex + numItems)); ++i)
			fadeInObjects.push_back(allObjects[i]);
	}
	else
	{
		int size = (gridView.getRowSize() * gridView.getColSize());
		for (int i = newBoundedVisIndex + size; i < NxMath::min((int)allObjects.size(), (int)(newBoundedVisIndex + size + numItems)); ++i)
			fadeAwayObjects.push_back(allObjects[i]);	
		for (int i = newBoundedVisIndex; i < NxMath::min((int)allObjects.size(), (int)(newBoundedVisIndex + numItems)); ++i)
			fadeInObjects.push_back(allObjects[i]);		
	}

	if (animateBeforeGridWorkaround)
	{
		// finish existing animations
		for (int i = 0; i < allObjects.size(); ++i)
			animManager->finishAnimation(allObjects[i]);

		// fade in/away the items requested (needs to come after grid())
		for (int i = 0; i < fadeAwayObjects.size(); ++i)
		{
			if (dynamic_cast<Actor *>(fadeAwayObjects[i]))
				dynamic_cast<Actor *>(fadeAwayObjects[i])->pushActorType(Invisible);
			fadeAwayObjects[i]->setAlphaAnim(fadeAwayObjects[i]->getAlpha(), 0.0f, 10);
		}
		for (int i = 0; i < fadeInObjects.size(); ++i)
		{
			if (dynamic_cast<Actor *>(fadeInObjects[i]))
				dynamic_cast<Actor *>(fadeInObjects[i])->popActorType(Invisible);
			fadeInObjects[i]->setAlphaAnim(fadeInObjects[i]->getAlpha(), 1.0f, 10);
		}

		// re-grid the pile using the new visible indices
		animationStepOverride = 4;
		gridView.setVisibleItemOffsetIndex(newBoundedVisIndex);
		grid(getGlobalPosition());
		animationStepOverride = -1;
	}
	else
	{
		// re-grid the pile using the new visible indices
		animationStepOverride = 4;
		gridView.setVisibleItemOffsetIndex(newBoundedVisIndex);
		grid(getGlobalPosition());
		animationStepOverride = -1;

		// finish existing animations
		for (int i = 0; i < allObjects.size(); ++i)
			animManager->finishAnimation(allObjects[i]);

		// fade in/away the items requested (needs to come after grid())
		for (int i = 0; i < fadeAwayObjects.size(); ++i)
		{
			if (dynamic_cast<Actor *>(fadeAwayObjects[i]))
				dynamic_cast<Actor *>(fadeAwayObjects[i])->pushActorType(Invisible);
			fadeAwayObjects[i]->setAlphaAnim(fadeAwayObjects[i]->getAlpha(), 0.0f, 10);
		}
		for (int i = 0; i < fadeInObjects.size(); ++i)
		{
			if (dynamic_cast<Actor *>(fadeInObjects[i]))
				dynamic_cast<Actor *>(fadeInObjects[i])->popActorType(Invisible);
			fadeInObjects[i]->setAlphaAnim(fadeInObjects[i]->getAlpha(), 1.0f, 10);
		}
	}

	// sync the widgets if there are any
	syncScrollWidgets();

	// increment the stats
	statsManager->getStats().bt.interaction.piles.gridScrolled++;
	
	return true;
}

bool Pile::scrollToGridItem(BumpObject * obj)
{
	assert(getPileState() == Grid);

	// get the index of the object
	int objIndex = -1;
	for (int i = 0; i < pileItems.size(); ++i)
	{
		if (pileItems[i] == obj)
		{
			objIndex = i;
			break;
		}
	}
	if (objIndex < 0)
		return false;

	// check if it's visible already in the grid
	int rowSize = gridView.getRowSize();
	int colSize = gridView.getColSize();
	int index = gridView.getVisibleItemOffsetIndex();
	if (index <= objIndex && objIndex < (rowSize * colSize))
		return false;

	// if it's not visible, then scroll until it is
	int objIndexRow = objIndex / rowSize;
	int visOffsetRow = index / rowSize;
	int numRowsToScroll = (objIndexRow - visOffsetRow) + 1 - colSize;
	scrollGridRow(numRowsToScroll, true);

	return true;
}

void Pile::close()
{
	// User clicked on the close Widget
	if (closeWidget)
	{
		animManager->finishAnimation(this);
		
		// If it was Laid Out, we need to re-enable the Phantom actor
		if (getPileState() == LaidOut)
		{
			rehingePhantomActor();
		}

		// Stack the pile
		if (savedStackPosition.isNotUsed())
		{
			stack(getGlobalPosition());
		}
		else
		{
			stack(savedStackPosition);
			savedStackPosition.setNotUsed();
		}

		sel->clear();
		sel->add(this);
		destroyPileWidgets();
	}
	else
	{
		if (getPileState() == Leaf)
		{
			// close the leafed out pile/re-stack the pile
			stack(getGlobalPosition(), false);
		}
	}
}

void Pile::updateCloseWidgetPos()
{	
	Vec3 pos;
	BumpObject *obj = NULL;
	Box tempBox;
	bool onLeftSide = false;

	if (closeWidget && 
		(closeWidget->getAlpha() == 0.0f) && 
		!closeWidget->isAnimating(AlphaAnim))
	{
		closeWidget->setAlphaAnim(closeWidget->getAlpha(), 1.0f, 15);
	}

	if (closeWidget && pileItems.size() > 0)
	{
		if (getPileState() == Grid)
		{
			updateGridWidgets();
		}
		else 
		{
			if (getPileState() == Leaf)
			{
				// move the close widget to the side of the pile
				obj = pileItems.back();

				// use the left, leafed side of the pile for the close widget if the pile is on the 
				// right side of the screen, so that we can actually see it
				if (!pileItems.empty() && 
					pileItems.back()->getGlobalPosition().x < 0.0f)
					onLeftSide = true;
			}
			else if (getPileState() == LaidOut)
			{
				obj = pileItems.front();

				if (pileItems.size() > 1 && 
					obj->getGlobalPosition().x > pileItems[1]->getGlobalPosition().x)
				{
					onLeftSide = true;
				}
			}
			else
			{
				// Delete the Widget, in cases where you go directly from Grid to LayingOut
				destroyPileWidgets();
				return;
			}

			pos = obj->getGlobalPosition();
			pos.x -= obj->getDims().x + GLOBAL(settings).xDist;
			pos.y = NxMath::max(closeWidget->getDims().z, pos.y);

			tempBox.center = pos;
			tempBox.extents = closeWidget->getDims();
			tempBox.rot = Mat33(NX_IDENTITY_MATRIX);

			// See if the close widget is inside a wall
			if (adjustBoxToInsideWalls(tempBox) || onLeftSide)
			{
				if (getPileState() == Leaf)
				{	
					// determine the height of the stack
					float heightOfStack = 0.0f;		
					for (int j = 0; j < pileItems.size(); ++j)
					{
						heightOfStack += (2.0f * pileItems[j]->getDims().z);
					}

					Vec3 dims = closeWidget->getDims();
					obj = pileItems.back();
					pos = obj->getGlobalPosition();
					pos = Vec3(pos.x + heightOfStack, dims.z, pos.z);
				}
				else 
				{
					pos = obj->getGlobalPosition();
				}

				// Move the widget to the other side of the actor
				pos.x += obj->getDims().x + GLOBAL(settings).xDist;
			}

			// Apply the change to the Widget
			closeWidget->setGlobalPosition(pos);
		}
	}
}

hash_map<uint, Vec3> Pile::getRelPositions()
{
	return relPosToPhantomCentroid;
}

int Pile::isInPile(BumpObject *obj)
{
	// See if this item is inside the pile
	for (uint i = 0; i < pileItems.size(); i++)
	{
		if (pileItems[i] == obj) return i;
	}

	return -1;
}

void Pile::updateObject()
{
	if (!phUnhinged)
		updatePhantomActorDims();

	// if we are in leaf mode, we should update the leaf item if necessary
	if (getPileState() == Leaf)
	{
		leafUp();
		leafDown();
	}

	// After the Anim, update the pile
	if (isAnimating())
	{
		animManager->removeAnimation(this);
		animManager->addAnimation(AnimationEntry(this, (FinishedCallBack) updatePileStateAfterAnim));
	}

	// Call the base code
	BumpObject::updateObject();
}

void Pile::updatePileState()
{
	// Relayout
	if (getPileState() == Grid)
	{
		Vec3 newPos = getGlobalPosition();
		// If the grid is parallel to the floor, adjust its Z position to be
		// in the "grid plane", which is above other objects
		if (isHorizontal())
			newPos.y = repoManager->getPlaneFloorLevel() + getDims().z;
		grid(newPos);
	}

	// NOTE: even if we are animating, we have to stack again when new items are
	// 		 added to the stacked pile
	if ((!isAnimating() || (pileItems.size() != relPosToPhantomCentroid.size()))
	&& (getPileState() == Stack))
	{
		stack(getGlobalPosition(), false);
	}
}

void Pile::onDropEnter( vector<BumpObject *> &objList )
{
	Vec3 size;
	BumpObject *obj = sel->getPickedActor();

	// Call the Parent
	DropObject::onDropEnter(objList);

	if (isSourceValid())
	{
		// Create a playful bounce
		finishAnimation();

		// Shrink the items hovering over this item
		for (uint i = 0; i < source.size(); i++)
		{
			if (source[i]->isBumpObjectType(BumpActor))
			{
				size = source[i]->getDims();

				if (size.x > size.z) size *= NxMath::min((getDimsOfLargestPileItem().x) / size.x, 2.0f);
				if (size.x < size.z) size *= NxMath::min((getDimsOfLargestPileItem().z) / size.z, 2.0f);

				source[i]->finishAnimation();
				source[i]->setSizeAnim(source[i]->getDims(), size, 15);
			}
		}

		updateDropCursor(objList);
	}
	else
	{
		// Exit the drop if its invalid
		DropObject::onDropExit();
	}
}

vector<BumpObject *> Pile::onDrop(vector<BumpObject *> &objList)
{
	vector<BumpObject *> failedObj;
	vector<BumpObject *> pileItems;

	for (uint i = 0; i < objList.size(); i++)
	{
		// Loop through and add all items to the pile
		if (objList[i]->isPilable(getPileType()))
		{
			// bound the object dimension to the largest item in the pile 
			Vec3 dims = objList[i]->stateBeforeDrag().dims;
			Vec3 pileDims = getDimsOfLargestPileItem();
			if (dims.x > dims.y) 
			{
				float aspect = dims.y / dims.x;
				dims.x = NxMath::min(dims.x, pileDims.x);
				dims.y = aspect * dims.x;
			}
			else
			{
				float aspect = dims.x / dims.y;
				dims.y = NxMath::min(dims.y, pileDims.y);
				dims.x = aspect * dims.y;
			}
			animManager->finishAnimation(objList[i]);
			objList[i]->stateBeforeDrag().dims = dims;
			objList[i]->setDims(dims);

			sel->remove(objList[i]);
			addToPile(objList[i]);
			if (objList[i]->getObjectType() == ObjectType(BumpActor))
				objList[i]->setFreshnessAlphaAnim(1.0f, 80);
		}else{
			failedObj.push_back(objList[i]);
		}
	}

	return failedObj;
}

QString Pile::resolveDropOperationString(vector<BumpObject *>& objList)
{
	return QT_TR_NOOP("Move into pile");
}

bool Pile::isValidDropTarget()
{
	switch (getPileState())
	{
	case LayingOut:
	case LaidOut:
		return false;
	default:
		return true;
	}
}

const vector<BumpObject *>& Pile::getPileItems()
{
	return pileItems;
}

bool Pile::isSourceValid()
{
	for (uint i = 0; i < source.size(); i++)
	{
		// Check to see if the icon is a filesystem icon
		if (source[i]->isBumpObjectType(BumpPile))
		{
			// NOTE: don't allow dragging gridded piles onto other things
			Pile * p = (Pile *) source[i];
			if (p->getPileType() == SoftPile)
				return (p->getPileState() != Grid);

			// If there is a hard pile in the selection, don't add it
			return false;
		}else if (source[i]->getObjectType() == ObjectType(BumpActor, Temporary))
		{
			// Don't allow temporary items to be dropped in
			return false;
		}
	}

	return true;
}

void Pile::clear(int indx)
{
	// -------------------------------------------------------------------------
	// NOTE: This removes items from the pile much like removeFromPile() does 
	//       but it does not preform any cleanup work that might happen. This 
	//       is mainly done to remove items form the pile that are being 
	//       deleted from the scene. To remove items properly, use the
	//       removeFromPile() function.
	// -------------------------------------------------------------------------
	if (indx == -1)
	{
		// Clear the items form this pile and move them to the parent
		for (uint i = 0; i < pileItems.size(); i++)
		{
			pileItems[i]->setParent(getParent());
		}

		pileItems.clear();
	}else{
		if (indx >= 0 && indx < pileItems.size())
		{
			BumpObject *obj = pileItems[indx];

			// Selectively clear an item from the pile
			obj->setParent(getParent());
			pileItems.erase(pileItems.begin() + indx);

			// re-stack the pile if we were leafing
			if (getPileState() == Leaf)
				stack(getGlobalPosition());
		}
	}
}

void Pile::onRender(uint flags)
{
	vector<BumpObject *> sortedPileItems = pileItems;
#ifdef DXRENDER
	dxr->device->SetRenderState(D3DRS_ZENABLE, false);
	dxr->device->SetRenderState(D3DRS_ZWRITEENABLE, false);
#else
	glPushAttribToken token(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT);
	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
#endif

	// Render the phantom actor if the alpha is enabled
	if (getAlpha() > 0.0)
	{
#ifdef DXRENDER
		if (getAlpha() != dxr->textureMaterial.Diffuse.a)
		{
			dxr->textureMaterial.Diffuse.a = dxr->textureMaterial.Ambient.a = getAlpha();
			dxr->device->SetMaterial(&dxr->textureMaterial);
		}
		dxr->renderSideLessBox(getGlobalPosition(), getGlobalOrientation(), getDims(), texMgr->getGLTextureId("pile.background"));
#else
		// Set the Texture
		glBindTexture(GL_TEXTURE_2D, texMgr->getGLTextureId("pile.background"));
		glColor4f(1,1,1,getAlpha());

		// Render phantom actor (Ignore the flags)
		ShapeVis::renderSideLessBox(getGlobalPosition(), getGlobalOrientation(), getDims());
#endif
	}

	// Sort the Pile Items by height so they will be rendered properly
	sort(sortedPileItems.begin(), sortedPileItems.end(), SortByActorHeight());
	
	// if we are leafing, then we want to dim the non-leafed actors
	if (getPileState() == Leaf)
	{			
		for (uint i = 0; i < sortedPileItems.size(); i++)
		{
			// render non-leafed items a little darker
			if (sortedPileItems[i]->isObjectType(BumpActor))
			{
				Actor * actor = (Actor *) sortedPileItems[i];
				if (actor != getActiveLeafItem())
				{
					actor->enableMaterialColorOverride(QColor::fromRgbF(0.45f, 0.45f, 0.45f));
					actor->onRender(RenderSideless);
					actor->disableMaterialColorOverride();
					continue;
				}
			}

			// otherwise, just render normally
			sortedPileItems[i]->onRender(RenderSideless);
		}
	}
	else 
 	{
		// just render each item 
		for (uint i = 0; i < sortedPileItems.size(); i++)
			sortedPileItems[i]->onRender(RenderSideless);
 	}

	// Render the close Widget
	if (closeWidget && !closeWidget->isActorType(Invisible)) closeWidget->onRender(RenderSideless | RenderIgnoreDepth);
	if (launchOwnerWidget && !launchOwnerWidget->isActorType(Invisible)) launchOwnerWidget->onRender(RenderSideless | RenderIgnoreDepth);
	if (prevPageWidget && !prevPageWidget->isActorType(Invisible)) prevPageWidget->onRender(RenderSideless | RenderIgnoreDepth);
	if (nextPageWidget && !nextPageWidget->isActorType(Invisible)) nextPageWidget->onRender(RenderSideless | RenderIgnoreDepth);
	if (launchExternalWidget && !launchExternalWidget->isActorType(Invisible)) launchExternalWidget->onRender(RenderSideless | RenderIgnoreDepth);

#ifdef DXRENDER
	dxr->device->SetRenderState(D3DRS_ZENABLE, true);
	dxr->device->SetRenderState(D3DRS_ZWRITEENABLE, true);
#endif
}

void Pile::setPileState(PileState pState)
{
	if (pState != Grid) repoManager->removeFromPileSpace(this);
	if (getPileState() == Grid)
	{
		// was previously a grid, so we should revert the visibility of all items
		Vec3 pos = getGlobalPosition();
		for (int i = 0; i < pileItems.size(); ++i)
		{
			Actor * actor = dynamic_cast<Actor *>(pileItems[i]);
			if (actor)
			{
				if (actor->isActorType(Invisible))
					actor->setGlobalPosition(pos);
				actor->popActorType(Invisible);
				actor->setAlpha(1.0f);
			}
		}
		textManager->invalidate(false);
	}

	if (pState != Leaf)
	{
		leafIndex = -1;
	}

	type.ternaryType = pState;
}

void Pile::onTossRecieve(vector<BumpObject *> tossedObjs)
{
	vector<BumpObject *> failedObj;

	// Add items to the pile
	failedObj = onDrop(tossedObjs);

	// Animate back to the original starting pose
	animateObjectsBackToPreDropPose(failedObj);
}

bool Pile::isValidTossTarget()
{
	return isValidDropTarget();
}

bool Pile::isValidToss(vector<BumpObject *> tossedObjs)
{
	// Soft piles can receive all items except for non-stacked piles
	bool isValid = true;
	for (int i = 0; i < tossedObjs.size() && isValid; ++i)
	{
		BumpObject * obj = tossedObjs[i];
		if (obj->getObjectType() == ObjectType(BumpPile))
		{
			Pile * p = (Pile *) obj;
			isValid = (p->getPileState() == Stack);
		}
	}
	return isValid;
}

void Pile::onDragBegin(FinishedDragCallBack func)
{
	// begin drag on all child items as well
	for (int i = 0; i < pileItems.size(); ++i)
		pileItems[i]->onDragBegin();
	
	// begin drag on self
	BumpObject::onDragBegin(func);
}

void Pile::onDragEnd()
{
	// end drag on all child items as well
	for (int i = 0; i < pileItems.size(); ++i)
		pileItems[i]->onDragEnd();
	
	// end drag on self
	BumpObject::onDragEnd();
}

BumpObject * Pile::leafItemHitTest(int& indexOut)
{
	// only do this if we have items leafed
	if ((getPileState() == Leaf) && (leafIndex >= 0) && (leafIndex < pileItems.size()))
	{
		// Cast the coordinates into space and figure out the Ray

		Vec3 lineStart, lineEnd, dir;
		window2world(GLOBAL(mx), GLOBAL(my), lineStart, lineEnd);
		dir = lineEnd - lineStart;
		dir.normalize();
		Ray ray = Ray(lineStart, dir);

		// get the leafed object info
		for (int i = 0; i < pileItems.size(); ++i)
		{
			Mat33 ori = pileItems[i]->getGlobalOrientation();
			Vec3 dims = pileItems[i]->getDims();
			Vec3 cent = pileItems[i]->getGlobalPosition();

			// Check if the ray is intersecting the box
			if (NxRayOBBIntersect(ray, cent, dims, ori))
			{
				indexOut = i;
				return pileItems[i];
			}
		}
	}
	return NULL;
}

void Pile::onLaunch()
{
	switch (getPileState())
	{
	case Leaf:
	case Stack:
		// expand the stacked pile to a grid
		{
			close();
			Vec3 pos = getGlobalPosition();
			pos.y = repoManager->getPlaneFloorLevel() + PILE_SLATE_THICKNESS + PILE_SLATE_SPACE_ADJUSTMENT;
			grid(pos);
			repoManager->update();
			sel->clear();
			sel->setPickedActor(this);
		}
		break;
	case Grid:
		// close the gridded pile
		close();
		sel->clear();
		sel->setPickedActor(this);
	default:
		break;
	}
}

void Pile::onWidgetClick(BumpObject * widget)
{
	if (widget == closeWidget)
	{
		if (!_onGridCloseHandler.empty())
			_onGridCloseHandler();
		close();
	}
	else if (widget == nextPageWidget)
	{
		assert(getPileState() == Grid);
		scrollGridRow(1);
	}
	else if (widget == prevPageWidget)
	{
		assert(getPileState() == Grid);
		scrollGridRow(-1);
	}
	else if (widget == launchOwnerWidget)
	{}
}

void Pile::setShuffleGroup(vector<BumpObject *> shufGroup)
{
	shuffleGroup = shufGroup;
}

vector<BumpObject *> Pile::getShuffleGroup()
{
	return shuffleGroup;
}

void Pile::clearShuffleGroup()
{
	shuffleGroup.clear();
}

void Pile::insertShuffleGroup(uint indx)
{
	BumpObject *keyObj;
	bool last = false, foundSpot = false;
	vector<BumpObject *> removedItems;

	// Check if the Index is appropriate
	if (!shuffleGroup.empty())
	{
		// Check if the item can be moved to the last place
		if (indx >= pileItems.size())
		{
			last = true;
		}else{
			keyObj = pileItems[indx];

			// If our key object is in the shuffle group, try the next item in the group
			for (int i = 0; !last && i < shuffleGroup.size(); i++)
			{
				Actor *actor = (Actor *) shuffleGroup[i];

				if (keyObj == actor->getObjectToMimic())
				{
					// Grab the next item in the list
					indx++;
					keyObj = (pileItems.size() > indx) ? pileItems[indx] : NULL;
					i = -1;

					// If we cannot put it before an item, plop it in last place
					if (keyObj == NULL)
					{
						last = true;
					}
				}
			}
		}

		// Remove the shuffled items from the pileItems
		for (int i = 0; i < pileItems.size(); i++)
		{
			for (int j = 0; j < shuffleGroup.size(); j++)
			{
				Actor *actor = (Actor *) shuffleGroup[j];

				if (pileItems[i] == actor->getObjectToMimic())
				{
					pileItems.erase(pileItems.begin() + i);
					removedItems.push_back(actor->getObjectToMimic());
					i--;
					j = shuffleGroup.size();
				}
			}
		}

		if (!last)
		{
			// Find the new Index with a modified pile List
			for (int j = 0; j < pileItems.size(); j++)
			{
				if (pileItems[j] == keyObj)
				{
					indx = j;
					j = pileItems.size();
				}
			}
		}

		if (!last)
		{
			pileItems.insert(pileItems.begin() + indx, removedItems.begin(), removedItems.end());
		}else{
			// Stick the items into the pileItems list
			for (int i = removedItems.size() - 1; i >= 0; i--)
			{
				pileItems.push_back(removedItems[-i + removedItems.size() - 1]);
			}
		}

		// Relayout
		updatePileState();

		// animation-wise, we finish the animations for the mimic actors, and just fade them in instead
		for (int i = 0; i < shuffleGroup.size(); ++i)
		{
			BumpObject * obj = ((Actor *) shuffleGroup[i])->getObjectToMimic();
			if (obj)
			{
				animManager->finishAnimation(obj);	// finish the animation to move it to it's final place
				obj->setAlphaAnim(0.0f, 1.0f, 25);
			}
		}
		shuffleGroup.clear();
	}
}

void Pile::sortByName()
{
	sort(pileItems.begin(), pileItems.end(), SortByActorName());
	animateItemsToRelativePos();
}

void Pile::setOnGridHandler( boost::function<void()> onGridHandler )
{
	_onGridHandler = onGridHandler;
}

void Pile::setOnGridCloseHandler( boost::function<void()> onGridCloseHandler )
{
	_onGridCloseHandler = onGridCloseHandler;
}	

Vec3 Pile::getDimsOfLargestPileItem()
{
	Vec3 dims(0.0f);
	for (int i = 0; i < pileItems.size(); ++i)
	{
		dims.max(pileItems[i]->getDims());
	}
	return dims;
}

const ContainerView<BumpObject *>& Pile::getGridView()
{
	return gridView;
}

// Update the reference dimensions of all the pile items with their current dimensions.
void Pile::updateReferenceDims()
{
	for (int i = 0; i < pileItems.size(); ++i)
	{
		pileItems[i]->updateReferenceDims();
	}
}

void Pile::onBreakPin()
{
	BumpObject::onBreakPin();

	// Vertical grids are not in the pile space, but we want to add them back
	// when they are reoriented
	if (getPileState() == Grid)
	{
		if (!repoManager->isInPileSpace(this))
			repoManager->addToPileSpace(this);

		// Revert the orientation of all actors
		setGlobalOrientation(GLOBAL(straightIconOri));
		for (int i = 0; i < pileItems.size(); ++i)
		{
			pileItems[i]->setGlobalOrientation(GLOBAL(straightIconOri));
		}

		finishAnimation();
	}
	
	// force adjust to desktop box now
	repoManager->adjustToDesktopBox(this);
}

bool Pile::shouldRenderText()
{
	if (getPileState() != Stack)
		return false;
	return BumpObject::shouldRenderText();
}

// Given an item's position that's relative to the phantom actor centroid,
// return the global position for the item, taking into account the
// orientation of the phantom actor.
// The relativePosition is always assumes that the phantom actor is in 
// straightIconOri, i.e. parallel to the floor.
Vec3 Pile::getGlobalPositionForItem(Vec3 relativePosition )
{
	// First, we translate the vector from straightIconOri to an actor-space vector
	Vec3 actorRelativeVec = GLOBAL(straightIconOri) % relativePosition;

	// Then, translate based on the orientation of the phantom actor
	return getGlobalPosition() + (getGlobalOrientation() * actorRelativeVec);
}

bool Pile::isHorizontal()
{
	return isParallel(getGlobalOrientation(), GLOBAL(straightIconOri));
}

bool Pile::serializeToPb(PbBumpObject * pbPile)
{
	struct SavedActorData
	{
		unsigned int actorType;
		Vec3 actorDim;
	};

	assert(pbPile);
	
	// PB_TODO: fix this so that we load and save in the same views as before
	// and don't have to write the pile centroid

	// NOTE: temporarily unmark the actors as invisible (for example, when the
	//		 actors are in a gridded pile)
	QMap<BumpObject *, SavedActorData> savedActorData;
	for (int i = 0; i < pileItems.size(); ++i)
	{
		BumpObject * object = pileItems[i];
		SavedActorData originalActorData;
		originalActorData.actorDim = object->getDims();
		if (object->isObjectType(ObjectType(BumpActor)))
		{
			Actor * actor = (Actor *) object;
			originalActorData.actorType = actor->getActorType();
			actor->popActorType(Invisible);
		}
		if (Grid == getPileState())
		{
			_ASSERT(itemDimsBeforeGrid.end() != itemDimsBeforeGrid.find(pileItems[i]));
			object->setDims(itemDimsBeforeGrid[object]);
		}
		savedActorData.insert(object, originalActorData);
	}

	// serialize the core bump object properties first
	if (!BumpObject::serializeToPb(pbPile))
		return false;

	// write the children
	for (int i = pileItems.size() - 1; i >= 0 ; --i)
	{
		PbPile_PbPileItem * item = pbPile->AddExtension(PbPile::children);
		if (!pileItems[i]->serializeToPb(item->mutable_item()))
			return false;

		// write the objects original pose
		assert(savedMessyPoses.find(pileItems[i]) != savedMessyPoses.end());
		const Mat34& originalPose = savedMessyPoses[pileItems[i]];
		PbMat34 * pose = item->mutable_original_world_pose();
		Quat q;
		originalPose.M.toQuat(q);
		toPbQuat(q, pose->mutable_quat());
		toPbVec3(originalPose.t, pose->mutable_t());
	}

	// NOTE: restore the actors' types
	QMapIterator<BumpObject *, SavedActorData> iter(savedActorData);
	while (iter.hasNext())
	{
		iter.next();
		if (iter.key())
		{
			if (Grid == getPileState())
				iter.key()->setDims(iter.value().actorDim);
			if (dynamic_cast<Actor *>(iter.key()))
				((Actor *)iter.key())->setActorType(iter.value().actorType);
		}
	}
	return pbPile->IsInitialized();
}

bool Pile::deserializeFromPb(const PbBumpObject * pbPile)
{
	assert(pbPile);
	
	// PB_TODO: fix this so that we load and save in the same views as before
	// and don't have to write the pile centroid

	// deserialize the core bump object properties first
	if (!BumpObject::deserializeFromPb(pbPile))
		return false;

	// NOTE: we don't want the new pile-type since we are stacking initially
	unsigned int tmpPileState = type.ternaryType;
	type.ternaryType = NoState;

	// read the pile children
	int numChildren = pbPile->ExtensionSize(PbPile::children);
	float yOffset = 0.0f;
	Vec3 posTmp;
	for (int i = 0; i < numChildren; ++i)
	{
		const PbPile_PbPileItem& pbChild = pbPile->GetExtension(PbPile::children, i);
		BumpObject * child = PbPersistenceManager::getInstance()->deserializeBumpObjectFromPb(&pbChild.item());

		// NOTE: we no longer set the original positions of the pile actors
		//		 after reloading
		yOffset += child->getDims().z + 0.5f /* buffer */;
		posTmp = child->getGlobalPosition();
		child->setGlobalPosition(Vec3(posTmp.x,yOffset,posTmp.z));
		addToPile(child);
		yOffset += child->getDims().z;
	}

	// NOTE: fail if there are no items in the pile
	if (getNumItems() <= 0)
		return false;

	// PB_TODO: fix this so that we load and save in the same views as before
	// and don't have to write the pile centroid
	// PB_TEMP: otherwise, stack in-place, and update the phantom actor dims after
	//			the children have been stacked
	switch (tmpPileState)
	{
	case Grid:
		grid(getGlobalPosition());
		break;
	default:
		stack(getGlobalPosition());
		break;
	}
	animManager->finishAnimation(this);
	updatePhantomActorDims();	// PB_TEMP: NECESSARY?!?!

	return true;
}