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
#include "BT_AnimationEntry.h"
#include "BT_AnimationManager.h"
#include "BT_Authorization.h"
#include "BT_BumpObject.h"
#include "BT_OverlayComponent.h"
#include "BT_OverlayComponent.h"
#include "BT_PbPersistenceHelpers.h"
#include "BT_Pile.h"
#include "BT_Rename.h"
#include "BT_RepositionManager.h"
#include "BT_SceneManager.h"
#include "BT_Selection.h"
#include "BT_StatsManager.h"
#include "BT_TextManager.h"
#include "BT_UndoStack.h"
#include "BT_Util.h"
#include "BT_WidgetManager.h"
#include "BT_WindowsOS.h"
#include "BumpTop.h"
#include "BumpTop.pb.h"

BumpObject::BumpObject()
: pinJoint(NULL)
{
	// Init
	parent = NULL;
	selected = false;
	pinned = false;
	pinWall = NULL;
	enforcePinning = false;
	_ignoreSizeUpdate = false;
	setAlpha(1.0f);
	
	// Set a default Size for the actor
	setDims(Vec3(1, 1, 1));
	updateReferenceDims();

	// Register this class with the Scene
	scnManager->addObject(this);
	userDataType = UserDataAvailable;
	renderType = WorldSpace;

	// register the mouse event handler with the name overlay
	getNameableOverlay()->addMouseEventHandler(this);
}

BumpObject::~BumpObject()
{
	Pile *pile;
	
	//Let the renamer know the object has been deleted.
	Renamer->deleted(this);

	// Delete Actor from the scene
	scnManager->removeObject(this);	// this must happen first, as deselecting will try and update the text with this pile
	sel->remove(this);

	// unregister the mouse event handler with the name overlay
	getNameableOverlay()->removeMouseEventHandler(this);

	// NOTE: this does not occur from filesystem actors, since the Actor's
	// destructor contains pile clean up code as well
	if (isParentType(BumpPile))
	{
		pile = (Pile *) parent;
		if (pile->isInPile(this) > -1)
			pile->removeFromPile(this);
	}

	// REFACTOR: once we get rid of the clunky Actor class and the reliance on 
	//           Manually Deleting NxActors rather then BumpObjects, we can remove
	//           The following line in place of something cleaner (ie. releaseToActorPool() code)
	DeleteActor(this);

	// update the shell extension status bar if necessary
	if (scnManager->isShellExtension)
		winOS->ShellExtUpdateStatusBar();
}


Vec3 BumpObject::getDefaultDims()
{
	Vec3 dims(GLOBAL(settings).xDist, GLOBAL(settings).zDist, GLOBAL(settings).yDist);
	dims *= scnManager->defaultActorGrowFactor;
	return dims;
}

void BumpObject::syncNameableOverlayToDims()
{
	if (getNameableOverlay())
	{
		// find the new max dimensions of the nameable label			
		int halfWidth = getDims().x;
		Vec3 negPoint(-halfWidth, 0, 0);
		Vec3 posPoint(halfWidth, 0, 0);
		int x1 = 0, y1 = 0, x2 = 0, y2 = 0;
		world2window(negPoint, x1, y1);
		world2window(posPoint, x2, y2);
		int screenWidth = NxMath::max(abs(x2-x1), abs(x1-x2));
		int defaultTextWidth = getNameableOverlay()->getDefaultTextWidth();
		screenWidth = int(NxMath::max(defaultTextWidth, int(screenWidth * 0.8f)));	// 80% of the width to a min of 60px
		getNameableOverlay()->getTextOverlay()->getTextBuffer().setMaxBounds(QSize(screenWidth, 0));
		getNameableOverlay()->getTextOverlay()->getTextBuffer().update();
		textManager->invalidate();
	}
}

void BumpObject::setDims(const Vec3 &s)
{
	NxActorWrapper::setDims(s);

	if (!_ignoreSizeUpdate)
		syncNameableOverlayToDims();		
}

bool BumpObject::isPilable(uint pileType)
{
	return false;
}

