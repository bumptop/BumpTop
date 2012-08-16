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

#ifndef _BT_TEXT_MANAGER_
#define _BT_TEXT_MANAGER_

// -----------------------------------------------------------------------------

#include "BT_Singleton.h"

// -----------------------------------------------------------------------------

class TextManager
{
	bool _isTextDirty;				// flag to reshape text labels
	bool _ignoreImmediateCollisions;	// flag to ignore collisions on the next update 
	bool _disableForceUpdates;		// flag to ignore force updates

	// Private Actions
	void	reshapeTextLabels(bool ignoreCollisions);	
	
	// Singleton
	friend class Singleton<TextManager>;
	TextManager();

public:

	~TextManager();

	// Actions
	void	invalidate(bool ignoreCollisions = false);
	void	forceValidate();
	bool	updateRelativeNameablePositions();
	void	updateAbsoluteNameablePositions();

	void	enableForceUpdates();
	void	disableForceUpdates();
	void	forceUpdate();

	// Getters
	bool	isValid() const;
};

// -----------------------------------------------------------------------------

#define textManager Singleton<TextManager>::getInstance()

// -----------------------------------------------------------------------------

#else
	class TextManager;
#endif