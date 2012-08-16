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
#include "BT_FileSystemPile.h"
#include "BT_FileSystemManager.h"
#include "BT_SceneManager.h"
#include "BT_DialogManager.h"
#include "BT_AnimationManager.h"
#include "BT_AnimationEntry.h"
#include "BT_Util.h"
#include "BT_Selection.h"
#include "BT_StatsManager.h"
#include "BT_RenderManager.h"
#include "BT_WindowsOS.h"
#include "BT_FileSystemActorFactory.h"
#include "BT_GLTextureManager.h"

FileSystemPile::FileSystemPile()
: _folderizeOnClose(false)
{
	// Init
	owner = NULL;
	setPileType(HardPile);
	displayLaunchOwnerWidget = true;
}

FileSystemPile::~FileSystemPile()
{
	fsManager->removeObject(this);
	if (winOS->IsWindowsVersion(WindowsXP) && getPileOwnerPath() != "") 
		texMgr->removeWinThumbnailSearchPath(getPileOwnerPath());
}

bool FileSystemPile::addToPile(BumpObject *obj)
{
	LOG_LINE_REACHED();
	
	Actor *aData = NULL;
	FileSystemActor *fsData = NULL;
	bool addRc = false;
	QString fullPath;
	vector<FileSystemActor *> objList, failedList, replacedList;
	vector<FileSystemActorRename> renamedList;
	QString targetDir;

	if (isInPile(obj) == -1)
	{
		if (obj->isBumpObjectType(BumpActor))
		{
			aData = (Actor *) obj;

			if (aData->isActorType(FileSystem))
			{
				fsData = (FileSystemActor *) obj;
				fullPath = fsData->getFullPath();
				objList.push_back(fsData);

				// Move Item from the parent directory into this directory
				if (fsData->isFileSystemType(Virtual))
				{
					// Virtual folders are not allowed in hard piles
					return false;
				}

				if (owner)
				{
					targetDir = owner->getTargetPath();

					if (!fsManager->moveFiles(objList, targetDir, failedList, replacedList, renamedList) || !failedList.empty())
					{
						return false;
					}

					QString fileName = fsData->getFileName(false);
					
					// If we renamed, set the new filename to the renamed name
					if (!renamedList.empty())
						fileName = renamedList.front().newFileName;
					
					// Set the filename of the added object so that the listener doesn't collide
					fsData->setFilePath(native(make_file(targetDir, fileName)), true);
				}

				// Call the parent code
				addRc = Pile::addToPile(obj);

				// If this line Fails, its because something went wrong inside Pile::addToPile()
				assert(addRc == true);

				// Success
				return true;
			}
		}
		else if (obj->isBumpObjectType(BumpPile))
		{
			Pile * pile = (Pile *) obj;

			// Decompose the first pile and add it to this pile
			while ( pile->getNumItems() > 0)
			{
				obj = (*pile)[0];

				pile->removeFromPile(obj, NoUpdate);
				addToPile(obj);
			}

			pile->updatePileState();

			return true;
		}
	}

	return false;
}