void BumpObject::onAnimTick()
{
	Vec3 size;
	Mat34 pose;

	if (alphaAnim.size() > 0)
	{
		// Animate Alpha
		setAlpha(alphaAnim.front());
		alphaAnim.pop_front();
	}

	if (poseAnim.size() > 0)
	{
		pose = poseAnim.front();
		poseAnim.pop_front();

		// Animate Poses
		setGlobalPose(pose);
	}

	if (sizeAnim.size() > 0)
	{
		size = sizeAnim.front();
		sizeAnim.pop_front();

		// Animate Sizes
		bool oldUpdateState = _ignoreSizeUpdate;
		_ignoreSizeUpdate = sizeAnim.size() > 0;
		setDims(size);
		_ignoreSizeUpdate = oldUpdateState;
	}

	if (freshnessAlphaAnim.size() > 0)
	{
		freshnessAlphaAnim.pop_front();
	}
}

void BumpObject::finishAnimation()
{
	// Jump to the end of each animation
	if (!alphaAnim.empty()) setAlpha(alphaAnim.back());
	if (!poseAnim.empty()) setGlobalPose(poseAnim.back());
	if (!sizeAnim.empty()) setDims(sizeAnim.back());
	
	alphaAnim.clear();
	poseAnim.clear();
	sizeAnim.clear();
	freshnessAlphaAnim.clear();
}

void BumpObject::killAnimation()
{
	alphaAnim.clear();
	poseAnim.clear();
	sizeAnim.clear();
	freshnessAlphaAnim.clear();

	// Remove ourself from the animation queue
	animManager->removeAnimation(this);

	setAnimationState(NoAnim);
}

void BumpObject::setPoseAnim(Mat34 &startPose, Mat34 &endPose, uint steps, FinishedCallBack callback, void *customData)
{
	Quat startOri, endOri;
	deque<Mat34> poses;

	// Create the anim
	poseAnim.clear();
	poseAnim = lerpMat34RangeRaw(startPose, endPose, steps, SoftEase);

	// Finish animations at the end Pose given to eliminate rounding errors
	poseAnim.push_back(endPose);
	animManager->addAnimation(AnimationEntry(this, callback, customData, true));

	// Break the pin if this item is involved in an animation
	if (!enforcePinning)
		breakPin();
}

void BumpObject::setPoseAnim(deque<Mat34> &customPoseAnim, FinishedCallBack callback, void * customData)
{
	poseAnim = customPoseAnim;
	animManager->addAnimation(AnimationEntry(this, callback, customData, true));

	// Break the pin if this item is involved in an animation
	if (!enforcePinning)
		breakPin();
}

void BumpObject::setPoseAnim(deque<Mat34> &customPoseAnim)
{
	poseAnim = customPoseAnim;
	animManager->addAnimation(AnimationEntry(this, NULL, NULL, true));

	// Break the pin if this item is involved in an animation
	if (!enforcePinning)
		breakPin();
}

Mat34 BumpObject::getAnimFinalPose()
{
	if (poseAnim.size() > 0)
		return poseAnim.back();
	return getGlobalPose();
}

Vec3 BumpObject::getAnimFinalDims()
{
	if (sizeAnim.size() > 0)
		return sizeAnim.back();
	return getDims();
}

void BumpObject::setSizeAnim(Vec3 &startSize, Vec3 &endSize, uint steps)
{
	sizeAnim.clear();

	// Create a nice Bounce animation
	sizeAnim = bounceGrow(startSize, endSize, steps);

	// Add to the animation manager
	if (isObjectType(ObjectType(BumpActor, FileSystem, StickyNote)))
		animManager->addAnimation(AnimationEntry(this, (FinishedCallBack) SyncStickyNoteAfterResize, NULL, true));
	else
		animManager->addAnimation(AnimationEntry(this, NULL, NULL, true));
}

void BumpObject::setSizeAnim(deque<Vec3> &customSizeAnim)
{
	sizeAnim = customSizeAnim;

	// add to the animation manager
	if (isObjectType(ObjectType(BumpActor, FileSystem, StickyNote)))
		animManager->addAnimation(AnimationEntry(this, (FinishedCallBack) SyncStickyNoteAfterResize, NULL, true));
	else
		animManager->addAnimation(AnimationEntry(this, NULL, NULL, true));
}

