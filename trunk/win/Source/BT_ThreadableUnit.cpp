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
#include "BT_ThreadableUnit.h"
#include "BT_Util.h"


ThreadableUnit::ThreadableUnit()
: _state(Queued)
, _thread(NULL)
, _result(false)
, _repeated(false)
, _repeatedDelay(0)
, _threadDebugIdentifier(0)
{}

ThreadableUnit::~ThreadableUnit()
{
	// force kill the thread
	join(512);
}

unsigned int __stdcall ThreadableUnit::pRunFunc(LPVOID lpParameter)
{
	ThreadableUnit * unit = (ThreadableUnit *) lpParameter;
	unit->_mutex.lock();
		unit->_state = Processing;
		bool repeated = unit->_repeated;		
	unit->_mutex.unlock();
		bool res = true;
		// if we are repeating, then run the runOnce func
		if (repeated)
			res = unit->_runRepeatedOnceFunc();
		// if not repeating or repeating and runOnce succeeds, then runFunc()
		do
		{
			if (res)
				unit->_runFunc();
			unit->_mutex.lock();
			if (!repeated || !res)
			{
				// thread has completed, just return
				unit->_result = res;
				unit->_state = ThreadableUnit::Complete;
				CloseHandle(unit->_thread);
				unit->_thread = NULL;
				unit->_mutex.unlock();
				break;
			}
			else
			{
				int delay = unit->_repeatedDelay;
				// run the thread again after the specified delay
				unit->_state = ProcessingIdle;
				unit->_mutex.unlock();
				// sleep _after_ unlocking the mutex
				Sleep(delay);
			}	
		} while (repeated);
	return 0;
}

void ThreadableUnit::run(boost::function<bool ()> runFunc, int threadDebugIdentifier)
{
	_mutex.lock();
		// create a new thread which calls the runFunc
		_state = Queued;
		_repeated = false;
		_result = false;
		_runFunc = runFunc;
		_threadDebugIdentifier = threadDebugIdentifier;
		_thread = (HANDLE) _beginthreadex(NULL, 0, &pRunFunc, (LPVOID) this, 0, NULL);
	_mutex.unlock();
}

void ThreadableUnit::reRun()
{
	_mutex.lock();
		// create a new thread which calls the previous runFunc
		_state = Queued;
		_repeated = false;
		_result = false;
		_thread = (HANDLE) _beginthreadex(NULL, 0, &pRunFunc, (LPVOID) this, 0, NULL);
	_mutex.unlock();
}

void ThreadableUnit::runRepeated( boost::function<bool ()> runRepeatedOnceFunc, boost::function<bool ()> runFunc, int delay, int threadDebugIdentifier )
{
	assert(delay > 0);

	_mutex.lock();
		// create a new thread which calls the runFunc over and over again until
		// the thread either joins or runFunc returns false
		_state = Queued;
		_repeated = true;
		_repeatedDelay = delay;
		_result = false;
		_runFunc = runFunc;
		_runRepeatedOnceFunc = runRepeatedOnceFunc;
		_threadDebugIdentifier = threadDebugIdentifier;
		_thread = (HANDLE) _beginthreadex(NULL, 0, &pRunFunc, (LPVOID) this, 0, NULL);
	_mutex.unlock();
}

void ThreadableUnit::join(int killAfterMillis)
{
	// wait until it is safe to terminate
	int count = 0;
	while ((getState() == Processing) &&
		   ((count < killAfterMillis) || (killAfterMillis < 0)))
	{
		Sleep(32);
		count += 32;
	}

	if (getState() == ProcessingIdle ||
		killAfterMillis > -1)
	{
		_mutex.lock();
		if (_thread)
		{
			TerminateThread(_thread, 0);
			CloseHandle(_thread);
		}
		_thread = NULL;
		_state = Complete;
		_mutex.unlock();
	}
}

ThreadableUnit::State ThreadableUnit::getState()
{
	if (_mutex.tryLock())
	{
		State s = _state;
		_mutex.unlock();
		return s;
	}
	else
		return ThreadableUnit::Processing;
}

