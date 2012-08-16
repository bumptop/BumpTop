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
#include "BT_DropObject.h"

DropObject::DropObject()
{
	hasObjectEntered = false;
	hasValidSources = false;
}

DropObject::~DropObject()
{
}

bool DropObject::isBeingHoveredOver()
{
	// -------------------------------------------------------------------------
	// NOTE: This function returns a state that signifies weather an object is
	//       hovering over it.
	// -------------------------------------------------------------------------

	return hasObjectEntered;
}

void DropObject::onDropEnter(vector<BumpObject *> &objList)
{
	// -------------------------------------------------------------------------
	// NOTE: This function gets triggered when an object enters the bounding 
	//       region of the object it represents.
	// -------------------------------------------------------------------------

	source = objList;

	if (!source.empty())
	{
		hasObjectEntered = true;
	}
}

void DropObject::onDropExit()
{
	// -------------------------------------------------------------------------
	// NOTE: This function gets triggered when an object, that was once dragged
	//       over another object, leaves its bounds.
	// -------------------------------------------------------------------------

	hasObjectEntered = false;
	source.clear();
}

vector<BumpObject *> DropObject::onDrop(vector<BumpObject *> &objList)
{
	// -------------------------------------------------------------------------
	// NOTE: This function receives a list of objects that the user dragged into
	//       its area and released the mouse. 
	// -------------------------------------------------------------------------

	return objList;
}

void DropObject::onDropHover(vector<BumpObject *> &objList)
{
	// -------------------------------------------------------------------------
	// NOTE: This function receives a list of objects that it will act upon when
	//       the user drags those objects into the area of this object. Only 
	//       after a time out threshold will this event get fired.
	// -------------------------------------------------------------------------
}

QString DropObject::resolveDropOperationString(vector<BumpObject *>& objList )
{
	return QT_TR_NOOP("Move");
}

bool DropObject::isValidDropTarget()
{
	// -------------------------------------------------------------------------
	// NOTE: If an object can receive other objects by way of dragging, this
	//       function should return true.
	// -------------------------------------------------------------------------

	return false;
}

bool DropObject::isSourceValid()
{
	// -------------------------------------------------------------------------
	// NOTE: This function will return true if the object(s) that are being
	//       dragged over it can be accepted as a valid source.
	// -------------------------------------------------------------------------

	return false;
}