void BumpObject::setAlphaAnim(float startAlpha, float endAlpha, uint steps)
{
	// if they are equal, then just set the stepts to 1 so that we still call
	// the animation finished callback
	if (NxMath::equals(endAlpha, startAlpha, 0.005f))
		steps = 1;

	float tickAmount = (endAlpha - startAlpha) / steps;

	// Clear the alpha animation if there is one
	alphaAnim.clear();
	for (uint i = 0; i < steps - 1; i++)
	{
		alphaAnim.push_back(startAlpha + (tickAmount * i));
	}

	// Last amount is the max amount for this run
	alphaAnim.push_back(endAlpha);
	animManager->addAnimation(AnimationEntry(this, NULL, NULL, true));
}

void BumpObject::setFreshnessAlphaAnim(float initialAlpha, uint steps)
{

	// apply to parent if it's stacked
	Pile * p = dynamic_cast<Pile *>(getParent());
	if (p && (p->getPileState() == Stack))
		p->setFreshnessAlphaAnim(initialAlpha, steps);
	else
	{
		float tickAmount = initialAlpha / steps;

		freshnessAlphaAnim.clear();
		for (uint i = 0; i < steps - 1; ++i)
			freshnessAlphaAnim.push_back(initialAlpha - (tickAmount * i));
		freshnessAlphaAnim.push_back(0.0f);
		animManager->addAnimation(AnimationEntry(this, NULL, NULL, true));
	}
}

Vec3 BumpObject::getMaximumDims()
{
	Vec3 szY = GLOBAL(WallsPos)[1] - GLOBAL(WallsPos)[0];
	Vec3 szX = GLOBAL(WallsPos)[2] - GLOBAL(WallsPos)[3];

	// Include a bit of buffer so it doesn't collide with walls
	return Vec3(szX.magnitude() / 2.2f, szY.magnitude() / 2.2f, 0.0f);
}

Vec3 BumpObject::getMinimumDims()
{
	// XXX: actor dims issue with different coordinate systems
	return Vec3(GLOBAL(settings).xDist * 0.8f, GLOBAL(settings).zDist * 0.8f, 0.0f);
}


void BumpObject::grow(uint numTicks, float growFactor)
{
	if (!_onGrowHandler.empty())
		_onGrowHandler();

	Vec3 maxDims = getMaximumDims();
	
	// CLear the animation
	sizeAnim.clear();

	if (getDims().x * growFactor < maxDims.x && getDims().y * growFactor < maxDims.y)
	{
		Vec3 newDims(getDims());
		newDims.x *= growFactor;
		newDims.y *= growFactor;

		if (isObjectType(ObjectType(BumpActor, FileSystem, StickyNote)))
			animManager->finishAnimation(this); // finish animations so that syncStickyNoteWithFileContents call back will be added
		// Create a custom animation
		setSizeAnim(getDims(), newDims, numTicks);
	}
	else
	{
		if (!scnManager->messages()->hasMessage("BumpObject::grow"))
		{
			MessageClearPolicy clearPolicy;
				clearPolicy.setTimeout(2);
			scnManager->messages()->addMessage(new Message("BumpObject::grow", QT_TR_NOOP("Can't make it any bigger!\nIt won't fit in the workspace."), Message::Ok, clearPolicy));
		}
	}

	// Update this object and its cohorts
	if (getParent())
		updateObject();
}

void BumpObject::shrink(uint numTicks, float shrinkFactor)
{
	sizeAnim.clear();

	Vec3 newDims(getDims() * shrinkFactor);
	Vec3 minDims = getMinimumDims();

	if (newDims.x > minDims.x ||
		newDims.y > minDims.y)
	{
		newDims.z = getDims().z;
		if (isObjectType(ObjectType(BumpActor, FileSystem, StickyNote)))
			animManager->finishAnimation(this); // finish animations so that syncStickyNoteWithFileContents call back will be added
		setSizeAnim(getDims(), newDims, numTicks); // Create a custom animation
	}
	else
	{
		if (!scnManager->messages()->hasMessage("BumpObject::shrink"))
		{
			MessageClearPolicy clearPolicy;
				clearPolicy.setTimeout(2);
			scnManager->messages()->addMessage(new Message("BumpObject::shrink", QT_TR_NOOP("Can't shrink it any further!\nIt'll be too hard to see."), Message::Ok, clearPolicy));
		}
	}

	// Update ourselves and our parent
	if (getParent())
	{
		updateObject();
	}
}

