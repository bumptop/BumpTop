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
#include "BT_Camera.h"
#include "BT_CommonFiniteStates.h"
#include "BT_FileSystemManager.h"
#include "BT_FileSystemPile.h"
#include "BT_KeyboardManager.h"
#include "BT_OverlayComponent.h"
#include "BT_Pile.h"
#include "BT_SceneManager.h"
#include "BT_Selection.h"
#include "BT_Util.h"
#include "BT_WindowsOS.h"

// -----------------------------------------------------------------------------

#define THROW_RUNTIME_ERROR(errMsg) throw runtime_error("["__FUNCTION__"]" errMsg " (" + _testFileName + ", " + QString::number(_testLineNumber).toStdString() + ")")

SelectFileSystemActorsState::SelectFileSystemActorsState(unsigned int duration, QString regex)
: FiniteState(duration)
, _fsActorsRegex(regex)
{}

SelectFileSystemActorsState::SelectFileSystemActorsState(unsigned int duration, QString regex, QString textToPrint)
: FiniteState(duration)
, _fsActorsRegex(regex)
, _textToPrint(textToPrint)
{}

void SelectFileSystemActorsState::onStateChanged()
{
	// select the actors
	sel->clear();
	vector<FileSystemActor *> demoFiles = scnManager->getFileSystemActors(_fsActorsRegex, false, true);
	for (int i = 0; i < demoFiles.size(); ++i)
	{
		sel->add(demoFiles[i]);
	}

	printTimedUnique("SelectFileSystemActorState", 3, _textToPrint);
}

bool SelectFileSystemActorsState::finalizeStateChanged()
{
	// INSERT LOGIC TO VERIFY STATE
	return true;
}

// -----------------------------------------------------------------------------

ZoomIntoSelectionState::ZoomIntoSelectionState (unsigned int duration, QString regex)
: FiniteState(duration)
, _fsActorsRegex(regex)
{}

void ZoomIntoSelectionState::onStateChanged()
{
	// select the actors
	if (!_fsActorsRegex.isEmpty())
	{
		sel->clear();
		vector<FileSystemActor *> demoFiles = scnManager->getFileSystemActors(_fsActorsRegex, false, true);
		for (int i = 0; i < demoFiles.size(); ++i)
		{
			sel->add(demoFiles[i]);
		}
	}
	
	Bounds zoomBounds = cam->getActorsBounds(sel->getBumpObjects());
	cam->animateQuadratic(calculateEyeForBounds(zoomBounds), Vec3(0, 500, 0), Vec3(0,-1,0), Vec3(0,0,1), 60);
}

bool ZoomIntoSelectionState::finalizeStateChanged()
{
	// INSERT LOGIC TO VERIFY STATE
	return true;
}

// -----------------------------------------------------------------------------

ZoomOutOfSelectionState::ZoomOutOfSelectionState(unsigned int duration)
: FiniteState(duration)
{}

void ZoomOutOfSelectionState::onStateChanged()
{
	sel->clear();
	cam->popWatchActors();
	cam->zoomToAngledView();
}

bool ZoomOutOfSelectionState::finalizeStateChanged()
{
	// INSERT LOGIC TO VERIFY STATE
	return true;
}

// -----------------------------------------------------------------------------

PointCameraToState::PointCameraToState(unsigned int duration, Vec3 location, QString textToPrint)
: FiniteState(duration)
, _location(location)
, _textToPrint(textToPrint)
{}

void PointCameraToState::onStateChanged()
{
	cam->pointCameraTo(_location, false);
	printTimedUnique("ZoomCameraToPointState", 3, _textToPrint);
}

bool PointCameraToState::finalizeStateChanged()
{
	// INSERT LOGIC TO VERIFY STATE
	return true;
}

// -----------------------------------------------------------------------------

ZoomCameraToPointState::ZoomCameraToPointState(unsigned int duration, Vec3 location, QString textToPrint)
: FiniteState(duration)
, _location(location)
, _textToPrint(textToPrint)
{}

void ZoomCameraToPointState::onStateChanged()
{
	cam->scrollToPoint(_location);
	printTimedUnique("ZoomCameraToPointState", 3, _textToPrint);
}

bool ZoomCameraToPointState::finalizeStateChanged()
{
	// INSERT LOGIC TO VERIFY STATE
	return true;
}

