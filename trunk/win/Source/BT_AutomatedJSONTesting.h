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

#ifndef BT_AUTOMATEDJSONTESTSUITE
#define BT_AUTOMATEDJSONTESTSUITE

// -----------------------------------------------------------------------------

#include "BT_FiniteStateMachine.h"
#include "BT_Replayable.h"
#include "BT_OverlayComponent.h"

// -----------------------------------------------------------------------------

class AutomatedJSONTestRunner;

// -----------------------------------------------------------------------------

/*
* Testing procedure read in from a JSON file.
*/

class AutomatedJSONTestSuite
{
	Q_DECLARE_TR_FUNCTIONS(AutomatedJSONTestSuite);
	
//	static QHash<QString, voidFuncPtr> testFunctionMap;
	AutomatedJSONTestRunner * _currentRunner;

	// the state machine
	FiniteStateMachine _testStates;
	FiniteState * _firstState;
	FiniteState * _finalizeState;
	FiniteState * _tmpState;
	FiniteState * _prevState;
		
	QString _testFileName;
	QString _testName;
	QString _testDescription;

	// the AutomatedJSONTestSuite specific states
	FSM_STATE(PrepareScene);
	FSM_STATE(FinalizeScene);

protected:
	
	OverlayLayout* overlay; // for HUD
	TextOverlay* HUDtext;
	VerticalOverlayLayout* textLayout;

public:
	AutomatedJSONTestSuite(QString, QString);
	~AutomatedJSONTestSuite();

	FiniteStateMachine *	getStates();
	FiniteState *			getStartingState();
	FiniteState *			getFinalizeState();
	
	QString					getFileName() const;
	QString					getName() const;
	QString					getDescription() const;

	void					setCurrentRunner(AutomatedJSONTestRunner * runner);
	AutomatedJSONTestRunner *	getCurrentRunner();

	void TestOne();
	void TestTwo();
	void TestThree();
	void TestFour();
	void CreateRenameDeleteFileTest();
	void MoveItemInOutFolder();
	void PileItemsTest();
};

/*
* An automated test runner that runs test suites within bumptop's main loop.
*/
class AutomatedJSONTestRunner : public Replayable
{
	// the test suites
	vector<AutomatedJSONTestSuite *> _testSuites;
	int _currentTestSuiteIndex;
	Stopwatch _timeSinceLastChange;
	unsigned int _durationUntilNextChange;
	int _testPassed, _testFailed;

	AutomatedJSONTestSuite * currentTestSuite();
	void setCurrentTestSuite(int index);

public:
	AutomatedJSONTestRunner();
	~AutomatedJSONTestRunner();

	// operations to add/remove which test suites are run
	void			addTestSuite(AutomatedJSONTestSuite * testSuite);
	void			removeTestSuite(AutomatedJSONTestSuite * testSuite);

	// Replayable
	virtual void	play();
	virtual void	pause();
	virtual void	stop();
	virtual Replayable::State	update();
};
#endif // BT_AUTOMATEDDEMO