// Based on the dimensions set by updateReferenceDims(), the scale factor scales
// the object.
void BumpObject::scaleFromReferenceDims(float scaleFactor)
{
	Vec3 maxDims = getMaximumDims();
	Vec3 minDims = getMinimumDims();
	Vec3 newDimensions = referenceDims * scaleFactor;
	if (newDimensions.x >= minDims.x && 
		newDimensions.y >= minDims.y &&
		newDimensions.x <= maxDims.x &&
		newDimensions.y <= maxDims.y)
	{
		newDimensions.z = referenceDims.z;
		setDims(newDimensions);
	}
}

void BumpObject::updateObject()
{
	// Update the parent
	if (getParent())
	{
		BumpObject *obj = getParent();

		// Update the parent
		obj->updateObject();
	}
}

NxActorWrapper * BumpObject::getPinWall()
{
	return pinWall;
}

// -------------------------------------------------------------------------
//                                  EVENTS                                 
// -------------------------------------------------------------------------

void BumpObject::onPin(bool generateNewPin, bool projectToWall)
{	
	NxSphericalJointDesc jointDesc;
	NxSpringDesc spring;
	NxJoint *joint;
	Vec3 pos = getGlobalPosition();
	Vec3 localPt(0, 1, 0);
			
	// get the wall that this item is pinned to
	Ray r(Vec3(0.0f, getGlobalPosition().y, 0.0f), getGlobalPosition());
		r.dir.y = 0.0f;
		r.dir.normalize();
	tuple<int, NxReal, Vec3, NxPlane> t = RayIntersectDesktop(r);
	assert(t.get<0>() > -1);
	pinWall = GLOBAL(Walls)[t.get<0>()];
	assert(pinWall);

	if (!generateNewPin)
	{
		setGlobalPosition(pinPointInSpace - pinPointOnActor);
	}
	else if (projectToWall)
	{
		Vec3 pt = t.get<2>();
		NxPlane plane = t.get<3>();
		setGlobalPosition(pt + (plane.normal * getDims().z));
		setGlobalPosition(adjustPtOnWall(this, pinWall));
	}

	// Proper locations in Global and local space or this pin
	pinPointOnActor = getGlobalOrientation() * localPt * (getDims().y * 0.8f);
	pinPointInSpace = getGlobalPosition() + pinPointOnActor;
	stopAllMotion();

	// Set up the type of joint
	jointDesc.setToDefault();
	jointDesc.actor[0] = getActor();
	jointDesc.actor[1] = NULL;
	jointDesc.localAnchor[0] = pinPointOnActor;
	jointDesc.setGlobalAnchor(pinPointInSpace);
	jointDesc.flags |= NX_SJF_JOINT_SPRING_ENABLED;

	// Create a Spring
	const float kSpring = 1200.0f;
	const float kDamping = 200.0f;
	spring.spring = getMass() * kSpring;
	spring.damper = getMass() * kDamping;
	spring.targetValue = 0;
	jointDesc.jointSpring = spring;

	// Check the joint for validity
	if (!jointDesc.isValid()) return;

	// Break the pin before applying the new pin
	breakPin();

	// Create the joint itself
	joint = GLOBAL(gScene)->createJoint(jointDesc);
	if (joint)
	{
		// Treat this item as Pinned
		pinned = true;

		// hide the name if this is an actor thumbnail (but not a photo frame, or widget)
		if (getObjectType() == ObjectType(BumpActor, FileSystem))
		{
			FileSystemActor * actor = dynamic_cast<FileSystemActor *>(this);
			Widget * w = widgetManager->getActiveWidgetForFile(actor->getFullPath());
			bool isWidget = (w != NULL) && w->isWidgetOverrideActor(actor);
			if (!isWidget && actor->isFileSystemType(Thumbnail) && !actor->isFileSystemType(PhotoFrame))
				hideText();
		}

		// Record this pinning
		statsManager->getStats().bt.interaction.actors.pinned++;

		// Pass this joint to the pool of other joints
		setPinJoint(joint);
	}

	textManager->invalidate();
}

