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

#ifndef _ACTOR_UNDO_DATA_
#define _ACTOR_UNDO_DATA_

// -----------------------------------------------------------------------------

class ActorUndoData
{
public:

	QString actorPath;
	Mat34 orientaiton;
	Vec3 size;

public:

	// Constructors
	inline ActorUndoData(const ActorUndoData &a);
	inline ActorUndoData(Mat34 ori, Vec3 sz, QString path);
};

// -----------------------------------------------------------------------------

#include "BT_ActorUndoData.inl"

// -----------------------------------------------------------------------------

#else
	class ActorUndoData;
#endif