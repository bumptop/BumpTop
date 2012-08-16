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
#include "BT_AutomatedTradeshowDemo.h"

// -----------------------------------------------------------------------------

#include "BT_WindowsOS.h"
#include "BT_FileSystemManager.h"
#include "BT_SceneManager.h"
#include "BT_FileSystemActor.h"
#include "BT_CommonFiniteStates.h"
#include "BT_OverlayComponent.h"
#include "BT_Util.h"
#include "BT_Camera.h"

AutomatedTradeshowDemo::PrepareScene::PrepareScene(unsigned int duration)
: FiniteState(duration)
{

}

AutomatedTradeshowDemo::PrepareScene::~PrepareScene()
{}

bool AutomatedTradeshowDemo::PrepareScene::prepareStateChanged()
{
	return true;
}

void AutomatedTradeshowDemo::PrepareScene::onStateChanged()
{
	Key_LoadTradeshowScene();
	CornerInfoControl *cornerInfoControl = cam->getCornerInfoControl();
	cornerInfoControl->init();
	cornerInfoControl->setMessageText(QT_TR_NOOP("BumpTop In Action!\nClick Here to Try"));
}

bool AutomatedTradeshowDemo::PrepareScene::finalizeStateChanged()
{
	// // ensure that all the images have actually loaded
	return true;
}

// -----------------------------------------------------------------------------

// using AutomatedTradeshowDemo::FinalizeScene;

AutomatedTradeshowDemo::FinalizeScene::FinalizeScene(unsigned int duration)
: FiniteState(duration)
{}

AutomatedTradeshowDemo::FinalizeScene::~FinalizeScene()
{}

bool AutomatedTradeshowDemo::FinalizeScene::prepareStateChanged()
{
	return true;
}

void AutomatedTradeshowDemo::FinalizeScene::onStateChanged()
{
	// Need to call this otherwise when the demo finishes we get weird behavior
	// When trying to interact with bumptop. Actors won't move around and we will 
	// Sometimes jump into random states.
	//ClearBumpTop();

	Key_LoadTradeshowScene();
}

bool AutomatedTradeshowDemo::FinalizeScene::finalizeStateChanged()
{
	return true;
}

// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------

