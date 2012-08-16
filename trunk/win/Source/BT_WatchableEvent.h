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

#pragma once

#ifndef _WATCHABLE_EVENT_
#define _WATCHABLE_EVENT_

// -----------------------------------------------------------------------------

class Watchable;

// -----------------------------------------------------------------------------

enum FileSystemOperation
{
	FileAdded,
	FileDeleted,
	FileRenamed,
	FileModified
};

// -----------------------------------------------------------------------------

class WatchableEvent
{
public:

	Watchable *obj;
	FileSystemOperation watchEvent;
	QString param1;
	QString param2;

public:

	inline WatchableEvent(Watchable *watchObj, FileSystemOperation evnt, QString p1, QString p2);
	inline bool operator==(const WatchableEvent& other);
	inline bool operator!=(const WatchableEvent& other);
};

// -----------------------------------------------------------------------------

#include "BT_WatchableEvent.inl"

// -----------------------------------------------------------------------------

#else
	class WatchableEvent;
	enum FileSystemOperation;
#endif