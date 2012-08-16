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

#ifndef _TIMER_
#define _TIMER_

// -----------------------------------------------------------------------------

class Stopwatch
{
	uint _anchorTime;
	uint _pausedTime;
	bool _paused;

	static inline uint max(uint a, uint b);

public:

	inline Stopwatch();

	// Actions
	inline uint restart();
	inline uint elapsed() const;
	inline void increaseBy(uint milliseconds);
	inline void decreaseBy(uint milliseconds);
	inline void pause();
	inline void unpause();

};

// -----------------------------------------------------------------------------

// replaces the boost timer
class StopwatchInSeconds : public Stopwatch
{
public:
	inline StopwatchInSeconds();

	inline double restart();
	inline double elapsed() const;

};
// -----------------------------------------------------------------------------

/*
 * Simple timer composite that contains a condition without which the timer 
 * will not start.
 */
class ConditionalStopwatch
{
	bool _condition;
	Stopwatch _stopwatch;

public:
	inline ConditionalStopwatch();
	inline ConditionalStopwatch(bool initialState);

	// Actions
	inline uint restart();	
	inline uint elapsed();		// returns 0 if the condition is not yet met
	inline void setCondition(bool newState);
	inline bool getCondition() const;
};

// -----------------------------------------------------------------------------

#include "BT_Stopwatch.inl"

// -----------------------------------------------------------------------------

#else
	class Stopwatch;
	class StopwatchInSeconds;
	class ConditionalStopwatch;
#endif