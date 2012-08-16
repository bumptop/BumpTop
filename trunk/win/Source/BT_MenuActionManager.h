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

#ifndef _BT_MENU_ACTION_MANAGER_
#define _BT_MENU_ACTION_MANAGER_

// -----------------------------------------------------------------------------

#include "BT_FontManager.h"
#include "BT_MenuAction.h"
#include "BT_Singleton.h"
#include "BT_ThemeManager.h"

class Selection;

// -----------------------------------------------------------------------------

class MenuActionManager : public ThemeEventHandler
{
	Q_DECLARE_TR_FUNCTIONS(MenuActionManager)

	weak_ptr<ThemeManager> themeManagerRef;

	vector<MenuAction> menuActions;
	FontDescription _primaryTextFont, _secondaryTextFont;

	// Singleton
	friend class Singleton<MenuActionManager>;
	MenuActionManager();

public:

	~MenuActionManager();

	// Actions
	void				init();
	vector<MenuAction *>determineMenuActions(Selection *s);
	
	virtual void		onThemeChanged();
#ifdef DXRENDER
	void				onRelease();
#endif
};

// -----------------------------------------------------------------------------

#define menuManager Singleton<MenuActionManager>::getInstance()

// -----------------------------------------------------------------------------

#endif