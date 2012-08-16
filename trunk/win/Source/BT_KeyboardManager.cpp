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
#include "BT_Find.h"
#include "BT_KeyboardManager.h"
#include "BT_SceneManager.h"
#include "BT_KeyboardHandler.h"
#include "BT_KeyCombo.h"
#include "BT_Util.h"
#include "BT_WindowsOS.h"
#include "BT_Selection.h"

KeyboardManager::KeyboardManager()
{

}

KeyboardManager::~KeyboardManager()
{
}

void KeyboardManager::mapKeyToFunction(KeyCombo &combo, VoidCallback callback)
{
	// Map a new value to to a specific function
	callbackMap[combo.subKeys.key] = callback;
}

void KeyboardManager::onKeyDown(KeyCombo &key)
{
	bool usedHandlerList = false;
	QChar dummyChar;

	// Pass the Keyboard functionality to the handlers first
	for (uint i = 0; i < handlerList.size(); i++)
	{
		if (handlerList[i]->onKeyDown(key))
		{
			usedHandlerList = true;
		}
	}

	if (!usedHandlerList)
	{		
		dummyChar = key.toAscii();
		bool isBackspaceCharacter = (dummyChar == KeyBackspace);
		if (callbackMap.find(key) != callbackMap.end())
		{
			// Call the function thats mapped to this key
			KeyCallback * callBack = &callbackMap[key]; // need reference to set inCall
			if (!callBack->blocking || !callBack->inCall)
			{
				callBack->inCall = true;
				callBack->callBack();
				callBack->inCall = false;
			}
		}
		// For control keys (0-9, A-Z)
		else if (!key.subKeys.isAltPressed && !key.subKeys.isCtrlPressed &&
			((dummyChar >= ' ' && dummyChar <= '~') || isBackspaceCharacter))
		{
			if (Finder->elapsedTime.elapsed() > GLOBAL(maxDelayBetweenKeyDown))
			{
				// Restart the timer if 1400ms of inactivity went by
				Finder->searchString.clear();
			}

			// Append the character to the search String
			if (isBackspaceCharacter)
				Finder->searchString.chop(1);
			else 
				Finder->searchString.append(QString(dummyChar));
			Finder->elapsedTime.restart();

			// Select the actors that match the query
			Finder->search();
			
		}
	}

}

void KeyboardManager::onKeyUp(KeyCombo &combo)
{
	// Pass the Keyboard functionality to the handlers first
	for (uint i = 0; i < handlerList.size(); i++)
	{
		handlerList[i]->onKeyUp(combo);
	}
}

bool KeyboardManager::addHandler(KeyboardEventHandler *eventHandler)
{
	// Add this handler to the list
	handlerList.push_back(eventHandler);

	return true;

}

bool KeyboardManager::removeHandler(KeyboardEventHandler *eventHandler)
{
	// Search through all the items and remove the requested handler
	for (uint i = 0; i < handlerList.size(); i++)
	{
		if (handlerList[i] == eventHandler)
		{
			// Remove form the handler list
			handlerList.erase(handlerList.begin() + i);
			return true;
		}
	}

	// Event Handler not found!
	return false;

}

bool KeyboardManager::isKeyMapped(unsigned char key)
{
	KeyCallbacks::const_iterator iter = callbackMap.begin();
	KeyCallbacks::const_iterator endIter = callbackMap.end();
	
	// Loop through looking for a specific key
	while (iter != endIter) 
	{
		// Check if this key is being used
		if (iter->first.subKeys.key == key) 
		{
			return true;
		}

		iter++;
	}

	return false;
}

