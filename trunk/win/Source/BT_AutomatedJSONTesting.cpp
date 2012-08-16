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
#include "BT_AutomatedJSONTesting.h"

// -----------------------------------------------------------------------------

#include "BT_Common.h"
#include "BT_WindowsOS.h"
#include "BT_FileSystemManager.h"
#include "BT_SceneManager.h"
#include "BT_FileSystemActor.h"
#include "BT_CommonFiniteStates.h"
#include "BT_OverlayComponent.h"
#include "BT_Util.h"
#include "BT_QtUtil.h"
#include "BT_Logger.h"

// Macros for ease of adding new beginning, middle and final states
#define NEW_FIRST_STATE(newState) _tmpState = newState; _firstState = _tmpState; _testStates.addState(_tmpState); _tmpState->setDebuggingInfo(__FILE__, __LINE__);
#define NEW_STATE(newState) _prevState = _tmpState; _tmpState = newState; _testStates.addState(_tmpState); _testStates.addTransition(_prevState, _tmpState); _tmpState->setDebuggingInfo(__FILE__, __LINE__);
#define NEW_FINAL_STATE(newState) _tmpState = newState; _finalizeState = _tmpState; _testStates.addState(_tmpState); _tmpState->setDebuggingInfo(__FILE__, __LINE__);

AutomatedJSONTestSuite::PrepareScene::PrepareScene(unsigned int duration)
: FiniteState(duration)
{}

AutomatedJSONTestSuite::PrepareScene::~PrepareScene()
{}

bool AutomatedJSONTestSuite::PrepareScene::prepareStateChanged()
{
	return true;
}

void AutomatedJSONTestSuite::PrepareScene::onStateChanged()
{
	// Clear out all files from the training directory 
	StrList dirContents = fsManager->getDirectoryContents(native(winOS->GetTrainingDirectory()));
	for_each(QString filename, dirContents)
	{
		fsManager->deleteFileByName(filename, true); // delete silently
	}

	// Copy files over that we need to the training directory
	QDir dataPath = scnManager->JSONTestFilesPath;
	QDir desktopPath = scnManager->getWorkingDirectory();
	QString desktopPathStr = native(desktopPath);
	QString trainingDir = native(winOS->GetTrainingDirectory());

	int argc;
	LPWSTR *argv = CommandLineToArgvW((LPCWSTR)GetCommandLineA(), &argc);
	
	StrList dirList = fsManager->getDirectoryContents(native(dataPath));
	for (int i = 0; i < dirList.size(); ++i)
	{
		QString fileName = filename(dirList[i]);
		if (!desktopPath.exists(fileName))
			fsManager->copyFileByName(dirList[i], trainingDir, fileName, false, true);
	}

	GLOBAL(skipSavingSceneFile) = true; // This will prevent saving of the scene file

	// Switch BumpTop to use the training directory for now
	scnManager->setWorkingDirectory(winOS->GetTrainingDirectory());
	
	// Let's clear out the desktop for the demo purposes
	ClearBumpTop();
	CreateBumpObjectsFromDirectory(native(winOS->GetTrainingDirectory()));
}

bool AutomatedJSONTestSuite::PrepareScene::finalizeStateChanged()
{
	// Ensure that all the images have actually loaded
	return true;
}

// -----------------------------------------------------------------------------

AutomatedJSONTestSuite::FinalizeScene::FinalizeScene(unsigned int duration)
: FiniteState(duration)
{}

AutomatedJSONTestSuite::FinalizeScene::~FinalizeScene()
{}

bool AutomatedJSONTestSuite::FinalizeScene::prepareStateChanged()
{
	return true;
}

void AutomatedJSONTestSuite::FinalizeScene::onStateChanged()
{
	Key_ReloadScene();
}

bool AutomatedJSONTestSuite::FinalizeScene::finalizeStateChanged()
{
	return true;
}

// -----------------------------------------------------------------------------

