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

#ifndef _BT_StopwatchEVENTSOURCE_
#define _BT_StopwatchEVENTSOURCE_

#include "BT_Singleton.h"

class Timer
{
public:
	Timer();
	~Timer();
	void setTimerDuration(int milliseconds);
	void setTimerEventHandler(boost::function<void()> onTimerEvent);
	void start();
	void stop();
	void onUpdate();
	int getDuration();
protected:
	bool _on;
	int _timerDurationMs;
	int _timeStarted;
	boost::function<void()> _onTimerEvent;


};

class TimerManager
{
public:
	TimerManager();
	void onUpdate();
	void registerTimer(Timer* t);
	void unregisterTimer(Timer* t);
protected:
	friend class Singleton<TimerManager>;
	vector<Timer*> _timerList;
	boost::function<void()> _onUpdateEvent;
};

#endif // _BT_StopwatchEVENTSOURCE_
