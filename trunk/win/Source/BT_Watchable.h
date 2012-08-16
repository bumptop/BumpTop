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

#ifndef _WATCHABLE_
#define _WATCHABLE_

// -----------------------------------------------------------------------------

class Watchable
{

public:

	Watchable();
	~Watchable();

	// Events
	virtual void onFileAdded(QString strFileName) = 0;
	virtual void onFileRemoved(QString strFileName) = 0;
	virtual void onFileNameChanged(QString strOldFileName, QString strNewFileName) = 0;
	virtual void onFileModified(QString strFileName) = 0;

	// Getters
	virtual StrList getWatchDir() = 0;
};

// -----------------------------------------------------------------------------

#else
	class Watchable;
#endif