AutomatedJSONTestSuite::AutomatedJSONTestSuite(QString testName, QString testFileName)
: _testFileName(testFileName), _testName(testName), _testDescription((LPCSTR)NULL), _prevState(NULL), _tmpState(NULL)
{
	overlay = new OverlayLayout();
	scnManager->registerOverlay(overlay);
	textLayout = new VerticalOverlayLayout();
	overlay->addItem(textLayout);
	HUDtext = new TextOverlay();
	textLayout->addItem(HUDtext);

	// PREPARE SCENE (NOTE, we need some time for the textures to load)
	NEW_FIRST_STATE(new PrepareScene(5000));
	
	// Match JSON value passed in to test name
	if(testName == "TestOne")
		TestOne();
	else if(testName == "TestTwo")
		TestTwo();
	else if(testName == "TestThree")
		TestThree();
	else if(testName == "TestFour")
		TestFour();
	else if(testName == "CreateRenameDeleteFileTest")
		CreateRenameDeleteFileTest();
	else if(testName == "MoveItemInOutFolder")
		MoveItemInOutFolder();
	else if(testName == "PileItemsTest")
		PileItemsTest();
	
	// CLEAN UP SCENE
	NEW_FINAL_STATE(new FinalizeScene(500));
}

void AutomatedJSONTestSuite::TestOne()
{
	_testDescription = "Testing leafing through a pile and searching for demo files.";
	NEW_STATE(new SelectFileSystemActorsState(1000, "\\_demo.*"));
	NEW_STATE(new CreatePileState(500, "\\_demo.*"));
	NEW_STATE(new SelectPilesWithFileSystemActorsState(2000, "\\_demo.*"));
	NEW_STATE(new LeafThroughPileState(1000));
	NEW_STATE(new LeafThroughPileState(1000));
	NEW_STATE(new LeafThroughPileState(1000));
	NEW_STATE(new LeafThroughPileState(1000));
	NEW_STATE(new SelectPilesWithFileSystemActorsState(100, "\\_demo.*"));
	NEW_STATE(new BreakSelectedPileState(1500));
	float keyDownTime = GLOBAL(maxDelayBetweenKeyDown) * 500;
	NEW_STATE(new FindAsYouTypeState(keyDownTime, KeyCombo('D'), "You can do this in BumpTop!!!"));
	NEW_STATE(new FindAsYouTypeState(keyDownTime, KeyCombo('E'), ""));
	NEW_STATE(new FindAsYouTypeState(keyDownTime, KeyCombo('M'), ""));
	NEW_STATE(new FindAsYouTypeState(keyDownTime, KeyCombo('O'), ""));
}

void AutomatedJSONTestSuite::TestTwo()
{
	_testDescription = "This test will zoom into some photos, pile all items and grid them";
	NEW_STATE(new ScatterActorsState(2000, "\\_demo.*"));
	NEW_STATE(new SelectFileSystemActorsState(1000, "\\_demo.*"));
	NEW_STATE(new ZoomIntoImageState(1500, "\\_demo[^\\.]*\\.jpg"));
	NEW_STATE(new ZoomToNextImageState(2000));
	NEW_STATE(new ZoomToNextImageState(2000));
	NEW_STATE(new ZoomToNextImageState(2000));
	NEW_STATE(new ZoomOutOfImageState(2000));
	NEW_STATE(new CreatePileState(500, "\\_demo.*"));
	NEW_STATE(new GridSelectedPileState(1500));
}

void AutomatedJSONTestSuite::TestThree()
{
	_testDescription = "This is a basic successful test.";
	NEW_STATE(new ScatterActorsState(2000, "\\_demo.*"));
}

void AutomatedJSONTestSuite::TestFour()
{
	_testDescription = "Blank test.";
}

void AutomatedJSONTestSuite::CreateRenameDeleteFileTest()
{
	_testDescription = "This test will create, rename both body and extension and then delete the file.";
	NEW_STATE(new AssertNumberOfFileSystemActors(2000, 13));
	NEW_STATE(new SelectFileSystemActorsState(1000, "_demo_document_.txt"));
	NEW_STATE(new RenameFileState(2000, "changeName.txt"));
	NEW_STATE(new RenameFileState(2000, "changeName.changeExt"));
	NEW_STATE(new DeleteFilesState(2000, "changeName.changeExt", false, true)); //Simulate cancel dialog box
	NEW_STATE(new DeleteFilesState(2000, "changeName.changeExt"));
	NEW_STATE(new AssertNumberOfFileSystemActors(2000, 12));
}

