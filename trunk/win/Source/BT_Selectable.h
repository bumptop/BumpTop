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

#ifndef _BT_SELECTABLE_
#define _BT_SELECTABLE_

// -----------------------------------------------------------------------------

class Selectable
{
protected:

	bool selected;

public:

	// Events
	virtual void onClick();
	virtual void onDoubleClick();
	virtual void onSelect();
	virtual void onDeselect();
};

// -----------------------------------------------------------------------------

#endif