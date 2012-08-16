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
#include "BT_BumpObject.h"
#include "BT_DragObject.h"

BtDragObject::BtDragObject()
: dragged(false)
, dragCancelled(false)
, _finishedDragCallback(NULL)
{}

BtDragObject::~BtDragObject()
{
}

void BtDragObject::onDragBegin(BumpObject * obj, FinishedDragCallBack func)
{
	// -------------------------------------------------------------------------
	// NOTE: This function gets triggered when the user capture the selected 
	//       object by clicking on an object and moving it.
	// -------------------------------------------------------------------------

	captureStateBeforeDrag(obj);
	dragged = true;
	setFinishedDragCallback(func);
}

void BtDragObject::onDragEnd()
{
	// -------------------------------------------------------------------------
	// NOTE: This function gets triggered when the user releases the selected 
	//       object by letting go of the mouse button.
	// -------------------------------------------------------------------------

	dragged = false;
	dragCancelled = false;
	_curHitList.clear();
}

bool BtDragObject::isDragging()
{
	// -------------------------------------------------------------------------
	// NOTE: This function returns the state that represents weather or not an
	//       object is selected by the user and moved around.
	// -------------------------------------------------------------------------

	return dragged;
}

void BtDragObject::onDragMove()
{
	// -------------------------------------------------------------------------
	// NOTE: A Drag Move event is triggered every frame of the timer callback
	//       when movement of the selected objects is determined.
	// -------------------------------------------------------------------------
}

void BtDragObject::onDragHover()
{
	// -------------------------------------------------------------------------
	// NOTE: This function gets called when this object hovers over another 
	//       object. Hovering is based on a timer.
	// -------------------------------------------------------------------------
}

bool BtDragObject::isDragHovering()
{
	// -------------------------------------------------------------------------
	// NOTE: This function returns the state of the object as it pertains to
	//       hovering. If this object is hovering over another object, the
	//       return value is true
	// -------------------------------------------------------------------------

	return !_curHitList.empty() && dragged;
}

QSet<BumpObject *>& BtDragObject::getTargets()
{
	// -------------------------------------------------------------------------
	// NOTE: The getTargets Function returns all the targets that this object
	//       is hovering over or has intersected. 
	// -------------------------------------------------------------------------

	return _curHitList;
}

void BtDragObject::captureStateBeforeDrag(BumpObject * object)
{
	_stateBeforeDrag.dims = object->getDims();
	_stateBeforeDrag.pose = object->getGlobalPose();
	_stateBeforeDrag.pinned = object->isPinned();
	_stateBeforeDrag.isValid = true;

	// TODO: zero the actor's motion?
}

void BtDragObject::restoreStateFromBeforeDrag(BumpObject * object)
{
	assert(_stateBeforeDrag.isValid);
	object->setDims(_stateBeforeDrag.dims);
	object->setPoseAnim(object->getGlobalPose(), _stateBeforeDrag.pose, 15);
	if (_stateBeforeDrag.pinned)
		object->onPin();
}

void BtDragObject::setFinishedDragCallback(FinishedDragCallBack func)
{
	_finishedDragCallback = func;
}

void BtDragObject::execFinishedDragCallback(BumpObject * target)
{
	if (_finishedDragCallback)
	{
		_finishedDragCallback(target);
		_finishedDragCallback = NULL;
	}
}

PreDragPhysicalState& BtDragObject::stateBeforeDrag()
{
	return _stateBeforeDrag;
}

void BtDragObject::markDragCancelled(bool cancelled)
{
	dragCancelled = cancelled;
}

bool BtDragObject::isDragCancelled() const
{
	return dragCancelled;
}