bool FileSystemPile::removeFromPile(BumpObject *obj, UpdateFlag updateFlag)
{
	LOG_LINE_REACHED();
	
	bool remRc = false;
	FileSystemActor	*fsObj;
	vector<FileSystemActor *> objList, failedObj, replacedObj;
	vector<FileSystemActorRename> renamedObj;

	if (isInPile(obj) > -1)
	{
		fsObj = (FileSystemActor *) obj;
		objList.push_back(fsObj);

		// -------------------------------------------------------------------------
		// NOTE: For recursive Piles, Query getParent() to get its full path and use
		//       it to get the directory where this item should go.
		// -------------------------------------------------------------------------
		QDir destDir = QFileInfo(getPileOwnerPath()).dir();
		// For removable drives, the pile owner's path is the removable drive path.
		// So when an item is dragged out, the destination dir is the same as source dir
		// In this case, set destination dir to working path (usually desktop)
		if (destDir == QFileInfo(fsObj->getFullPath()).dir())
		{
			destDir = scnManager->getWorkingDirectory();
			_ASSERT(getOwner()->isFileSystemType(Removable)); 
		}

		// disallow dragging of same files to directory for now
		QString filename = fsObj->getFileName(false);
		QFileInfo newPath = make_file(destDir, filename);
		bool dirAlreadyExists = fsObj->isFileSystemType(Folder) && exists(newPath);

		if (!fsManager->moveFiles(objList, native(destDir), failedObj, replacedObj, renamedObj) || !failedObj.empty())
		{
			// If something went wrong, but the filename does exist, we shouldn't remove this item from the pile
			if (fsManager->isValidFileName(fsObj->getFullPath()))
				return false;
		}

		if (!replacedObj.empty())
		{
			// Check if an object was replaced and delete the original actor representing the replaced file
			vector<FileSystemActor*> fsActors = scnManager->getFileSystemActors(native(newPath), true, false);
			for (int i = 0; i < fsActors.size(); i++)
			{
				animManager->addAnimation(AnimationEntry(fsActors[i], (FinishedCallBack) DeleteActorAfterAnim));
			}
		}
		else if (!renamedObj.empty())
		{
			// If an object was moved and renamed, make sure that when it is added to the scene via file system watching calls
			// that it doesn't create a new actor, and instead assumes the actor which was attempted to be moved
			filename = renamedObj.front().newFileName;
		}

		// Set the new name and path
		if (dirAlreadyExists)
		{
			// delete the folder
			animManager->addAnimation(AnimationEntry(fsObj, (FinishedCallBack) DeleteActorAfterAnim));
		}
		else
		{
			fsObj->setFilePath(native(make_file(destDir, filename)));
		}

		// Call the parent code to remove form the pile
		Pile::removeFromPile(obj, updateFlag);

		return true;
	}

	return false;
}

bool FileSystemPile::copyToPile(BumpObject *obj)
{
	LOG_LINE_REACHED();
	
	Actor * actor = NULL;
	FileSystemActor * fsActor = dynamic_cast<FileSystemActor *>(obj);
	bool addRc = false;
	QString fullPath;
	vector<FileSystemActor *> objList, failedList, replacedList;
	
	if (isInPile(obj) == -1)
	{
		if (obj->isBumpObjectType(BumpActor))
		{
			if (fsActor)
			{
				fullPath = fsActor->getFullPath();
				objList.push_back(fsActor);

				if (fsActor->isFileSystemType(Virtual))
					return false; // Virtual folders are not allowed in hard piles

				if (owner)
				{
					if (!fsManager->copyFiles(objList, owner->getTargetPath(), failedList, true) || !failedList.empty())
						return false;
				}
				return true;
			}
		}
		else if (obj->isBumpObjectType(BumpPile)) 
		{
			bool retVal = true;
			Pile * pile = (Pile *) obj;
			unsigned int size = pile->getNumItems();
			for (unsigned int i=0; i<size; i++)
				retVal &= copyToPile((*pile)[i]);
			pile->updatePileState();
			return retVal;
		}
		return false;
	}
	return false;
}

bool FileSystemPile::copyFromPile(BumpObject *obj, UpdateFlag updateFlag)
{
	LOG_LINE_REACHED();
	
	bool remRc = false;
	FileSystemActor	* fsActor = dynamic_cast<FileSystemActor *>(obj);
	vector<FileSystemActor *> objList, failedObj;

	if (fsActor && isInPile(obj) > -1)
	{
		objList.push_back(fsActor);

		QDir destDir = QFileInfo(getPileOwnerPath()).dir();
		// For removable drives, the pile owner's path is the removable drive path.
		// So when an item is dragged out, the destination dir is the same as source dir
		// In this case, set destination dir to working path (usually desktop)
		if (destDir == QFileInfo(fsActor->getFullPath()).dir())
		{
			destDir = scnManager->getWorkingDirectory();
			_ASSERT(getOwner()->isFileSystemType(Removable)); 
		}

		QString filename = fsActor->getFileName(false);
		QFileInfo newPath = make_file(destDir, filename);
		bool dirAlreadyExists = fsActor->isFileSystemType(Folder) && exists(newPath);

		if (!fsManager->copyFiles(objList, native(destDir), failedObj, true) || !failedObj.empty())
			return false;
			
		if (dirAlreadyExists)
		{
			// delete the folder
			animManager->addAnimation(AnimationEntry(fsActor, (FinishedCallBack) DeleteActorAfterAnim));
		}
		
		return true;
	}
	return false;
}

