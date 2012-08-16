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

#ifdef USING_UNDO
/*

#include "BT_Common.h"
#include "BT_UndoStack.h"
#include "BT_UndoStackEntry.h"
#include "BT_Util.h"
#include "BT_FileSystemActor.h"
#include "BT_AnimationManager.h"
#include "BT_AnimationEntry.h"
#include "BT_ActorUndoData.h"
#include "BT_MenuAction.h"
#include "BT_TextManager.h"
#include "BT_SceneManager.h"

UndoStack::UndoStack()
{
	sceneNeedsSaving = true;
	saveTimeout.restart();
}

UndoStack::~UndoStack()
{
	for (int i = 0; i < redoStack.size(); i++)
	{
		unsigned char *redoEntry = redoStack[i].buf;
		
		// Delete any outstanding redo entries
		SAFE_DELETE_ARRAY(redoEntry);
	}

	for (int i = 0; i < undoStack.size(); i++)
	{
		unsigned char *undoEntry = undoStack[i].buf;

		// Delete any outstanding redo entries
		SAFE_DELETE_ARRAY(undoEntry);
	}
}

bool UndoStack::undo()
{
	
	unsigned char *undoEntry = NULL;
	uint undoEntrySize = 0;
	vector<ActorUndoData> undoDataList;
	vector<NxActorWrapper *> actors;

	if (!animManager->isAnimating())
	{
		if (undoStack.size() > 1)
		{
			GLOBAL(settings).EnableAnimations = false;

			// Show a progress box
			//uiDialog->DisplayModelessDialog("Undoing....");

			// Push the current state to the redo stack
			undoEntry = undoStack[undoStack.size() - 1].buf;
			undoEntrySize = undoStack[undoStack.size() - 1].size;
			undoDataList = undoStack[undoStack.size() - 1].pos;
			redoStack.push_back(UndoStackEntry(undoEntry, undoEntrySize, undoDataList));

			// Remove the current state and get the previous one
			undoStack.pop_back();
			undoEntry = undoStack[undoStack.size() - 1].buf;
			undoEntrySize = undoStack[undoStack.size() - 1].size;
			//undoDataList = undoStack[undoStack.size() - 1].pos;

			// Delete all the actors
			ClearBumpTop();

			// Load up the previous scene
			LoadScene(undoEntry, undoEntrySize);

			// Create an animation form the old position to the new one
			actors = GetDesktopItems();
			for (int i = 0; i < actors.size(); i++)
			{
				Actor *data = GetBumpActor(actors[i]);
				ActorUndoData *undoData = NULL;

				for (int j = 0; !undoData && j < undoDataList.size(); j++)
				{
					if (data->isActorType(FileSystem))
					{
						FileSystemActor * actor = (FileSystemActor *) data;
						if (fsManager->isIdenticalPath(actor->getFullPath(), undoDataList[j].actorPath))
						{
							// Get the data that corresponds with this actor
							undoData = &undoDataList[j];

							// Create an animation Path
							if (!data->isPinned() || !isMatEqual(undoData->orientaiton, actors[i]->getGlobalPose()))
							{
								data->setPoseAnim(undoData->orientaiton, actors[i]->getGlobalPose(), 30);

								// Add the animation
								if (data->isPinned())
								{
									animManager->addAnimation(AnimationEntry(GetBumpActor(actors[i]), (FinishedCallBack) PinItemAfterAnim, new Vec3(data->getPinPointInSpace())));
								}else{
									animManager->addAnimation(AnimationEntry(GetBumpActor(actors[i])));
								}
							}
						}
					}
				}
			}

			// Force an update
			animManager->update();

			GLOBAL(settings).EnableAnimations = true;

			// Clear the Progress Box
			//uiDialog->DisplayModelessDialog("");
			consoleWrite("<- Undoing Scene || UndoStates: %d, RedoStates: %d\n", getUndoCount(), getRedoCount());

			textManager->invalidate();
			return true;
		}
		return false;

	}

	return true;
}

bool UndoStack::redo()
{
	
	unsigned char *redoEntry;
	uint redoEntrySize = 0;
	vector<ActorUndoData> redoDataList;
	vector<NxActorWrapper *> actors;

	if (!animManager->isAnimating())
	{
		if (redoStack.size() > 0)
		{
			GLOBAL(settings).EnableAnimations = false;

			// Show a progress box
			//uiDialog->DisplayModelessDialog("Redoing....");

			// Get the entry and remove it from the redo stack
			redoEntry = redoStack[redoStack.size() - 1].buf;
			redoEntrySize = redoStack[redoStack.size() - 1].size;
			redoDataList = redoStack[redoStack.size() - 1].pos;
			redoStack.pop_back();
			undoStack.push_back(UndoStackEntry(redoEntry, redoEntrySize, redoDataList));
			redoDataList = getActorOri();

			// Clear the desktop
			ClearBumpTop();

			// Load the scene
			LoadScene(redoEntry, redoEntrySize);

			// Create an animation form the current position to the new one
			actors = GetDesktopItems();
			for (int i = 0; i < actors.size(); i++)
			{
				Actor *data = GetBumpActor(actors[i]);
				ActorUndoData *redoData = NULL;

				for (int j = 0; !redoData && j < redoDataList.size(); j++)
				{
					if (data->isActorType(FileSystem))
					{
						FileSystemActor * actor = (FileSystemActor *) data;
						if (fsManager->isIdenticalPath(actor->getFullPath(), redoDataList[j].actorPath))
						{
							// Get the data that corresponds with this actor
							redoData = &redoDataList[j];

							// Create an animation Path
							if (data->isPinned() || !isMatEqual(redoData->orientaiton, actors[i]->getGlobalPose()))
							{
								data->setPoseAnim(redoData->orientaiton, actors[i]->getGlobalPose(), 30);

								// Add the animation
								if (data->isPinned())
								{
									animManager->addAnimation(AnimationEntry(GetBumpActor(actors[i]), (FinishedCallBack) PinItemAfterAnim, new Vec3(data->getPinPointInSpace())));
								}else{
									animManager->addAnimation(AnimationEntry(GetBumpActor(actors[i]), NULL, NULL));
								}
							}
						}
					}
				}
			}

			// Force an update
			animManager->update();

			// Clear the Progress Box
			//uiDialog->DisplayModelessDialog("");

			GLOBAL(settings).EnableAnimations = true;
			consoleWrite("-> Redoing Scene || UndoStates: %d, RedoStates: %d\n", getUndoCount(), getRedoCount());

			textManager->invalidate();
			return true;
		}
		return false;

	}

	return true;
}

bool UndoStack::saveState()
{
	return false;
	uint undoBufSize = 0;
	unsigned char *undoBuf = NULL;

	/// *
	if (undoStack.size() >= GLOBAL(settings).undoLevels)
	{
		// Pop the bottom item off the stack to make room for a new undo
		undoStack.pop_front();
	}// * /

	// Save the scene using the newly redone Save Scene
	SaveScene(&undoBuf, undoBufSize);

	// / *
	undoStack.push_back(UndoStackEntry(undoBuf, undoBufSize, getActorOri()));

	for (int i = 0; i < redoStack.size(); i++)
	{
		undoBuf = redoStack[i].buf;

		// Delete any outstanding redo entries
		SAFE_DELETE_ARRAY(undoBuf);
	}

	redoStack.clear();
	// * /
	return true;
}

vector<ActorUndoData> UndoStack::getActorOri()
{
	vector<ActorUndoData> rc;
	vector<NxActorWrapper *> actors = GetDesktopItems();

	for (int i = 0; i < actors.size(); i++)
	{
		Actor *data = GetBumpActor(actors[i]);

		if(data->isActorType(FileSystem)) 
		{
			// Get the relative data
			Mat34 mat = actors[i]->getGlobalPose();
			Vec3 size = GetBumpActor(actors[i])->getDims();
			String id = ((FileSystemActor *) data)->getFullPath();

			// Push into our undo stack
			rc.push_back(ActorUndoData(mat, size, id));
		}
	}

	return rc;
}

int UndoStack::getUndoCount()
{
	return undoStack.size() - 1;
}

int UndoStack::getRedoCount()
{
	return redoStack.size();
}

void UndoStack::update(MenuAction *action)
{
	if (action->getLabel().find("Undo") != string::npos)
	{
		// Modify Undo
		action->setEnabled(getUndoCount() > 0);
	}else{
		// Modify Redo
		action->setEnabled(getRedoCount() > 0);
	}
}

void UndoStack::update()
{
	// Save if needed but at a minimum of 5 second intervals
	if (sceneNeedsSaving && saveTimeout.elapsed() > 5000)
	{
		saveState();

		// Restart the timer and flags
		sceneNeedsSaving = false;
		saveTimeout.restart();
	}
}

void UndoStack::execute(MenuAction *action)
{
	if (action->getLabel().find("Undo") != string::npos)
	{
		// Modify Undo
		undo();
	}else{
		// Modify Redo
		redo();
	}
}

void UndoStack::invalidate()
{
	sceneNeedsSaving = true;
}
*/

#endif