bool ThreadableUnit::getResult()
{
	// XXX: Note that it is expected that this is only called when the state is complete
	if (getState() == ThreadableUnit::Complete)
		return _result;
	else
		return false;
}

void ThreadableUnit::markAsSafeToJoin(bool safeToTerminate)
{	
	_mutex.lock();
		_state = (safeToTerminate ? ProcessingIdle : Processing);
	_mutex.unlock();
}

void ThreadableUnit::setRepeatDelay( int delay )
{
	_mutex.lock();
		_repeatedDelay = delay;
	_mutex.unlock();
}

// ------------------------------------------------------------------------------------------------


ThreadableTextureUnit::ThreadableTextureUnit(const GLTextureObject& param, int maxRuntime)
: _state(Queued)
, _thread(NULL)
, _result(false)
, _param(param)
{
	_runtimeTimer.setTimerDuration(maxRuntime);
	_runtimeTimer.setTimerEventHandler(boost::bind(&ThreadableTextureUnit::joinNow, this));
}

ThreadableTextureUnit::ThreadableTextureUnit(const GLTextureObject& param)
: _state(Queued)
, _thread(NULL)
, _result(false)
, _param(param)
{}

ThreadableTextureUnit::~ThreadableTextureUnit()
{
	// force kill the thread
	join(512);
}

unsigned int __stdcall ThreadableTextureUnit::pRunFunc(LPVOID lpParameter)
{
	ThreadableTextureUnit * unit = (ThreadableTextureUnit *) lpParameter;
	unit->_mutex.lock();
	unit->_state = Processing;
	unit->_mutex.unlock();
	int res = unit->_runFunc(unit->_param);
	unit->_mutex.lock();
		// thread has completed, just return
		unit->_result = res;
		unit->_state = ThreadableTextureUnit::Complete;
		CloseHandle(unit->_thread);
		unit->_thread = NULL;
	unit->_mutex.unlock();
	return 0;
}

void ThreadableTextureUnit::run(boost::function<int (GLTextureObject)> runFunc, int threadDebugIdentifier)
{
	_mutex.lock();
		// create a new thread which calls the runFunc
		_result = -1;
		_runFunc = runFunc;
		_threadDebugIdentifier = threadDebugIdentifier;
		if (_runtimeTimer.getDuration() > 0)
			_runtimeTimer.start();
		_thread = (HANDLE) _beginthreadex(NULL, 0, &pRunFunc, (LPVOID) this, 0, NULL);
	_mutex.unlock();
}

void ThreadableTextureUnit::join(int killAfterMillis)
{
	// wait until it is safe to terminate
	int count = 0;
	while (getState() == Processing &&
		count < killAfterMillis)
	{
		Sleep(32);
		count += 32;
	}

	if (getState() == ProcessingIdle ||
		killAfterMillis > -1)
	{
		_mutex.lock();
		if (_thread)
		{
			TerminateThread(_thread, 0);
			CloseHandle(_thread);
		}
		_thread = NULL;
		_state = Complete;
		_mutex.unlock();
	}		
}

ThreadableTextureUnit::State ThreadableTextureUnit::getState() const
{
	if (_mutex.tryLock())
	{
		State s = _state;
		_mutex.unlock();
		return s;
	}
	else
		return ThreadableTextureUnit::Processing;
}

int ThreadableTextureUnit::getResult() const
{
	// XXX: Note that it is expected that this is only called when the state is complete
	if (getState() == ThreadableTextureUnit::Complete)
		return _result;
	else
		return -1;
}

void ThreadableTextureUnit::markAsSafeToJoin(bool safeToTerminate)
{	
	_mutex.lock();
		_state = (safeToTerminate ? ProcessingIdle : Processing);
	_mutex.unlock();
}

GLTextureObject ThreadableTextureUnit::getParam()
{
	// NOTE: since this is a read-only variable, we don't have to synchronize access to it
	return _param;
}

void ThreadableTextureUnit::joinNow()
{
	join(0);
	_mutex.lock();
	_state = Expired;
	_mutex.unlock();
}