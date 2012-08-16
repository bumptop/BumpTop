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
#include "BT_Timer.h"

void Timer::setTimerDuration( int milliseconds )
{
	_timerDurationMs = milliseconds;
}

void Timer::setTimerEventHandler( boost::function<void()> onTimerEvent )
{
	_onTimerEvent = onTimerEvent;
}

void Timer::start()
{
	_timeStarted = timeGetTime();
	if (!_on)
	{
		_on = true;
		Singleton<TimerManager>::getInstance()->registerTimer(this);
	}
}

void Timer::stop()
{
	if (_on)
	{
		_on = false;
		Singleton<TimerManager>::getInstance()->unregisterTimer(this);
	}
}

void Timer::onUpdate()
{
	if (_on && timeGetTime() - _timeStarted > _timerDurationMs)
	{
		_onTimerEvent();
		stop();
	}
}

int Timer::getDuration()
{
	return _timerDurationMs;
}

Timer::Timer()
: _on(false)
, _timerDurationMs(0)
{ }

Timer::~Timer()
{
	if (_on)
		Singleton<TimerManager>::getInstance()->unregisterTimer(this);
}


void TimerManager::onUpdate()
{
	if (!_onUpdateEvent.empty())
		_onUpdateEvent();
	vector<Timer*> timerListCopy = _timerList;
	for_each(Timer* timer, timerListCopy)
		timer->onUpdate();
}

TimerManager::TimerManager()
{}

void TimerManager::registerTimer( Timer* t )
{
	if (find(_timerList.begin(), _timerList.end(), t) == _timerList.end())
	{
		_timerList.push_back(t);
	}
}

void TimerManager::unregisterTimer( Timer* t )
{
	vector<Timer*>::iterator tIterator = find(_timerList.begin(), _timerList.end(), t);
	if (tIterator != _timerList.end())
	{
		_timerList.erase(tIterator);
	} 
}