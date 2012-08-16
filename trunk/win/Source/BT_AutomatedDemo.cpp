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
#include "BT_AutomatedDemo.h"

// -----------------------------------------------------------------------------

#include "BT_WindowsOS.h"
#include "BT_FileSystemManager.h"
#include "BT_SceneManager.h"
#include "BT_FileSystemActor.h"
#include "BT_CommonFiniteStates.h"
#include "BT_OverlayComponent.h"
#include "BT_Util.h"
#include "BT_QtUtil.h"

AutomatedDemo::PrepareScene::PrepareScene(unsigned int duration)
: FiniteState(duration)
{

}

AutomatedDemo::PrepareScene::~PrepareScene()
{}

bool AutomatedDemo::PrepareScene::prepareStateChanged()
{
	return true;
}

void AutomatedDemo::PrepareScene::onStateChanged()
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
	QDir dataPath = parent(winOS->GetTexturesDirectory()) / "AutomatedDemoDemo";
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

bool AutomatedDemo::PrepareScene::finalizeStateChanged()
{
	// // ensure that all the images have actually loaded
	return true;
}

// -----------------------------------------------------------------------------

// using AutomatedDemo::FinalizeScene;

AutomatedDemo::FinalizeScene::FinalizeScene(unsigned int duration)
: FiniteState(duration)
{}

AutomatedDemo::FinalizeScene::~FinalizeScene()
{}

bool AutomatedDemo::FinalizeScene::prepareStateChanged()
{
	return true;
}

void AutomatedDemo::FinalizeScene::onStateChanged()
{
	Key_ReloadScene();
}

bool AutomatedDemo::FinalizeScene::finalizeStateChanged()
{
	return true;
}

// -----------------------------------------------------------------------------

// using AutomatedDemo::InitPerformanceData;

AutomatedDemo::InitPerformanceData::InitPerformanceData(unsigned int duration, QString perfDescription)
: FiniteState(duration)
, _perfDescription(perfDescription)
{}

AutomatedDemo::InitPerformanceData::~InitPerformanceData()
{}

bool AutomatedDemo::InitPerformanceData::prepareStateChanged()
{
	return true;
}

void AutomatedDemo::InitPerformanceData::onStateChanged()
{
	// Init
	GLOBAL(framesPerSecond) = 0;
	GLOBAL(avgFramesPerSecond) = 0;


}

bool AutomatedDemo::InitPerformanceData::finalizeStateChanged()
{
	return true;
}

// -----------------------------------------------------------------------------

// using AutomatedDemo::CollectPerformanceData;

AutomatedDemo::CollectPerformanceData::CollectPerformanceData(unsigned int duration, QString perfDescription)
: FiniteState(duration)
, _perfDescription(perfDescription)
{
}

AutomatedDemo::CollectPerformanceData::~CollectPerformanceData()
{}

bool AutomatedDemo::CollectPerformanceData::prepareStateChanged()
{
	return true;
}

void AutomatedDemo::CollectPerformanceData::onStateChanged()
{
		//hudText.push_back("Frames Per Second: " + QString::number(GLOBAL(framesPerSecond)));
		//hudText.push_back("Average FPS:       " + QString::number(GLOBAL(avgFramesPerSecond) / GLOBAL(frameCounter)));
		//hudText.push_back("Max Average FPS:   " + QString::number(scnManager->maxFramesPerSecond));
		//hudText.push_back("Video Mem Usage:   " + QString::number(texMgr->getApproximatedVideoMemoryUsage()/1024) + "KB");
		//hudText.push_back(" "); //new line
		//hudText.push_back("Bump Objects:      " + QString::number(scnManager->getBumpObjects().size()));
		//hudText.push_back("Actors:            " + QString::number(scnManager->getFileSystemActors().size()));
		//hudText.push_back("Piles:             " + QString::number(scnManager->getPiles().size()));
	
	// Write performance details
	QString framesPerSecond = QString::number(GLOBAL(framesPerSecond));
	QString totalFramesRendered = QString::number(GLOBAL(avgFramesPerSecond));
	QString maxFramesPerSecond = QString::number(scnManager->maxFramesPerSecond);
	QString numObjects = QString::number(scnManager->getBumpObjects().size());
	QString numActors = QString::number(scnManager->getFileSystemActors().size());
	QString numPiles = QString::number(scnManager->getPiles().size());
	
	QString path = native(winOS->GetDataDirectory()) + "\\demoPerf.txt";
	QString output = QString(
	"\n\r"
	"Operation: %1\n\r"
	"FPS: %2\n\r"
	"Total frames rendered: %3\n\r"
	"Max Average FPS: %4\n\r"
	"O/A/P: %5/%6/%7\n\r"
	"\n\r"
	)
	.arg(_perfDescription)
	.arg(framesPerSecond)
	.arg(totalFramesRendered)
	.arg(maxFramesPerSecond)
	.arg(numObjects).arg(numActors).arg(numPiles);
	append_file_utf8(output, path);

	// launch the file after the perf data
	fsManager->launchFile(path);
}

