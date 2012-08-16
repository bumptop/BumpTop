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
#include "BT_FiniteStateMachine.h"

FiniteStateMachine::FiniteStateMachine()
: _current(NULL), _dummyState(NULL)
{}

FiniteStateMachine::~FiniteStateMachine()
{
	// free all of the states in the machine
	TransitionMap::iterator iter = _transitions.begin();
	while (iter != _transitions.end())
	{
		// remove the source key
		delete iter->first;
		iter++;
	}
}

bool FiniteStateMachine::contains(FiniteState * state)
{
	return _transitions.find(state) != _transitions.end();
}

void FiniteStateMachine::addState(FiniteState * state)
{
	assert(state);
	assert(!contains(state));

	_transitions.insert(make_pair(state, (FiniteState *)NULL));
}

void FiniteStateMachine::addTransition(FiniteState * fromState, FiniteState * toState)
{
	assert(fromState);
	assert(contains(fromState));
	assert(toState);
	assert(contains(toState));

	_transitions[fromState] = toState;
}

bool FiniteStateMachine::start(FiniteState * state)
{
	assert(state);
	assert(contains(state));

	_current = state;
	if (_current->prepareStateChanged())
	{
		_current->onStateChanged();
		return true;
	}
	return false;
}

FiniteState * FiniteStateMachine::current() const
{
	return _current;
}

bool FiniteStateMachine::next(bool& awaitingCurrentStateFinalizeOut)
{
	assert(_current);
	if (!_current->finalizeStateChanged())
	{
		// we are still waiting for this state to finalize
		awaitingCurrentStateFinalizeOut = true;
		return false;
	}
	else
	{
		awaitingCurrentStateFinalizeOut = false;
		_current->cleanup();

		TransitionMap::iterator iter = _transitions.find(_current);
		if (iter->second)
		{
			if (iter->second->prepareStateChanged())
			{
				_current = iter->second;
				iter->second->onStateChanged();
				return true;
			}
		}
	}
	return false;
}

void FiniteStateMachine::jumpToState( FiniteState * state )
{
	assert(contains(state));

	if (_dummyState == NULL) {
		_dummyState = new FiniteState(-1);
		addState(_dummyState);
	}
	addTransition(_dummyState, state);


	_current->cleanup();

	_current = _dummyState;

	// let the next method jump you to the desired state from the dummy state
	bool dummyBool;
	next(dummyBool);

}
// -----------------------------------------------------------------------------

FiniteState::FiniteState(unsigned int duration)
: _expectedDuration(duration)
{}

FiniteState::~FiniteState()
{}

unsigned int FiniteState::getExpectedDuration() const
{
	return _expectedDuration;
}

bool FiniteState::prepareStateChanged()
{
	return true;
}

void FiniteState::onStateChanged()
{}

bool FiniteState::finalizeStateChanged()
{
	return true;
}

void FiniteState::cleanup()
{}

void FiniteState::setDebuggingInfo(std::string fileName, int lineNumber)
{
	_testFileName = fileName;
	_testLineNumber = lineNumber;
}