void KeyboardManager::init()
{
	
	callbackMap.clear();

	// Active keys
	callbackMap[KeyCombo('A', true)] = Key_SelectAll;
	//callbackMap[KeyCombo('Y', true)] = Key_TriggerRedo;
	//callbackMap[KeyCombo('Z', true)] = Key_TriggerUndo;
	//callbackMap[KeyCombo('Z', true, true)] = Key_TriggerRedo;
	callbackMap[KeyCombo('T', true)] = Key_SortIntoPilesByType;
	callbackMap[KeyCombo('F', true)] = Key_SearchSubString;
	callbackMap[KeyCombo('G', true)] = Key_Grow;
	callbackMap[KeyCombo('S', true)] = Key_Shrink;
	callbackMap[KeyCombo('R', true)] = Key_ResetSize;
	callbackMap[KeyCombo('P', true)] = Key_CreateStickyNote;
	callbackMap[KeyCombo('N', true)] = Key_RenameSelection;
	callbackMap[KeyCombo('N',true,true,false)] = Key_CreateNewDirectory;

	// Windows Explorer-esque hotkeys
	callbackMap[KeyCombo(KeyEnter)] = Key_LaunchSelection;
	callbackMap[KeyCombo(KeyDelete, true)] = Key_ClearBumpTop;
	callbackMap[KeyCombo(KeyDelete)] = Key_DeleteSelection;
	callbackMap[KeyCombo(KeyDelete, false, true)] = Key_DeleteSelectionSkipRecycleBin;
	callbackMap[KeyCombo(KeyF1)] = Key_ShowAbout;
	callbackMap[KeyCombo(KeyF2)] = Key_RenameSelection;
	callbackMap[KeyCombo(KeyF3)] = Key_ToggleFPS;
	callbackMap[KeyCombo(KeyF5)] = Key_ToggleSharingMode;
	// callbackMap[KeyCombo(KeyF5)] = Key_RefreshFileActors;
	callbackMap[KeyCombo(KeyBackspace, true)] = Key_ToggleDebugKeys;
	callbackMap[KeyCombo('C', true)] = Key_CopySelection;
	callbackMap[KeyCombo('X', true)] = Key_CutSelection;
	callbackMap[KeyCombo('V', true)] = Key_PasteSelection;
	callbackMap[KeyCombo('W', true)] = Key_ClosePile;
	callbackMap[KeyCombo(KeyComma, true)] = Key_ShowSettingsDialog;
	callbackMap[KeyCombo(KeyF6)] = Key_NextMonitor;

	// First person camera controls
	callbackMap[KeyCombo('W', true, true)] = Key_MoveCameraForward;
	callbackMap[KeyCombo('S', true, true)] = Key_MoveCameraBackwards;
	callbackMap[KeyCombo('A', true, true)] = Key_MoveCameraLeft;
	callbackMap[KeyCombo('D', true, true)] = Key_MoveCameraRight;
	callbackMap[KeyCombo(KeySpace, true, true)] = Key_ShootCuboid;

	// debug keys
#ifndef _DEBUG
	if (GLOBAL(settings).enableDebugKeys)
#endif
	{
		// callbackMap[KeyCombo(KeySpace)] = Key_PlayPause;
		callbackMap[KeyCombo(KeyColon, true)] = Key_ShowConsole;
		callbackMap[KeyCombo('I', true)] = Key_TogglePausePhysics;
		callbackMap[KeyCombo('O', true)] = Key_ToggleCollisions;
		callbackMap[KeyCombo(KeyQuestionMark, true)] = Key_TogglePrintMode;

		callbackMap[KeyCombo('B', true)] = Key_ToggleBubbleClusters;
		callbackMap[KeyCombo('J', true)] = Key_ToggleInvisibleActors;
		callbackMap[KeyCombo('N', true)] = Key_ToggleRepositionBounds;
		callbackMap[KeyCombo('M', true)] = Key_ToggleWindowMode;
		callbackMap[KeyCombo('U', true)] = Key_ToggleUserAgent;

		callbackMap[KeyCombo(KeyRightSqBrkt, true)] = Key_Test; //Testing and debugging key.  Throw your code here.  
		callbackMap[KeyCombo(KeyQuote, true)] = Key_ToggleDrawText;
		callbackMap[KeyCombo(KeyPipe, true)] = Key_ToggleCPUOptimization;

		callbackMap[KeyCombo('5', true)] = Key_SetCameraAsTopDown;
		callbackMap[KeyCombo('6', true)] = Key_SetCameraAsAngled;
		callbackMap[KeyCombo('7', true)] = Key_ZoomToAll;
		callbackMap[KeyCombo('0', true)] = Key_RunBumptopTests;

		callbackMap[KeyCombo(KeyF4)] = Key_StartAutomatedDemo;
		callbackMap[KeyCombo(KeyF9)] = Key_StartAutomatedTests;
		callbackMap[KeyCombo(KeyF8)] = Key_CreateCustomActor;

		callbackMap[KeyCombo(KeyF8, true)] = Key_ToggleProfiling;
		callbackMap[KeyCombo(KeyF12, true, false, true)] = Key_ForceDump;

		callbackMap[KeyCombo(KeyF7)] = KeyCallback(Key_LoadTradeshowScene, true);
		callbackMap[KeyCombo(KeyF7, true)] = KeyCallback(Key_StartAutomatedTradeshowDemo, true);
		callbackMap[KeyCombo(KeyF7, true, true)] = KeyCallback(Key_ReloadScene, true);
	}
}