void AutomatedJSONTestSuite::MoveItemInOutFolder()
{
	_testDescription = "This test will move an item in and out of a folder.";
	NEW_STATE(new AssertNumberOfFileSystemActors(100, 13));
	NEW_STATE(new FolderizeSelectedFilesState(2000, "_demo_document_.txt", "_demo_folder")); // Folder must already exist
	NEW_STATE(new SelectFileSystemActorsState(2000, "_demo_folder"));
	NEW_STATE(new PileizeSelectedFileSystemActorState(2000));
	NEW_STATE(new RemoveFileSystemActorFromPileState(2000, "_demo_document_.txt"));
	NEW_STATE(new AssertNumberOfFileSystemActors(100, 13));
}

void AutomatedJSONTestSuite::PileItemsTest()
{
	_testDescription = "This test will pile 3 items, move another in and out of the pile, move pile to folder, move out of folder, unpile.";
	NEW_STATE(new AssertNumberOfFileSystemActors(100, 13));
	NEW_STATE(new CreatePileState(2000, ".*.jpg", "We can create piles!"));
	NEW_STATE(new CreatePileState(2000, ".*.tif|.*.jpg", ""));
	NEW_STATE(new RemoveFileSystemActorFromPileState(2000, ".*.tif"));
	NEW_STATE(new ScatterActorsState(2000, ".*.*"));
	NEW_STATE(new SelectPilesWithFileSystemActorsState(2000, "_demo_image_bt_.jpg"));
	NEW_STATE(new FolderizeSelectedPileState(3000, "newly_created_folder"));
	NEW_STATE(new SelectFileSystemActorsState(2000, "newly_created_folder"));
	NEW_STATE(new PileizeSelectedFileSystemActorState(2000));
	NEW_STATE(new BreakSelectedPileState(2000));
	NEW_STATE(new AssertNumberOfFileSystemActors(100, 13))
}

AutomatedJSONTestSuite::~AutomatedJSONTestSuite()
{}

FiniteStateMachine * AutomatedJSONTestSuite::getStates()
{
	return &_testStates;
}

FiniteState * AutomatedJSONTestSuite::getStartingState()
{
	return _firstState;
}

FiniteState * AutomatedJSONTestSuite::getFinalizeState()
{
	return _finalizeState;
}

QString AutomatedJSONTestSuite::getFileName() const
{
	return _testFileName;
}

QString AutomatedJSONTestSuite::getName() const
{
	return _testName;
}

QString AutomatedJSONTestSuite::getDescription() const
{
	return _testDescription;
}	

void AutomatedJSONTestSuite::setCurrentRunner(AutomatedJSONTestRunner * runner)
{
	_currentRunner = runner;
}

AutomatedJSONTestRunner * AutomatedJSONTestSuite::getCurrentRunner()
{
	return _currentRunner;
}

// -----------------------------------------------------------------------------

AutomatedJSONTestRunner::AutomatedJSONTestRunner()
: _testPassed(0), _testFailed(0), _currentTestSuiteIndex(0)
{}

AutomatedJSONTestRunner::~AutomatedJSONTestRunner()
{
	for(int i = 0; i < _testSuites.size(); i++)
		SAFE_DELETE(_testSuites[i]);
	_testSuites.clear();
}

AutomatedJSONTestSuite * AutomatedJSONTestRunner::currentTestSuite()
{
	assert(_currentTestSuiteIndex > -1);
	return _testSuites[_currentTestSuiteIndex];
}

void AutomatedJSONTestRunner::setCurrentTestSuite(int index)
{
	if (_currentTestSuiteIndex > -1)
		_testSuites[_currentTestSuiteIndex]->setCurrentRunner(NULL);

	_currentTestSuiteIndex = index;
	_testSuites[_currentTestSuiteIndex]->setCurrentRunner(this);
}

void AutomatedJSONTestRunner::addTestSuite(AutomatedJSONTestSuite * testSuite)
{
	assert(find(_testSuites.begin(), _testSuites.end(), testSuite) == _testSuites.end());
	_testSuites.push_back(testSuite);

	if (_currentTestSuiteIndex < 0)
		_currentTestSuiteIndex = 0;
}