bool FileSystemPile::breakPile()
{
	LOG_LINE_REACHED();
	
	_ASSERT(!owner->isFileSystemType(Removable)); // break pile shouldn't be called on removable drives
	
	if (pileItems.empty())
		return false;
	if (!owner)
		return false;

	BumpObject *obj = NULL;
	int breakApartVel = 50; // Velocity at which the items fall apart
	bool failedDelete = false;
	vector<FileSystemActor *> delList, failedObj;
	QString message = QT_TR_NOOP("Breaking this pile will move all items from the folder '%1' into this Desktop and delete the remaining empty folder")
		.arg(owner->getFileName(false));
	bool hasSameName = false;

	dlgManager->clearState();
	dlgManager->setPrompt(message);

	// Prompt User if they want to delete this folder
	if (!dlgManager->promptDialog(DialogBreakPile))
		return false;

	// determine if we are just moving or breaking the pile
 	Pile * newPile = NULL;
	bool movePile = (dlgManager->getText() != "break");
	if (movePile)
	{
		/*
		newPile = new Pile;	
		newPile->setGlobalPosition(getGlobalPosition());
		*/
	}

	// Reset the owner actor
	owner->setGlobalPose(pileItems.back()->getGlobalPose());
	
	// Move All items to the pile's parent
	for (int i = 0; i < getNumItems(); i++)
	{
		FileSystemActor *fsActor = (FileSystemActor *) pileItems[i];
		obj = pileItems[i];

		if (fsActor->getFileName(false) == owner->getFileName(false))
		{
			hasSameName = true;
		}	

		// Attempt to remove this item from the pile
		if (removeFromPile(obj, NoUpdate))
		{
			/*
			if (movePile)
				newPile->addToPile(obj);
			*/

			// Make the actor jump out of the pile
			obj->setLinearVelocity(Vec3(float(rand() % breakApartVel - (breakApartVel / 2)), float(breakApartVel), float(rand() % breakApartVel - (breakApartVel / 2))));
			--i;

			if(!sel->isInSelection(obj))
				sel->add(obj);
		}
	}

	// Check if all items were removed
	if (getNumItems() > 0)
	{
		// If there are items left over, we just stack the pile again
		stack(getGlobalPosition());
		return false;
	}else{
		// Try to delete the folder
		delList.push_back(owner);
		if (!hasSameName) fsManager->deleteFiles(delList, failedObj, false);

		if (failedObj.size() == 0)
		{
			// All items were removed.
			owner->setPileizedPile(NULL);
			fsManager->removeObject(this);
			SAFE_DELETE(owner);

			// This should use the animation manager as a delayed delete
			killAnimation();
			animManager->addAnimation(AnimationEntry(this, (FinishedCallBack) DeletePileAfterAnim));
			return true;

		}else{

			// If the delete failed, the folder still exists so folderize it
			folderize(true);
			return false;
		}
	}

	/*
	if (movePile)
		newPile->finishAnimation();
	*/
}

