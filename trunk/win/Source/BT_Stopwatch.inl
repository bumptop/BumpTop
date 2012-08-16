// Copyright 2011 Google Inc. All Rights Reserved.
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

uint Stopwatch::max(uint a, uint b)
{
	return a > b ? a : b;
}

Stopwatch::Stopwatch()
{
	restart();
}

uint Stopwatch::restart()
{
	uint newAnchorTime = timeGetTime();
	uint elapsed = Stopwatch::max(0, newAnchorTime - _anchorTime);
	_anchorTime = newAnchorTime;
	return elapsed;
}

uint Stopwatch::elapsed() const
{
	// This is more accurate then the BOOST timer class
	return Stopwatch::max(0, timeGetTime() - _anchorTime);
}

void Stopwatch::increaseBy(uint milliseconds)
{
	_anchorTime += milliseconds;
}

void Stopwatch::decreaseBy(uint milliseconds)
{
	_anchorTime -= milliseconds;
}

void Stopwatch::pause()
{
	if (!_paused)
	{
		_pausedTime = timeGetTime();
		_paused = true;
	}
}

void Stopwatch::unpause()
{
	if (_paused)
	{
		uint elapsedSincePaused = Stopwatch::max(0, timeGetTime() - _pausedTime);
		increaseBy(elapsedSincePaused);
		_paused = false;
	}
}
// -----------------------------------------------------------------------------

StopwatchInSeconds::StopwatchInSeconds() : Stopwatch()
{}

double StopwatchInSeconds::restart()
{
	return Stopwatch::restart() / 1000.0;
}

double StopwatchInSeconds::elapsed() const
{
	return Stopwatch::elapsed() / 1000.0;
}

// -----------------------------------------------------------------------------

ConditionalStopwatch::ConditionalStopwatch()
: _condition(false)
{}

ConditionalStopwatch::ConditionalStopwatch(bool initiatalState)
: _condition(initiatalState)
{
	setCondition(initiatalState);
}

// Actions
uint ConditionalStopwatch::restart()
{
	_stopwatch.restart();
}

uint ConditionalStopwatch::elapsed()
{
	if (_condition)
		return _stopwatch.elapsed();
	return 0;
}

void ConditionalStopwatch::setCondition(bool newState)
{
	_condition = newState;
	if (newState)
		_stopwatch.restart();
}

bool ConditionalStopwatch::getCondition() const
{
	return _condition;
}