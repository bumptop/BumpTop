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
#include "BT_Actor.h"
#include "BT_AnimationManager.h"
#include "BT_Camera.h"
#include "BT_FileSystemPile.h"
#include "BT_GLTextureManager.h"
#include "BT_MousePointer.h"
#ifdef DXRENDER
	#include "BT_DXRender.h"
#endif
#include "BT_Pile.h"
#include "BT_RenderManager.h"
#include "BT_SceneManager.h"
#include "BT_Selection.h"
#include "BT_Util.h"
#include "BT_WindowsOS.h"
#include "BumpTop.h"

Actor::Actor()
: _displayListId(0)
, _useMaterialColorOverride(false)
{
	type.primaryType = BumpActor;

	// Init
	crumpleSizeX = 0.0;
	crumpleSizeY = 0.0;
	pinPointOnActor = Vec3(0.0f);
	pinned = false;
	crumpled = false;
	peeled = false;
	maximized = false;
	actorToMimic = NULL;
	
	setAlpha(1.0f);
#ifdef DXRENDER
	ZeroMemory(&_materialColorOverride, sizeof(_materialColorOverride));
	_materialColorOverride = dxr->textureMaterial;
#endif
}

Actor::~Actor()
{
	if (isParentType(BumpPile))
	{
		// Remove this Actor from the pile it resides in
		Pile *pile = (Pile *) getParent();

		for (uint i = 0; i < pile->getNumItems(); i++)
		{
			if ((*pile)[i] == this)
			{
				pile->clear(i);
			}
		}
	}	
#ifdef DXRENDER
	onRelease();
#else
	if (_displayListId)
	{
		glDeleteLists(_displayListId, 1);
		_displayListId = 0;
	}
#endif
}

// Returns the object that this actor is mimicing as an Actor if it is one
// otherwise, returns this.
Actor * Actor::getActorToMimic()
{	
	Actor * actorToMimic = this;
	if (isActorType(Temporary) && 
		getObjectToMimic() && getObjectToMimic()->isBumpObjectType(BumpActor))
	{
		actorToMimic = (Actor *) getObjectToMimic();
	}
	return actorToMimic;
}

bool Actor::isPilable(uint pileType)
{
	// Conditionals
	if (isParentType(BumpPile)) return false;
	if (!isActorType(FileSystem) && !isActorType(Logical) && !isActorType(Custom) && !isActorType(Webpage)) return false;
	if (isActorType(Temporary)) return false;

	// Allow Pileization
	return true;
}

void Actor::pushActorType(ActorType aType)
{
	// Adds a Type
	type.secondaryType |= aType;

	// If we're in infinite desktop mode, update the list of all actors	
	if (GLOBAL(isInInfiniteDesktopMode))
	{
		if ((aType == Invisible) ||
			(aType == Temporary))
		{
			// we should discard this type from being watched if it is not good.
			cam->unwatchActor(this);
		}
	}
}

void Actor::popActorType(ActorType aType)
{
	// Removes a Type 
	type.secondaryType &= ~aType;

	// If we're in infinite desktop mode, update the list of all actors	
	if (GLOBAL(isInInfiniteDesktopMode))
	{
		if ((aType == Invisible) ||
			(aType == Temporary))
		{
			// we should add to the watched list if we are popping a bad type
			cam->watchActor(this);
		}
	}
}

unsigned int Actor::getActorType() const
{
	return type.secondaryType;
}

void Actor::setActorType(unsigned int typeBitMask)
{
	type.secondaryType = typeBitMask;

	// If we're in infinite desktop mode, update the list of all actors	
	if (GLOBAL(isInInfiniteDesktopMode))
	{
		if ((typeBitMask & Invisible) ||
			(typeBitMask & Temporary))
		{
			// we should discard this type from being watched if it is invisible
			cam->unwatchActor(this);
		}
		else
		{
			// we should add to the watched list if it's not invisible
			cam->watchActor(this);
		}
	}
}

void Actor::setTextureID(QString newTextureID)
{
	textureID = newTextureID;
}

