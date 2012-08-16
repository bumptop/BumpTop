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

#ifndef BT_AUTOMATEDTEST
#define BT_AUTOMATEDTEST

// -----------------------------------------------------------------------------

#include "BT_FiniteStateMachine.h"
#include "BT_Replayable.h"
#include "BT_Singleton.h"

// -----------------------------------------------------------------------------

class AutomatedTestRunner;

// -----------------------------------------------------------------------------

class AutomatedTestSuite
{
	AutomatedTestRunner * _currentRunner;

protected:
	// the state machine
	FiniteStateMachine _states;
	FiniteState * _firstState;
	FiniteState * _finalizeState;

	QString _name;
	QString _description;

public:
	AutomatedTestSuite(QString name, QString description);
	~AutomatedTestSuite();

	FiniteStateMachine *	getStates();
	FiniteState *			getStartingState();
	FiniteState *			getFinalizeState();

	QString					getName() const;
	QString					getDescription() const;

	// allows test suites to query the current running environment
	void					setCurrentRunner(AutomatedTestRunner * runner);
	AutomatedTestRunner *	getCurrentRunner();
};

// -----------------------------------------------------------------------------

/*
* An automated test runner that runs test suites within bumptop's main loop.
*/
class AutomatedTestRunner : public Replayable
{
	// the test suites
	vector<AutomatedTestSuite *> _testSuites;
	int _currentTestSuiteIndex;
	Stopwatch _timeSinceLastChange;
	unsigned int _durationUntilNextChange;

public:
	enum Verbosity
	{
		Concise,
		Verbose
	};

private:
	AutomatedTestSuite * currentTestSuite();
	void setCurrentTestSuite(int index);

public:
	AutomatedTestRunner();
	~AutomatedTestRunner();

	// operations to add/remove which test suites are run
	void			addTestSuite(AutomatedTestSuite * testSuite);
	// void			removeTestSuite(AutomatedTestSuite * testSuite);

	// Replayable
	virtual void	play();
	virtual void	pause();
	virtual void	stop();
	virtual Replayable::State	update();
};

// -----------------------------------------------------------------------------

/*
* A registry of AutomatedTestSuites.
*/
class AutomatedTestSuiteRegistry
{
	typedef set<AutomatedTestSuite *> AutomatedTestSuiteRegistrySet;
	AutomatedTestSuiteRegistrySet _testSuites;

private:
	friend class Singleton<AutomatedTestSuiteRegistry>;
	AutomatedTestSuiteRegistry();

	bool contains(AutomatedTestSuite * testSuite) const;

public:
	~AutomatedTestSuiteRegistry();

	bool			registerTestSuite(AutomatedTestSuite * testSuite);
	// bool			unregisterTestSuite(AutomatedTestSuite * testSuite);

	const set<AutomatedTestSuite *>&	getTestSuites();
};

// -----------------------------------------------------------------------------

/*
* A helper class to registry AutomatedTestSuites easily.
*/
template<class T>
class AutomatedTestSuiteAutoRegister
{
public:
	AutomatedTestSuiteAutoRegister()
	{
		AutomatedTestSuite * suite = new T;
		Singleton<AutomatedTestSuiteRegistry>::getInstance()->registerTestSuite(suite);
	}

	// don't bother unregistering for now
};

#endif // BT_AUTOMATEDTEST