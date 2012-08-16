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

#ifndef BT_FINITESTATEMACHINE
#define BT_FINITESTATEMACHINE

// -----------------------------------------------------------------------------

#include "BT_Stopwatch.h"

// -----------------------------------------------------------------------------

class FiniteState;

// -----------------------------------------------------------------------------

class FiniteStateMachine
{
	typedef map<FiniteState *, FiniteState *> TransitionMap;
	TransitionMap _transitions;
	FiniteState * _current;
	FiniteState * _dummyState;

private:
	bool contains(FiniteState * state);

public:
	FiniteStateMachine();
	~FiniteStateMachine();

	// operations on the fsm
	void			addState(FiniteState * state);
	void			addTransition(FiniteState * fromState, FiniteState * toState);
	// void			removeState(FiniteState * state);
	// void			removeTransition(FiniteState * fromState, FiniteState * toState);
	
	// interacting with the fsm
	bool			start(FiniteState * state);

	// NOTE: next can return false if the current state is still incomplete
	bool			next(bool& awaitingCurrentStateFinalizeOut);
	
	// forces a jump to a new state, even if the previous one has not completed yet
	// ideally, FiniteStates should have a cleanup function separate from the finalize function
	void			jumpToState(FiniteState * state);

	FiniteState *	current() const;
};

// -----------------------------------------------------------------------------

class FiniteState
{
	unsigned int _expectedDuration;

protected:
	std::string _testFileName;
	int _testLineNumber;

public:
	FiniteState(unsigned int duration);
	~FiniteState();

	// accessors
	unsigned int	getExpectedDuration() const;		// in milliseconds

	// events (onStateChanged/onFinalizeStateChanged is only called if onPrepareStateChanged returns true)
	virtual bool	prepareStateChanged();
	virtual void	onStateChanged();
	virtual bool	finalizeStateChanged();
	virtual void	cleanup();
	virtual void	setDebuggingInfo(std::string fileName, int lineNumber);
};

// -----------------------------------------------------------------------------

#define FSM_STATE(uniqueStateName) class uniqueStateName : public FiniteState {	public: uniqueStateName(unsigned int duration); ~uniqueStateName(); virtual bool prepareStateChanged(); virtual void onStateChanged(); virtual bool finalizeStateChanged();	}

// -----------------------------------------------------------------------------

#endif // BT_FINITESTATEMACHINE