FileSystemActor* FileSystemPile::folderize(bool animate)
{
	LOG_LINE_REACHED();
	
	// REFACTOR: Add in recursive folderization here
	if (owner)
	{
		// If the actor was Invisible (which it should be), make if visible again
		assert(owner->isActorType(Invisible));
		if (owner->isActorType(Invisible))
			owner->popActorType(Invisible);

		// just to give it a bit of a drop
		Mat34 folderizePose;
		if (_folderizeOnClose)
		{
			folderizePose = Mat34(owner->getGlobalOrientation(), _folderizeOnClosePosition);
		}
		else
		{
			if (pileItems.size() > 2)
				folderizePose = pileItems[min(pileItems.size(), (uint)2)]->getGlobalPose();
			else
				folderizePose = Mat34(GLOBAL(straightIconOri), getGlobalPosition());
		}

		// should be using the dims of the actors and not the pile to determine the folderize actor's size
		// Also update the contentDims list. We will clear whatever is currently stored in contentDims
		// and replace it with the dimensions of each item inside fsActor
		Vec3 dims(0.0f);
		FileSystemActor *fsActor;
		owner->clearFolderContentDimensions();
		for (int i = 0; i < pileItems.size(); ++i)
		{
			fsActor = (FileSystemActor *) pileItems[i];
			dims.max(pileItems[i]->getDims());			
			owner->addFolderContentDimensions(fsActor->getFullPath(), fsActor->getDims());
		}

		
		// Reset the owner actor
		owner->setGlobalPose(folderizePose);
		Vec3 ownerDims = owner->getDims();
		float maxDims = NxMath::max(dims.x, dims.z);
			ownerDims.x = maxDims;
			ownerDims.y = maxDims;
		owner->setDims(ownerDims);
		owner->showText();
		owner->setGravity(true);
		owner->setCollisions(true);
		owner->setFrozen(false);
		owner->setPileizedPile(NULL);		
		owner->popActorType(Invisible);

		// Delete the Close Widget if we have one
		SAFE_DELETE(closeWidget);

		// Collapse the Pile into its owner
		while (!pileItems.empty())
		{
			Actor *obj = (Actor *) pileItems[0];

			// Kill the animation first
			obj->killAnimation();

			// Clear and add a fade out animation
			clear(0);
			obj = FadeAndDeleteActor(obj);
			obj->setAlphaAnim(1.0f, 0.0f, 10);
			obj->setPoseAnim(obj->getGlobalPose(), folderizePose, 10);
		}

		// Delete the pile after the animation is over
		animManager->addAnimation(AnimationEntry(this, (FinishedCallBack) DeletePileAfterAnim));
		fsManager->removeObject(this);

		// This pile officially doesn't exist anymore
		setPileState(NoState);

		// record this folderization
		statsManager->getStats().bt.interaction.piles.folderized++;
		return owner;
	}

	return NULL;
}

QString FileSystemPile::getPileOwnerPath()
{
	// If we have an owner, return its path
	return (owner != NULL) ? owner->getFullPath() : "";
}

FileSystemActor* FileSystemPile::getOwner()
{
	return owner;
}

void FileSystemPile::setOwner(FileSystemActor *newOwner)
{
	if (winOS->IsWindowsVersion(WindowsXP) && getPileOwnerPath() != "")
		texMgr->removeWinThumbnailSearchPath(getOwner()->getTargetPath());
	owner = newOwner;
	if (winOS->IsWindowsVersion(WindowsXP) && getPileOwnerPath() != "") 
		texMgr->appendWinThumbnailSearchPath(getOwner()->getTargetPath());

	fsManager->addObject(this);
}