AutomatedTradeshowDemo::AutomatedTradeshowDemo()
{
	// Time lapse for keyboard manager
	// maxDelayBetweenKeyDown is a float, whereas the state takes a number of milliseconds
	float keyDownTime = GLOBAL(maxDelayBetweenKeyDown) * 500;
	// Data for zooming into wall
	vector<NxActorWrapper*> walls = GLOBAL(Walls);
	Bounds frontWall = walls[0]->getBoundingBox();
	Vec3 frontCenter, frontExtents;
	frontWall.getCenter(frontCenter);
	frontWall.getExtents(frontExtents);

	QDir desktopPath = scnManager->getWorkingDirectory();
	QString desktopPathStr = native(desktopPath);

	FiniteState * tmpState = NULL, * prevState = NULL;
	
	// PREPARE SCENE (NOTE, we need some time for the textures to load
	_firstState = tmpState = new PrepareScene(5000);
	_demoStates.addState(tmpState);

	

	// SELECT ALL DOCs FILES
	prevState = tmpState;
	tmpState = new SelectFileSystemActorsState(1000, ".*\\.doc");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// ZOOM INTO DOCS
	prevState = tmpState;
	tmpState = new ZoomIntoSelectionState(2000, ".*\\.doc");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// GROW Selection
	prevState = tmpState;
	tmpState = new ResizeActorsState(3000, ".*\\.doc", FileSystem, true, "We can grow objects if they are important", true);
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// ZOOM OUT OF DOCS
	prevState = tmpState;
	tmpState = new ZoomOutOfSelectionState(1000);
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);
	
	// Point Camera to front wall
	prevState = tmpState;
	tmpState = new PointCameraToState(2000, Vec3(frontCenter.x - frontExtents.x, 0, frontCenter.z), "You can control what you see");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// Zoom Camera To Front wall
	prevState = tmpState;
	tmpState = new ZoomCameraToPointState(2500, Vec3(frontCenter.x - frontExtents.x, 0, frontCenter.z), "You can even zoom in using the scroll wheel!");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// ZOOM OUT OF SELECTION
	prevState = tmpState;
	tmpState = new ZoomOutOfSelectionState(1500);
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// SELECT ALL FILES
	prevState = tmpState;
	tmpState = new SelectFileSystemActorsState(1000, ".*");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// ZOOM INTO IMAGE
	prevState = tmpState;
	tmpState = new ZoomIntoImageState(2500, ".*.jpg", "Double clicking an image will start up BumpTop Slideshow");
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

	// FIND WALDO ONE LETTER AT A TIME
	prevState = tmpState;
	tmpState = new FindAsYouTypeState(keyDownTime, KeyCombo('W'), "We can find files by typing in their name. Where's Waldo?");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	prevState = tmpState;
	tmpState = new FindAsYouTypeState(keyDownTime, KeyCombo('A'), "");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);
	
	prevState = tmpState;
	tmpState = new FindAsYouTypeState(keyDownTime, KeyCombo('L'), "");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	prevState = tmpState;
	tmpState = new FindAsYouTypeState(keyDownTime, KeyCombo('D'), "");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	prevState = tmpState;
	tmpState = new FindAsYouTypeState(keyDownTime, KeyCombo('O'), "");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// CREATE PILE
	prevState = tmpState;
	tmpState = new CreatePileState(3000, ".*Waldo.*", "We can pile items just like we can on a regular table.");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// GRID PILE
	prevState = tmpState;
	tmpState = new GridSelectedPileState(3000, "Double clicking a pile will bring up a grid of the items");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// SELECT GRIDDED PILE 
	prevState = tmpState;
	tmpState = new SelectPilesWithFileSystemActorsState(100, ".*Waldo.*");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// STACK PILE
	prevState = tmpState;
	tmpState = new StackSelectedPileState(500);
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// BREAK PILE
	prevState = tmpState;
	tmpState = new BreakSelectedPileState(2500, "We can also break piles when we are done with them");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// SORT ALL FILES INTO PILES
	prevState = tmpState;
	tmpState = new SortFileSystemActorsByTypeState(2500, ".*", "Bumptop also lets you quickly pile all your items by type!");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// SELECT ALL PILES
	prevState = tmpState;
	tmpState = new SelectPilesWithFileSystemActorsState(100, "");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// ZOOM INTO PILES
	prevState = tmpState;
	tmpState = new ZoomIntoSelectionState(2000, "");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);
	
	// SELECT PILE OF DOCS
	prevState = tmpState;
	tmpState = new SelectPilesWithFileSystemActorsState(100, ".*\\.doc");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// LEAF THROUGH PILE
	prevState = tmpState;
	tmpState = new LeafThroughPileState(1500, "Using the scroll wheel you can leaf through a pile");
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// LEAF THROUGH PILE
	prevState = tmpState;
	tmpState = new LeafThroughPileState(1000);
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// LEAF THROUGH PILE
	prevState = tmpState;
	tmpState = new LeafThroughPileState(1000);
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// LEAF THROUGH PILE
	prevState = tmpState;
	tmpState = new LeafThroughPileState(1000);
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// PRINT MESSAGE
	prevState = tmpState;
	tmpState = new PrintMessageState(2000, "This is just a small taste of what BumpTop can do. In a few moments try for yourself.", 5);
	_demoStates.addState(tmpState);
	_demoStates.addTransition(prevState, tmpState);

	// --------------------------------------
	// Finalization in case the demo is stopped at any time

	// CLEAN UP SCENE
	_finalizeState = new FinalizeScene(500);
	_demoStates.addState(_finalizeState);
}

AutomatedTradeshowDemo::~AutomatedTradeshowDemo()
{}

void AutomatedTradeshowDemo::play()
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
	scnManager->messages()->addMessage(new Message("AutomatedTradeshowDemo::play", QT_TR_NOOP("Starting Demo Mode"), Message::Ok, clearPolicy));
}

void AutomatedTradeshowDemo::pause()
{
	// notify the user

	Replayable::pause();
}

void AutomatedTradeshowDemo::stop()
{
	_timeSinceLastChange.restart();
	_durationUntilNextChange = -1;

	// clean up/finalize if we are stopping early
	if (getPlayState() == Running)
	{
		Replayable::stop();
		if (_finalizeState && _demoStates.start(_finalizeState))
		{
			bool tmp;
			do {}
			while (_demoStates.next(tmp));
		}
	}

	// notify the user
	MessageClearPolicy clearPolicy;
	clearPolicy.setTimeout(2);
	scnManager->messages()->addMessage(new Message("AutomatedTradeshowDemo::stop", QT_TR_NOOP("Stopping Demo Mode"), Message::Ok, clearPolicy));
}

void AutomatedTradeshowDemo::quickStop ()
{
	if (getPlayState() == Running)
	{
		cam->zoomToAngledView();
		Replayable::stop();
	}
}

Replayable::State AutomatedTradeshowDemo::update() 
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