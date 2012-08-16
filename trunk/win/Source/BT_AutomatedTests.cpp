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
#include "BT_AutomatedTests.h"

// -----------------------------------------------------------------------------

#include "BT_WindowsOS.h"
#include "BT_FileSystemManager.h"
#include "BT_SceneManager.h"
#include "BT_FileSystemActor.h"
#include "BT_CommonFiniteStates.h"
#include "BT_OverlayComponent.h"
#include "BT_Util.h"
#include "BT_QtUtil.h"

AutomatedTests::PrepareScene::PrepareScene(unsigned int duration)
: FiniteState(duration)
{

}

AutomatedTests::PrepareScene::~PrepareScene()
{}

bool AutomatedTests::PrepareScene::prepareStateChanged()
{
	return true;
}

void AutomatedTests::PrepareScene::onStateChanged()
{
	// clear out all files from the training directory 
	StrList dirContents = fsManager->getDirectoryContents(native(winOS->GetTrainingDirectory()));
	for_each(QString filename, dirContents)
	{
		fsManager->deleteFileByName(filename, true); // delete silently
	}

	//get Theme Directory (true), rename
	// reload theme
	//delete
	// rename old one back
	// reload theme

	// copy files over that we need to the training directory
	QDir dataPath = parent(winOS->GetTexturesDirectory()) / "AutomatedTestData";
	QDir desktopPath = scnManager->getWorkingDirectory();
	QString desktopPathStr = native(desktopPath);
	QString trainingDir = native(winOS->GetTrainingDirectory());

	StrList dirList = fsManager->getDirectoryContents(native(dataPath));
	for (int i = 0; i < dirList.size(); ++i)
	{
		QString fileName = filename(dirList[i]);
		if (!desktopPath.exists(fileName))
			fsManager->copyFileByName(dirList[i], trainingDir, fileName, false, true);
	}

	GLOBAL(skipSavingSceneFile) = true; // this will prevent saving of the scene file

	// switch BumpTop to use the training directory for now
	scnManager->setWorkingDirectory(winOS->GetTrainingDirectory());
	// Let's clear out the desktop for the demo purposes

	ClearBumpTop();

	CreateBumpObjectsFromDirectory(native(winOS->GetTrainingDirectory()));


}

bool AutomatedTests::PrepareScene::finalizeStateChanged()
{
	// // ensure that all the images have actually loaded
	return true;
}

// -----------------------------------------------------------------------------

// using AutomatedTests::FinalizeScene;

AutomatedTests::FinalizeScene::FinalizeScene(unsigned int duration)
: FiniteState(duration)
{}

AutomatedTests::FinalizeScene::~FinalizeScene()
{}

bool AutomatedTests::FinalizeScene::prepareStateChanged()
{
	return true;
}

void AutomatedTests::FinalizeScene::onStateChanged()
{
	ExitBumptop();
}

bool AutomatedTests::FinalizeScene::finalizeStateChanged()
{
	return true;
}

// -----------------------------------------------------------------------------

AutomatedTests::AutomatedTests()
{
	// disable CPU optimizations
	GLOBAL(settings).useCPUOptimizations = false;

	overlay = new OverlayLayout();
	scnManager->registerOverlay(overlay);
	textLayout = new VerticalOverlayLayout();
	overlay->addItem(textLayout);
	HUDtext = new TextOverlay();
	textLayout->addItem(HUDtext);

	QDir desktopPath = scnManager->getWorkingDirectory();
	QString desktopPathStr = native(desktopPath);

	// PREPARE SCENE (NOTE, we need some time for the textures to load)
	FiniteState * tmpState = NULL, * prevState = NULL;
	_firstState = tmpState = new PrepareScene(12000);
	_demoStates.addState(tmpState);

// -----------------------------------------------------------------------------
	/* 
		INSERT VARIOUS TEST STATES HERE
	*/

	// SELECT ALL DEMO FILES
	prevState = tmpState;
	tmpState = new SelectFileSystemActorsState(1000, "\\_demo.*");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// CREATE PILE
	prevState = tmpState;
	tmpState = new CreatePileState(500, "\\_demo.*");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

// -----------------------------------------------------------------------------

	// CLEAN UP SCENE
	_finalizeState = new FinalizeScene(500);
	_demoStates.addState(_finalizeState);

	// disable CPU optimizations
//	GLOBAL(settings).useCPUOptimizations = true;
}

void AutomatedTests::play()
{
	assert(_firstState);

	// start the demo
	_demoStates.start(_firstState);
	_timeSinceLastChange.restart();
	_durationUntilNextChange = _firstState->getExpectedDuration();
	Replayable::play();

	// notify the user
	MessageClearPolicy clearPolicy;
	clearPolicy.setTimeout(2);
	scnManager->messages()->addMessage(new Message("AutomatedTests::play", "Starting automated tests", Message::Ok, clearPolicy));
}

void AutomatedTests::stop()
{
	_timeSinceLastChange.restart();
	_durationUntilNextChange = -1;

	// clean up/finalize if we are stopping early
	if (getPlayState() == Running)
	{
		if (_finalizeState && _demoStates.start(_finalizeState))
		{
			bool tmp;
			do {}
			while (_demoStates.next(tmp));
		}
	}

	Replayable::stop();

	// notify the user
	MessageClearPolicy clearPolicy;
	clearPolicy.setTimeout(2);
	scnManager->messages()->addMessage(new Message("AutomatedTests::stop", "Stopping automated tests", Message::Ok, clearPolicy));
}