FileSystemActor *FileSystemPile::convert(Pile *softPile, QString newDirPath)
{
	LOG_LINE_REACHED();
	
	FileSystemActor *fsData = NULL;
	QString newPath;
	Vec3 newPos;

	// Convert Soft Pile to Hard Pile
	if (softPile->getPileType() == SoftPile && !owner)
	{
		for (uint i = 0; i < softPile->getNumItems(); i++)
		{
			fsData = (FileSystemActor *) (*softPile)[i];

			// Check if this item can be put into a soft Pile
			if (!fsData->isPilable(HardPile))
			{
				return NULL;
			}
		}

		if (softPile->getNumItems() == 0) return NULL;

		// Create the Directory that this can be moved into (May fail if exists - Thats ok)
		if (fsManager->createDirectory(newDirPath))
		{
			// Create the new Actor
			fsData = (FileSystemActor *) softPile->getLastItem();
			owner =  FileSystemActorFactory::createFileSystemActor(newDirPath);
			owner->setDimsToDefault();
			owner->setGlobalPose(Mat34(true));
			owner->setFilePath(newDirPath);

			// Semi-Pileize the owner
			owner->setPileizedPile(this);
			owner->hideAndDisable();

			// Add all items to the new pile
			for (int i = 0; i < softPile->getNumItems(); i++)
			{
				fsData = (FileSystemActor *) (*softPile)[i];
				fsData->setParent(NULL);

				if (addToPile(fsData))
				{
					// Success, remove item from pile
					softPile->clear(i);
					fsData->setParent(this);
					i--;
				}else{
					// Failed the move
					fsData->setParent(softPile);					
				}
			}

			// Update this Pile to reflect a full HardPile
			setText(owner->getText());
			stack(softPile->getGlobalPosition());
			updatePhantomActorDims();

			sel->clear();
			sel->add(this);
		}

		// If something could not be moved, stack the old pile
		if (softPile->getNumItems() > 0)
		{
			softPile->stack(softPile->getGlobalPosition());
		}else{
			// Delete the soft pile
			DeletePile(softPile);
		}

		return owner;
	}

	return NULL;
}

void FileSystemPile::close()
{
	if (closeWidget)
	{
		if (_folderizeOnClose)
		{
			if (Grid == getPileState())
				Pile::restoreDimsBeforeGrid();
			FileSystemActor * fsActor = folderize(true);
			fsActor->setGlobalPosition(_folderizeOnClosePosition);
		}	
		else
		{
			// REFACTOR: Add in recursive folderization here
			Pile::close();
		}
	}
	else
		Pile::close();
}

int FileSystemPile::isInPile(BumpObject *obj)
{
	if (!obj) return false;

	// -------------------------------------------------------------------------
	// NOTE: This will have to change when we start supporting recursive piles
	//       and instead we should check the pile's owner's path as well.
	// -------------------------------------------------------------------------
	if (obj->isBumpObjectType(BumpActor))
	{
		Actor *data = (Actor *) obj;

		// Only filesystem bound actors can be in this pile
		if (data->isActorType(FileSystem))
		{
			FileSystemActor *fsData = (FileSystemActor *) obj;

			// Manually checks to see if an item is in a path by checking its file name
			for (uint i = 0; i < pileItems.size(); i++)
			{
				FileSystemActor * pileActor = (FileSystemActor *) pileItems[i];
				if (fsManager->isIdenticalPath(fsData->getFullPath(), pileActor->getFullPath()))
					return i;
			}
		}
	}

	return -1;
}

QString FileSystemPile::resolveDropOperationString(vector<BumpObject *>& objList)
{
	// operation for dropping objects into this pile; ctrl to force copy, shift to force move
	if (isCopyIntoPile(objList))
		return QT_TR_NOOP("Copy into pile");
	
	return Pile::resolveDropOperationString(objList);
}

bool FileSystemPile::isCopyIntoPile(const vector<BumpObject *>& objList) const
{
	return owner->isCopyIntoActor(objList);
}

bool FileSystemPile::isSourceValid()
{
	for (uint i = 0; i < source.size(); i++)
	{
		// Check to see if the icon is a filesystem icon
		if (source[i]->getObjectType() == ObjectType(BumpPile, SoftPile))
		{
			// NOTE: don't allow dragging gridded piles onto other things
			Pile * p = (Pile *) source[i];
			return (p->getPileState() != Grid);
		} 
		else if (source[i]->getObjectType() == ObjectType(BumpActor, FileSystem) &&
			!(source[i]->getObjectType() == ObjectType(BumpActor, FileSystem, PhotoFrame)) &&
			!(source[i]->getObjectType() == ObjectType(BumpActor, FileSystem, Virtual)))
		{
			// Only file system actors are allowed
			return true;
		}
	}

	return false;
}

