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

#ifndef BT_THREADABLE_UNIT
#define BT_THREADABLE_UNIT

#include "BT_GLTextureObject.h"
#include "BT_Timer.h"

class ThreadableUnit 
{
public:	
	enum State
	{
		Queued,
		Processing,
		ProcessingIdle,		// implies that the thread is safe to terminate
		Complete
	};

	enum ThreadDebugIdentifiers
	{
		GLTextureManagerTexhelperThread		= 1,
		StatsManagerUploadThread			= 2,
		RecycleBinCheckThread				= 3,
		PhotoFrameActorSourceUpdateThread	= 4,
		PhotoFrameActorImageUpdateThread	= 5
	};

private:
	int					_threadDebugIdentifier;
	QMutex				_mutex;
	State				_state;
	boost::function<bool ()>	_runFunc;
	boost::function<bool ()>	_runRepeatedOnceFunc;
	HANDLE				_thread;
	bool				_repeated;
	int					_repeatedDelay;

	// the run result is only valid if the state is Completed
	bool				_result;


private:
	static unsigned int __stdcall pRunFunc(LPVOID lpParameter);

public:
	ThreadableUnit();
	~ThreadableUnit();

	// operations
	void run(boost::function<bool ()> runFunc, int threadDebugIdentifier);
	void reRun();
	void runRepeated(boost::function<bool ()> runRepeatedOnceFunc, boost::function<bool ()> runFunc, int delay, int threadDebugIdentifier);
	void join(int killAfterMillis = -1);
	void markAsSafeToJoin(bool safeToTerminate);
	void setRepeatDelay(int delay);

	// accessors
	State getState();
	bool getResult();
};

// ------------------------------------------------------------------------------------------------
// XXX: templatize these later into the same class
// ------------------------------------------------------------------------------------------------

class ThreadableTextureUnit 
{
public:	
	enum State
	{
		Queued,
		Processing,
		ProcessingIdle,		// implies that the thread is safe to terminate
		Expired,
		Complete
	};

private:
	int					_threadDebugIdentifier;
	mutable QMutex		_mutex;
	State				_state;
	boost::function<int (GLTextureObject)>	_runFunc;
	HANDLE				_thread;
	GLTextureObject		_param;
	Timer				_runtimeTimer;
	// the run result is only valid if the state is Completed
	int					_result;


private:
	static unsigned int __stdcall pRunFunc(LPVOID lpParameter);
	void joinNow();

public:
	ThreadableTextureUnit(const GLTextureObject& param, int maxRuntime);
	ThreadableTextureUnit(const GLTextureObject& param);
	~ThreadableTextureUnit();

	// operations
	void run(boost::function<int (GLTextureObject)> runFunc, int threadDebugIdentifier);
	void join(int killAfterMillis = -1);
	void markAsSafeToJoin(bool safeToTerminate);

	// accessors
	State getState() const;
	int getResult() const;
	GLTextureObject getParam();

};

#endif // BT_THREADABLE_UNITE