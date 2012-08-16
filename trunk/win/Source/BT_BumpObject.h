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

#ifndef _BT_BUMP_OBJECT_
#define _BT_BUMP_OBJECT_

// -----------------------------------------------------------------------------

#include "BT_Animatable.h"
#include "BT_Nameable.h"
#include "BT_DropObject.h"
#include "BT_DragObject.h"
#include "BT_ObjectType.h"
#include "BT_Renderable.h"
#include "BT_OverlayEvent.h"
#include "BT_Selectable.h"
#include "BT_TossTarget.h"
#include "BT_LaunchTarget.h"
#include "BT_AnimationEntry.h"
#include "BT_NxActorWrapper.h"

class PbBumpObject;
class Pile;
 
// -----------------------------------------------------------------------------

// The type of Visible Object this represents (Max 15 Types of Objects)
enum BumpObjectType
{
	BumpPile			= 1,
	BumpActor			= 2,
	BumpWidget			= 3,
	BumpCluster			= 4,
};

// -----------------------------------------------------------------------------

class BumpObject : public NxActorWrapper, 
				   public Animatable, 
				   public Nameable, 
				   public DropObject, 
				   public BtDragObject,
				   public Renderable,
				   public Selectable,
				   public TossTarget,
				   public LaunchTarget,
				   public MouseOverlayEventHandler
{
	Q_DECLARE_TR_FUNCTIONS(BumpObject)

protected:

	ObjectType type;
	BumpObject *parent;
	bool _ignoreSizeUpdate;
	FinishedDragCallBack _finishedDragCallback;

	// Pinning
	bool pinned;
	bool enforcePinning;
	Vec3 pinPointInSpace;
	Vec3 pinPointOnActor;
	NxJoint * pinJoint;
	NxActorWrapper * pinWall;

	boost::function<void()> _onGrowHandler;

	// Reference Dimensions. Used as reference point when performing uniform scaling.
	Vec3 referenceDims;

	// Animation Frame Lists
	deque<Mat34> poseAnim;
	deque<Vec3> sizeAnim;
	deque<float> alphaAnim;
	deque<float> freshnessAlphaAnim;

	virtual void onBreakPin();

public:

	BumpObject();
	virtual ~BumpObject();

	virtual Vec3 getDefaultDims();

	// General Functions
	virtual void		updateObject();
	virtual inline void	fadeOut(uint numTicks = 25);
	virtual inline void	fadeIn(uint numTicks = 25);
	virtual Vec3		getMinimumDims();
	virtual Vec3		getMaximumDims();
	virtual void		grow(uint numTicks = 25, float growFactor = 1.6f);
	virtual void		shrink(uint numTicks = 25, float growFactor = 0.6f);
	virtual void		scaleFromReferenceDims(float scaleFactor);
	virtual void		killAnimation();
	virtual void		finishAnimation();
	virtual void		updateDropCursor(vector<BumpObject *> objList);
	void				syncNameableOverlayToDims();

	// Getters
	NxActorWrapper *	getPinWall();
	inline Vec3			getPinPointInSpace();
	inline Vec3			getPinPointOnActor();
	inline NxJoint *	getPinJoint() const;
	inline BumpObject	*getParent() const;
	inline ObjectType	getObjectType();
	inline float		getFreshnessAlpha();
	Mat34				getAnimFinalPose();
	Vec3				getAnimFinalDims();
	inline bool			isBumpObjectType(BumpObjectType inType);
	inline bool			isObjectType(ObjectType type);
	inline bool			isParentType(ObjectType type);
	virtual bool		isPilable(uint pileType);
	inline bool			isAnimating(uint animType = SizeAnim | AlphaAnim | PoseAnim | FreshnessAnim);
	inline bool			isPinned();
	inline bool			isSelected();
	inline bool			getEnforcePinning();
	virtual bool		shouldRenderText();
	Pile *				getPileParent();

	// Setters
	inline void			setPinPointInSpace(Vec3 newPinInGlobalSpace);
	inline void			setPinPointOnActor(Vec3 newPinInActorSpace);
	inline void			setPinJoint(NxJoint * joint);
	void				setPoseAnim(Mat34 &startPose, Mat34 &lastPose, uint steps, FinishedCallBack callback = NULL, void *customData = NULL);
	void				setPoseAnim(deque<Mat34> &customPoseAnim, FinishedCallBack callback, void *customData = NULL);
	void				setPoseAnim(deque<Mat34> &customPoseAnim);
	virtual void		setSizeAnim(Vec3 &startSize, Vec3 &lastSize, uint steps);
	virtual void		setSizeAnim(deque<Vec3> &customSizeAnim);
	void				setAlphaAnim(float startAlpha, float endAlpha, uint steps);
	void				setFreshnessAlphaAnim(float initialAlpha, uint steps);
	virtual void		setDims(const Vec3 &s);
	inline void			setDimsToDefault();
	inline void			setParent(BumpObject *p);
	inline void			setBumpObjectType(BumpObjectType t);
	inline void			setObjectType(ObjectType t);
	virtual inline void	updateReferenceDims();
	// whether to keep the pin, even if the animation takes the object
	// away from the wall
	void				setEnforcePinning(bool enforce);
	void				setOnGrowHandler( boost::function<void()> onGrowHandler );


	// Object Events
	virtual void		onAnimFinished();
	virtual inline void	onAnimTick();
	virtual void		onSelect();
	virtual void		onDeselect();
	virtual void		onClick();
	virtual void		onPin(bool generateNewPin = true, bool projectToWall = false);
	virtual void		breakPin();

	// Drag-and-drop. See documentation in BT_DropObject.h for an explanation.
	virtual void		onDragMove();
	virtual void		onDragBegin(FinishedDragCallBack func = NULL);
	virtual void		onDragEnd();
	virtual void		onDropEnter(vector<BumpObject *> &objList);
	virtual void		onDropMove(vector<BumpObject *> &objList); // when an object is dragged into this, and move within this
	virtual void		onDropExit();

	virtual bool		onMouseDown(MouseOverlayEvent& mouseEvent);
	virtual bool		onMouseUp(MouseOverlayEvent& mouseEvent);
	virtual bool		onMouseMove(MouseOverlayEvent& mouseEvent);

	// protocol buffers
	virtual bool		serializeToPb(PbBumpObject * pbObject);
	virtual bool		deserializeFromPb(const PbBumpObject * pbObject);
};

// -----------------------------------------------------------------------------

#include "BT_BumpObject.inl"

// -----------------------------------------------------------------------------

#else
	class BumpObject;
	enum BumpObjectType;
#endif