// This may be called unconditionally, whether the object is pinned or not
void BumpObject::breakPin()
{
	if (pinned)
	{
		// Remove the joints from this actor
		PruneJoints((NxActorWrapper *) this);

		// Reset these vars AFTER PruneJoints()
		pinned = false;
		pinWall = NULL;
		pinPointInSpace = Vec3(0.0f);
		pinPointOnActor = Vec3(0.0f);

		onBreakPin();
	}
}

// This is called only after a pinned object becomes unpinned
// Subclasses may override but should call the superclass implementation
void BumpObject::onBreakPin()
{
	// record this break pin
	statsManager->getStats().bt.interaction.actors.unpinned++;

	textManager->invalidate();
}

void BumpObject::onSelect()
{
	textManager->invalidate();

	if (getNameableOverlay())
		getNameableOverlay()->markSelected(true);
	Selectable::onSelect();
}

void BumpObject::onDeselect()
{
	if (getNameableOverlay())
		getNameableOverlay()->markSelected(false);

	Selectable::onDeselect();
}

void BumpObject::onDragMove()
{
	QSet<BumpObject *> curList;
	int x, y;

	if (isMoving() && isDragging())
	{
		// Check which items we are crossing over
		winOS->GetMousePosition(x, y);
		curList = getAllObjectsUnderCursor(x, y);

		// Remove the reference for this object from the list
		curList.remove(this);
		
		// Prune any actors from the previous hit list that have been
		// deleted from the scene
		QSetIterator<BumpObject *> iter(_curHitList);
		while (iter.hasNext())
		{
			BumpObject * obj = iter.next();
			if (!scnManager->containsObject(obj))
				_curHitList.remove(obj); 
		}
		
		QSetIterator<BumpObject *> curListIter(curList);
		while (curListIter.hasNext())
		{
			// When the hit list contains an object and its pile parent as a grid, 
			// remove the grid if the object is a drop target, else remove the object and use grid as target
			BumpObject * obj = curListIter.next();
			if (obj->isParentType(BumpPile))
			{
				Pile * parent = (Pile *) obj->getParent();
				if (parent->getPileState() == Grid)
				{
					if (obj->isValidDropTarget())
						curList.remove(obj->getParent());
					else
						curList.remove(obj);
				}
				else if (parent->getPileState() == Leaf)
				{
					// redirect all drops to the parent pile if this is not the leafed item
					// otherwise, remove the parent if it is (in case we are hovering over
					// the leafed actor and it is being rendered over the pile)
					if (parent->getActiveLeafItem() != obj)
						curList.remove(obj);
					else
						curList.remove(parent);
				}
			}
		}
		
		if (curList.size() > 1)
		{
			// we disable dropping on everything
			QSet<BumpObject *> deprecatedHits = (curList + _curHitList);
			QSetIterator<BumpObject *> deprecatedHitsIter(deprecatedHits);
			while (deprecatedHitsIter.hasNext())
			{
				BumpObject * obj = deprecatedHitsIter.next();
				if (obj->isValidDropTarget())
					obj->onDropExit();
			}

			// set the current list of hit objects
			_curHitList.clear();
		}
		else
		{
			// call onDropMove on existing hits
			QSet<BumpObject *> existingHits = (_curHitList & curList);
			QSetIterator<BumpObject *> existingHitsIter(existingHits);
			while (existingHitsIter.hasNext())
			{
				BumpObject * obj = existingHitsIter.next();
				if (obj->isValidDropTarget())
					obj->onDropMove(sel->getBumpObjects());
			}

			// call onDropEnd on old, deprecated hits
			QSet<BumpObject *> deprecatedHits = (_curHitList - existingHits);
			QSetIterator<BumpObject *> deprecatedHitsIter(deprecatedHits);
			while (deprecatedHitsIter.hasNext())
			{
				BumpObject * obj = deprecatedHitsIter.next();
				if (obj->isValidDropTarget())
					obj->onDropExit();
			}

			// call onDropBegin on new hits
			QSet<BumpObject *> newHits = (curList - _curHitList);
			QSetIterator<BumpObject *> newHitsIter(newHits);
			while (newHitsIter.hasNext())
			{
				BumpObject * obj = newHitsIter.next();
				if (obj->isValidDropTarget())
					obj->onDropEnter(sel->getBumpObjects());
			}

			// set the current list of hit objects
			_curHitList = curList;

		}
	}
}