bool AutomatedDemo::CollectPerformanceData::finalizeStateChanged()
{
	return true;
}


// -----------------------------------------------------------------------------

AutomatedDemo::AutomatedDemo()
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

	// Initialize performance data
	prevState = tmpState;
	tmpState = new InitPerformanceData(0, "Initialize performance data");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// SCATTER ACTORS
	prevState = tmpState;
	tmpState = new ScatterActorsState(2000, "\\_demo.*");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);
	
	// SELECT ALL DEMO FILES
	prevState = tmpState;
	tmpState = new SelectFileSystemActorsState(1000, "\\_demo.*");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// ZOOM INTO IMAGE
	prevState = tmpState;
	tmpState = new ZoomIntoImageState(1500, "\\_demo[^\\.]*\\.jpg");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// NEXT IMAGE
	prevState = tmpState;
	tmpState = new ZoomToNextImageState(2000);
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// NEXT IMAGE
	prevState = tmpState;
	tmpState = new ZoomToNextImageState(2000);
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// NEXT IMAGE
	prevState = tmpState;
	tmpState = new ZoomToNextImageState(2000);
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// NEXT IMAGE
	prevState = tmpState;
	tmpState = new ZoomToNextImageState(2000);
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// NEXT IMAGE
	prevState = tmpState;
	tmpState = new ZoomToNextImageState(2000);
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// ZOOM OUT
	prevState = tmpState;
	tmpState = new ZoomOutOfImageState(1000);
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);	

	// CREATE PILE
	prevState = tmpState;
	tmpState = new CreatePileState(500, "\\_demo.*");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// GRID PILE
	prevState = tmpState;
	tmpState = new GridSelectedPileState(1500);
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// SELECT GRIDDED PILE 
	prevState = tmpState;
	tmpState = new SelectPilesWithFileSystemActorsState(100, "\\_demo.*");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// STACK PILE
	prevState = tmpState;
	tmpState = new StackSelectedPileState(1500);
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// BREAK PILE
	prevState = tmpState;
	tmpState = new BreakSelectedPileState(1500);
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// SORT ALL DEMO FILES INTO PILES
	prevState = tmpState;
	tmpState = new SortFileSystemActorsByTypeState(1000, "\\_demo.*");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// SELECT PILE OF IMAGES
	prevState = tmpState;
	tmpState = new SelectPilesWithFileSystemActorsState(100, "\\_demo[^\\.]*\\.(jpg|gif)");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// GRID PILE
	prevState = tmpState;
	tmpState = new GridSelectedPileState(500);
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// SELECT ALL IMAGE FILES
	prevState = tmpState;
	tmpState = new SelectFileSystemActorsState(1000, "\\_demo[^\\.]*\\.(jpg|gif)");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// ZOOM INTO IMAGE
	prevState = tmpState;
	tmpState = new ZoomIntoImageState(1500, "\\_demo[^\\.]*\\.(jpg|gif)");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// NEXT IMAGE
	prevState = tmpState;
	tmpState = new ZoomToNextImageState(2000);
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// NEXT IMAGE
	prevState = tmpState;
	tmpState = new ZoomToNextImageState(2000);
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// NEXT IMAGE
	prevState = tmpState;
	tmpState = new ZoomToNextImageState(2000);
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// NEXT IMAGE
	prevState = tmpState;
	tmpState = new ZoomToNextImageState(2000);
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// ZOOM OUT
	prevState = tmpState;
	tmpState = new ZoomOutOfImageState(1000);
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);	

	// SELECT PILE OF IMAGES
	prevState = tmpState;
	tmpState = new SelectPilesWithFileSystemActorsState(100, "\\_demo[^\\.]*\\.jpg");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// STACK PILE
	prevState = tmpState;
	tmpState = new StackSelectedPileState(1500);
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// Collect performance data
	prevState = tmpState;
	tmpState = new CollectPerformanceData(0, "All demo tasks - ");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);