void FileSystemPile::onFileAdded(QString strFileName)
{
	
	FileSystemActor *fsData;

	// Look for collisions
	for (uint i = 0; i < pileItems.size(); i++)
	{
		fsData = (FileSystemActor *) pileItems[i];

		if (fsManager->isIdenticalPath(fsData->getFullPath(), strFileName))
		{
			// This item is already in the pile
			return;
		}
	}

	// Disallow hidden files when not specified
	if (fsManager->getFileAttributes(strFileName) & Hidden &&
		GLOBAL(settings).LoadHiddenFiles == false) return;

	// Create the new Actor
	FileSystemActor *actor =  FileSystemActorFactory::createFileSystemActor(strFileName);
	actor->setDimsToDefault();
	actor->setGlobalOrientation(GLOBAL(straightIconOri));
	actor->setGlobalPosition(getGlobalPosition());
	actor->setAlpha(1.0f);
	actor->setFilePath(strFileName);

	// Add it to this pile
	Pile::addToPile(actor);

	// Relayout
	updatePileState();
}

void FileSystemPile::onFileRemoved(QString strFileName)
{
	FileSystemActor *actor;

	// Search all items in the pile for this path
	for (uint i = 0; i < pileItems.size(); i++)
	{
		actor = (FileSystemActor *) pileItems[i];

		// If the pile Item patches the file path, delete it
		if (fsManager->isIdenticalPath(actor->getFullPath(), strFileName))
		{
			Pile::removeFromPile(actor);
			SAFE_DELETE(actor);

			i = pileItems.size();
		}
	}

	// If no items left, folderize
	if (pileItems.size() == 0)
	{
		folderize(false);
	}
}

void FileSystemPile::onFileNameChanged(QString strOldFileName, QString strNewFileName)
{
	FileSystemActor *actor;

	// Search all items in the pile for this path
	for (uint i = 0; i < pileItems.size(); i++)
	{
		actor = (FileSystemActor *) pileItems[i];

		// If the pile Item patches the file path, change its fileName
		if (fsManager->isIdenticalPath(actor->getFullPath(), strOldFileName))
		{
			actor->setFilePath(strNewFileName);
			rndrManager->invalidateRenderer();

			return;
		}
	}
}

void FileSystemPile::onFileModified(QString strFileName)
{
	FileSystemActor *actor;

	// Search all items in the pile for this path
	for (uint i = 0; i < pileItems.size(); i++)
	{
		actor = (FileSystemActor *) pileItems[i];

		// If the pile Item patches the file path, change its fileName
		if (fsManager->isIdenticalPath(actor->getFullPath(), strFileName) && actor->isFileSystemType(Thumbnail))
		{
			// Update Thumbnail
			actor->enableThumbnail();

			return;
		}
	}
}

StrList FileSystemPile::getWatchDir()
{
	StrList wList;

	// Return the folder this pile represents
	wList.push_back(owner->getTargetPath());

	return wList;
}

vector<BumpObject *> FileSystemPile::onDrop(vector<BumpObject *> &objList)
{
	QString targetDir = owner->getTargetPath();
	vector<BumpObject *> failedObj;
	
	bool copyFile = isCopyIntoPile(objList);

	for (uint i = 0; i < objList.size(); i++)
	{
		// Loop through and add all items to the pile
		if (objList[i] != this && objList[i]->isPilable(getPileType()))
		{
			objList[i]->setAlpha(1.0f);

			// bound the object dimension to the largest item in the pile 
			Vec3 dims = objList[i]->stateBeforeDrag().dims;			
			Vec3 pileDims = getDims();
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

			if (copyFile)
			{
				copyToPile(objList[i]);
				// restore source file actor, directory watcher will create actor for new copy in the pile				
				objList[i]->restoreStateFromBeforeDrag(objList[i]);
			}
			else
				addToPile(objList[i]);
			sel->remove(objList[i]);
		}
		else
		{
			failedObj.push_back(objList[i]);
		}
	}

	updatePileState();

	// Record this drop
	statsManager->getStats().bt.interaction.dragAndDrop.toPile++;

	return failedObj;
}
 