void BumpObject::onDragBegin(FinishedDragCallBack func)
{
	NxShape **shapes = actor->getShapes();

	// finish all animations on this object
	animManager->finishAnimation(this);
	
	// Call Parent event
	BtDragObject::onDragBegin(this, NULL);
	BtDragObject::setFinishedDragCallback(func);
}

void BumpObject::onDragEnd()
{
	NxShape **shapes = actor->getShapes();
	float closestDistSq = FLT_MAX;
	BumpObject *closestObj = NULL, *obj;

	if (isDragCancelled())
	{
		// end the drop on all the items that the cursor is currently over
		QSetIterator<BumpObject *> curHitsIter(_curHitList);
		while (curHitsIter.hasNext())
		{
			BumpObject * obj = curHitsIter.next();
			if (obj->isValidDropTarget())
				obj->onDropExit();
		}

		// Call Parent event
		BtDragObject::onDragEnd();
	}
	else
	{
		// Check if we can drop into the object we just released on
		QSetIterator<BumpObject *> iter(_curHitList);
		while (iter.hasNext())
		{
			obj = iter.next();
			// Check which object is closest to ours
			if (scnManager->containsObject(obj) &&
				obj->isBeingHoveredOver() && obj->isValidDropTarget() && obj->isSourceValid() && 
				obj->getGlobalPosition().distanceSquared(getGlobalPosition()) < closestDistSq)
			{
				closestObj = obj;
			}
		}

		// Drop the object we were holding into this one
		if (closestObj)
		{
			obj = sel->getPickedActor();

			if (closestObj->isSourceValid())
			{
				// Only drop if the item were dragging has a small velocity
				// XXX: we are temporarily disabling the velocity check for drops
				// if (obj && obj->getLinearVelocity().magnitudeSquared() < 0.3f)
				{
					closestObj->onDrop(sel->getBumpObjects());
					sel->clear();
					sel->add(closestObj);
				}
			}

			closestObj->onDropExit();
		}

		// Call Parent event
		BtDragObject::onDragEnd();
		BtDragObject::execFinishedDragCallback(this);
	}

	if (isObjectType(ObjectType(BumpActor)) && !isParentType(BumpPile))
	{
		// re-enable rotations
		setRotation(true);
	}
}

void BumpObject::onDropEnter(vector<BumpObject *> &objList)
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

				if (size.x > size.z) size *= NxMath::min((getDims().x * 0.7f) / size.x, 2.0f);
				if (size.x < size.z) size *= NxMath::min((getDims().z * 0.7f) / size.z, 2.0f);

				source[i]->finishAnimation();
				source[i]->setSizeAnim(source[i]->getDims(), size, 15);
			}
		}

		updateDropCursor(objList);
	}else{
		// Exit the drop if its invalid
		DropObject::onDropExit();
	}
}