void AutomatedJSONTestRunner::play()
{
	// Disable CPU optimizations
	GLOBAL(settings).useCPUOptimizations = false;
	
	assert(currentTestSuite()->getStartingState());

	// start the demo
	currentTestSuite()->getStates()->start(currentTestSuite()->getStartingState());
	_timeSinceLastChange.restart();
	_durationUntilNextChange = currentTestSuite()->getStartingState()->getExpectedDuration();
	Replayable::play();

	// notify the user
	MessageClearPolicy clearPolicy;
	clearPolicy.setTimeout(2);
	scnManager->messages()->addMessage(new Message("AutomatedJSONTestRunner::play", "Starting New JSON Test \nFile Name: " + currentTestSuite()->getFileName() + 
		"\nTest Name: " + currentTestSuite()->getName() + "\nTest Description: " + currentTestSuite()->getDescription(), Message::Ok, clearPolicy));
	
	// Write the name of the JSON test script when the first test suite of the script is played
	if(_currentTestSuiteIndex == 0 || _testSuites[_currentTestSuiteIndex-1]->getFileName() != currentTestSuite()->getFileName())
		TEST_LOG(QString("\nRUNNING TEST " + currentTestSuite()->getFileName()));
}

void AutomatedJSONTestRunner::pause()
{
	// notify the user
	Replayable::pause();
}

void AutomatedJSONTestRunner::stop()
{
	_timeSinceLastChange.restart();
	_durationUntilNextChange = -1;

	// Clean up/finalize if we are stopping early
	if (getPlayState() == Running)
	{
		if (currentTestSuite()->getFinalizeState() && 
			currentTestSuite()->getStates()->start(currentTestSuite()->getFinalizeState()))
		{
			bool tmp;
			do {}
			while (currentTestSuite()->getStates()->next(tmp));
		}
	}

	Replayable::stop();

	// notify the user
	MessageClearPolicy clearPolicy;
	clearPolicy.setTimeout(2);
	scnManager->messages()->addMessage(new Message("AutomatedJSONTestRunner::stop", "Stopping JSON Test", Message::Ok, clearPolicy));

	// Enable CPU optimizations
	GLOBAL(settings).useCPUOptimizations = true;
}

Replayable::State AutomatedJSONTestRunner::update() 
{
	try {
		if (getPlayState() == Running)
		{
			if (_timeSinceLastChange.elapsed() >= _durationUntilNextChange)
			{
				bool awaitingCurrentStateFinalizeOut = false;
				if (currentTestSuite()->getStates()->next(awaitingCurrentStateFinalizeOut))
				{				
					_timeSinceLastChange.restart();
					_durationUntilNextChange = currentTestSuite()->getStates()->current()->getExpectedDuration();
				} 
				else
				{
					if (awaitingCurrentStateFinalizeOut)
					{
						// If we are still awaiting the current state finalization, then
						// re-update this demo in a bit
						// AKA. Do nothing to restart the timer
					}
					else
					{
						// we are not awaiting any states, so we are complete
						TEST_LOG(QString(currentTestSuite()->getName() + " : PASSED"));
						_testPassed++;

						stop();
						if ((_currentTestSuiteIndex + 1) < _testSuites.size())
						{
							// Move onto the next test suite
							setCurrentTestSuite(_currentTestSuiteIndex + 1);
							play();
						}
						else
						{
							// All the tests have completed!
							TEST_LOG(QString("\nTESTING COMPLETE: %1 Tests Have Run %2 PASSED %3 FAILED").arg(_testSuites.size()).arg(_testPassed).arg(_testFailed));
							ExitBumptop();
						}
					}
				}
			}
		}
	}
	catch (const std::exception& e)
	{
		// Notify the user that the test failed
		MessageClearPolicy clearPolicy;
		clearPolicy.setTimeout(2);
		scnManager->messages()->addMessage(new Message("AutomatedJSONTestRunner::fail", "TEST FAILED: See log for details", Message::Ok, clearPolicy));
		TEST_LOG(QString(currentTestSuite()->getName() + " : FAILED \n\tTest Description : " + currentTestSuite()->getDescription() + "\n\tError Message : %1").arg(e.what()));
		_testFailed++;

		// Move onto the next test suite
		stop();
		if ((_currentTestSuiteIndex + 1) < _testSuites.size())
		{
			setCurrentTestSuite(_currentTestSuiteIndex + 1);
			play();
		} else {
			// We get here if the final test fails
			TEST_LOG(QString("\nTESTING COMPLETE: %1 Tests Have Run %2 PASSED %3 FAILED").arg(_testSuites.size()).arg(_testPassed).arg(_testFailed));
			ExitBumptop();
		}			
	}

	return getPlayState();
}

#undef NEW_STATE
#undef NEW_FIRST_STATE