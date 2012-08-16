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

#ifndef BT_COMMONFINITESTATES
#define BT_COMMONFINITESTATES

// -----------------------------------------------------------------------------

#include "BT_FiniteStateMachine.h"
#include "BT_ObjectType.h"
#include "BT_BumpObject.h"
#include "BT_FileSystemActor.h"
#include "BT_KeyCombo.h"

// -----------------------------------------------------------------------------

/* 
* Replaces the current selection with those actors specified
*/
class SelectFileSystemActorsState : public FiniteState
{
	QString _fsActorsRegex;
	QString _textToPrint;

public:
	SelectFileSystemActorsState(unsigned int duration, QString regex);
	SelectFileSystemActorsState(unsigned int duration, QString regex, QString textToPrint);

	// FiniteState
	virtual void	onStateChanged();
	virtual bool	finalizeStateChanged();
};


/*
* Zooms into a selection
*/
class ZoomIntoSelectionState : public FiniteState
{
	QString _fsActorsRegex;

public:
	ZoomIntoSelectionState(unsigned int duration, QString regex);

	// FiniteState
	virtual void	onStateChanged();
	virtual bool	finalizeStateChanged();
};

/*
* Zoom Out of Selection
*/
class ZoomOutOfSelectionState : public FiniteState
{
public:
	ZoomOutOfSelectionState(unsigned int duration);

	// FiniteState
	virtual void	onStateChanged();
	virtual bool finalizeStateChanged();
};

/*
* Point Camera to 
*/
class PointCameraToState : public FiniteState
{
	Vec3 _location;
	QString _textToPrint;

public:
	PointCameraToState(unsigned int duration, Vec3 location, QString textToPrint);

	// FiniteState
	virtual void	onStateChanged();
	virtual bool	finalizeStateChanged();
};

/*
* Zoom Camera To Point
*/
class ZoomCameraToPointState : public FiniteState
{
	Vec3 _location;
	QString _textToPrint;

public:
	ZoomCameraToPointState(unsigned int duration, Vec3 location, QString textToPrint);

	// FiniteState
	virtual void	onStateChanged();
	virtual bool	finalizeStateChanged();
};

/*
* Move Camera
*/
class MoveCameraState : public FiniteState
{
	bool _leftNotRight;

public:
	MoveCameraState(unsigned int duration, bool leftNotRight);

	// FiniteState
	virtual void	onStateChanged();
	virtual bool	finalizeStateChanged();
};

/*
* Leaf Through Pile
*/
class LeafThroughPileState : public FiniteState
{
	QString _textToPrint;
public:
	LeafThroughPileState(unsigned int duration);
	LeafThroughPileState(unsigned int duration, QString textToPrint);

	// FiniteState
	virtual void	onStateChanged();
	virtual bool	finalizeStateChanged();
};

/*
* Print a unique timed message
*/
class PrintMessageState : public FiniteState
{
	QString _textToPrint;
	int _durationOfMessage;
public:
	PrintMessageState(unsigned int duration, QString textToPrint, int durationOfMessage);

	// FiniteState
	virtual void	onStateChanged();
	virtual bool	finalizeStateChanged();
};


/* 
* Zooms into a specific image file
*/
class ZoomIntoImageState : public FiniteState
{
	QString _actorRegex;
	QString _textToPrint;

public:
	ZoomIntoImageState(unsigned int duration, QString regex);
	ZoomIntoImageState(unsigned int duration, QString regex, QString textToPrint);

	// FiniteState
	virtual void	onStateChanged();
	virtual bool	finalizeStateChanged();
};

/*
* Zooms to the next image file
*/
class ZoomToNextImageState : public FiniteState
{
public:
	ZoomToNextImageState(unsigned int duration);

	// FiniteState
	virtual bool	prepareStateChanged();
	virtual void	onStateChanged();
	virtual bool	finalizeStateChanged();
};

/*
* Zooms out of the image.
*/
class ZoomOutOfImageState : public FiniteState
{
public:
	ZoomOutOfImageState(unsigned int duration);

	// FiniteState
	virtual bool	prepareStateChanged();
	virtual void	onStateChanged();
	virtual bool	finalizeStateChanged();
};

/*
* Scatter the actors
*/
class ScatterActorsState : public FiniteState
{
	QString _fsActorsRegex;

public:
	ScatterActorsState(unsigned int duration, QString regex);

	// FiniteState
	virtual void	onStateChanged();
	virtual bool	finalizeStateChanged();
};

/*
* Create a new pile with the specified actors.
*/
class CreatePileState : public FiniteState
{
	QString _fsActorsRegex;
	QString _textToPrint;

public:
	CreatePileState(unsigned int duration, QString regex);
	CreatePileState(unsigned int duration, QString regex, QString textToPrint);

