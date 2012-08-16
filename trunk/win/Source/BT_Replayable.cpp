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
#include "BT_Replayable.h"

// -----------------------------------------------------------------------------

Replayable::Replayable()
: _currentState(Stopped) 
{}

Replayable::~Replayable()
{}

void Replayable::play()
{
	assert(_currentState != Running);
	assert(_currentState != Completed);
	_currentState = Running;
}

void Replayable::pause()
{
	assert(_currentState == Running);
	_currentState = Paused;
}

void Replayable::stop()
{
	assert(_currentState != Stopped);
	assert(_currentState != Completed);
	_currentState = Stopped;
}

Replayable::State Replayable::getPlayState() const
{
	return _currentState;
}