void FileSystemPile::onTossRecieve(vector<BumpObject *> tossedObjs)
{
	for (uint i = 0; i < tossedObjs.size(); i++)
		GLOBAL(Tossing).push_back(tossedObjs[i]);

	// Force a render so that the arrow is shown
	winOS->Render();

	dlgManager->clearState();
	dlgManager->setCaption(QT_TR_NOOP("Move Files?"));
	QString name = owner->isFileSystemType(Link) ? filename(owner->getLinkTarget()) : filename(owner->getFullPath());
	if (owner->isFileSystemType(LogicalVolume))
		name = owner->getText();
	dlgManager->setPrompt(QT_TR_NOOP("Are you sure you want to move the selected items to '%1'").arg(name));

	// Drop in the items as if dropped on
	if (dlgManager->promptDialog(DialogYesNo))
	{
		onDrop(tossedObjs);
	}else{
		// Animate back to the original starting pose
		animateObjectsBackToPreDropPose(tossedObjs);
	}
}

void FileSystemPile::onWidgetClick(BumpObject * widget)
{
	if (widget == launchOwnerWidget)
	{
		// launch the owner
		getOwner()->onLaunch();

		// Close the pile's grid view.  Done as a convenience, because the 
		// user will now likely deal with the folder in explorer.
		// Special case: don't do it for a grid that is pinned to the wall
		if (!isPinned())
		{
			if (!_onGridCloseHandler.empty()) 
				_onGridCloseHandler();
			close();
		}
	}
	else if (widget == launchExternalWidget)
	{
		// In the Live Mesh demo version, gridded piles that are pinned to the
		// wall have an extra widget which launches a second instance of
		// BumpTop pointed to the Live Mesh folder

		QString program = winOS->GetExecutableDirectory().absolutePath() + "/BumpTop.exe";
		consoleWrite(QString("Launching %1...\n").arg(program));
		QStringList arguments;
		arguments << "-childProcess" << "-d" << getOwner()->getTargetPath();
		if (scnManager->childProcess)
		{
			scnManager->childProcess->kill();
			SAFE_DELETE(scnManager->childProcess);
		}
		scnManager->childProcess = new QProcess();
		scnManager->childProcess->start(program, arguments);
	}
	else
	{
		Pile::onWidgetClick(widget);
	}
}
 
bool FileSystemPile::isValidToss(vector<BumpObject *> tossedObjs)
{
	bool rc = false;

	// Use the Drop logic to determine valid tosses
	source = tossedObjs;
	rc = isSourceValid();
	source.clear();

	return rc;
}


bool FileSystemPile::isPilable(uint pileType)
{
	return false;
}

void FileSystemPile::grow( uint numSteps /*= 25*/, float growFactor /*= 1.6f*/ )
{
	Pile::grow(numSteps, growFactor);
	// update the owner as well
	if (getOwner())
		getOwner()->grow(numSteps, growFactor);
}

void FileSystemPile::shrink( uint numSteps /*= 25*/, float shrinkFactor /*= 0.6f*/ )
{
	Pile::shrink(numSteps, shrinkFactor);
	// update the owner as well
	if (getOwner())
		getOwner()->shrink(numSteps, shrinkFactor);
}

void FileSystemPile::setFolderizeOnClose( bool foc, Vec3 pos )
{
	_folderizeOnClose = foc;
	_folderizeOnClosePosition = pos;
}