	// FiniteState
	virtual void	onStateChanged();
	virtual bool	finalizeStateChanged();
};

/* 
 * Collapses the currently selected pile into a folder
 */ 
class FolderizeSelectedPileState : public FiniteState
{
	QString _textToPrint;
	QString _folderizedName;

public:
	FolderizeSelectedPileState(unsigned int duration, QString folderName = NULL);

	// FiniteState
	virtual bool	prepareStateChanged();
	virtual void	onStateChanged();
	virtual bool	finalizeStateChanged();
};

/* 
* Collapses the currently selected pile into a folder
*/ 
class FolderizeSelectedFilesState : public FiniteState
{
	QString _fsActorsRegex;
	QString _textToPrint;
	QString _folderName;
	bool _fsSimulateCancel;
	vector<FileSystemActor *> _failedMoves;

public:
	FolderizeSelectedFilesState(unsigned int duration, QString regex, QString folderName, bool cancel = false);
	
	// FiniteState
	virtual bool	prepareStateChanged();
	virtual void	onStateChanged();
	virtual bool	finalizeStateChanged();
};

/* 
 * Expands the currently selected folder actor into a hard pile
 */
class PileizeSelectedFileSystemActorState : public FiniteState
{
public:
	PileizeSelectedFileSystemActorState(unsigned int duration);

	// FiniteState
	virtual void	onStateChanged();
	virtual bool	finalizeStateChanged();
};

/*
* Deletes the specified actors.
* NOTE: this may take time to take effect in bumptop, so give a reasonable duration
*/
class DeleteFilesState : public FiniteState
{
	QString _fsActorsRegex;
	bool _fsConfirm, _fsSimulateCancel;
	QMap<FileSystemActor *, bool> _isActorDeleted;

public:
	DeleteFilesState(unsigned int duration, QString regex, bool delBySel = false, bool cancel = false);

	// FiniteState
	virtual void	onStateChanged();
	virtual bool	finalizeStateChanged();
};

/* 
* Grid the selected pile.
*/
class GridSelectedPileState : public FiniteState
{
	QString _textToPrint;
public:
	GridSelectedPileState(unsigned int duration);
	GridSelectedPileState(unsigned int duration, QString textToPrint);

	// FiniteState
	virtual bool	prepareStateChanged();
	virtual void	onStateChanged();
	virtual bool	finalizeStateChanged();
};

/* 
* Selects the pile(s) containing the specified files.
*/
class SelectPilesWithFileSystemActorsState : public FiniteState
{
	QString _fsActorsRegex;

public:
	SelectPilesWithFileSystemActorsState(unsigned int duration, QString regex);

	// FiniteState
	virtual void	onStateChanged();
	virtual bool	finalizeStateChanged();
};

/*
* Stack the selected pile.
*/
class StackSelectedPileState : public FiniteState
{
public:
	StackSelectedPileState(unsigned int duration);

	// FiniteState
	virtual bool	prepareStateChanged();
	virtual void	onStateChanged();
	virtual bool	finalizeStateChanged();
};

/*
* Break the selected pile.
*/
class BreakSelectedPileState : public FiniteState
{
	QString _textToPrint;
public:
	BreakSelectedPileState(unsigned int duration);
	BreakSelectedPileState(unsigned int duration, QString textToPrint);

	// FiniteState
	virtual bool	prepareStateChanged();
	virtual void	onStateChanged();
	virtual bool	finalizeStateChanged();
};

/*
* Remove file from the selected pile.
*/
class RemoveFileSystemActorFromPileState : public FiniteState
{
	QString _fsActorsRegex;
	vector<FileSystemActor *> _demoFiles;

public:
	RemoveFileSystemActorFromPileState(unsigned int duration, QString regex);

	// FiniteState
	virtual bool	prepareStateChanged();
	virtual void	onStateChanged();
	virtual bool	finalizeStateChanged();
};

/* 
* Fanout the selected pile with the specified points.
*/
class FanoutSelectedPileState : public FiniteState
{
	vector<Vec3> _points;

public:
	FanoutSelectedPileState(unsigned int duration, const vector<Vec3>& points);

	// FiniteState
	virtual bool	prepareStateChanged();
	virtual void	onStateChanged();
	virtual bool	finalizeStateChanged();
};

/* 
* Close fanout on a pile with the specified items. (Since you can't select a fanned out pile)
*/
class CloseFannedOutPileState : public FiniteState
{
	QString _fsActorsRegex;

public:
	CloseFannedOutPileState(unsigned int duration, QString regex);

	// FiniteState
	virtual void	onStateChanged();
	virtual bool	finalizeStateChanged();
};