// -----------------------------------------------------------------------------

MoveCameraState::MoveCameraState(unsigned int duration, bool leftNotRight)
: FiniteState(duration)
, _leftNotRight(leftNotRight)
{}

void MoveCameraState::onStateChanged()
{
	if (_leftNotRight)
		Key_MoveCameraLeft();
	else
		Key_MoveCameraRight();
}

bool MoveCameraState::finalizeStateChanged()
{
	// INSERT LOGIC TO VERIFY STATE
	return true;
}
// -----------------------------------------------------------------------------

LeafThroughPileState::LeafThroughPileState(unsigned int duration)
: FiniteState(duration)
{}

LeafThroughPileState::LeafThroughPileState(unsigned int duration, QString textToPrint)
: FiniteState(duration)
, _textToPrint(textToPrint)
{}
void LeafThroughPileState::onStateChanged()
{
	vector<BumpObject *> selection = sel->getBumpObjects();
	if (selection.size() == 1)
	{
		Pile * p = dynamic_cast<Pile *>(selection.front());
		if (!p) 
			p = dynamic_cast<Pile *>(selection.front()->getParent());
		if (p)
		{
			if (p->getPileState() == Leaf || p->getPileState() == Stack)
			{
				p->leafDown();
			}
		}
	}

	printTimedUnique("LeafThroughPile", 3, _textToPrint);
}

bool LeafThroughPileState::finalizeStateChanged()
{
	// INSERT LOGIC TO VERIFY STATE
	return true;
}
// -----------------------------------------------------------------------------

PrintMessageState::PrintMessageState(unsigned int duration, QString textToPrint, int durationOfMessage)
: FiniteState(duration)
, _textToPrint(textToPrint)
, _durationOfMessage(durationOfMessage)
{}
void PrintMessageState::onStateChanged()
{
	printTimedUnique("PrintMessageState", _durationOfMessage, _textToPrint);
}

bool PrintMessageState::finalizeStateChanged()
{
	// INSERT LOGIC TO VERIFY STATE
	return true;
}
// -----------------------------------------------------------------------------

ZoomIntoImageState::ZoomIntoImageState(unsigned int duration, QString regex)
: FiniteState(duration)
, _actorRegex(regex)
{}

ZoomIntoImageState::ZoomIntoImageState(unsigned int duration, QString regex, QString textToPrint)
: FiniteState(duration)
, _actorRegex(regex)
, _textToPrint(textToPrint)
{}

void ZoomIntoImageState::onStateChanged()
{
	// Print description message
	printTimedUnique("ZoomIntoImageState", 3, _textToPrint);
	// launch the first actor
	vector<FileSystemActor *> demoFiles = scnManager->getFileSystemActors(_actorRegex, false, true);
	assert(!demoFiles.empty());
	Key_ToggleSlideShow();
	if (!_textToPrint.isNull() || !_textToPrint.isEmpty())
		dismiss("Key_ToggleSlideShow");
}

bool ZoomIntoImageState::finalizeStateChanged()
{
	// INSERT LOGIC TO VERIFY STATE
	return true;
}

// -----------------------------------------------------------------------------

ZoomToNextImageState::ZoomToNextImageState(unsigned int duration)
: FiniteState(duration)
{}

bool ZoomToNextImageState::prepareStateChanged()
{
	bool result = isSlideshowModeActive();
	if(!result)
		THROW_RUNTIME_ERROR("Slideshow Mode is not active!");
	return result;
}

void ZoomToNextImageState::onStateChanged()
{
	// launch the first actor
	cam->highlightNextWatchedActor(true);
}

bool ZoomToNextImageState::finalizeStateChanged()
{
	// INSERT LOGIC TO VERIFY STATE
	return true;
}

// -----------------------------------------------------------------------------

ZoomOutOfImageState::ZoomOutOfImageState(unsigned int duration)
: FiniteState(duration)
{}

bool ZoomOutOfImageState::prepareStateChanged()
{
	bool result = isSlideshowModeActive();
	if(!result)
		THROW_RUNTIME_ERROR("Slideshow Mode is not active!");
	return result;
}

void ZoomOutOfImageState::onStateChanged()
{
	// launch the first actor
	Key_DisableSlideShow();
}