void BumpObject::onDropMove(vector<BumpObject *> &objList)
{
	assert(!objList.empty());
	QString operation = resolveDropOperationString(objList);
	if (!operation.isEmpty())
	{
		unsigned int properFileCount = 0;
		for (unsigned int i = 0; i < objList.size(); ++i)
		{
			if (objList[i]->isBumpObjectType(BumpPile) &&
				(objList[i]->getObjectType() == ObjectType(BumpPile, SoftPile)))
			{
				Pile * p = (Pile *) objList[i];
				properFileCount += p->getNumItems();
			}
			else
				++properFileCount;
		}

		TextOverlay * cursorMessage = NULL;
		AbsoluteOverlayLayout * cursorContainer = scnManager->cursor(&cursorMessage, NULL);
		if (properFileCount <= 1)
			cursorMessage->setText(operation);
		else
			cursorMessage->setText(QString(QT_TR_NOOP("%1 (%2 files)")).arg(operation).arg(properFileCount));
	}
}
void BumpObject::onDropExit()
{
	if (isSourceValid())
	{
		finishAnimation();

		// Grow back to the right size
		for (uint i = 0; i < source.size(); i++)
		{
			if (scnManager->containsObject(source[i]) && 
				source[i]->isBumpObjectType(BumpActor))
			{
				source[i]->finishAnimation();
				if (scnManager->containsObject(source[i]))
					source[i]->setSizeAnim(source[i]->getDims(), source[i]->stateBeforeDrag().dims, 15);
			}
		}

		// Call the Parent
		DropObject::onDropExit();

		// update the cursor
		OverlayLayout * layout = NULL;
		TextOverlay * cursorMessage = NULL;
		scnManager->cursor(NULL, &layout);
		layout->setAlpha(0.0f);
		layout->setSize(layout->getPreferredDimensions());
		layout->reLayout();
	}
}

bool BumpObject::onMouseDown( MouseOverlayEvent& mouseEvent )
{
	// handle the click from the nameable overlay
	bool isInvisible = ((NamableOverlayLayout *) (mouseEvent.getTarget()->getParent()))->isInvisible(getNameableOverlay());
	if (!isInvisible && !isTextHidden() && getNameableOverlay()->getStyle().isVisible())
	{
		if (isParentType(BumpPile))
		{			
			// Don't process Text actors of items that are in a Stacked Pile
			// (should never occur)
			Pile *pile = (Pile *) getParent();
			assert(!(pile->getPileState() == Stack));
		}

		// set the picked actor
		if (!sel->isInSelection(this) && !winOS->IsKeyDown(KeyControl))
			sel->clear();
	
		//only pass this if it is a left click
		if (mouseEvent.getButton() == Qt::LeftButton)
			Renamer->mouseDown(this, (int)mouseEvent.getAbsolutePosition().x, (int)mouseEvent.getAbsolutePosition().y);
		
		sel->add(this);
		sel->setPickedActor(this);

		// get the point on the actor
		float dist = 0.0f;
		Vec3 nearPt, farPt, dir;
		Vec3 pointOnActor;
		window2world(mouseEvent.getAbsolutePosition().x, winOS->GetWindowHeight() - mouseEvent.getAbsolutePosition().y, nearPt, farPt);
		dir = farPt - nearPt;
		dir.normalize();	
		Ray worldRay(nearPt, dir);
		NxPlane frontFace = getFrontFacePlane();
		frontFace.d = -frontFace.d;
		NxRayPlaneIntersect(worldRay, frontFace, dist, pointOnActor);
		getGlobalPose().multiplyByInverseRT(pointOnActor, pointOnActor);
		sel->setStabPointActorSpace(pointOnActor);
		
		// Always return False so that we can continue through with the mouse handling
		return false;
	}
	return false;
}

bool BumpObject::onMouseUp(MouseOverlayEvent& mouseEvent)
{
	// handle the click from the nameable overlay
	bool isInvisible = ((NamableOverlayLayout *) (mouseEvent.getTarget()->getParent()))->isInvisible(getNameableOverlay());
	if (!isInvisible && !isTextHidden() && getNameableOverlay()->getStyle().isVisible())
	{
		Renamer->mouseUp(this, (int)mouseEvent.getAbsolutePosition().x, (int)mouseEvent.getAbsolutePosition().y);
	}
	
	return false;
}

bool BumpObject::onMouseMove(MouseOverlayEvent& mouseEvent)
{
	return false;
}

bool BumpObject::shouldRenderText()
{
	if (getNameableOverlay() && getNameableOverlay()->getStyle().isVisible())
	{
		if (!getText().isEmpty())
			return true;
	}
	return false;
}


Pile* BumpObject::getPileParent()
{
	if (isParentType(BumpPile))
		return (Pile*) getParent();

	return NULL;

}

void BumpObject::onClick()
{}

void BumpObject::onAnimFinished()
{
	// Stop the item from moving after an animation
	stopAllMotion();

	// Stop the parent as well
	if (parent)
	{
		parent->stopAllMotion();
	}
}