#ifdef DXRENDER
// Sets the displayList and increments the reference count
void Actor::setDisplayListId(IDirect3DVertexBuffer9 * displayListId)
{
	SAFE_RELEASE(_displayListId)
	_displayListId = displayListId;
	_displayListId->AddRef();
}
#else
void Actor::setDisplayListId(uint displayListId)
{
	if (_displayListId)
	{
		glDeleteLists(_displayListId, 1);
		_displayListId = 0;
	}
	_displayListId = displayListId;
}
#endif

#ifdef DXRENDER
IDirect3DVertexBuffer9 * Actor::getCustomDisplayListId()
#else
unsigned int Actor::getCustomDisplayListId()
#endif
{
	// by default, we use this actor for texture information, but
	// if we are mimic'ing another actor, then use that instead
	Actor * actorToMimicForTextures = getActorToMimic();
	if (actorToMimicForTextures != this)
		return actorToMimicForTextures->getCustomDisplayListId();

	return _displayListId;
}

QString Actor::getTextureID()
{
	// by default, we use this actor for texture information, but
	// if we are mimic'ing another actor, then use that instead
	Actor * actorToMimicForTextures = getActorToMimic();
	if (actorToMimicForTextures != this)
		return actorToMimicForTextures->getTextureID();

	return textureID;
}


#ifdef DXRENDER
IDirect3DTexture9 * Actor::getTextureNum()
#else
uint Actor::getTextureNum()
#endif
{
	// by default, we use this actor for texture information, but
	// if we are mimic'ing another actor, then use that instead
	Actor * actorToMimicForTextures = getActorToMimic();
	if (actorToMimicForTextures != this)
		return actorToMimicForTextures->getTextureNum();

	return texMgr->getGLTextureId(textureID);
}

//************************************
// Method:    onLaunch
// FullName:  Actor::onLaunch
// Access:    public 
// Returns:   void
// Qualifier:
// Description: This Maximizes and Minimizes (towards the camera) the actor
//************************************
void Actor::onLaunch()
{
	assert(false);
	if (maximized)
	{
		setFrozen(true);
		float maxDim = maxVec3Dim(getDims());
		preMaxOrient = getGlobalPose();

		// Determine shortest rotation to get in parallel to camera
		Quat Foo = Quat(90, Vec3(1,0,0));

		if (cam->getDir().x != 0 && cam->getDir().z != 0)
		{
			// Looking On an Angle (24°)
			Foo = Quat(90 - 24, Vec3(1,0,0));
		}

		// Create an Animation
		setPoseAnim(actor->getGlobalPose(), Mat34(Foo, cam->getEye() + maxDim * 2.0f * cam->getDir()), 25);
		maximized = true;
	}else{
		setPoseAnim(getGlobalPose(), preMaxOrient, 25);
		setFrozen(false);
		maximized = false;
	}
}

bool Actor::shouldRenderText()
{
	if (isActorType(Invisible))
	{
		return false;
	}
	return BumpObject::shouldRenderText();
}