bool ZoomOutOfImageState::finalizeStateChanged()
{
	// INSERT LOGIC TO VERIFY STATE
	return true;
}

// -----------------------------------------------------------------------------

ScatterActorsState::ScatterActorsState(unsigned int duration, QString regex)
: FiniteState(duration)
, _fsActorsRegex(regex)
{}

void ScatterActorsState::onStateChanged()
{
	// give each actor a random velocity away from the origin
	vector<FileSystemActor *> demoFiles = scnManager->getFileSystemActors(_fsActorsRegex, false, true);
	srand((unsigned int) time(NULL));
	for (int i = 0; i < demoFiles.size(); ++i)
	{
		float randXVel = float(rand() % 250) - 125.0f;
		float randZVel = float(rand() % 250) - 125.0f;
		demoFiles[i]->setLinearVelocity(Vec3(randXVel, 0, randZVel));
	}
}

bool ScatterActorsState::finalizeStateChanged()
{
	// INSERT LOGIC TO VERIFY STATE
	return true;
}

// -----------------------------------------------------------------------------

CreatePileState::CreatePileState(unsigned int duration, QString regex)
: FiniteState(duration)
, _fsActorsRegex(regex)
{}

CreatePileState::CreatePileState(unsigned int duration, QString regex, QString textToPrint)
: FiniteState(duration)
, _fsActorsRegex(regex)
, _textToPrint(textToPrint)
{}

void CreatePileState::onStateChanged()
{
	// select the actors
	sel->clear();
	vector<FileSystemActor *> demoFiles = scnManager->getFileSystemActors(_fsActorsRegex, false, true);
	for (int i = 0; i < demoFiles.size(); ++i)
	{
		sel->add(demoFiles[i]);
	}

	printTimedUnique("CreatePileState", 3, _textToPrint);

	Key_MakePile();
}

bool CreatePileState::finalizeStateChanged()
{
	// Verify every actor that satisfies _fsActorsRegex all have parents, and are all children to the same
	// pile node

	// Get list of file actors
	vector<FileSystemActor *> demoFiles = scnManager->getFileSystemActors(_fsActorsRegex, false, true);
	BumpObject *parent = NULL;

	// Determine if the objects have a parent
	if (demoFiles.size() > 0)
		parent = demoFiles[0]->getParent();

	// Iterate through every object and see if they have a parent or a different kind of pattern
	for (int i = 0;i<demoFiles.size();i++)
	{
		FileSystemActor *fsActor = demoFiles[i];
		if (fsActor->getParent() == NULL || fsActor->getParent() != parent)
		{
			// Write error to a file
			append_file_utf8("Failed to pile items", winOS->GetDataDirectory().absoluteFilePath("ErrorLog.txt"));
			THROW_RUNTIME_ERROR("Failed to pile items");
		}
	}
	return true;
}

// -----------------------------------------------------------------------------

FolderizeSelectedPileState::FolderizeSelectedPileState(unsigned int duration, QString folderName)
: FiniteState(duration)
, _folderizedName(folderName)
{}

bool FolderizeSelectedPileState::prepareStateChanged()
{
	vector<Pile *> selection = sel->getFullPiles();
	if(!selection.size())
		THROW_RUNTIME_ERROR("No piles have been selected");
	return true;
}

void FolderizeSelectedPileState::onStateChanged()
{
	vector<Pile *> selection = sel->getFullPiles();
	for (uint i = 0; i < selection.size(); i++)
	{
		Pile *pile = (Pile *) selection[i];

		if (selection[i]->getPileType() == HardPile)
		{
			FileSystemPile *fsPile = (FileSystemPile *) pile;
			fsPile->folderize();
		}
		else if (selection[i]->getPileType() == SoftPile)
		{
			SoftPileToFolderIcon(pile, _folderizedName);
		}
	}
	selection.clear();
}

bool FolderizeSelectedPileState::finalizeStateChanged()
{
	// INSERT LOGIC TO VERIFY STATE
	return true;
}

// -----------------------------------------------------------------------------

FolderizeSelectedFilesState::FolderizeSelectedFilesState(unsigned int duration, QString regex, QString folderName, bool cancel)
: FiniteState(duration)
, _fsActorsRegex(regex)
, _folderName(folderName)
, _fsSimulateCancel(cancel)
{}