/*
* Sort items by type
*/
class SortFileSystemActorsByTypeState : public FiniteState
{
	QString _fsActorsRegex;
	QString _textToPrint;

public:
	SortFileSystemActorsByTypeState(unsigned int duration, QString regex);
	SortFileSystemActorsByTypeState(unsigned int duration, QString regex, QString textToPrint);

	// FiniteState
	virtual void	onStateChanged();
	virtual bool	finalizeStateChanged();
};

/*
* Grows or shrinks the selected objects
*/
class ResizeActorsState : public FiniteState
{
	QString _fsActorsRegex;
	ObjectType _objType;
	bool _growActors;
	QString _textToPrint;
	bool _animate;

public:
	ResizeActorsState(unsigned int duration, QString regex, const ObjectType& type, bool grow);
	ResizeActorsState(unsigned int duration, QString regex, BumpObjectType type, bool grow);
	ResizeActorsState(unsigned int duration, QString regex, ActorType type, bool grow);
	ResizeActorsState(unsigned int duration, QString regex, ActorType type, bool grow, QString textToPrint, bool animate);

	// FiniteState
	virtual void	onStateChanged();
	virtual bool	finalizeStateChanged();
};

/*
* Highlight actors with a typed in string
*/
class FindAsYouTypeState : public FiniteState
{
	QString _textToPrint;
	KeyCombo _letter;

public:
	FindAsYouTypeState(unsigned int duration, KeyCombo letter ,QString textToPrint);

	// FiniteState
	virtual void	onStateChanged();
	virtual bool	finalizeStateChanged();
};

/*
* Rename the body of a file
*/
class RenameFileState : public FiniteState
{
	QString _newFileName;

public:
	RenameFileState(unsigned int duration, QString newName);

	// FiniteState
	virtual bool	prepareStateChanged();
	virtual void	onStateChanged();
	virtual bool	finalizeStateChanged();
};

// ----------------------------------------------------------------------------

/* 
* Ensures that there is no pile(s) containing the specified files.
*/
class AssertNoPilesWithFileSystemActorsState : public FiniteState
{
	QString _fsActorsRegex;

public:
	AssertNoPilesWithFileSystemActorsState(unsigned int duration, QString regex);

	// FiniteState
	virtual void	onStateChanged();
};

/*
 * Ensures that the specified BumpObjects either exist or don't exist
 */
class AssertBumpObjectExists : public FiniteState
{
	QString _fsActorsRegex;
	ObjectType _objType;
	bool _objExists;

public:
	AssertBumpObjectExists(unsigned int duration, QString regex, const ObjectType& type, bool exists);
	AssertBumpObjectExists(unsigned int duration, QString regex, BumpObjectType type, bool exists);
	AssertBumpObjectExists(unsigned int duration, QString regex, FileSystemActorType type, bool exists);

	// FiniteState
	virtual void	onStateChanged();
};

/*
 * Ensures that there are N of the specified BumpObjects
 */
class AssertBumpObjectCount : public FiniteState
{
	QString _fsActorsRegex;
	ObjectType _objType;
	int _objCount;

public:
	AssertBumpObjectCount(unsigned int duration, QString regex, const ObjectType& type, int count);
	AssertBumpObjectCount(unsigned int duration, QString regex, BumpObjectType type, int count);
	AssertBumpObjectCount(unsigned int duration, QString regex, FileSystemActorType type, int count);
	AssertBumpObjectCount(unsigned int duration, BumpObjectType type, int count);

	// FiniteState
	virtual void	onStateChanged();
};

/*
 * Ensures the state of the slideshow mode
 */
class AssertSlideShowEnabled : public FiniteState
{
	bool _expectedEnabled;

public:
	AssertSlideShowEnabled(unsigned int duration, bool shouldBeEnabled);

	// FiniteState
	virtual void	onStateChanged();
};

/*
* Ensures the specified items are of the specified size
*/
class AssertBumpObjectDimensions : public FiniteState
{
public:
	enum DimensionComparisonType {
		LessThan,
		EqualTo,	// within an epsilon
		GreaterThan
	};

private:
	QString _fsActorsRegex;
	DimensionComparisonType _comparisonType;
	Vec3 _expectedDims;
	ObjectType _objType;

public:
	AssertBumpObjectDimensions(unsigned int duration, QString regex, const ObjectType& type, const Vec3& expected, DimensionComparisonType cmptype);

	// FiniteState
	virtual void	onStateChanged();
};

class AssertNumberOfFileSystemActors : public FiniteState
{
	int _expectedNumberOfActors;

public:
	AssertNumberOfFileSystemActors(unsigned int duration, int numberOfActors);

	// FiniteState
	virtual void	onStateChanged();
};

#endif // BT_COMMONFINITESTATES