void Actor::onRender(uint flags)
{
	if (isActorType(Invisible)) return;
	
#ifdef DXRENDER
	// TODO DXR depth writing is mostly disabled in GL, so depth test should just be disabled ?
	if ((flags & RenderIgnoreDepth) || isSelected() || isActorType(Temporary))
		dxr->device->SetRenderState(D3DRS_ZENABLE, false);
	if (isActorType(Temporary))
		dxr->device->SetRenderState(D3DRS_ZWRITEENABLE, true);

	// update the material if the color override (if enabled) or alpha changes
	if (_useMaterialColorOverride)
	{
		// just ensure that the alpha is up to date
		_materialColorOverride.Diffuse.a = _materialColorOverride.Ambient.a = getAlpha();
		dxr->device->SetMaterial(&_materialColorOverride);
	}
	else if (getAlpha() != dxr->textureMaterial.Diffuse.a)
	{
		dxr->textureMaterial.Diffuse.a = dxr->textureMaterial.Ambient.a = getAlpha();
		dxr->device->SetMaterial(&dxr->textureMaterial);
	}

	if (RenderCustomDisplayList & flags)
		dxr->renderVertexBuffer(getGlobalPosition(), getGlobalOrientation(), getDims(), getTextureNum(), getCustomDisplayListId());
	else
		dxr->renderSideLessBox(getGlobalPosition(), getGlobalOrientation(), getDims(), getTextureNum());

	if ((flags & RenderIgnoreDepth) || isSelected() || isActorType(Temporary))
		dxr->device->SetRenderState(D3DRS_ZENABLE, true);
	if (isActorType(Temporary))
		dxr->device->SetRenderState(D3DRS_ZWRITEENABLE, false);
#else

	bool useAlpha = texMgr->getTextureAlpha(getTextureID());

	glPushAttribToken token(GL_ENABLE_BIT);
	if (flags & RenderIgnoreDepth) 
		glDisable(GL_DEPTH_TEST);
	else
		glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);	
	
	// Conditionals
	if (isBumpObjectType(BumpWidget) || isMaximized()) glDisable(GL_LIGHTING);
	if (isSelected() || isActorType(Temporary)) glDisable(GL_DEPTH_TEST);

	// Set the Texture
	glColor4f(1,1,1,getAlpha());
	glBindTexture(GL_TEXTURE_2D, getTextureNum());

	// if (isSelected()) glEnable(GL_POLYGON_STIPPLE);
	if (isActorType(Temporary)) glDepthMask(GL_TRUE);
	// Render Self
	if (flags & RenderCustomDisplayList)
	{
		_ASSERT(getCustomDisplayListId());
		ShapeVis::renderDisplayList(getGlobalPosition(), getGlobalOrientation(), getDims(), getCustomDisplayListId());
	}
	else if (flags & RenderSideless && useAlpha) 
	{
		ShapeVis::renderSideLessBox(getGlobalPosition(), getGlobalOrientation(), getDims());
	}
	else
	{
		if (getParent())
			ShapeVis::renderBox(getGlobalPosition(), getGlobalOrientation(), getDims());
		else
			ShapeVis::renderColorSidedBox(getGlobalPosition(), getGlobalOrientation(), getDims());
	}

	if (isActorType(Temporary)) glDepthMask(GL_FALSE);
	// if (isSelected()) glDisable(GL_POLYGON_STIPPLE);
#endif
}

#ifdef DXRENDER
void Actor::onRelease()
{
	SAFE_RELEASE(_displayListId);
}
#endif

void Actor::onTouchDown(Vec2 &pt, MousePointer* mousePointer)
{
	assert(mousePointer != NULL);

	if (find(touches.begin(), touches.end(), mousePointer) == touches.end())
	{
		touches.push_back(mousePointer);

		// if we have two touches on the object, start zooming
		if (touches.size() == 2) // only need to start when we make transition from 1 bg touch -> 2 bg touches
		{ 
			// start panning
			initialPinchDist = sqrt((float)pow((float)touches[0]->getX() - touches[1]->getX(), 2) + (float)pow((float)touches[0]->getY() - touches[1]->getY(), 2));
			pinchScaleFactor = 1.0;
		}
	}
	// pt is in client coords?

	// add to touches
}
void Actor::onTouchUp(Vec2 &pt, MousePointer* mousePointer)
{
	onTouchMove(pt);

	// remove from touches
	vector<MousePointer *>::iterator mousePointerIterator = find(touches.begin(), touches.end(), mousePointer);
	if (mousePointerIterator != touches.end()) touches.erase(mousePointerIterator);

}
void Actor::onTouchMove(Vec2 &pt)
{
	if (touches.size() >= 2)
	{
		float pinchDist = sqrt((float)pow((float)touches[0]->getX() - touches[1]->getX(), 2) + (float)pow((float)touches[0]->getY() - touches[1]->getY(), 2));

		float newPinchScaleFactor = pinchDist/initialPinchDist;

		Vec3 newDims = getDims() * newPinchScaleFactor/pinchScaleFactor;
			//consoleWrite("size change: %e, %e --> %e, %e\n", getDims().x, getDims().y, newDims.x, newDims.y);
		stateBeforeDrag().dims = (stateBeforeDrag().dims * newPinchScaleFactor/pinchScaleFactor);
		setDims(stateBeforeDrag().dims);
		// consoleWrite(QString("grow %f").arg(newPinchScaleFactor/pinchScaleFactor));

		pinchScaleFactor = newPinchScaleFactor;
	}

	// update zoom factor
}