bool FolderizeSelectedFilesState::prepareStateChanged()
{
	int size = scnManager->getFileSystemActors(_fsActorsRegex, false, true).size();
	if(!size)
		THROW_RUNTIME_ERROR("Expected files not found!");
	return true;
}

void FolderizeSelectedFilesState::onStateChanged()
{
	// Move files to the folder name located on the desktop
	vector<FileSystemActor *> demoFiles = scnManager->getFileSystemActors(_fsActorsRegex, false, true);
	QString folderPath = scnManager->getWorkingDirectory().absoluteFilePath(_folderName);
	folderPath = QDir::toNativeSeparators(folderPath);
	if(_fsSimulateCancel)
		_failedMoves = demoFiles;
	else
		fsManager->moveFiles(demoFiles, folderPath, _failedMoves);
}

bool FolderizeSelectedFilesState::finalizeStateChanged()
{
	if(_failedMoves.size())
		THROW_RUNTIME_ERROR("Certain files could not be moved to the folder");

	return true;
}

// -----------------------------------------------------------------------------
PileizeSelectedFileSystemActorState::PileizeSelectedFileSystemActorState(unsigned int duration)
: FiniteState(duration)
{}

void PileizeSelectedFileSystemActorState::onStateChanged()
{
	vector<BumpObject *> demoFiles = sel->getBumpObjects();
	assert(demoFiles.size() == 1);

	Key_ExpandToPile();	
}

bool PileizeSelectedFileSystemActorState::finalizeStateChanged()
{
	// INSERT LOGIC TO VERIFY STATE
	return true;
}

// -----------------------------------------------------------------------------

DeleteFilesState::DeleteFilesState(unsigned int duration, QString regex, bool confirmDialogue, bool cancel)
: FiniteState(duration)
, _fsActorsRegex(regex)
, _fsConfirm(confirmDialogue)
, _fsSimulateCancel(cancel)
{}

void DeleteFilesState::onStateChanged()
{
	// select the actors
	vector<FileSystemActor *> demoFiles = scnManager->getFileSystemActors(_fsActorsRegex, false, true);
	for (int i = 0; i < demoFiles.size(); ++i)
	{
		QString filePath = demoFiles[i]->getFullPath();
		// Set value of QMap to true if actor is supposed to be deleted
		if(exists(filePath) && _fsSimulateCancel)
			_isActorDeleted.insert(demoFiles[i], false);
		else if(exists(filePath) && !_fsConfirm)
			_isActorDeleted.insert(demoFiles[i], fsManager->deleteFileByName(filePath)); // Automatically confirm the delete dialog box
		else if(exists(filePath) && _fsConfirm)
			_isActorDeleted.insert(demoFiles[i], fsManager->deleteFileByName(filePath, false, false, true)); // Prompt the user with the delete dialog box
	}
}

bool DeleteFilesState::finalizeStateChanged()
{
	vector<FileSystemActor *> demoFiles = scnManager->getFileSystemActors(_fsActorsRegex, false, true);
	// If actor is found in the scene and it was supposed to be deleted, then throw error
	for (int i = 0; i < demoFiles.size(); ++i)
	{
		if (_isActorDeleted.value(demoFiles[i]))
			THROW_RUNTIME_ERROR("Could not delete all files specified!");
	}
	return true;
}

// -----------------------------------------------------------------------------

GridSelectedPileState::GridSelectedPileState(unsigned int duration)
: FiniteState(duration)
{}

GridSelectedPileState::GridSelectedPileState(unsigned int duration, QString textToPrint)
: FiniteState(duration)
, _textToPrint(textToPrint)
{}

bool GridSelectedPileState::prepareStateChanged()
{
	// ensure that there is one item in the selection and that it is a pile
	vector<Pile *> selection = sel->getFullPiles();
	if(selection.size() != 1)
		THROW_RUNTIME_ERROR("Must select only one pile!");
	return (selection.size() == 1);
}

void GridSelectedPileState::onStateChanged()
{
	// grid the selected pile
	Key_GridView();
	printTimedUnique("GridSelectedPileState", 3, _textToPrint);
}

bool GridSelectedPileState::finalizeStateChanged()
{
	// INSERT LOGIC TO VERIFY STATE
	return true;
}

// -----------------------------------------------------------------------------

