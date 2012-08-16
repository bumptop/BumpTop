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

#ifndef _BT_MARKING_MENU_
#define _BT_MARKING_MENU_

// -----------------------------------------------------------------------------

#include "BT_Singleton.h"
#include "BT_KeyboardHandler.h"
#include "BT_ColorVal.h"
#include "BT_MouseEventHandler.h"
#include "BT_Renderable.h"
#include "BT_Macros.h"
#include "BT_ThemeManager.h"
#include "BT_Timer.h"
#include "BT_FontManager.h"

class BumpObject;
class MenuAction;

// -----------------------------------------------------------------------------

enum CompassDirections
{
	North		= 0,
	NorthEast	= 45,
	East		= 90,
	SouthEast	= 135,
	South		= 180,
	SouthWest	= 225,
	West		= 270,
	NorthWest	= 315,
	Ellipsis	= -1,
	NoPref		= -2,
};

// -----------------------------------------------------------------------------

struct MenuLocation
{
	float startDegrees;
	float endDegrees;
	float startRadius;
	float endRadius;
	MenuAction *menuAction;

	MenuLocation()
	{
		startDegrees = endDegrees = startRadius = endRadius = 0;
		menuAction = NULL;
	}
};

// -----------------------------------------------------------------------------

class MarkingMenu : public MouseEventHandler, 
					public KeyboardEventHandler, 
					public ThemeEventHandler,
					public Renderable
{
	weak_ptr<ThemeManager> themeManagerRef;

	vector<MenuAction *> topLevelActions;
	vector<MenuAction *> lowerLevelActions;
	deque<float> sizeMultiplier;
	BumpObject *lastSelectedObject;
	bool flyoutHidden;
	Vec3 lastMousePt;
	float innerDeadZone;
	MenuAction *lastHit;
	MenuAction *currHit;
	bool updateMenu;
	vector<MenuLocation> menuLayout; // <Start Degrees, End Degrees, StartRadius, EndRadius>
	float curDeadzone;
	float maxDeadzone;
	float curThickness;
	float normalThickness;
	float maxThickness;
	Vec2 ballisticHitPt;

	ColorVal textColor;
	ColorVal subTextColor;
	ColorVal textColorDisabled;
	ColorVal borderColor;
	ColorVal innerColor;
	ColorVal outerColor;
	ColorVal innerHighlightColor;
	ColorVal outerHighlightColor;
	FontDescription textFont;
	FontDescription subTextFont;

	bool animating;
	bool ballisticMode;
	uint buttons;
	Vec2 centroid;

	// more menu timer
	// Timer _moreOptionsInvokeDelayTimer;

	// Singleton
	friend class Singleton<MarkingMenu>;
	MarkingMenu();

	// Private Actions
	void			calculateMenuLayout();
	void			calculateOnScreenPosition();
	void			pollMenuActionBiases();
	MenuAction		*hitTest(Vec2 &in);
	void			launchMoreOptionsMenuOnHover();
#ifdef DXRENDER
	void			renderBackground();
#endif

public:

	~MarkingMenu();

	// Actions
	void			init();
	void			update();
	void			prepare();
	void			invoke(Vec2 &mousePos);
	void			offsetFromMouse(Vec2 &mousePos);
	bool			isMenuItemSelected(Vec2 &in);
	void			destroy();

	// Events
	void			onMouseMove(Vec2 &pt);
	bool			onMouseDown(Vec2 &pt, MouseButtons button);
	bool			onMouseUp(Vec2 &pt, MouseButtons button);
	bool			onKeyUp(KeyCombo &key);
	bool			onKeyDown(KeyCombo &key);
	void			onRender(uint flags = RenderSideless);
	virtual void	onThemeChanged();

	// Getters
	bool			isAnimating();
	bool			isHittingMenuWithCallback(const Vec3& pt, VoidCallback cb);
	vector<MenuAction *> getLowerLevelActions() const;
	const ColorVal& getInnerColor() const;
	const ColorVal& getOuterColor() const;
	const ColorVal& getBorderColor() const;
	Vec2			getCentroid();
	float			getRadius();
};

// -----------------------------------------------------------------------------

#define markingMenu Singleton<MarkingMenu>::getInstance()

// -----------------------------------------------------------------------------

#else
	class MarkingMenu;
	enum CompassDirections;
#endif