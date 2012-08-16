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

#ifndef _DROP_OBJECT_
#define _DROP_OBJECT_

class BumpObject;

// Interface for an object to be the drop target of a drag-and-drop operation.
// NOTE: You probably don't need to inherit from this class, since BumpObject
// already does.
// Minimally, you need to override isValidDropTarget(), isSourceValid(), and
// onDrop().
//
// The sequence of events as implemented in BumpObject is as follows:
//   isValidDropTarget() - if false, no other methods are called
//   onDropEnter()
//   isSourceValid() - called multiple times between onDropEnter & onDropExit
//   onDrop() - called when the mouse is released over this object
//   onDropExit() - will always be called, whether a drop occurred or not
//
// isSourceValid is checked multiple times in the drop process. If it is an
// expensive call, you may want to cache the value, and reset it in onDropExit.
// Once isSourceValid() returns true, it must return true for the entire drop
// process, until after BumpObject::onDropExit is called. This means that if
// you cache the value, don't reset it until AFTER the superclass onDropExit.
// Also note that isSourceValid is called when tossing occurs, so be careful
// if you are cacheing the value.
class DropObject
{
	Q_DECLARE_TR_FUNCTIONS(DropObject)

protected:

	bool hasObjectEntered;
	bool hasValidSources;
	vector<BumpObject *> source;

public:

	DropObject();
	~DropObject();

	// Events
	virtual void	onDropEnter(vector<BumpObject *> &objList);
	virtual void	onDropExit();
	virtual vector<BumpObject *> onDrop(vector<BumpObject *> &objList);
	virtual void	onDropHover(vector<BumpObject *> &objList);

	// Getters
	virtual QString resolveDropOperationString(vector<BumpObject *>& objList);
	bool			isBeingHoveredOver();
	virtual bool	isValidDropTarget();
	virtual bool	isSourceValid();
};

// -----------------------------------------------------------------------------

#else
	class DropObject;
#endif