StackSelectedPileState::StackSelectedPileState(unsigned int duration)
: FiniteState(duration)
{}

bool StackSelectedPileState::prepareStateChanged()
{
	// ensure that there is one item in the selection and that it is a pile
	vector<Pile *> selection = sel->getFullPiles();
	if(selection.size() != 1)
		THROW_RUNTIME_ERROR("Must select only one pile");
	return (selection.size() == 1);
}

void StackSelectedPileState::onStateChanged()
{
	// stack the selected pile
	Key_StackPile();
}

bool StackSelectedPileState::finalizeStateChanged()
{
	// INSERT LOGIC TO VERIFY STATE
	return true;
}

// -----------------------------------------------------------------------------

BreakSelectedPileState::BreakSelectedPileState(unsigned int duration)
: FiniteState(duration)
{}

BreakSelectedPileState::BreakSelectedPileState(unsigned int duration, QString textToPrint)
: FiniteState(duration)
, _textToPrint(textToPrint)
{}

bool BreakSelectedPileState::prepareStateChanged()
{
	// ensure that there is one item in the selection and that it is a pile
	vector<Pile *> selection = sel->getFullPiles();
	if(!selection.size())
		THROW_RUNTIME_ERROR("No piles were selected!");
	return (!selection.empty());
}

void BreakSelectedPileState::onStateChanged()
{
	// break the selected pile
	Key_BreakPile();
	printTimedUnique("BreakSelectedPileState", 3, _textToPrint);
}

bool BreakSelectedPileState::finalizeStateChanged()
{
	// INSERT LOGIC TO VERIFY STATE
	return true;
}

// -----------------------------------------------------------------------------

RemoveFileSystemActorFromPileState::RemoveFileSystemActorFromPileState(unsigned int duration, QString regex)
: FiniteState(duration)
, _fsActorsRegex(regex)
{}

bool RemoveFileSystemActorFromPileState::prepareStateChanged()
{
	// Select the actors
	_demoFiles = scnManager->getFileSystemActors(_fsActorsRegex, false, true);
	for (int i = 0; i < _demoFiles.size(); ++i)
	{
		// Check to see that each file selected belongs to a pile
		BumpObject * parent = _demoFiles[i]->getParent();
		if (parent->getObjectType() != BumpPile)
			THROW_RUNTIME_ERROR("Must select file that is contained in a pile!");
	}

	return true;
}

void RemoveFileSystemActorFromPileState::onStateChanged()
{
	for (int i = 0; i < _demoFiles.size(); ++i)
	{
		Pile * pile = dynamic_cast<Pile *>(_demoFiles[i]->getParent());
		if(pile->getPileType() == SoftPile)
			assert(pile->getNumItems() > 1);
		else
			assert(pile->getNumItems() > 0);
		// Soft piles can't have only one item left, however hard piles can
		if(pile->getPileType() == SoftPile && pile->getNumItems() == 2)
		{
			pile->removeFromPile(_demoFiles[i]);
			break;
		}
		else
			pile->removeFromPile(_demoFiles[i]);
	}
}

bool RemoveFileSystemActorFromPileState::finalizeStateChanged()
{
	// INSERT LOGIC TO VERIFY STATE
	return true;
}

// -----------------------------------------------------------------------------

FanoutSelectedPileState::FanoutSelectedPileState(unsigned int duration, const vector<Vec3>& points)
: FiniteState(duration)
, _points(points)
{}

bool FanoutSelectedPileState::prepareStateChanged()
{
	// ensure that there is one item in the selection and that it is a pile
	vector<Pile *> selection = sel->getFullPiles();
	if(selection.size() != 1)
		THROW_RUNTIME_ERROR("Must select only one pile!");
	return (selection.size() == 1);
}

void FanoutSelectedPileState::onStateChanged()
{
	// fanout the selected pile
	Pile * pile = sel->getFullPiles().front();
	Vec3 pilePos = pile->getGlobalPosition();
	pile->beginFanout();
	for (int i = 0; i < _points.size(); ++i)
		pile->fanoutTick(pilePos + _points[i]);
	pile->endFanout();
}

bool FanoutSelectedPileState::finalizeStateChanged()
{
	// INSERT LOGIC TO VERIFY STATE
	return true;
}

// -----------------------------------------------------------------------------

