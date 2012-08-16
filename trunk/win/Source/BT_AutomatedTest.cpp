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
#include "BT_AutomatedTest.h"
#include "BT_OverlayComponent.h"
#include "BT_SceneManager.h"
#include "BT_Util.h"

// -----------------------------------------------------------------------------

#include "BT_ATestGeneral.h"

#ifdef BTDEBUG
// Automated Test Registration
AutomatedTestSuiteAutoRegister<ATS_General> generalTestSuiteRegistration;
AutomatedTestSuiteAutoRegister<ATS_Cleanup> cleanupTestSuiteRegistration;
#endif

// -----------------------------------------------------------------------------

AutomatedTestSuite::AutomatedTestSuite(QString name, QString description)
: _firstState(NULL)
, _finalizeState(NULL)
, _name(name)
, _description(description)
, _currentRunner(NULL)
{}

AutomatedTestSuite::~AutomatedTestSuite()
{}

FiniteStateMachine * AutomatedTestSuite::getStates()
{
	return &_states;
}

FiniteState * AutomatedTestSuite::getStartingState()
{
	return _firstState;
}

FiniteState * AutomatedTestSuite::getFinalizeState()
{
	return _finalizeState;
}

QString AutomatedTestSuite::getName() const
{
	return _name;
}

QString AutomatedTestSuite::getDescription() const
{
	return _description;
}	

void AutomatedTestSuite::setCurrentRunner(AutomatedTestRunner * runner)
{
	_currentRunner = runner;
}

AutomatedTestRunner * AutomatedTestSuite::getCurrentRunner()
{
	return _currentRunner;
}

// -----------------------------------------------------------------------------

AutomatedTestRunner::AutomatedTestRunner()
:_currentTestSuiteIndex(0)
{}

AutomatedTestRunner::~AutomatedTestRunner()
{
	for(int i = 0; i < _testSuites.size(); i++)
		SAFE_DELETE(_testSuites[i]);
	_testSuites.clear();
}

AutomatedTestSuite * AutomatedTestRunner::currentTestSuite()
{
	assert(_currentTestSuiteIndex > -1);
	return _testSuites[_currentTestSuiteIndex];
}

void AutomatedTestRunner::setCurrentTestSuite(int index)
{
	if (_currentTestSuiteIndex > -1)
		_testSuites[_currentTestSuiteIndex]->setCurrentRunner(NULL);

	_currentTestSuiteIndex = index;
	_testSuites[_currentTestSuiteIndex]->setCurrentRunner(this);
}

void AutomatedTestRunner::addTestSuite(AutomatedTestSuite * testSuite)
{
	assert(find(_testSuites.begin(), _testSuites.end(), testSuite) == _testSuites.end());
	_testSuites.push_back(testSuite);

	if (_currentTestSuiteIndex < 0)
		_currentTestSuiteIndex = 0;
}

void AutomatedTestRunner::play()
{
	assert(currentTestSuite()->getStartingState());

	// start the demo
	currentTestSuite()->getStates()->start(currentTestSuite()->getStartingState());
	_timeSinceLastChange.restart();
	_durationUntilNextChange = currentTestSuite()->getStartingState()->getExpectedDuration();
	Replayable::play();

	// notify the user
	MessageClearPolicy clearPolicy;
	clearPolicy.setTimeout(2);
	scnManager->messages()->addMessage(new Message("AutomatedTestRunner::play", "Starting Automated Tests", Message::Ok, clearPolicy));

	cout << "[" << stdString(currentTestSuite()->getName()) << "]";
}

void AutomatedTestRunner::pause()
{
	// notify the user

	Replayable::pause();
}

void AutomatedTestRunner::stop()
{
	_timeSinceLastChange.restart();
	_durationUntilNextChange = -1;

	// clean up/finalize if we are stopping early
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
	scnManager->messages()->addMessage(new Message("AutomatedTestRunner::stop", "Stopping Automated Tests", Message::Ok, clearPolicy));
}

Replayable::State AutomatedTestRunner::update() 
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
						// if we are still awaiting the current state finalization, then
						// re-update this demo in a bit
						// AKA. do nothing to restart the timer
					}
					else
					{
						// we are not awaiting any states, so we are complete
						cout << " PASSED" << endl;

						stop();
						if ((_currentTestSuiteIndex + 1) < _testSuites.size())
						{
							// move onto the next test suite
							setCurrentTestSuite(_currentTestSuiteIndex + 1);
							play();
						}
						else
						{
							// all the tests have completed!
							cout << "[" << _testSuites.size() << "] Tests Ran" << endl;
						}
					}
				}
			}
		}
	}
	catch (const std::exception& e)
	{
		// this test failed, so move onto the next one
		consoleWrite(QString(" FAILED: %s").arg(e.what()));

		// move onto the next test suite
		stop();
		if ((_currentTestSuiteIndex + 1) < _testSuites.size())
		{
			setCurrentTestSuite(_currentTestSuiteIndex + 1);
			play();
		}
	}
	return getPlayState();
}

// -----------------------------------------------------------------------------

AutomatedTestSuiteRegistry::AutomatedTestSuiteRegistry()
{} 

AutomatedTestSuiteRegistry::~AutomatedTestSuiteRegistry()
{
	// delete all the test suites
	AutomatedTestSuiteRegistrySet::iterator iter = _testSuites.begin();
	while (iter != _testSuites.end())
	{
		delete *iter;
		iter++;
	}
}

bool AutomatedTestSuiteRegistry::contains(AutomatedTestSuite * testSuite) const
{
	return _testSuites.find(testSuite) != _testSuites.end();
}

bool AutomatedTestSuiteRegistry::registerTestSuite(AutomatedTestSuite * testSuite)
{
	assert(testSuite);
	assert(!contains(testSuite));
	_testSuites.insert(testSuite);
	return true;
}

const set<AutomatedTestSuite *>& AutomatedTestSuiteRegistry::getTestSuites()
{
	return _testSuites;
}