void Actor::onDragEnd()
{
	if (!isDragCancelled())
	{
		if (isActorType(Temporary) && actorToMimic)
		{
			// NOTE: Inside processInPileGhosting(), there are comments as to what this
			//       function actually returns. 
			//       * -2 = Not over Slate (Remove from Pile)
			//       * -1 = Over Slate, Not Over Quadrant (Do Nothing)
			int index = -2;
			Pile * pile = NULL;
			Pile * parent = dynamic_cast<Pile*>(actorToMimic->getParent());
			processInPileGhosting(false, index, &pile);

			if (pile || parent)
			{
				// restore the mimic'd actor's alpha
				if (!actorToMimic->isAnimating(AlphaAnim))
					actorToMimic->setAlphaAnim(actorToMimic->getAlpha(), 1.0f, 15);

				// handle the various drop cases
				if (index == -2)
				{
					// Drag Item Out if we are no longer hovering over the slate
					FileSystemPile * fsPile = dynamic_cast<FileSystemPile *>(pile);
					
					// actorToMimic is the real file actor, this is the visual actor for dragging
					if (fsPile && ((fsPile->getOwner()->isFileSystemType(Removable) && !winOS->IsKeyDown(KeyShift)) || winOS->IsKeyDown(KeyControl)))
					{
						// If copy file, files will be copied out from pile but the actor remains in pile. 
						// Directory watcher will add actors for the new copies outside of pile, so set their drop point.
						if (fsPile->copyFromPile(actorToMimic, Update))
						{
							Vec3 v = WorldToClient(getGlobalPosition());
							POINT point = {v.x,v.y};
							winOS->SetDropPoint(point, 1);
						}
					}
					else if (parent && parent->removeFromPile(actorToMimic, Update))
					{
						actorToMimic->finishAnimation();
						actorToMimic->setGlobalPose(getGlobalPose());
						sel->add(actorToMimic);
					}
				}
				else if (pile && index > -1)
				{
					if (pile != parent)
					{
						// remove from the old pile first
						if (parent && parent->removeFromPile(actorToMimic, Update))
							actorToMimic->finishAnimation();

						// add to the new pile
						if (pile->addToPile(actorToMimic))
							animManager->finishAnimation(pile);

						// NOTE: we need to set the shuffle group of the new pile if we 
						//		 want to shuffle it
						pile->setShuffleGroup(sel->getBumpObjects());
					}

					// Allow the pile to shuffle
					pile->insertShuffleGroup(index);
					
					// select the parent
					sel->add(actorToMimic);
				}

				// Clear the shuffle groups
				if (pile)
					pile->clearShuffleGroup();
				if (parent)
					parent->clearShuffleGroup();
			}
		}
		markDragCancelled(false);
	}

	BumpObject::onDragEnd();
}

void Actor::setSizeAnim( Vec3 &startSize, Vec3 &lastSize, uint steps )
{
	BumpObject::setSizeAnim(startSize, lastSize, steps);
}

void Actor::setSizeAnim(deque<Vec3> &customSizeAnim)
{
	BumpObject::setSizeAnim(customSizeAnim);
}

void Actor::enableMaterialColorOverride(const QColor& color)
{
#ifdef DXRENDER
	_materialColorOverride.Diffuse.r = _materialColorOverride.Ambient.r = color.redF();
	_materialColorOverride.Diffuse.g = _materialColorOverride.Ambient.g = color.greenF();
	_materialColorOverride.Diffuse.b = _materialColorOverride.Ambient.b = color.blueF();
	_useMaterialColorOverride = true;
#endif
}

void Actor::disableMaterialColorOverride()
{
#ifdef DXRENDER 
	if (_useMaterialColorOverride)
		dxr->device->SetMaterial(&dxr->textureMaterial);
	_useMaterialColorOverride = false;
#endif
}

void Actor::hideAndDisable()
{
	pushActorType(Invisible);
	setFrozen(true);
	setGravity(false);
	setCollisions(false);
	PushBelowGround(this);
}

void Actor::showAndEnable(Vec3 &pos)
{
	popActorType(Invisible);
	setFrozen(false);
	setGravity(true);
	setCollisions(true);
	setGlobalPosition(pos);
}