CloseFannedOutPileState::CloseFannedOutPileState(unsigned int duration, QString regex)
: FiniteState(duration)
, _fsActorsRegex(regex)
{}

void CloseFannedOutPileState::onStateChanged()
{
	// stack each of the piles
	vector<FileSystemActor *> demoFiles = scnManager->getFileSystemActors(_fsActorsRegex, false, true);
	for (int i = 0; i < demoFiles.size(); ++i)
	{
		if (demoFiles[i]->getParent())
		{
			Pile * pile = dynamic_cast<Pile *>(demoFiles[i]->getParent());
			if (pile)
				pile->close();
		}
	}

}

bool CloseFannedOutPileState::finalizeStateChanged()
{
	// INSERT LOGIC TO VERIFY STATE
	return true;
}

// -----------------------------------------------------------------------------

SelectPilesWithFileSystemActorsState::SelectPilesWithFileSystemActorsState(unsigned int duration, QString regex)
: FiniteState(duration)
, _fsActorsRegex(regex)
{}

void SelectPilesWithFileSystemActorsState::onStateChanged()
{
	vector<FileSystemActor *> demoFiles;
	vector<Pile *> piles;
	sel->clear();
	if (_fsActorsRegex.isEmpty())
	{
		piles = scnManager->getPiles();
		for (int i = 0;i<piles.size();i++)
			sel->add(piles[i]);
	}
	else
	{
		// select the actors
		demoFiles = scnManager->getFileSystemActors(_fsActorsRegex, false, true);
		for (int i = 0; i < demoFiles.size(); ++i)
		{
			BumpObject * parent = demoFiles[i]->getParent();
			if (parent && !sel->isInSelection(parent))
			{
				sel->setPickedActor(parent);
				sel->add(parent);
			}
		}
	}
	sel->setPickedActor(NULL);
}

bool SelectPilesWithFileSystemActorsState::finalizeStateChanged()
{
	// INSERT LOGIC TO VERIFY STATE
	return true;
}

// -----------------------------------------------------------------------------

SortFileSystemActorsByTypeState::SortFileSystemActorsByTypeState(unsigned int duration, QString regex)
: FiniteState(duration)
, _fsActorsRegex(regex)
{}

SortFileSystemActorsByTypeState::SortFileSystemActorsByTypeState(unsigned int duration, QString regex, QString textToPrint)
: FiniteState(duration)
, _fsActorsRegex(regex)
, _textToPrint(textToPrint)
{}
void SortFileSystemActorsByTypeState::onStateChanged()
{
	printTimedUnique("SortFileSystemActorsByTypeState", 3, _textToPrint);

	// select the actors
	sel->clear();
	vector<FileSystemActor *> demoFiles = scnManager->getFileSystemActors(_fsActorsRegex, false, true);
	for (int i = 0; i < demoFiles.size(); ++i)
	{
		sel->add(demoFiles[i]);
	}

	Key_SortIntoPilesByType();
	dismiss("Key_SortIntoPilesByType");
}

bool SortFileSystemActorsByTypeState::finalizeStateChanged()
{
	// INSERT LOGIC TO VERIFY STATE
	return true;
}

// -----------------------------------------------------------------------------

ResizeActorsState::ResizeActorsState( unsigned int duration, QString regex, const ObjectType& type, bool grow )
: FiniteState(duration)
, _fsActorsRegex(regex)
, _objType(type)
, _growActors(grow)
, _animate(false)
{}

ResizeActorsState::ResizeActorsState( unsigned int duration, QString regex, BumpObjectType type, bool grow )
: FiniteState(duration)
, _fsActorsRegex(regex)
, _objType(ObjectType(type))
, _growActors(grow)
, _animate(false)
{}

ResizeActorsState::ResizeActorsState( unsigned int duration, QString regex, ActorType type, bool grow )
: FiniteState(duration)
, _fsActorsRegex(regex)
, _objType(ObjectType(BumpActor, type))
, _growActors(grow)
, _animate(false)
{}

ResizeActorsState::ResizeActorsState( unsigned int duration, QString regex, ActorType type, bool grow, QString textToPrint, bool animate)
: FiniteState(duration)
, _fsActorsRegex(regex)
, _objType(ObjectType(BumpActor, type))
, _growActors(grow)
, _textToPrint(textToPrint)
, _animate (animate)
{}