void BumpObject::setEnforcePinning( bool enforce )
{
	enforcePinning = enforce;
}

void BumpObject::setOnGrowHandler( boost::function<void()> onGrowHandler )
{
	{
		_onGrowHandler = onGrowHandler;
	}
}

void BumpObject::updateDropCursor(vector<BumpObject *> objList)
{
	// update the cursor
	assert(!objList.empty());
	QString operation = resolveDropOperationString(objList);
	if (!operation.isEmpty())
	{
		// do a proper count of all the items (invididual and normal pile items)
		int properFileCount = 0;
		for (int i = 0; i < objList.size(); ++i)
		{
			if (objList[i]->isBumpObjectType(BumpPile) &&
				(objList[i]->getObjectType() == ObjectType(BumpPile, SoftPile)))
			{
				Pile * p = (Pile *) objList[i];
				properFileCount += p->getNumItems();
			}
			else
				++properFileCount;
		}

		OverlayLayout * layout = NULL;
		TextOverlay * cursorMessage = NULL;
		AbsoluteOverlayLayout * cursorContainer = scnManager->cursor(&cursorMessage, &layout);
		if (properFileCount <= 1)
			cursorMessage->setText(operation);
		else
			cursorMessage->setText(QString(QT_TR_NOOP("%1 (%2 files)")).arg(operation).arg(properFileCount));
		layout->setAlpha(1.0f);
		layout->setSize(layout->getPreferredDimensions());
		int x, y;
		winOS->GetMousePosition(x, y);
		layout->getStyle().setOffset(Vec3(x, winOS->GetWindowHeight() - y, 0));
		cursorContainer->reLayout();
	}
}

// Update the reference dimensions with the current dimensions. This causes subsequent calls
// to scaleFromReferenceDims() to scale the object based on a current snapshot of its dimensions.
// Pinch Zoom gesture makes heavy use of this.
void BumpObject::updateReferenceDims()
{
	referenceDims = getDims();
}

bool BumpObject::serializeToPb(PbBumpObject * pbObject)
{
	assert(pbObject);

	// save the object type
	assert(type.secondaryType != 12);
	pbObject->set_type(type.asUInt());

	// save the pose and dimensions
	PbPoseDims * poseDims = pbObject->mutable_pose_dims();
	PbMat34 * pose = poseDims->mutable_pose();
	toPbQuat(getGlobalOrientationQuat(), pose->mutable_quat());
	toPbVec3(getGlobalPosition(), pose->mutable_t());
	toPbVec3(getDims(), poseDims->mutable_dims());

	// save the text and the visibility
	pbObject->set_text(stdString(getFullText()));
	pbObject->set_text_visible(!isTextHidden());

	// save the pinned state
	pbObject->set_pinned(isPinned());
	if (isPinned())
	{
		toPbVec3(pinPointOnActor, pbObject->mutable_pin_point_actor());
		toPbVec3(pinPointInSpace, pbObject->mutable_pin_point_world());
	}

	return pbObject->IsInitialized();
}

bool BumpObject::deserializeFromPb(const PbBumpObject * pbObject)
{
	assert(pbObject);

	// read the object type
	if (pbObject->has_type())
		setObjectType(ObjectType::fromUInt(pbObject->type()));

	// read the pose and dims
	if (pbObject->has_pose_dims())
	{
		const PbPoseDims& poseDims = pbObject->pose_dims();
		const PbMat34& pose = poseDims.pose();
		setGlobalOrientationQuat(fromPbQuat(pose.quat()));
		setGlobalPosition(fromPbVec3(pose.t()));
		setDims(fromPbVec3(poseDims.dims()));
	}

	// read the text
	if (pbObject->has_text())
		setText(qstring(pbObject->text()));

	// read the text visibility
	if (pbObject->has_text_visible())
	{
		if (!pbObject->text_visible())
			hideText(true);
	}

	// read the pinned state
	if (pbObject->has_pinned() && pbObject->has_pin_point_actor() && pbObject->has_pin_point_world())
	{
		if (pbObject->pinned())
		{
			onPin(true, true);
		}
	}
	return true;
}