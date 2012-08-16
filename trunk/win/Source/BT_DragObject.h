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

#ifndef BT_DRAG_OBJECT
#define BT_DRAG_OBJECT

// -----------------------------------------------------------------------------

typedef void (* FinishedDragCallBack)(BumpObject * dragObject);

// -----------------------------------------------------------------------------

class BumpObject;

class PreDragPhysicalState 
{
public:
	PreDragPhysicalState() : dims(0.0f), isValid(false), pinned(false) {}

	Vec3 dims;
	Mat34 pose;
	bool pinned;
	bool isValid;
};

// -----------------------------------------------------------------------------

class BtDragObject
{
protected:

	bool dragged;
	bool dragCancelled;
	QSet<BumpObject *> _curHitList;
	PreDragPhysicalState _stateBeforeDrag;
	FinishedDragCallBack _finishedDragCallback;

public:

	BtDragObject();
	virtual ~BtDragObject();

	// Events
	virtual void		onDragBegin(BumpObject * obj, FinishedDragCallBack func = NULL);
	virtual void		onDragEnd();
	virtual void		onDragMove();
	virtual void		onDragHover();

	// Setters/Getters
	QSet<BumpObject *>& getTargets();
	void				markDragCancelled(bool cancelled);
	bool				isDragCancelled() const;
	void				captureStateBeforeDrag(BumpObject * object);
	void				restoreStateFromBeforeDrag(BumpObject * object);
	void				setFinishedDragCallback(FinishedDragCallBack func);
	void				execFinishedDragCallback(BumpObject * target);
	PreDragPhysicalState& stateBeforeDrag();
	virtual bool		isDragHovering();
	virtual bool		isDragging();
};

// -----------------------------------------------------------------------------
#endif // BT_DRAG_OBJECT