void ResizeActorsState::onStateChanged()
{
	vector<BumpObject *> objects = scnManager->getBumpObjects(_fsActorsRegex, _objType, true);
	for (int i = 0; i < objects.size(); ++i)
	{
		if (_growActors)
			objects[i]->grow();
		else
			objects[i]->shrink();
		
		if (!_animate)
			animManager->finishAnimation(objects[i]);
	}

	printTimedUnique("ResizeActorState", 3, _textToPrint);
}

bool ResizeActorsState::finalizeStateChanged()
{
	// INSERT LOGIC TO VERIFY STATE
	return true;
}

// -----------------------------------------------------------------------------

FindAsYouTypeState::FindAsYouTypeState(unsigned int duration, KeyCombo letter ,QString textToPrint)
: FiniteState(duration)
, _letter(letter)
, _textToPrint(textToPrint)
{}
void FindAsYouTypeState::onStateChanged()
{
	keyManager->onKeyDown(_letter);
	printTimedUnique("FindAsYouTypeState", 3, _textToPrint);
}

bool FindAsYouTypeState::finalizeStateChanged()
{
	// INSERT LOGIC TO VERIFY STATE
	return true;
}

// -----------------------------------------------------------------------------

RenameFileState::RenameFileState(unsigned int duration, QString newName)
: FiniteState(duration)
, _newFileName(newName)
{}

bool RenameFileState::prepareStateChanged()
{
	// Ensure that there is one item in the selection and that it is a file system actor
	vector<BumpObject *> selection = sel->getBumpObjects();

	if(selection.size() != 1)
		THROW_RUNTIME_ERROR("Must select only one file system actor!");
	else if(!selection.front()->isBumpObjectType(BumpActor))
		THROW_RUNTIME_ERROR("Selected bump object must be a bump actor!");
	else if(!dynamic_cast<Actor *>(selection.front())->isActorType(FileSystem))
		THROW_RUNTIME_ERROR("Selected bump actor must be a file system actor!");

	return true;
}

void RenameFileState::onStateChanged()
{
	vector<BumpObject *> selection = sel->getBumpObjects();
	FileSystemActor * fsActor = dynamic_cast<FileSystemActor *>(selection.front());
	fsManager->renameFile(fsActor, _newFileName);
}

bool RenameFileState::finalizeStateChanged()
{
	// INSERT LOGIC TO VERIFY STATE
	return true;
}

// -----------------------------------------------------------------------------

AssertNoPilesWithFileSystemActorsState::AssertNoPilesWithFileSystemActorsState(unsigned int duration, QString regex)
: FiniteState(duration)
, _fsActorsRegex(regex)
{}

void AssertNoPilesWithFileSystemActorsState::onStateChanged()
{
	vector<FileSystemActor *> demoFiles = scnManager->getFileSystemActors(_fsActorsRegex, false, true);
	for (int i = 0; i < demoFiles.size(); ++i)
	{
		BumpObject * parent = demoFiles[i]->getParent();
		if (parent)
			THROW_RUNTIME_ERROR("Found pile containing specified file actors!");
	}
}

// -----------------------------------------------------------------------------

AssertBumpObjectExists::AssertBumpObjectExists( unsigned int duration, QString regex, const ObjectType& type, bool exists )
: FiniteState(duration)
, _fsActorsRegex(regex)
, _objType(type)
, _objExists(exists)
{}

AssertBumpObjectExists::AssertBumpObjectExists( unsigned int duration, QString regex, BumpObjectType type, bool exists )
: FiniteState(duration)
, _fsActorsRegex(regex)
, _objType(ObjectType(type))
, _objExists(exists)
{}

AssertBumpObjectExists::AssertBumpObjectExists( unsigned int duration, QString regex, FileSystemActorType type, bool exists )
: FiniteState(duration)
, _fsActorsRegex(regex)
, _objType(ObjectType(BumpActor, FileSystem, type))
, _objExists(exists)
{}

void AssertBumpObjectExists::onStateChanged()
{
	// check if any actors of the specified regex and type exists as specified
	vector<BumpObject *> objects = scnManager->getBumpObjects(_fsActorsRegex, _objType, true);
	if (_objExists && objects.empty())
		THROW_RUNTIME_ERROR("Expected files not found!");
	else if (!_objExists && !objects.empty())
		THROW_RUNTIME_ERROR("Unexpected files found!");
}

