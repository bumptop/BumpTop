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

#ifndef _BT_PROFILER_
#define _BT_PROFILER_

#include "BT_Singleton.h"
#include "BT_Stopwatch.h"

class Profiler
{
	Stopwatch _timer;
	bool _isProfiling;
	uint _numFrames;
	StrList _code_block_names;
	QHash<QString, uint> _code_blocks;
	StrList _repeated_code_names;
	QHash<QString, uint> _repeated_code_blocks;
public:
	Profiler();
	~Profiler();

	bool isProfiling();
	void incrementTime(QString code_block_name, uint ms);
	void incrementRepeated(QString code_block_name, uint ms);
	void start();
	void stop(bool ignoreNumFrames = false);
	void onFrame();
};

#define profiler Singleton<Profiler>::getInstance()

#endif