/*
	// FANOUT PILE
	// some random func: f(x) = (x/10) ^ 2.4f
	vector<Vec3> points;
	points.push_back(Vec3(0,0,0));
	points.push_back(Vec3(-10, 0, -1));
	points.push_back(Vec3(-15, 0, -2.6462f));
	points.push_back(Vec3(-20, 0, -5.278f));
	points.push_back(Vec3(-25, 0, -9.0169f));
	points.push_back(Vec3(-30, 0, -13.9666f));
	points.push_back(Vec3(-35, 0, -20.2192f));
	points.push_back(Vec3(-40, 0, -27.8576f));
	points.push_back(Vec3(-45, 0, -36.9581f));
	points.push_back(Vec3(-50, 0, -47.5913f));
	points.push_back(Vec3(-55, 0, -59.8233f));
	points.push_back(Vec3(-60, 0, -73.7162f));
	prevState = tmpState;
	tmpState = new FanoutSelectedPileState(1500, points);
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// SELECT ALL IMAGE FILES
	prevState = tmpState;
	tmpState = new SelectFileSystemActorsState(1000, "\\_demo[^\\.]*\\.jpg");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// ZOOM INTO IMAGE
	prevState = tmpState;
	tmpState = new ZoomIntoImageState(1500, "\\_demo[^\\.]*\\.jpg");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// NEXT IMAGE
	prevState = tmpState;
	tmpState = new ZoomToNextImageState(2000);
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// ZOOM OUT
	prevState = tmpState;
	tmpState = new ZoomOutOfImageState(1000);
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);	

	// CLOSE FANNED OUT PILE
	prevState = tmpState;
	tmpState = new CloseFannedOutPileState(500, "\\_demo[^\\.]*\\.jpg");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);
	*/

	// CLEAN UP SCENE
	//prevState = tmpState;
	//tmpState = new FinalizeScene(500);
	//_demoStates.addState(tmpState);
	//_demoStates.addTransition(prevState, tmpState);

	// --------------------------------------
	// Finalization in case the demo is stopped at any time

	// CLEAN UP SCENE
	_finalizeState = new FinalizeScene(500);
	_demoStates.addState(_finalizeState);

	// disable CPU optimizations
	GLOBAL(settings).useCPUOptimizations = true;
}

AutomatedDemo::~AutomatedDemo()
{}

void AutomatedDemo::play()
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
	scnManager->messages()->addMessage(new Message("AutomatedDemo::play", QT_TR_NOOP("Starting Demo Mode"), Message::Ok, clearPolicy));
}

void AutomatedDemo::pause()
{
	// notify the user

	Replayable::pause();
}

void AutomatedDemo::stop()
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
	scnManager->messages()->addMessage(new Message("AutomatedDemo::stop", QT_TR_NOOP("Stopping Demo Mode"), Message::Ok, clearPolicy));
}

Replayable::State AutomatedDemo::update() 
{
	if (getPlayState() == Running)
	{
		if (_timeSinceLastChange.elapsed() >= _durationUntilNextChange)
		{
			bool awaitingCurrentStateFinalizeOut = false;
			if (_demoStates.next(awaitingCurrentStateFinalizeOut))
			{				
				_timeSinceLastChange.restart();
				_durationUntilNextChange = _demoStates.current()->getExpectedDuration();
			} 
			else
			{
				if (awaitingCurrentStateFinalizeOut)
				{
					// if we are still awaiting the current state finalization, then
					// re-update this demo in a bit
					// AKA. do nothing to restart the timer
				}
				else
				{
					// we are not awaiting any states, so just stop the demo
					stop();
				}
			}
		}
	}
	return getPlayState();
}