// -----------------------------------------------------------------------------

AssertBumpObjectCount::AssertBumpObjectCount( unsigned int duration, QString regex, const ObjectType& type, int count )
: FiniteState(duration)
, _fsActorsRegex(regex)
, _objType(type)
, _objCount(count)
{}

AssertBumpObjectCount::AssertBumpObjectCount( unsigned int duration, QString regex, BumpObjectType type, int count )
: FiniteState(duration)
, _fsActorsRegex(regex)
, _objType(ObjectType(type))
, _objCount(count)
{}

AssertBumpObjectCount::AssertBumpObjectCount( unsigned int duration, QString regex, FileSystemActorType type, int count )
: FiniteState(duration)
, _fsActorsRegex(regex)
, _objType(ObjectType(BumpActor, FileSystem, type))
, _objCount(count)
{}

AssertBumpObjectCount::AssertBumpObjectCount( unsigned int duration, BumpObjectType type, int count )
: FiniteState(duration)
, _objType(ObjectType(type))
, _objCount(count)
{}

void AssertBumpObjectCount::onStateChanged()
{
	// ensure that the number of actors of the specified type with the specified name is equal to the count specified
	vector<BumpObject *> objects = scnManager->getBumpObjects(_fsActorsRegex, _objType, true);
	if (objects.size() != _objCount)
		THROW_RUNTIME_ERROR("Unexpected number of files found!");
}

// -----------------------------------------------------------------------------

AssertSlideShowEnabled::AssertSlideShowEnabled( unsigned int duration, bool shouldBeEnabled )
: FiniteState(duration)
, _expectedEnabled(shouldBeEnabled)
{}

void AssertSlideShowEnabled::onStateChanged()
{
	// check if the slide show is enabled or not
	if (!_expectedEnabled && cam->isWatchingActors())
		THROW_RUNTIME_ERROR("Slideshow is unexpectedly enabled!");
	else if (_expectedEnabled && !cam->isWatchingActors())
		THROW_RUNTIME_ERROR("Slideshow is unexpectedly disabled!");
}

// -----------------------------------------------------------------------------

AssertBumpObjectDimensions::AssertBumpObjectDimensions( unsigned int duration, QString regex, const ObjectType& type, const Vec3& dims, DimensionComparisonType cmptype )
: FiniteState(duration)
, _fsActorsRegex(regex)
, _expectedDims(dims)
, _objType(type)
, _comparisonType(cmptype)
{}

void AssertBumpObjectDimensions::onStateChanged()
{
	const float epsilon = 0.005f;
	float expectedMagnitude = _expectedDims.magnitude();
	vector<BumpObject *> objects = scnManager->getBumpObjects(_fsActorsRegex, _objType, true);
	for (int i = 0; i < objects.size(); ++i)
	{
		float magnitude = objects[i]->getDims().magnitude();
		switch (_comparisonType)
		{
		case AssertBumpObjectDimensions::EqualTo:
			if (abs(magnitude - expectedMagnitude) > epsilon)
				THROW_RUNTIME_ERROR("Expected equal object dimensions!");
			break;
		case AssertBumpObjectDimensions::LessThan:
			if (!(magnitude < expectedMagnitude))
				THROW_RUNTIME_ERROR("Expected smaller object dimensions!");
			break;
		case AssertBumpObjectDimensions::GreaterThan:
			if (!(magnitude > expectedMagnitude))
				THROW_RUNTIME_ERROR("Expected larger object dimensions!");
			break;
		default:
			break;
		}
	}
}

// -----------------------------------------------------------------------------

AssertNumberOfFileSystemActors::AssertNumberOfFileSystemActors(unsigned int duration, int numberOfActors)
: FiniteState(duration)
, _expectedNumberOfActors(numberOfActors)
{}

void AssertNumberOfFileSystemActors::onStateChanged()
{
	int num = scnManager->getFileSystemActors().size();
	if(num > _expectedNumberOfActors)
		THROW_RUNTIME_ERROR("A greater number of actors than expected was found!");
	else if(num < _expectedNumberOfActors)
		THROW_RUNTIME_ERROR("A lesser number of actors than expected was found!");
}